#!/bin/bash
mkdir release
echo "Created ./release/"
cd release
../configure \
        --sysconfdir=/etc \
        --localstatedir=/var \
        --disable-nls \
        --disable-debug \
        --enable-openmp \
        --enable-xspice \
        --enable-pss \
        --enable-cider \
        --with-ngshared \
        --enable-sp


echo "Configured build"
make
make install