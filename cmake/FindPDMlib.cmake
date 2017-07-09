###################################################################################
#
# PDMlib - Particle Data Management library
#
# Copyright (c) 2014 Advanced Institute for Computational Science(AICS), RIKEN.
# All rights reserved.
#
# Copyright (c) 2017 Research Institute for Information Technology(RIIT), Kyushu University.
# All rights reserved.
#
###################################################################################

# - Try to find PDMlib
# Once done, this will define
#
#  PDMlib_FOUND - system has PDMlib
#  PDMlib_INCLUDE_DIRS - the PDMlib include directories
#  PDMlib_LIBRARIES - link these to use PDMlib

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(PDMlib_PKGCONF PDMlib)

if(CMAKE_PREFIX_PATH)
  set(PDMlib_CANDIDATE_PATH ${CMAKE_PREFIX_PATH})
  file(GLOB tmp "${CMAKE_PREFIX_PATH}/[Pp][Dd][Mm]*/")
  list(APPEND PDMlib_CANDIDATE_PATH ${tmp})
endif()

# Include dir
find_path(PDMlib_INCLUDE_DIR
  NAMES PDMlib.h
  PATHS ${PDM_ROOT} ${PDMlib_PKGCONF_INCLUDE_DIRS} ${PDMlib_CANDIDATE_PATH}
  PATH_SUFFIXES include
)

# Finally the library itself
find_library(PDMlib_LIBRARY
  NAMES PDMlib
  PATHS ${PDM_ROOT} ${PDMlib_PKGCONF_LIBRARY_DIRS} ${PDMlib_CANDIDATE_PATH}
  PATH_SUFFIXES lib
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(PDMlib_INCLUDES PDMlib_INCLUDE_DIR)
set(PDMlib_LIBS PDMlib_LIBRARY)
libfind_process(PDMlib)
