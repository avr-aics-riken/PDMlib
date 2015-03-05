###################################################################################
#
# PDMlib - Particle Data Management library
#
#
# Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN. 
# All rights reserved. 
#
###################################################################################

cmake_minimum_required(VERSION 2.6)

project(PDMlib CXX)

set(CMAKE_MODULE_PATH  ${PROJECT_SOURCE_DIR}/cmake)

#デフォルトのコンパイラおよびコンパイルオプションの指定
#set(CMAKE_CXX_COMPILER "mpicxx")
#set(CMAKE_CXX_FLAGS "-g")

option(build_samples "build sample program for PDMlib" OFF)
option(build_tests "build test program for PDMlib" OFF)

#ビルド設定の表示
message( STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER})
message( STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS})
message( STATUS "Destination PATH: " ${CMAKE_INSTALL_PREFIX})
message( STATUS "build sample program: " ${build_samples})
message( STATUS "build test program: " ${build_tests})


##############################################
#      コンパイル対象ファイルの指定
##############################################
#PDMlib本体
set(LIB_SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(LIB_SRC
    ${LIB_SRC_DIR}/Read.C
    ${LIB_SRC_DIR}/Write.C
    ${LIB_SRC_DIR}/WriteFactory.C
    ${LIB_SRC_DIR}/ReadFactory.C
    ${LIB_SRC_DIR}/PDMlib.C
    ${LIB_SRC_DIR}/MetaData.C
    ${LIB_SRC_DIR}/Utility.C
    )

# コンバータ
set(FV14CONVERTER_SRC   ${PROJECT_SOURCE_DIR}/tools/FV14Converter.C)
set(H5PartCONVERTER_SRC ${PROJECT_SOURCE_DIR}/tools/H5PartConverter.C)

#テストコード
set(TEST_SRC_DIR ${PROJECT_SOURCE_DIR}/test/src)
set(UNIT_TEST_SRC 
    ${TEST_SRC_DIR}/gtest_main.cc
    ${TEST_SRC_DIR}/WriteFileTest.cpp
    ${TEST_SRC_DIR}/ReadFileTest.cpp
    ${TEST_SRC_DIR}/EncodeDecodeTest.cpp
    ${TEST_SRC_DIR}/MetaDataTest.cpp
    ${TEST_SRC_DIR}/UtilityTest.cpp
   )
set(MIGRATION_TEST_SRC  ${TEST_SRC_DIR}/MigrationTest.cpp)

#サンプル
set(WRITE_SAMPLE_SRC   ${PROJECT_SOURCE_DIR}/example/write.cpp)
set(RESTART_SAMPLE_SRC ${PROJECT_SOURCE_DIR}/example/restart.cpp)
set(MIGRATE_SAMPLE_SRC ${PROJECT_SOURCE_DIR}/example/migrate.cpp)



##############################################
# 依存するパッケージを探す
##############################################
find_package (ZLIB REQUIRED)
find_package (HDF5)
if ( NOT HDF5_FOUND )
    message("libhdf5 not found. H5PartConverter will not be build!")
endif ( NOT HDF5_FOUND )
find_package (TextParser REQUIRED)
find_package (FPZIP      REQUIRED)
find_package (Zoltan     REQUIRED)

##############################################
# include PATHの設定(ライブラリ用)
##############################################
set (INCLUDE_DIRS   ${PROJECT_SOURCE_DIR}/include
                    ${LIB_SRC_DIR}
                    ${ZLIB_INCLUDE_DIRS}
                    ${FPZIP_INCLUDE_DIRS}
                    ${Zoltan_INCLUDE_DIRS}
                    ${TextParser_INCLUDE_DIRS}
                   )
include_directories(${INCLUDE_DIRS})
if (HDF5_FOUND )
include_directories(${HDF5_INCLUDE_DIRS})
endif(HDF5_FOUND)


##############################################
# ライブラリ本体のビルド&インストール
##############################################
add_library(PDM STATIC ${LIB_SRC})
install (TARGETS PDM DESTINATION lib)
install (FILES ${PROJECT_SOURCE_DIR}/include/PDMlib.h         DESTINATION include)
install (FILES ${PROJECT_SOURCE_DIR}/doc/PDMlibUserManual.pdf DESTINATION doc)
install (FILES ${PROJECT_SOURCE_DIR}/LICENSE                  DESTINATION share)
install (FILES ${PROJECT_SOURCE_DIR}/README.md                DESTINATION share)
install (FILES ${PROJECT_SOURCE_DIR}/cmake/FindPDMlib.cmake   DESTINATION share)


##############################################
# ツール類のビルド&インストール
##############################################

install (PROGRAMS ${PROJECT_SOURCE_DIR}/tools/stage.py DESTINATION bin)

set (MANDATORY_LIBS PDM ${TextParser_LIBRARIES} ${ZLIB_LIBRARIES} ${FPZIP_LIBRARIES} ${Zoltan_LIBRARIES})

# FV14用コンバータ
add_executable(FV14Converter    ${FV14CONVERTER_SRC})
target_link_libraries(FV14Converter   ${MANDATORY_LIBS})
install (TARGETS  FV14Converter    DESTINATION bin)

# HDF5用コンバータ
if (HDF5_FOUND )
add_executable(H5PartConverter  ${H5PartCONVERTER_SRC})
target_link_libraries(H5PartConverter ${MANDATORY_LIBS} ${HDF5_LIBRARIES})
install (TARGETS  H5PartConverter  DESTINATION bin)
endif(HDF5_FOUND)


##############################################
# サンプルのビルド
#   cmakeの実行時に -Dbuild_samples=yes が指定された時のみbuildする
##############################################
if(build_samples)
# include PATHの設定(テスト用)
include_directories(${TEST_SRC_DIR})

# ビルド対象の指定
add_executable(WriteSample    ${WRITE_SAMPLE_SRC})
add_executable(RestartSample  ${RESTART_SAMPLE_SRC})
add_executable(MigrateSample  ${MIGRATE_SAMPLE_SRC})

#リンクするライブラリの指定
target_link_libraries(WriteSample    ${MANDATORY_LIBS})
target_link_libraries(RestartSample  ${MANDATORY_LIBS})
target_link_libraries(MigrateSample  ${MANDATORY_LIBS})

# インストール
install (PROGRAMS ${PROJECT_SOURCE_DIR}/test/migration_test.sh DESTINATION bin)
install (TARGETS WriteSample RestartSample MigrateSample DESTINATION bin)
endif()

##############################################
# テストプログラムのビルド&インストール
#   cmakeの実行時に -Dbuild_tests=yes が指定された時のみbuildする
##############################################
if(build_tests)
#GoogleTest libraryのBUILD
add_subdirectory(${PROJECT_SOURCE_DIR}/test/gtest)

# include PATHの設定(テスト用)
include_directories(
                    ${PROJECT_SOURCE_DIR}/test 
                    ${TEST_SRC_DIR}
                   )

# ビルド対象の指定
add_executable(UnitTest       ${UNIT_TEST_SRC})
add_executable(MigrationTest  ${MIGRATION_TEST_SRC})

#リンクするライブラリの指定
target_link_libraries(UnitTest       ${MANDATORY_LIBS} gtest)
target_link_libraries(MigrationTest  ${MANDATORY_LIBS})

# インストール
install (TARGETS UnitTest DESTINATION bin)
endif()
