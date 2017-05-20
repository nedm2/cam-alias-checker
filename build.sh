#!/bin/bash

mkdir install
export CAM_INSTALL=$PWD/install
export PKG_CONFIG_PATH=$CAM_INSTALL/lib/pkgconfig

cd libplatform
./bootstrap
./configure --prefix=$CAM_INSTALL
make install
cd ..

cd xanlib
./bootstrap
./configure --prefix=$CAM_INSTALL
make install
cd ..


cd libcam
./bootstrap
./configure --prefix=$CAM_INSTALL --enable-cambinary
make install
cd ..
