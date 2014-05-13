#include "helpers/debug.hpp"
#include "helpers/measure.h"
#include "access/Generator.h"

#include "access/ScanOperator.h"
#include  "access/MaterializingScan.h"
#include "access/JoinScan.h"
#include "storage/structural.h"


int main(int argc, char* const argv[]) {
    /*auto t = makeEqualPartitionTable(1000, 10, 1);
    t->structure(std::cout);
    t->cacheOffsets();
    std::cout << t->width() << " " << t->height() << std::endl;
    for (size_t row=0; row < t->height(); row++) {
        for (size_t col=0; col < t->width(); col++) {
            std::cout << t->getValue<dis_int>(col, row) << "\t|";
        }
        std::cout << std::endl;
    }
    exit(0);*/

    for (size_t x: {10, 100, 1000, 10000, 100000, 1000*1000, 10*1000*1000 }) {
        for (size_t p : {1, 2, 4, 8, 16, 32}) {
            std::cerr << "Making store with rows: " << x << " parts" << p << std::endl;
            if (p > x) {
                continue;
            }
            auto somestore = makeCEqualPartitionTable(x, 10, p);
            auto smallstore = makeSmallStore();
            somestore->cacheOffsets();
            smallstore->cacheOffsets();
            auto num = std::to_string(static_cast<size_t>(x));
            auto parts = std::to_string(p);
            std::random_device rd;
            std::uniform_int_distribution<int> dist(0, UPPER_VID);
            dis_int value = dist(rd);

            {
                std::string mo {"MaterializingScanOperator"};
                MaterializingScanOperator so(somestore.get(), 3);
                times_measure({mo, parts, num, "dispatch"}, [&so]() {
                        so.execute();
                    });
                times_measure({mo, parts, num, "fallback"}, [&so]() {
                        so.executeFallback();
                    });
                times_measure({mo, parts, num, "abstract"}, [&so]() {
                        so.executeAbstract();
                    });
            }
            dis_int default_value = 0;
            {
                std::string mo { "Scan DefaultValue" };
                ScanOperator so(somestore.get(), 0, default_value);
                times_measure({mo, parts, num, "dispatch"}, [&]() {
                        so.execute();
                    });
                times_measure({mo, parts, num, "fallback"}, [&]() {
                        so.executeFallback();
                    });
                times_measure({mo, parts, num, "abstract"}, [&]() {
                        so.executeAbstract();
                    });
                // times_measure({mo, parts, num, "perfect"}, [&]() {
                //         so.executePerfect();
                //     });
            }
            default_value = 1;
            {
                std::string mo { "Scan NonDefaultValue" };
                ScanOperator so(somestore.get(), 0, default_value);
                times_measure({mo, parts, num, "dispatch"}, [&]() {
                        so.execute();
                    });
                times_measure({mo, parts, num, "fallback"}, [&]() {
                        so.executeFallback();
                    });
                times_measure({mo, parts, num, "abstract"}, [&]() {
                        so.executeAbstract();
                    });
                // times_measure({mo, parts, num, "perfect"}, [&]() {
                //         so.executePerfect();
                //     });
            }
            /*

              {
              std::string mo { "Scan DefaultValueCompressed"};
              ScanOperator so(somestore.get(), 3, default_value);
              times_measure({mo, parts, num, "dispatch"}, [&]() {
              so.execute();
              });
              times_measure({mo, parts, num, "fallback"}, [&]() {
              so.executeFallback();
              });
              times_measure({mo, parts, num, "abstract"}, [&]() {
              so.executeAbstract();
              });
              }
              auto other_value = ((BaseDictionary<dis_int>*)somestore->getValueId(3, 0).dict)->getValue(DEFAULT_VID) + 1;
              {
              std::string mo { "Scan DefaultValueCompressed (non-compressed)"};
              ScanOperator so(somestore.get(), 3, other_value);
              times_measure({mo, parts, num, "dispatch"}, [&]() {
              so.execute();
              });
              times_measure({mo, parts, num, "fallback"}, [&]() {
              so.executeFallback();
              });
              times_measure({mo, parts, num, "abstract"}, [&]() {
              so.executeAbstract();
              });
              }
            */
            {
                std::string mo { "Join"};
                JoinScan so(somestore.get(), smallstore.get(), col_t(4), col_t(4));
                times_measure({mo, parts, num, "dispatch"}, [&]() {
                        so.execute();
                    });
                times_measure({mo, parts, num, "fallback"}, [&]() {
                        so.executeFallback();
                    });
                times_measure({mo, parts, num, "abstract"}, [&]() {
                        so.executeAbstract();
                    });

            }

        }}
}
