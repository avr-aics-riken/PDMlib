#CMAKE_SYSTEM_NAMEにUnixPathsを指定することで
#無理矢理Platform/UnixPathsを読み込んで、UNIX=Trueを設定させている
#CMakeCXXInformation.cmake内で
#CMAKE_CXX_OUTPUT_EXTENSIONが.objに設定されてしまうことの回避策
set(CMAKE_SYSTEM_NAME UnixPaths)
set(CMAKE_SYSTEM_VERSION 1.2.0)

include(CMakeForceCompiler)
CMAKE_FORCE_C_COMPILER(mpifccpx GNU)
CMAKE_FORCE_CXX_COMPILER(mpiFCCpx GNU)
CMAKE_FORCE_Fortran_COMPILER(frtpx GNU)

#CMAKE_FIND_ROOT_PATHを指定すると、全てのライブラリ探索の起点が変わるため
#TextParserやfpzip, Zoltanなどのユーザがインストールしていると思われるライブラリの
#探索に失敗する。
#set(CMAKE_FIND_ROOT_PATH /opt/FJSVXosDevkit/sparc64fx/target)

# set(CMAKE_SYSROOT /opt/FJSVXosDevkit/sparc64fx/target)
# set(CMAKE_INCLUDE_PATH ${CMAKE_SYSROOT}/usr/include)
# set(CMAKE_LIBRARY_PATH ${CMAKE_SYSROOT}/usr/lib64)
set(CMAKE_INCLUDE_PATH /opt/FJSVXosDevkit/sparc64fx/target/usr/include)
set(CMAKE_LIBRARY_PATH /opt/FJSVXosDevkit/sparc64fx/target/usr/lib64)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_CXX_FLAGS "-Xg -std=gnu++03"  CACHE STRING "" FORCE)
set(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_EXE_LINKER_FLAGS} -Kparallel"  CACHE STRING "" FORCE)
