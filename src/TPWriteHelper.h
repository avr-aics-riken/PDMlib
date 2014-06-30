#ifndef PDMLIB_TPWRIT_HELPER_H
#define PDMLIB_TPWRIT_HELPER_H
namespace PDMlib
{
//! TextPaser形式のファイル出力時に括弧と対応して
//自動的にindentレベルを揃えるクラス
class TPWriteHelper
{
public:
    TPWriteHelper(): level(0){}

    void write_header(std::ofstream& out, const std::string& header)
    {
        out<<"\n";
        for(int i = 0; i < level; i++)
        {
            out<<"  ";
        }
        out<<header;
        out<<"{\n";
        level++;
    }

    void write_rbrace(std::ofstream& out)
    {
        level--;
        for(int i = 0; i < level; i++)
        {
            out<<"  ";
        }
        out<<"}\n";
    }

    template<typename T>
    void write_vector(std::ofstream& out, std::string name, const T* value, const size_t& vlen)
    {
        for(int i = 0; i < level; i++)
        {
            out<<"  ";
        }
        out<<name<<" = (";
        for(int i = 0; i < vlen-1; i++)
        {
            out<<value[i]<<",";
        }
        out<<value[vlen-1]<<")\n";
    }

    template<typename T>
    void write_value(std::ofstream& out, std::string name, const T& value)
    {
        for(int i = 0; i < level; i++)
        {
            out<<"  ";
        }
        out<<name<<" = "<<value<<"\n";
    }

private:
    int level;
};

//! @attention char or char*は特殊化していないので、文字列をstd::stringではなくcharやchar[]で出力すると正しいTextParser形式では出力できない
template<>
void TPWriteHelper::write_vector<std::string>(std::ofstream& out, std::string name, const std::string* const value, const size_t& vlen)
{
    for(int i = 0; i < level; i++)
    {
        out<<"  ";
    }
    out<<name<<" = (";
    for(int i = 0; i < vlen-1; i++)
    {
        out<<"\""<<value[i]<<"\",";
    }
    out<<value[vlen-1]<<"\")\n";
}

template<>
void TPWriteHelper::write_value<std::string>(std::ofstream& out, std::string name, const std::string& value)
{
    for(int i = 0; i < level; i++)
    {
        out<<"  ";
    }
    out<<name<<" = \""<<value<<"\"\n";
}
} //end of namespace
#endif