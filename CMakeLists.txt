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
#
#-*- CMAKE -*-#
# Process this file with cmake to produce a configure script.

# Following parameters must be given at cmake command line

# Build options, see Readme.md for details.

# -D INSTALL_DIR=install_directory

# -D with_MPI={yes|no}

# -D with_TP={*|installed_directory}

# -D with_ZIP={*|installed_directory}

# -D with_FPZIP={*|installed_directory}

# -D with_HDF5=installed_directory

# -D with_ZOLTAN={*|installed_directory}

# -D with_util={yes|no}

# -D with_example={no|yes}

# -D real_type={float|double}

# -D CMAKE_BUILD_TYPE=DEBUG

# -D build_vtk_converter={yes|no}

# -D build_fv_converter={no|yes}

# -D build_h5part_converter={yes|no}

# -D build_tests={yes|no}



cmake_minimum_required(VERSION 2.6)


#######
# Build options
#######

option (with_util "Enable utility" "ON")
option (with_MPI "Enable MPI" "ON")
option (with_example "Compiling examples" "OFF")
# option (real_type "Type of floating point" "OFF")
option (build_vtk_converter "Build VTK converter" "ON")
option (build_fv_converter "Build FieldView converter" "OFF")
option (build_h5part_converter "Build H5Part converter" "ON")
option (build_tests "Build test programs" "ON")

#######
# Project setting
#######

project(PDMlib)

if(INSTALL_DIR STREQUAL "")
  set(INSTALL_DIR "/usr/local/PDMlib")
endif()

# project()でCMAKE_INSTALL_PREFIX=/usr/localとなるので、再定義
set (CMAKE_INSTALL_PREFIX "${INSTALL_DIR}")

#set(CMAKE_BUILD_TYPE Release)



#######
set(PROJECT_VERSION "1.2.0")
set(LIB_REVISION "20170708_2230")

message(" ")
message("PROJECT : PDMlib " ${PROJECT_VERSION} " : " ${LIB_REVISION})
message(" ")

#######



#######
# Cmake module
#######

set(CMAKE_MODULE_PATH  ${PROJECT_SOURCE_DIR}/cmake)
include(${PROJECT_SOURCE_DIR}/cmake/CompileOptionSelector.cmake)
include(${PROJECT_SOURCE_DIR}/cmake/PDM_package.cmake)



#######
# Options
#######


AddOptimizeOption()

#checkOpenMP()

# Real type
#precision()

# Serial
if(NOT with_MPI)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DWITHOUT_MPI")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DWITHOUT_MPI")
endif()


#######
# Display options
#######

message(" ")
message( STATUS "Destination PATH  : "      ${CMAKE_INSTALL_PREFIX})
message( STATUS "CMAKE_C_COMPILER  : "      ${CMAKE_C_COMPILER})
message( STATUS "CMAKE_C_FLAGS     : "      ${CMAKE_C_FLAGS})
message( STATUS "CMAKE_CXX_COMPILER: "      ${CMAKE_CXX_COMPILER})
message( STATUS "CMAKE_CXX_FLAGS   : "      ${CMAKE_CXX_FLAGS})

message(" ")
#message( STATUS "Type of floating point : "  ${real_type})
message( STATUS "Utilities              : "  ${with_util})
message( STATUS "MPI support            : "  ${with_MPI})
message( STATUS "Example                : "  ${with_example})
message( STATUS "TextParser support     : "  ${with_TP})
message( STATUS "ZIP support            : "  ${with_ZIP})
message( STATUS "FPZIP support          : "  ${with_FPZIP})
message( STATUS "HDF5 support           : "  ${with_HDF5})
message( STATUS "ZOLTAN support         : "  ${with_ZOLTAN})
message( STATUS "Build VTK converter    : "  ${build_vtk_converter})
message( STATUS "Build FV converter     : "  ${build_fv_converter})
message( STATUS "Build H5Part converter : "  ${build_h5part_converter})
message( STATUS "Build test programs    : "  ${build_tests})
message(" ")

if(USE_F_TCS STREQUAL "YES")
else()
  if(with_MPI)
    include(FindMPI)

    if(MPI_CXX_FOUND)
      message(" ")
      message("MPI-CXX-CMP  : ${MPI_CXX_COMPILER}")
      message("MPI-CXX-FLAGS: ${MPI_CXX_COMPILE_FLAGS}")
      message("MPI-CXX-INC  : ${MPI_CXX_INCLUDE_PATH}")
      message("MPI-LNK-FLAGS: ${MPI_CXX_LINK_FLAGS}")
      message("MPI-LIBS     : ${MPI_CXX_LIBRARIES}")
      message(" ")
      include_directories( ${MPI_CXX_INCLUDE_PATH} )
    endif(MPI_CXX_FOUND)
  endif()
endif()


#######
# include PATH
#######

include_directories(
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/include
)


#######
# Find Libraries to depend on
#######

# Use Find*.cmake
#find_package (ZLIB       REQUIRED)
#find_package (HDF5)
#find_package (TextParser REQUIRED)
#find_package (FPZIP      REQUIRED)
#find_package (Zoltan     REQUIRED)


# TextParser
IF(with_TP)
  SET(TP_DIR "${with_TP}")
  SET(TP_INC "${TP_DIR}/include")
  SET(TP_LIB "${TP_DIR}/lib")
ELSE()
  MESSAGE("Error: can not find TextParser.")
ENDIF()


# ZIP
IF(with_ZIP)
  SET(ZIP_DIR "${with_ZIP}")
  SET(ZIP_INC "${ZIP_DIR}/include")
  SET(ZIP_LIB "${ZIP_DIR}/lib")
ELSE()
  MESSAGE("Error: can not find ZIP library.")
ENDIF()


# FPZIP
IF(with_FPZIP)
  SET(FPZIP_DIR "${with_FPZIP}")
  SET(FPZIP_INC "${FPZIP_DIR}/include")
  SET(FPZIP_LIB "${FPZIP_DIR}/lib")
ELSE()
  MESSAGE("Error: can not find ZIP library.")
ENDIF()


# HDF5
IF(with_HDF5)
  SET(HDF5_DIR "${with_HDF5}")
  SET(HDF5_INC "${HDF5_DIR}/include")
  SET(HDF5_LIB "${HDF5_DIR}/lib")
ELSE()
  MESSAGE("Error: can not find HDF5 library.")
ENDIF()


# ZOLTAN
IF(with_ZOLTAN)
  SET(ZOLTAN_DIR "${with_ZOLTAN}")
  SET(ZOLTAN_INC "${ZOLTAN_DIR}/include")
  SET(ZOLTAN_LIB "${ZOLTAN_DIR}/lib")
ELSE()
  MESSAGE("Error: can not find ZOLTAN.")
ENDIF()


#add_definitions(-DHAVE_CONFIG_H)



#######
# tools
#######

if(with_util)
  if(PDM_CROSS_OPTION)
    message(" ")
    message("PDMlib tools : not build because cross compilation is assumed")
  else()
    message("PDMlib tools : build")
  endif()
endif()



#######
# Example
#######

if (with_example)
  message("Exampels are built")
else()
  message("Skip compiling examples")
endif()


#######
# Test code
#######

if (with_tests)
  enable_testing()
  message("Code test will be performed")
else()
  message("Code test is skipped")
endif()




#######
# Sub Directories
#######

add_subdirectory(src)

if(with_util)
  if(NOT PDM_CROSS_OPTION)
    add_subdirectory(tools)
  endif()
endif()

add_subdirectory(doc)

if(with_example)
  if (NOT with_MPI)
    message("Examples require MPI")
  else()
    add_subdirectory(example)
  endif()
endif()

if (with_tests)
  add_subdirectory(test)
endif()


#######
# configure files
#######

#configure_file( config.h.in.cmake config.h )
configure_file( include/pdm_version.h.in ${PROJECT_BINARY_DIR}/include/pdm_version.h @ONLY)


#######
# Install files
#######

install(FILES  ChangeLog.md
               Readme.md
               License.txt
               ${PROJECT_BINARY_DIR}/CMakeCache.txt
        DESTINATION share)


#######
# Installer
#######

pdm_package()
