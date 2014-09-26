# - Try to find TextParser
# Once done, this will define
#
#  TextParser_FOUND - system has TextParser
#  TextParser_INCLUDE_DIRS - the TextParser include directories
#  TextParser_LIBRARIES - link these to use TextParser

include(LibFindMacros)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(TextParser_PKGCONF TextParser)

# Include dir
find_path(TextParser_INCLUDE_DIR
  NAMES TextParser.h
  PATHS ${TP_ROOT} ${TextParser_PKGCONF_INCLUDE_DIRS}
  PATH_SUFFIXES include
)

# Finally the library itself
find_library(TextParser_LIBRARY
  NAMES TP
  PATHS ${TP_ROOT} ${TextParser_PKGCONF_LIBRARY_DIRS}
  PATH_SUFFIXES lib 
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(TextParser_INCLUDES TextParser_INCLUDE_DIR)
set(TextParser_LIBS TextParser_LIBRARY)
libfind_process(TextParser)
