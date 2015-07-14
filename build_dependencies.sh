brew install cmake

git submodule init
git submodule update

cd jansson
mkdir build
cd build
cmake .. "-DCMAKE_OSX_ARCHITIECTURES:=x86_64;i386"
make
cd ../..

cd libwebsockets
mkdir build
cd build
cmake .. -DCMAKE_C_FLAGS='-arch i386'
make
cmake .. -DCMAKE_C_FLAGS='-arch i386 -arch x86_64'
make
cd ../..
