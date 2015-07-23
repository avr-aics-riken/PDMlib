###################################################################################
#
# PDMlib - Particle Data Management library
#
#
# Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN. 
# All rights reserved. 
#
###################################################################################

# - Try to find fpzip
# Once done, this will define
#
#  FPZIP_FOUND - system has FPZIP
#  FPZIP_INCLUDE_DIRS - the FPZIP include directories
#  FPZIP_LIBRARIES - link these to use FPZIP

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(FPZIP_PKGCONF FPZIP)

if(CMAKE_PREFIX_PATH)
  set(FPZIP_CANDIDATE_PATH ${CMAKE_PREFIX_PATH})
  file(GLOB tmp "${CMAKE_PREFIX_PATH}/[Ff][Pp][Zz][Ii][Pp]*/")
  list(APPEND FPZIP_CANDIDATE_PATH ${tmp})
endif()

# Include dir
find_path(FPZIP_INCLUDE_DIR
  NAMES fpzip.h
  PATHS ${FPZIP_ROOT} ${FPZIP_PKGCONF_INCLUDE_DIRS} ${FPZIP_CANDIDATE_PATH}
  PATH_SUFFIXES include
)

# Finally the library itself
find_library(FPZIP_LIBRARY
  NAMES fpzip
  PATHS ${FPZIP_ROOT} ${FPZIP_PKGCONF_LIBRARY_DIRS} ${FPZIP_CANDIDATE_PATH}
  PATH_SUFFIXES lib
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(FPZIP_INCLUDES FPZIP_INCLUDE_DIR)
set(FPZIP_LIBS FPZIP_LIBRARY)
libfind_process(FPZIP)
