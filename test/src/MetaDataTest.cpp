/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */

#include "gtest/gtest.h"
#define private public
#include "MetaData.h"
#undef private

TEST(MetaDataTest, write)
{
    PDMlib::MetaData md("MetaDataTest.txt");
    //単位系情報を追加
    PDMlib::UnitElem p = {"Pressure", "Pa", 2.0, 0.3, "true"};
    PDMlib::UnitElem v = {"velocity", "m/s", 0.9, 30, "false"};
    md.AddUnit(p);
    md.AddUnit(v);
    double bbox[6]={0.1, 0.2, 0.3, 0.4, 0.5, 0.6};
    md.SetBoundingBox(bbox);


    //コンテナを追加
    PDMlib::ContainerInfo PV = {"ParticleVerocity", "", "zip", PDMlib::FLOAT, "vel", 3, PDMlib::NIJK};
    PDMlib::ContainerInfo T  = {"temperature", "", "zip", PDMlib::FLOAT, "temp", 1};
    PDMlib::ContainerInfo id = {"ParticleID", "", "RLE", PDMlib::INT64, "id", 1};
    md.AddContainer(PV);
    md.AddContainer(T);
    md.AddContainer(id);

    EXPECT_EQ(0, md.Write());
//    md.WriteTimeSlice();

    PDMlib::MetaData md2("MetaDataTest.txt");
    EXPECT_EQ(0, md2.Read());
    double bbox2[6];
    md2.GetBoundingBox(bbox2);

    //mdとmd2が保持する値が等しいことを確認する
    EXPECT_TRUE(md.Compare(md2));

    //bounding boxはgoogle testのマクロで検査
    for (int i =0; i<6; i++)
    {
      EXPECT_DOUBLE_EQ(bbox[i], bbox2[i]);
    }
}
