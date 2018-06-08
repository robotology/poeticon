/*
 * Copyright (C) 2011 Department of Robotics Brain and Cognitive Sciences - Istituto Italiano di Tecnologia
 * Author: Vadim Tikhanoff Ali Paikan
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

#ifndef TARGET_OBJECT_INC
#define TARGET_OBJECT_INC

#include <yarp/os/ConstString.h>
#include <yarp/os/Semaphore.h>
#include <yarp/sig/Vector.h>
#include <yarp/os/Stamp.h>
#include <cv.h>
#include <vector>
#include <yarp/os/Log.h>

class TargetObject {
public:
    TargetObject(unsigned int id)
    {
        tpl         = NULL;
        seg         = NULL;
        hasSent     = false;
        owner       = id;
        group       = -1;
    }

    std::vector<double>     boundingBox;
    std::vector<CvPoint>    particlePoints;
    yarp::os::Stamp         time;
    unsigned int            owner;
    int                     group;
    IplImage                *tpl;
    IplImage                *seg;
    bool                    hasSent;
    CvScalar                colour;
};

/**
 *  A singleton class to record the target object
 */
class TargetObjectRecord {
public:
    typedef std::vector<TargetObject*> vector_type;
    typedef vector_type::iterator iterator;
    typedef vector_type::const_iterator const_iterator;

    vector_type targets;

    bool add(TargetObject *obj)
    {
        yAssert(obj);
        TargetObjectRecord::iterator itr = findOwner(obj->owner);
        if(itr != targets.end())
            targets.erase(itr);
        targets.push_back(obj);

        return true;
    }

    bool remove(unsigned int owner)
    {
        TargetObjectRecord::iterator itr = findOwner(owner);
        if(itr == targets.end())
            return true;
        targets.erase(itr);
        return true;
    }

    TargetObject* get(unsigned int owner)
    {
        TargetObjectRecord::iterator itr = findOwner(owner);
        if(itr != targets.end())
            return (*itr);

        return NULL;
    }

    void lock() { mutex.wait(); }
    void unlock() { mutex.post(); }

    static TargetObjectRecord& getInstance(void) {
        static TargetObjectRecord __instance_TargetObjectRecord;
        return __instance_TargetObjectRecord;
    }

private:
    TargetObjectRecord() { }
    TargetObjectRecord(TargetObjectRecord const &);
    void operator=(TargetObjectRecord const &);

    TargetObjectRecord::iterator findOwner(unsigned int owner) {
        TargetObjectRecord::iterator itr;
        for(itr=targets.begin(); itr<targets.end(); itr++)
            if ((*itr)->owner == owner)
                return itr;
        return targets.end();
    }
    
private:
    yarp::os::Semaphore mutex;
};

#endif //TargetObject_INC
//empty line to make gcc happy
