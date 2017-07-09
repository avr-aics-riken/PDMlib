###################################################################################
#
# PDMlib - Particle Data Management library
#
# Copyright (c) 2014-2017 Advanced Institute for Computational Science(AICS), RIKEN.
# All rights reserved.
#
# Copyright (c) 2017 Research Institute for Information Technology (RIIT), Kyushu University.
# All rights reserved.
#
###################################################################################

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
    ${PROJECT_BINARY_DIR}/include           # pdm_version.h
    ${TP_INC}
    ${ZIP_INC}
    ${FPZIP_INC}
    ${HDF5_INC}
    ${ZOLTAN_INC}
)


link_directories(
      ${TP_LIB}
      ${ZIP_LIB}
      ${FPZIP_LIB}
      ${HDF5_LIB}
      ${ZOLTAN_LIB}
)


set(pdm_files
    MetaData.C
    PDMlib.C
    Read.C
    ReadFactory.C
    Utility.C
    Write.C
    WriteFactory.C
)


if(NOT with_MPI)
  set(pdm_target PDM)
  set(pdm_libs "-lTP -lz -lfpzip -lhdf5 -lzoltan")
else()
  set(pdm_target PDMmpi)
  set(pdm_libs "-lTPmpi -lz -lfpzip -lhdf5 -lzoltan")
endif()


add_library(${pdm_target} STATIC ${pdm_files})
target_link_libraries(${pdm_target} ${pdm_libs})
install(TARGETS ${pdm_target} DESTINATION lib)


install(FILES
        ${PROJECT_SOURCE_DIR}/include/PDMlib.h
        ${PROJECT_BINARY_DIR}/include/pdm_version.h
        DESTINATION include
)
