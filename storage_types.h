#pragma once

#include "dispatch_fwd.h"
#include <boost/mpl/vector.hpp>
#include <cstdint>

using table_types = boost::mpl::vector<RawTable, Table>;

using storage_types = boost::mpl::vector<FixedStorage, BitStorage<2>, DefaultValueCompressedStorage>;

using dictionary_types =
    boost::mpl::vector<OrderedDictionary<dis_float>, OrderedDictionary<dis_string>, UnorderedDictionary<dis_float>,
                       UnorderedDictionary<dis_string>, OrderedDictionary<dis_int>, UnorderedDictionary<dis_int> >;
