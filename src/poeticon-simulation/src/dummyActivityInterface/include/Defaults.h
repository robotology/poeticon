/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2018 VisLab, Institute for Systems and Robotics,
 *                Istituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.tecnico.ulisboa.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 * Based on activityInterface by Vadim Tikhanoff
 *
 */

#ifndef DEFAULTS_H
#define DEFAULTS_H

#include <string>

const std::string DefContext    = "poeticon";
const std::string DefConfigFile = "dummyActivityInterface.ini";

const int         DefThreadPeriod = 33; // [ms]
const std::string DefModuleName   = "activityInterface";

const std::string Success = "SUCCESS";
const std::string Failure = "FAIL";

const double DefProbabilityGraspToolLeft  = 1.0;
const double DefProbabilityGraspToolRight = 1.0;
const double DefProbabilityPerceiveGrasp  = 1.0;
const double DefProbabilityPull           = 1.0;
const double DefProbabilityPush           = 1.0;
const double DefProbabilityPutLeft        = 1.0;
const double DefProbabilityPutRight       = 1.0;
const double DefProbabilityTakeLeft       = 1.0;
const double DefProbabilityTakeRight      = 1.0;
const double DefProbabilityVisionObject   = 1.0;

const double DefReachableThresholdX       = -0.48;
const double DefReachableThresholdYLeft   = -0.15;
const double DefReachableThresholdYRight  =  0.15;

#endif
