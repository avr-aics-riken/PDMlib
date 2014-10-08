/*
 * マイグレーション機能のデモプログラム
 *
 *   実行時のディレクトリにdfiファイルが存在しなければ、新規のデータを作成して終了し
 *   dfiファイルがあれば、それを読み込んでマイグレーション後の同じデータを次のステップのデータとして出力する
 *
 */
#include <mpi.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include "TestDataGenerator.h"
#include "PDMlib.h"
#include "Utility.h"

int main(int argc, char* argv[])
{
    int      nproc, myrank;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &nproc);
    MPI_Comm_rank(comm, &myrank);
    std::string    base_filename("PDMlibMigrationDemo");
    std::string    dfi_filename = base_filename+".dfi";
    size_t         NumData      = 10000;
    float*         Coord        = NULL;

    int*           IntScaler    = NULL;
    long*          LongScaler   = NULL;
    unsigned int*  uIntScaler   = NULL;
    unsigned long* uLongScaler  = NULL;
    float*         FloatScaler  = NULL;
    double*        DoubleScaler = NULL;
    int*           IntVector    = NULL;
    long*          LongVector   = NULL;
    unsigned int*  uIntVector   = NULL;
    unsigned long* uLongVector  = NULL;
    float*         FloatVector  = NULL;
    double*        DoubleVector = NULL;
    int*           Rank         = NULL;

    // 指定されたdfiファイルが存在しなければ、Rank0のみが初期データを作成、出力して終了
    if(!PDMlib::isFile(dfi_filename))
    {
        if(myrank == 0)
        {
            MPI_Comm new_comm;
            MPI_Comm_split(comm, 0, 0, &new_comm);
            PDMlib::PDMlib::GetInstance().Init(argc, argv, dfi_filename);
            PDMlib::PDMlib::GetInstance().SetBaseFileName(base_filename);
            PDMlib::PDMlib::GetInstance().SetComm(new_comm);

            Coord        = TestDataGenerator<float>::create(3*NumData, "random", myrank);
            IntScaler    = TestDataGenerator<int>::create(NumData, "sequential", myrank);
            LongScaler   = TestDataGenerator<long>::create(NumData, "sequential", myrank);
            uIntScaler   = TestDataGenerator<unsigned int>::create(NumData, "sequential", myrank);
            uLongScaler  = TestDataGenerator<unsigned long>::create(NumData, "sequential", myrank);
            FloatScaler  = TestDataGenerator<float>::create(NumData, "sequential", myrank);
            DoubleScaler = TestDataGenerator<double>::create(NumData, "sequential", myrank);
            IntVector    = TestDataGenerator<int>::create(3*NumData, "sequential_vector", myrank);
            LongVector   = TestDataGenerator<long>::create(3*NumData, "sequential_vector", myrank);
            uIntVector   = TestDataGenerator<unsigned int>::create(3*NumData, "sequential_vector", myrank);
            uLongVector  = TestDataGenerator<unsigned long>::create(3*NumData, "sequential_vector", myrank);
            FloatVector  = TestDataGenerator<float>::create(3*NumData, "sequential_vector", myrank);
            DoubleVector = TestDataGenerator<double>::create(3*NumData, "sequential_vector", myrank);
            Rank         = new int[NumData];
            for(int i = 0; i < NumData; i++)
            {
                Rank[i] = 0;
            }

            double                bbox[6] = {0, 0, 0, 3*NumData, 3*NumData, 3*NumData};
            PDMlib::PDMlib::GetInstance().SetBoundingBox(bbox);
            PDMlib::ContainerInfo container_info1  = {"IntScaler",    "N/A", "none", PDMlib::INT32,  "ints",    1};
            PDMlib::ContainerInfo container_info2  = {"LongScaler",   "N/A", "none", PDMlib::INT64,  "longs",   1};
            PDMlib::ContainerInfo container_info3  = {"uIntScaler",   "N/A", "none", PDMlib::uINT32, "uints",   1};
            PDMlib::ContainerInfo container_info4  = {"uLongScaler",  "N/A", "none", PDMlib::uINT64, "ulongs",  1};
            PDMlib::ContainerInfo container_info5  = {"FloatScaler",  "N/A", "none", PDMlib::FLOAT,  "floats",  1};
            PDMlib::ContainerInfo container_info6  = {"DoubleScaler", "N/A", "none", PDMlib::DOUBLE, "doubles", 1};
            PDMlib::ContainerInfo container_info7  = {"IntVector",    "N/A", "none", PDMlib::INT32,  "intv",    3, PDMlib::NIJK};
            PDMlib::ContainerInfo container_info8  = {"LongVector",   "N/A", "none", PDMlib::INT64,  "longv",   3, PDMlib::IJKN};
            PDMlib::ContainerInfo container_info9  = {"uIntVector",   "N/A", "none", PDMlib::uINT32, "uintv",   3, PDMlib::NIJK};
            PDMlib::ContainerInfo container_info10 = {"uLongVector",  "N/A", "none", PDMlib::uINT64, "ulongv",  3, PDMlib::IJKN};
            PDMlib::ContainerInfo container_info11 = {"FloatVector",  "N/A", "none", PDMlib::FLOAT,  "floatv",  3, PDMlib::NIJK};
            PDMlib::ContainerInfo container_info12 = {"DoubleVector", "N/A", "none", PDMlib::DOUBLE, "doublev", 3, PDMlib::IJKN};
            PDMlib::ContainerInfo container_info13 = {"Coordinate",   "N/A", "none", PDMlib::FLOAT,  "coord",   3, PDMlib::NIJK};
            PDMlib::ContainerInfo container_info14 = {"Rank_Number",  "N/A", "none", PDMlib::INT32,  "rank",    1};

            PDMlib::PDMlib::GetInstance().AddContainer(container_info1);
            PDMlib::PDMlib::GetInstance().AddContainer(container_info2);
            PDMlib::PDMlib::GetInstance().AddContainer(container_info3);
            PDMlib::PDMlib::GetInstance().AddContainer(container_info4);
            PDMlib::PDMlib::GetInstance().AddContainer(container_info5);
            PDMlib::PDMlib::GetInstance().AddContainer(container_info6);
            PDMlib::PDMlib::GetInstance().AddContainer(container_info7);
            PDMlib::PDMlib::GetInstance().AddContainer(container_info8);
            PDMlib::PDMlib::GetInstance().AddContainer(container_info9);
            PDMlib::PDMlib::GetInstance().AddContainer(container_info10);
            PDMlib::PDMlib::GetInstance().AddContainer(container_info11);
            PDMlib::PDMlib::GetInstance().AddContainer(container_info12);
            PDMlib::PDMlib::GetInstance().AddContainer(container_info13);
            PDMlib::PDMlib::GetInstance().AddContainer(container_info14);

            int    TimeStep = 0;
            double Time     = 0.0;
            PDMlib::PDMlib::GetInstance().Write("Coordinate", NumData, Coord, (float*)NULL, 3, TimeStep, Time);
            PDMlib::PDMlib::GetInstance().Write("Rank_Number", NumData, Rank, (int*)NULL, 1, TimeStep, Time);

            PDMlib::PDMlib::GetInstance().Write("IntScaler", NumData, IntScaler, (int*)NULL, 1, TimeStep, Time);
            PDMlib::PDMlib::GetInstance().Write("LongScaler", NumData, LongScaler, (long*)NULL, 1, TimeStep, Time);
            PDMlib::PDMlib::GetInstance().Write("uIntScaler", NumData, uIntScaler, (unsigned int*)NULL, 1, TimeStep, Time);
            PDMlib::PDMlib::GetInstance().Write("uLongScaler", NumData, uLongScaler, (unsigned long*)NULL, 1, TimeStep, Time);
            PDMlib::PDMlib::GetInstance().Write("FloatScaler", NumData, FloatScaler, (float*)NULL, 1, TimeStep, Time);
            PDMlib::PDMlib::GetInstance().Write("DoubleScaler", NumData, DoubleScaler, (double*)NULL, 1, TimeStep, Time);
            PDMlib::PDMlib::GetInstance().Write("IntVector", NumData, IntVector, (int*)NULL, 3, TimeStep, Time);
            PDMlib::PDMlib::GetInstance().Write("LongVector", NumData, LongVector, (long*)NULL, 3, TimeStep, Time);
            PDMlib::PDMlib::GetInstance().Write("uIntVector", NumData, uIntVector, (unsigned int*)NULL, 3, TimeStep, Time);
            PDMlib::PDMlib::GetInstance().Write("uLongVector", NumData, uLongVector, (unsigned long*)NULL, 3, TimeStep, Time);
            PDMlib::PDMlib::GetInstance().Write("FloatVector", NumData, FloatVector, (float*)NULL, 3, TimeStep, Time);
            PDMlib::PDMlib::GetInstance().Write("DoubleVector", NumData, DoubleVector, (double*)NULL, 3, TimeStep, Time);
        }else{
            MPI_Comm new_comm;
            MPI_Comm_split(comm, 1, myrank, &new_comm);
        }
        MPI_Finalize();
        return 0;
    }

    // PDMlibのセットアップ
    PDMlib::PDMlib::GetInstance().Init(argc, argv, dfi_filename, dfi_filename);

    // 読み込み対象のデータポインタをPDMlibに登録する
    PDMlib::PDMlib::GetInstance().RegisterContainer("IntScaler", &IntScaler);
    PDMlib::PDMlib::GetInstance().RegisterContainer("LongScaler", &LongScaler);
    PDMlib::PDMlib::GetInstance().RegisterContainer("uIntScaler", &uIntScaler);
    PDMlib::PDMlib::GetInstance().RegisterContainer("uLongScaler", &uLongScaler);
    PDMlib::PDMlib::GetInstance().RegisterContainer("FloatScaler", &FloatScaler);
    PDMlib::PDMlib::GetInstance().RegisterContainer("DoubleScaler", &DoubleScaler);
    PDMlib::PDMlib::GetInstance().RegisterContainer("IntVector", &IntVector);
    PDMlib::PDMlib::GetInstance().RegisterContainer("LongVector", &LongVector);
    PDMlib::PDMlib::GetInstance().RegisterContainer("uIntVector", &uIntVector);
    PDMlib::PDMlib::GetInstance().RegisterContainer("uLongVector", &uLongVector);
    PDMlib::PDMlib::GetInstance().RegisterContainer("FloatVector", &FloatVector);
    PDMlib::PDMlib::GetInstance().RegisterContainer("DoubleVector", &DoubleVector);

    PDMlib::PDMlib::GetInstance().RegisterContainer("Coordinate", &Coord);
    PDMlib::PDMlib::GetInstance().RegisterContainer("Rank_Number", &Rank);

    // 最新ステップのデータを読み込む
    int TimeStep = -1;
    NumData = PDMlib::PDMlib::GetInstance().ReadAll(&TimeStep, true, "Coordinate");

    // Rank番号だけを更新
    for(int i = 0; i < NumData; i++)
    {
        Rank[i] = myrank;
    }

    // マイグレーション後のデータを出力
    ++TimeStep;
    double Time = TimeStep*0.1;
    PDMlib::PDMlib::GetInstance().Write("Coordinate", NumData, Coord, (float*)NULL, 3, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("Rank_Number", NumData, Rank, (int*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("IntScaler", NumData, IntScaler, (int*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("LongScaler", NumData, LongScaler, (long*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("uIntScaler", NumData, uIntScaler, (unsigned int*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("uLongScaler", NumData, uLongScaler, (unsigned long*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("FloatScaler", NumData, FloatScaler, (float*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("DoubleScaler", NumData, DoubleScaler, (double*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("IntVector", NumData, IntVector, (int*)NULL, 3, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("LongVector", NumData, LongVector, (long*)NULL, 3, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("uIntVector", NumData, uIntVector, (unsigned int*)NULL, 3, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("uLongVector", NumData, uLongVector, (unsigned long*)NULL, 3, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("FloatVector", NumData, FloatVector, (float*)NULL, 3, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("DoubleVector", NumData, DoubleVector, (double*)NULL, 3, TimeStep, Time);

        MPI_Finalize();

    return 0;
}
