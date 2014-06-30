cmake_minimum_required(VERSION 2.6)

project(PDMlib CXX C)

#コンパイラおよびコンパイルオプションの指定
set(CMAKE_CXX_COMPILER "mpicxx")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")

if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++03 -pthread")
endif()

#コンパイラ設定の表示
MESSAGE( STATUS "CMAKE_CXX_COMPILER: " ${CMAKE_CXX_COMPILER} )
MESSAGE( STATUS "CMAKE_CXX_FLAGS: " ${CMAKE_CXX_FLAGS} )

#
# コンパイル対象ファイルの指定
#

#ライブラリ本体
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
set(FV14CONVERTER_SRC ${PROJECT_SOURCE_DIR}/tools/FV14Converter.C)

#googletest
set (GTEST_SRC
     ${PROJECT_SOURCE_DIR}/test/gtest/gtest-all.cc 
     ${PROJECT_SOURCE_DIR}/test/gtest/gtest_main.cc
    )
#テストコード
set(TEST_SRC_DIR ${PROJECT_SOURCE_DIR}/test/src)
set(UNIT_TEST_SRC 
    ${GTEST_SRC}
    ${TEST_SRC_DIR}/WriteFileTest.cpp
    ${TEST_SRC_DIR}/ReadFileTest.cpp
    ${TEST_SRC_DIR}/EncodeDecodeTest.cpp
    ${TEST_SRC_DIR}/MetaDataTest.cpp
   )
set(MIGRATION_TEST_SRC  ${TEST_SRC_DIR}/MigrationTest.cpp)

#サンプル
set(WRITE_SAMPLE_SRC   ${PROJECT_SOURCE_DIR}/example/write.cpp)
set(RESTART_SAMPLE_SRC ${PROJECT_SOURCE_DIR}/example/restart.cpp)
set(MIGRATE_SAMPLE_SRC ${PROJECT_SOURCE_DIR}/example/migrate.cpp)



#
# 依存するパッケージを探す
#
find_package (ZLIB REQUIRED)

#
# include PATHの設定(ライブラリ用)
#
include_directories(
                    ${PROJECT_SOURCE_DIR}/include
                    ${LIB_SRC_DIR}
                    $ENV{HOME}/WORK/TextParser/include
                    $ENV{HOME}/WORK/PDM/fpzip-1.0.1/inc
                    $ENV{HOME}/WORK/PDM/Zoltan_v3.8/src/include
                    $ENV{HOME}/WORK/PDM/Zoltan_v3.8/BUILD/src/include
                    ${ZLIB_INCLUDE_DIRS}
                   )


#
# ライブラリのビルド
#
add_library(PDM STATIC ${LIB_SRC})


#
# include PATHの設定(テスト＆サンプル用)
#
include_directories(
                    ${PROJECT_SOURCE_DIR}/include
                    ${PROJECT_SOURCE_DIR}/test
                    ${TEST_SRC_DIR}
                    )
#
# 単体テスト、サンプルのビルド
#
# link_directories, add_executable, target_link_librariesは
# この順に呼ばないと有効にならないので注意
#

# libraly PATHの設定
link_directories($ENV{HOME}/WORK/TextParser/src)
link_directories($ENV{HOME}/WORK/PDM/fpzip-1.0.1/lib)
link_directories($ENV{HOME}/WORK/PDM/Zoltan_v3.8/BUILD/src)

# ビルド対象の指定
add_executable(UnitTest       ${UNIT_TEST_SRC})
add_executable(WriteSample    ${WRITE_SAMPLE_SRC})
add_executable(RestartSample  ${RESTART_SAMPLE_SRC})
add_executable(MigrateSample  ${MIGRATE_SAMPLE_SRC})
add_executable(MigrationTest  ${MIGRATION_TEST_SRC})
add_executable(FV14Converter  ${FV14CONVERTER_SRC})

#リンクするライブラリの指定
target_link_libraries(UnitTest       PDM TP ${ZLIB_LIBRARIES} fpzip zoltan)
target_link_libraries(WriteSample    PDM TP ${ZLIB_LIBRARIES} fpzip zoltan)
target_link_libraries(RestartSample  PDM TP ${ZLIB_LIBRARIES} fpzip zoltan)
target_link_libraries(MigrateSample  PDM TP ${ZLIB_LIBRARIES} fpzip zoltan)
target_link_libraries(MigrationTest  PDM TP ${ZLIB_LIBRARIES} fpzip zoltan)
target_link_libraries(FV14Converter  PDM TP ${ZLIB_LIBRARIES} fpzip zoltan)

# install先の指定
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR}/../)
install (TARGETS PDM      DESTINATION lib)
install (FILES   ${PROJECT_SOURCE_DIR}/include/PDMlib.h   DESTINATION include)

install (TARGETS UnitTest       DESTINATION bin)
install (TARGETS WriteSample    DESTINATION bin)
install (TARGETS RestartSample  DESTINATION bin)
install (TARGETS MigrateSample  DESTINATION bin)
install (TARGETS MigrationTest  DESTINATION bin)
install (TARGETS FV14Converter  DESTINATION bin)
