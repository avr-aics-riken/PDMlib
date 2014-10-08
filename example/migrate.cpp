/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */


/*
 * データの読み込みとマイグレーションを行うサンプルプログラム
 *
 * 出力サンプルプログラム(write.cpp)が出力するデータを入力データとして使用しているので
 * 出力データを置いているディレクトリで実行してください。
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
    {
        PDMlib::PDMlib::GetInstance().Init(argc, argv, "PDMlibMigrationSample.dfi", "PDMlibMigrationSample.dfi");
    }

    int*    ID          = NULL;
    float*  Coordinate  = NULL;
    double* Velocity    = NULL;
    double* Temparature = NULL;

    // 読み込み対象のデータポインタをPDMlibに登録する
    PDMlib::PDMlib::GetInstance().RegisterContainer("ParticleID", &ID);
    PDMlib::PDMlib::GetInstance().RegisterContainer("Coordinate", &Coordinate);
    PDMlib::PDMlib::GetInstance().RegisterContainer("Velocity", &Velocity);
    PDMlib::PDMlib::GetInstance().RegisterContainer("Temparature", &Temparature);

    // データを読み込む
    int    TimeStep = -1;
    size_t NumData  = PDMlib::PDMlib::GetInstance().ReadAll(&TimeStep, true, "Coordinate");

    ++TimeStep;

    //MinMax値の計算を行う
    double u          = std::sqrt(Velocity[0]*Velocity[0]+Velocity[NumData]*Velocity[NumData]+Velocity[2*NumData]*Velocity[2*NumData]);
    double vMinMax[8] = {u, u, Velocity[0], Velocity[0], Velocity[NumData], Velocity[NumData], Velocity[2*NumData], Velocity[2*NumData]};
    for(int i = 0; i < NumData; i++)
    {
        u = std::sqrt(Velocity[i]*Velocity[i]+Velocity[NumData+i]*Velocity[NumData+i]+Velocity[2*NumData+i]*Velocity[2*NumData+i]);
        if(vMinMax[0] > u){vMinMax[0] = u;}
        if(vMinMax[1] < u){vMinMax[1] = u;}
        if(vMinMax[2] > Velocity[i]){vMinMax[2] = Velocity[i];}
        if(vMinMax[3] < Velocity[i]){vMinMax[3] = Velocity[i];}
        if(vMinMax[4] > Velocity[NumData+i]){vMinMax[4] = Velocity[NumData+i];}
        if(vMinMax[5] < Velocity[NumData+i]){vMinMax[5] = Velocity[NumData+i];}
        if(vMinMax[6] > Velocity[2*NumData+i]){vMinMax[6] = Velocity[2*NumData+i];}
        if(vMinMax[7] < Velocity[2*NumData+i]){vMinMax[7] = Velocity[2*NumData+i];}
    }
    double tMinMax[2] = {Temparature[0], Temparature[0]};
    for(int i = 0; i < NumData; i++)
    {
        if(tMinMax[0] > Temparature[i]){tMinMax[0] = Temparature[i];}
        if(tMinMax[1] < Temparature[i]){tMinMax[1] = Temparature[i];}
    }

    double Time = 0.1;
    //Write()を呼び出してコンテナ毎にファイル出力
    PDMlib::PDMlib::GetInstance().Write("ParticleID", NumData, ID, (int*)NULL, 1, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("Coordinate", NumData, Coordinate, (float*)NULL, 3, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("Velocity", NumData, Velocity, vMinMax, 3, TimeStep, Time);
    PDMlib::PDMlib::GetInstance().Write("Temparature", NumData, Temparature, tMinMax, 1, TimeStep, Time);

    MPI_Finalize();

    return 0;
}