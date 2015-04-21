/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */

#include <mpi.h>
#include <set>
#include <climits>
#include <vector>
#include <unistd.h>
#include <sstream>
#include "PDMlib.h"
#include "Utility.h"
#include "FV14Writer.h"

namespace
{
void PrintUsageAndAbort(const char* cmd)
{
    std::cerr<<"usage: "<<cmd<<" -f meta_data_file {-c Coordinate Container name} {-s start time} {-e end time} {-d directory} {-b}"<<std::endl;
    std::cerr<<"       if '-b' is specified, bounding box will be output as FV-UNS(text)"<<std::endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
}

void ComandLineParser(const int& argc, char** argv, int* start, int* end, std::string* dir_name, std::string* dfi_filename, std::string* coordinate, bool* with_bbox)
{
    int results = 0;
    while((results = getopt(argc, argv, "s:e:c:d:f:b")) != -1)
    {
        switch(results)
        {
        case 's':
            *start = atoi(optarg);
            break;

        case 'e':
            *end = atoi(optarg);
            break;

        case 'c':
            *coordinate = optarg;

        case 'd':
            *dir_name = optarg;
            break;

        case 'f':
            *dfi_filename = optarg;
            break;

        case 'b':
            *with_bbox = true;
            break;

        case '?':
            PrintUsageAndAbort(argv[0]);
            break;
        }
    }
}

template<typename T>
void ReadAndWriteCoordinate(std::ofstream& out, T** ptr, PDMlib::ContainerInfo container_info, const int& time_step)
{
    int    tmp_time_step = time_step;
    size_t length        = -1;
    PDMlib::PDMlib::GetInstance().Read(container_info.Name, &length, ptr, &tmp_time_step, true);
    if(container_info.VectorOrder == PDMlib::NIJK)
    {
        FV14Writer::WriteCoordNIJK(out, ptr, length);
    }else if(container_info.VectorOrder == PDMlib::IJKN){
        FV14Writer::WriteCoordIJKN(out, ptr, length);
    }
    delete[] *ptr;
}

void ReadAndWriteCoordinateSelector(std::ofstream& out, PDMlib::ContainerInfo container_info, const int& time_step)
{
    if(container_info.Type == PDMlib::INT32)
    {
        int* ptr = NULL;
        ReadAndWriteCoordinate(out, &ptr, container_info, time_step);
    }else if(container_info.Type == PDMlib::uINT32){
        unsigned int* ptr = NULL;
        ReadAndWriteCoordinate(out, &ptr, container_info, time_step);
    }else if(container_info.Type == PDMlib::INT64){
        long* ptr = NULL;
        ReadAndWriteCoordinate(out, &ptr, container_info, time_step);
    }else if(container_info.Type == PDMlib::uINT64){
        unsigned long* ptr = NULL;
        ReadAndWriteCoordinate(out, &ptr, container_info, time_step);
    }else if(container_info.Type == PDMlib::FLOAT){
        float* ptr = NULL;
        ReadAndWriteCoordinate(out, &ptr, container_info, time_step);
    }else if(container_info.Type == PDMlib::DOUBLE){
        double* ptr = NULL;
        ReadAndWriteCoordinate(out, &ptr, container_info, time_step);
    }
}

template<typename T>
void ReadAndWriteContainer(std::ofstream& out, T** ptr, PDMlib::ContainerInfo container_info, const int& time_step)
{
    int    tmp_time_step = time_step;
    size_t length        = -1;
    PDMlib::PDMlib::GetInstance().Read(container_info.Name, &length, ptr, &tmp_time_step, true);
    if(container_info.nComp == 1)
    {
        FV14Writer::WriteScalar(out, ptr, length);
    }else if(container_info.nComp == 3){
        if(container_info.VectorOrder == PDMlib::NIJK)
        {
            FV14Writer::WriteVectorNIJK(out, ptr, length);
        }else if(container_info.VectorOrder == PDMlib::IJKN){
            FV14Writer::WriteVectorIJKN(out, ptr, length);
        }
    }
    delete[] *ptr;
}

void ReadAndWriteContainerSelector(std::ofstream& out, PDMlib::ContainerInfo container_info, const int& time_step)
{
    if(container_info.Type == PDMlib::INT32)
    {
        int* ptr = NULL;
        ReadAndWriteContainer(out, &ptr, container_info, time_step);
    }else if(container_info.Type == PDMlib::uINT32){
        unsigned int* ptr = NULL;
        ReadAndWriteContainer(out, &ptr, container_info, time_step);
    }else if(container_info.Type == PDMlib::INT64){
        long* ptr = NULL;
        ReadAndWriteContainer(out, &ptr, container_info, time_step);
    }else if(container_info.Type == PDMlib::uINT64){
        unsigned long* ptr = NULL;
        ReadAndWriteContainer(out, &ptr, container_info, time_step);
    }else if(container_info.Type == PDMlib::FLOAT){
        float* ptr = NULL;
        ReadAndWriteContainer(out, &ptr, container_info, time_step);
    }else if(container_info.Type == PDMlib::DOUBLE){
        double* ptr = NULL;
        ReadAndWriteContainer(out, &ptr, container_info, time_step);
    }
}
}
int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int start_time = -1;
    int end_time   = INT_MAX;
    std::string dir_name("./");
    std::string dfi_filename;
    std::string coordinate("Coordinate");
    bool with_bbox = false;
    ComandLineParser(argc, argv, &start_time, &end_time, &dir_name, &dfi_filename, &coordinate, &with_bbox);
    std::ifstream ifs(dfi_filename.c_str());
    if(ifs.fail())
    {
        std::cerr<<"meta data file not found! ("<<dfi_filename<<")"<<std::endl;
        PrintUsageAndAbort(argv[0]);
    }

    PDMlib::PDMlib& pdmlib = PDMlib::PDMlib::GetInstance();
    pdmlib.Init(argc, argv, "FILE_CONVERTER_DUMMY", dfi_filename);

    // メタデータファイルからコンテナ情報を読み込む
    std::vector<PDMlib::ContainerInfo>& containers = pdmlib.GetContainerInfo();

    // メタデータファイルから読み込んだコンテナ情報を元に出力変数の名前のリストを作成（座標は除く）
    std::vector<std::string> container_names;
    bool coord_found = false;
    PDMlib::ContainerInfo    coord_container;
    for(std::vector<PDMlib::ContainerInfo>::iterator it = containers.begin(); it != containers.end();)
    {
        if((*it).Name != coordinate)
        {
            if((*it).nComp == 1)
            {
                container_names.push_back((*it).Name);
            }else if((*it).nComp == 3){
                // FV14のParticlePath形式では、ベクトル量は扱えないので、個々にスカラー値として出力する
                container_names.push_back((*it).Name+"_x");
                container_names.push_back((*it).Name+"_y");
                container_names.push_back((*it).Name+"_z");
            }
            ++it;
        }else{
            coord_found     = true;
            coord_container = *it;
            it              = containers.erase(it);
        }
    }
    if(!coord_found)
    {
        std::cerr<<"Coordinate container not found ! ("<<coordinate<<")"<<std::endl;
        PrintUsageAndAbort(argv[0]);
    }

    // 時間方向でデータ分散
    std::set<int> time_steps;
    std::string   coord_suffix = "*."+coord_container.Suffix;
    PDMlib::MakeTimeStepList(&time_steps, pdmlib.GetBaseFileName(), dir_name, start_time, end_time, coord_suffix);

    int nproc, myrank;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &nproc);
    MPI_Comm_rank(comm, &myrank);

    std::set<int>::iterator my_start_time = time_steps.begin();
    std::set<int>::iterator my_end_time   = time_steps.begin();

    for(int i = 0; i < PDMlib::GetStartIndex(time_steps.size(), nproc, myrank); i++)
    {
        ++my_start_time;
    }
    for(int i = 0; i < PDMlib::GetStartIndex(time_steps.size(), nproc, myrank+1); i++)
    {
        ++my_end_time;
    }

    // タイムステップ毎にPDMlibのフィールドデータファイルを読み込んでfvpファイルを出力する
    for(std::set<int>::iterator it_time_step = my_start_time; it_time_step != my_end_time; ++it_time_step)
    {
        std::string filename;
        filename  = pdmlib.GetBaseFileName();
        filename += "_"+PDMlib::to_string(*it_time_step);
        filename += ".fvp";
        std::ofstream(Out);
        Out.open(filename.c_str(), std::ios::binary);
        int    time_step = *it_time_step;
        // 粒子数を取得するために、座標コンテナを読み込む
        size_t length    = -1;
        if(coord_container.Type == PDMlib::FLOAT)
        {
            float* ptr = NULL;
            PDMlib::PDMlib::GetInstance().Read(coord_container.Name, &length, &ptr, &time_step, true);
            delete[] ptr;
        }else if(coord_container.Type == PDMlib::DOUBLE){
            double* ptr = NULL;
            PDMlib::PDMlib::GetInstance().Read(coord_container.Name, &length, &ptr, &time_step, true);
            delete[] ptr;
        }
        // ヘッダ部を出力
        FV14Writer::WriteHeader(Out, container_names, length/coord_container.nComp);
        //座標データの出力
        ReadAndWriteCoordinateSelector(Out, coord_container, time_step);

        // 座標以外のデータ出力
        for(std::vector<PDMlib::ContainerInfo>::iterator it = containers.begin(); it != containers.end(); ++it)
        {
            ReadAndWriteContainerSelector(Out, *it, time_step);
        }

        // 解析領域全体のbounding boxをFV-unsとして出力
        if(with_bbox)
        {
            double bbox[6];
            pdmlib.GetBoundingBox(bbox);
            filename  = pdmlib.GetBaseFileName();
            filename += "_"+PDMlib::to_string(*it_time_step);
            filename += ".uns";
            FV14Writer::WriteBoundingBox(bbox, filename);
        }
    }

    MPI_Finalize();

    return 0;
}