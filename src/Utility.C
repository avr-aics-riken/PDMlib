/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */

#include <mpi.h>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <set>
#include <string>
#include <glob.h>
#include <mpi.h>
#include "Utility.h"

namespace
{
// int型の1をファイルに書き込んで、先頭からindex Byte目の位置の値を返す
// endianの判定はintをcharにcastして読んでもできる可能性が高いが
// castして値を読んだ時にどのような値を返すかは処理系依存なので
// 念の為この実装にしている。
int read_nth_byte(const size_t& index)
{
    const char filename[] = "PDMlib_util_temp\0";
    if(index < 0 || index >= sizeof(int))return -1;

    int i = 1;
    std::ofstream out(filename, std::ios::binary);
    out.write((char*)&i, sizeof(int));
    out.close();
    char c[sizeof(int)];
    std::ifstream in(filename, std::ios::binary);
    in.read(c, sizeof(int));
    in.close();
    std::remove(filename);
    return c[index];
}
}

namespace PDMlib
{
bool is_little(void)
{
    //先頭Byteが1になってるか確認
    return read_nth_byte(0);
}

bool is_big(void)
{
    //末尾Byteが1になってるか確認
    return read_nth_byte(sizeof(int)-1);
}

bool isFile(const std::string& filename)
{
    std::ifstream ifs(filename.c_str());
    return !ifs.fail();
}

int GetStartIndex(const int& N, const int& NumProc, const int& MyRank)
{
    int index    = N/NumProc*MyRank;
    int reminder = N%NumProc;
    if(reminder > 0)
    {
        if(reminder > MyRank)
        {
            index += MyRank;
        }else{
            index += reminder;
        }
    }
    return index;
}

void ListDirectoryContents(const std::string& dir_name, std::vector<std::string>* filenames, const std::string& wild_card)
{
    filenames->clear();
    std::string tmp = dir_name+"/"+wild_card;
    glob_t      result;
    glob(tmp.c_str(), GLOB_MARK || GLOB_NOSORT || GLOB_NOCHECK || GLOB_TILDE, NULL, &result);
    for(int i = 0; i < result.gl_pathc; i++)
    {
        filenames->push_back(result.gl_pathv[i]);
    }
    globfree(&result);
}

bool is_all_digit(const std::string& str)
{
    for(std::string::const_iterator it = str.begin(); it != str.end(); ++it)
    {
        if(!isdigit(*it))return false;
    }
    return true;
}

int get_time_step(const std::string& filename)
{
    int pos_period   = filename.find_last_of('.');
    int pos_underbar = filename.find_last_of('_');
    if(pos_period == std::string::npos || pos_underbar == std::string::npos)
    {
        return -1;
    }
    std::string str_time_step(filename.substr(pos_underbar+1, pos_period-(pos_underbar+1)));
    if(!is_all_digit(str_time_step))
    {
        return -2;
    }
    return stoi(str_time_step);
}

int get_region_number(const std::string& filename)
{
    int pos_underbar = filename.find_last_of('_');
    if(pos_underbar == std::string::npos)
    {
        return -1;
    }
    std::string first_half(filename.substr(0, pos_underbar));
    pos_underbar = first_half.find_last_of('_');
    if(pos_underbar == std::string::npos)
    {
        return -1;
    }
    std::string str_region_number(first_half.substr(pos_underbar+1, std::string::npos));
    if(!is_all_digit(str_region_number))
    {
        return -2;
    }
    return stoi(str_region_number);
}

void MakeTimeStepList(std::set<int>* time_steps, const std::string& keyword, const std::string& dir_name, const int& start_time, const int& end_time, const std::string& wild_card)
{
    std::vector<std::string> filenames;
    ListDirectoryContents(dir_name, &filenames, wild_card);
    for(std::vector<std::string>::iterator it = filenames.begin(); it != filenames.end(); ++it)
    {
        if((*it).find(keyword) != std::string::npos)
        {
            int time_step = get_time_step(*it);
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

int stoi(std::string str){return atoi(str.c_str());}
long stol(std::string str){return atol(str.c_str());}
long long stoll(std::string str){return atoll(str.c_str());}
float stof(std::string str){return (float)atof(str.c_str());}
double stod(std::string str){return atof(str.c_str());}

SupportedType string2enumType(std::string sType)
{
    SupportedType enumType = SupportedType(-1);
    if(sType == "INT32")
    {
        enumType = INT32;
    }else if(sType == "uINT32"){
        enumType = uINT32;
    }else if(sType == "INT64"){
        enumType = INT64;
    }else if(sType == "uINT64"){
        enumType = uINT64;
    }else if(sType == "FLOAT"){
        enumType = FLOAT;
    }else if(sType == "DOUBLE"){
        enumType = DOUBLE;
    }
    return enumType;
}

std::string enumType2string(SupportedType enumType)
{
    std::string sType("Unknown Type");
    switch(enumType)
    {
    case INT32:
        sType = "INT32";
        break;

    case uINT32:
        sType = "uINT32";
        break;

    case INT64:
        sType = "INT64";
        break;

    case uINT64:
        sType = "uINT64";
        break;

    case FLOAT:
        sType = "FLOAT";
        break;

    case DOUBLE:
        sType = "DOUBLE";
        break;
    }
    return sType;
}

StorageOrder string2enumStorageOrder(std::string sOrder)
{
    StorageOrder enumOrder = StorageOrder(-1);
    if(sOrder == "NIJK")
    {
        enumOrder = NIJK;
    }else if(sOrder == "IJKN"){
        enumOrder = IJKN;
    }
    return enumOrder;
}

size_t GetSize(SupportedType enumType)
{
    size_t size_of_type = 1;
    switch(enumType)
    {
    case INT32:
        size_of_type = sizeof(int);
        break;

    case uINT32:
        size_of_type = sizeof(unsigned int);
        break;

    case INT64:
        size_of_type = sizeof(long);
        break;

    case uINT64:
        size_of_type = sizeof(unsigned long);
        break;

    case FLOAT:
        size_of_type = sizeof(float);
        break;

    case DOUBLE:
        size_of_type = sizeof(double);
        break;
    }

    return size_of_type;
}
} //end of namespace