/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author: Vadim Tikhanoff
 * email:   vadim.tikhanoff@iit.it
 * website: www.robotcub.org
 * Algorithm taken from R. Hess, A. Fern, "Discriminatively trained particle filters for complex multi-object tracking,"
 * cvpr, pp.240-247, 2009 IEEE Conference on Computer Vision and Pattern Recognition, 2009
 *
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


#ifndef __MEMORY_REPORTER__H__
#define __MEMORY_REPORTER__H__

#include <string>

#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/PortReport.h>
#include <yarp/os/Stamp.h>
#include <yarp/os/Semaphore.h>
#include <yarp/sig/Vector.h>
#include <yarp/sig/Image.h>

#include "activityInterface.h"

class ActivityInterface;

/**********************************************************/
class MemoryReporter : public yarp::os::PortReport
{
    ActivityInterface *manager;
    
public:
    MemoryReporter();
    void setManager(ActivityInterface *manager);
    void report(const yarp::os::PortInfo &info);
};

/**********************************************************/
class PradaReporter : public yarp::os::BufferedPort<yarp::os::Bottle>
{
protected:
    ActivityInterface *manager;
    void onRead(yarp::os::Bottle &b);
public:
    PradaReporter();
    void setManager(ActivityInterface *manager);
};

/**********************************************************/
class SpeechReporter : public yarp::os::BufferedPort<yarp::os::Bottle>
{
protected:
    ActivityInterface *manager;
    void onRead(yarp::os::Bottle &b);
public:
    SpeechReporter();
    void setManager(ActivityInterface *manager);
};

#endif
//empty line to make gcc happy