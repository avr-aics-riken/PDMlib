/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */


/*
 * データの読み込みを行うサンプルプログラム
 *
 * 出力サンプルプログラム(write.cpp)が出力するデータを入力データとして使用しているので
 * 出力データを置いているディレクトリで実行してください。
 *
 * 出力サンプルプログラム実行時のプロセス数より多いプロセス数で実行すると
 * DataRead()内のエラーチェックで、担当するファイルが無いRankがエラーになるため正常に動作しません。
 *
 * 本来はPDMlib::Read()の返り値に対して、担当するファイルがあるプロセスでは0以下、
 * 担当するファイルが無いプロセスでは0以外が返ってきた時にエラーとする必要があります。
 *
 * ただし、このような状況が起こり得る場合は、restart2.cppのようにRegisterContainer()とReadAll()を使って
 * 読み込んだデータのマイグレーションを行うことをお勧めします。
 *
 */
#include <mpi.h>
#include <cmath>
#include "TestDataGenerator.h"
#include "PDMlib.h"

size_t DataRead(int** ID, float** Coordinate, double** Velocity, double** Temparature, int* TimeStep)
{
    // コンテナの一覧を取得
    std::vector<PDMlib::ContainerInfo> containers = PDMlib::PDMlib::GetInstance().GetContainerInfo();

    // フィールドデータの読み込み
    // PDMlib::Read()の第四引数（デフォルトは-1)にタイムステップを指定すると
    // 特定のタイムステップのデータを読み込むこともできる。
    size_t len_id    = 30000;
    size_t len_coord = 30000;
    size_t len_vel   = 30000;
    size_t len_temp  = 30000;
    if(PDMlib::PDMlib::GetInstance().Read("ParticleID", &len_id, ID, TimeStep) <= 0)
    {
        std::cerr<<"ID read error"<<std::endl;
        MPI_Abort(MPI_COMM_WORLD, -1);
    }
    if(PDMlib::PDMlib::GetInstance().Read("Coordinate", &len_coord, Coordinate, TimeStep) <= 0)
    {
        std::cerr<<"Coordinate read error"<<std::endl;
        MPI_Abort(MPI_COMM_WORLD, -1);
    }
    if(PDMlib::PDMlib::GetInstance().Read("Velocity", &len_vel, Velocity, TimeStep) <= 0)
    {
        std::cerr<<"Velocity read error"<<std::endl;
        MPI_Abort(MPI_COMM_WORLD, -1);
    }
    if(PDMlib::PDMlib::GetInstance().Read("Temparature", &len_temp, Temparature, TimeStep) <= 0)
    {
        std::cerr<<"Temparature read error"<<std::endl;
        MPI_Abort(MPI_COMM_WORLD, -1);
    }

    return len_id;
}

int main(int argc, char* argv[])
{
    int      nproc, myrank;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(comm, &nproc);
    MPI_Comm_rank(comm, &myrank);

    // PDMlibのセットアップ
    {
        PDMlib::PDMlib::GetInstance().Init(argc, argv, "PDMlibRestartSample.dfi", "PDMlibWriteSample.dfi");
        PDMlib::PDMlib::GetInstance().SetBaseFileName("PDMlibRestartSample");
    }

    /*
     * PDMlibを使って既存のデータを読み込む
     */
    int*    ID          = NULL;
    float*  Coordinate  = NULL;
    double* Velocity    = NULL;
    double* Temparature = NULL;

    // PDMlibを用いてデータの読み込みを行なう
    int     StartTime = -1;
    size_t  NumData   = DataRead(&ID, &Coordinate, &Velocity, &Temparature, &StartTime);

    double  Time   = 0;
    double  deltaT = 0.1;
    for(int TimeStep = StartTime; TimeStep < StartTime+1000; TimeStep++)
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