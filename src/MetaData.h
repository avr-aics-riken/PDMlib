#ifndef PDMLIB_METADATA_H
#define PDMLIB_METADATA_H
#include <mpi.h>
#include <vector>
#include <map>
#include "PDMlib.h"
namespace PDMlib
{
class MetaData
{
public:
    MetaData(std::string arg_filename): Version("0.1"),
                                        Comm(MPI_COMM_WORLD),
                                        Communicator("MPI_COMM_WORLD"),
                                        ReadOnly(false),
                                        HeaderOutput(false),
                                        FileName(arg_filename)
    {
        Endian = GetEndian();
        MPI_Comm_size(MPI_COMM_WORLD, &NumCommWorldProc);
        MPI_Comm_rank(MPI_COMM_WORLD, &MyRank);
        NumProc = NumCommWorldProc;
        for(int i = 0; i < 6; i++)
        {
            BoundingBox[i] = -0.1;
        }
    }

    ~MetaData();

private:
    //non-copyable
    MetaData(const MetaData& arg);
    MetaData& operator=(const MetaData& arg);

public:
    //! MetaData情報を読み込む
    //@return 0: 正常終了
    int Read();

    //! MetaData情報を書き出す
    //@return 0: 正常終了
    //
    //Rank0以外では常に何もせずに0を返す
    int Write();

    //! タイムスライス情報を出力する
    //
    //!@return 0  正常終了
    //!@return -1 指定されたコンテナの情報が現在のタイムステップで既に出力されていた
    //
    //Rank0以外では常に何もせずに0を返す
    template<typename T>
    int WriteTimeSlice(const int& TimeStep, const double& Time, T* MinMax, const int& ContainerLength, const std::string& Name);

    //! このオブジェクトのメンバに対するSetterを無効化する
    void SetReadOnly(void){this->ReadOnly = true;}

    //! このオブジェクトのメンバに対するSetterを有効化する
    void SetReadWrite(void){this->ReadOnly = false;}

    //! 対応するフィールドデータのベースファイル名を設定する
    void SetBaseFileName(const std::string filename){if(!ReadOnly) this->BaseFileName = filename;}

    //! 対応するフィールドデータのベースファイル名を取得する
    std::string GetBaseFileName(void) const {return BaseFileName;}

    //! 解析領域全体のBoundingBoxを設定する
    void SetBoundingBox(const double* const BoundingBox)
    {
        if(!ReadOnly)
        {
            for(int i = 0; i < 6; i++)
            {
                this->BoundingBox[i] = BoundingBox[i];
            }
        }
    }

    //! 解析領域全体のBoundingBoxを取得する
    void GetBoundingBox(double* BoundingBox)
    {
        for(int i = 0; i < 6; i++)
        {
            BoundingBox[i] = this->BoundingBox[i];
        }
    }

    //! 粒子計算に使っているコミュニケータを文字列として設定する
    void SetCommunicator(const std::string& Communicator){if(!ReadOnly) this->Communicator = Communicator;}

    //! 粒子計算に使っているコミュニケータをMPI_Comm型の変数として設定する
    void SetComm(const MPI_Comm& Comm)
    {
        if(ReadOnly) return;

        this->Comm = Comm;
        MPI_Comm_size(Comm, &NumProc);
        MPI_Comm_rank(Comm, &MyRank);
    }

    //! コンテナ定義を追加する
    void AddContainer(const ContainerInfo& Container){if(!ReadOnly) Containers.push_back(Container);}

    //! 設定されたコンテナ定義の数を取得する
    int GetNumContainers(void) const {return Containers.size();}

    //! コンテナ情報の一覧を取得する
    void GetContainerInfo(std::vector<ContainerInfo>& Containers) const {Containers = this->Containers;}

    //! 引数で指定された名前のコンテナ情報を取得する
    bool GetContainerInfo(const std::string& name, ContainerInfo* container_info) const;

    //! 引数で指定された名前のコンテナがあるかどうか確認する
    bool FindContainerInfo(const std::string& name
                           ) const;

    //! 単位系の定義を追加する
    void AddUnit(const UnitElem& Unit){if(!ReadOnly) Units.push_back(Unit);}

    //! 単位系の定義を取得する
    void GetUnitInfo(std::vector<UnitElem>& Units) const {Units = this->Units;}

    //! 格納されているコンテナの順番を示す番号を返す
    bool Name2Tag(const std::string& Name, int* Tag) const;

    //! コンテナの名前を返す
    bool Tag2Name(const int& Tag, std::string* Name) const;

    //! 引数で渡された値をもとに、フィールドデータのファイル名を生成する
    void GetFileName(std::string* filename, const std::string& name, const int my_rank, const int& time_step) const;

    //! 自Rankのランク番号を返す
    //
    //! 後述のComm内でのRank番号を返す
    int GetMyRank(void) const {return this->MyRank;}

    //! Comm内のプロセス数を返す
    int GetNumProc(void) const {return this->NumProc;}

    //! Commを返す
    MPI_Comm GetComm(void) const {return this->Comm;}

    //! 現在のタイムステップを返す
    int GetTimeStep(void) const {return this->TimeStep;}

    //! 出力結果を読み込み直して、一致するかどうかをテストするためのルーチン
    //
    //! 通常の実行時(以前のジョブ実行結果を読み込む）は一致しないメンバも含まれるので
    //! テスト以外では使用しないこと
    bool Compare(const MetaData& lhs) const;

private:
    //! 実行中の処理系におけるエンディアンを判定する
    std::string GetEndian(void
                          ) const;

    //! このタイムステップのタイムスライス情報が1つでも書かれているかどうか調べる
    bool is_never_written(void)
    {
        for(std::map<std::string, bool>::iterator it = Written.begin(); it != Written.end(); ++it)
        {
            if((*it).second) return false;
        }
        return true;
    }

    //! このタイムステップのタイムスライス情報が全コンテナについて書かれたかどうかを調べる
    bool is_all_written(void)
    {
        for(std::map<std::string, bool>::iterator it = Written.begin(); it != Written.end(); ++it)
        {
            if(!(*it).second) return false;
        }
        return true;
    }

    //! PDMlibのバージョン
    std::string                Version;

    //! フィールドデータのコンテナ
    std::vector<ContainerInfo> Containers;

    //! 単位系の情報
    std::vector<UnitElem>      Units;

    //! 解析領域のBoundingBox
    //
    // bounding boxの頂点座標を(x1,y1,z1) (x2,y2,z2)とすると
    //   BoundingBox={x1,y1,z1,x2,y2,z2}
    // の順に格納する
    double BoundingBox[6];

    ///                                                            ///
    /// ここから下のデータメンバは入力用と出力用で異なる可能性あり ///
    ///                                                            ///

    //! フィールドデータのエンディアン
    //
    //出力時はライブラリ内で判定して処理系のエンディアンを設定する
    //入力時はDFIファイルから読み込んだ値を格納し
    //実際にPDMlib::Read()が呼ばれた時にフィールドデータのエンディアンと異なっていたら
    //ワーニングを出力する
    std::string Endian;

    //! MPI_COMM_WROLD内のプロセス数
    int         NumCommWorldProc;

    //! PDMlibを呼び出すプロセスが属するコミュニケータ
    //
    // Comm内の全プロセスからPDMlibが呼ばれることを仮定しているが
    // 特にチェックはしていないので、一部のプロセスしか呼ばなかった場合も動作する
    MPI_Comm                    Comm;

    //! Comm内のプロセス数
    int                         NumProc;

    //! Comm内でのランク番号
    int                         MyRank;

    //! Commを識別するためのラベル
    std::string                 Communicator;

    //! 最新の出力を行なったタイムステップ
    int                         TimeStep;

    //! 最新の出力を行なった時刻
    double                      Time;

    //! 最新の出力されたコンテナに含まれる粒子数
    long                        NumParticle;

    //! タイムスライス情報を出力した回数
    int                         NumOutputTimeSlice;

    //! 現在のタイムステップで各コンテナが出力済かどうかを記録するテーブル
    std::map<std::string, bool> Written;

    //! 各変数の値をReadOnlyに設定するフラグ
    bool                        ReadOnly;

    //! DFIファイルのファイル名
    std::string                 FileName;

    //! タイムスライス情報のヘッダ出力を行なったかどうかのフラグ
    bool                        HeaderOutput;

    //! フィールドデータのベースファイル名
    //
    //! 実際に使われるファイル名は "BaseFileName_Rank番号_タイムステップ.拡張子”となり
    //! 拡張子はContainerInfo構造体の中でコンテナ毎に指定されたものが使われる
    std::string BaseFileName;
};
} //end of namespcae
#endif
