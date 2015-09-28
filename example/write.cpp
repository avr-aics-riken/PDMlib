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
    int*      ID          = TestDataGenerator<int>::create(NumData, "sequential");
    float*    Coordinate  = TestDataGenerator<float>::create(3*NumData, "random", myrank);
    double*   Velocity    = TestDataGenerator<double>::create(3*NumData, "random", myrank);
    double*   Temparature = TestDataGenerator<double>::create(1*NumData, "random", myrank);

    // PDMlibのセットアップ
    {
        PDMlib::PDMlib::GetInstance().Init(argc, argv, "PDMlibWriteSample.dfi");
        PDMlib::PDMlib::GetInstance().SetBaseFileName("PDMlibWriteSample");
        PDMlib::PDMlib::GetInstance().SetFileNameFormat("Step_rank");

        PDMlib::ContainerInfo ID          = {"ParticleID", "ID_number", "zip", PDMlib::INT32, "id", 1};
        PDMlib::ContainerInfo Coordinate  = {"Coordinate", "N/A", "none", PDMlib::FLOAT, "geo", 3, PDMlib::IJKN};
        PDMlib::ContainerInfo Velocity    = {"Velocity", "N/A", "none", PDMlib::DOUBLE, "vel", 3, PDMlib::IJKN};
        PDMlib::ContainerInfo Temparature = {"Temparature", "N/A", "none", PDMlib::DOUBLE, "temp", 1};
        PDMlib::PDMlib::GetInstance().AddContainer(ID);
        PDMlib::PDMlib::GetInstance().AddContainer(Coordinate);
        PDMlib::PDMlib::GetInstance().AddContainer(Velocity);
        PDMlib::PDMlib::GetInstance().AddContainer(Temparature);
        double bbox[6] = {0, 0, 0, 3*NumData, 3*NumData, 3*NumData};
        PDMlib::PDMlib::GetInstance().SetBoundingBox(bbox);
    }

    double Time   = 0;
    double deltaT = 0.1;
    for(int TimeStep = 0; TimeStep < 1000; TimeStep++)
    {
        Time += deltaT;
        //
        // 本来はこの部分で計算を行う
        //

        // 100ステップに1回ファイル出力する
        if(TimeStep%100 == 0)
        {
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

            //Write()を呼び出してコンテナ毎にファイル出力
            PDMlib::PDMlib::GetInstance().Write("ParticleID", NumData, ID, (int*)NULL, 1, TimeStep, Time);
            PDMlib::PDMlib::GetInstance().Write("Coordinate", NumData, Coordinate, (float*)NULL, 3, TimeStep, Time);
            PDMlib::PDMlib::GetInstance().Write("Velocity", NumData, Velocity, vMinMax, 3, TimeStep, Time);
            PDMlib::PDMlib::GetInstance().Write("Temparature", NumData, Temparature, tMinMax, 1, TimeStep, Time);
        }
    }

    MPI_Finalize();

    return 0;
}
