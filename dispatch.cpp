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

using type_id_t = strong_typedef<std::size_t>;

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
  type_id_t getTypeId() const override { return this->typeId; }
  static type_id_t typeId;
};

template <typename B, typename RegisteringClass, typename RegisteredTypes>
type_id_t BaseType<B, RegisteringClass, RegisteredTypes>::typeId { boost::mpl::find<RegisteredTypes, RegisteringClass>::type::pos::value };



class ADictionary : public Typed {};

class OrderedDictionary : public BaseType<ADictionary, OrderedDictionary, dictionary_types> {};
class UnorderedDictionary : public BaseType<ADictionary, UnorderedDictionary, dictionary_types> {};

using value_id_t = std::uint8_t;

class AStorage : public Typed {
 public:
  ~AStorage() {}
  virtual void set(std::size_t x, value_id_t vid) = 0;
  virtual value_id_t get(std::size_t x) const = 0;
};

class FixedStorage : public BaseType<AStorage, FixedStorage, storage_types> {
 public:
  explicit FixedStorage(std::size_t len) : _values(len, 0) {}
  void set(std::size_t x, value_id_t vid) { _values[x] = vid; };
  value_id_t get(std::size_t x) const { return _values[x]; }
  const value_id_t& getRef(std::size_t x) const { return _values[x]; }
 private:
  std::vector<value_id_t> _values;
};

class BitStorage : public BaseType<AStorage, BitStorage, storage_types> {
 public:
  explicit BitStorage(std::size_t len) : _values(len, 0) {}
  void set(std::size_t x, value_id_t vid) { _values[x] = vid; };
  value_id_t get(std::size_t x) const { return _values[x]; }
  const value_id_t& getRef(std::size_t x) const { return _values[x]; }
 private:
  std::vector<value_id_t> _values;
};

class ATable : public Typed {};

class Table : public BaseType<ATable, Table, table_types> {};
class RawTable : public BaseType<ATable, RawTable, table_types> {};

class Operator {
  // Operator defines special overloads for different combinations of
  // ATable, AStorage, ADictionary descendants, so we can write
  // special implementations
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

  template<class ATABLE,
           class ASTORAGE,
           class ADICT,
           /* protect from random invokations withouth at least being
              part of the inheritance chain */
           typename std::enable_if<std::is_base_of<ATable, ATABLE>::value, int>::type = 0,
           typename std::enable_if<std::is_base_of<AStorage, ASTORAGE>::value, int>::type = 0,
           typename std::enable_if<std::is_base_of<ADictionary, ADICT>::value, int>::type = 0>
  void execute_general(ATABLE*, ASTORAGE* as, ADICT*) {
    debug("General implementation kicks in");
    as->ASTORAGE::get(0); // no virtual function call, uses actual types
  }
  
  void execute_fallback(ATable*, AStorage* as, ADictionary*) {
    debug("Fallback kicks in");
    as->get(0);
  }

  // Dispatch method
  void execute(ATable*, AStorage*, ADictionary*);
};

/// Checks for existance of a method named "execute_special"
template<typename T, typename RESULT, typename... ARGS>
class HasExecuteSpecial {
  template <typename U, RESULT (U::*)(ARGS...)> struct Check;
  template <typename U> static std::true_type test(Check<U, &U::execute_special> *) ;
  template <typename U> static std::false_type test(...);
 public:
  typedef HasExecuteSpecial type;
  static const bool value = decltype(test<T>(0))::value;
};

template <typename T, typename S, typename D>
bool matchingTypeIds(ATable* t, AStorage* s, ADictionary* d) {
  return (t->getTypeId() == T::typeId) && (s->getTypeId() == S::typeId) && (d->getTypeId() == D::typeId);
}

// Call specialized implementation when available
// Just slightly ugly: enable_if is valid when: op.execute_special(...) is overloaded exactly for tall params
// through this specialization, we don't need a fallback execute_special for abstract base classes
template <class OP,
          class TABLE,
          class STORAGE,
          class DICT,
          /* restricting unnamed parameter */
          typename std::enable_if<HasExecuteSpecial<OP, void, TABLE*, STORAGE*, DICT*>::value, int>::type = 0>
void call_special(OP& op, TABLE* table, STORAGE* store, DICT* dict) {
  /// extracting table/store/dict actual typeIds through virtual function calls
  /// and compare to what we need for thise combination of types
  if (matchingTypeIds<TABLE, STORAGE, DICT>(table, store, dict)) {
    debug("Matching special found, executing");
    op.execute_special(table, store, dict);
  }
}

template <class OP,
          class TABLE,
          class STORAGE,
          class DICT,
          typename std::enable_if<!HasExecuteSpecial<OP, void, TABLE*, STORAGE*, DICT*>::value, int>::type = 0>
// Is valid when there is no viable overload in op for the given
// params -- don't do anything, there is no match here
void call_special(OP& op, TABLE* table, STORAGE* store, DICT* dict) {}

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


template <class OP>
struct general_impl {
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
    if (matchingTypeIds<TABLE, STORAGE, DICT>(table, store, dict)) {
      op.execute_general(static_cast<TABLE*>(table), static_cast<STORAGE*>(store), static_cast<DICT*>(dict));
    }
  }
};


typedef boost::mpl::vector<table_types, storage_types, dictionary_types> types;

void Operator::execute(ATable* tab, AStorage* store, ADictionary* dict) {
  choose_special<Operator> ci { *this, tab, store, dict };
  // generates the cartesian product of all types and per
  // combination SEQUENCE, invokes choose_impl<SEQUENCE>()
  boost::mpl::cartesian_product<types>(ci);

  // todo: if one impl was found, don't continue with general impl
  general_impl<Operator> gi { *this, tab, store, dict };
  boost::mpl::cartesian_product<types>(gi);

  // todo: if no impl was found, ie types were not registered
  execute_fallback(tab, store, dict);
}

int main (int argc, char const *argv[]) {  
  ADictionary* dict = new UnorderedDictionary;
  ADictionary* odict = new OrderedDictionary;
  ATable* tab = new Table;
  AStorage* store = new FixedStorage(10);
  AStorage* bitstore = new BitStorage(10);

  debug("uo", dict->getTypeId(), UnorderedDictionary::typeId);
  debug("o", odict->getTypeId(), OrderedDictionary::typeId);

  Operator o;
  
  o.execute(tab, store, dict); // Executes
  std::cout << "TAB, BS, OD" << std::endl;
  o.execute(tab, bitstore, odict);
  return 0;
}
