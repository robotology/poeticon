/*
 * Copyright (C) 2006-2018 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

// This is an automatically generated file.
// It could get re-generated if the ALLOW_IDL_GENERATION flag is on.

#include <WorldStateMgr_IDL.h>
#include <yarp/os/idl/WireTypes.h>



class WorldStateMgr_IDL_init : public yarp::os::Portable {
public:
  bool _return;
  void init();
  bool write(yarp::os::ConnectionWriter& connection) const override;
  bool read(yarp::os::ConnectionReader& connection) override;
};

class WorldStateMgr_IDL_isInitialized : public yarp::os::Portable {
public:
  bool _return;
  void init();
  bool write(yarp::os::ConnectionWriter& connection) const override;
  bool read(yarp::os::ConnectionReader& connection) override;
};

class WorldStateMgr_IDL_dump : public yarp::os::Portable {
public:
  bool _return;
  void init();
  bool write(yarp::os::ConnectionWriter& connection) const override;
  bool read(yarp::os::ConnectionReader& connection) override;
};

class WorldStateMgr_IDL_update : public yarp::os::Portable {
public:
  bool _return;
  void init();
  bool write(yarp::os::ConnectionWriter& connection) const override;
  bool read(yarp::os::ConnectionReader& connection) override;
};

class WorldStateMgr_IDL_reset : public yarp::os::Portable {
public:
  bool _return;
  void init();
  bool write(yarp::os::ConnectionWriter& connection) const override;
  bool read(yarp::os::ConnectionReader& connection) override;
};

class WorldStateMgr_IDL_pause : public yarp::os::Portable {
public:
  std::string objName;
  bool _return;
  void init(const std::string& objName);
  bool write(yarp::os::ConnectionWriter& connection) const override;
  bool read(yarp::os::ConnectionReader& connection) override;
};

class WorldStateMgr_IDL_resume : public yarp::os::Portable {
public:
  std::string objName;
  bool _return;
  void init(const std::string& objName);
  bool write(yarp::os::ConnectionWriter& connection) const override;
  bool read(yarp::os::ConnectionReader& connection) override;
};

class WorldStateMgr_IDL_pauseID : public yarp::os::Portable {
public:
  std::int32_t objID;
  bool _return;
  void init(const std::int32_t objID);
  bool write(yarp::os::ConnectionWriter& connection) const override;
  bool read(yarp::os::ConnectionReader& connection) override;
};

class WorldStateMgr_IDL_resumeID : public yarp::os::Portable {
public:
  std::int32_t objID;
  bool _return;
  void init(const std::int32_t objID);
  bool write(yarp::os::ConnectionWriter& connection) const override;
  bool read(yarp::os::ConnectionReader& connection) override;
};

class WorldStateMgr_IDL_quit : public yarp::os::Portable {
public:
  bool _return;
  void init();
  bool write(yarp::os::ConnectionWriter& connection) const override;
  bool read(yarp::os::ConnectionReader& connection) override;
};

bool WorldStateMgr_IDL_init::write(yarp::os::ConnectionWriter& connection) const {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("init",1,1)) return false;
  return true;
}

bool WorldStateMgr_IDL_init::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void WorldStateMgr_IDL_init::init() {
  _return = false;
}

bool WorldStateMgr_IDL_isInitialized::write(yarp::os::ConnectionWriter& connection) const {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("isInitialized",1,1)) return false;
  return true;
}

bool WorldStateMgr_IDL_isInitialized::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void WorldStateMgr_IDL_isInitialized::init() {
  _return = false;
}

bool WorldStateMgr_IDL_dump::write(yarp::os::ConnectionWriter& connection) const {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("dump",1,1)) return false;
  return true;
}

bool WorldStateMgr_IDL_dump::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void WorldStateMgr_IDL_dump::init() {
  _return = false;
}

bool WorldStateMgr_IDL_update::write(yarp::os::ConnectionWriter& connection) const {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("update",1,1)) return false;
  return true;
}

bool WorldStateMgr_IDL_update::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void WorldStateMgr_IDL_update::init() {
  _return = false;
}

bool WorldStateMgr_IDL_reset::write(yarp::os::ConnectionWriter& connection) const {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("reset",1,1)) return false;
  return true;
}

bool WorldStateMgr_IDL_reset::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void WorldStateMgr_IDL_reset::init() {
  _return = false;
}

bool WorldStateMgr_IDL_pause::write(yarp::os::ConnectionWriter& connection) const {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(2)) return false;
  if (!writer.writeTag("pause",1,1)) return false;
  if (!writer.writeString(objName)) return false;
  return true;
}

bool WorldStateMgr_IDL_pause::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void WorldStateMgr_IDL_pause::init(const std::string& objName) {
  _return = false;
  this->objName = objName;
}

bool WorldStateMgr_IDL_resume::write(yarp::os::ConnectionWriter& connection) const {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(2)) return false;
  if (!writer.writeTag("resume",1,1)) return false;
  if (!writer.writeString(objName)) return false;
  return true;
}

bool WorldStateMgr_IDL_resume::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void WorldStateMgr_IDL_resume::init(const std::string& objName) {
  _return = false;
  this->objName = objName;
}

bool WorldStateMgr_IDL_pauseID::write(yarp::os::ConnectionWriter& connection) const {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(2)) return false;
  if (!writer.writeTag("pauseID",1,1)) return false;
  if (!writer.writeI32(objID)) return false;
  return true;
}

bool WorldStateMgr_IDL_pauseID::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void WorldStateMgr_IDL_pauseID::init(const std::int32_t objID) {
  _return = false;
  this->objID = objID;
}

bool WorldStateMgr_IDL_resumeID::write(yarp::os::ConnectionWriter& connection) const {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(2)) return false;
  if (!writer.writeTag("resumeID",1,1)) return false;
  if (!writer.writeI32(objID)) return false;
  return true;
}

bool WorldStateMgr_IDL_resumeID::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void WorldStateMgr_IDL_resumeID::init(const std::int32_t objID) {
  _return = false;
  this->objID = objID;
}

bool WorldStateMgr_IDL_quit::write(yarp::os::ConnectionWriter& connection) const {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("quit",1,1)) return false;
  return true;
}

bool WorldStateMgr_IDL_quit::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void WorldStateMgr_IDL_quit::init() {
  _return = false;
}

WorldStateMgr_IDL::WorldStateMgr_IDL() {
  yarp().setOwner(*this);
}
bool WorldStateMgr_IDL::init() {
  bool _return = false;
  WorldStateMgr_IDL_init helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool WorldStateMgr_IDL::init()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool WorldStateMgr_IDL::isInitialized() {
  bool _return = false;
  WorldStateMgr_IDL_isInitialized helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool WorldStateMgr_IDL::isInitialized()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool WorldStateMgr_IDL::dump() {
  bool _return = false;
  WorldStateMgr_IDL_dump helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool WorldStateMgr_IDL::dump()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool WorldStateMgr_IDL::update() {
  bool _return = false;
  WorldStateMgr_IDL_update helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool WorldStateMgr_IDL::update()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool WorldStateMgr_IDL::reset() {
  bool _return = false;
  WorldStateMgr_IDL_reset helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool WorldStateMgr_IDL::reset()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool WorldStateMgr_IDL::pause(const std::string& objName) {
  bool _return = false;
  WorldStateMgr_IDL_pause helper;
  helper.init(objName);
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool WorldStateMgr_IDL::pause(const std::string& objName)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool WorldStateMgr_IDL::resume(const std::string& objName) {
  bool _return = false;
  WorldStateMgr_IDL_resume helper;
  helper.init(objName);
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool WorldStateMgr_IDL::resume(const std::string& objName)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool WorldStateMgr_IDL::pauseID(const std::int32_t objID) {
  bool _return = false;
  WorldStateMgr_IDL_pauseID helper;
  helper.init(objID);
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool WorldStateMgr_IDL::pauseID(const std::int32_t objID)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool WorldStateMgr_IDL::resumeID(const std::int32_t objID) {
  bool _return = false;
  WorldStateMgr_IDL_resumeID helper;
  helper.init(objID);
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool WorldStateMgr_IDL::resumeID(const std::int32_t objID)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool WorldStateMgr_IDL::quit() {
  bool _return = false;
  WorldStateMgr_IDL_quit helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool WorldStateMgr_IDL::quit()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}

bool WorldStateMgr_IDL::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  reader.expectAccept();
  if (!reader.readListHeader()) { reader.fail(); return false; }
  std::string tag = reader.readTag();
  bool direct = (tag=="__direct__");
  if (direct) tag = reader.readTag();
  while (!reader.isError()) {
    // TODO: use quick lookup, this is just a test
    if (tag == "init") {
      bool _return;
      _return = init();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "isInitialized") {
      bool _return;
      _return = isInitialized();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "dump") {
      bool _return;
      _return = dump();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "update") {
      bool _return;
      _return = update();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "reset") {
      bool _return;
      _return = reset();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "pause") {
      std::string objName;
      if (!reader.readString(objName)) {
        reader.fail();
        return false;
      }
      bool _return;
      _return = pause(objName);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "resume") {
      std::string objName;
      if (!reader.readString(objName)) {
        reader.fail();
        return false;
      }
      bool _return;
      _return = resume(objName);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "pauseID") {
      std::int32_t objID;
      if (!reader.readI32(objID)) {
        reader.fail();
        return false;
      }
      bool _return;
      _return = pauseID(objID);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "resumeID") {
      std::int32_t objID;
      if (!reader.readI32(objID)) {
        reader.fail();
        return false;
      }
      bool _return;
      _return = resumeID(objID);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
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
          if (!writer.writeListBegin(BOTTLE_TAG_INT32, static_cast<uint32_t>(_return.size()))) return false;
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
    std::string next_tag = reader.readTag();
    if (next_tag=="") break;
    tag.append("_").append(next_tag);
  }
  return false;
}

std::vector<std::string> WorldStateMgr_IDL::help(const std::string& functionName) {
  bool showAll=(functionName=="--all");
  std::vector<std::string> helpString;
  if(showAll) {
    helpString.emplace_back("*** Available commands:");
    helpString.emplace_back("init");
    helpString.emplace_back("isInitialized");
    helpString.emplace_back("dump");
    helpString.emplace_back("update");
    helpString.emplace_back("reset");
    helpString.emplace_back("pause");
    helpString.emplace_back("resume");
    helpString.emplace_back("pauseID");
    helpString.emplace_back("resumeID");
    helpString.emplace_back("quit");
    helpString.emplace_back("help");
  }
  else {
    if (functionName=="init") {
      helpString.emplace_back("bool init() ");
      helpString.emplace_back("Initialize the world state database. ");
      helpString.emplace_back("NOTE: before launching this command, make sure that segmentation and ");
      helpString.emplace_back("      object recognition are stable. ");
      helpString.emplace_back("@return true/false on success/failure ");
    }
    if (functionName=="isInitialized") {
      helpString.emplace_back("bool isInitialized() ");
      helpString.emplace_back("Check if initialization phase has been completed. This is accomplished when ");
      helpString.emplace_back("tracker, short-term memory model and WSOPC database possess all entries. ");
      helpString.emplace_back("@return true/false on initialized/uninitialized ");
    }
    if (functionName=="dump") {
      helpString.emplace_back("bool dump() ");
      helpString.emplace_back("Print information the current world state on the screen. ");
      helpString.emplace_back("@return true/false on success/failure ");
    }
    if (functionName=="update") {
      helpString.emplace_back("bool update() ");
      helpString.emplace_back("Update the world state database. ");
      helpString.emplace_back("The new state will be created from robot perception. ");
      helpString.emplace_back("@return true/false on success/failure ");
    }
    if (functionName=="reset") {
      helpString.emplace_back("bool reset() ");
      helpString.emplace_back("Reset the world state database, initializing it from scratch. ");
      helpString.emplace_back("In the WSOPC database, with respect to the previous experiment hand entries ");
      helpString.emplace_back("will get their fields cleared (but their IDs preserved), whereas object ");
      helpString.emplace_back("entries will be deleted and new ones will be created with new IDs, except ");
      helpString.emplace_back("for special object entries that are protected from deletion (objects that ");
      helpString.emplace_back("are currently hidden in a stack, and objects that are currently grasped). ");
      helpString.emplace_back("NOTE: before launching this command, make sure that segmentation and ");
      helpString.emplace_back("      object recognition are stable. ");
      helpString.emplace_back("@return true/false on success/failure ");
    }
    if (functionName=="pause") {
      helpString.emplace_back("bool pause(const std::string& objName) ");
      helpString.emplace_back("DEPRECATED ");
      helpString.emplace_back("Pauses a specific stacking thread. This will ");
      helpString.emplace_back("pause the required tracking thread with the _name_ ");
      helpString.emplace_back("provided by the user. ");
      helpString.emplace_back("@param objName specifies the label of the tracking thread ");
      helpString.emplace_back("to be paused ");
      helpString.emplace_back("@return true/false on success/failure ");
    }
    if (functionName=="resume") {
      helpString.emplace_back("bool resume(const std::string& objName) ");
      helpString.emplace_back("DEPRECATED ");
      helpString.emplace_back("Resumes a specific stacking thread. This will ");
      helpString.emplace_back("resume the required tracking thread with the _name_ ");
      helpString.emplace_back("provided by the user. ");
      helpString.emplace_back("@param objName specifies the label of the tracking thread ");
      helpString.emplace_back("to be resumed ");
      helpString.emplace_back("@return true/false on success/failure ");
    }
    if (functionName=="pauseID") {
      helpString.emplace_back("bool pauseID(const std::int32_t objID) ");
      helpString.emplace_back("DEPRECATED ");
      helpString.emplace_back("Pauses a specific stacking thread. This will ");
      helpString.emplace_back("pause the required tracking thread with the _ID_ ");
      helpString.emplace_back("provided by the user. ");
      helpString.emplace_back("@param objID specifies the numeric identifier of the tracking thread ");
      helpString.emplace_back("to be paused ");
      helpString.emplace_back("@return true/false on success/failure ");
    }
    if (functionName=="resumeID") {
      helpString.emplace_back("bool resumeID(const std::int32_t objID) ");
      helpString.emplace_back("DEPRECATED ");
      helpString.emplace_back("Resumes a specific stacking thread. This will ");
      helpString.emplace_back("resume the required tracking thread with the _ID_ ");
      helpString.emplace_back("provided by the user. ");
      helpString.emplace_back("@param objID specifies the numeric identifier of the tracking thread ");
      helpString.emplace_back("to be resumed ");
      helpString.emplace_back("@return true/false on success/failure ");
    }
    if (functionName=="quit") {
      helpString.emplace_back("bool quit() ");
      helpString.emplace_back("Quit the module. ");
      helpString.emplace_back("@return true/false on success/failure ");
    }
    if (functionName=="help") {
      helpString.emplace_back("std::vector<std::string> help(const std::string& functionName=\"--all\")");
      helpString.emplace_back("Return list of available commands, or help message for a specific function");
      helpString.emplace_back("@param functionName name of command for which to get a detailed description. If none or '--all' is provided, print list of available commands");
      helpString.emplace_back("@return list of strings (one string per line)");
    }
  }
  if ( helpString.empty()) helpString.emplace_back("Command not found");
  return helpString;
}


