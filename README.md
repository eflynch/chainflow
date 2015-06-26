# MaxChain #
This build max external objects for interacting with chainAPI

## Build instructions ##

### Install dependencies ###
> brew install openssl --universal
> brew install cmake

### Clone repository ###
> git clone git@github.com:eflynch/maxchain
> cd maxchain
> git submodule init
> git submodule update

### Build dependencies ###
To do a universal build of jansson which is a library for JSON in C
> cd jansson
> mkdir build
> cd build
> cmake .. "-DCMAKE_OSX_ARCHITECTURES:=x86_64;i386"
> make
> cd ../..

To do a universal build of libwebsockets do the following. I don't really understand why this sequence of operations works. If you try to do the second cmake command without having first done the first you will encounter compiler errors when you run make.
> cd libwebsockets
> mkdir build
> cd build
> cmake .. -DCMAKE_C_FLAGS='-arch i386'
> make
> cmake .. -DCMAKE_C_FLAGS='-arch i386 -arch x86_64'
> make
> cd ../..

### Build externals ###
Each external can be built with it's own XCode project or you can use the convenience Makefile to build them all at once.
> make
