/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */

#ifndef VTK_WRITER_H
#define VTK_WRITER_H
#include <cstring>
#include <string>
#include <vector>
#include <utility>
#include <list>
#include <iostream>
#include <fstream>
#include <typeinfo>
namespace
{
  static const std::string table("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
  inline char lower2bit(const char& input) { return input&0x03; }
  inline char lower4bit(const char& input) { return input&0x0f; }
  inline char lower6bit(const char& input) { return input&0x3f; }
  inline char lower8bit(const char& input) { return input&0xff; }

  /// @brief 3byteの文字列をbase64で変換して4Byteの文字列にする
  ///
  /// @param input[3]  入力文字列
  /// @param output[4] 変換後の文字列
  void encode3(const char input[3], char output[4])
  {
    output[0]=table[lower6bit(input[0] >>2)];                       // input0の上位6bit
    output[1]=table[lower2bit(input[0])<<4|lower4bit(input[1]>>4)]; // input[0]の下位2bitとinput[1]の上位4bit
    output[2]=table[lower4bit(input[1])<<2|lower2bit(input[2]>>6)]; // input[1]の下位4bitとinput[2]の上位2bit
    output[3]=table[lower6bit(input[2])];                           // input2の下位6bit
  }

  /// @brief 2byteの文字列をbase64で変換して4Byteの文字列にする
  ///
  /// @param input[2] 入力文字列
  /// @param output[4] 変換後の文字列
  void encode2(const char input[2], char output[4])
  {
    output[0]=table[lower6bit(input[0] >>2)];                       // input0の上位6bit
    output[1]=table[lower2bit(input[0])<<4|lower4bit(input[1]>>4)]; // input[0]の下位2bitとinput[1]の上位4bit
    output[2]=table[lower4bit(input[1])<<2];                        // input[1]の下位4bitと0を2bit足す
    output[3]='=';
  }

  /// @brief 1byteの文字列をbase64で変換して4Byteの文字列にする
  ///
  /// @param input[1]  入力文字列
  /// @param output[4] 変換後の文字列
  void encode1(const char input[1], char output[4])
  {
    output[0]=table[lower6bit(input[0] >>2)]; // input0の上位6bit
    output[1]=table[lower2bit(input[0])<<4];  // input[0]の下位2bitと0を4bit足したもの
    output[2]='=';
    output[3]='=';
  }

  /// @brief 与えられた配列をbase64でエンコードする
  ///
  /// @param len    入力文字列長
  /// @param input  入力文字列
  /// @param output 変換後の文字列
  //
  //outputが空のstringでは無い場合は、末尾にエンコード済文字列を連結して返すが
  //base64エンコードの仕様上、encode関数を複数回呼び出して追記すると
  //不正な文字列となるので注意が必要
  void encode(const size_t& len, const char* input, std::string* output)
  {
    char buff[4];
    for(size_t i=0; i<len/3*3;i+=3)
    {
      encode3(&(input[i]), buff);
      output->push_back(buff[0]);
      output->push_back(buff[1]);
      output->push_back(buff[2]);
      output->push_back(buff[3]);
    }
    int reminder=len%3;
    if(reminder >0)
    {
      if(reminder==1)
      {
        encode1(&(input[len-1]), buff);
      }else if (reminder==2) {
        encode2(&(input[len-2]), buff);
      }
      output->push_back(buff[0]);
      output->push_back(buff[1]);
      output->push_back(buff[2]);
      output->push_back(buff[3]);
    }
  }
}//end of namespace

namespace VtkWriter
{
  //VTK(ascii)形式で出力するための関数群
  template <typename T>
    static std::string get_type(T data)
    {
      std::string type;
      if(typeid(data)==typeid(int))
      {
        type = "Int32";
      } else if(typeid(data)==typeid(long)){
        type = "Int64";
      } else if(typeid(data)==typeid(unsigned int)){
        type = "UInt32";
      } else if(typeid(data)==typeid(unsigned long)){
        type = "Unt64";
      } else if(typeid(data)==typeid(float)){
        type = "Float32";
      } else if(typeid(data)==typeid(double)){
        type = "Float64";
      }
      return type;
    }

  template <typename T>
  struct WriteContainer
  {
    virtual void write(std::ofstream& ofs, const int& num_comp, const size_t& num_elements, const T* container)=0;
  };

  template <typename T>
  struct WriteContainerAscii: public WriteContainer<T>
  {
    void write(std::ofstream& ofs, const int& num_comp, const size_t& num_elements, const T* container)
    {
      for (size_t j=0; j<num_elements; j++)
      {
        for(int i=0; i<num_comp; i++)
        {
          ofs << container[j*num_comp+i] << " ";
        }
        ofs <<std::endl;
      }
    }
  };
  template <typename T>
  struct WriteContainerBinary: public WriteContainer<T>
  {
    void write(std::ofstream& ofs, const int& num_comp, const size_t& num_elements, const T* container)
    {
      std::string output;
      const int header_size=8;
      size_t n=sizeof(T)*num_elements*num_comp;
      size_t len=n+header_size;
      char* buff=new char[len];
      memcpy(buff, &n, header_size);
      memcpy(&(buff[header_size]), container, n);
      encode(len, buff, &output);
      delete [] buff;
      ofs.write(output.c_str(), output.size());
      ofs<<std::endl;
    }
  };

  template<typename T>
    static WriteContainer<T>* WriteContainerFactory(const std::string& format, const T* const container)
    {
      if(format == "ascii")
      {
        return new WriteContainerAscii<T>();
      }else if(format == "binary") {
        return new  WriteContainerBinary<T>();
      }else{
        throw;
      }
    }

  class WriteVTKFile
  {
    public:
      WriteVTKFile(std::string& filename)
      {
        ofs.open(filename.c_str());
        ofs <<"<?xml version=\"1.0\"?>"<<std::endl;
      }
      virtual ~WriteVTKFile(){}
      void WriteStartTag(const std::string& tag)
      {
        ofs<<"<"<<tag<<">"<<std::endl;
      }
      void WriteEndTag(const std::string& tag)
      {
        ofs<<"</"<<tag<<">"<<std::endl;
      }

      template <typename T>
        void WriteDataArray(const std::string& name, const int& num_comp, const size_t& num_elements, const T* container, std::string format)
        {
          ofs <<"<DataArray ";
          ofs <<"Name=\""<<name<<"\" ";
          ofs <<"type=\""<<get_type(container[0])<<"\" ";
          ofs <<"NumberOfComponents=\""<<num_comp<<"\" ";
          ofs <<"format=\""<<format<<"\">"<<std::endl;
          WriteContainer<T>* writer=WriteContainerFactory(format, container);
          writer->write(ofs, num_comp, num_elements, container);
          delete writer;
          ofs <<"</DataArray>"<<std::endl;
        }
    protected:
      std::ofstream ofs;
  };
  class PolyData:public WriteVTKFile
  {
    public:
      PolyData(std::string& filename, const size_t& nPoints=0, const size_t& nVerts=0, const size_t& nLines=0, const size_t& nStrips=0, const size_t& nPolys=0):
        WriteVTKFile(filename),num_points(nPoints), num_verts(nVerts), num_lines(nLines), num_strips(nStrips), num_polys(nPolys)
    {
      ofs <<"<VTKFile type=\"PolyData\" version=\"0.1\" byte_order=\"LittleEndian\" header_type=\"UInt64\">"<<std::endl;
      ofs <<"<PolyData>"<<std::endl;
      ofs <<"<Piece "<<std::endl;
      ofs <<"  NumberOfPoints=\""<<num_points<<"\""<<std::endl;;
      ofs <<"  NumberOfVerts=\""<< num_verts<<"\""<<std::endl;;
      ofs <<"  NumberOfLines=\""<< num_lines<<"\" "<<std::endl;;
      ofs <<"  NumberOfStrips=\""<<num_strips<<"\" "<<std::endl;;
      ofs <<"  NumberOfPolys=\""<< num_polys<<"\""<<std::endl;
      ofs <<">"<<std::endl;
    }
      ~PolyData()
      {
        ofs <<"</Piece>"<<std::endl;
        ofs <<"</PolyData>"<<std::endl;
        ofs <<"</VTKFile>"<<std::endl;
      }
      template <typename T>
        void WritePoints(T* coords, const std::string& format)
        {
          WriteStartTag("Points");
          WriteDataArray("Points", 3, num_points, coords, format);
          WriteEndTag("Points");
        }

      void WriteAllPointsAsVerts(const std::string& format)
      {
        WriteStartTag("Verts");
        long* data=new long [num_verts+1];
        for(size_t i=0;i<num_verts;i++)
        {
          data[i]=i;
        }
        WriteDataArray("connectivity", 1, num_verts, data, format);

        for(size_t i=0;i<num_verts+1;i++)
        {
          data[i]=i+1;
        }
        WriteDataArray("offsets", 1, num_verts+1, data, format);
        delete [] data;
        WriteEndTag("Verts");
      }
      const size_t num_points;
      const size_t num_verts;
      const size_t num_lines;
      const size_t num_strips;
      const size_t num_polys;
  };
}//end of namespace
#endif
