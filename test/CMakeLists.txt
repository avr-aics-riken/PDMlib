###################################################################################
#
# PDMlib - Particle Data Management library
#
# Copyright (c) 2012-2015 Institute of Industrial Science, The University of Tokyo.
# All rights reserved.
#
# Copyright (c) 2015-2017 Advanced Institute for Computational Science, RIKEN.
# All rights reserved.
#
# Copyright (c) 2017 Research Institute for Information Technology (RIIT), Kyushu University.
# All rights reserved.
#
###################################################################################

add_subdirectory(${PROJECT_SOURCE_DIR}/test/gtest)

include_directories(
       ${PROJECT_BINARY_DIR}/include/
       ${PROJECT_SOURCE_DIR}
       ${TP_INC}
       ${ZIP_INC}
       ${FPZIP_INC}
       ${HDF5_INC}
       ${ZOLTAN_INC}
)


link_directories(
      ${PROJECT_BINARY_DIR}/src/
      ${ZOLTAN_LIB}
      ${HDF5_LIB}
      ${FPZIP_LIB}
      ${ZIP_LIB}
      ${TP_LIB}
)

if(with_MPI)

  set(EXT_LIB_MPI "-lPDMmpi -lTPmpi -lzoltan -lhdf5 -lfpzip -lz ${MPI_CXX_LIBRARIES}") 

  add_executable(UnitTest 
    ${PROJECT_SOURCE_DIR}/test/src/gtest_main.cc
    ${PROJECT_SOURCE_DIR}/test/src/WriteFileTest.cpp
    ${PROJECT_SOURCE_DIR}/test/src/ReadFileTest.cpp
    ${PROJECT_SOURCE_DIR}/test/src/EncodeDecodeTest.cpp
    ${PROJECT_SOURCE_DIR}/test/src/MetaDataTest.cpp
    ${PROJECT_SOURCE_DIR}/test/src/UtilityTest.cpp
   )
  target_link_libraries(UnitTest ${EXT_LIB_MPI} gtest)

  add_executable(MigrationTest  ${PROJECT_SOURCE_DIR}/test/src/MigrationTest.cpp)
  target_link_libraries(MigrationTest  ${EXT_LIB_MPI} gtest)

else()

  set(EXT_LIB "-lPDM -lTP -lzoltan -lhdf5 -lfpzip -lz") 

  add_executable(UnitTest 
    ${PROJECT_SOURCE_DIR}/test/src/gtest_main.cc
    ${PROJECT_SOURCE_DIR}/test/src/WriteFileTest.cpp
    ${PROJECT_SOURCE_DIR}/test/src/ReadFileTest.cpp
    ${PROJECT_SOURCE_DIR}/test/src/EncodeDecodeTest.cpp
    ${PROJECT_SOURCE_DIR}/test/src/MetaDataTest.cpp
    ${PROJECT_SOURCE_DIR}/test/src/UtilityTest.cpp
   )
  target_link_libraries(UnitTest ${EXT_LIB} gtest)

endif()

