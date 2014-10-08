###################################################################################
#
# PDMlib - Particle Data Management library
#
#
# Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN. 
# All rights reserved. 
#
###################################################################################

#!/usr/bin/python
# -*- coding: utf-8 -*-
import os.path
import os
import sys
import glob
sys.path.append(os.path.dirname(os.path.abspath(__file__)))
import TextParser


def __get_timestep(filename):
    """
    以下のような形式の文字列からbazを取り出して、整数値として返す

        foo_bar_baz.ext

    引数で渡されたfilenameが期待する書式の文字列となっていなかった場合は
    IndexError, ValueErrorなどの例外が送出される可能性がある
    """
    return int(filename.rsplit('_',1)[1].split('.',1)[0])

def __get_region_number(filename):
    """
    以下のような形式の文字列からbarを取り出して、整数値として返す

        foo_bar_baz.ext

    引数で渡されたfilenameが期待する書式の文字列となっていなかった場合は
    IndexError, ValueErrorなどの例外が送出される可能性がある
    """
    return int(filename.split('_')[-2])

def __get_start_index(n, nproc, myrank):
    index=n//nproc*myrank
    reminder=n%nproc
    if reminder > 0:
        if reminder > myrank:
            index=index+myrank
        else:
            index=index+reminder
    return index

def determine_part(nproc, base_filename, timestep):
    """
    リスタート計算に必要なファイルの分割を決定しファイル名のリストを返す

    戻り値は次のような形式のリストのリスト
    [[Rank0が必要なファイルのリスト], [Rank1が読むファイルのリスト], ・・・ [RankN が読むファイルのリスト]]
    """
    files_to_use=glob.glob(base_filename+"*_"+timestep+".*")
    mproc=0
    for f in files_to_use:
        rank=__get_region_number(f)
        if mproc < rank:
            mproc = rank
    mproc=mproc+1

    start_index = [ __get_start_index(mproc, nproc, x) for x in xrange(nproc+1)]

    result=list()
    for i in xrange(nproc):
        files=list()
        for j in xrange(start_index[i], start_index[i+1]):
            files.extend(glob.glob(base_filename+"*_"+str(j)+"_"+timestep+".*"))
        result.append(files)

    return result


def check_timestep(base_filename, timestep):
    """
    カレントディレクトリにあるファイルから有効なタイムステップを取り出して返す

    引数で負の数が指定された場合は、最新のタイムステップを
    0以上の数が指定された場合は指定されたタイムステップを返す
    ただし、0以上の数が指定された時にそのタイムステップのファイルが1つも存在しない場合は
    プログラム自体を終了させる
    """

    try:
        timesteps=[f.rsplit('_',1)[1].rsplit('.',1)[0]  for f in glob.glob(base_filename+"*") if '_' in f and '.' in f]
    except IndexError:
        pass
    if len(timesteps) < 1:
        sys.exit("no files found")

    if int(timestep) < 0:
        try:
            num_timesteps=[int(x) for x in timesteps]
        except ValueError:
            pass
        timestep=str(sorted(num_timesteps)[-1])
        
    if len(glob.glob(base_filename+"*_"+timestep+".*"))>0:
        return timestep
    else:
        sys.exit("no field data file for "+timestep+" found")


def get_basefilename(dfi_filename):
    """
    引数で渡されたdfiファイルから、"/Header/BaseFileName"の値を返す
    """
    instance=TextParser.getInstanceSingleton()
    TextParser.read(instance, dfi_filename)

    rt = TextParser.getValue(instance,"/Header/BaseFileName")
    if rt[0] != 0:
        raise
    return rt[1]

def print_staging_direction(files):
    """
    標準出力に対してランクディレクトリを使用する方式での
    ステージング指示行を出力する
    """
    print '#PJM --mpi "use-rankdir"'
    for i, files_for_this_rank in enumerate(files):
        for inputfile in files_for_this_rank:
            print '#PJM --stagin "rank ="',i," ",inputfile,"  %r:./"

def __usage_and_exit(cmd):
    print "usage: ",cmd," -i DIR -s STEP NPROC DFI_FILE"
    print """
          NPROC      number of process to run
          DFI_FILE   dfi filename to read

          options:
            -h, --help           print this message
            -i DIR, --input=DIR  specify PATH to DFI_FILE  [default: "./"]
            -s STEP, --step=STEP specify time step to read [default: -1]
          """
    sys.exit(1)


if __name__ == "__main__":
    import getopt

    try:
        opts, args = getopt.gnu_getopt(sys.argv[1:], "i:s:h", ["help", "input=", "step="])
    except getopt.GetoptError, err:
        print (err)
        __usage_and_exit(sys.argv[0])
    

    #デフォルト値の設定
    dir_name="./"
    step="-1"
    nproc=-1
    dfi_filename=None


    for option, optval in opts:
        if option in ("-h", "--help"):
            __usage_and_exit(sys.argv[0])
        elif option in ("-i", "--input"):
            dir_name=optval
        elif option in ("-s", "--step"):
            step=optval
        else:
            assert("unrecognized option")

    #必須の引数のチェック
    #nprocに正の整数が指定されているかどうか
    try:
        nproc=int(args[0])
    except IndexError:
        print "number of procs must be specified\n"
        __usage_and_exit(sys.argv[0])
    except ValueError:
        print "invalid number of procs is specified\n"
        __usage_and_exit(sys.argv[0])
    if nproc <= 0:
        print "number of procs must be larger than 1\n"
        __usage_and_exit(sys.argv[0])

    #dfi_filenameが指定されていて、存在するかどうか
    try:
        dfi_filename=args[1]
    except IndexError:
        print "dfi filename must be specified\n"
        __usage_and_exit(sys.argv[0])

    os.chdir(dir_name)
    if not os.path.exists(dfi_filename):
        sys.exit("dfi file not found!")


    base_filename=get_basefilename(dfi_filename)
    timestep=check_timestep(base_filename, step)
    files=determine_part(nproc, base_filename, timestep)

    print_staging_direction(files)


