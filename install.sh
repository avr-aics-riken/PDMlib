#!/bin/sh
# -*- coding: utf-8 -*-
##############################################################################
#
# PDMlib Install Script
#
# Copyright (c) 2015 Advanced Institute for Computational Science, RIKEN.
# All rights reserved.
#

FPZIP=fpzip-1.0.1
FPZIP_ARCHIVE=${FPZIP}.tar.gz
ZOLTAN=Zoltan_v3.82
ZOLTAN_ARCHIVE=zoltan_distrib_v3.82.tar.gz

#error code definision
ILLEGAL_OPTION=1
MISSING_INSTALL_PATH=2
FPZIP_NOT_FOUND=3
ZOLTAN_NOT_FOUND=4
FPZIP_BUILD_FAILED=5
ZOLTAN_BUILD_FAILED=6
PDMLIB_BUILD_FAILED=7

# argument parse
INSTALL_TO=""  
COMPILER=""
MPI=""
BUILD_ENV=""
MPI_ENV=""

while getopts "c:m:p:" flag; do
  case $flag in
    c) COMPILER="$OPTARG";;
    m) MPI="$OPTARG";;
    p) INSTALL_TO="$OPTARG";;
    \?) 
    echo "usage: $0 -gif [-I] -p INSTALL_TO"
    echo "optios:"
    echo " -c [GNU|Intel|FX] specify compiler"
    echo " -m [OpenMPI | IntelMPI | mpich ]  specify MPI lib"
    echo " -p INSTALL_TO specify install directory"
    exit ${ILLEGAL_OPTION}
  esac
done

if [ x${INSTALL_TO} == x ];then
  echo "install path (-p) must be specified!"
  exit ${MISSING_INSTALL_PATH}
fi

#指定されたコンパイラ名を確認
for comp in GNU INTEL FX10;
do
  echo ${COMPILER} |grep -i ${comp} 1>/dev/null 2>&1 
  if [ $? -eq 0 ];then
    BUILD_ENV=${comp}
  fi
done

#指定されたMPIライブラリがIntelMPIかどうかを判定
#MPIはIntelMPIの時以外は挙動を変える必要が無い
echo ${MPI} |grep -i IntelMPI 1>/dev/null 2>&1 
if [ $? -eq 0 ];then
  MPI_ENV="IMPI"
fi


#デフォルト設定
MPICC=mpicc
MPICXX=mpicxx
MPIFC=mpif90

if [ x${BUILD_ENV} == "xGNU" ];then
  CC=gcc
  CXX=g++
  FC=gfortran
elif [ x${BUILD_ENV} == "xINTEL" ];then
  CC=icc
  CXX=icpc
  FC=ifort
  if [ x${MPI_ENV} == "xIMPI" ];then
    MPICC=mpiicc
    MPICXX=mpiicpc
    MPIFC=mpiifort
  fi
elif [ x${BUILD_ENV} == "xFX10" ];then
  CC=fccpx
  CXX=FCCpx
  FC=frtpx
  MPICC=mpifccpx
  MPICXX=mpiFCCpx
  MPIFC=mpifrtpx
fi


#install fpzip
if [ ! -f ${FPZIP_ARCHIVE} ]; then
  echo "fpzip archive is not found in this directory"
  echo "please download it from http://computation.llnl.gov/casc/fpzip/"
  echo "note: ver. 1.1 is not supported for now. please use ver. 1.0.1."
  exit ${FPZIP_NOT_FOUND}
else
  if [ ! -d ${FPZIP} ]; then
    tar xfz ${FPZIP_ARCHIVE}
  fi
fi

pushd  ${FPZIP}/src
make CC=${MPICC} CXX=${MPICXX} 
if [ $? -ne 0 ];then
  exit ${FIPZIP_BUILD_FAILED}
fi
if [ ! -d ${INSTALL_TO}/fpzip/include ]; then
  mkdir -p ${INSTALL_TO}/fpzip/include 
fi
cp ../inc/fpzip.h ${INSTALL_TO}/fpzip/include

if [ ! -d ${INSTALL_TO}/fpzip/lib ]; then
  mkdir -p ${INSTALL_TO}/fpzip/lib
fi
cp ../lib/libfpzip.a ${INSTALL_TO}/fpzip/lib
popd

#install Zoltan
if [ ! -f ${ZOLTAN_ARCHIVE} ]; then
  echo "Zoltan archive is not found in this directory"
  echo "please download it from http://www.cs.sandia.gov/web1400/1400_download.html"
  exit ${ZOLTAN_NOT_FOUND}
else
  if [ ! -d ${ZOLTAN} ]; then
    tar xfz ${ZOLTAN_ARCHIVE} 2>/dev/null
  fi
fi

pushd ${ZOLTAN}
if [ ! -d BUILD ];then
  mkdir BUILD
fi
cd BUILD
../configure --with-id-type=ulong --prefix=${INSTALL_TO}/Zoltan CC=${MPICC} FC=${MPIFC} CXX=${MPICXX} 
make everything
if [ $? -ne 0 ];then
  exit ${ZOLTAN_BUILD_FAILED}
fi
make install
popd


#install PDMlib
if [ ! -d BUILD ];then
  mkdir BUILD
fi
pushd BUILD
cmake ../ -DCMAKE_PREFIX_PATH=${INSTALL_TO} -DCMAKE_INSTALL_PREFIX=${INSTALL_TO}/PDMlib \
-DCMAKE_C_COMPIER=${CC} -DCMAKE_CXX_COMPLER=${CXX} -DCMAKE_Fortran_COMPILER=${FC}
make install
if [ $? -ne 0 ];then
  exit ${PDMLIB_BUILD_FAILED}
fi
popd
