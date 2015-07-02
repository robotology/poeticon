// This is an automatically-generated file.
// It could get re-generated if the ALLOW_IDL_GENERATION flag is on.

#include <planningCycle_IDLserver.h>
#include <yarp/os/idl/WireTypes.h>



class planningCycle_IDLserver_quit : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_startPlanner : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_stopPlanner : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_pausePlanner : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_resumePlanner : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_goBack : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_goForward : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_updateState : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_loadObjects : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_printObjects : public yarp::os::Portable {
public:
  yarp::os::Bottle _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_loadState : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_showCurrentState : public yarp::os::Portable {
public:
  std::string _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_compileGoal : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_updateGoals : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_showCurrentGoal : public yarp::os::Portable {
public:
  std::string _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_ground : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_plan : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_run1Step : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_showPlannedAction : public yarp::os::Portable {
public:
  std::string _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_executePlannedAction : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_checkGoalCompleted : public yarp::os::Portable {
public:
  std::string _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_resetRules : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_resetConfig : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_resetLevel : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class planningCycle_IDLserver_increaseHorizon : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

bool planningCycle_IDLserver_quit::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("quit",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_quit::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_quit::init() {
  _return = false;
}

bool planningCycle_IDLserver_startPlanner::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("startPlanner",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_startPlanner::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_startPlanner::init() {
  _return = false;
}

bool planningCycle_IDLserver_stopPlanner::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("stopPlanner",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_stopPlanner::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_stopPlanner::init() {
  _return = false;
}

bool planningCycle_IDLserver_pausePlanner::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("pausePlanner",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_pausePlanner::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_pausePlanner::init() {
  _return = false;
}

bool planningCycle_IDLserver_resumePlanner::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("resumePlanner",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_resumePlanner::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_resumePlanner::init() {
  _return = false;
}

bool planningCycle_IDLserver_goBack::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("goBack",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_goBack::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_goBack::init() {
  _return = false;
}

bool planningCycle_IDLserver_goForward::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("goForward",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_goForward::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_goForward::init() {
  _return = false;
}

bool planningCycle_IDLserver_updateState::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("updateState",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_updateState::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_updateState::init() {
  _return = false;
}

bool planningCycle_IDLserver_loadObjects::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("loadObjects",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_loadObjects::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_loadObjects::init() {
  _return = false;
}

bool planningCycle_IDLserver_printObjects::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("printObjects",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_printObjects::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.read(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_printObjects::init() {
}

bool planningCycle_IDLserver_loadState::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("loadState",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_loadState::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_loadState::init() {
  _return = false;
}

bool planningCycle_IDLserver_showCurrentState::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("showCurrentState",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_showCurrentState::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readString(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_showCurrentState::init() {
  _return = "";
}

bool planningCycle_IDLserver_compileGoal::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("compileGoal",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_compileGoal::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_compileGoal::init() {
  _return = false;
}

bool planningCycle_IDLserver_updateGoals::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("updateGoals",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_updateGoals::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_updateGoals::init() {
  _return = false;
}

bool planningCycle_IDLserver_showCurrentGoal::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("showCurrentGoal",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_showCurrentGoal::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readString(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_showCurrentGoal::init() {
  _return = "";
}

bool planningCycle_IDLserver_ground::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("ground",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_ground::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_ground::init() {
  _return = false;
}

bool planningCycle_IDLserver_plan::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("plan",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_plan::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_plan::init() {
  _return = false;
}

bool planningCycle_IDLserver_run1Step::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("run1Step",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_run1Step::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_run1Step::init() {
  _return = false;
}

bool planningCycle_IDLserver_showPlannedAction::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("showPlannedAction",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_showPlannedAction::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readString(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_showPlannedAction::init() {
  _return = "";
}

bool planningCycle_IDLserver_executePlannedAction::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("executePlannedAction",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_executePlannedAction::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_executePlannedAction::init() {
  _return = false;
}

bool planningCycle_IDLserver_checkGoalCompleted::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("checkGoalCompleted",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_checkGoalCompleted::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readString(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_checkGoalCompleted::init() {
  _return = "";
}

bool planningCycle_IDLserver_resetRules::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("resetRules",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_resetRules::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_resetRules::init() {
  _return = false;
}

bool planningCycle_IDLserver_resetConfig::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("resetConfig",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_resetConfig::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_resetConfig::init() {
  _return = false;
}

bool planningCycle_IDLserver_resetLevel::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("resetLevel",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_resetLevel::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_resetLevel::init() {
  _return = false;
}

bool planningCycle_IDLserver_increaseHorizon::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("increaseHorizon",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_increaseHorizon::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_increaseHorizon::init() {
  _return = false;
}

planningCycle_IDLserver::planningCycle_IDLserver() {
  yarp().setOwner(*this);
}
bool planningCycle_IDLserver::quit() {
  bool _return = false;
  planningCycle_IDLserver_quit helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::quit()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::startPlanner() {
  bool _return = false;
  planningCycle_IDLserver_startPlanner helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::startPlanner()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::stopPlanner() {
  bool _return = false;
  planningCycle_IDLserver_stopPlanner helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::stopPlanner()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::pausePlanner() {
  bool _return = false;
  planningCycle_IDLserver_pausePlanner helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::pausePlanner()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::resumePlanner() {
  bool _return = false;
  planningCycle_IDLserver_resumePlanner helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::resumePlanner()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::goBack() {
  bool _return = false;
  planningCycle_IDLserver_goBack helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::goBack()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::goForward() {
  bool _return = false;
  planningCycle_IDLserver_goForward helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::goForward()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::updateState() {
  bool _return = false;
  planningCycle_IDLserver_updateState helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::updateState()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::loadObjects() {
  bool _return = false;
  planningCycle_IDLserver_loadObjects helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::loadObjects()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
yarp::os::Bottle planningCycle_IDLserver::printObjects() {
  yarp::os::Bottle _return;
  planningCycle_IDLserver_printObjects helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","yarp::os::Bottle planningCycle_IDLserver::printObjects()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::loadState() {
  bool _return = false;
  planningCycle_IDLserver_loadState helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::loadState()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
std::string planningCycle_IDLserver::showCurrentState() {
  std::string _return = "";
  planningCycle_IDLserver_showCurrentState helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","std::string planningCycle_IDLserver::showCurrentState()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::compileGoal() {
  bool _return = false;
  planningCycle_IDLserver_compileGoal helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::compileGoal()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::updateGoals() {
  bool _return = false;
  planningCycle_IDLserver_updateGoals helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::updateGoals()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
std::string planningCycle_IDLserver::showCurrentGoal() {
  std::string _return = "";
  planningCycle_IDLserver_showCurrentGoal helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","std::string planningCycle_IDLserver::showCurrentGoal()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::ground() {
  bool _return = false;
  planningCycle_IDLserver_ground helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::ground()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::plan() {
  bool _return = false;
  planningCycle_IDLserver_plan helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::plan()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::run1Step() {
  bool _return = false;
  planningCycle_IDLserver_run1Step helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::run1Step()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
std::string planningCycle_IDLserver::showPlannedAction() {
  std::string _return = "";
  planningCycle_IDLserver_showPlannedAction helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","std::string planningCycle_IDLserver::showPlannedAction()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::executePlannedAction() {
  bool _return = false;
  planningCycle_IDLserver_executePlannedAction helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::executePlannedAction()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
std::string planningCycle_IDLserver::checkGoalCompleted() {
  std::string _return = "";
  planningCycle_IDLserver_checkGoalCompleted helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","std::string planningCycle_IDLserver::checkGoalCompleted()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::resetRules() {
  bool _return = false;
  planningCycle_IDLserver_resetRules helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::resetRules()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::resetConfig() {
  bool _return = false;
  planningCycle_IDLserver_resetConfig helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::resetConfig()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::resetLevel() {
  bool _return = false;
  planningCycle_IDLserver_resetLevel helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::resetLevel()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool planningCycle_IDLserver::increaseHorizon() {
  bool _return = false;
  planningCycle_IDLserver_increaseHorizon helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::increaseHorizon()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}

bool planningCycle_IDLserver::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  reader.expectAccept();
  if (!reader.readListHeader()) { reader.fail(); return false; }
  yarp::os::ConstString tag = reader.readTag();
  bool direct = (tag=="__direct__");
  if (direct) tag = reader.readTag();
  while (!reader.isError()) {
    // TODO: use quick lookup, this is just a test
    if (tag == "quit") {
      bool _return;
      _return = quit();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "startPlanner") {
      bool _return;
      _return = startPlanner();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "stopPlanner") {
      bool _return;
      _return = stopPlanner();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "pausePlanner") {
      bool _return;
      _return = pausePlanner();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "resumePlanner") {
      bool _return;
      _return = resumePlanner();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "goBack") {
      bool _return;
      _return = goBack();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "goForward") {
      bool _return;
      _return = goForward();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "updateState") {
      bool _return;
      _return = updateState();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "loadObjects") {
      bool _return;
      _return = loadObjects();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "printObjects") {
      yarp::os::Bottle _return;
      _return = printObjects();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.write(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "loadState") {
      bool _return;
      _return = loadState();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "showCurrentState") {
      std::string _return;
      _return = showCurrentState();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeString(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "compileGoal") {
      bool _return;
      _return = compileGoal();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "updateGoals") {
      bool _return;
      _return = updateGoals();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "showCurrentGoal") {
      std::string _return;
      _return = showCurrentGoal();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeString(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "ground") {
      bool _return;
      _return = ground();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "plan") {
      bool _return;
      _return = plan();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "run1Step") {
      bool _return;
      _return = run1Step();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "showPlannedAction") {
      std::string _return;
      _return = showPlannedAction();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeString(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "executePlannedAction") {
      bool _return;
      _return = executePlannedAction();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "checkGoalCompleted") {
      std::string _return;
      _return = checkGoalCompleted();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeString(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "resetRules") {
      bool _return;
      _return = resetRules();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "resetConfig") {
      bool _return;
      _return = resetConfig();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "resetLevel") {
      bool _return;
      _return = resetLevel();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "increaseHorizon") {
      bool _return;
      _return = increaseHorizon();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "help") {
      std::string functionName;
      if (!reader.readString(functionName)) {
        functionName = "--all";
      }
      std::vector<std::string> _return=help(functionName);
      yarp::os::idl::WireWriter writer(reader);
        if (!writer.isNull()) {
          if (!writer.writeListHeader(2)) return false;
          if (!writer.writeTag("many",1, 0)) return false;
          if (!writer.writeListBegin(BOTTLE_TAG_INT, static_cast<uint32_t>(_return.size()))) return false;
          std::vector<std::string> ::iterator _iterHelp;
          for (_iterHelp = _return.begin(); _iterHelp != _return.end(); ++_iterHelp)
          {
            if (!writer.writeString(*_iterHelp)) return false;
           }
          if (!writer.writeListEnd()) return false;
        }
      reader.accept();
      return true;
    }
    if (reader.noMore()) { reader.fail(); return false; }
    yarp::os::ConstString next_tag = reader.readTag();
    if (next_tag=="") break;
    tag = tag + "_" + next_tag;
  }
  return false;
}

std::vector<std::string> planningCycle_IDLserver::help(const std::string& functionName) {
  bool showAll=(functionName=="--all");
  std::vector<std::string> helpString;
  if(showAll) {
    helpString.push_back("*** Available commands:");
    helpString.push_back("quit");
    helpString.push_back("startPlanner");
    helpString.push_back("stopPlanner");
    helpString.push_back("pausePlanner");
    helpString.push_back("resumePlanner");
    helpString.push_back("goBack");
    helpString.push_back("goForward");
    helpString.push_back("updateState");
    helpString.push_back("loadObjects");
    helpString.push_back("printObjects");
    helpString.push_back("loadState");
    helpString.push_back("showCurrentState");
    helpString.push_back("compileGoal");
    helpString.push_back("updateGoals");
    helpString.push_back("showCurrentGoal");
    helpString.push_back("ground");
    helpString.push_back("plan");
    helpString.push_back("run1Step");
    helpString.push_back("showPlannedAction");
    helpString.push_back("executePlannedAction");
    helpString.push_back("checkGoalCompleted");
    helpString.push_back("resetRules");
    helpString.push_back("resetConfig");
    helpString.push_back("resetLevel");
    helpString.push_back("increaseHorizon");
    helpString.push_back("help");
  }
  else {
    if (functionName=="quit") {
      helpString.push_back("bool quit() ");
      helpString.push_back("Quit the module. ");
      helpString.push_back("Abruptly terminates the module and the rpc port ");
    }
    if (functionName=="startPlanner") {
      helpString.push_back("bool startPlanner() ");
      helpString.push_back("Start fully-automated planner. ");
      helpString.push_back("The planner may be paused, unpaused or stopped. ");
      helpString.push_back("It will update the state, ground the rules, compile the goals, and plan the necessary steps to achieve the goals. ");
      helpString.push_back("@returns ok upon receiving/understanding the command ");
    }
    if (functionName=="stopPlanner") {
      helpString.push_back("bool stopPlanner() ");
      helpString.push_back("Stop fully-automated planner. ");
      helpString.push_back("Rules and symbols will have to be grounded again, goals will have to be recompiled. ");
      helpString.push_back("@returns ok upon receiving/understanding the command ");
    }
    if (functionName=="pausePlanner") {
      helpString.push_back("bool pausePlanner() ");
      helpString.push_back("Pauses the fully-automated planner. ");
      helpString.push_back("It will execute the last instruction started before the pause command. ");
      helpString.push_back("Planning variables may be changed while paused. ");
      helpString.push_back("@returns ok upon receiving/understanding the command ");
    }
    if (functionName=="resumePlanner") {
      helpString.push_back("bool resumePlanner() ");
      helpString.push_back("Resumes the fully-automated planner. ");
      helpString.push_back("Resumes the planner. It will start from where it last stopped. ");
      helpString.push_back("@returns ok upon receiving/understanding the command ");
    }
    if (functionName=="goBack") {
      helpString.push_back("bool goBack() ");
      helpString.push_back("Goes back one step in the subgoal list. ");
      helpString.push_back("@returns ok upon receiving/understanding the command ");
    }
    if (functionName=="goForward") {
      helpString.push_back("bool goForward() ");
      helpString.push_back("Goes forward one step in the subgoal list. ");
      helpString.push_back("@returns ok upon receiving/understanding the command ");
    }
    if (functionName=="updateState") {
      helpString.push_back("bool updateState() ");
      helpString.push_back("Updates the planner world state. ");
      helpString.push_back("Sends an update command to both the world state manager and opc2prada ");
      helpString.push_back("@returns ok/fail if successful/not. ");
    }
    if (functionName=="loadObjects") {
      helpString.push_back("bool loadObjects() ");
      helpString.push_back("Updates and loads the object IDs and labels into the planner. ");
      helpString.push_back("Sends an update command to the opc2prada ");
      helpString.push_back("@returns ok/fail if successful/not. ");
    }
    if (functionName=="printObjects") {
      helpString.push_back("yarp::os::Bottle printObjects() ");
      helpString.push_back("Prints the last loaded objects on the planner. ");
      helpString.push_back("Does NOT send an update command to the opc2prada ");
      helpString.push_back("@returns bottle of bottles with (IDs, labels), or fail. ");
    }
    if (functionName=="loadState") {
      helpString.push_back("bool loadState() ");
      helpString.push_back("Loads the world state to the planner. ");
      helpString.push_back("Completes the state file with missing symbols (negated), and loads it into the planner. ");
      helpString.push_back("@returns ok/fail if successful/not. ");
    }
    if (functionName=="showCurrentState") {
      helpString.push_back("std::string showCurrentState() ");
      helpString.push_back("Prints world state on the terminal. ");
      helpString.push_back("Prints the list of symbols on the current world state on the terminal. ");
      helpString.push_back("Make sure to loadState if the state changed. ");
      helpString.push_back("@returns bottle with the world state symbol-list, or fail if not successful. ");
    }
    if (functionName=="compileGoal") {
      helpString.push_back("bool compileGoal() ");
      helpString.push_back("Compiles subgoal list from Praxicon instructions ");
      helpString.push_back("Waits for a praxicon instruction bottle and translates it into robot symbols. ");
      helpString.push_back("The compiler will timeout if no bottle is received in 5 minutes ");
      helpString.push_back("@returns ok/fail upon successful compiling/not. ");
    }
    if (functionName=="updateGoals") {
      helpString.push_back("bool updateGoals() ");
      helpString.push_back("Loads the current subgoal. ");
      helpString.push_back("Loads the current subgoal according to the planning step. ");
      helpString.push_back("@returns ok/fail upon success/not. ");
    }
    if (functionName=="showCurrentGoal") {
      helpString.push_back("std::string showCurrentGoal() ");
      helpString.push_back("Prints current planner goal to the terminal. ");
      helpString.push_back("Prints the list of symbols of the current planner goal to the terminal. ");
      helpString.push_back("Make sure you have updated the goals if the plan level changed. ");
      helpString.push_back("@returns bottle with the planner goal symbol-list, or fail if not. ");
    }
    if (functionName=="ground") {
      helpString.push_back("bool ground() ");
      helpString.push_back("Grounds rules. ");
      helpString.push_back("Creates a list of rules grounded for the available objects, with affordances. ");
      helpString.push_back("Requires an updated world state to get the object list. ");
      helpString.push_back("@returns ok/fail upon success/not. ");
    }
    if (functionName=="plan") {
      helpString.push_back("bool plan() ");
      helpString.push_back("Plans the next action. ");
      helpString.push_back("Runs PRADA to find the next best action to achieve the current subgoal. ");
      helpString.push_back("Make sure all rules have been grounded, the state updated, the goals compiled and updated, and that it is in the correct plan level. ");
      helpString.push_back("@returns ok/fail if an action is found/not. ");
    }
    if (functionName=="run1Step") {
      helpString.push_back("bool run1Step() ");
      helpString.push_back("Runs a full loop of the planning cycle. ");
      helpString.push_back("Updates the world state, checks the subgoals, plans the next action, codes and executes it. ");
      helpString.push_back("Make sure the rules have been grounded, the goals compiled and updated, and that it is in the correct plan level. ");
      helpString.push_back("@returns ok/fail upon completion/failure. ");
    }
    if (functionName=="showPlannedAction") {
      helpString.push_back("std::string showPlannedAction() ");
      helpString.push_back("Prints the planned action on the terminal. ");
      helpString.push_back("Translates the planned action to object labels and prints it on the terminal. ");
      helpString.push_back("Only the last planned action will be printed. ");
      helpString.push_back("@returns last planned action. If no action has been planned, returns an empty string. ");
    }
    if (functionName=="executePlannedAction") {
      helpString.push_back("bool executePlannedAction() ");
      helpString.push_back("Executes the last planned action. ");
      helpString.push_back("Translates the planned action to objects labels and sends the instruction to ActivityInterface for execution. ");
      helpString.push_back("@returns ok/fail upon success/failure of the action. ");
    }
    if (functionName=="checkGoalCompleted") {
      helpString.push_back("std::string checkGoalCompleted() ");
      helpString.push_back("Checks if the current subgoal has been achieved. ");
      helpString.push_back("Compares the current subgoal with the world state to verify if it has been achieved. ");
      helpString.push_back("Make sure the correct plan level is set, and that the goals are updated. ");
      helpString.push_back("@returns goal achieved/not achieved. ");
    }
    if (functionName=="resetRules") {
      helpString.push_back("bool resetRules() ");
      helpString.push_back("Resets rules. ");
      helpString.push_back("Returns the rule list to the point of the last grounding. ");
      helpString.push_back("@returns ok upon receiving/understanding the command. ");
    }
    if (functionName=="resetConfig") {
      helpString.push_back("bool resetConfig() ");
      helpString.push_back("Resets config file. ");
      helpString.push_back("Returns the config file for PRADA to the default values. ");
      helpString.push_back("Sets the planning horizon back to 5. ");
      helpString.push_back("@returns ok/fail upon writing the file/failing. ");
    }
    if (functionName=="resetLevel") {
      helpString.push_back("bool resetLevel() ");
      helpString.push_back("Resets plan level. ");
      helpString.push_back("Sets the plan level back to 0. ");
      helpString.push_back("@returns ok upon receiving/understanding the command. ");
    }
    if (functionName=="increaseHorizon") {
      helpString.push_back("bool increaseHorizon() ");
      helpString.push_back("Increases planning horizon. ");
      helpString.push_back("Increases the planning horizon by 1. The config file will be written over. ");
      helpString.push_back("Horizon won't exceed 15. ");
      helpString.push_back("@returns ok/fail upon writing the file/failure ");
    }
    if (functionName=="help") {
      helpString.push_back("std::vector<std::string> help(const std::string& functionName=\"--all\")");
      helpString.push_back("Return list of available commands, or help message for a specific function");
      helpString.push_back("@param functionName name of command for which to get a detailed description. If none or '--all' is provided, print list of available commands");
      helpString.push_back("@return list of strings (one string per line)");
    }
  }
  if ( helpString.empty()) helpString.push_back("Command not found");
  return helpString;
}


