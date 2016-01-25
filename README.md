# ChainFlow #
This build max external objects for interacting with chainAPI

## Build instructions ##

### Install dependencies ###
```
brew install cmake
```

### Clone repository ###
```
git clone git@github.com:eflynch/chainflow
cd chainflow
git submodule init
git submodule update
```

### Build dependencies ###
To do a universal build of jansson which is a library for JSON in C
```
cd jansson
mkdir build
cd build
cmake .. "-DCMAKE_OSX_ARCHITECTURES:=x86_64;i386"
make
cd ../..
```

To do a universal build of libwebsockets do the following. I don't really understand why this sequence of operations works. If you try to do the second cmake command without having first done the first you will encounter compiler errors when you run make.
```
cd libwebsockets
mkdir build
cd build
cmake .. -DCMAKE_C_FLAGS='-arch i386'
make
cmake .. -DCMAKE_C_FLAGS='-arch i386 -arch x86_64'
make
cd ../..
```

This sequence has been provided as a shell script in `build_dependencies.sh`.

### Build externals ###
Each external can be built with it's own XCode project or you can use the convenience Makefile to build them all at once.
```
make
```


## Documentation ##
ChainFlow consists of a set of Max external objects that interact with eachother and collectively provide an interface for accessing spatially distributed time-series data hosted by chainAPI server.

### Definitions ###
* "site" - a collection of devices, sensors, and their data
* "device" - a location in space associated with one or more streams of time-series data usually corresponding to a real-world physical device carrying sensors
* "metric" - a type of time-series data (e.g. "humidity", "temperature")

### Objects ###
The `[chain.site]` object manages a connection to a chainAPI server. The other `[chain.*]` objects need to be attached to an instance of `[chain.site]` which manages them. The first and only argument to all `[chain.*]` objects is an identifier to associate workers with a site. This defaults to a genric identifier "defaul_site." Each instance of `[chain.site]` should be given a unique identifier.

#### `[chain.site]` ####


#### `[chain.device]` ####


#### `[chain.info]` ####


#### `[chain.zone]` ####


#### `[chain.time]` ####


#### `[chain.metric]` ####


#### `[chain.map]` ####

