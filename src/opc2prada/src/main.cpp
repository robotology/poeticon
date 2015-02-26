// -*- mode:C++; tab-width:4; c-basic-offset:4; indent-tabs-mode:nil -*-

/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Pedro Vicente <pvicente@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 */

#include <yarp/os/Network.h>
#include <yarp/os/RFModule.h>

#include "translator.h"

using namespace yarp::os;
using namespace std;

int main(int argc, char *argv[]) {

	Network yarp;

	if(! yarp.checkNetwork() ) {
        fprintf(stdout,"Error: yarp server does not seem available\n");
        return -1;
    }

	TranslatorModule module;

    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefaultContext("poeticon");
    rf.setDefaultConfigFile("opc2prada.ini");
    rf.configure(argc, argv);
    module.runModule(rf);

    return 0;
}
