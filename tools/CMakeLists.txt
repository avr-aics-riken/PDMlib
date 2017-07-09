###################################################################################
#
# UDMlib - Unstructured Data Management library
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

# udm_version.hはPROJECT_BINARY_DIR
include_directories(
       ${PROJECT_BINARY_DIR}/include
       ${PROJECT_SOURCE_DIR}/include
       ${PROJECT_SOURCE_DIR}/tools/udm-frm/include
       ${TP_INC}
       ${CGNS_INC}
       ${HDF5_INC}
       ${ZOLTAN_INC}
)


link_directories(
      ${PROJECT_BINARY_DIR}/src
      ${ZOLTAN_LIB}
      ${CGNS_LIB}
      ${HDF5_LIB}
      ${TP_LIB}
)


set(frm_files
    src/udm_frm.cpp
    src/UdmStaging.cpp
)


if(with_MPI)
  add_executable(frm ${frm_files})
  target_link_libraries(frm -lUDMmpi -lTPmpi -lzoltan -lcgns -lhdf5)
  install(TARGETS frm DESTINATION bin)
else()
  add_executable(frm ${frm_files})
  target_link_libraries(frm -lUDM -lTP -lzoltan -lcgns -lhdf5)
  install(TARGETS frm DESTINATION bin)
endif()
