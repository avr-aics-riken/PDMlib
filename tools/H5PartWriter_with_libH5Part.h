#ifndef H5PartWRITER_H
#define H5PartWRITER_H
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <typeinfo>
#include "H5Part.h"

namespace H5PartWriter
{
//! H5Part形式でファイル出力を行う関数群
class H5PartWriter
{
    H5PartFile* file; 

public:
    H5PartWriter(const std::string& filename)
    {
        file= H5PartOpenFile(filename.c_str(), H5PART_WRITE);
        H5PartWriteFileAttribString(file, "Origin", "Test Strings for H5PartWriteFileAttribString");
    }

    ~H5PartWriter()
    {
    }

    //! スカラー量を出力
    template<typename T>
    void WriteScalar(T** ptr, const size_t& length, std::string& container_name, const int& stride = 1, const int& offset = 0)
    {
        // visit の H5Part readerが32bit整数および符号無し整数に対応していないため、整数型は全てint64で出力する
        // @attention unsigned long(符号無し64bit 整数でしか表現できない値（2^63〜2^64)が入っているとデータが壊れる
        if(typeid(T) == typeid(int) || typeid(T) == typeid(unsigned int) || typeid(T) == typeid(long) || typeid(T) == typeid(unsigned long))
        {
            int64_t* buffer = new int64_t[length];
            for(int i = 0; i < length; i++)
            {
                buffer[i] = (int64_t)((*ptr)[stride*i+offset]);
            }
            H5PartWriteDataInt64(file, container_name.c_str(), buffer);
            delete buffer;
        }else if(typeid(T) == typeid(float)){
            float* buffer = new float[length];
            for(int i = 0; i < length; i++)
            {
                buffer[i] = (float)((*ptr)[stride*i+offset]);
            }
            H5PartWriteDataFloat32(file, container_name.c_str(), buffer);
            delete buffer;
        }else if(typeid(T) == typeid(double)){
            double* buffer = new double[length];
            for(int i = 0; i < length; i++)
            {
                buffer[i] = (double)((*ptr)[stride*i+offset]);
            }

            H5PartWriteDataFloat64(file, container_name.c_str(), buffer);
            delete buffer;
        }else{
            std::cerr<<"unsported type!"<<std::endl;
        }
    }

//! NIJKで格納されているベクトル量を出力
    template<typename T>
    void WriteVectorNIJK(T** ptr, const size_t& length, std::string& container_name)
    {
        size_t      num_particle = length/3;
        std::string name         = container_name+"x";
            WriteScalar(ptr, num_particle, name, 3, 0);
        name = container_name+"y";
            WriteScalar(ptr, num_particle, name, 3, 1);
        name = container_name+"z";
            WriteScalar(ptr, num_particle, name, 3, 2);
    }

//! IJKNで格納されているベクトル量を出力
    template<typename T>
    void WriteVectorIJKN(T** ptr, const size_t& length, std::string& container_name)
    {
        size_t      num_particle = length/3;

        T*          ptr2 = *ptr;
        std::string name = container_name+"x";
        WriteScalar(&ptr2, num_particle, name);

        ptr2+=num_particle;
        name=container_name+"y";
        WriteScalar(&ptr2, num_particle, name);

        ptr2+=num_particle;
        name=container_name+"z";
        WriteScalar(&ptr2, num_particle, name);
    }

    template<typename T>
    void ReadAndWriteContainer(T** ptr, PDMlib::ContainerInfo container_info, const int& time_step, const bool& coordinate_flag)
    {
        int    tmp_time_step = time_step;
        size_t length        = -1;
        if(PDMlib::PDMlib::GetInstance().Read(container_info.Name, &length, ptr, &tmp_time_step, true) <= 0)
        {
            return;
        }

        std::string label(container_info.Name);
        if(coordinate_flag)
        {
            label="";
        }

        if(container_info.nComp == 1)
        {
            H5PartWriter::WriteScalar(ptr, length, label);
        }else if(container_info.nComp == 3){
            if(container_info.VectorOrder == PDMlib::NIJK)
            {
                H5PartWriter::WriteVectorNIJK(ptr, length, label);
            }else if(container_info.VectorOrder == PDMlib::IJKN){
                H5PartWriter::WriteVectorIJKN(ptr, length, label);
            }
        }
        delete [] *ptr;
    }

    void ReadAndWriteContainerSelector(PDMlib::ContainerInfo container_info, const int& time_step, const bool& coordinate_flag)
    {
        if(container_info.Type == PDMlib::INT32)
        {
            int* ptr = NULL;
            ReadAndWriteContainer(&ptr, container_info, time_step, coordinate_flag);
        }else if(container_info.Type == PDMlib::uINT32){
            unsigned int* ptr = NULL;
            ReadAndWriteContainer(&ptr, container_info, time_step, coordinate_flag);
        }else if(container_info.Type == PDMlib::INT64){
            long* ptr = NULL;
            ReadAndWriteContainer(&ptr, container_info, time_step, coordinate_flag);
        }else if(container_info.Type == PDMlib::uINT64){
            unsigned long* ptr = NULL;
            ReadAndWriteContainer(&ptr, container_info, time_step, coordinate_flag);
        }else if(container_info.Type == PDMlib::FLOAT){
            float* ptr = NULL;
            ReadAndWriteContainer(&ptr, container_info, time_step, coordinate_flag);
        }else if(container_info.Type == PDMlib::DOUBLE){
            double* ptr = NULL;
            ReadAndWriteContainer(&ptr, container_info, time_step, coordinate_flag);
        }
    }

    void set_attribute(const int& time_step, const size_t& count)
    {
        H5PartSetStep(file, time_step);
        H5PartSetNumParticles(file, count);
    }


};
} // end of namespace
#endif
