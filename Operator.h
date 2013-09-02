#pragma once

#include <stdexcept>
#include "boost/mpl/at.hpp"
#include "cartesian.hpp"
#include "storage.h"

// Called by SFINAE if reserve does not exist or is not accessible
template <typename TP, typename... ARGS>
constexpr auto has_special(TP t, ARGS... args) -> decltype(t->execute_special(std::forward<ARGS>(args)...), bool()) { return true; }

// Used as fallback when SFINAE culls the template method
template <typename... ARGS>
constexpr bool has_special(ARGS...) { return false; }

template <typename T>
inline  constexpr std::size_t typeId() { return typeid(T).hash_code(); }

template <typename T, typename S, typename D>
bool matchingTypeIds(ATable* t, AStorage* s, ADictionary* d) {
  return (t->getTypeId() == T::typeId) && (s->getTypeId() == S::typeId) && (d->getTypeId() == D::typeId);
}

class ImplementationFound {};

// Call specialized implementation when available
// Just slightly ugly: enable_if is valid when: op.execute_special(...) is overloaded exactly for tall params
// through this specialization, we don't need a fallback execute_special for abstract base classes
template <class OP,
          class TABLE,
          class STORAGE,
          class DICT>
/* restricting unnamed parameter */
//typename std::enable_if<has_special(HasExecuteSpecial<OP, void, TABLE*, STORAGE*, DICT*>::value, int>::type = 0>
auto call_special(OP& op, TABLE* table, STORAGE* store, DICT* dict) -> typename std::enable_if<has_special((OP*) 0, (TABLE*) 0, (STORAGE*) 0, (DICT*) 0), void>::type {
  /// extracting table/store/dict actual typeIds through virtual function calls
  /// and compare to what we need for thise combination of types
  // op.checks++;
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

template <typename OperatorType>
class Operator {
 public:
  virtual ~Operator() {}
  std::size_t checks = 0;
  void execute(ATable* tab, AStorage* store, ADictionary* dict) {
    choose_special<OperatorType> ci { *static_cast<OperatorType*>(this), tab, store, dict };
    try {
      // generates the cartesian product of all types and per
      // combination SEQUENCE, invokes choose_impl<SEQUENCE>()
      typedef boost::mpl::vector<table_types, storage_types, dictionary_types> types;
      boost::mpl::cartesian_product<types>(ci);
    } catch (const ImplementationFound&) {
      return;
    }

    execute_fallback(tab, store, dict);
  }

  virtual void execute_fallback(ATable*, AStorage*, ADictionary*) = 0;
};
