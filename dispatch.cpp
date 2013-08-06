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
 public:
  void execute_special(Table*, BitStorage*, OrderedDictionary*) {
    std::cout << "Special for Table*, BitStorage*, OrderedDictionary*" << std::endl;
  }

  void execute_special(Table*, FixedStorage*, OrderedDictionary*) {
    std::cout << "Special for Table*, FixedStorage*, OrderedDictionary*" << std::endl;
  }

  void execute_special(Table*, FixedStorage*, UnorderedDictionary*) {
    std::cout << "Special for Table*, FixedStorage*, UnorderedDictionary*" << std::endl;
  }
  
  void execute_fallback(ATable*, AStorage*, ADictionary*) {
    
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

// Call specialized implementation when available
// Just slightly ugly: enable_if is valid when: op.execute_special(...) is overloaded exactly for tall params
// through this specialization, we don't need a fallback execute_special for abstract base classes
template <class OP,
          class TABLE,
          class STORAGE,
          class DICT,
          /* restricting unnamed parameter */
          typename std::enable_if<HasExecuteSpecial<OP, void, TABLE*, STORAGE*, DICT*>::value, int>::type = 0>
void call_impl(OP& op, TABLE* table, STORAGE* store, DICT* dict) {
  /// extracting table/store/dict actual typeIds through virtual function calls
  /// and compare to what we need for thise combination of types
  const auto types = std::make_tuple(table->getTypeId(), store->getTypeId(), dict->getTypeId());
  const auto expected_types = std::make_tuple(TABLE::typeId, STORAGE::typeId, DICT::typeId);
  if (types == expected_types) {
    std::cout << "Matching special found, executing" << std::endl;
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
void call_impl(OP& op, TABLE* table, STORAGE* store, DICT* dict) {}

template <class OP>
struct choose_impl {
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
    call_impl(op, static_cast<TABLE*>(table), static_cast<STORAGE*>(store), static_cast<DICT*>(dict));
  }
};

typedef boost::mpl::vector<table_types, storage_types, dictionary_types> types;

void Operator::execute(ATable* tab, AStorage* store, ADictionary* dict) {
  choose_impl<Operator> ci { *this, tab, store, dict };
  // generates the cartesian product of all types and per
  // combination SEQUENCE, invokes choose_impl<SEQUENCE>()
  boost::mpl::cartesian_product<types>(ci);
}

template<typename T>
void debug(T arg) {
  std::cout << arg << std::endl;
}

template <typename T, typename... ARGS>
void debug(T arg, ARGS... args) {
  std::cout << arg << " ";
  debug(args...);
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
