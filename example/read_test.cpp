/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */

#include <mpi.h>
#include <cmath>
#include "TestDataGenerator.h"
#include "PDMlib.h"

int main(int argc, char* argv[])
{
    int      nproc, myrank;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &nproc);
    MPI_Comm_rank(comm, &myrank);

    // PDMlibのセットアップ
    PDMlib::PDMlib::GetInstance().Init(argc, argv, "dummy.dfi", "EndianConversionTest.dfi");
    PDMlib::PDMlib::GetInstance().SetBaseFileName("EndianConversionTest");

    size_t NumData     = 10000;
    // 読み込み用のポインタ
    int*           Data_int    = NULL;
    unsigned int*  Data_uint   = NULL;
    long*          Data_long   = NULL;
    unsigned long* Data_ulong  = NULL;
    float*         Data_float  = NULL;
    double*        Data_double = NULL;
    int            TimeStep    = -1;

    // PDMlibを用いてデータの読み込みを行なう
    PDMlib::PDMlib::GetInstance().Read("TestData_int", &NumData, &Data_int, &TimeStep);
    std::cerr<<"TestData_int: ";
    for(int i=0;i<3;i++) std::cerr<<Data_int[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_uint", &NumData,   &Data_uint, &TimeStep);
    std::cerr<<"TestData_uint: ";
    for(int i=0;i<3;i++) std::cerr<<Data_uint[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_long",  &NumData,  &Data_long, &TimeStep);
    std::cerr<<"TestData_long: ";
    for(int i=0;i<3;i++) std::cerr<<Data_long[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_ulong", &NumData,  &Data_ulong, &TimeStep);
    std::cerr<<"TestData_ulong: ";
    for(int i=0;i<3;i++) std::cerr<<Data_ulong[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_float", &NumData,  &Data_float, &TimeStep);
    std::cerr<<"TestData_float: ";
    for(int i=0;i<3;i++) std::cerr<<Data_float[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_double",&NumData,  &Data_double, &TimeStep);
    std::cerr<<"TestData_double: ";
    for(int i=0;i<3;i++) std::cerr<<Data_double[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_zip",   &NumData,  &Data_int, &TimeStep);
    std::cerr<<"TestData_zip: ";
    for(int i=0;i<3;i++) std::cerr<<Data_int[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_RLE",   &NumData,  &Data_int, &TimeStep);
    std::cerr<<"TestData_RLE: ";
    for(int i=0;i<3;i++) std::cerr<<Data_int[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_fpzip", &NumData,  &Data_float, &TimeStep);
    std::cerr<<"TestData_fpzip: ";
    for(int i=0;i<3;i++) std::cerr<<Data_float[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_zf",    &NumData,  &Data_float, &TimeStep);
    std::cerr<<"TestData_zf: ";
    for(int i=0;i<3;i++) std::cerr<<Data_float[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_zr",    &NumData,  &Data_float, &TimeStep);
    std::cerr<<"TestData_zr: ";
    for(int i=0;i<3;i++) std::cerr<<Data_float[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_zfr",   &NumData,  &Data_float, &TimeStep);
    std::cerr<<"TestData_zfr: ";
    for(int i=0;i<3;i++) std::cerr<<Data_float[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_zrf",   &NumData,  &Data_float, &TimeStep);
    std::cerr<<"TestData_zrf: ";
    for(int i=0;i<3;i++) std::cerr<<Data_float[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_fz",    &NumData,  &Data_float, &TimeStep);
    std::cerr<<"TestData_fz: ";
    for(int i=0;i<3;i++) std::cerr<<Data_float[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_fr",    &NumData,  &Data_float, &TimeStep);
    std::cerr<<"TestData_fr: ";
    for(int i=0;i<3;i++) std::cerr<<Data_float[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_fzr",   &NumData,  &Data_float, &TimeStep);
    std::cerr<<"TestData_fzr: ";
    for(int i=0;i<3;i++) std::cerr<<Data_float[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_frz",   &NumData,  &Data_float, &TimeStep);
    std::cerr<<"TestData_frz: ";
    for(int i=0;i<3;i++) std::cerr<<Data_float[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_rz",    &NumData,  &Data_float, &TimeStep);
    std::cerr<<"TestData_rz: ";
    for(int i=0;i<3;i++) std::cerr<<Data_float[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_rf",    &NumData,  &Data_float, &TimeStep);
    std::cerr<<"TestData_rf: ";
    for(int i=0;i<3;i++) std::cerr<<Data_float[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_rzf",   &NumData,  &Data_float, &TimeStep);
    std::cerr<<"TestData_rzf: ";
    for(int i=0;i<3;i++) std::cerr<<Data_float[i]<<",";
    std::cerr<<std::endl;

    PDMlib::PDMlib::GetInstance().Read("TestData_rfz",   &NumData,  &Data_float, &TimeStep);
    std::cerr<<"TestData_rfz: ";
    for(int i=0;i<3;i++) std::cerr<<Data_float[i]<<",";
    std::cerr<<std::endl;


    MPI_Finalize();

    return 0;
}
