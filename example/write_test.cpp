/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */

/*
 * データ出力サンプルプログラム
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

    // 出力用のダミーデータを用意
    const int NumData     = 10000;
    int*           Data_int    = TestDataGenerator<int>::create(NumData, "sequential");
    unsigned int*  Data_uint   = TestDataGenerator<unsigned int>::create(NumData, "sequential");
    long*          Data_long   = TestDataGenerator<long>::create(NumData, "sequential");
    unsigned long* Data_ulong  = TestDataGenerator<unsigned long>::create(NumData, "sequential");
    float*         Data_float  = TestDataGenerator<float>::create(NumData, "sequential");
    double*        Data_double = TestDataGenerator<double>::create(NumData, "sequential");

    // PDMlibのセットアップ
    PDMlib::PDMlib::GetInstance().Init(argc, argv, "EndianConversionTest.dfi");
    PDMlib::PDMlib::GetInstance().SetBaseFileName("EndianConversionTest");
    PDMlib::PDMlib::GetInstance().SetFileNameFormat("Step_rank");

    PDMlib::ContainerInfo CI_int    = {"TestData_int",    "ID_number", "none", PDMlib::INT32,  "int",    1};
    PDMlib::ContainerInfo CI_uint   = {"TestData_uint",   "ID_number", "none", PDMlib::uINT32, "uint",   1};
    PDMlib::ContainerInfo CI_long   = {"TestData_long",   "ID_number", "none", PDMlib::INT64,  "long",   1};
    PDMlib::ContainerInfo CI_ulong  = {"TestData_ulong",  "ID_number", "none", PDMlib::uINT64, "ulong",  1};
    PDMlib::ContainerInfo CI_float  = {"TestData_float",  "ID_number", "none", PDMlib::FLOAT,  "float",  1};
    PDMlib::ContainerInfo CI_double = {"TestData_double", "ID_number", "none", PDMlib::DOUBLE, "double", 1};
    PDMlib::ContainerInfo CI_zip    = {"TestData_zip",    "ID_number", "zip", PDMlib::INT32,  "zip",    1};
    PDMlib::ContainerInfo CI_RLE    = {"TestData_RLE",    "ID_number", "rle", PDMlib::INT32,  "RLE",    1};
    PDMlib::ContainerInfo CI_fpzip  = {"TestData_fpzip",  "ID_number", "fpzip", PDMlib::FLOAT,  "fpzip",  1};
    PDMlib::ContainerInfo CI_zf     = {"TestData_zf",  "ID_number", "zip-fpzip", PDMlib::FLOAT,  "zf",  1};
    PDMlib::ContainerInfo CI_zr     = {"TestData_zr",  "ID_number", "zip-rle", PDMlib::FLOAT,  "zr",  1};
    PDMlib::ContainerInfo CI_zfr    = {"TestData_zfr",  "ID_number", "zip-fpzip-rle", PDMlib::FLOAT,  "zfr",  1};
    PDMlib::ContainerInfo CI_zrf    = {"TestData_zrf",  "ID_number", "zip-rle-fpzip", PDMlib::FLOAT,  "zrf",  1};
    PDMlib::ContainerInfo CI_fz     = {"TestData_fz",  "ID_number", "fpzip-zip", PDMlib::FLOAT,  "fz",  1};
    PDMlib::ContainerInfo CI_fr     = {"TestData_fr",  "ID_number", "fpzip-rle", PDMlib::FLOAT,  "fr",  1};
    PDMlib::ContainerInfo CI_fzr    = {"TestData_fzr",  "ID_number", "fpzip-zip-rle", PDMlib::FLOAT,  "fzr",  1};
    PDMlib::ContainerInfo CI_frz    = {"TestData_frz",  "ID_number", "fpzip-rle-zip", PDMlib::FLOAT,  "frz",  1};
    PDMlib::ContainerInfo CI_rz     = {"TestData_rz",  "ID_number", "rle-zip", PDMlib::FLOAT,  "rz",  1};
    PDMlib::ContainerInfo CI_rf     = {"TestData_rf",  "ID_number", "rle-fpzip", PDMlib::FLOAT,  "rf",  1};
    PDMlib::ContainerInfo CI_rzf    = {"TestData_rzf",  "ID_number", "rle-zip-fpzip", PDMlib::FLOAT,  "rzf",  1};
    PDMlib::ContainerInfo CI_rfz    = {"TestData_rfz",  "ID_number", "rle-fpzip-zip", PDMlib::FLOAT,  "rfz",  1};

    PDMlib::PDMlib::GetInstance().AddContainer(CI_int);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_uint);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_ulong);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_long);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_float);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_double);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_zip);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_RLE);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_fpzip);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_zf);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_zr);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_zfr);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_zrf);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_fz);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_fr);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_fzr);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_frz);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_rz);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_rf);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_rzf);
    PDMlib::PDMlib::GetInstance().AddContainer(CI_rfz);

    double bbox[6] = {0, 0, 0, 3*NumData, 3*NumData, 3*NumData};
    PDMlib::PDMlib::GetInstance().SetBoundingBox(bbox);

    const int TimeStep=0;
    const double Time=0.0;
    // データ出力
    PDMlib::PDMlib::GetInstance().Write("TestData_int",    NumData, Data_int,    (int*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_uint",   NumData, Data_uint,   (unsigned int*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_long",   NumData, Data_long,   (long*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_ulong",  NumData, Data_ulong,  (unsigned long*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_float",  NumData, Data_float,  (float*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_double", NumData, Data_double, (double*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_zip",    NumData, Data_int,    (int*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_RLE",    NumData, Data_int,    (int*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_fpzip",  NumData, Data_float,  (float*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_zf",     NumData, Data_float,  (float*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_zr",     NumData, Data_float,  (float*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_zfr",    NumData, Data_float,  (float*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_zrf",    NumData, Data_float,  (float*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_fz",     NumData, Data_float,  (float*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_fr",     NumData, Data_float,  (float*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_fzr",    NumData, Data_float,  (float*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_frz",    NumData, Data_float,  (float*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_rz",     NumData, Data_float,  (float*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_rf",     NumData, Data_float,  (float*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_rzf",    NumData, Data_float,  (float*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("TestData_rfz",    NumData, Data_float,  (float*)NULL, 1, TimeStep, Time);


    MPI_Finalize();

    return 0;
}
