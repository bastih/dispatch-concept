#include <iostream>
#include <stdexcept>
#include <vector>

#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/find.hpp>

#include "boost/mpl/at.hpp"
#include "boost/mpl/contains.hpp"

#include "cartesian.hpp"
#include "strong_typedef.hpp"
#include "debug.hpp"

class ATable;
class ADictionary;
class AStorage;
class Table;
class RawTable;
class FixedStorage;
class BitStorage;
class OrderedDictionary;
class UnorderedDictionary;

// dispatch types
using table_types = boost::mpl::vector<Table, RawTable>;
using storage_types = boost::mpl::vector<FixedStorage, BitStorage>;
using dictionary_types = boost::mpl::vector<OrderedDictionary, UnorderedDictionary>;

using type_id_t = char;

class Typed {
 public:
  virtual ~Typed() {}
  virtual type_id_t getTypeId() const = 0;

};

template <typename BaseClass, class RegisteringClass, typename RegisteredTypes>
class BaseType : public std::enable_if<std::is_base_of<Typed, BaseClass>::value, BaseClass>::type {
 public:
  static_assert(boost::mpl::contains<RegisteredTypes, RegisteringClass>::value,
                "RegisteringClass needs to be in RegisteredTypes");
  type_id_t getTypeId() const override { return typeId; }
  static type_id_t typeId;
};

template <typename B, typename RegisteringClass, typename RegisteredTypes>
type_id_t BaseType<B, RegisteringClass, RegisteredTypes>::typeId { boost::mpl::find<RegisteredTypes, RegisteringClass>::type::pos::value };

class ADictionary : public Typed {};

class OrderedDictionary final : public BaseType<ADictionary, OrderedDictionary, dictionary_types> {};
class UnorderedDictionary final : public BaseType<ADictionary, UnorderedDictionary, dictionary_types> {};

using value_id_t = std::uint8_t;

class AStorage : public Typed {
 public:
  ~AStorage() {}
  virtual void set(std::size_t x, value_id_t vid) = 0;
  virtual value_id_t get(std::size_t x) const = 0;
};

class FixedStorage final : public BaseType<AStorage, FixedStorage, storage_types> {
 public:
  explicit FixedStorage(std::size_t len) : _values(len, 0) {}
  void set(std::size_t x, value_id_t vid) { _values[x] = vid; }
  value_id_t get(std::size_t x) const { return _values[x]; }
  const value_id_t& getRef(std::size_t x) const { return _values[x]; }
 private:
  std::vector<value_id_t> _values;
};

class BitStorage final : public BaseType<AStorage, BitStorage, storage_types> {
 public:
  explicit BitStorage(std::size_t len) : _values(len, 0) {}
  void set(std::size_t x, value_id_t vid) { _values[x] = vid; }
  value_id_t get(std::size_t x) const { return _values[x]; }
  const value_id_t& getRef(std::size_t x) const { return _values[x]; }
 private:
  std::vector<value_id_t> _values;
};

class ATable : public Typed {};
class Table final : public BaseType<ATable, Table, table_types> {};
class RawTable final : public BaseType<ATable, RawTable, table_types> {};

template <typename OperatorType>
class Operator {
  // Operator defines special overloads for different combinations of
  // ATable, AStorage, ADictionary descendants, so we can write
  // special implementations
 public:
  // Dispatch method
  virtual ~Operator() {}
  void execute(ATable*, AStorage*, ADictionary*);
  virtual void execute_fallback(ATable*, AStorage*, ADictionary*);
};

class OperatorImpl : public Operator<OperatorImpl> {
 public:
  void execute_special(Table*, BitStorage* bs, OrderedDictionary*) {
    debug("Special for Table*, BitStorage*, OrderedDictionary*");
    bs->BitStorage::get(1); // Not a virtual function call
  }

  void execute_special(Table*, FixedStorage* fs, OrderedDictionary*) {
    debug("Special for Table*, FixedStorage*, OrderedDictionary*");
    fs->FixedStorage::getRef(1); // not a virtual function call
  }

  void execute_special(Table*, FixedStorage*, UnorderedDictionary*) {
    debug("Special for Table*, FixedStorage*, UnorderedDictionary*");
  }
  
  void execute_fallback(ATable*, AStorage* as, ADictionary*) {
    debug("Fallback kicks in");
    as->get(0);
  }

};

// Called by SFINAE if reserve does not exist or is not accessible
template <typename TP, typename... ARGS>
constexpr auto has_special(TP t, ARGS... args) -> decltype(t->execute_special(std::forward<ARGS>(args)...), bool()) { return true; }

// Used as fallback when SFINAE culls the template method
template <typename... ARGS>
constexpr bool has_special(ARGS...) { return false; }

template <typename T, typename S, typename D>
bool matchingTypeIds(ATable* t, AStorage* s, ADictionary* d) {
  return (t->getTypeId() == T::typeId) && (s->getTypeId() == S::typeId) && (d->getTypeId() == D::typeId);
}

class ImplementationFound {};

template <class T>
using ptr = T*;

// Call specialized implementation when available
// Just slightly ugly: enable_if is valid when: op.execute_special(...) is overloaded exactly for tall params
// through this specialization, we don't need a fallback execute_special for abstract base classes
template <class OP,
          class TABLE,
          class STORAGE,
          class DICT>
          /* restricting unnamed parameter */
          //typename std::enable_if<has_special(HasExecuteSpecial<OP, void, TABLE*, STORAGE*, DICT*>::value, int>::type = 0>
auto call_special(OP& op, TABLE* table, STORAGE* store, DICT* dict) -> typename std::enable_if<has_special(ptr<OP>(), (TABLE*) 0, (STORAGE*) 0, (DICT*) 0), void>::type {
  /// extracting table/store/dict actual typeIds through virtual function calls
  /// and compare to what we need for thise combination of types
  if (matchingTypeIds<TABLE, STORAGE, DICT>(table, store, dict)) {
    op.execute_special(table, store, dict);
    // UGLY: Exceptions for control flow
    throw ImplementationFound();
  }
}

template <class OP,
          class TABLE,
          class STORAGE,
          class DICT> 
//typename std/::enable_if<!HasExecuteSpecial<OP, void, TABLE*, STORAGE*, DICT*>::value, int>::type = 0>
// Is valid when there is no viable overload in op for the given
// params -- don't do anything, there is no match here
auto call_special(OP&, TABLE*, STORAGE*, DICT*) -> typename std::enable_if<not has_special((OP*) 0, (TABLE*) 0, (STORAGE*) 0, (DICT*) 0), void>::type {}

template <class OP>
struct choose_special {
  OP& op;
  ATable* table;
  AStorage* store;
  ADictionary * dict;

  template <typename SEQUENCE>
  inline void operator()() {
    using TABLE = typename boost::mpl::at_c<SEQUENCE, 0>::type;
    using STORAGE = typename boost::mpl::at_c<SEQUENCE, 1>::type;
    using DICT = typename boost::mpl::at_c<SEQUENCE, 2>::type;
    // cast all operators to their presumed types, so we can invoke
    // the correct implementation
    call_special(op, static_cast<TABLE*>(table), static_cast<STORAGE*>(store), static_cast<DICT*>(dict));
  }
};


typedef boost::mpl::vector<table_types, storage_types, dictionary_types> types;

template <typename OperatorType>
void Operator<OperatorType>::execute(ATable* tab, AStorage* store, ADictionary* dict) {
  choose_special<OperatorType> ci { *static_cast<OperatorType*>(this), tab, store, dict };
  // generates the cartesian product of all types and per
  // combination SEQUENCE, invokes choose_impl<SEQUENCE>()
  try {
    boost::mpl::cartesian_product<types>(ci);
  } catch (const ImplementationFound&) {
    return;
  }

  // todo: if no impl was found, ie types were not registered
  execute_fallback(tab, store, dict);
}

int main (int argc, char const *argv[]) {  
  ADictionary* dict = new UnorderedDictionary;
  ADictionary* odict = new OrderedDictionary;
  ATable* tab = new Table;
  AStorage* store = new FixedStorage(10);
  AStorage* bitstore = new BitStorage(10);
  OperatorImpl o;
  
  o.execute(tab, store, dict); // Executes
  debug("TAB, BS, OD");
  o.execute(tab, bitstore, odict);
  debug("No specialization:");
  o.execute(tab, bitstore, dict);
  return 0;
}
