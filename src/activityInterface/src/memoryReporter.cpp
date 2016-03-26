/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author: Vadim Tikhanoff
 * email:  vadim.tikhanoff@iit.it
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

#include <yarp/os/Time.h>

#include "activityInterface.h"

using namespace yarp::os;

/**********************************************************/
MemoryReporter::MemoryReporter()
{
    manager=NULL;
}

/**********************************************************/
void MemoryReporter::setManager(ActivityInterface *manager)
{
    this->manager=manager;
}

/**********************************************************/
void MemoryReporter::report(const PortInfo &info)
{
    if ((manager!=NULL) && info.created && !info.incoming)
        manager->scheduleLoadMemory=true;
}

/**********************************************************/
PradaReporter::PradaReporter()
{
    manager=NULL;
    useCallback();
}

/**********************************************************/
void PradaReporter::setManager(ActivityInterface *manager)
{
    this->manager=manager;
}

/**********************************************************/
void PradaReporter::onRead(Bottle &status)
{
    yError("-------------------------------------------");
    yError("%s", status.toString().c_str());
    yError("-------------------------------------------");
    
    if (status.size()>0)
        manager->processPradaStatus(status);
}

/**********************************************************/
SpeechReporter::SpeechReporter()
{
    manager=NULL;
    useCallback();
}

/**********************************************************/
void SpeechReporter::setManager(ActivityInterface *manager)
{
    this->manager=manager;
}

/**********************************************************/
void SpeechReporter::onRead(Bottle &speech)
{
    if (speech.size()>0)
        manager->processSpeech(speech);
}
//empty line to make gcc happy