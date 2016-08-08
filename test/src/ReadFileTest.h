/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */

#ifndef READ_FILE_TEST_H
#define READ_FILE_TEST_H
#include <vector>
#include <iostream>
#include <sstream>
#include "gtest/gtest.h"
#include "FileUtils.h"
#include "TestDataGenerator.h"
#include "Write.h"
#include "Read.h"

// パラメータに指定する値
//   0:テストデータのサイズ (要素数）
//   1:テストデータの種類(sequential, ran, or same)
//   2:TextFileフラグ(falseならバイナリファイル)
class ReadFileTestWithIntData: public ::testing::TestWithParam<std::tr1::tuple<int, std::string> >
{
protected:
    ReadFileTestWithIntData()
    {
        data = TestDataGenerator<int>::create(std::tr1::get<0>(GetParam()), std::tr1::get<1>(GetParam()));
        std::ostringstream oss;
        oss<<std::tr1::get<0>(GetParam());

        filename = std::tr1::get<1>(GetParam())+"_int_"+oss.str();
        BaseIO::Write* writer;
        filename += ".bin";
        reader    = BaseIO::ReadFactory::create(filename, "", "int", 1);
        writer    = BaseIO::WriteFactory::create("", "int", 1);
        const int length = std::tr1::get<0>(GetParam())*sizeof(data[0]);
        writer->write(filename.c_str(), length, length, (char*)(data));
        delete writer;
    }

    ~ReadFileTestWithIntData()
    {
        delete reader;
        delete data;
    }

    int*          data;
    BaseIO::Read* reader;
    std::string   filename;
};

TEST_P(ReadFileTestWithIntData, read)
{
    int*   read_data;
    size_t length = std::tr1::get<0>(GetParam())*sizeof(read_data[0]);
        EXPECT_EQ(length,  reader->read(length, (char**)&read_data));
    for(int i = 0; i < std::tr1::get<0>(GetParam()); i++)
    {
        EXPECT_EQ(data[i], read_data[i]);
    }
    delete (char*)read_data;
}

class ReadFileTestWithLongData: public ::testing::TestWithParam<std::tr1::tuple<int, std::string> >
{
protected:
    ReadFileTestWithLongData()
    {
        data = TestDataGenerator<long>::create(std::tr1::get<0>(GetParam()), std::tr1::get<1>(GetParam()));
        std::ostringstream oss;
        oss<<std::tr1::get<0>(GetParam());

        filename = std::tr1::get<1>(GetParam())+"_long_"+oss.str();
        BaseIO::Write* writer;
        filename += ".bin";
        reader    = BaseIO::ReadFactory::create(filename, "", "long", 1);
        writer    = BaseIO::WriteFactory::create("", "long", 1);
        const int length = std::tr1::get<0>(GetParam())*sizeof(data[0]);
        writer->write(filename.c_str(), length, length, (char*)(data));
        delete writer;
    }

    ~ReadFileTestWithLongData()
    {
        delete reader;
        delete data;
    }

    long*         data;
    BaseIO::Read* reader;
    std::string   filename;
};

TEST_P(ReadFileTestWithLongData, read)
{
    long*  read_data;
    size_t length = std::tr1::get<0>(GetParam())*sizeof(read_data[0]);
        EXPECT_EQ(length,  reader->read(length, (char**)&read_data));
    for(int i = 0; i < std::tr1::get<0>(GetParam()); i++)
    {
        EXPECT_EQ(data[i], read_data[i]);
    }
    delete (char*)read_data;
}

class ReadFileTestWithFloatData: public ::testing::TestWithParam<std::tr1::tuple<int, std::string> >
{
protected:
    ReadFileTestWithFloatData()
    {
        data = TestDataGenerator<float>::create(std::tr1::get<0>(GetParam()), std::tr1::get<1>(GetParam()));
        std::ostringstream oss;
        oss<<std::tr1::get<0>(GetParam());

        filename = std::tr1::get<1>(GetParam())+"_float_"+oss.str();
        BaseIO::Write* writer;
        filename += ".bin";
        reader    = BaseIO::ReadFactory::create(filename, "", "float", 1);
        writer    = BaseIO::WriteFactory::create("", "float", 1);
        const int length = std::tr1::get<0>(GetParam())*sizeof(data[0]);
        writer->write(filename.c_str(), length, length, (char*)(data));
        delete writer;
    }

    ~ReadFileTestWithFloatData()
    {
        delete reader;
        delete data;
    }

    float*        data;
    BaseIO::Read* reader;
    std::string   filename;
};

TEST_P(ReadFileTestWithFloatData, read)
{
    float* read_data;
    size_t length = std::tr1::get<0>(GetParam())*sizeof(read_data[0]);
        EXPECT_EQ(length,  reader->read(length, (char**)&read_data));
    for(int i = 0; i < std::tr1::get<0>(GetParam()); i++)
    {
        EXPECT_EQ(data[i], read_data[i]);
    }
    delete (char*)read_data;
}

class ReadFileTestWithDoubleData: public ::testing::TestWithParam<std::tr1::tuple<int, std::string> >
{
protected:
    ReadFileTestWithDoubleData()
    {
        data = TestDataGenerator<double>::create(std::tr1::get<0>(GetParam()), std::tr1::get<1>(GetParam()));
        std::ostringstream oss;
        oss<<std::tr1::get<0>(GetParam());

        filename = std::tr1::get<1>(GetParam())+"_double_"+oss.str();
        BaseIO::Write* writer;
        filename += ".bin";
        reader    = BaseIO::ReadFactory::create(filename, "", "double", 1);
        writer    = BaseIO::WriteFactory::create("", "double", 1);
        size_t length = std::tr1::get<0>(GetParam())*sizeof(data[0]);
        writer->write(filename.c_str(), length, length, (char*)(data));
        delete writer;
    }

    ~ReadFileTestWithDoubleData()
    {
        delete reader;
        delete data;
    }

    double*       data;
    BaseIO::Read* reader;
    std::string   filename;
};

TEST_P(ReadFileTestWithDoubleData, read)
{
    double* read_data;
    size_t  length = std::tr1::get<0>(GetParam())*sizeof(read_data[0]);
        EXPECT_EQ(length,  reader->read(length, (char**)&read_data));
    for(int i = 0; i < std::tr1::get<0>(GetParam()); i++)
    {
        EXPECT_EQ(data[i], read_data[i]);
    }
    delete (char*)read_data;
}
#endif
