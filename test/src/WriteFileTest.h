/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */

#ifndef WRITE_FILE_TEST_H
#define WRITE_FILE_TEST_H
#include <vector>
#include <iostream>
#include <sstream>
#include "gtest/gtest.h"
#include "FileUtils.h"
#include "TestDataGenerator.h"
#include "Write.h"

// パラメータに指定する値
//   0:テストデータのサイズ
//   1:テストデータの種類(sequential, ran, or same)
//   2:TextFileフラグ(falseならバイナリファイル)
class WriteFileTestWithIntData: public ::testing::TestWithParam<std::tr1::tuple<int, std::string, bool> >
{
protected:
    WriteFileTestWithIntData()
    {
        data = TestDataGenerator<int>::create(std::tr1::get<0>(GetParam()), std::tr1::get<1>(GetParam()));
        std::ostringstream oss;
        oss<<std::tr1::get<0>(GetParam());

        filename = std::tr1::get<1>(GetParam())+"_int_"+oss.str();
        if(std::tr1::get<2>(GetParam()))
        {
            filename += ".txt";
            writer    = BaseIO::WriteFactory::create("", "int", 1, true);
        }else{
            filename += ".bin";
            writer    = BaseIO::WriteFactory::create("", "int", 1);
        }
    }

    ~WriteFileTestWithIntData()
    {
        delete writer;
        delete data;
    }

    int*           data;
    BaseIO::Write* writer;
    std::string    filename;
};

TEST_P(WriteFileTestWithIntData, write)
{
    const int     length = std::tr1::get<0>(GetParam())*sizeof(this->data[0]);
    EXPECT_EQ(length, this->writer->write(filename.c_str(), length, length, (char*)(this->data)));
    std::ifstream file(filename.c_str());
    EXPECT_TRUE(file.is_open());
    file.close();
}

class WriteFileTestWithLongData: public ::testing::TestWithParam<std::tr1::tuple<int, std::string, bool> >
{
protected:
    WriteFileTestWithLongData()
    {
        data = TestDataGenerator<long>::create(std::tr1::get<0>(GetParam()), std::tr1::get<1>(GetParam()));
        std::ostringstream oss;
        oss<<std::tr1::get<0>(GetParam());

        filename = std::tr1::get<1>(GetParam())+"_long_"+oss.str();
        if(std::tr1::get<2>(GetParam()))
        {
            filename += ".txt";
            writer    = BaseIO::WriteFactory::create("", "long", 1, true);
        }else{
            filename += ".bin";
            writer    = BaseIO::WriteFactory::create("", "long", 1);
        }
    }

    ~WriteFileTestWithLongData()
    {
        delete writer;
        delete data;
    }

    long*          data;
    BaseIO::Write* writer;
    std::string    filename;
};

TEST_P(WriteFileTestWithLongData, write)
{
    const int     length = std::tr1::get<0>(GetParam())*sizeof(this->data[0]);
    EXPECT_EQ(length, this->writer->write(filename.c_str(), length, length, (char*)(this->data)));
    std::ifstream file(filename.c_str());
    EXPECT_TRUE(file.is_open());
    file.close();
}

class WriteFileTestWithFloatData: public ::testing::TestWithParam<std::tr1::tuple<int, std::string, bool> >
{
protected:
    WriteFileTestWithFloatData()
    {
        data = TestDataGenerator<float>::create(std::tr1::get<0>(GetParam()), std::tr1::get<1>(GetParam()));
        std::ostringstream oss;
        oss<<std::tr1::get<0>(GetParam());

        filename = std::tr1::get<1>(GetParam())+"_float_"+oss.str();
        if(std::tr1::get<2>(GetParam()))
        {
            filename += ".txt";
            writer    = BaseIO::WriteFactory::create("", "float", 1, true);
        }else{
            filename += ".bin";
            writer    = BaseIO::WriteFactory::create("", "float", 1);
        }
    }

    ~WriteFileTestWithFloatData()
    {
        delete writer;
        delete data;
    }

    float*         data;
    BaseIO::Write* writer;
    std::string    filename;
};

TEST_P(WriteFileTestWithFloatData, write)
{
    const int     length = std::tr1::get<0>(GetParam())*sizeof(this->data[0]);
    EXPECT_EQ(length, this->writer->write(filename.c_str(), length, length, (char*)(this->data)));
    std::ifstream file(filename.c_str());
    EXPECT_TRUE(file.is_open());
    file.close();
}

class WriteFileTestWithDoubleData: public ::testing::TestWithParam<std::tr1::tuple<int, std::string, bool> >
{
protected:
    WriteFileTestWithDoubleData()
    {
        data = TestDataGenerator<double>::create(std::tr1::get<0>(GetParam()), std::tr1::get<1>(GetParam()));
        std::ostringstream oss;
        oss<<std::tr1::get<0>(GetParam());

        filename = std::tr1::get<1>(GetParam())+"_double_"+oss.str();
        if(std::tr1::get<2>(GetParam()))
        {
            filename += ".txt";
            writer    = BaseIO::WriteFactory::create("", "double", 1, true);
        }else{
            filename += ".bin";
            writer    = BaseIO::WriteFactory::create("", "double", 1);
        }
    }

    ~WriteFileTestWithDoubleData()
    {
        delete writer;
        delete data;
    }

    double*        data;
    BaseIO::Write* writer;
    std::string    filename;
};

TEST_P(WriteFileTestWithDoubleData, write)
{
    const int     length = std::tr1::get<0>(GetParam())*sizeof(this->data[0]);
    EXPECT_EQ(length, this->writer->write(filename.c_str(), length, length, (char*)(this->data)));
    std::ifstream file(filename.c_str());
    EXPECT_TRUE(file.is_open());
    file.close();
}
#endif