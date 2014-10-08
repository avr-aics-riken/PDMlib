/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */

#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include "Write.h"

namespace BaseIO
{
Write* WriteFactory::create(const std::string& decorator, const std::string& type, const int& NumComp, const bool& text_flag, const char& delimiter)
{
    Write* writer = NULL;
    if(text_flag)
    {
        writer = new WriteTextFile(type, delimiter);
    }else{
        writer = new WriteBinaryFile;

        //decoratorで指定された内容にしたがってEncoderを追加する
        std::istringstream       iss(decorator);
        std::string              piece;
        std::vector<std::string> decorator_container;
        while(std::getline(iss, piece, '+'))
        {
            std::transform(piece.begin(), piece.end(), piece.begin(), tolower);
            if(piece == "zip" || piece == "fpzip" || piece == "rle")
            {
                decorator_container.push_back(piece);
            }
        }

        for(std::vector<std::string>::iterator it = decorator_container.begin(); it != decorator_container.end(); ++it)
        {
            if(*it == "zip")
            {
                writer = new ZipEncoder(writer);
            }else if(*it == "fpzip"){
                //float or double以外の時はfpzip encoderは無視
                if(type == "float")
                {
                    writer = new FpzipEncoder(writer, false, NumComp);
                }else if(type == "double"){
                    writer = new FpzipEncoder(writer, true, NumComp);
                }
            }else if(*it == "rle"){
                writer = new RLEEncoder(writer);
            }else{
                std::cerr<<"unknown encoder!!"<<std::endl;
            }
        }
    }

    return writer;
}
} //end of namespace