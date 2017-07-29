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

  if (build_vtk_converter)
    add_executable(VtkConverter VtkConverter.C)
    target_link_libraries(VtkConverter ${EXT_LIB_MPI})
    install(TARGETS VtkConverter DESTINATION bin)
  endif()
  if (build_h5part_converter)
    add_executable(H5PartConverter H5PartConverter.C)
    target_link_libraries(H5PartConverter ${EXT_LIB_MPI})
    install(TARGETS H5PartConverter DESTINATION bin)
  endif()
  if (build_fv_converter)
    add_executable(FV14Converter FV14Converter.C)
    target_link_libraries(FV14Converter ${EXT_LIB_MPI})
    install(TARGETS FV14Converter DESTINATION bin)
  endif()

else()

  set(EXT_LIB "-lPDM -lTP -lzoltan -lhdf5 -lfpzip -lz") 

  if (build_vtk_converter)
    add_executable(VtkConverter VtkConverter.C)
    target_link_libraries(VtkConverter ${EXT_LIB})
    install(TARGETS VtkConverter DESTINATION bin)
  endif()
  if (build_h5part_converter)
    add_executable(H5PartConverter H5PartConverter.C)
    target_link_libraries(H5PartConverter ${EXT_LIB})
    install(TARGETS H5PartConverter DESTINATION bin)
  endif()
  if (build_fv_converter)
    add_executable(FV14Converter FV14Converter.C)
    target_link_libraries(FV14Converter ${EXT_LIB})
    install(TARGETS FV14Converter DESTINATION bin)
  endif()

endif()

