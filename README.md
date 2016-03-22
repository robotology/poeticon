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
- [YARP Java bindings](http://www.yarp.it/yarp_swig.html) (in the YARP build directory, call CMake and enable ```YARP_COMPILE_BINDINGS```, ```CREATE_JAVA```, ```PREPARE_CLASS_FILES```)
- YARP for MATLAB (add the directory with the Java ```*.class``` files to the classpath, and the directory with ```libjyarp.so``` to ```librarypath.txt```)
- [pmtk3](https://github.com/probml/pmtk3) (installation: launch MATLAB as superuser, run ```initPmtk3.m```, click on setPath, select the pmtk3 directories then Save to permanently add them to MATLABPATH)

### Additional dependencies for specific modules

External projects used in the full POETICON++ demo:
- [Hierarchical Image Representation](https://github.com/robotology/himrep)
- [Interactive Objects Learning](https://github.com/robotology/iol)
- [karma](https://github.com/robotology/karma)
- [segmentation](https://github.com/robotology/segmentation)
- [stereo-vision](https://github.com/robotology/stereo-vision)

Speech recognition dependencies:
- [YARP Lua bindings](http://wiki.icub.org/yarpdoc/yarp_swig.html)
- [rFSM](https://github.com/kmarkus/rFSM) (just clone it, no need to compile)

### Linux

First install libPRADA, required by the probabilistic planner:

    cd poeticon
    tar xzvf 3rdparty/libPRADA.tgz && cd libPRADA
    patch src/MT/util.h < ../extern/libPRADA/prada_unistd.patch
    patch test/relational_plan/main.cpp < ../extern/libPRADA/prada_readgoalfromfile.patch
    make
    cp test/relational_plan/x.exe ../app/conf/planner.exe

Then install the core POETICON++ build:

    git clone https://github.com/robotology/poeticon
    cd poeticon && mkdir build && cd build && cmake .. && make

Note: the modules belonging to the probabilistic planner part (planningCycle, affordanceCommunication, geometricGrounding, goalCompiler) must have access to the same "contexts/poeticon" directory. One way to accomplish this is to run them on the same machine.

## Documentation

Please refer to the documentation inside each module, and to the help commands available via RPC interfaces.

## Articles

- Alexandre Antunes, Lorenzo Jamone, Giovanni Saponaro, Alexandre Bernardino, Rodrigo Ventura. *From Human Instructions to Robot Actions: Formulation of Goals, Affordances and Probabilistic Planning*. IEEE International Conference on Robotics and Automation (ICRA 2016).
- Tanis Mar, Vadim Tikhanoff, Giorgio Metta, Lorenzo Natale. *Self-supervised learning of grasp dependent tool affordances on the iCub Humanoid robot*. IEEE International Conference on Robotics and Automation (ICRA 2015).
- Tanis Mar, Vadim Tikhanoff, Giorgio Metta, Lorenzo Natale. *Multi-model approach based on 3D functional features for tool affordance learning in robotics*. IEEE-RAS International Conference on Humanoid Robots (Humanoids 2015).
- Afonso Gonçalves, João Abrantes, Giovanni Saponaro, Lorenzo Jamone, Alexandre Bernardino. *Learning Intermediate Object Affordances: Towards the Development of a Tool Concept*. IEEE International Conference on Development and Learning and on Epigenetic Robotics (ICDL-EpiRob 2014).
- Afonso Gonçalves, Giovanni Saponaro, Lorenzo Jamone, Alexandre Bernardino. *Learning Visual Affordances of Objects and Tools through Autonomous Robot Exploration*. IEEE International Conference on Autonomous Robot Systems and Competitions (ICARSC 2014).
- Vadim Tikhanoff, Ugo Pattacini, Lorenzo Natale, Giorgio Metta. *Exploring affordances and tool use on the iCub*. IEEE-RAS International Conference on Humanoid Robots (Humanoids 2013).

## License

Released under the terms of the GPL v2.0 or later. See the file LICENSE for details.
