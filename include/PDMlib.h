/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */

#ifndef PDMLIB_PDMLIB_H
#define PDMLIB_PDMLIB_H
#include <mpi.h>
#include <string>
#include <vector>
#include <set>
#include <climits>

namespace PDMlib
{
enum SupportedType {INT32, uINT32, INT64, uINT64, FLOAT, DOUBLE};

enum StorageOrder {NIJK, IJKN};

//! コンテナ情報を表す構造体
struct ContainerInfo
{
    std::string Name;
    std::string Annotation;
    std::string Compression;
    SupportedType Type;
    std::string Suffix;
    int nComp;
    StorageOrder VectorOrder;
};

//! 単位系を表す構造体
//
//cio_UnitElemと同じ内容で、メソッドを全て省略したもの
struct UnitElem
{
    std::string Name;         ///<単位の種類名(Length,Velovity,,,)
    std::string Unit;         ///<単位のラベル(m,m/s,Pa,,,,)
    double reference;         ///<規格化に用いたスケール
    double difference;        ///<差
    bool BsetDiff;            ///<differenceの有無（false:なし true:あり）
};

//! タイムスライス情報を表わす構造体
template<typename T>
struct TimeSliceInfo
{
    //!MinMax値
    //
    //0,2,4,6が最小値
    //1,3,5,7が最大値
    //スカラの場合は0,1のみ使用
    //ベクトルの場合は0,1を合成値に使用し、後は成分毎の値を順に入れる
    T MinMax[8];
};

class PDMlib
{
    //Singleton patturn

private:
    PDMlib();
    PDMlib(const PDMlib& obj);
    PDMlib& operator=(const PDMlib& obj);

public: ~PDMlib();
    static PDMlib& GetInstance()
    {
        static PDMlib instance;
        return instance;
    }

public:
    //! @brief PDMlibの初期化を行う
    //! @param [in] argc   main()のargc
    //! @param [in] argv   main()のargv
    //! @param [in] WriteMetaDataFile メタデータの出力先ファイル名
    //! @param [in] ReadMetaDataFile  メタデータの入力元ファイル名
    //
    //ReadMetaDataFileにファイル名として""が指定された時は
    //読み込み用のメタデータはインスタンスを作らない
    void Init(const int& argc, char** argv, const std::string& WriteMetaDataFile, const std::string& ReadMetaDataFile = "");

    //! @brief 1コンテナ分のフィールドデータを読み込む
    //! @param [in]    Name             読み込むコンテナの名前（ContainerInfo::Nameで指定した文字列）
    //! @param [inout] ContainerLength  Containerのサイズ（要素数）終了時は実際に格納されている要素数
    //! @param [inout] Container        データを格納する領域へのポインタのポインタ
    //! @param [inout] TimeStep         読み込む対象のタイムステップ 終了時は実際に読み込んだTimeStep
    //
    // ContainerとしてNULLが渡されると内部で動的に領域を確保します。
    // この場合はユーザコード側で明示的にdelteしてください。
    // @attention ContainerにNULLを指定するのではなく、NULLが指定されたポインタ変数ヘのポインタを渡すこと
    //
    // ContainerとしてNULL以外が渡された場合は
    // ファイルに格納されていたデータサイズ >= ContainerLengthの場合
    //   **Containerはdeleteされた上で必要十分な大きさの領域として再確保されます
    //   @attention この可能性がある場合、Containerは必ず動的に確保した領域を指した状態で呼び出してください
    // ファイルに格納されていたデータサイズ < ContainerLengthの場合
    //   **Container の先頭から順に必要な領域だけを使って値を上書きして返します。
    // いずれの場合もContainerLengthには、使用した要素数を格納して返します。
    //
    // *TimeStepとして負の値が指定された時、またはTimeStepがNULLの時はカレントディレクトリ以下の
    // 最新のタイムステップのデータを読み込みます。
    // 指定されたタイムステップのデータが存在しない場合は、それより古いもののうち最新のデータを読み込みます。
    //
    // read_all_filesにtrueが指定された場合は、全Rankが指定されたコンテナ/タイムステップの全データを重複して読み込みます
    // コンバータ等での読み込みなど、通常のデータ分散とは異なる割り当てで読み込む場合に使用する
    template<typename T>
    int Read(const std::string& Name, size_t* ContainerLength, T** Container, int* TimeStep = NULL, bool read_all_files = false);

    //! @breif データ読み込みに使用するコンテナを登録する
    //! @param [in] Name      コンテナの名前
    //! @param [in] Container コンテナのデータを格納する領域へのポインタのポインタ
    //! @return  0 正常終了
    //! @return -1 初期化される前に呼び出された
    //! @return -2 すでに登録済みのコンテナに対してポインタを登録しようとした
    //
    //! 同じ名前のコンテナに対して複数回呼び出したときは、最初に登録されたポインタのみが有効
    template<typename T>
    int RegisterContainer(const std::string& Name, T** Container) const;

    //! @brief RegisterContainerで登録した全てのコンテナに対してフィールドデータを読み込む
    //! @param [inout] TimeStep             読み込む対象のタイムステップ
    //! @param [in]    MigrationFlag        ロードバランスのためのマイグレーションを行うかどうかのフラグ
    //! @param [in]    CoordinateContainer  座標を格納しているコンテナの名前
    //! @return  読み込んだデータの数(ベクトルデータは3要素で1とする）
    size_t ReadAll(int* TimeStep = NULL, const bool& MigrationFlag = false, const std::string& CoordinateContainer = "Coordinate");

    //! @brief フィールドデータを出力する
    //! @param [in] Name             出力するコンテナの名前（ContainerInfo::Nameで指定した文字列）
    //! @param [in] ContainerLength  出力するデータの要素数
    //! @param [in] Container        データを格納する領域へのポインタ
    //! @param [in] MinMax[8]        出力するデータの最大/最小値を格納する。出力が不要な場合はNULLを渡すこと
    //! @param [in] NumComp          データがスカラー（1)かベクトル（3）かを示す
    //! @param [in] TimeStep         現在のタイムステップ
    //! @param [in] Time             現在の時刻
    //
    //! @return  0以上 正常終了（出力サイズが返ってくる）
    //! @return -1     Init()が呼ばれる前に呼ばれた
    //! @return -2     Tagの値が不正値 （負またはコンテナ数以上の値）
    //! @return -3     ContainerLengthが不正値 (負の値）
    //! @return -4     ContaierにNULLポインタが指定されていた
    //! @return -5     NumCompが不正値（1または3以外の値）
    //! @return -6     TimeStepが不正値（負の値）
    //
    template<typename T>
    int Write(const std::string&Name, const size_t &ContainerLength, T*Container, T MinMax[8], const int& NumComp, const int& TimeStep, const double& Time);

    //
    // 出力用メタデータオブジェクトに対するgetter/setter
    //
    //! 出力用として指定されたDFIファイルに出力するコンテナ情報を追加します。
    int AddContainer(const ContainerInfo& Container);

    //!出力するフィールドデータのベースファイル名を指定します。
    int SetBaseFileName(const std::string& FileName);

    /// フィールドデータの出力先ディレクトリを指定します。
    int SetPath(const std::string& path);

    //!解析領域全体のbounding boxを設定します。
    void SetBoundingBox(double* bbox);

    //! ファイル出力を行うプロセスが所属するコミュニケータを設定します
    void SetComm(const MPI_Comm& comm);

    //! 出力ファイル名のフォーマットを指定します
    int SetFileNameFormat(const std::string& format);

    //
    // 入力用メタデータオブジェクトに対するgetter/setter
    //
    //! 入力用として指定されたDFIファイル内で定義されているコンテナ情報を取得します
    std::vector<ContainerInfo>& GetContainerInfo(void);

    //! 入力用として指定されたフィールドデータのベースファイル名を取得します
    std::string GetBaseFileName(void);

    /// フィールドデータの入力元ディレクトリを取得します
    std::string GetPath(void);

    //! 入力用として指定されたDFIファイルからBoundingBoxの情報を取得します
    void GetBoundingBox(double* bbox);

    //! フィールドデータのファイル名フォーマットがrank_stepかどうかを判定します。
    bool is_rank_step(void);

    //
    // Setter/Getter for PDMlib parameter
    //
    //!出力バッファのサイズを設定します
    void SetBufferSize(const int& BufferSize);

    //!出力バッファのサイズを取得します
    int GetBufferSize(void);

    //!出力をバッファリングする最大の回数を設定します。
    void SetMaxBufferingTime(const int& MaxBufferingTime);

    //!出力をバッファリングする最大の回数を取得します。
    int GetMaxBufferingTime(void);

    //
    // Utility function for converter
    //
    //! 存在するフィールドデータのファイルからタイムステップの一覧を作成して返す
    int MakeTimeStepList(std::set<int>* time_steps, const int& start_time = 0, const int& end_time = INT_MAX, const std::string& wild_card="*") const;
//
//pimpl idiom
//

private:
    class Impl; // forward declaration
    Impl* pImpl;
};
} //end of namespace
#endif
