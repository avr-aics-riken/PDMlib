#!/bin/sh
# -*- coding: utf-8 -*-
##############################################################################
#
# PDMlib Install Script
#
# Copyright (c) 2015 Advanced Institute for Computational Science, RIKEN.
# All rights reserved.
#
##############################################################################

FPZIP=fpzip-1.0.1
FPZIP_ARCHIVE=${FPZIP}.tar.gz
ZOLTAN=Zoltan_v3.81
ZOLTAN_ARCHIVE=zoltan_distrib_v3.81.tar.gz
TP=TextParser-1.6.3
TP_ARCHIVE=${TP}.tar.gz

#error code definision
ILLEGAL_OPTION=1
MISSING_INSTALL_PATH=2
FPZIP_NOT_FOUND=3
ZOLTAN_NOT_FOUND=4
FPZIP_BUILD_FAILED=5
ZOLTAN_BUILD_FAILED=6
PDMLIB_BUILD_FAILED=7

# argument parse
PREFIX=""  
COMPILER=""
MPI=""
BUILD_ENV=""
MPI_ENV=""
Zoltan_path=""
Fpzip_path=""
TextParser_path=""
Hdf5_path=""

while getopts "c:m:p:z:f:t:h:" flag; do
  case $flag in
    c) COMPILER="$OPTARG";;
    m) MPI="$OPTARG";;
    p) PREFIX="$OPTARG";;
    z) Zoltan_path="$OPTARG";;
    f) Fpzip_path="$OPTARG";;
    t) TextParser_path="$OPTARG";;
    h) Hdf5_path="$OPTARG";;
    \?) 
    echo "usage: $0 -p PREFIX [options]"
    echo "optios:"
    echo " -c [GNU|Intel|FX] specify compiler"
    echo " -m [OpenMPI | IntelMPI | mpich ]  specify MPI lib"
    echo " -p PREFIX specify install directory"
    echo " -z PATH   specify zoltan installed directory"
    echo " -f PATH   specify fpzip installed directory"
    echo " -t PATH   specify TextParser installed directory"
    echo " -h PATH   specify HDF5 installed directory"
    exit ${ILLEGAL_OPTION}
  esac
done

if [ x${PREFIX} == x ];then
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
if [ x${Fpzip_path} == x ];then
  echo "fpzip will be build from source"
  if [ ! -d ${FPZIP} ]; then
    if [ ! -f ${FPZIP_ARCHIVE} ]; then
      echo "fpzip archive is not found in this directory"
      echo "please download it from http://computation.llnl.gov/casc/fpzip/"
      echo "note: ver. 1.1 is not supported for now. please use ver. 1.0.1."
      exit ${FPZIP_NOT_FOUND}
    else
      tar xfz ${FPZIP_ARCHIVE} 2>/dev/null
    fi
  fi
  pushd  ${FPZIP}/src
  make CC=${MPICC} CXX=${MPICXX} 
  if [ $? -ne 0 ];then
    exit ${FIPZIP_BUILD_FAILED}
  fi
  if [ ! -d ${PREFIX}/fpzip/include ]; then
    mkdir -p ${PREFIX}/fpzip/include 
  fi
  cp ../inc/fpzip.h ${PREFIX}/fpzip/include

  if [ ! -d ${PREFIX}/fpzip/lib ]; then
    mkdir -p ${PREFIX}/fpzip/lib
  fi
  cp ../lib/libfpzip.a ${PREFIX}/fpzip/lib
  popd
  Fpzip_path=${PREFIX}/fpzip
fi

#install Zoltan
if [ x${Zoltan_path} == x ];then
  echo "Zoltan will be build from source"
  if [ ! -d ${ZOLTAN} ]; then
    if [ ! -f ${ZOLTAN_ARCHIVE} ]; then
      echo "Zoltan archive is not found in this directory"
      echo "please download it from http://www.cs.sandia.gov/web1400/1400_download.html"
      exit ${ZOLTAN_NOT_FOUND}
    else
      tar xfz ${ZOLTAN_ARCHIVE} 2>/dev/null
    fi
  fi

  pushd ${ZOLTAN}
  if [ ! -d BUILD ];then
    mkdir BUILD
  fi
  cd BUILD
  ../configure --with-id-type=ulong --prefix=${PREFIX}/Zoltan CC=${MPICC} FC=${MPIFC} CXX=${MPICXX} 
  make everything
  if [ $? -ne 0 ];then
    exit ${ZOLTAN_BUILD_FAILED}
  fi
  make install
  popd
  Zoltan_path=${PREFIX}/Zoltan
fi

#install TextParser
if [  x${TextParser_path} == x"" ];then
  echo "TextParser will be build from source"
  if [ ! -d ${TP} ]; then
    if [ ! -f ${TP_ARCHIVE} ];then
      echo "TextParser archive is not found in this directory"
      echo "please download it from https://github.com/avr-aics-riken/TextParser/releases"
      exit ${TP_NOT_FOUND}
    else
      tar xfz ${TP_ARCHIVE} 2>/dev/null
    fi
  fi
  pushd ${TP}
  if [ ! -d BUILD ];then
    mkdir BUILD
  fi
  cd BUILD
  ../configure  --prefix=${PREFIX}/TP CXX=${MPICXX} CXXFLAGS="-O3"
  make everything
  if [ $? -ne 0 ];then
    exit ${TP_BUILD_FAILED}
  fi
  make install
  popd
  TextParser_path=${PREFIX}/TP
fi

#set environment variable for HDF5 serch path 
if [  x${Hdf5_path} != x"" ];then
  export HDF5_ROOT=${Hdf5_path}
fi

#install PDMlib
if [ -d BUILD ];then
  rm -fr BUILD
fi
mkdir BUILD
pushd BUILD
export CC
export CXX
export FC
cmake ../ -Dbuild_h5part_converter=no -DCMAKE_INSTALL_PREFIX=${PREFIX}/PDMlib -DCMAKE_PREFIX_PATH=${PREFIX} -DTP_ROOT=${TextParser_path} -DZOLTAN_ROOT=${Zoltan_path} -DFPZIP_ROOT=${Fpzip_path}
make install
if [ $? -ne 0 ];then
  exit ${PDMLIB_BUILD_FAILED}
fi
popd
