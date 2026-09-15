#include <iostream>

#include <gtest/gtest.h>
#include <Titanium/RHI.hpp>

TEST(DummyTest, Test)
{
    TiRHI::RHI device;
    EXPECT_TRUE(true);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
