#ifndef PDMLIB_WRITE_H
#define PDMLIB_WRITE_H
#include <fstream>
#include <climits>

namespace BaseIO
{
//forward declaration
class WriteFactory;

//! ファイル出力基底クラス
class Write
{
    //non-copyable
    Write(const Write&);
    Write& operator=(const Write&);

protected:
    Write(){}

public:
    virtual ~Write(){}
    //! @param [in] filename      出力ファイル名（ファイルが存在する場合は上書きされる）
    //! @param [in] original_size 圧縮前のデータサイズ（Byte)
    //! @param [in] actual_size   実際の出力サイズ（Byte)
    //! @param [in] data          出力データ
    virtual int write(const char* filename, const size_t& original_size, const size_t& actual_size, char* data) = 0;
};

//! 抽象ファイル出力クラス
class WriteFile: public Write
{
public:
    virtual ~WriteFile(){}
    virtual int write(const char* filename, const size_t& original_size, const size_t& actual_size, char* data) = 0;

protected:
    std::ofstream out;
};

//! テキスト形式でファイル出力を行う具象クラス
//
//主にデバッグ出力に使うことを想定して作成したクラス
//圧縮すると色々と問題が出る可能性があるので、圧縮はしないこと
class WriteTextFile: public WriteFile
{
    friend WriteFactory;
    WriteTextFile(const std::string& arg_type, const char& arg_delimiter): type(arg_type),
                                                                           delimiter(arg_delimiter)
    {
        if((type == "int") || (type == "unsigned int") || (type == "float"))
        {
            size_of_type = 4;
        }else if((type == "long") || (type == "unsigned long") || (type == "double")){
            size_of_type = 8;
        }else{
            size_of_type = 1;
        }
    }

public:
    int write(const char* filename, const size_t& original_size, const size_t& actual_size, char* data);

private:
    std::string type;
    size_t      size_of_type;
    char        delimiter;
};

//! バイナリ形式でファイル出力を行う具象クラス
class WriteBinaryFile: public WriteFile
{
    friend WriteFactory;
    WriteBinaryFile(){}

public:
    int write(const char* filename, const size_t& original_size, const size_t& actual_size, char* data);
};

//
// declaration and implimentation of decorator
//
//! 抽象デコレータ
//
//!privateメンバのbuffはwrite内で確保して圧縮用の一次領域として使うためのもの
class Encoder: public Write
{
protected:
    Encoder(Write* arg): base(arg),
                         buff(NULL){}

public:
    virtual ~Encoder()
    {
        delete base;
        delete buff;
    }

    virtual int write(const char* filename, const size_t& original_size, const size_t& actual_size, char* data) = 0;

protected:
    Write* base;
    char*  buff;
};

//! zip形式による圧縮機能を提供する具象デコレータ
class ZipEncoder: public Encoder
{
    friend WriteFactory;
    explicit ZipEncoder(Write* arg): Encoder(arg){}

public:
    int write(const char* filename, const size_t& original_size, const size_t& actual_size, char* data);
};

//! fpzip形式による圧縮機能を提供する具象デコレータ
class FpzipEncoder: public Encoder
{
    friend WriteFactory;
    explicit FpzipEncoder(Write* arg, bool is_dp, int arg_vlen): Encoder(arg),
                                                                 dp(0),
                                                                 vlen(arg_vlen)
    {
        if(is_dp) dp = 1;
    }

public:
    int write(const char* filename, const size_t& original_size, const size_t& actual_size, char* data);

private:
    int      dp;
    unsigned vlen;
};

//! RLEアルゴリズムによる圧縮機能を提供する具象デコレータ
class RLEEncoder: public Encoder
{
    friend WriteFactory;
    explicit RLEEncoder(Write* arg): Encoder(arg){}

public:
    int write(const char* filename, const size_t& original_size, const size_t& actual_size, char* data);
};

//! Writeクラス用シンプルファクトリ
class WriteFactory
{
public:
    static Write* create(const std::string& decorator, const std::string& type, const int& NumComp, const bool& text_flag = false, const char& delimiter = ',');
};
}
#endif
