POETICON++
==========

This is the repository of the [POETICON++ EU project](http://www.poeticon.eu).

## Installation

### Dependencies

Needed to compile the core build:
- [YARP](https://github.com/robotology/yarp)
- [icub-contrib-common](https://github.com/robotology/icub-contrib-common)
- [OpenCV](http://opencv.org/downloads.html), version 2.4

PRADA probabilistic planner also needs:
- [libPRADA](http://www.informatik.uni-hamburg.de/ML/contents/people/lang/private/prada/), mirrored in the *3rdparty* directory

Affordance network dependencies:
- MATLAB
- [pmtk3](https://github.com/probml/pmtk3) (installation: launch MATLAB as superuser, run ```initPmtk3.m```, click on setPath, select the pmtk3 directories then Save to permanently add them to MATLABPATH)
- [YARP Java bindings](http://www.yarp.it/yarp_swig.html)

### Additional dependencies for specific modules

External projects used in the full POETICON++ demo:
- [Hierarchical Image Representation](https://github.com/robotology/himrep)
- [Interactive Objects Learning](https://github.com/robotology/iol)
- [karma](https://github.com/robotology/karma)
- [segmentation](https://github.com/robotology/segmentation)

Speech recognition dependencies:
- [YARP Lua bindings](http://wiki.icub.org/yarpdoc/yarp_swig.html)
- [rFSM](https://github.com/kmarkus/rFSM) (just clone it, no need to compile)

### Linux

Installation of the core POETICON++ build:

    git clone https://github.com/robotology/poeticon
    cd poeticon && mkdir build && cd build && cmake .. && make

Instructions to install libPRADA, required by the probabilistic planner:

    cd poeticon
    tar xzvf 3rdparty/libPRADA.tgz && cd libPRADA
    patch src/MT/util.h < ../extern/libPRADA/prada_unistd.patch
    patch test/relational_plan/main.cpp < ../extern/libPRADA/prada_readgoalfromfile.patch
    make
    cp test/relational_plan/x.exe ../app/conf/planner.exe

## Documentation

Please refer to the documentation inside each module, and to the help commands available via RPC interfaces.

## Articles

## License

Released under the terms of the GPL v2.0 or later. See the file LICENSE for details.
