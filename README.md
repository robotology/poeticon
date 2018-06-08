POETICON++
==========

This is the repository of the [POETICON++ EU project](http://www.poeticon.eu).

Video by Istituto Italiano di Tecnologia:

[![POETICON D6.2 videoDemo](http://img.youtube.com/vi/FNeYixxmeTM/0.jpg)](http://www.youtube.com/watch?v=FNeYixxmeTM&t=1m06s "POETICON D6.2 videoDemo")

More videos are available on the [YouTube channel](https://www.youtube.com/user/Poeticoneu).

## Installation

### Dependencies

Mandatory dependencies of the core build:
- [YARP](https://github.com/robotology/yarp)
- [icub-contrib-common](https://github.com/robotology/icub-contrib-common)
- [OpenCV](http://opencv.org/downloads.html), version 2.4

Dependencies of probabilistic planning:
- [libPRADA](http://userpage.fu-berlin.de/tlang/prada/), mirrored in the *3rdparty* directory

Dependencies of affordance perception:
- [MATLAB](http://www.mathworks.com/products/matlab)
- [YARP Java bindings](http://www.yarp.it/yarp_swig.html) (in the YARP build directory, call CMake and enable ```YARP_COMPILE_BINDINGS```, ```CREATE_JAVA```, ```PREPARE_CLASS_FILES```)
- YARP for MATLAB (add the directory with the Java ```*.class``` files to the classpath, and the directory with ```libjyarp.so``` to ```librarypath.txt```)
- [pmtk3](https://github.com/probml/pmtk3) (installation: launch MATLAB as superuser, run ```initPmtk3.m```, click on setPath, select the pmtk3 directories then Save to permanently add them to MATLABPATH)

### External projects

The following external projects are needed to execute the full POETICON++ demo:
- [Hierarchical Image Representation](https://github.com/robotology/himrep)
- [Interactive Objects Learning](https://github.com/robotology/iol)
- [karma](https://github.com/robotology/karma)
- [segmentation](https://github.com/robotology/segmentation)
- [stereo-vision](https://github.com/robotology/stereo-vision)

Dependencies of the optional speech recognition component:
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

**Affordance perception** sensorimotor data and documentation are [here](./src/affNetwork).

Instructions on how to run the **simulated symbolic reasoner** (to test **probabilistic planning** under noisy conditions and with different heuristics) are [here](./src/poeticon-simulation).

Further documentation is available inside each module source, and via the help commands available via RPC interfaces.

## Articles

- Alexandre Antunes, Lorenzo Jamone, Giovanni Saponaro, Alexandre Bernardino, Rodrigo Ventura. *From Human Instructions to Robot Actions: Formulation of Goals, Affordances and Probabilistic Planning*. IEEE International Conference on Robotics and Automation (ICRA 2016).
- Tanis Mar, Vadim Tikhanoff, Giorgio Metta, Lorenzo Natale. *Self-supervised learning of grasp dependent tool affordances on the iCub Humanoid robot*. IEEE International Conference on Robotics and Automation (ICRA 2015).
- Tanis Mar, Vadim Tikhanoff, Giorgio Metta, Lorenzo Natale. *Multi-model approach based on 3D functional features for tool affordance learning in robotics*. IEEE-RAS International Conference on Humanoid Robots (Humanoids 2015).
- Afonso Gonçalves, João Abrantes, Giovanni Saponaro, Lorenzo Jamone, Alexandre Bernardino. *Learning Intermediate Object Affordances: Towards the Development of a Tool Concept*. IEEE International Conference on Development and Learning and on Epigenetic Robotics (ICDL-EpiRob 2014).
- Afonso Gonçalves, Giovanni Saponaro, Lorenzo Jamone, Alexandre Bernardino. *Learning Visual Affordances of Objects and Tools through Autonomous Robot Exploration*. IEEE International Conference on Autonomous Robot Systems and Competitions (ICARSC 2014).
- Vadim Tikhanoff, Ugo Pattacini, Lorenzo Natale, Giorgio Metta. *Exploring affordances and tool use on the iCub*. IEEE-RAS International Conference on Humanoid Robots (Humanoids 2013).

## License

Released under the terms of the GPL v2.0 or later. See the file LICENSE for details.
