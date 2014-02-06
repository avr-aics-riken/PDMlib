#ifndef FILE_UTILS_H
#define FILE_UTILS_H
#include <iostream>
#include <fstream>

namespace FileUtils
{
static void RemoveFile(const std::string& filename)
{
    std::ifstream file(filename.c_str());
    if(file.is_open())
    {
        file.close();
        remove(const_cast<char*>(filename.c_str()));
    }
}

static void CopyFile(const std::string& src, const std::string& dst)
{
    std::ifstream input(src.c_str(), std::fstream::binary);
    std::ofstream output(dst.c_str(), std::fstream::trunc|std::fstream::binary);
    output<<input.rdbuf();
}
}
#endif