/*
 * Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
 * Copyright: (C) 2018 VisLab, Institute for Systems and Robotics,
 *                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
 * Author: Giovanni Saponaro <gsaponaro@isr.tecnico.ulisboa.pt>
 * CopyPolicy: Released under the terms of the GNU GPL v2.0
 *
 * Based on activityInterface by Vadim Tikhanoff
 *
 */

#ifndef DUMMY_ACTIVITY_INTERFACE_THREAD_H
#define DUMMY_ACTIVITY_INTERFACE_THREAD_H

#include <cstring> // strcmp
#include <iostream> // __func__
#include <map>
#include <sstream>
#include <string>

#include <yarp/os/Bottle.h>
#include <yarp/os/BufferedPort.h>
#include <yarp/os/LockGuard.h>
#include <yarp/os/Log.h>
#include <yarp/os/LogStream.h>
#include <yarp/os/Mutex.h>
#include <yarp/os/Port.h>
#include <yarp/os/RateThread.h>
#include <yarp/os/ResourceFinder.h>
#include <yarp/os/RpcClient.h>
#include <yarp/os/Time.h>
#include <yarp/os/Vocab.h>

// make sure __func__ is set correctly, http://stackoverflow.com/a/17528983
#if __STDC_VERSION__ < 199901L
# if __GNUC__ >= 2
#  define __func__ __FUNCTION__
# else
#  define __func__ "<unknown>"
# endif
#endif

struct RobotAction {
    std::string action;
    std::string outcome;
};

class DummyActivityInterfaceThread : public yarp::os::RateThread
{
    private:
        std::string moduleName;
        yarp::os::ResourceFinder rf;
        bool closing;

        double probability_grasp_tool_left;
        double probability_grasp_tool_right;
        double probability_perceive_grasp;
        double probability_pull;
        double probability_push;
        double probability_put_left;
        double probability_put_right;
        double probability_take_left;
        double probability_take_right;
        double probability_vision_object;
        double reachable_threshold_x;
        double reachable_threshold_y_left;
        double reachable_threshold_y_right;

        std::map<std::string, std::string> inHandStatus;
        std::map<int, std::string> onTopElements;
        std::vector<std::string> availableTools;
        int elements;
        yarp::os::RpcClient rpcMemory;
        yarp::os::RpcClient rpcPrada;
        yarp::os::RpcClient rpcPraxiconInterface;
        yarp::os::Port praxiconToPradaPort;
        yarp::os::BufferedPort<yarp::os::Bottle> pradaInputPort;
        yarp::os::Mutex mutex;
        std::string praxiconRequest;
        yarp::os::Bottle listOfGoals;

        std::vector<RobotAction> robotActions;
        int varGood;
        int varSuccess;

    public:
        DummyActivityInterfaceThread(const std::string &_moduleName,
                                     yarp::os::ResourceFinder &_rf);
        bool openPorts();
        void close();
        void interrupt();
        bool threadInit();
        void run();

        void delayAfterMotor();
        void enforceProbabilityBounds(double &n);
        yarp::os::Bottle getMemoryBottle();
        void getParameters();
        yarp::os::Bottle getToolLikeNames();
        std::string holdIn(const std::string &handName);
        bool isConnectedOutput(yarp::os::RpcClient &rpcClient);
        int name2id(const std::string &objName);
        bool processPradaStatus(const yarp::os::Bottle &status);
        yarp::os::Bottle queryUnderOf(const std::string &objName);
        void resetActionCounters(bool verbose=true);
        bool setObjProperty(const std::string &objName, const std::string &prop, const yarp::os::Bottle &v);
        bool validate2D(const std::string &objName);
        bool validate3D(const std::string &objName);
        bool validateName(const std::string &objName);

        // IDL functions
        bool askForTool(const std::string &handName, const int32_t xpos, const int32_t ypos);
        yarp::os::Bottle askPraxicon(const std::string &request);
        bool drop(const std::string &objName);
        bool dump();
        yarp::os::Bottle get2D(const std::string &objName);
        yarp::os::Bottle get3D(const std::string &objName);
        std::string getLabel(const int32_t xpos, const int32_t ypos);
        yarp::os::Bottle getNames();
        bool goHome();
        bool handStat(const std::string &handName);
        std::string inHand(const std::string &objName);
        bool pop();
        bool pull(const std::string &objName, const std::string &toolName);
        yarp::os::Bottle pullableWith(const std::string &objName);
        bool push(const std::string &objName, const std::string &toolName);
        bool put(const std::string &objName, const std::string &targetName);
        yarp::os::Bottle reachableWith(const std::string &objName);
        std::string simulate();
        bool take(const std::string &objName, const std::string &handName);
        yarp::os::Bottle underOf(const std::string &objName);
};

#endif
