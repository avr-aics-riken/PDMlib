/*
 * PDMlib - Particle Data Management library
 *
 *
 * Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
 * All rights reserved.
 *
 */

/*
 * google testをパラレルに実行するためのmain関数
 * テストは各プロセスで独立に実行され、結果の集計などは行なわないため
 * プロセス間でテスト結果に違いが生じる可能性がある
 * テスト結果を確認する時は必ず全プロセスの出力を確認すること
 */
#include <mpi.h>
#include "gtest/gtest.h"

GTEST_API_ int main(int argc, char **argv)
{
  MPI_Init(&argc, &argv);
  testing::InitGoogleTest(&argc, argv);

  int rt = RUN_ALL_TESTS();

  MPI_Finalize();
  return rt;
}
