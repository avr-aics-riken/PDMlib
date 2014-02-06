#include "ReadFileTest.h"

INSTANTIATE_TEST_CASE_P(AllTest, ReadFileTestWithIntData,
                        ::testing::Combine(
                            ::testing::Range(10, 20, 11),
                            ::testing::Values("sequential", "random", "same")
                            )
                        );
INSTANTIATE_TEST_CASE_P(AllTest, ReadFileTestWithLongData,
                        ::testing::Combine(
                            ::testing::Range(10, 20, 11),
                            ::testing::Values("sequential", "random", "same")
                            )
                        );
INSTANTIATE_TEST_CASE_P(AllTest, ReadFileTestWithFloatData,
                        ::testing::Combine(
                            ::testing::Range(10, 20, 11),
                            ::testing::Values("sequential", "random", "same")
                            )
                        );
INSTANTIATE_TEST_CASE_P(AllTest, ReadFileTestWithDoubleData,
                        ::testing::Combine(
                            ::testing::Range(10, 20, 11),
                            ::testing::Values("sequential", "random", "same")
                            )
                        );