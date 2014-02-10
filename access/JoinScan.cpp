#include "JoinScan.h"

#include <iostream>

#include "dispatch2/dispatch.h"
#include "storage/alltypes.h"

using jtables = std::tuple<Table*>;
using jstorages = std::tuple<FixedStorage*, BitStorage<2>* >;

template <typename ValueType>
using jdicts = std::tuple<OrderedDictionary<ValueType>*,
                          UnorderedDictionary<ValueType>* >;

struct JoinScanImpl {
  std::vector<std::pair<std::size_t, std::size_t> > join_positions;

  template <class OuterTableType,
            class OuterStoreType,
            class OuterDictType,
            class InnerTableType,
            class InnerStoreType,
            class InnerDictType>
  void execute(OuterTableType* outer,
               OuterStoreType* outer_store,
               OuterDictType* outer_dict,
               InnerTableType* inner,
               InnerStoreType* inner_store,
               InnerDictType* inner_dict,
               std::size_t outer_offset,
               std::size_t inner_offset) {
    for (std::size_t outer_row {0}, outer_end {outer_store->rows()}; outer_row < outer_end; ++outer_row) 
      for (std::size_t inner_row {0}, inner_end {inner_store->rows()}; inner_row < inner_end; ++inner_row) 
        if (outer_dict->getValue(outer_store->get(outer_row)) == inner_dict->getValue(inner_store->get(inner_row))) 
          join_positions.emplace_back(outer_row + outer_offset, inner_row + inner_offset);
  }
  /*
  template <class ColumnType>
  void execute(ATable* a, AStorage* a1, AbstractDictionary* a2,
               ATable* b, AStorage* b1, AbstractDictionary* b2,
               std::size_t outer_offset, std::size_t inner_offset) {
    execute(a, a1, static_cast<BaseDictionary<ColumnType>*>(a2),
            b, b1, static_cast<BaseDictionary<ColumnType>*>(b2),
            outer_offset, inner_offset); }*/
};

dispatch< product<jtables, jstorages, jdicts<dis_int>, jtables, jstorages, jdicts<dis_int> > , 
          JoinScanImpl,
          void, std::tuple<size_t, size_t> > join_dispatch;

dispatch< product<jtables, jstorages, jdicts<dis_int>, jtables, jstorages, jdicts<dis_int> > , 
          JoinScanImpl,
          void, std::tuple<size_t, size_t> > join_dispatch2;


void JoinScan::execute() {
  JoinScanImpl impl;

  auto outer_parts = _outer->getVerticalPartitions(_outer_col);
  auto inner_parts = _inner->getVerticalPartitions(_inner_col);

  for (auto outer_part: outer_parts) {
    for (auto inner_part: inner_parts) {
      join_dispatch(impl, const_cast<ATable*>(outer_part.table),
                   const_cast<AStorage*>(outer_part.storage),
                   static_cast<BaseDictionary<dis_int>*>(outer_part.dict),
                   const_cast<ATable*>(inner_part.table),
                   const_cast<AStorage*>(inner_part.storage),
                   static_cast<BaseDictionary<dis_int>*>(inner_part.dict),
                   outer_part.offset,
                   inner_part.offset);
    }
  }
}


void JoinScan::executeFallback() {
  JoinScanImpl impl;

  auto outer_parts = _outer->getVerticalPartitions(_outer_col);
  auto inner_parts = _inner->getVerticalPartitions(_inner_col);

  for (auto outer_part: outer_parts) {
    for (auto inner_part: inner_parts) {
      impl.execute(const_cast<ATable*>(outer_part.table),
                   const_cast<AStorage*>(outer_part.storage),
                   static_cast<BaseDictionary<dis_int>*>(outer_part.dict),
                   const_cast<ATable*>(inner_part.table),
                   const_cast<AStorage*>(inner_part.storage),
                   static_cast<BaseDictionary<dis_int>*>(inner_part.dict),
                   outer_part.offset,
                   inner_part.offset);
    }
  }
}

void JoinScan::executeAbstract() {
  std::vector<std::pair<std::size_t, std::size_t> > join_positions;
  std::size_t oheight = _outer->height(), iheight = _inner->height();
  for (std::size_t outer_row {0}, outer_end {oheight}; outer_row < outer_end; ++outer_row) 
    for (std::size_t inner_row {0}, inner_end {iheight}; inner_row < inner_end; ++inner_row)
      if (_inner->getValue<dis_int>(_inner_col, inner_row) ==
          _outer->getValue<dis_int>(_outer_col, outer_row))
        join_positions.emplace_back(outer_row, inner_row);
}
