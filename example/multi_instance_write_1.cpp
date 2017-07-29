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

    // PDMlibのセットアップその1
    PDMlib::PDMlib pdm1;
    {
        pdm1.Init(argc, argv, "PDMlibWriteSample_1.dfi");
        pdm1.SetBaseFileName("PDMlibWriteSample_1");
        pdm1.SetFileNameFormat("Step_rank");

        PDMlib::ContainerInfo ID          = {"ParticleID", "ID_number", "zip", PDMlib::INT32, "id", 1};
        PDMlib::ContainerInfo Coordinate  = {"Coordinate", "N/A", "none", PDMlib::FLOAT, "geo", 3, PDMlib::NIJK};
        PDMlib::ContainerInfo Velocity    = {"Velocity", "N/A", "none", PDMlib::DOUBLE, "vel", 3, PDMlib::NIJK};
        PDMlib::ContainerInfo Temparature = {"Temparature", "N/A", "none", PDMlib::DOUBLE, "temp", 1};
        pdm1.AddContainer(ID);
        pdm1.AddContainer(Coordinate);
        pdm1.AddContainer(Velocity);
        pdm1.AddContainer(Temparature);
        double bbox[6] = {0, 0, 0, 3*NumData/2, 3*NumData/2, 3*NumData/2};
        pdm1.SetBoundingBox(bbox);
    }
    // PDMlibのセットアップその2
    PDMlib::PDMlib pdm2;
    {
        pdm2.Init(argc, argv, "PDMlibWriteSample_2.dfi");
        pdm2.SetBaseFileName("PDMlibWriteSample_2");
        pdm2.SetFileNameFormat("Step_rank");

        PDMlib::ContainerInfo ID          = {"ParticleID", "ID_number", "zip", PDMlib::INT32, "id", 1};
        PDMlib::ContainerInfo Coordinate  = {"Coordinate", "N/A", "none", PDMlib::FLOAT, "geo", 3, PDMlib::NIJK};
        PDMlib::ContainerInfo Velocity    = {"Velocity", "N/A", "none", PDMlib::DOUBLE, "vel", 3, PDMlib::NIJK};
        PDMlib::ContainerInfo Temparature = {"Temparature", "N/A", "none", PDMlib::DOUBLE, "temp", 1};
        pdm2.AddContainer(ID);
        pdm2.AddContainer(Coordinate);
        pdm2.AddContainer(Velocity);
        pdm2.AddContainer(Temparature);
        double bbox[6] = {0, 0, 0, 3*NumData/2, 3*NumData/2, 3*NumData/2};
        pdm2.SetBoundingBox(bbox);
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
            //前半のMinMax値の計算を行う
            double u          = std::sqrt(Velocity[0]*Velocity[0]+Velocity[1]*Velocity[1]+Velocity[2]*Velocity[2]);
            double vMinMax1[8] = {u, u, Velocity[0], Velocity[0], Velocity[1], Velocity[1], Velocity[2], Velocity[2]};
            for(int i = 0; i < NumData*3/2; i+=3)
            {
                u = std::sqrt(Velocity[i]*Velocity[i]+Velocity[i+1]*Velocity[i+1]+Velocity[i+2]*Velocity[i+2]);
                if(vMinMax1[0] > u){vMinMax1[0] = u;}
                if(vMinMax1[1] < u){vMinMax1[1] = u;}
                if(vMinMax1[2] > Velocity[i]){vMinMax1[2] = Velocity[i];}
                if(vMinMax1[3] < Velocity[i]){vMinMax1[3] = Velocity[i];}
                if(vMinMax1[4] > Velocity[i+1]){vMinMax1[4] = Velocity[i+1];}
                if(vMinMax1[5] < Velocity[i+1]){vMinMax1[5] = Velocity[i+1];}
                if(vMinMax1[6] > Velocity[i+2]){vMinMax1[6] = Velocity[i+2];}
                if(vMinMax1[7] < Velocity[i+2]){vMinMax1[7] = Velocity[i+2];}
            }
            double tMinMax1[2] = {Temparature[0], Temparature[0]};
            for(int i = 0; i < NumData/2; i++)
            {
                if(tMinMax1[0] > Temparature[i]){tMinMax1[0] = Temparature[i];}
                if(tMinMax1[1] < Temparature[i]){tMinMax1[1] = Temparature[i];}
            }

            //後半のMinMax値の計算を行う
            u          = std::sqrt(Velocity[NumData*3/2]*Velocity[NumData*3/2]+Velocity[1+NumData*3/2]*Velocity[1+NumData*3/2]+Velocity[2+NumData*3/2]*Velocity[2+NumData*3/2]);
            double vMinMax2[8] = {u, u, Velocity[NumData*3/2], Velocity[NumData*3/2], Velocity[1+NumData*3/2], Velocity[1+NumData*3/2], Velocity[2+NumData*3/2], Velocity[2+NumData*3/2]};
            for(int i = NumData*3/2; i < NumData*3; i+=3)
            {
                u = std::sqrt(Velocity[i]*Velocity[i]+Velocity[i+1]*Velocity[i+1]+Velocity[i+2]*Velocity[i+2]);
                if(vMinMax2[0] > u){vMinMax2[0] = u;}
                if(vMinMax2[1] < u){vMinMax2[1] = u;}
                if(vMinMax2[2] > Velocity[i]){vMinMax2[2] = Velocity[i];}
                if(vMinMax2[3] < Velocity[i]){vMinMax2[3] = Velocity[i];}
                if(vMinMax2[4] > Velocity[i+1]){vMinMax2[4] = Velocity[i+1];}
                if(vMinMax2[5] < Velocity[i+1]){vMinMax2[5] = Velocity[i+1];}
                if(vMinMax2[6] > Velocity[i+2]){vMinMax2[6] = Velocity[i+2];}
                if(vMinMax2[7] < Velocity[i+2]){vMinMax2[7] = Velocity[i+2];}
            }
            double tMinMax2[2] = {Temparature[NumData/2], Temparature[NumData/2]};
            for(int i = NumData/2; i < NumData; i++)
            {
                if(tMinMax2[0] > Temparature[i]){tMinMax2[0] = Temparature[i];}
                if(tMinMax2[1] < Temparature[i]){tMinMax2[1] = Temparature[i];}
            }

            //前半のデータをpdm1を使って出力
            pdm1.Write("ParticleID",  NumData/2, ID, (int*)NULL, 1, TimeStep, Time);
            pdm1.Write("Coordinate",  NumData/2, Coordinate, (float*)NULL, 3, TimeStep, Time);
            pdm1.Write("Velocity",    NumData/2, Velocity, vMinMax1, 3, TimeStep, Time);
            pdm1.Write("Temparature", NumData/2, Temparature, tMinMax2, 1, TimeStep, Time);



            //後半のデータをpdm2を使って出力
            pdm2.Write("ParticleID",  NumData/2, ID+NumData/2, (int*)NULL, 1, TimeStep, Time);
            pdm2.Write("Coordinate",  NumData/2, Coordinate+NumData/2, (float*)NULL, 3, TimeStep, Time);
            pdm2.Write("Velocity",    NumData/2, Velocity+NumData/2, vMinMax2, 3, TimeStep, Time);
            pdm2.Write("Temparature", NumData/2, Temparature+NumData/2, tMinMax2, 1, TimeStep, Time);
        }
    }

    MPI_Finalize();

    return 0;
}
