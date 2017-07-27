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

  add_executable(WriteSample ${PROJECT_SOURCE_DIR}/example/write.cpp)
  target_link_libraries(WriteSample ${EXT_LIB_MPI})

  add_executable(RestartSample ${PROJECT_SOURCE_DIR}/example/restart.cpp)
  target_link_libraries(RestartSample ${EXT_LIB_MPI})

  add_executable(MigrateSample ${PROJECT_SOURCE_DIR}/example/migrate.cpp)
  target_link_libraries(MigrateSample ${EXT_LIB_MPI})

  add_executable(EndianConversionTest_Write ${PROJECT_SOURCE_DIR}/example/write_test.cpp)
  target_link_libraries(EndianConversionTest_Write ${EXT_LIB_MPI})

  add_executable(EndianConversionTest_Read ${PROJECT_SOURCE_DIR}/example/read_test.cpp)
  target_link_libraries(EndianConversionTest_Read ${EXT_LIB_MPI})

  add_executable(MultiInstanceWrite1 ${PROJECT_SOURCE_DIR}/example/multi_instance_write_1.cpp)
  target_link_libraries(MultiInstanceWrite1 ${EXT_LIB_MPI})

  add_executable(MultiInstanceWrite2 ${PROJECT_SOURCE_DIR}/example/multi_instance_write_2.cpp)
  target_link_libraries(MultiInstanceWrite2 ${EXT_LIB_MPI})

else()

  set(EXT_LIB "-lPDM -lTP -lzoltan -lhdf5 -lfpzip -lz") 

    add_executable(WriteSample ${PROJECT_SOURCE_DIR}/example/write.cpp)
    target_link_libraries(WriteSample ${EXT_LIB})

    add_executable(RestartSample ${PROJECT_SOURCE_DIR}/example/restart.cpp)
    target_link_libraries(RestartSample ${EXT_LIB})

    add_executable(MigrateSample ${PROJECT_SOURCE_DIR}/example/migrate.cpp)
    target_link_libraries(MigrateSample ${EXT_LIB})

    add_executable(EndianConversionTest_Write ${PROJECT_SOURCE_DIR}/example/write_test.cpp)
    target_link_libraries(EndianConversionTest_Write ${EXT_LIB})

    add_executable(EndianConversionTest_Read ${PROJECT_SOURCE_DIR}/example/read_test.cpp)
    target_link_libraries(EndianConversionTest_Read ${EXT_LIB})

    add_executable(MultiInstanceWrite1 ${PROJECT_SOURCE_DIR}/example/multi_instance_write_1.cpp)
    target_link_libraries(MultiInstanceWrite1 ${EXT_LIB})

    add_executable(MultiInstanceWrite2 ${PROJECT_SOURCE_DIR}/example/multi_instance_write_2.cpp)
    target_link_libraries(MultiInstanceWrite2 ${EXT_LIB})

endif()

