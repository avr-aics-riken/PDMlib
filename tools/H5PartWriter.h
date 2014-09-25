#ifndef H5PartWRITER_H
#define H5PartWRITER_H
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <typeinfo>
#include <stdint.h>

#include <stdlib.h>
#include <stdarg.h>

#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
 
extern "C" {
#include <hdf5.h>
}
typedef int64_t int64_t;

namespace H5PartWriter
{
//! H5Part形式でファイル出力を行う関数群
class H5PartWriter
{
    hid_t         timegroup;
    hid_t         shape;
    hid_t         f;
    std::string   filename;

    std::ofstream out;

public:
    H5PartWriter(const std::string& arg_filename): shape(H5S_ALL),
                                                   filename(arg_filename)
    {
        hid_t access_prop = H5Pcreate(H5P_FILE_ACCESS);
        f = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, access_prop);
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
            write_data(container_name.c_str(), (void*)buffer, H5T_NATIVE_INT64);
            delete buffer;
        }else if(typeid(T) == typeid(float)){
            float* buffer = new float[length];
            for(int i = 0; i < length; i++)
            {
                buffer[i] = (float)((*ptr)[stride*i+offset]);
            }
            write_data(container_name.c_str(), (void*)buffer, H5T_NATIVE_FLOAT);
            delete buffer;
        }else if(typeid(T) == typeid(double)){
            double* buffer = new double[length];
            for(int i = 0; i < length; i++)
            {
                buffer[i] = (double)((*ptr)[stride*i+offset]);
            }

            write_data(container_name.c_str(), (void*)buffer, H5T_NATIVE_DOUBLE);
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

        ptr2 += num_particle;
        name  = container_name+"y";
            WriteScalar(&ptr2, num_particle, name);

        ptr2 += num_particle;
        name  = container_name+"z";
            WriteScalar(&ptr2, num_particle, name);
    }

    template<typename T>
    void ReadAndWriteContainer(T** ptr, PDMlib::ContainerInfo container_info, const int& time_step, const bool& coordinate_flag)
    {
        int    tmp_time_step = time_step;
        size_t length        = -1;
        if(PDMlib::PDMlib::GetInstance().Read(container_info.Name, &length, ptr, &tmp_time_step) <= 0)
        {
            return;
        }

        std::string label(container_info.Name);
        if(coordinate_flag)
        {
            label = "";
        }

        if(container_info.nComp == 1)
        {
            WriteScalar(ptr, length, label);
        }else if(container_info.nComp == 3){
            if(container_info.VectorOrder == PDMlib::NIJK)
            {
                WriteVectorNIJK(ptr, length, label);
            }else if(container_info.VectorOrder == PDMlib::IJKN){
                WriteVectorIJKN(ptr, length, label);
            }
        }
        delete[] *ptr;
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

    void set_attribute(const int& time_step, hsize_t count)
    {
        std::ostringstream stepname;
        stepname<<"Step#"<<time_step;
        this->timegroup = H5Gcreate(f, stepname.str().c_str(), H5P_DEFAULT, H5P_DEFAULT, 0);

        if(shape != H5S_ALL)
        {
            H5Sclose(shape);
            shape = H5S_ALL;
        }
        shape = H5Screate_simple(1, &count, NULL);
    }

private:
    void write_data(const char* name, /*!< IN: Name to associate array with */
                     const void* array, /*!< IN: Array to commit to disk */
                     const hid_t type /*!< IN: Type of data */
                     )
    {
        hid_t dataset_id;

        char  name2[64];
        strncpy(name2, name, 64);
        name2[63] = '\0';

        dataset_id = H5Dcreate(timegroup, name2, type, shape, H5P_DEFAULT, H5P_DEFAULT, H5P_DEFAULT);
        H5Dwrite(dataset_id, type, H5S_ALL, H5S_ALL, H5P_DEFAULT, array);
        H5Dclose(dataset_id);
    }

};
} // end of namespace
#endif
