###################################################################################
#
# PDMlib - Particle Data Management library
#
#
# Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN. 
# All rights reserved. 
#
###################################################################################

# - Try to find Zoltan
# Once done, this will define
#
#  Zoltan_FOUND - system has Zoltan
#  Zoltan_INCLUDE_DIRS - the Zoltan include directories
#  Zoltan_LIBRARIES - link these to use Zoltan

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(Zoltan_PKGCONF Zoltan)

if(CMAKE_PREFIX_PATH)
  set(Zoltan_CANDIDATE_PATH ${CMAKE_PREFIX_PATH})
  file(GLOB tmp "${CMAKE_PREFIX_PATH}/[Zz][Oo][Ll][Tt][Aa][Nn]*/")
  list(APPEND Zoltan_CANDIDATE_PATH ${tmp})
endif()

# Include dir
find_path(Zoltan_INCLUDE_DIR
  NAMES zoltan_cpp.h
  PATHS ${ZOLTAN_ROOT} ${Zoltan_PKGCONF_INCLUDE_DIRS} ${Zoltan_CANDIDATE_PATH}
  PATH_SUFFIXES include
)

# Finally the library itself
find_library(Zoltan_LIBRARY
  NAMES zoltan
  PATHS ${ZOLTAN_ROOT} ${Zoltan_PKGCONF_LIBRARY_DIRS} ${Zoltan_CANDIDATE_PATH}
  PATH_SUFFIXES lib 
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Zoltan_INCLUDES Zoltan_INCLUDE_DIR)
set(Zoltan_LIBS Zoltan_LIBRARY)
libfind_process(Zoltan)
