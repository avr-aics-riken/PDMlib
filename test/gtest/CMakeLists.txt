if(CMAKE_CXX_COMPILER_ID STREQUAL GNU)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++03")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL Intel)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x")
elseif(CMAKE_CXX_COMPILER EQUAL '.*FCC.*')
    #TODO google testのオプションをなんか変える？
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++03")
endif()

include_directories("../")
add_library(gtest gtest-all.cc)
