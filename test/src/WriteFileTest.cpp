/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */

#include "WriteFileTest.h"

INSTANTIATE_TEST_CASE_P(AllTest, WriteFileTestWithIntData,
                        ::testing::Combine(
                            ::testing::Range(10, 20, 11),
                            ::testing::Values("sequential", "random", "same"),
                            ::testing::Bool()
                            )
                        );
INSTANTIATE_TEST_CASE_P(AllTest, WriteFileTestWithLongData,
                        ::testing::Combine(
                            ::testing::Range(10, 20, 11),
                            ::testing::Values("sequential", "random", "same"),
                            ::testing::Bool()
                            )
                        );
INSTANTIATE_TEST_CASE_P(AllTest, WriteFileTestWithFloatData,
                        ::testing::Combine(
                            ::testing::Range(10, 20, 11),
                            ::testing::Values("sequential", "random", "same"),
                            ::testing::Bool()
                            )
                        );
INSTANTIATE_TEST_CASE_P(AllTest, WriteFileTestWithDoubleData,
                        ::testing::Combine(
                            ::testing::Range(10, 20, 11),
                            ::testing::Values("sequential", "random", "same"),
                            ::testing::Bool()
                            )
                        );