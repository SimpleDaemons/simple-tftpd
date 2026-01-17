#include <gtest/gtest.h>

// Basic test to ensure the test framework works
TEST(BasicTest, TestFramework) {
    EXPECT_EQ(1, 1);
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
}

// Main function for Google Test
int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
