/*
 * Copyright (C) 2015 VisLab - Instituto Superior Tecnico
 * Author: Lorenzo Jamone
 * email:  ljamone@isr.ist.utl.pt
 * Permission is granted to copy, distribute, and/or modify this program
 * under the terms of the GNU General Public License, version 2 or any
 * later version published by the Free Software Foundation.
 *
 * A copy of the license can be found at
 * http://www.robotcub.org/icub/license/gpl.txt
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General
 * Public License for more details
*/

#include <yarp/os/Network.h>
#include "iCub/module.h"

using namespace yarp::os;

/**********************************************************/
int main(int argc, char *argv[])
{
    Network yarp;
    if (!yarp.checkNetwork())
        return 1;

    ResourceFinder rf;
    rf.setVerbose(true);
    rf.setDefault("name","affExploreManager");
    rf.setDefaultContext("poeticon");
    rf.setDefaultConfigFile("affExploreManager.ini");
    rf.setDefault("tracking_period","30");
    rf.configure(argc,argv);

    Manager manager;
    return manager.runModule(rf);
}
