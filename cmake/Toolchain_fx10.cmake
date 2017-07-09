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

set(CMAKE_SYSTEM_NAME Linux)

include(CMakeForceCompiler)

if(with_MPI)
  CMAKE_FORCE_C_COMPILER(mpifccpx GNU)
  CMAKE_FORCE_CXX_COMPILER(mpiFCCpx GNU)
  CMAKE_FORCE_Fortran_COMPILER(mpifrtpx GNU)
else()
  CMAKE_FORCE_C_COMPILER(fccpx GNU)
  CMAKE_FORCE_CXX_COMPILER(FCCpx GNU)
  CMAKE_FORCE_Fortran_COMPILER(frtpx GNU)
endif()

set(CMAKE_FIND_ROOT_PATH /opt/FJSVfxlang/1.2.1)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)


## Flag for cross-compiling
set(PDM_CROSS_OPTION "ON")

set(TARGET_ARCH "FX10")
set(USE_F_TCS "YES")
