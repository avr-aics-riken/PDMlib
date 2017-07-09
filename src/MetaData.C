/*
###################################################################################
#
# PDMlib - Particle Data Management library
#
# Copyright (c) 2014-2017 Advanced Institute for Computational Science(AICS), RIKEN.
# All rights reserved.
#
# Copyright (c) 2017 Research Institute for Information Technology (RIIT), Kyushu University.
# All rights reserved.
#
###################################################################################
*/

#include "PDMlib.h"
#include "TextParser.h"

#include <dirent.h>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <list>
#include <string>
#include "Utility.h"
#include "MetaData.h"
#include "TPWriteHelper.h"
#include "Utility.h"

namespace PDMlib
{
MetaData::~MetaData()
{
    if(HeaderOutput)
    {
        std::ofstream out(FileName.c_str(), std::ios::out|std::ios::app);
        TPWriteHelper TpHelper;
        TpHelper.write_rbrace(out);
    }
}

int MetaData::Read()
{
    TextParser tp;
    if(tp.read_local(FileName) != 0)
    {
        std::cerr<<"MetaData read failed!! ("<<FileName<<")"<<std::endl;
    }

    std::string tp_value;
    int ierr;
    tp.getValue("/DomainInfo/BoundingBox", tp_value);
    std::vector<std::string> strBbox;
    tp.splitVector(tp_value, strBbox);
    for(int i = 0; i < 6; i++)
    {
        BoundingBox[i] = stod_wrapper(strBbox[i]);
    }

    tp.getValue("/Header/Version",             Version);
    tp.getValue("/Header/Endian",              Endian);
    tp.getValue("/Header/Prefix",        Prefix);
    tp.getValue("/Header/DirectoryPath",       DirectoryPath);
    tp.getValue("/Header/FieldFilenameFormat", FieldFilenameFormat);
    tp.getValue("/MPI/NumProc",                tp_value);
    NumProc = tp.convertInt(tp_value, &ierr);
    tp.getValue("/Header/NumContainer", tp_value);
    int NumContainer = tp.convertInt(tp_value, &ierr);

    {
        tp.changeNode("ContainerList");
        std::vector<std::string> labels;
        tp.getNodes(labels, 2);
        if(labels.size() != NumContainer)
        {
            std::cerr<<"NumContainer = "<<NumContainer<<" Actual Number of Containers = "<<labels.size()<<std::endl;
            NumContainer = labels.size();
        }

        for(std::vector<std::string>::iterator it = labels.begin(); it != labels.end(); ++it)
        {
            std::string Name = *it;

            std::string Annotation;
            tp.getValue(Name+"/Annotation",  Annotation);

            std::string Compression;
            tp.getValue(Name+"/Compression", Compression);

            tp.getValue(Name+"/Type",        tp_value);
            SupportedType Type = string2enumType(tp_value);

            std::string   Suffix;
            tp.getValue(Name+"/Suffix",      Suffix);

            tp.getValue(Name+"/nComp",       tp_value);
            int nComp = tp.convertInt(tp_value, &ierr);

            tp.getValue(Name+"/VectorOrder", tp_value);
            StorageOrder  VectorOrder = string2enumStorageOrder(tp_value);

            ContainerInfo tmp         = {Name, Annotation, Compression, Type, Suffix, nComp, VectorOrder};
            AddContainer(tmp);
        }
    }

    tp.changeNode("../Header");
    std::vector<std::string> labels;
    tp.getNodes(labels, 2);
    if(std::find(labels.begin(), labels.end(), "UnitList") != labels.end())
    {
        tp.changeNode("UnitList");
        std::vector<std::string> labels;
        tp.getNodes(labels, 2);
        for(std::vector<std::string>::iterator it = labels.begin(); it != labels.end(); ++it)
        {
            std::string Name = *it;

            std::string Unit;
            tp.getValue(Name+"/Unit",       Unit);

            tp.getValue(Name+"/reference",  tp_value);
            double reference = tp.convertDouble(tp_value, &ierr);

            tp.getValue(Name+"/difference", tp_value);
            double difference = tp.convertDouble(tp_value, &ierr);

            tp.getValue(Name+"/BsetDiff",   tp_value);
            bool     BsetDiff = tp.convertBool(tp_value, &ierr);

            UnitElem tmp      = {Name, Unit, reference, difference, BsetDiff};
            AddUnit(tmp);
        }
    }
    tp.remove();

    //pathにあるフィールドデータのうち最新のステップのものを探してTimeStepに代入
    DIR* dp;
    std::string dirname(GetPath());
    if((dp = opendir(dirname.c_str())) == NULL)
    {
        std::cerr<<"Couldn't open "<<dirname<<std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    struct dirent* entry;
    std::list<std::string> timesteps;
    entry = readdir(dp);
    while(entry != NULL)
    {
        std::string filename(entry->d_name);
        if(filename.find(Prefix) != std::string::npos)
        {
            //後ろから順にピリオドまでを削除
            //後ろから順に_までを取り出す
            timesteps.push_back(dirname+filename);
        }
        entry = readdir(dp);
    }
    closedir(dp);
    timesteps.sort();
    timesteps.unique();

    TimeStep = stoi_wrapper(timesteps.back());

    return 0;
}

#define WRITE_VALUE(VAR) TpHelper.write_value(out, #VAR, VAR);
int MetaData::Write()
{
    if(GetMyRank() > 0)return 0;

    SetReadOnly();
    for(std::vector<ContainerInfo>::iterator it = Containers.begin(); it != Containers.end(); ++it)
    {
        Written.insert(make_pair((*it).Name, false));
    }
    std::ofstream out(FileName.c_str());
    TPWriteHelper TpHelper;
    //ヘッダ情報の出力
    TpHelper.write_header(out, "Header");
    WRITE_VALUE(Version);
    WRITE_VALUE(Endian);
    WRITE_VALUE(Prefix);
    WRITE_VALUE(DirectoryPath);
    WRITE_VALUE(FieldFilenameFormat);
    TpHelper.write_value(out, "NumContainer", Containers.size());
    if(Units.size() > 0)
    {
        TpHelper.write_header(out, "UnitList");
        for(std::vector<UnitElem>::iterator it = Units.begin(); it != Units.end(); ++it)
        {
            TpHelper.write_header(out, (*it).Name);
            TpHelper.write_value(out, "Unit",       (*it).Unit);
            TpHelper.write_value(out, "reference",  (*it).reference);
            TpHelper.write_value(out, "difference", (*it).difference);
            TpHelper.write_value(out, "BsetDiff",   (*it).BsetDiff);
            TpHelper.write_rbrace(out);
        }
        TpHelper.write_rbrace(out);
    }
    TpHelper.write_rbrace(out);

    //コンテナ情報の出力
    if(Containers.size() > 0)
    {
        TpHelper.write_header(out, "ContainerList");
        for(std::vector<ContainerInfo>::iterator it = Containers.begin(); it != Containers.end(); ++it)
        {
            TpHelper.write_header(out, (*it).Name);
            TpHelper.write_value(out, "Annotation",  (*it).Annotation);
            TpHelper.write_value(out, "Compression", (*it).Compression);
            TpHelper.write_value(out, "Type",        enumType2string((*it).Type));
            TpHelper.write_value(out, "Suffix",      (*it).Suffix);
            TpHelper.write_value(out, "nComp",       (*it).nComp);
            std::string VectorOrder = (*it).VectorOrder == NIJK ? "NIJK" : "IJKN";
            TpHelper.write_value(out, "VectorOrder", VectorOrder);
            TpHelper.write_rbrace(out);
        }
        TpHelper.write_rbrace(out);
    }

    //MPI情報の出力
    TpHelper.write_header(out, "MPI");
    WRITE_VALUE(NumCommWorldProc);
    WRITE_VALUE(Communicator);
    WRITE_VALUE(NumProc);
    TpHelper.write_rbrace(out);

    //計算領域情報の出力
    TpHelper.write_header(out, "DomainInfo");
    WRITE_VALUE(NumProc);
    TpHelper.write_vector(out, "GlobalOrigin", BoundingBox,  3);
    double GlobalRegion[3] = {BoundingBox[3]-BoundingBox[0], BoundingBox[4]-BoundingBox[1], BoundingBox[5]-BoundingBox[2]};
    TpHelper.write_vector(out, "GlobalRegion", GlobalRegion, 3);
    TpHelper.write_vector(out, "BoundingBox",  BoundingBox,  6);
    TpHelper.write_rbrace(out);
    return 0;
}

#undef WRITE_VALUE

template<typename T>
int MetaData::WriteTimeSlice(const int& TimeStep, const double& Time, T* MinMax, const int& ContainerLength, const std::string& Name)
{
    if(GetMyRank() > 0)return 0;

    if(Written[Name])return -1;

    std::ofstream out(FileName.c_str(), std::ios::out|std::ios::app);
    TPWriteHelper TpHelper;
    if(!HeaderOutput)
    {
        TpHelper.write_header(out, "TimeSlice");
        HeaderOutput = true;
    }
    //このタイムステップでの最初の呼び出しの時は、ヘッダとタイムステップなどを出力する
    if(is_never_written())
    {
        TpHelper.write_header(out, "Slice[@]");
        this->TimeStep    = TimeStep;
        this->Time        = Time;
        this->NumParticle = ContainerLength;
    }

    ContainerInfo container;
    GetContainerInfo(Name, &container);

    if(MinMax != NULL)
    {
        if(container.nComp == 1)
        {
            TpHelper.write_header(out, container.Name+"MinMax[@]");
            TpHelper.write_value(out, "Min", MinMax[0]);
            TpHelper.write_value(out, "Max", MinMax[1]);
            TpHelper.write_rbrace(out);
        }else{
            TpHelper.write_header(out, container.Name+"VectorMinMax");
            TpHelper.write_value(out, "Min", MinMax[0]);
            TpHelper.write_value(out, "Max", MinMax[1]);
            TpHelper.write_rbrace(out);
            TpHelper.write_header(out, container.Name+"MinMax[@]");
            TpHelper.write_value(out, "Min", MinMax[2]);
            TpHelper.write_value(out, "Max", MinMax[3]);
            TpHelper.write_rbrace(out);
            TpHelper.write_header(out, container.Name+"MinMax[@]");
            TpHelper.write_value(out, "Min", MinMax[4]);
            TpHelper.write_value(out, "Max", MinMax[5]);
            TpHelper.write_rbrace(out);
            TpHelper.write_header(out, container.Name+"MinMax[@]");
            TpHelper.write_value(out, "Min", MinMax[6]);
            TpHelper.write_value(out, "Max", MinMax[7]);
            TpHelper.write_rbrace(out);
        }
    }
    Written[Name] = true;

    //全コンテナのタイムスライス情報が出力されていたら
    //Written 閉括弧を出力してWrittenの全要素をfalseにする
    if(is_all_written())
    {
        TpHelper.write_rbrace(out);
        for(std::map<std::string, bool>::iterator it = Written.begin(); it != Written.end(); ++it)
        {
            (*it).second = false;
        }
        NumOutputTimeSlice++;
    }
    //! @attention "TimeSlice" の"{"に対応する"}"はwMetaDataのデストラクタ内で出力するので
    //! TimeSlice出力を追加する場合は整合性に気をつけること
    return 0;
}

std::string MetaData::GetEndian(void) const
{
    static bool first_call = true;
    static std::string endian;
    if(first_call)
    {
        if(is_little())
        {
            endian = "little";
        }else if(is_big()){
            endian = "big";
        }else{
            endian = "unknown";
        }
        first_call = false;
    }
    return endian;
}

bool MetaData::GetContainerInfo(const std::string& Name, ContainerInfo* container_info) const
{
    bool rt = false;
    for(std::vector<ContainerInfo>::const_iterator it = Containers.begin(); it != Containers.end(); ++it)
    {
        if((*it).Name == Name)
        {
            rt              = true;
            *container_info = *it;
            break;
        }
    }
    return rt;
}

bool MetaData::FindContainerInfo(const std::string& name) const
{
    ContainerInfo tmp;
    return GetContainerInfo(name, &tmp);
}

bool MetaData::Compare(const MetaData& lhs) const
{
    if(Version != lhs.Version)
    {
        std::cerr<<"Version is differ"<<std::endl;
        return false;
    }
    if(Endian != lhs.Endian)
    {
        std::cerr<<"Endian is differ"<<std::endl;
        return false;
    }
    if(NumCommWorldProc != lhs.NumCommWorldProc)
    {
        std::cerr<<"NumCommWorldProc is differ"<<std::endl;
        return false;
    }
    if(NumProc != lhs.NumProc)
    {
        std::cerr<<"NumProc is differ"<<std::endl;
        return false;
    }
    if(Communicator != lhs.Communicator)
    {
        std::cerr<<"Communiucator is differ"<<std::endl;
        return false;
    }
    if(FileName != lhs.FileName)
    {
        std::cerr<<"FileName is differ"<<std::endl;
        return false;
    }
    if(Prefix != lhs.Prefix)
    {
        std::cerr<<"Prefix is differ"<<std::endl;
        return false;
    }
    if(DirectoryPath != lhs.DirectoryPath)
    {
        std::cerr<<"DirectoryPath is differ"<<std::endl;
        return false;
    }
    if(GetNumContainers() != lhs.GetNumContainers())
    {
        std::cerr<<"Number of Containers is differ"<<std::endl;
        return false;
    }

    std::vector<ContainerInfo> org_container(Containers);
    std::vector<ContainerInfo> lhs_container(lhs.Containers);
    std::sort(org_container.begin(), org_container.end(), ContainerInfoSorter());
    std::sort(lhs_container.begin(), lhs_container.end(), ContainerInfoSorter());
    for(int i = 0; i < GetNumContainers(); i++)
    {
        if(org_container[i].Name != (lhs_container)[i].Name)
        {
            std::cerr<<i<<" th Container's Name is differ"<<std::endl;
            return false;
        }
        if(org_container[i].Annotation != (lhs_container)[i].Annotation)
        {
            std::cerr<<i<<" th Container's Name is differ"<<std::endl;
            return false;
        }
        if(org_container[i].Compression != (lhs_container)[i].Compression)
        {
            std::cerr<<i<<" th Container's Compression is differ"<<std::endl;
            return false;
        }
        if(org_container[i].Type != (lhs_container)[i].Type)
        {
            std::cerr<<i<<" th Container's Type is differ"<<std::endl;
            return false;
        }
        if(org_container[i].Suffix != (lhs_container)[i].Suffix)
        {
            std::cerr<<i<<" th Container's Suffix is differ"<<std::endl;
            return false;
        }
        if(org_container[i].nComp != (lhs_container)[i].nComp)
        {
            std::cerr<<i<<" th Container's nComp is differ"<<std::endl;
            return false;
        }
        if(org_container[i].VectorOrder != (lhs_container)[i].VectorOrder)
        {
            std::cerr<<i<<" th Container's VectorOrder is differ"<<std::endl;
            return false;
        }
    }

    if(Units.size() != lhs.Units.size())
    {
        std::cerr<<"Number of Units is differ"<<std::endl;
        return false;
    }

    std::vector<UnitElem> org_unit(Units);
    std::vector<UnitElem> lhs_unit(lhs.Units);
    std::sort(org_unit.begin(), org_unit.end(), UnitElemSorter());
    std::sort(lhs_unit.begin(), lhs_unit.end(), UnitElemSorter());

    for(int i = 0; i < Units.size(); i++)
    {
        if(Units[i].Name != (lhs.Units)[i].Name)
        {
            std::cerr<<i<<" th Unit's Name is differ"<<std::endl;
            return false;
        }
        if(Units[i].Unit != (lhs.Units)[i].Unit)
        {
            std::cerr<<i<<" th Unit's Unit is differ"<<std::endl;
            return false;
        }
        if(Units[i].reference != (lhs.Units)[i].reference)
        {
            std::cerr<<i<<" th Unit's reference is differ"<<std::endl;
            return false;
        }

        if(Units[i].difference != (lhs.Units)[i].difference)
        {
            std::cerr<<i<<" th Unit's difference is differ"<<std::endl;
            return false;
        }
        if(Units[i].BsetDiff != (lhs.Units)[i].BsetDiff)
        {
            std::cerr<<i<<" th Unit's BsetDiff is differ"<<std::endl;
            return false;
        }
    }

    return true;
}
void MetaData::MakeTimeStepList(std::set<int>* time_steps, const int& start_time, const int& end_time, const std::string& wild_card) const
{
    std::vector<std::string> filenames;
    ListDirectoryContents(GetPath(), &filenames, wild_card);
    for(std::vector<std::string>::iterator it = filenames.begin(); it != filenames.end(); ++it)
    {
        if((*it).find(GetBaseFileName()) != std::string::npos)
        {
            int time_step = get_time_step(*it, is_rank_step());
            if(time_step >= 0)
            {
                if(start_time <= time_step && time_step <= end_time)
                {
                    time_steps->insert(time_step);
                }
            }
        }
    }
}

void MetaData::GetFileName(std::string* filename, const std::string& name, const int my_rank, const int& time_step) const
{
    *filename  = GetPath();
    *filename += "/"; //path separator
    *filename += GetBaseFileName();
    if(is_rank_step())
    {
      *filename += "_"+to_string(my_rank);
      *filename += "_"+to_string(time_step);
    }else{
      *filename += "_"+to_string(time_step);
      *filename += "_"+to_string(my_rank);
    }
    ContainerInfo container_info;
    GetContainerInfo(name, &container_info);
    *filename += "."+container_info.Suffix;
}

template int MetaData::WriteTimeSlice(const int& TimeStep, const double& Time, int* MinMax, const int& ContainerLength, const std::string& Name);
template int MetaData::WriteTimeSlice(const int& TimeStep, const double& Time, unsigned int* MinMax, const int& ContainerLength, const std::string& Name);
template int MetaData::WriteTimeSlice(const int& TimeStep, const double& Time, long* MinMax, const int& ContainerLength, const std::string& Name);
template int MetaData::WriteTimeSlice(const int& TimeStep, const double& Time, unsigned long* MinMax, const int& ContainerLength, const std::string& Name);
template int MetaData::WriteTimeSlice(const int& TimeStep, const double& Time, float* MinMax, const int& ContainerLength, const std::string& Name);
template int MetaData::WriteTimeSlice(const int& TimeStep, const double& Time, double* MinMax, const int& ContainerLength, const std::string& Name);
} //end of namespcae
