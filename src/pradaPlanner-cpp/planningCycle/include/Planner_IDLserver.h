// This is an automatically-generated file.
// It could get re-generated if the ALLOW_IDL_GENERATION flag is on.

#ifndef YARP_THRIFT_GENERATOR_Planner_IDLserver
#define YARP_THRIFT_GENERATOR_Planner_IDLserver

#include <yarp/os/Wire.h>
#include <yarp/os/idl/WireTypes.h>

class Planner_IDLserver;


class Planner_IDLserver : public yarp::os::Wire {
public:
  Planner_IDLserver();
  virtual bool quit();
  virtual bool startPlanning();
  virtual bool pausePlanner();
  virtual bool resumePlanner();
  virtual bool goBack();
  virtual bool goForward();
  virtual bool updateState();
  virtual bool updateGoals();
  virtual bool plan();
  virtual bool resetRules();
  virtual bool resetConfig();
  virtual bool resetLevel();
  virtual bool ground();
  virtual bool compileGoal();
  virtual bool increaseHorizon();
  virtual bool executePlannedAction();
  virtual bool checkGoalCompleted();
  virtual bool run1Step();
  virtual std::string showPlannedAction();
  virtual std::string showCurrentState();
  virtual std::string showCurrentGoal();
  virtual bool read(yarp::os::ConnectionReader& connection);
  virtual std::vector<std::string> help(const std::string& functionName="--all");
};

#endif

