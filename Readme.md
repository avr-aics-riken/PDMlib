
# PDMlib - Particle Data Management library


## Outline

Particle Data Management library provides functions to help file I/O of massive particles on distributed parallel environments. PDMlib includes:
 - Management of particles by a DFI file (meta data).
 - data compression by fpzip, zlib, RLE encodings.
 - re-distribution of particle data for a different number of processes at restart.
 - data conversion
 - staging helper for the K computer.


## Copyright
- Copyright (c) 2014-2017 Advanced Institute for Computational Science, RIKEN.
All rights reserved.
- Copyright (c) 2017 Research Institute for Information Technology(RIIT), Kyushu University. All rights reserved.


## Software Requirement

- MPI library
- Text parser library
- zlib
- fpzip
- Zoltan
- HDF5 (option)


## HOW TO BUILD

### Build

~~~
$ export PDM_HOME=/hogehoge
$ mkdir build
$ cd build
$ cmake [options] ..
$ make
$ sudo make install
~~~


### Options

`-D INSTALL_DIR=` *Install_directory*

>  Specify the directory that this library will be installed. Built library is installed at `install_directory/lib` and the header files are placed at `install_directory/include`. The default install directory is `/usr/local/UDMlib`.

`-D with_util=` {yes | no}

>  Install utility tools. The default of this option is yes.
>
>  In case of cross-compilation, the frm tool is not installed. On the other hand, in case of native compile environment, `-D with_util=yes` indicates that all utilities are installed at the same time as CDMlib is compiled.

`-D with_MPI=` {yes | no}

>  If you use an MPI library, specify `with_MPI=yes`, the default is yes.

`-D with_TP =` *TextParser_directory*

> Specify the directory path that TextParser is installed.

`-D with_ZIP=` *ZIP_directory*

> Specify the directory path that ZIP is installed.

`-D with_FPZIP=` *FPZIP_directory*

> Specify the directory path that FPZIP is installed.

`-D with_HDF5=` *HDF5_directory*

> Specify the directory path that HDF5 is installed.

`-D with_ZOLTAN=` *Zoltan_directory*

> Specify the directory path that Zoltan library is installed. Zoltan library must be the same compiler with the one used to build OpenMPI library.

`-D with_example=` {no | yes}

>  This option turns on compiling sample codes. The default is no.

`-D CMAKE_BUILD_TYPE=DEBUG`

>  Build debung version, no optimization, timing off, and debug print is on.

`-D build_vtk_converter=` {yes|no}

>  Build VTK converter, default is yes.

`-D build_fv_converter=` {no|yes}

>  Build FieldView converter program, default is no.

`-Dbuild_h5part_converter=`
>  Build H5Part converter, default is yes.

`-Dbuild_tests=` {yes|no}

>  Build test programs, default is yes.


## Configure Examples

`$ export PDM_HOME=hogehoge`

In following examples, assuming that TextParser, ZIP, FPZIP, HDF5, and Zoltan libraries are installed under the PDM_HOME directory. If not, please specify applicable directory path.


### INTEL/GNU compiler

~~~
$ cmake -DINSTALL_DIR=${PDM_HOME}/PDMlib \
        -Dwith_MPI=yes \
        -Dwith_util=no \
        -Dwith_example=no \
        -Dwith_TP=${PDM_HOME}/TextParser \
        -Dwith_ZOLTAN=${PDM_HOME}/Zoltan \
        -Dwith_ZIP=${PDM_HOME}/ZIP \
        -Dwith_FPZIP=${PDM_HOME}/FPZIP \
        -Dwith_HDF5=${PDM_HOME}/HDF5 \
        -Dbuild_vtk_converter=no \
        -Dbuild_fv_converter=no \
        -Dbuild_h5part_converter=no \
        -Dbuild_tests=no ..
~~~


### FUJITSU compiler / FX10, FX100, K on login nodes (Cross compilation)

~~~
$ cmake -DINSTALL_DIR=${CDM_HOME}/CDMlib \
            -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain_fx10.cmake \
            -Dwith_MPI=yes \
            -Dwith_example=no \
            -Dwith_util=yes \
            -Dwith_TP=${CDM_HOME}/TextParser \
            -Dwith_CPM=${CDM_HOME}/CPMlib \
            -Dwith_HDF=no \
            -Dwith_NetCDF=no \
            -Denable_BUFFER_SIZE=no ..

$ cmake -DINSTALL_DIR=${CDM_HOME}/CDMlib \
            -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain_fx100.cmake \
            -Dwith_MPI=yes \
            -Dwith_example=no \
            -Dwith_util=yes \
            -Dwith_TP=${CDM_HOME}/TextParser \
            -Dwith_CPM=${CDM_HOME}/CPMlib \
            -Dwith_HDF=no \
            -Dwith_NetCDF=no \
            -Denable_BUFFER_SIZE=no ..

$ cmake -DINSTALL_DIR=${CDM_HOME}/CDMlib \
            -DCMAKE_TOOLCHAIN_FILE=../cmake/Toolchain_K.cmake \
            -Dwith_MPI=yes \
            -Dwith_example=no \
            -Dwith_util=yes \
            -Dwith_TP=${CDM_HOME}/TextParser \
            -Dwith_CPM=${CDM_HOME}/CPMlib \
            -Dwith_HDF=no \
            -Dwith_NetCDF=no \
            -Denable_BUFFER_SIZE=no ..
~~~



## Note for building required libraries


### Zlib

~~~
$ tar xvfz zlib-x.x.x.tar.gz
$ cd zlib-x.x.x
$ export CC=icc && ./configure --prefix=${HOME}/Zlib
$ make && make install
~~~

* In case of Futjitsu compiler CC=fccpx


### FPZIP

- [fpzip](https://computation.llnl.gov/projects/floating-point-compression)

- Use fpzip-1.0.1 for a while, because `fpzip_memory_read()` is only in this version.

~~~
$ tar xvfz fpzip-x.x.x.tar.gz
$ cd fpzip-x.x.x/src
$ export CXX=icpc CXXFLAGS=-O3
$ vi Makefile
$ make
~~~

- After compiling, copy inc/ and lib/ directories manually to ${hoge}/FPZIP/incliude, lib/.



### HDF5

~~~
$ tar xvfz hdf5-x.x.x.tar.gz
$ cd hdf5-x.x.x
$ export CC=icc CFLAGS=-O3
$ export CXX=icpc CXXFLAGS=-O3
$ export FC=ifort FCFLAGS=-O3
$ ./configure --prefix=${HOME}/HDF5 --enable-64bit \
              --enable-fortran \
              --with-zlib=${HOME}/Zlib/include,${HOME}/Zlib/lib \
              --enable-cxx \
              --with-szip=${HOME}/SZIP
$ make && make install
~~~


### Zoltan

~~~
$ tar xvfz zoltan_distrib_v3.81.tar.gz
$ cd Zoltan_v3.81
$ mkdir BUILD_DIR
$ cd BUILD_DIR
$ export CC=mpicc CFLAGS=-O3 FC=mpif90 FCFLAGS=-O3
$ ../configure --prefix=${HOME}/ZOLTAN --enable-mpi --enable-f90interface
$ make everything
$ make install
~~~


## Examples

* If you specify the test option by `-Denable_example=yes`, you can
execute the intrinsic tests by;

	`$ make test` or `$ ctest`

* The detailed results are written in `BUILD/Testing/Temporary/LastTest.log` file.
Meanwhile, the summary is displayed for stdout.


## Contributors

- Kenji Ono            _keno@cc.kyushu-u.ac.jp_, _keno@riken.jp_
- Naoyuki Sogo
* Jorji     Nonaka     _jorji@riken.jp_
