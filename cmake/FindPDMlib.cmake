###################################################################################
#
# PDMlib - Particle Data Management library
#
#
# Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN. 
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

# Include dir
find_path(PDMlib_INCLUDE_DIR
  NAMES PDMlib.h
  PATHS ${TP_ROOT} ${PDMlib_PKGCONF_INCLUDE_DIRS}
  PATH_SUFFIXES include
)

# Finally the library itself
find_library(PDMlib_LIBRARY
  NAMES PDMlib
  PATHS ${TP_ROOT} ${PDMlib_PKGCONF_LIBRARY_DIRS}
  PATH_SUFFIXES lib 
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(PDMlib_INCLUDES PDMlib_INCLUDE_DIR)
set(PDMlib_LIBS PDMlib_LIBRARY)
libfind_process(PDMlib)
