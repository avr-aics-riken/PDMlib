#include <vector>
#include <iostream>
#include <sstream>
#include "gtest/gtest.h"
#include "FileUtils.h"
#include "TestDataGenerator.h"
#include "Read.h"
#include "Write.h"

// パラメータに指定する値
//   1:テストデータの種類(sequential, random, or same)
//   2:Encode/Decodeの種類
class EncodeDecodeTest: public ::testing::TestWithParam<std::tr1::tuple<std::string, std::string> >
{
protected:
    EncodeDecodeTest(): length(1000)
    {
        data   = TestDataGenerator<double>::create(length, std::tr1::get<1>(GetParam()));
        writer = BaseIO::WriteFactory::create(std::tr1::get<0>(GetParam()), "double", 1);
        reader = BaseIO::ReadFactory::create(std::tr1::get<0>(GetParam()), "double", 1);
        std::ostringstream oss;
        oss<<length;
        filename = std::tr1::get<0>(GetParam())+"_"+std::tr1::get<1>(GetParam())+"_"+oss.str()+".bin";
        writer->write(filename.c_str(), length*sizeof(double), length*sizeof(double), (char*)(data));
    }

    ~EncodeDecodeTest()
    {
        delete data;
        delete writer;
        delete reader;
    }

    double*        data;
    BaseIO::Read*  reader;
    BaseIO::Write* writer;
    std::string    filename;
    const int      length;
};

TEST_P(EncodeDecodeTest, read)
{
    double* read_data;
    size_t  tmp_size;
        EXPECT_EQ(length*sizeof(double), reader->read(filename.c_str(), tmp_size, (char**)&read_data));

    for(int i = 0; i < length; i++)
    {
        EXPECT_EQ(data[i], read_data[i]);
    }
    delete (char*)read_data;
}

INSTANTIATE_TEST_CASE_P(AllTest, EncodeDecodeTest,
                        ::testing::Combine(
                            ::testing::Values("none", "zip", "fpzip", "RLE", "zip+rLe", "RLE+ZIP"),
                            ::testing::Values("sequential", "random", "same")
                            )
                        );