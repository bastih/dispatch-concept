#include "JoinScan.h"

#include "dispatch/Operator.h"
#include "storage/alltypes.h"

using jtables = std::tuple<Table, RawTable>;

using jstorages = std::tuple<FixedStorage, BitStorage<2> >;

template <typename ValueType>
using jdicts = std::tuple<OrderedDictionary<ValueType>,
                          UnorderedDictionary<ValueType> >;

template <typename ValueType>
using join_types = std::tuple<jtables, jstorages, jdicts<ValueType>,
                              jtables, jstorages, jdicts<ValueType> >;

template <typename ColumnType>
struct JoinScanImpl : public OperatorNew<JoinScanImpl<ColumnType>,
                                         join_types<ColumnType> > {
  std::vector<std::pair<std::size_t, std::size_t> > join_positions;

  template <class OuterTableType,
            class OuterStoreType,
            template <typename> class OuterDictType,
            class InnerTableType,
            class InnerStoreType,
            template <typename> class InnerDictType>
  void execute_special(OuterTableType* outer,
                       OuterStoreType* outer_store,
                       OuterDictType<ColumnType>* outer_dict,
                       InnerTableType* inner,
                       InnerStoreType* inner_store,
                       InnerDictType<ColumnType>* inner_dict,
                       std::size_t outer_offset,
                       std::size_t inner_offset) {
    for (std::size_t outer_row {0}, outer_end {outer_store->rows()}; outer_row < outer_end; ++outer_row) 
      for (std::size_t inner_row {0}, inner_end {inner_store->rows()}; inner_row < inner_end; ++inner_row) 
        if (outer_dict->getValue(outer_store->get(outer_row)) == inner_dict->getValue(inner_store->get(inner_row))) 
          join_positions.emplace_back(outer_row + outer_offset, inner_row + inner_offset);
  }

  void execute_special(ATable* a, AStorage* a1, ADictionary* a2,
                       ATable* b, AStorage* b1, ADictionary* b2,
                       std::size_t outer_offset, std::size_t inner_offset) {
    execute_special(a, a1, static_cast<BaseDictionary<ColumnType>*>(a2),
                    b, b1, static_cast<BaseDictionary<ColumnType>*>(b2),
                    outer_offset, inner_offset); }
};

void JoinScan::execute() {
  JoinScanImpl<dis_int> impl;

  auto outer_parts = _outer->getVerticalPartitions(_outer_col);
  auto inner_parts = _inner->getVerticalPartitions(_inner_col);

  for (auto outer_part: outer_parts) {
    for (auto inner_part: inner_parts) {
      impl.execute(const_cast<ATable*>(outer_part.table),
                   const_cast<AStorage*>(outer_part.storage),
                   const_cast<ADictionary*>(outer_part.dict),
                   const_cast<ATable*>(inner_part.table),
                   const_cast<AStorage*>(inner_part.storage),
                   const_cast<ADictionary*>(inner_part.dict),
                   outer_part.offset,
                   inner_part.offset);
    }
  }
}


void JoinScan::executeFallback() {
  JoinScanImpl<dis_int> impl;

  auto outer_parts = _outer->getVerticalPartitions(_outer_col);
  auto inner_parts = _inner->getVerticalPartitions(_inner_col);

  for (auto outer_part: outer_parts) {
    for (auto inner_part: inner_parts) {
      impl.execute_special(const_cast<ATable*>(outer_part.table),
                           const_cast<AStorage*>(outer_part.storage),
                           const_cast<ADictionary*>(outer_part.dict),
                           const_cast<ATable*>(inner_part.table),
                           const_cast<AStorage*>(inner_part.storage),
                           const_cast<ADictionary*>(inner_part.dict),
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
