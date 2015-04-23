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

class planningCycle_IDLserver_startPlanning : public yarp::os::Portable {
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

class planningCycle_IDLserver_updateGoals : public yarp::os::Portable {
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

class planningCycle_IDLserver_ground : public yarp::os::Portable {
public:
  bool _return;
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

class planningCycle_IDLserver_increaseHorizon : public yarp::os::Portable {
public:
  bool _return;
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

class planningCycle_IDLserver_showCurrentState : public yarp::os::Portable {
public:
  std::string _return;
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

bool planningCycle_IDLserver_startPlanning::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("startPlanning",1,1)) return false;
  return true;
}

bool planningCycle_IDLserver_startPlanning::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_startPlanning::init() {
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
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void planningCycle_IDLserver_checkGoalCompleted::init() {
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
bool planningCycle_IDLserver::startPlanning() {
  bool _return = false;
  planningCycle_IDLserver_startPlanning helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::startPlanning()");
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
bool planningCycle_IDLserver::checkGoalCompleted() {
  bool _return = false;
  planningCycle_IDLserver_checkGoalCompleted helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool planningCycle_IDLserver::checkGoalCompleted()");
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
    if (tag == "startPlanning") {
      bool _return;
      _return = startPlanning();
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
      bool _return;
      _return = checkGoalCompleted();
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
    helpString.push_back("startPlanning");
    helpString.push_back("pausePlanner");
    helpString.push_back("resumePlanner");
    helpString.push_back("goBack");
    helpString.push_back("goForward");
    helpString.push_back("updateState");
    helpString.push_back("updateGoals");
    helpString.push_back("plan");
    helpString.push_back("resetRules");
    helpString.push_back("resetConfig");
    helpString.push_back("resetLevel");
    helpString.push_back("ground");
    helpString.push_back("compileGoal");
    helpString.push_back("increaseHorizon");
    helpString.push_back("executePlannedAction");
    helpString.push_back("checkGoalCompleted");
    helpString.push_back("run1Step");
    helpString.push_back("showPlannedAction");
    helpString.push_back("showCurrentState");
    helpString.push_back("showCurrentGoal");
    helpString.push_back("help");
  }
  else {
    if (functionName=="quit") {
      helpString.push_back("bool quit() ");
    }
    if (functionName=="startPlanning") {
      helpString.push_back("bool startPlanning() ");
    }
    if (functionName=="pausePlanner") {
      helpString.push_back("bool pausePlanner() ");
    }
    if (functionName=="resumePlanner") {
      helpString.push_back("bool resumePlanner() ");
    }
    if (functionName=="goBack") {
      helpString.push_back("bool goBack() ");
    }
    if (functionName=="goForward") {
      helpString.push_back("bool goForward() ");
    }
    if (functionName=="updateState") {
      helpString.push_back("bool updateState() ");
    }
    if (functionName=="updateGoals") {
      helpString.push_back("bool updateGoals() ");
    }
    if (functionName=="plan") {
      helpString.push_back("bool plan() ");
    }
    if (functionName=="resetRules") {
      helpString.push_back("bool resetRules() ");
    }
    if (functionName=="resetConfig") {
      helpString.push_back("bool resetConfig() ");
    }
    if (functionName=="resetLevel") {
      helpString.push_back("bool resetLevel() ");
    }
    if (functionName=="ground") {
      helpString.push_back("bool ground() ");
    }
    if (functionName=="compileGoal") {
      helpString.push_back("bool compileGoal() ");
    }
    if (functionName=="increaseHorizon") {
      helpString.push_back("bool increaseHorizon() ");
    }
    if (functionName=="executePlannedAction") {
      helpString.push_back("bool executePlannedAction() ");
    }
    if (functionName=="checkGoalCompleted") {
      helpString.push_back("bool checkGoalCompleted() ");
    }
    if (functionName=="run1Step") {
      helpString.push_back("bool run1Step() ");
    }
    if (functionName=="showPlannedAction") {
      helpString.push_back("std::string showPlannedAction() ");
    }
    if (functionName=="showCurrentState") {
      helpString.push_back("std::string showCurrentState() ");
    }
    if (functionName=="showCurrentGoal") {
      helpString.push_back("std::string showCurrentGoal() ");
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


