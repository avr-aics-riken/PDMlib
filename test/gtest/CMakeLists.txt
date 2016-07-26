###################################################################################
#
# PDMlib - Particle Data Management library
#
#
# Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN. 
# All rights reserved. 
#
###################################################################################

if(CMAKE_CROSSCOMPILING)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DGTEST_USE_OWN_TR1_TUPLE=1" CACHE STRING "" FORCE)
endif()

include_directories("../")
message("CMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}")
add_library(gtest gtest-all.cc)
