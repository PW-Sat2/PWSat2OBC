#include <gtest/gtest.h>

#ifdef ENABLE_COVERAGE
extern "C" void __gcov_flush(void);
#endif

int run(int argc, char** argv)
{
    ::testing::GTEST_FLAG(throw_on_failure) = true;

    testing::InitGoogleTest(&argc, argv);
    auto r = RUN_ALL_TESTS();

#ifdef ENABLE_COVERAGE
    __gcov_flush();
#endif

    return r;
}
