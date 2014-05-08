#include "helpers/debug.hpp"
#include "helpers/measure.h"
#include "access/Generator.h"

#include "access/ScanOperator.h"
#include  "access/MaterializingScan.h"
#include "access/JoinScan.h"
#include "storage/structural.h"


int main(int argc, char* const argv[]) {
    auto t = makeSomeTable();
    t->cacheOffsets();
    std::cout << t->width() << " " << t->height() << std::endl;
    for (size_t row=0; row < t->height(); row++) {
        for (size_t col=0; col < t->width(); col++) {
            std::cout << t->getValue<dis_int>(col, row) << "\t|";
        }
        std::cout << std::endl;
    }
    exit(0);
    for (double x: {10, 100, 1000, 10000, 100000, 1000*1000, 10*1000*1000 }) {
        std::cerr << "Making store with rows: " << x << std::endl;
        auto somestore = makeStore(std::ceil(x * 0.9), std::ceil(x * 0.1));
        auto smallstore = makeSmallStore();
        somestore->cacheOffsets();
        smallstore->cacheOffsets();
        auto num = std::to_string(static_cast<size_t>(x));
        std::random_device rd;
        std::uniform_int_distribution<int> dist(0, UPPER_VID);
        dis_int value = dist(rd);

        {
            std::string mo {"MaterializingScanOperator"};
            MaterializingScanOperator so(somestore.get(), 3);
            times_measure({mo, num, "dispatch"}, [&so]() {
                    so.execute();
                });
            times_measure({mo, num, "fallback"}, [&so]() {
                    so.executeFallback();
                });
            times_measure({mo, num, "abstract"}, [&so]() {
                    so.executeAbstract();
                });
        }

        {
            std::string mo { "Scan FixedLengthStorage" };
            ScanOperator so(somestore.get(), 1, value);
            times_measure({mo, num, "dispatch"}, [&]() {
                    so.execute();
                });
            times_measure({mo, num, "fallback"}, [&]() {
                    so.executeFallback();
                });
            times_measure({mo, num, "abstract"}, [&]() {
                    so.executeAbstract();
                });
            times_measure({mo, num, "perfect"}, [&]() {
                    so.executePerfect();
                });
        }

        auto default_value = ((BaseDictionary<dis_int>*)somestore->getValueId(3, 0).dict)->getValue(DEFAULT_VID);
        {
            std::string mo { "Scan DefaultValueCompressed"};
            ScanOperator so(somestore.get(), 3, default_value);
            times_measure({mo, num, "dispatch"}, [&]() {
                    so.execute();
                });
            times_measure({mo, num, "fallback"}, [&]() {
                    so.executeFallback();
                });
            times_measure({mo, num, "abstract"}, [&]() {
                    so.executeAbstract();
                });
        }
        auto other_value = ((BaseDictionary<dis_int>*)somestore->getValueId(3, 0).dict)->getValue(DEFAULT_VID) + 1;
        {
            std::string mo { "Scan DefaultValueCompressed (non-compressed)"};
            ScanOperator so(somestore.get(), 3, other_value);
            times_measure({mo, num, "dispatch"}, [&]() {
                    so.execute();
                });
            times_measure({mo, num, "fallback"}, [&]() {
                    so.executeFallback();
                });
            times_measure({mo, num, "abstract"}, [&]() {
                    so.executeAbstract();
                });
        }
        {
            std::string mo { "Join"};
            JoinScan so(somestore.get(), smallstore.get(), col_t(4), col_t(4));
            times_measure({mo, num, "dispatch"}, [&]() {
                    so.execute();
                });
            times_measure({mo, num, "fallback"}, [&]() {
                    so.executeFallback();
                });
            times_measure({mo, num, "abstract"}, [&]() {
                    so.executeAbstract();
                });

        }

    }
}
