PDMlib
======

Particle Data Management library provides functions to help file I/O of massive particles on distributed parallel environments. PDMlib includes:
 - Management of particles by a DFI file (meta data).
 - data compression by fpzip, zlib, RLE encodings.
 - re-distribution of particle data for a different number of processes at restart.
 - data conversion
 - staging helper for the K computer.


Contributors
------------
- Kenji Ono
- Naoyuki Sogo


Copyright
---------
Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN.
All rights reserved.


License
-------
BSD (2-clause) license


Software Requirement
--------------------
- MPI library
- Text parser library
- zlib
- fpzip
- Zoltan
- HDF5 (option)
