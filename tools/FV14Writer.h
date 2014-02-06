#ifndef FV14WRITER_H
#define FV14WRITER_H
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
namespace FV14Writer
{
//! FV14 形式でファイル出力を行う関数群
//!
//! FV14のParticlePathファイルは以下の順に出力する
//!   ヘッダ
//!   粒子数
//!   座標値
//!   ヘッダ部で指定した物理量(スカラーのみ)

//! 変数を1つ出力
template<typename T>
void WriteVariable(std::ofstream& out, const T& var)
{
    out.write((char*)&var, sizeof(var));
}

//! 配列を出力
template<typename T>
void WriteArray(std::ofstream& out, const T* vec, const int& length)
{
    out.write((char*)vec, sizeof(T)*length);
}

//! NIJKで格納されている座標を出力
template<typename T>
void WriteCoordNIJK(std::ofstream& out, T** ptr, const size_t& length)
{
    float* write_buff = new float[length];
    for(int i = 0; i < length; ++i)
    {
        write_buff[i] = (float)((*ptr)[i]);
    }
    WriteArray(out, write_buff, length);
    delete[] write_buff;
}

//! IJKNで格納されている座標を出力
template<typename T>
void WriteCoordIJKN(std::ofstream& out, T** ptr, const size_t& length)
{
    float* write_buff = new float[length];
    for(int i = 0; i < length/3; ++i)
    {
        write_buff[3*i]   = (float)((*ptr)[i]);
        write_buff[3*i+1] = (float)((*ptr)[i+length/3]);
        write_buff[3*i+2] = (float)((*ptr)[i+length/3*2]);
    }
    WriteArray(out, write_buff, length);
    delete[] write_buff;
}

//! スカラー量を出力
template<typename T>
void WriteScalar(std::ofstream& out, T** ptr, const size_t& length)
{
    float* write_buff = new float[length];
    for(int i = 0; i < length; ++i)
    {
        write_buff[i] = (float)((*ptr)[i]);
    }
    WriteArray(out, write_buff, length);
    delete[] write_buff;
}

//! NIJKで格納されているベクトル量を出力
template<typename T>
void WriteVectorNIJK(std::ofstream& out, T** ptr, const size_t& length)
{
    size_t num_particle = length/3;
    float* write_buff_x = new float[num_particle];
    float* write_buff_y = new float[num_particle];
    float* write_buff_z = new float[num_particle];
    for(int i = 0; i < num_particle; ++i)
    {
        write_buff_x[i] = (float)((*ptr)[i]);
        write_buff_y[i] = (float)((*ptr)[i+1]);
        write_buff_z[i] = (float)((*ptr)[i+2]);
    }
    WriteArray(out, write_buff_x, num_particle);
    WriteArray(out, write_buff_y, num_particle);
    WriteArray(out, write_buff_z, num_particle);
    delete[] write_buff_x;
    delete[] write_buff_y;
    delete[] write_buff_z;
}

//! IJKNで格納されているベクトル量を出力
template<typename T>
void WriteVectorIJKN(std::ofstream& out, T** ptr, const size_t& length)
{
    size_t num_particle = length/3;
    float* write_buff_x = new float[num_particle];
    float* write_buff_y = new float[num_particle];
    float* write_buff_z = new float[num_particle];
    for(int i = 0; i < num_particle; ++i)
    {
        write_buff_x[i] = (float)((*ptr)[i]);
    }
    for(int i = 0; i < num_particle; ++i)
    {
        write_buff_y[i] = (float)((*ptr)[i+num_particle]);
    }
    for(int i = 0; i < num_particle; ++i)
    {
        write_buff_z[i] = (float)((*ptr)[i+num_particle*2]);
    }
    WriteArray(out, write_buff_x, num_particle);
    WriteArray(out, write_buff_y, num_particle);
    WriteArray(out, write_buff_z, num_particle);
    delete[] write_buff_x;
    delete[] write_buff_y;
    delete[] write_buff_z;
}

//! FV14形式のヘッダ部分を出力する。
void WriteHeader(std::ofstream& out, const std::vector<std::string>& container_names, const int num_particle)
{
    WriteVariable(out, 0x0010203); //FV_MAGIC
    {
        char write_buffer[80] = "FVPARTICLES";
        WriteArray(out, write_buffer, 80);
    }
    WriteVariable(out, 3);  //Major version
    WriteVariable(out, 1);  //Minor version
    // FieldViewのリファレンスマニュアルにはここに0を出力するように記載されているが
    // 実際には0があると正常に読めないので削除した 2014/1/7
    //WriteVariable(out, 0);  //reserved
    WriteVariable(out, (int)container_names.size());
    for(std::vector<std::string>::const_iterator it = container_names.begin(); it != container_names.end(); ++it)
    {
        char write_buffer[80];
        strncpy(write_buffer, (*it).c_str(), 80);
        WriteArray(out, write_buffer, 80);
    }
    WriteVariable(out, num_particle);
}

//! BoundingBoxをFV-UNS(text)形式で6面体のセルとして出力する
void WriteBoundingBox(double bbox[6], const std::string& filename)
{
    std::ofstream out(filename.c_str());
    out<<"FIELDVIEW_Grids 3 0"<<std::endl;
    out<<"Grids 1"<<std::endl;
    out<<"Boundary Table 0"<<std::endl;
    out<<"Nodes 8"<<std::endl;
    out<<bbox[0]<<" "<<bbox[1]<<" "<<bbox[2]<<std::endl;
    out<<bbox[0]<<" "<<bbox[1]<<" "<<bbox[5]<<std::endl;
    out<<bbox[0]<<" "<<bbox[4]<<" "<<bbox[2]<<std::endl;
    out<<bbox[0]<<" "<<bbox[4]<<" "<<bbox[5]<<std::endl;
    out<<bbox[3]<<" "<<bbox[1]<<" "<<bbox[2]<<std::endl;
    out<<bbox[3]<<" "<<bbox[1]<<" "<<bbox[5]<<std::endl;
    out<<bbox[3]<<" "<<bbox[4]<<" "<<bbox[2]<<std::endl;
    out<<bbox[3]<<" "<<bbox[4]<<" "<<bbox[5]<<std::endl;
    out<<"Boundary Faces 0"<<std::endl;
    out<<"Elements"<<std::endl;
    out<<"2 1 1 2 3 4 5 6 7 8"<<std::endl;
}
} // end of namespace
#endif
