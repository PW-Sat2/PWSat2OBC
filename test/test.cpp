#include <gtest/gtest.h>
#include <stdlib.h>

GTEST_TEST(MyTest, MyName)
{
	printf("Running my test\n");
	EXPECT_EQ(2, 4);
}

GTEST_TEST(MyTest, ThisOneWorks)
{
	printf("Running my second test\n");
	EXPECT_EQ(2, 2);
}

GTEST_TEST(MyTest, WritingFiles)
{
	char buf[] = "Hello ARM";
	FILE * f = fopen("../arm.txt", "w");
	fwrite(buf, sizeof(char), sizeof(buf), f);
	fclose(f);
}

int run(int argc, char** argv)
{
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
