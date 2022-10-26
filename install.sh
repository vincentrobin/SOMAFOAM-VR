#!/bin/bash

cd ${0%/*} || exit 1    # run from this directory

#commande a faire la premiere fois seulement (ca marche rarement du premier coup...)
./alias

source "$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"/etc/bashrc

echo "*---------------------------*LTPS Installation Initiated*--------------------------------*"
echo

# gcc5.5 (V.Robin)
mkdir usr
cd gcc-5.5.0
patch -p1 < glib.sanitizer.patch
contrib/download_prerequisites
cd ..
mkdir build-gcc && cd build-gcc

# configure gcc-5
../gcc-5.5.0/configure -v --build=x86_64-linux-gnu --host=x86_64-linux-gnu  --target=x86_64-linux-gnu --prefix=$WM_PROJECT_DIR/usr/gcc-5.5  --enable-checking=release --enable-languages=c,c++,fortran  --enable-shared --disable-multilib --program-suffix=-5
# Change 4 to any number based on the available CPU cores
make -j 4 > log.make 2>&1
# Inspect log.make for possible errors
make install-strip
cd ..

# wmake is required for subsequent targets
( cd wmake/src && make )

# build ThirdParty sources
( cd metis && ./AllMake )

. $WM_PROJECT_DIR/etc/settings.sh

# build all libraries and applications
src/Allwmake

applications/Allwmake

echo
echo "*--------------------------*LTPS Installation Finished*----------------------------------*"
