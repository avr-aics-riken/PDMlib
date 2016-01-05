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
#include <list>
#include <utility>
#include <vector>
#include <climits>
#include <unistd.h>
#include <sstream>
#include "PDMlib.h"
#include "Utility.h"
#include "VtkWriter.h"

namespace
{
  void PrintUsageAndAbort(const char* cmd)
  {
    std::cerr<<"usage: "<<cmd<<" meta_data_file {-c Coordinate Container name} {-s start time} {-e end time} {-f format} "<<std::endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
  }

  void ComandLineParser(const int& argc, char** argv, int* start, int* end, std::string* dfi_filename, std::string* coordinate, std::string* format)
  {
    int results = 0;
    while((results = getopt(argc, argv, "s:e:c:f:")) != -1)
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

        case 'f':
          *format= optarg;
          break;

        case '?':
          PrintUsageAndAbort(argv[0]);
          break;
      }
    }
    if(optind>=argc)
    {
      PrintUsageAndAbort(argv[0]);
    }
    *dfi_filename=argv[optind];
  }

  template <typename T>
  void ReadWriteVtk(VtkWriter::PolyData* PD, PDMlib::ContainerInfo& container_info, int& time_step, const std::string& format)
  {
    size_t length=-1;
    T* ptr=NULL;
    PDMlib::PDMlib::GetInstance().Read(container_info.Name, &length, &ptr, &time_step, true);
    PD->WriteDataArray(container_info.Name, container_info.nComp, length/container_info.nComp, ptr, format);
    delete [] ptr;
  }

  void ReadWriteVtkHelper(VtkWriter::PolyData* PD, PDMlib::ContainerInfo& container_info, int& time_step, const std::string& format)
  {
    if(container_info.Type == PDMlib::FLOAT)
    {
      ReadWriteVtk<float>(PD, container_info,time_step, format);
    }else if(container_info.Type == PDMlib::DOUBLE){
      ReadWriteVtk<double>(PD, container_info,time_step, format);
    }else if(container_info.Type == PDMlib::INT32){
      ReadWriteVtk<int>(PD, container_info,time_step, format);
    }else if(container_info.Type == PDMlib::INT64){
      ReadWriteVtk<long>(PD, container_info,time_step, format);
    }else if(container_info.Type == PDMlib::uINT32){
      ReadWriteVtk<unsigned int>(PD, container_info,time_step, format);
    }else if(container_info.Type == PDMlib::uINT64){
      ReadWriteVtk<unsigned long>(PD, container_info,time_step, format);
    }
  }
}

int main(int argc, char* argv[])
{
  MPI_Init(&argc, &argv);
  int nproc, myrank;
  MPI_Comm comm = MPI_COMM_WORLD;
  MPI_Comm_size(comm, &nproc);
  MPI_Comm_rank(comm, &myrank);

  int start_time = -1;
  int end_time   = INT_MAX;
  std::string dfi_filename;
  std::string coordinate("Coordinate");
  std::string format("ascii");
  ComandLineParser(argc, argv, &start_time, &end_time, &dfi_filename, &coordinate, &format);
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

  // 座標コンテナを探し出す
  bool coord_found = false;
  PDMlib::ContainerInfo    coord_container;
  for(std::vector<PDMlib::ContainerInfo>::iterator it = containers.begin(); it != containers.end();)
  {
    if((*it).Name != coordinate)
    {
      ++it;
    }else{
      coord_found     = true;
      coord_container = *it;
      break;
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
  pdmlib.MakeTimeStepList(&time_steps, start_time, end_time, coord_suffix);

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

  // タイムステップ毎にPDMlibのフィールドデータファイルを読み込んでvtpファイルを出力する
  for(std::set<int>::iterator it_time_step = my_start_time; it_time_step != my_end_time; ++it_time_step)
  {
    std::string filename;
    filename  = pdmlib.GetBaseFileName();
    filename += "_"+PDMlib::to_string(*it_time_step);
    filename += ".vtp";
    std::cerr<< "writing "<<filename<<std::endl;
    int    time_step = *it_time_step;
    // 粒子数を取得するために、座標コンテナを読み込む
    size_t length    = -1;
    VtkWriter::PolyData* PD;

    size_t num_particle=0;
    if(coord_container.Type == PDMlib::FLOAT)
    {
      float* ptr = NULL;
      PDMlib::PDMlib::GetInstance().Read(coord_container.Name, &length, &ptr, &time_step, true);
      num_particle=length/coord_container.nComp;
      PD=new VtkWriter::PolyData(filename, num_particle, num_particle);
      PD->WritePoints(ptr, format);
      delete[] ptr;
    }else if(coord_container.Type == PDMlib::DOUBLE){
      double* ptr = NULL;
      PDMlib::PDMlib::GetInstance().Read(coord_container.Name, &length, &ptr, &time_step, true);
      num_particle=length/coord_container.nComp;
      PD=new VtkWriter::PolyData(filename, num_particle, num_particle);
      PD->WritePoints(ptr, format);
      delete[] ptr;
    }
    PD->WriteAllPointsAsVerts(format);

    PD->WriteStartTag("PointData");
    //出力するデータ数ぶん繰り返し
    for(std::vector<PDMlib::ContainerInfo>::iterator it = containers.begin(); it != containers.end();++it)
    {
      ReadWriteVtkHelper(PD, *it, time_step, format);
    }
    PD->WriteEndTag("PointData");
    delete PD;
  }
  MPI_Finalize();
  return 0;
}
