POETICON++
==========

This is the repository of the [POETICON++ EU project](http://www.poeticon.eu).

## Installation

### Dependencies

- [YARP](https://github.com/robotology/yarp)
- [icub-contrib-common](https://github.com/robotology/icub-contrib-common)
- [OpenCV](http://opencv.org/downloads.html), version 2.4

### Additional dependencies for specific modules

PRADA probabilistic planner dependencies:
- [libPRADA](http://www.informatik.uni-hamburg.de/ML/contents/people/lang/private/prada/), mirrored in the *3rdparty* directory

Affordance network dependencies:
- MATLAB
- [pmtk3](https://github.com/probml/pmtk3)
- [YARP Java bindings](http://wiki.icub.org/yarpdoc/yarp_swig.html)

Speech recognition dependencies:
- [YARP Lua bindings](http://wiki.icub.org/yarpdoc/yarp_swig.html)
- [rFSM](https://github.com/kmarkus/rFSM) (just clone it, no need to compile)

SLIC Superpixels dependencies:
- [TBB](https://www.threadingbuildingblocks.org/)

### Linux

Installation of the core POETICON++ build:

    git clone https://github.com/robotology/poeticon
    cd poeticon && mkdir build && cd build && cmake .. && make

Instructions to install libPRADA, required by the probabilistic planner:

    cd poeticon
    // a copy of libPRADA.tgz is in the *3rdparty* directory
    wget http://www.informatik.uni-hamburg.de/ML/contents/people/lang/private/prada/libPRADA.tgz
    tar xzvf libPRADA.tgz && cd libPRADA
    patch src/MT/util.h < ../extern/libPRADA/prada_unistd.patch
    patch test/relational_plan/main.cpp < ../extern/libPRADA/prada_readgoalfromfile.patch
    make
    cp test/relational_plan/x.exe ../app/conf/planner.exe

## Documentation

Please refer to the documentation inside each module, and to the help commands available via RPC interfaces.

## Articles

## License

Released under the terms of the GPL v2.0 or later. See the file LICENSE for details.
