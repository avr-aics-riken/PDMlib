###################################################################################
#
# PDMlib - Particle Data Management library
#
#
# Copyright (c) 2014 Advanced Institute for Computational Science, RIKEN. 
# All rights reserved. 
#
###################################################################################

#!/bin/sh

mpirun -np 1 ../bin/MigrationTest
mpirun -np 1 ../bin/MigrationTest
mpirun -np 1 ../bin/MigrationTest
mpirun -np 2 ../bin/MigrationTest
mpirun -np 2 ../bin/MigrationTest
mpirun -np 1 ../bin/MigrationTest
mpirun -np 2 ../bin/MigrationTest
mpirun -np 1 ../bin/MigrationTest
