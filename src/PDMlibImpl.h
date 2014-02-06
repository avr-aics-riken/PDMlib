#ifndef PDMLIB_PDMLIB_IMPL_H
#define PDMLIB_PDMLIB_IMPL_H
#include <vector>
#include <typeinfo>
#include "zoltan_cpp.h"
#include "Utility.h"
#include "MetaData.h"
#include "Read.h"

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
    size_t ContainerLength;        // コンテナの要素数
    void** Container;              // ユーザコード側にデータを渡す時のポインタ
    size_t size;                   // buffのデータ長（byte)
    char* buff;                    // ライブラリ内で一時的にデータを格納する領域
    size_t nComp;                  // コンテナの1オブジェクトあたりのベクトル長
};

//! PDMlibの実装を提供するクラス
class PDMlib::Impl
{
public:
    Impl(): Initialized(false),
            FirstCall(true),
            WriteDFI_FileName("PDMlib.dfi"),
            rMetaData(NULL),
            wMetaData(NULL)
    {}

    ~Impl()
    {
        delete wMetaData;
        wMetaData = NULL;
        delete rMetaData;
        rMetaData = NULL;
        for(std::set<ContainerPointer*>::iterator it = ContainerTable.begin(); it != ContainerTable.end(); ++it)
        {
            delete *it;
        }
    }

    void Init(const int& argc, char** argv, const std::string& WriteMetaDataFile, const std::string&  ReadMetaDataFile)
    {
        if(Initialized) return;

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

        Initialized = true;
    }

    //! コンテナの型がメタデータファイルに書かれた型と適合するか確認する
    template<typename T>
    bool TypeCheck(const std::string& name, T** Container)
    {
        ContainerInfo container_info;
        rMetaData->GetContainerInfo(name, &container_info);
        if(container_info.Type == INT32)
        {
            if(typeid(T) != typeid(int)) return false;
        }else if(container_info.Type == uINT32){
            if(typeid(T) != typeid(unsigned int)) return false;
        }else if(container_info.Type == INT64){
            if(typeid(T) != typeid(long)) return false;
        }else if(container_info.Type == uINT64){
            if(typeid(T) != typeid(unsigned long)) return false;
        }else if(container_info.Type == FLOAT){
            if(typeid(T) != typeid(float)) return false;
        }else if(container_info.Type == DOUBLE){
            if(typeid(T) != typeid(double)) return false;
        }else{
            return false;
        }
        return true;
    }

    //! カレントディレクトリ以下にあるファイルを元にタイムステップのリストを作る
    void MakeTimeStep(std::set<int>* time_steps, const std::string& dir_name = "./")
    {
        MakeTimeStepList(time_steps, rMetaData->GetBaseFileName(), dir_name);
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
            ListDirectoryContents("./", &tmp_filenames);
            std::string              tail1("_");
            tail1 += to_string(time_step);
            ContainerInfo            container_info;
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
        BaseIO::Read* reader = BaseIO::ReadFactory::create(container_info.Compression, enumType2string(container_info.Type), container_info.nComp);
        for(std::vector<std::string>::const_iterator it = filenames.begin(); it != filenames.end(); ++it)
        {
            size_t tmp;
            char** read_buff = new char*;
            *read_buff = NULL;
            size_t read_size = reader->read((*it).c_str(), tmp, read_buff);
            *total_size += read_size;
            buffers->push_back(std::make_pair(read_size, *read_buff));
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

        std::vector<std::vector<T>*> send_buffs(num_procs);
        for(int i = 0; i < num_procs; i++)
        {
            send_buffs[i] = new std::vector<T>;
        }

        // 転送するデータを転送バッファにコピーしつつ、元のデータを前に寄せる
        std::vector<std::vector<ZOLTAN_ID_TYPE>::iterator> memo(num_procs);
        for(int i = 0; i < num_procs; i++)
        {
            memo[i] = export_objs[i]->begin();
        }

        int index = 0;
        for(int i = 0; i < *ContainerLength;)
        {
            bool send_flag = false;
            for(int dst_rank = 0; dst_rank < num_procs; dst_rank++)
            {
                std::vector<ZOLTAN_ID_TYPE>::iterator result = std::find(memo[dst_rank], export_objs[dst_rank]->end(), i/nComp);
                if(result != export_objs[dst_rank]->end())
                {
                    memo[dst_rank] = result;
                    send_buffs[dst_rank]->push_back((*Container)[i++]);
                    if(nComp == 3)
                    {
                        send_buffs[dst_rank]->push_back((*Container)[i++]);
                        send_buffs[dst_rank]->push_back((*Container)[i++]);
                    }
                    send_flag = true;
                    break;
                }
            }
            if(!send_flag)
            {
                (*Container)[index++] = (*Container)[i++];
            }
        }
        int dst_rank = 0;
        int tag      = 0;
        for(std::vector<std::vector<ZOLTAN_ID_TYPE>*>::const_iterator it = export_objs.begin(); it != export_objs.end(); ++it)
        {
            int send_count = ((*it)->size())*nComp;
            if(send_count > 0)
            {
                Send(&(send_buffs[dst_rank]->front()), send_count, dst_rank, tag++, wMetaData->GetComm());
            }
            dst_rank++;
        }
        MPI_Barrier(wMetaData->GetComm()); // 送信しないRankが通り抜けてしまうので、この位置でのBarrierは必須

        // 受信したデータ+転送しなかったデータのサイズで領域を確保
        int room = *ContainerLength-index;   // 送信データ数(=確保済領域の空きサイズを計算)
        *ContainerLength -= room;            // 送信したデータ数をContainerLengthから引く
        int sum_recv_count = 0;
        for(int i = 0; i < num_procs; i++)
        {
            sum_recv_count += recv_counts[i]*nComp;
        }
        if(room < sum_recv_count)
        {
            if(*Container == NULL)
            {
                *Container = new T[index+sum_recv_count];
            }else{
                //reallocate
                T* tmp = *Container;
                *Container = new T[index+sum_recv_count];
                for(int i = 0; i < index; i++)
                {
                    (*Container)[i] = tmp[i];
                }
            }
        }

        //受信バッファを元データの末尾に追加
        int counter = 0;
        for(int src_rank = 0; src_rank < num_procs; src_rank++)
        {
            counter += recv_counts[src_rank]*nComp;
            for(int i = 0; i < recv_counts[src_rank]*nComp; i++)
            {
                (*Container)[index++] = recv_buffs[src_rank][i];
            }
        }
        *ContainerLength += counter;

        for(typename std::vector<std::vector<T>*>::iterator it = send_buffs.begin(); it != send_buffs.end(); ++it)
        {
            delete *it;
        }
        delete[] requests;
        for(int i = 0; i < num_procs; i++)
        {
            delete[] recv_buffs[i];
        }
        delete[] recv_buffs;
    }

    void migrate_container_selector(ContainerPointer* container, int* recv_counts, const std::vector<std::vector<ZOLTAN_ID_TYPE>*>& export_objs)
    {
        if((container)->Type == INT32)
        {
            migrate_container((int**)&((container)->buff),           &(container->ContainerLength), recv_counts, container->nComp, export_objs);
        }else if((container)->Type == uINT32){
            migrate_container((unsigned int**)&((container)->buff),  &(container->ContainerLength), recv_counts, container->nComp, export_objs);
        }else if((container)->Type == INT64){
            migrate_container((long**)&((container)->buff),          &(container->ContainerLength), recv_counts, container->nComp, export_objs);
        }else if((container)->Type == uINT64){
            migrate_container((unsigned long**)&((container)->buff), &(container->ContainerLength), recv_counts, container->nComp, export_objs);
        }else if((container)->Type == FLOAT){
            migrate_container((float**)&((container)->buff),         &(container->ContainerLength), recv_counts, container->nComp, export_objs);
        }else if((container)->Type == DOUBLE){
            migrate_container((double**)&((container)->buff),        &(container->ContainerLength), recv_counts, container->nComp, export_objs);
        }
        container->size = (container->ContainerLength)*GetSize((container)->Type);
    }

    void Set_Geom_Multi_Fn(Zoltan* zz)
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
    }

    bool Migrate()
    {
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
        zz->Set_Param("DEBUG_LEVEL", "0");      //debug level default値は1
        zz->Set_Param("OBJ_WEIGHT_DIM", "0");   //ロードバランス計算時にオブジェクトの重みをつけない (default)

        zz->Set_Param("LB_METHOD", "RCB");      // パーティショニングのアルゴリズム (default)
        zz->Set_Param("RETURN_LISTS", "ALL");   // import listとexport listの両方を返す (default)
        zz->Set_Param("IMBALANCE_TOL", "1.1");  // 110%以下のインバランスは許容する (default)

        // register query functions
        zz->Set_Num_Obj_Fn(get_num_object, CoordinateContainer->buff);
        zz->Set_Obj_List_Fn(get_object_list, CoordinateContainer->buff);
        zz->Set_Num_Geom_Fn(get_num_geometry, CoordinateContainer->buff);
        Set_Geom_Multi_Fn(zz);

        int           changes;
        int           numGidEntries;
        int           numLidEntries;
        int           numImport;
        ZOLTAN_ID_PTR importGlobalIds;
        ZOLTAN_ID_PTR importLocalIds;
        int*          importProcs;
        int*          importToPart;
        int           numExport;
        ZOLTAN_ID_PTR exportGlobalIds;
        ZOLTAN_ID_PTR exportLocalIds;
        int*          exportProcs;
        int*          exportToPart;

        int           rc = zz->LB_Partition(changes, numGidEntries, numLidEntries,
                                            numImport, importGlobalIds, importLocalIds, importProcs, importToPart,
                                            numExport, exportGlobalIds, exportLocalIds, exportProcs, exportToPart);
        int           max_rc;
        int           min_rc;
        MPI_Allreduce(&rc, &max_rc, 1, MPI_INT, MPI_MAX, comm);
        MPI_Allreduce(&rc, &min_rc, 1, MPI_INT, MPI_MIN, comm);
        if(max_rc != ZOLTAN_OK || min_rc != ZOLTAN_OK)
        {
            std::cerr<<"Zoltan LB_Partition failed!"<<std::endl;
            return false;
        }

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

        // LB_Partitionの結果を元に相手プロセス毎の送信オブジェクトのリストを作成
        // 本当は外側のvectorはarrayにしたいがC++11非対応の環境向けにvectorにしている
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

        // コンテナ毎にマイグレーションを実行
        for(std::set<ContainerPointer*>::iterator it = ContainerTable.begin(); it != ContainerTable.end(); ++it)
        {
            migrate_container_selector(*it, recv_counts, export_objs);
        }
        delete[] recv_counts;
        for(std::vector<std::vector<ZOLTAN_ID_TYPE>*>::iterator it = export_objs.begin(); it != export_objs.end(); ++it)
        {
            delete *it;
        }
        delete zz;
        return true;
    }

    static void get_geometry_list_int(void* data, int num_gid_entries, int num_lid_entries, int num_obj, ZOLTAN_ID_PTR global_ids, ZOLTAN_ID_PTR local_ids, int num_dim, double* geom_vec, int* ierr)
    {
        int* coord = (int*)data;
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
        int           length = CoordinateContainer->ContainerLength;
        for(int i = 0; i < length; i++)
        {
            geom_vec[i] = (double)coord[i];
        }
    }

    static void get_geometry_list_long(void* data, int num_gid_entries, int num_lid_entries, int num_obj, ZOLTAN_ID_PTR global_ids, ZOLTAN_ID_PTR local_ids, int num_dim, double* geom_vec, int* ierr)
    {
        long* coord = (long*)data;
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
        int            length = CoordinateContainer->ContainerLength;
        for(int i = 0; i < length; i++)
        {
            geom_vec[i] = (double)coord[i];
        }
    }

    static void get_geometry_list_float(void* data, int num_gid_entries, int num_lid_entries, int num_obj, ZOLTAN_ID_PTR global_ids, ZOLTAN_ID_PTR local_ids, int num_dim, double* geom_vec, int* ierr)
    {
        float* coord = (float*)data;
        *ierr = ZOLTAN_OK;
        int    length = CoordinateContainer->ContainerLength;
        for(int i = 0; i < length; i++)
        {
            geom_vec[i] = (double)coord[i];
        }
    }

    static void get_geometry_list_double(void* data, int num_gid_entries, int num_lid_entries, int num_obj, ZOLTAN_ID_PTR global_ids, ZOLTAN_ID_PTR local_ids, int num_dim, double* geom_vec, int* ierr)
    {
        double* coord = (double*)data;
        *ierr = ZOLTAN_OK;
        int     length = CoordinateContainer->ContainerLength;
        for(int i = 0; i < length; i++)
        {
            geom_vec[i] = (double)coord[i];
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

    //! 座標情報を保存したコンテナ
    static ContainerPointer* CoordinateContainer;

    //! 自Rankのランク番号
    //
    static int                  static_my_rank;

    //! RegisterContainer()で渡されたポインタを登録するテーブル
    std::set<ContainerPointer*> ContainerTable;

    //! ファイル出力バッファのサイズ 単位はMiB
    int                         BufferSize;

    //! ファイル出力をバッファリングする回数
    int                         MaxBufferingTime;

    //! 読み出すDFIファイルの名前
    std::string                 ReadDFI_FileName;

    //! 書きみ出すDFIファイルの名前
    std::string                 WriteDFI_FileName;

    //! 初期化済を示すフラグ
    bool                        Initialized;

    //! Writeの呼び出しが1回目か2回目以降かを示すフラグ
    bool                        FirstCall;

    //! ファイル入力用のメタデータオブジェクトへのポインタ
    MetaData*                   rMetaData;

    //! ファイル出力用のメタデータオブジェクトへのポインタ
    MetaData*                   wMetaData;
};

ContainerPointer* PDMlib::Impl::CoordinateContainer;
int               PDMlib::Impl::static_my_rank;
} //end of namespace
#endif
