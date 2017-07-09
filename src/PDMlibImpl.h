/*
###################################################################################
#
# PDMlib - Particle Data Management library
#
# Copyright (c) 2014-2017 Advanced Institute for Computational Science(AICS), RIKEN.
# All rights reserved.
#
# Copyright (c) 2017 Research Institute for Information Technology (RIIT), Kyushu University.
# All rights reserved.
#
###################################################################################
*/

#ifndef PDMLIB_PDMLIB_IMPL_H
#define PDMLIB_PDMLIB_IMPL_H
#include <vector>
#include <typeinfo>
#include "zoltan_cpp.h"
#include "Utility.h"
#include "MetaData.h"
#include "Read.h"
#ifdef _OPENMP
#include <omp.h>
#endif
#include <set>

//forward declaration
namespace BaseIO
{
class Write;
}
namespace PDMlib
{
struct ContainerPointer
{
    std::string Name;
    SupportedType Type;
    size_t ContainerLength;        //< コンテナの要素数
    void** Container;              //< ユーザコード側にデータを渡す時のポインタ
    size_t size;                   //< buffのデータ長（byte)
    char* buff;                    //< ライブラリ内で一時的にデータを格納する領域
    size_t nComp;                  //< コンテナの1オブジェクトあたりのベクトル長
    bool NIJK_Flag;                //< データの格納順がNIJKであればtrue, IJKNであればfalse
};

//! PDMlibの実装を提供するクラス
class PDMlib::Impl
{
public:
    Impl() : Initialized(false),
        FirstCall(true),
        WriteDFI_FileName("PDMlib.dfi"),
        rMetaData(NULL),
        wMetaData(NULL),
        PM(false)
    {}

    ~Impl()
    {
        int myrank=wMetaData->GetMyRank();
        delete wMetaData;
        wMetaData = NULL;
        delete rMetaData;
        rMetaData = NULL;
        for(std::vector<ContainerPointer*>::iterator it = ContainerTable.begin(); it != ContainerTable.end(); ++it)
        {
            delete *it;
        }
        if(PM)
        {
          std::stringstream ss;
          ss <<"PDMlibPerformance_"<<myrank<<".txt";
          std::ofstream out(ss.str().c_str());
          out<< "elapsed time"<<std::endl;
          for( std::map<std::string, double>::iterator it = elapse.begin(); it!= elapse.end(); ++it)
          {
            out<<it->first<<", "<<it->second<<std::endl;
          }
        }
    }
    void pm_begin(const std::string& label)
    {
      if(!PM) return;
      std::map<std::string, double>::iterator it_t0 = t0.find(label);
      if (it_t0 != t0.end())
      {
          std::cerr <<"WARN: pm_begin() is called with "<<label<<" more than 2 times!"<<std::endl;
      }else{
        t0[label]=MPI_Wtime();
      }
    }

    void pm_end(const std::string& label)
    {
      if(!PM) return;
      double now=MPI_Wtime();
      std::map<std::string, double>::iterator it_t0 = t0.find(label);
      if (it_t0 == t0.end())
      {
          std::cerr <<"WARN: "<<label<<" section is not started!"<<std::endl;
      }else{
        double start=it_t0->second;
        t0.erase(it_t0);

        std::map<std::string, double>::iterator it_elapse = elapse.find(label);
        if (it_elapse != elapse.end())
        {
          elapse[label]+=now-start;
        }else{
          elapse[label]=now-start;
        }
      }
    }

    void Init(const int& argc, char** argv, const std::string& WriteMetaDataFile, const std::string&  ReadMetaDataFile, const bool& Timing)
    {
        if(Initialized)return;
        PM=Timing;

        if(!WriteMetaDataFile.empty())
        {
            WriteDFI_FileName = WriteMetaDataFile;
        }
        // 出力側は必須なので、WriteMetaDataFileが空文字列の時は
        // デフォルトのファイル名を使って"PDMlib.dfi"でオブジェクトを生成する
        wMetaData = new MetaData(WriteDFI_FileName);

        if(!ReadMetaDataFile.empty())
        {
            ReadDFI_FileName = ReadMetaDataFile;
            //読み込み側はオプションなので、ReadMetaDataFileがemptyでは無い時だけ
            //MetaDataオブジェクトを生成し、ファイルを読み込む
            rMetaData = new MetaData(ReadMetaDataFile);
            rMetaData->Read();
            // rMetaDataからContainers, Units, BoundingBox, BaseFileNameをwMetaDataにコピ-
            std::vector<ContainerInfo> rContainers;
            rMetaData->GetContainerInfo(rContainers);
            for(std::vector<ContainerInfo>::iterator it = rContainers.begin(); it != rContainers.end(); ++it)
            {
                wMetaData->AddContainer(*it);
            }

            std::vector<UnitElem> rUnits;
            rMetaData->GetUnitInfo(rUnits);
            for(std::vector<UnitElem>::iterator it = rUnits.begin(); it != rUnits.end(); ++it)
            {
                wMetaData->AddUnit(*it);
            }

            double rBoundingBox[6];
            rMetaData->GetBoundingBox(rBoundingBox);
            wMetaData->SetBoundingBox(rBoundingBox);

            wMetaData->SetBaseFileName(rMetaData->GetBaseFileName());
            rMetaData->SetReadOnly();
        }

        float version;
        Zoltan_Initialize(argc, argv, &version);

        static_my_rank = wMetaData->GetMyRank();

        Initialized    = true;
    }

    //! コンテナの型がメタデータファイルに書かれた型と適合するか確認する
    template<typename T>
    bool TypeCheck(const std::string& name, T** Container)
    {
        ContainerInfo container_info;
        rMetaData->GetContainerInfo(name, &container_info);
        if(container_info.Type == INT32)
        {
            if(typeid(T) != typeid(int))return false;
        }else if(container_info.Type == uINT32){
            if(typeid(T) != typeid(unsigned int))return false;
        }else if(container_info.Type == INT64){
            if(typeid(T) != typeid(long))return false;
        }else if(container_info.Type == uINT64){
            if(typeid(T) != typeid(unsigned long))return false;
        }else if(container_info.Type == FLOAT){
            if(typeid(T) != typeid(float))return false;
        }else if(container_info.Type == DOUBLE){
            if(typeid(T) != typeid(double))return false;
        }else{
            return false;
        }
        return true;
    }

    //! カレントディレクトリ以下にあるファイルを元にタイムステップのリストを作る
    void MakeTimeStep(std::set<int>* time_steps)
    {
        rMetaData->MakeTimeStepList(time_steps);
    }

    //! 読み込む対象のTimeStepを決める
    void DetermineTimeStep(int* TimeStep, const std::set<int>& time_steps)
    {
        if(TimeStep == NULL)
        {
            TimeStep  = new int;
            *TimeStep = -1;
        }

        if(*TimeStep < 0)
        {
            *TimeStep = *(time_steps.rbegin());
        }else{
            *TimeStep = *(--time_steps.upper_bound(*TimeStep));
        }
    }

    //! 自ランクが読み込むファイルのリストを作る
    void MakeFilenameList(std::vector<std::string>* filenames, const int& time_step, const std::string& name, bool read_all_files = false)
    {
        if(read_all_files)
        {
            std::vector<std::string> tmp_filenames;
            ListDirectoryContents(rMetaData->GetPath(), &tmp_filenames);
            std::string   tail1("_");
            tail1 += to_string(time_step);
            ContainerInfo container_info;
            rMetaData->GetContainerInfo(name, &container_info);
            tail1 += "."+container_info.Suffix;
            for(std::vector<std::string>::iterator it = tmp_filenames.begin(); it != tmp_filenames.end(); ++it)
            {
                int pos_underbar = (*it).find_last_of('_');
                if(pos_underbar != std::string::npos)
                {
                    std::string tail((*it).substr(pos_underbar));
                    if(tail == tail1)
                    {
                        filenames->push_back(*it);
                    }
                }
            }
            std::sort(filenames->begin(), filenames->end());
        }else{
            int M       = rMetaData->GetNumProc();
            int N       = wMetaData->GetNumProc();
            int my_rank = wMetaData->GetMyRank();
            int start   = GetStartIndex(M, N, my_rank);
            int end     = GetStartIndex(M, N, my_rank+1);

            for(int i = start; i < end; i++)
            {
                std::string filename;
                rMetaData->GetFileName(&filename, name, i, time_step);
                filenames->push_back(filename);
            }
        }
    }

    //! 必要なファイルを全て読んで、読み込んだデータへのポインタを格納したvectorを作る
    void Read(const std::string& name, const std::vector<std::string>& filenames, std::vector<std::pair<size_t, char*> >* buffers, size_t* total_size)
    {
        ContainerInfo container_info;
        rMetaData->GetContainerInfo(name, &container_info);
        for(std::vector<std::string>::const_iterator it = filenames.begin(); it != filenames.end(); ++it)
        {
            BaseIO::Read* reader = BaseIO::ReadFactory::create(*it, container_info.Compression, enumType2string(container_info.Type), container_info.nComp);
            size_t tmp;
            char** read_buff = new char*;
            *read_buff   = NULL;
            size_t read_size = reader->read(tmp, read_buff);
            *total_size += read_size;
            buffers->push_back(std::make_pair(read_size, *read_buff));
            delete reader;
        }
    }

    // Containerに領域を確保する
    template<typename T>
    void AllocateContainer(const size_t& total_size, const size_t& ContainerLength, T** Container)
    {
        size_t length = total_size/sizeof(T);
        if(length > ContainerLength)
        {
            delete *Container;
            *Container = NULL;
        }
        if(*Container == NULL)
        {
            *Container = new T[length];
        }
    }

    //! buffersに格納されたポインタをContainerが指す領域にコピーする
    template<typename T>
    void CopyBufferToContainer(const std::vector<std::pair<size_t, char*> >&  buffers, size_t* ContainerLength, T* Container)
    {
        *ContainerLength = 0;
        for(std::vector<std::pair<size_t, char*> >::const_iterator it = buffers.begin(); it != buffers.end(); ++it)
        {
            *ContainerLength += CopyBufferToContainer(Container+*ContainerLength, (*it).second, (*it).first);
            delete (*it).second;
        }
    }

    template<typename T>
    size_t CopyBufferToContainer(T* Container, const char* buffer, const size_t& size)
    {
        size_t length = size/sizeof(T);
        for(int i = 0; i < length; i++)
        {
            Container[i] = ((T*)buffer)[i];
        }
        return length;
    }

    int Send(int* buf, int count, int dest, int tag, MPI_Comm comm)
    {
        return MPI_Send(buf, count, MPI_INT, dest, tag, comm);
    }

    int Send(unsigned int* buf, int count, int dest, int tag, MPI_Comm comm)
    {
        return MPI_Send(buf, count, MPI_UNSIGNED, dest, tag, comm);
    }

    int Send(long* buf, int count, int dest, int tag, MPI_Comm comm)
    {
        return MPI_Send(buf, count, MPI_LONG, dest, tag, comm);
    }

    int Send(unsigned long* buf, int count, int dest, int tag, MPI_Comm comm)
    {
        return MPI_Send(buf, count, MPI_UNSIGNED_LONG, dest, tag, comm);
    }

    int Send(float* buf, int count, int dest, int tag, MPI_Comm comm)
    {
        return MPI_Send(buf, count, MPI_FLOAT, dest, tag, comm);
    }

    int Send(double* buf, int count, int dest, int tag, MPI_Comm comm)
    {
        return MPI_Send(buf, count, MPI_DOUBLE, dest, tag, comm);
    }

    int Irecv(int* buf, int count, int source, int tag, MPI_Comm comm, MPI_Request* request)
    {
        return MPI_Irecv(buf, count, MPI_INT, source, tag, comm, request);
    }

    int Irecv(unsigned int* buf, int count, int source, int tag, MPI_Comm comm, MPI_Request* request)
    {
        return MPI_Irecv(buf, count, MPI_UNSIGNED, source, tag, comm, request);
    }

    int Irecv(long* buf, int count, int source, int tag, MPI_Comm comm, MPI_Request* request)
    {
        return MPI_Irecv(buf, count, MPI_LONG, source, tag, comm, request);
    }

    int Irecv(unsigned long* buf, int count, int source, int tag, MPI_Comm comm, MPI_Request* request)
    {
        return MPI_Irecv(buf, count, MPI_UNSIGNED_LONG, source, tag, comm, request);
    }

    int Irecv(float* buf, int count, int source, int tag, MPI_Comm comm, MPI_Request* request)
    {
        return MPI_Irecv(buf, count, MPI_FLOAT, source, tag, comm, request);
    }

    int Irecv(double* buf, int count, int source, int tag, MPI_Comm comm, MPI_Request* request)
    {
        return MPI_Irecv(buf, count, MPI_DOUBLE, source, tag, comm, request);
    }

    template<typename T>
    void migrate_container(T** Container, size_t* ContainerLength, int* recv_counts, const size_t nComp, const std::vector<std::vector<ZOLTAN_ID_TYPE>*>& export_objs)
    {
        pm_begin("migrate_container: prepare to receive");
        // 受信バッファを確保しつつMPI_Irecvを発行
        const int    num_procs  = export_objs.size();
        T**          recv_buffs = new T*[num_procs];
        MPI_Request* requests   = new MPI_Request[num_procs];
        int          src_rank   = 0;
        int          num_recv   = 0;
        for(int i = 0; i < num_procs; i++)
        {
            if(recv_counts[i] > 0)
            {
                recv_buffs[src_rank] = new T[recv_counts[i]*nComp];
                Irecv(recv_buffs[src_rank], recv_counts[i]*nComp, src_rank, MPI_ANY_TAG, wMetaData->GetComm(), &(requests[i]));
                num_recv++;
            }else{
                recv_buffs[src_rank] = NULL;
            }
            src_rank++;
        }
        pm_end("migrate_container: prepare to receive");
        pm_begin("migrate_container: prepare to send");

        // count number of items to send
        std::vector<T*> send_buffs(num_procs);
        std::vector<int> indices(num_procs);
        for(int i = 0; i < num_procs; i++)
        {
          if((export_objs[i])->size() > 0)
          {
            send_buffs[i] = new T [(export_objs[i])->size() * nComp];
          }else{
            send_buffs[i] = NULL;
          }
          indices[i]=0;
        }

        // 転送するデータを転送バッファにコピー
        std::vector<ZOLTAN_ID_TYPE> export_ids;
        for(int dst_rank = 0; dst_rank < num_procs; dst_rank++)
        {
          for (std::vector<ZOLTAN_ID_TYPE>::iterator it = export_objs[dst_rank]->begin(); it != export_objs[dst_rank]->end(); ++it)
          {
            for ( int i=0; i<nComp; i++)
            {
              send_buffs[dst_rank][indices[dst_rank]]=(*Container)[(*it)*nComp+i];
              indices[dst_rank]=indices[dst_rank]+1;
              export_ids.push_back((*it)*nComp+i);
            }
          }
        }

        //元のデータを前に寄せる
        int index_keep = 0;
        if(export_ids.size()>0)
        {
          std::sort(export_ids.begin(), export_ids.end());
          std::vector<ZOLTAN_ID_TYPE>::iterator it=export_ids.begin();
          for(int i = 0; i < *ContainerLength; i++)
          {
            if(i==*it)
            {
              ++it;
            }else{
              (*Container)[index_keep++] = (*Container)[i];
            }
          }
        }else{
          index_keep=*ContainerLength;
        }
        pm_end("migrate_container: prepare to send");
        pm_begin("migrate_container: call MPI_Send");
        int dst_rank = 0;
        int tag      = 0;
        for(std::vector<std::vector<ZOLTAN_ID_TYPE>*>::const_iterator it = export_objs.begin(); it != export_objs.end(); ++it)
        {
            int send_count = ((*it)->size())*nComp;
            if(send_count > 0)
            {
                Send(send_buffs[dst_rank], send_count, dst_rank, tag++, wMetaData->GetComm());
            }
            dst_rank++;
        }
        pm_end("migrate_container: call MPI_Send");
        pm_begin("migrate_container: call MPI_Send 2");
        MPI_Barrier(wMetaData->GetComm()); // 送信しないRankが通り抜けてしまうので、この位置でのBarrierは必須
        pm_end("migrate_container: call MPI_Send 2");
        pm_begin("migrate_container: unpack recieved data");

        // 受信したデータ+転送しなかったデータのサイズで領域を確保
        int room           = *ContainerLength-index_keep; // 送信データ数(=確保済領域の空きサイズを計算)
        *ContainerLength   = index_keep;                  // ContainerLengthを未送信データ数で置き換える
        int sum_recv_count = 0;
        for(int i = 0; i < num_procs; i++)
        {
            sum_recv_count += recv_counts[i]*nComp;
        }
        if(room < sum_recv_count)
        {
            if(*Container == NULL)
            {
                *Container = new T[index_keep+sum_recv_count];
            }else{
                //reallocate
                T* tmp = *Container;
                *Container = new T[index_keep+sum_recv_count];
                for(int i = 0; i < index_keep; i++)
                {
                    (*Container)[i] = tmp[i];
                }
            }
        }

        //受信バッファを元データの末尾に追加
        int recieved_size= 0;
        for(int src_rank = 0; src_rank < num_procs; src_rank++)
        {
          recieved_size+= recv_counts[src_rank]*nComp;
          for(int i = 0; i < recv_counts[src_rank]*nComp; i++)
          {
            (*Container)[index_keep++] = recv_buffs[src_rank][i];
          }
        }
        *ContainerLength += recieved_size;
        pm_end("migrate_container: unpack recieved data");
        pm_begin("migrate_container: post process");

        for(typename std::vector<T*>::iterator it = send_buffs.begin(); it != send_buffs.end(); ++it)
        {
            delete[] *it;
        }
        delete[] requests;
        for(int i = 0; i < num_procs; i++)
        {
            delete[] recv_buffs[i];
        }
        delete[] recv_buffs;
        pm_end("migrate_container: post process");
    }

    void migrate_container_selector(ContainerPointer* container, int* recv_counts, const std::vector<std::vector<ZOLTAN_ID_TYPE>*>& export_objs)
    {
        if((container)->Type == INT32)
        {
            migrate_container((int**)&((container)->buff), &(container->ContainerLength), recv_counts, container->nComp, export_objs);
        }else if((container)->Type == uINT32){
            migrate_container((unsigned int**)&((container)->buff), &(container->ContainerLength), recv_counts, container->nComp, export_objs);
        }else if((container)->Type == INT64){
            migrate_container((long**)&((container)->buff), &(container->ContainerLength), recv_counts, container->nComp, export_objs);
        }else if((container)->Type == uINT64){
            migrate_container((unsigned long**)&((container)->buff), &(container->ContainerLength), recv_counts, container->nComp, export_objs);
        }else if((container)->Type == FLOAT){
            migrate_container((float**)&((container)->buff), &(container->ContainerLength), recv_counts, container->nComp, export_objs);
        }else if((container)->Type == DOUBLE){
            migrate_container((double**)&((container)->buff), &(container->ContainerLength), recv_counts, container->nComp, export_objs);
        }
        container->size = (container->ContainerLength)*GetSize((container)->Type);
    }

    void Set_Geom_Multi_Fn(Zoltan* zz)
    {
        /*
         * memo: Set_Geom_Multi_Fnと同様のquery関数にSet_Geom_Fnがあるが
         *       こちらは一要素づつ取得するためのもの
         *       複数要素を一括して取得する時は_Multi版を使う
         *       他のZoltanのquery関数についても同様
         */
        if(CoordinateContainer->NIJK_Flag)
        {
            if(CoordinateContainer->Type == INT32)
            {
                zz->Set_Geom_Multi_Fn(get_geometry_list_int, CoordinateContainer->buff);
            }else if(CoordinateContainer->Type == uINT32){
                zz->Set_Geom_Multi_Fn(get_geometry_list_uint, CoordinateContainer->buff);
            }else if(CoordinateContainer->Type == INT64){
                zz->Set_Geom_Multi_Fn(get_geometry_list_long, CoordinateContainer->buff);
            }else if(CoordinateContainer->Type == uINT64){
                zz->Set_Geom_Multi_Fn(get_geometry_list_ulong, CoordinateContainer->buff);
            }else if(CoordinateContainer->Type == FLOAT){
                zz->Set_Geom_Multi_Fn(get_geometry_list_float, CoordinateContainer->buff);
            }else if(CoordinateContainer->Type == DOUBLE){
                zz->Set_Geom_Multi_Fn(get_geometry_list_double, CoordinateContainer->buff);
            }
        }else{
            if(CoordinateContainer->Type == INT32)
            {
                zz->Set_Geom_Multi_Fn(get_geometry_list_int_ijkn, CoordinateContainer->buff);
            }else if(CoordinateContainer->Type == uINT32){
                zz->Set_Geom_Multi_Fn(get_geometry_list_uint_ijkn, CoordinateContainer->buff);
            }else if(CoordinateContainer->Type == INT64){
                zz->Set_Geom_Multi_Fn(get_geometry_list_long_ijkn, CoordinateContainer->buff);
            }else if(CoordinateContainer->Type == uINT64){
                zz->Set_Geom_Multi_Fn(get_geometry_list_ulong_ijkn, CoordinateContainer->buff);
            }else if(CoordinateContainer->Type == FLOAT){
                zz->Set_Geom_Multi_Fn(get_geometry_list_float_ijkn, CoordinateContainer->buff);
            }else if(CoordinateContainer->Type == DOUBLE){
                zz->Set_Geom_Multi_Fn(get_geometry_list_double_ijkn, CoordinateContainer->buff);
            }
        }
    }

    bool Migrate()
    {
        pm_begin("Migrate: setup Zoltan");
        MPI_Comm comm = wMetaData->GetComm();
        Zoltan*  zz   = new Zoltan(comm);
        if(zz == NULL)
        {
            std::cerr<<"create zoltan object failed!"<<std::endl;
            return false;
        }

        // parameter setting
        zz->Set_Param("NUM_GID_ENTRIES", "2");  //global id としてunsigned integer 2つを使用する
        zz->Set_Param("NUM_LID_ENTRIES", "1");  //local id としてunsigned integer 1つを使用する (default)
        zz->Set_Param("DEBUG_LEVEL",     "0");  //debug level default値は1
        zz->Set_Param("OBJ_WEIGHT_DIM",  "0");  //ロードバランス計算時にオブジェクトの重みをつけない (default)

        zz->Set_Param("LB_METHOD",       "RCB"); // パーティショニングのアルゴリズム (default)
        zz->Set_Param("RETURN_LISTS",    "ALL"); // import listとexport listの両方を返す (default)
        zz->Set_Param("IMBALANCE_TOL",   "1.1"); // 110%以下のインバランスは許容する (default)

        // register query functions
        zz->Set_Num_Obj_Fn(get_num_object, CoordinateContainer->buff);
        zz->Set_Obj_List_Fn(get_object_list, CoordinateContainer->buff);
        zz->Set_Num_Geom_Fn(get_num_geometry, CoordinateContainer->buff);
        Set_Geom_Multi_Fn(zz);

        int changes;
        int numGidEntries;
        int numLidEntries;
        int numImport;
        ZOLTAN_ID_PTR importGlobalIds;
        ZOLTAN_ID_PTR importLocalIds;
        int*          importProcs;
        int*          importToPart;
        int numExport;
        ZOLTAN_ID_PTR exportGlobalIds;
        ZOLTAN_ID_PTR exportLocalIds;
        int*          exportProcs;
        int*          exportToPart;

        pm_end("Migrate: setup Zoltan");
        pm_begin("Migrate: Zoltan::LB_Partition");
        int rc = zz->LB_Partition(changes, numGidEntries, numLidEntries,
                                  numImport, importGlobalIds, importLocalIds, importProcs, importToPart,
                                  numExport, exportGlobalIds, exportLocalIds, exportProcs, exportToPart);
        pm_end("Migrate: Zoltan::LB_Partition");
        pm_begin("Migrate: check return code of Zoltan::LB_Partition");
        int max_rc;
        int min_rc;
        MPI_Allreduce(&rc, &max_rc, 1, MPI_INT, MPI_MAX, comm);
        MPI_Allreduce(&rc, &min_rc, 1, MPI_INT, MPI_MIN, comm);
        if(max_rc != ZOLTAN_OK || min_rc != ZOLTAN_OK)
        {
            std::cerr<<"Zoltan LB_Partition failed!"<<std::endl;
            return false;
        }
        pm_end("Migrate: check return code of Zoltan::LB_Partition");
        pm_begin("Migrate: prepare to receive");

        // LB_Partitionの結果を元に相手プロセス毎の受信オブジェクト数リストを作成
        const int num_procs   = wMetaData->GetNumProc();
        int*      recv_counts = new int[num_procs];
        for(int i = 0; i < num_procs; i++)
        {
            recv_counts[i] = 0;
        }
        for(int i = 0; i < numImport; i++)
        {
            ++(recv_counts[importProcs[i]]);
        }
        pm_end("Migrate: prepare to receive");
        pm_begin("Migrate: prepare to send");

        // LB_Partitionの結果を元に相手プロセス毎の送信オブジェクトのリストを作成
        // 本当は外側のvectorはarrayで十分だがC++11非対応の環境向けにvectorにしている
        std::vector<std::vector<ZOLTAN_ID_TYPE>*> export_objs(num_procs);
        for(int i = 0; i < num_procs; i++)
        {
            export_objs[i] = new std::vector<ZOLTAN_ID_TYPE>;
        }
        for(int i = 0; i < numExport; i++)
        {
            (export_objs[exportProcs[i]])->push_back(exportLocalIds[i]);
        }

        Zoltan::LB_Free_Part(&importGlobalIds, &importLocalIds, &importProcs, &importToPart);
        Zoltan::LB_Free_Part(&exportGlobalIds, &exportLocalIds, &exportProcs, &exportToPart);
        pm_end("Migrate: prepare to send");
        pm_begin("Migrate: do migration");

        // コンテナ毎にマイグレーションを実行
        for(std::vector<ContainerPointer*>::iterator it = ContainerTable.begin(); it != ContainerTable.end(); ++it)
        {
            migrate_container_selector(*it, recv_counts, export_objs);
        }
        delete[] recv_counts;
        for(std::vector<std::vector<ZOLTAN_ID_TYPE>*>::iterator it = export_objs.begin(); it != export_objs.end(); ++it)
        {
            delete *it;
        }
        delete zz;
        pm_end("Migrate: do migration");
        return true;
    }

    static void get_geometry_list_int(void* data, int num_gid_entries, int num_lid_entries, int num_obj, ZOLTAN_ID_PTR global_ids, ZOLTAN_ID_PTR local_ids, int num_dim, double* geom_vec, int* ierr)
    {
        int* coord  = (int*)data;
        *ierr = ZOLTAN_OK;
        int  length = CoordinateContainer->ContainerLength;
        for(int i = 0; i < length; i++)
        {
            geom_vec[i] = (double)coord[i];
        }
    }

    static void get_geometry_list_uint(void* data, int num_gid_entries, int num_lid_entries, int num_obj, ZOLTAN_ID_PTR global_ids, ZOLTAN_ID_PTR local_ids, int num_dim, double* geom_vec, int* ierr)
    {
        unsigned int* coord = (unsigned int*)data;
        *ierr = ZOLTAN_OK;
        int length          = CoordinateContainer->ContainerLength;
        for(int i = 0; i < length; i++)
        {
            geom_vec[i] = (double)coord[i];
        }
    }

    static void get_geometry_list_long(void* data, int num_gid_entries, int num_lid_entries, int num_obj, ZOLTAN_ID_PTR global_ids, ZOLTAN_ID_PTR local_ids, int num_dim, double* geom_vec, int* ierr)
    {
        long* coord  = (long*)data;
        *ierr = ZOLTAN_OK;
        int   length = CoordinateContainer->ContainerLength;
        for(int i = 0; i < length; i++)
        {
            geom_vec[i] = (double)coord[i];
        }
    }

    static void get_geometry_list_ulong(void* data, int num_gid_entries, int num_lid_entries, int num_obj, ZOLTAN_ID_PTR global_ids, ZOLTAN_ID_PTR local_ids, int num_dim, double* geom_vec, int* ierr)
    {
        unsigned long* coord = (unsigned long*)data;
        *ierr = ZOLTAN_OK;
        int length           = CoordinateContainer->ContainerLength;
        for(int i = 0; i < length; i++)
        {
            geom_vec[i] = (double)coord[i];
        }
    }

    static void get_geometry_list_float(void* data, int num_gid_entries, int num_lid_entries, int num_obj, ZOLTAN_ID_PTR global_ids, ZOLTAN_ID_PTR local_ids, int num_dim, double* geom_vec, int* ierr)
    {
        float* coord  = (float*)data;
        *ierr = ZOLTAN_OK;
        int    length = CoordinateContainer->ContainerLength;
        for(int i = 0; i < length; i++)
        {
            geom_vec[i] = (double)coord[i];
        }
    }

    static void get_geometry_list_double(void* data, int num_gid_entries, int num_lid_entries, int num_obj, ZOLTAN_ID_PTR global_ids, ZOLTAN_ID_PTR local_ids, int num_dim, double* geom_vec, int* ierr)
    {
        double* coord  = (double*)data;
        *ierr = ZOLTAN_OK;
        int     length = CoordinateContainer->ContainerLength;
        for(int i = 0; i < length; i++)
        {
            geom_vec[i] = (double)coord[i];
        }
    }

    static void get_geometry_list_int_ijkn(void* data, int num_gid_entries, int num_lid_entries, int num_obj, ZOLTAN_ID_PTR global_ids, ZOLTAN_ID_PTR local_ids, int num_dim, double* geom_vec, int* ierr)
    {
        int* coord  = (int*)data;
        *ierr = ZOLTAN_OK;
        int  length = (CoordinateContainer->ContainerLength)/3;
        for(int i = 0; i < length; i++)
        {
            geom_vec[3*i]   = (double)coord[i];
            geom_vec[3*i+1] = (double)coord[i+length];
            geom_vec[3*i+2] = (double)coord[i+length*2];
        }
    }

    static void get_geometry_list_uint_ijkn(void* data, int num_gid_entries, int num_lid_entries, int num_obj, ZOLTAN_ID_PTR global_ids, ZOLTAN_ID_PTR local_ids, int num_dim, double* geom_vec, int* ierr)
    {
        unsigned int* coord = (unsigned int*)data;
        *ierr = ZOLTAN_OK;
        int length          = (CoordinateContainer->ContainerLength)/3;
        for(int i = 0; i < length; i++)
        {
            geom_vec[3*i]   = (double)coord[i];
            geom_vec[3*i+1] = (double)coord[i+length];
            geom_vec[3*i+2] = (double)coord[i+length*2];
        }
    }

    static void get_geometry_list_long_ijkn(void* data, int num_gid_entries, int num_lid_entries, int num_obj, ZOLTAN_ID_PTR global_ids, ZOLTAN_ID_PTR local_ids, int num_dim, double* geom_vec, int* ierr)
    {
        long* coord  = (long*)data;
        *ierr = ZOLTAN_OK;
        int   length = (CoordinateContainer->ContainerLength)/3;
        for(int i = 0; i < length; i++)
        {
            geom_vec[3*i]   = (double)coord[i];
            geom_vec[3*i+1] = (double)coord[i+length];
            geom_vec[3*i+2] = (double)coord[i+length*2];
        }
    }

    static void get_geometry_list_ulong_ijkn(void* data, int num_gid_entries, int num_lid_entries, int num_obj, ZOLTAN_ID_PTR global_ids, ZOLTAN_ID_PTR local_ids, int num_dim, double* geom_vec, int* ierr)
    {
        unsigned long* coord = (unsigned long*)data;
        *ierr = ZOLTAN_OK;
        int length           = (CoordinateContainer->ContainerLength)/3;
        for(int i = 0; i < length; i++)
        {
            geom_vec[3*i]   = (double)coord[i];
            geom_vec[3*i+1] = (double)coord[i+length];
            geom_vec[3*i+2] = (double)coord[i+length*2];
        }
    }

    static void get_geometry_list_float_ijkn(void* data, int num_gid_entries, int num_lid_entries, int num_obj, ZOLTAN_ID_PTR global_ids, ZOLTAN_ID_PTR local_ids, int num_dim, double* geom_vec, int* ierr)
    {
        float* coord  = (float*)data;
        *ierr = ZOLTAN_OK;
        int    length = (CoordinateContainer->ContainerLength)/3;
        for(int i = 0; i < length; i++)
        {
            geom_vec[3*i]   = (double)coord[i];
            geom_vec[3*i+1] = (double)coord[i+length];
            geom_vec[3*i+2] = (double)coord[i+length*2];
        }
    }

    static void get_geometry_list_double_ijkn(void* data, int num_gid_entries, int num_lid_entries, int num_obj, ZOLTAN_ID_PTR global_ids, ZOLTAN_ID_PTR local_ids, int num_dim, double* geom_vec, int* ierr)
    {
        double* coord  = (double*)data;
        *ierr = ZOLTAN_OK;
        int     length = (CoordinateContainer->ContainerLength)/3;
        for(int i = 0; i < length; i++)
        {
            geom_vec[3*i]   = (double)coord[i];
            geom_vec[3*i+1] = (double)coord[i+length];
            geom_vec[3*i+2] = (double)coord[i+length*2];
        }
    }

    static void get_object_list(void* data, int num_gid_entries, int num_lid_entries, ZOLTAN_ID_PTR global_ids, ZOLTAN_ID_PTR local_ids, int wgt_dim, float* obj_wgts, int* ierr)
    {
        *ierr = ZOLTAN_OK;
        int num_obj = CoordinateContainer->ContainerLength/3;
        for(int i = 0; i < num_obj; i++)
        {
            global_ids[2*i]   = static_my_rank;
            global_ids[2*i+1] = i;
            local_ids[i]      = i;
        }
    }

    static int get_num_object(void* data, int* ierr)
    {
        *ierr = ZOLTAN_OK;
        return CoordinateContainer->ContainerLength/3;
    }

    static int get_num_geometry(void* data, int* ierr)
    {
        *ierr = ZOLTAN_OK;
        return 3;
    }

    static ContainerPointer* CoordinateContainer;   //< 座標情報を保存したコンテナ
    static int static_my_rank;                      //< 自Rankのランク番号
    std::vector<ContainerPointer*> ContainerTable;  //< RegisterContainer()で渡されたポインタを登録するテーブル
    int BufferSize;                                 //< ファイル出力バッファのサイズ 単位はMiB
    int MaxBufferingTime;                           //< ファイル出力をバッファリングする回数
    std::string ReadDFI_FileName;                   //< 読み出すDFIファイルの名前
    std::string WriteDFI_FileName;                  //< 書きみ出すDFIファイルの名前
    bool Initialized;                               //< 初期化済を示すフラグ
    bool FirstCall;                                 //< Writeの呼び出しが1回目か2回目以降かを示すフラグ
    MetaData* rMetaData;                            //< ファイル入力用のメタデータオブジェクトへのポインタ
    MetaData* wMetaData;                            //< ファイル出力用のメタデータオブジェクトへのポインタ
    bool PM;                                        //< 計時機能を有効にするかどうかのフラグ
    std::map<std::string, double>  elapse;          //< 計時結果を保存するテーブル
    std::map<std::string, double>  t0;              //< 計時開始時刻を保存するテーブル

};

ContainerPointer* PDMlib::Impl::CoordinateContainer;
int PDMlib::Impl::static_my_rank;
} //end of namespace
#endif
