mkdir -p softlinks

cd softlinks
ln -f -s /Volumes/ece391cc/cross/bin/i686-elf-ar i686-tedkos-ar
ln -f -s /Volumes/ece391cc/cross/bin/i686-elf-as i686-tedkos-as
ln -f -s /Volumes/ece391cc/cross/bin/i686-elf-gcc i686-tedkos-gcc
ln -f -s /Volumes/ece391cc/cross/bin/i686-elf-gcc i686-tedkos-cc
ln -f -s /Volumes/ece391cc/cross/bin/i686-elf-ranlib i686-tedkos-ranlib
ln -f -s /Volumes/ece391cc/cross/bin/i686-elf-ld i686-tedkos-ld
export PATH=$PATH:`pwd`
cd ..

cd userland
mkdir -p build-newlib

cd build-newlib
../newlib-2.2.0-1/configure --prefix=/ --target=i686-tedkos
make all
make DESTDIR=`pwd`/../clib install

