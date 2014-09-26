#include <set>
#include <climits>
#include <vector>
#include <list>
#include <unistd.h>
#include <sstream>
#include <mpi.h>
#include "PDMlib.h"
#include "Utility.h"
#include "H5PartWriter.h"

namespace
{
void PrintUsageAndAbort(const char* cmd)
{
    std::cerr<<"usage: "<<cmd<<" -f meta_data_file {-c Coordinate Container name} {-s start time} {-e end time} {-d directory}"<<std::endl;
    MPI_Abort(MPI_COMM_WORLD, 1);
}

void ComandLineParser(const int& argc, char** argv, int* start, int* end, std::string* dir_name, std::string* dfi_filename, std::string* coordinate)
{
    int results = 0;
    while((results = getopt(argc, argv, "s:e:c:d:f:")) != -1)
    {
        switch(results)
        {
        case 's':
            *start = atoi(optarg);
            break;

        case 'e':
            *end = atoi(optarg);
            break;

        case 'c':
            *coordinate = optarg;

        case 'd':
            *dir_name = optarg;
            break;

        case 'f':
            *dfi_filename = optarg;
            break;

        case '?':
            PrintUsageAndAbort(argv[0]);
            break;
        }
    }
}
}
int main(int argc, char* argv[])
{
    MPI_Init(&argc, &argv);
    int      nproc, myrank;
    MPI_Comm comm = MPI_COMM_WORLD;
    MPI_Comm_size(comm, &nproc);
    MPI_Comm_rank(comm, &myrank);

    int           start_time = -1;
    int           end_time   = INT_MAX;
    std::string   dir_name("./");
    std::string   dfi_filename;
    std::string   coordinate("Coordinate");
    bool          with_bbox = false;
    ComandLineParser(argc, argv, &start_time, &end_time, &dir_name, &dfi_filename, &coordinate);
    std::ifstream ifs(dfi_filename.c_str());
    if(ifs.fail())
    {
        std::cerr<<"meta data file not found! ("<<dfi_filename<<")"<<std::endl;
        PrintUsageAndAbort(argv[0]);
    }

    PDMlib::PDMlib& pdmlib = PDMlib::PDMlib::GetInstance();
    pdmlib.Init(argc, argv, "FILE_CONVERTER_DUMMY", dfi_filename);

    // 読み込んだファイルの各time stepにおけるプロセス数を取得し
    // minimumのプロセス数以下でコンバータが動作するように制限する

    std::vector<std::string> filenames;
    PDMlib::ListDirectoryContents(dir_name, &filenames);

    const std::string        base_filename = PDMlib::PDMlib::GetInstance().GetBaseFileName();
    //TODO suffix listを取得し、filenamesから関係の無い拡張子のファイルを削除する
    //ListDirectoryContentsを拡張する形で実装する

    std::set<int> time_steps;
    PDMlib::MakeTimeStepList(&time_steps, base_filename, dir_name, start_time, end_time);
    int           min_timestep = *time_steps.begin();

    int           minimum_nproc = INT_MAX;
    for(std::set<int>::iterator it_time = time_steps.begin(); it_time != time_steps.end(); ++it_time)
    {
        std::set<int> ranks;
        for(std::vector<std::string>::iterator it_file = filenames.begin(); it_file != filenames.end(); ++it_file)
        {
            if(PDMlib::get_time_step(*it_file) == *it_time)
            {
                ranks.insert(PDMlib::get_region_number(*it_file));
            }
        }
        minimum_nproc = minimum_nproc > *ranks.rbegin()+1 ? *ranks.rbegin()+1 : minimum_nproc;
    }

    int color = 0;
    if(minimum_nproc < nproc)
    {
        color = myrank/minimum_nproc;
        MPI_Comm_split(MPI_COMM_WORLD, color, myrank, &comm);
    }

    if(color == 0)
    {
        PDMlib::PDMlib::GetInstance().SetComm(comm);

        // メタデータファイルからコンテナ情報を読み込む
        std::vector<PDMlib::ContainerInfo>& containers = pdmlib.GetContainerInfo();

        // 座標コンテナの名前が正しく指定されているか確認
        bool                                coord_found = false;
        PDMlib::ContainerInfo               coord_container;
        for(std::vector<PDMlib::ContainerInfo>::iterator it = containers.begin(); it != containers.end(); ++it)
        {
            if((*it).Name == coordinate)
            {
                coord_found     = true;
                coord_container = *it;
                break;
            }
        }
        if(!coord_found)
        {
            std::cerr<<"Coordinate container not found ! ("<<coordinate<<")"<<std::endl;
            PrintUsageAndAbort(argv[0]);
        }

        std::string filename;
        filename  = pdmlib.GetBaseFileName();
        filename += "_"+PDMlib::to_string(myrank);
        filename += ".h5";

        H5PartWriter::H5PartWriter writer(filename);
        // タイムステップ毎にPDMlibのフィールドデータファイルを読み込んでh5ファイルを出力する
        for(std::set<int>::iterator it_time_step = time_steps.begin(); it_time_step != time_steps.end(); ++it_time_step)
        {
            int    time_step = *it_time_step;
            // 粒子数を取得するために、座標コンテナを読み込む
            size_t length = -1;
            if(coord_container.Type == PDMlib::FLOAT)
            {
                float* ptr = NULL;
                PDMlib::PDMlib::GetInstance().Read(coord_container.Name, &length, &ptr, &time_step);
                delete[] ptr;
            }else if(coord_container.Type == PDMlib::DOUBLE){
                double* ptr = NULL;
                PDMlib::PDMlib::GetInstance().Read(coord_container.Name, &length, &ptr, &time_step);
                delete[] ptr;
            }
            if(length <= 0)
            {
                continue;
            }

            // アトリビュートの設定
            writer.set_attribute(time_step, length/coord_container.nComp);

            // データ出力
            for(std::vector<PDMlib::ContainerInfo>::iterator it = containers.begin(); it != containers.end(); ++it)
            {
                writer.ReadAndWriteContainerSelector(*it, time_step, (*it).Name == coord_container.Name);
            }
        }
    }

    MPI_Finalize();

    return 0;
}
