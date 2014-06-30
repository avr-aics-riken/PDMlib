#include <iostream>
#include <fstream>
#include <cstdlib>
#include <set>
#include <string>
#include <dirent.h>
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
    const char    filename[] = "PDMlib_util_temp\0";
    if(index < 0 || index >= sizeof(int)) return -1;

    int           i = 1;
    std::ofstream out(filename, std::ios::binary);
    out.write((char*)&i, sizeof(int));
    out.close();
    char          c[sizeof(int)];
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

void ListDirectoryContents(std::string dir_name, std::vector<std::string>* filenames)
{
    filenames->clear();
    DIR* dp;
    if((dp = opendir(dir_name.c_str())) == NULL)
    {
        std::cerr<<"Couldn't open current directory!"<<std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    struct dirent* entry;
    entry = readdir(dp);
    while(entry != NULL)
    {
        std::string filename(entry->d_name);
        filenames->push_back(filename);
        entry = readdir(dp);
    }
    closedir(dp);
}

void MakeTimeStepList(std::set<int>* time_steps, const std::string& keyword, const std::string& dir_name, const int& start_time, const int& end_time)
{
    std::vector<std::string> filenames;
    ListDirectoryContents(dir_name, &filenames);
    for(std::vector<std::string>::iterator it = filenames.begin(); it != filenames.end(); ++it)
    {
        if((*it).find(keyword) != std::string::npos)
        {
            int         pos_underbar = (*it).find_last_of('_');
            int         pos_period   = (*it).find_last_of('.');
            std::string str_time_step((*it).substr(pos_underbar+1, pos_period-pos_underbar));
            int         time_step = stoi(str_time_step);
            if(start_time <= time_step && time_step <= end_time)
            {
                time_steps->insert(time_step);
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