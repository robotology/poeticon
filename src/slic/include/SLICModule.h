/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 * Note: SLICSuperpixel class is by Saburo Okita,
 *       http://subokita.com/2014/04/23/slic-superpixels-on-opencv/
 *
 */

#ifndef __SLIC_MODULE_H__
#define __SLIC_MODULE_H__

#include <yarp/os/RFModule.h>
#include "SLICThread.h"

using namespace yarp::os;

class SLICModule : public RFModule
{
    private:
        // module parameters
        string moduleName;
        bool closing;        

        // pointer to a new thread
        SLICThread *thread;
        
        // thread parameters
        double threadPeriod;
        int numSuperpixels;
        int m;
        int maxIter;

    public:
        virtual bool configure(ResourceFinder &rf);
        virtual bool interruptModule();
        virtual bool close();

        virtual bool quit();

        virtual bool updateModule();
        virtual double getPeriod();
};

#endif
