#!/bin/bash

# Install script for 3dfier deps on Ubuntu 16.04 LTS
# Parameters:
#   $1 – path to Source Directory where LASzip, libLAS, will be installed, e.g. /opt
# Usage:
#   ./build_deps.sh /opt

# You need to have write permission on the Source Directory
# The script downloads the source files of LASzip, libLAS.

# For trusty you need the ubuntugis and ubuntu-toolchain-r ppa repositories.

# -------
# LASzip
# note that LASzip need to be compiled before libLAS

cd $1
wget https://github.com/LASzip/LASzip/releases/download/v2.2.0/laszip-src-2.2.0.tar.gz
wget https://github.com/LASzip/LASzip/releases/download/v2.2.0/laszip-src-2.2.0.tar.gz.md5
md5sum -c laszip-src-2.2.0.tar.gz.md5
rm laszip-src-2.2.0.tar.gz.md5
tar -xf laszip-src-2.2.0.tar.gz
rm laszip-src-2.2.0.tar.gz
cd laszip-src-2.2.0

# The Makefile need to be modified in order to be compliant to what libLAS
# is looking for. https://github.com/libLAS/libLAS/issues/9
sed -i 's/laszipdir = $(includedir)\//laszipdir = $(includedir)\/laszip/' ./include/laszip/Makefile.am
sed -i 's/laszipdir = $(includedir)\//laszipdir = $(includedir)\/laszip/' ./include/laszip/Makefile.in

# Store the compiled sofware within its directory instead of distributing
# the files in the filesystem. It is easier for libLAS to locate and link
# the executables this way.
mkdir build
./configure --prefix=$1/laszip-src-2.2.0/build
make
make install
make clean

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$1/laszip-src-2.2.0/build/lib

#--------
# libLAS

cd $1
wget http://download.osgeo.org/liblas/libLAS-1.8.1.tar.bz2
wget http://download.osgeo.org/liblas/libLAS-1.8.1.tar.bz2.md5
md5sum -c libLAS-1.8.1.tar.bz2.md5
rm libLAS-1.8.1.tar.bz2.md5
tar -xf libLAS-1.8.1.tar.bz2
rm libLAS-1.8.1.tar.bz2
cd libLAS-1.8.1
mkdir build
mkdir cmake_build
cd cmake_build
# Compile with GDAL and LASzip. GDAL should be found automatically, thus no
# need to provide the link here.
cmake .. \
-DCMAKE_INSTALL_PREFIX=$1/libLAS-1.8.1/build \
-DWITH_GDAL=ON \
-DWITH_LASZIP=ON \
-DLASZIP_INCLUDE_DIR=$1/laszip-src-2.2.0/build/include \
-DLASZIP_LIBRARY=$1/laszip-src-2.2.0/build/lib/liblaszip.so
make
make install
make clean

export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$1/libLAS-1.8.1/build/lib

# test installation, should GDAL, LASzip should be listed
$1/libLAS-1.8.1/build/bin/lasinfo -h | grep -iq "lasinfo (libLAS 1.8.1 with GeoTIFF 1.4.0 GDAL 2.1.0 LASzip 2.2.0)"
