#include <cstdio>
#include <gtest/gtest.h>

#ifdef ENABLE_COVERAGE
extern "C" void __gcov_flush(void);
#endif

int run(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    auto r = RUN_ALL_TESTS();

#ifdef ENABLE_COVERAGE
    __gcov_flush();
#endif

    return r;
}
