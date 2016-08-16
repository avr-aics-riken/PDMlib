/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */

#include "PDMlib.h"

#include <dirent.h>
#include <typeinfo>
#include <string>
#include <vector>
#include <list>
#include <set>
#include <algorithm>

#include "PDMlibImpl.h"
#include "Write.h"
#include "Read.h"
#include "Utility.h"


//! @file PDMlibのコンストラクタ/デストラクタ/publicメソッドの実装
namespace PDMlib
{
PDMlib::PDMlib() : pImpl(new PDMlib::Impl()){}

PDMlib::~PDMlib()
{
    delete pImpl;
    pImpl = NULL;
}

void PDMlib::Init(const int& argc, char** argv, const std::string& WriteMetaDataFile, const std::string& ReadMetaDataFile)
{
    pImpl->Init(argc, argv, WriteMetaDataFile, ReadMetaDataFile);
}

template<typename T>
int PDMlib::Read(const std::string& Name, size_t* ContainerLength, T** Container, int* TimeStep, bool read_all_files)
{
    if(!pImpl->Initialized)
    {
        std::cerr<<"PDMlib::Read() called before Init()"<<std::endl;
        return -1;
    }
    if(!pImpl->rMetaData->FindContainerInfo(Name))
    {
        std::cerr<<"PDMlib::Read(): "<<Name<<" is not found in MetaDataFile "<<std::endl;
        return -2;
    }
    if(!pImpl->TypeCheck(Name, Container))
    {
        std::cerr<<"PDMlib::Read(): Container Data type mismatch ("<<Name<<")"<<std::endl;
        return -3;
    }

    std::set<int> time_steps;
    pImpl->MakeTimeStep(&time_steps);
    pImpl->DetermineTimeStep(TimeStep, time_steps);

    int& time_step = *TimeStep;

    std::vector<std::string> filenames;
    pImpl->MakeFilenameList(&filenames, time_step, Name, read_all_files);
    if(filenames.empty())
    {
        *ContainerLength = 0;
        return *ContainerLength;
    }

    size_t total_size = 0;
    std::vector<std::pair<size_t, char*> > buffers;
    pImpl->Read(Name, filenames, &buffers, &total_size);
    pImpl->AllocateContainer(total_size, *ContainerLength, Container);
    pImpl->CopyBufferToContainer(buffers, ContainerLength, *Container);
    return *ContainerLength;
}

template<typename T>
int PDMlib::RegisterContainer(const std::string& Name, T** Container) const
{
    if(!pImpl->Initialized)
    {
        std::cerr<<"PDMlib::RegisterContainer() called before Init()"<<std::endl;
        return -1;
    }
    if(!pImpl->TypeCheck(Name, Container))
    {
        std::cerr<<"PDMlib::RegisterContainer(): Container Data type mismatch ("<<Name<<")"<<std::endl;
        return -3;
    }

    ContainerInfo container_info;
    pImpl->rMetaData->GetContainerInfo(Name, &container_info);

    ContainerPointer* tmp = new ContainerPointer;
    tmp->Name            = container_info.Name;
    tmp->Type            = container_info.Type;
    tmp->ContainerLength = 0;
    tmp->Container       = (void**)Container;
    tmp->size            = 0;
    tmp->buff            = NULL;
    tmp->nComp           = container_info.nComp;

    std::pair<std::set<ContainerPointer*>::iterator, bool> result = (pImpl->ContainerTable).insert(tmp);
    if(!result.second)return -2;

    return 0;
}

size_t PDMlib::ReadAll(int* TimeStep, const bool& MigrationFlag, const std::string& CoordinateContainerName)
{
    if(!pImpl->Initialized)
    {
        std::cerr<<"PDMlib::ReadAll() called before Init()"<<std::endl;
        return -1;
    }

    std::set<int> time_steps;
    pImpl->MakeTimeStep(&time_steps);
    if(time_steps.size() <1) return 0;
    pImpl->DetermineTimeStep(TimeStep, time_steps);
    int& time_step = *TimeStep;

    for(std::set<ContainerPointer*>::iterator it = pImpl->ContainerTable.begin(); it != pImpl->ContainerTable.end(); ++it)
    {
        std::vector<std::string> filenames;
        pImpl->MakeFilenameList(&filenames, time_step, (*it)->Name);
        size_t total_size = 0;
        std::vector<std::pair<size_t, char*> > buffers;
        pImpl->Read((*it)->Name, filenames, &buffers, &total_size);
        (*it)->size = total_size;
        size_t written_size = 0;
        if(total_size > 0)
        {
            (*it)->buff = new char[total_size];
            // buffersに入っている複数のポインタを1つにまとめる
            for(std::vector<std::pair<size_t, char*> >::iterator it_buff = buffers.begin(); it_buff != buffers.end(); ++it_buff)
            {
                memcpy((*it)->buff+written_size, (*it_buff).second, (*it_buff).first);
                delete (*it_buff).second;
                written_size += (*it_buff).first;
            }
        }else{
            (*it)->buff = NULL;
        }
        (*it)->size            = written_size;
        (*it)->ContainerLength = written_size/GetSize((*it)->Type);
        if((*it)->Name == CoordinateContainerName)
        {
            pImpl->CoordinateContainer = *it;
        }
    }

    // ここまでで、ContainerPointer::buff に全データがある状態
    if(MigrationFlag)
    {
        if(!pImpl->Migrate())
        {
            std::cerr<<"Migration failed!"<<std::endl;
        }
    }

    // ContainerPointer::buffからContainerPointer::Containerへコピー
    for(std::set<ContainerPointer*>::iterator it = pImpl->ContainerTable.begin(); it != pImpl->ContainerTable.end(); ++it)
    {
        if((*it)->Type == INT32)
        {
            pImpl->AllocateContainer((*it)->size, (*it)->ContainerLength, (int**)((*it)->Container));
            (*it)->ContainerLength = pImpl->CopyBufferToContainer((int*)*((*it)->Container), (*it)->buff, (*it)->size);
        }else if((*it)->Type == uINT32){
            pImpl->AllocateContainer((*it)->size, (*it)->ContainerLength, (unsigned int**)((*it)->Container));
            (*it)->ContainerLength = pImpl->CopyBufferToContainer((unsigned int*)*((*it)->Container), (*it)->buff, (*it)->size);
        }else if((*it)->Type == INT64){
            pImpl->AllocateContainer((*it)->size, (*it)->ContainerLength, (long**)((*it)->Container));
            (*it)->ContainerLength = pImpl->CopyBufferToContainer((long*)*((*it)->Container), (*it)->buff, (*it)->size);
        }else if((*it)->Type == uINT64){
            pImpl->AllocateContainer((*it)->size, (*it)->ContainerLength, (unsigned long**)((*it)->Container));
            (*it)->ContainerLength = pImpl->CopyBufferToContainer((unsigned long*)*((*it)->Container), (*it)->buff, (*it)->size);
        }else if((*it)->Type == FLOAT){
            pImpl->AllocateContainer((*it)->size, (*it)->ContainerLength, (float**)((*it)->Container));
            (*it)->ContainerLength = pImpl->CopyBufferToContainer((float*)*((*it)->Container), (*it)->buff, (*it)->size);
        }else if((*it)->Type == DOUBLE){
            pImpl->AllocateContainer((*it)->size, (*it)->ContainerLength, (double**)((*it)->Container));
            (*it)->ContainerLength = pImpl->CopyBufferToContainer((double*)*((*it)->Container), (*it)->buff, (*it)->size);
        }
    }

    ContainerPointer* container_pointer = *(pImpl->ContainerTable.begin());

    return container_pointer->ContainerLength/container_pointer->nComp;
}

template<typename T>
int PDMlib::Write(const std::string& Name, const size_t& ContainerLength, T* Container, T MinMax[8], const int& NumComp, const int& TimeStep, const double& Time)
{
    if(!pImpl->Initialized)
    {
        std::cerr<<"PDMlib::Write() called before Init()"<<std::endl;
        return -1;
    }
    if(!pImpl->wMetaData->FindContainerInfo(Name))
    {
        std::cerr<<"PDMlib::Write(): "<<Name<<" is not found in MetaDataFile"<<std::endl;
        return -2;
    }
    if(Container == NULL)
    {
        std::cerr<<"PDMlib::Write(): Null pointer is passed as Container pointer of pointer"<<std::endl;
        return -4;
    }
    if((NumComp != 1) && (NumComp != 3))
    {
        std::cerr<<"PDMlib::Write(): NumComp must be 1 or 3 ("<<NumComp<<")"<<std::endl;
        return -5;
    }
    if(TimeStep < 0)
    {
        std::cerr<<"PDMlib::Write(): TimeStep must be positive number ("<<TimeStep<<")"<<std::endl;
        return -6;
    }

    if(pImpl->FirstCall)
    {
        pImpl->wMetaData->SetReadOnly();
        pImpl->wMetaData->Write();
        pImpl->FirstCall = false;

        // create directory by atomic operation in MPI world
        for ( int i=0; i<pImpl->wMetaData->GetNumProc(); i++)
        {
          MPI_Barrier(pImpl->wMetaData->GetComm());
          if(i != pImpl->wMetaData->GetMyRank()) continue;
          if(!RecursiveMkdir(pImpl->wMetaData->GetPath()))
          {
            std::cerr<<"mkdir faild! field data will be output to current directory!"<<std::endl;
            pImpl->wMetaData->SetPath("./");
          }
        }
    }
    //タイムスライス情報の出力
    pImpl->wMetaData->WriteTimeSlice(TimeStep, Time, MinMax, ContainerLength, Name);
    std::string filename;
    pImpl->wMetaData->GetFileName(&filename, Name, pImpl->wMetaData->GetMyRank(), TimeStep);

    //出力するデータサイズが0の時はタイムスライスだけ出力して終了
    if(ContainerLength == 0)
    {
      return 0;
    }

    //フィールドデータの出力
    ContainerInfo  container_info;
    pImpl->wMetaData->GetContainerInfo(Name, &container_info);
    BaseIO::Write* writer = BaseIO::WriteFactory::create(container_info.Compression, enumType2string(container_info.Type), container_info.nComp);
    int write_size        = writer->write(filename.c_str(), ContainerLength*NumComp*sizeof(T), ContainerLength*NumComp*sizeof(T), (char*)Container);
    delete writer;
    return write_size;
}

int PDMlib::AddContainer(const ContainerInfo& Container)
{
    if(!pImpl->Initialized)
    {
        std::cerr<<"PDMlib::AddContainer() called before Init()"<<std::endl;
        return -1;
    }
    pImpl->wMetaData->AddContainer(Container);
    return 0;
}

int PDMlib::SetBaseFileName(const std::string& FileName)
{
    if(!pImpl->Initialized)
    {
        std::cerr<<"PDMlib::SetBaseFileName() called before Init()"<<std::endl;
        return -1;
    }
    pImpl->wMetaData->SetBaseFileName(FileName);
    return 0;
}

int PDMlib::SetFileNameFormat(const std::string& format)
{
    if(!pImpl->Initialized)
    {
        std::cerr<<"PDMlib::SetFileNameFormat() called before Init()"<<std::endl;
        return -1;
    }
    pImpl->wMetaData->SetFileNameFormat(format);
    return 0;
}

int PDMlib::SetPath(const std::string& path)
{
    if(!pImpl->Initialized)
    {
        std::cerr<<"PDMlib::SetPath() called before Init()"<<std::endl;
        return -1;
    }
    pImpl->wMetaData->SetPath(path);
    return 0;
}

int PDMlib::MakeTimeStepList(std::set<int>* time_steps, const int& start_time, const int& end_time, const std::string& wild_card) const
{
    if(!pImpl->Initialized)
    {
        std::cerr<<"PDMlib::MakeTimeStepList() called before Init()"<<std::endl;
        return -1;
    }
  pImpl->rMetaData->MakeTimeStepList(time_steps, start_time, end_time, wild_card);
  return 0;
}

std::vector<ContainerInfo>& PDMlib::GetContainerInfo(void)
{
    static std::vector<ContainerInfo> container_info;
    if(!pImpl->Initialized)
    {
        std::cerr<<"PDMlib::GetContainerInfo() called before Init()"<<std::endl;
    }else{
        pImpl->rMetaData->GetContainerInfo(container_info);
    }
    return container_info;
}

void PDMlib::SetBufferSize(const int& BufferSize)
{
    pImpl->BufferSize = BufferSize;
}

int PDMlib::GetBufferSize(void)
{
    return pImpl->BufferSize;
}

void PDMlib::SetMaxBufferingTime(const int& MaxBufferingTime)
{
    pImpl->MaxBufferingTime = MaxBufferingTime;
}

int PDMlib::GetMaxBufferingTime(void)
{
    return pImpl->MaxBufferingTime;
}

std::string PDMlib::GetBaseFileName(void)
{
    return pImpl->rMetaData->GetBaseFileName();
}

std::string PDMlib::GetPath(void)
{
    return pImpl->rMetaData->GetPath();
}

void PDMlib::GetBoundingBox(double* bbox)
{
    pImpl->rMetaData->GetBoundingBox(bbox);
}

void PDMlib::SetBoundingBox(double* bbox)
{
    pImpl->wMetaData->SetBoundingBox(bbox);
}

void PDMlib::SetComm(const MPI_Comm& comm)
{
    pImpl->wMetaData->SetComm(comm);
    pImpl->static_my_rank = pImpl->wMetaData->GetMyRank();
    if(pImpl->rMetaData != NULL)pImpl->rMetaData->SetComm(comm);
}

template int PDMlib::Read(const std::string& Name, size_t* ContainerLength, int**           Container, int* TimeStep, bool read_all_files);
template int PDMlib::Read(const std::string& Name, size_t* ContainerLength, unsigned int**  Container, int* TimeStep, bool read_all_files);
template int PDMlib::Read(const std::string& Name, size_t* ContainerLength, long**          Container, int* TimeStep, bool read_all_files);
template int PDMlib::Read(const std::string& Name, size_t* ContainerLength, unsigned long** Container, int* TimeStep, bool read_all_files);
template int PDMlib::Read(const std::string& Name, size_t* ContainerLength, float**         Container, int* TimeStep, bool read_all_files);
template int PDMlib::Read(const std::string& Name, size_t* ContainerLength, double**        Container, int* TimeStep, bool read_all_files);

template int PDMlib::RegisterContainer(const std::string& Name, int**           Container) const;
template int PDMlib::RegisterContainer(const std::string& Name, unsigned int**  Container) const;
template int PDMlib::RegisterContainer(const std::string& Name, long**          Container) const;
template int PDMlib::RegisterContainer(const std::string& Name, unsigned long** Container) const;
template int PDMlib::RegisterContainer(const std::string& Name, float**         Container) const;
template int PDMlib::RegisterContainer(const std::string& Name, double**        Container) const;

template int PDMlib::Write(const std::string& Name, const size_t& ContainerLength, int*           Container, int*           MinMax, const int& NumComp, const int& TimeStep, const double& Time);
template int PDMlib::Write(const std::string& Name, const size_t& ContainerLength, unsigned int*  Container, unsigned int*  MinMax, const int& NumComp, const int& TimeStep, const double& Time);
template int PDMlib::Write(const std::string& Name, const size_t& ContainerLength, long*          Container, long*          MinMax, const int& NumComp, const int& TimeStep, const double& Time);
template int PDMlib::Write(const std::string& Name, const size_t& ContainerLength, unsigned long* Container, unsigned long* MinMax, const int& NumComp, const int& TimeStep, const double& Time);
template int PDMlib::Write(const std::string& Name, const size_t& ContainerLength, float*         Container, float*         MinMax, const int& NumComp, const int& TimeStep, const double& Time);
template int PDMlib::Write(const std::string& Name, const size_t& ContainerLength, double*        Container, double*        MinMax, const int& NumComp, const int& TimeStep, const double& Time);
} //end of namespace
