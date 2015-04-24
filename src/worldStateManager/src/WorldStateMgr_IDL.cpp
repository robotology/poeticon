// This is an automatically-generated file.
// It could get re-generated if the ALLOW_IDL_GENERATION flag is on.

#include <WorldStateMgr_IDL.h>
#include <yarp/os/idl/WireTypes.h>



class WorldStateMgr_IDL_dump : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class WorldStateMgr_IDL_update : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class WorldStateMgr_IDL_pause : public yarp::os::Portable {
public:
  std::string objName;
  bool _return;
  void init(const std::string& objName);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class WorldStateMgr_IDL_resume : public yarp::os::Portable {
public:
  std::string objName;
  bool _return;
  void init(const std::string& objName);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class WorldStateMgr_IDL_getColorHist : public yarp::os::Portable {
public:
  int32_t u;
  int32_t v;
  yarp::os::Bottle _return;
  void init(const int32_t u, const int32_t v);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class WorldStateMgr_IDL_quit : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

bool WorldStateMgr_IDL_dump::write(yarp::os::ConnectionWriter& connection) {
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

bool WorldStateMgr_IDL_update::write(yarp::os::ConnectionWriter& connection) {
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

bool WorldStateMgr_IDL_pause::write(yarp::os::ConnectionWriter& connection) {
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

bool WorldStateMgr_IDL_resume::write(yarp::os::ConnectionWriter& connection) {
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

bool WorldStateMgr_IDL_getColorHist::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(3)) return false;
  if (!writer.writeTag("getColorHist",1,1)) return false;
  if (!writer.writeI32(u)) return false;
  if (!writer.writeI32(v)) return false;
  return true;
}

bool WorldStateMgr_IDL_getColorHist::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.read(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void WorldStateMgr_IDL_getColorHist::init(const int32_t u, const int32_t v) {
  this->u = u;
  this->v = v;
}

bool WorldStateMgr_IDL_quit::write(yarp::os::ConnectionWriter& connection) {
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
yarp::os::Bottle WorldStateMgr_IDL::getColorHist(const int32_t u, const int32_t v) {
  yarp::os::Bottle _return;
  WorldStateMgr_IDL_getColorHist helper;
  helper.init(u,v);
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","yarp::os::Bottle WorldStateMgr_IDL::getColorHist(const int32_t u, const int32_t v)");
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
  yarp::os::ConstString tag = reader.readTag();
  bool direct = (tag=="__direct__");
  if (direct) tag = reader.readTag();
  while (!reader.isError()) {
    // TODO: use quick lookup, this is just a test
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
    if (tag == "getColorHist") {
      int32_t u;
      int32_t v;
      if (!reader.readI32(u)) {
        reader.fail();
        return false;
      }
      if (!reader.readI32(v)) {
        reader.fail();
        return false;
      }
      yarp::os::Bottle _return;
      _return = getColorHist(u,v);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.write(_return)) return false;
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

std::vector<std::string> WorldStateMgr_IDL::help(const std::string& functionName) {
  bool showAll=(functionName=="--all");
  std::vector<std::string> helpString;
  if(showAll) {
    helpString.push_back("*** Available commands:");
    helpString.push_back("dump");
    helpString.push_back("update");
    helpString.push_back("pause");
    helpString.push_back("resume");
    helpString.push_back("getColorHist");
    helpString.push_back("quit");
    helpString.push_back("help");
  }
  else {
    if (functionName=="dump") {
      helpString.push_back("bool dump() ");
      helpString.push_back("Print information the current world state on the screen. ");
      helpString.push_back("@return true/false on success/failure ");
    }
    if (functionName=="update") {
      helpString.push_back("bool update() ");
      helpString.push_back("Update the world state database. ");
      helpString.push_back("If the module was started in perception mode (default), the new state will ");
      helpString.push_back("be created from robot perception. If the module was started in playback ");
      helpString.push_back("mode, the new state will be created from the next time instant in the ");
      helpString.push_back("world state text file. ");
      helpString.push_back("@return true/false on success/failure ");
    }
    if (functionName=="pause") {
      helpString.push_back("bool pause(const std::string& objName) ");
      helpString.push_back("Pauses a specific stacking thread. This will ");
      helpString.push_back("pause the required tracking thread with the name ");
      helpString.push_back("provided by the user. ");
      helpString.push_back("@param objName specifies the label of the tracking thread ");
      helpString.push_back("to be paused ");
      helpString.push_back("@return true/false on success/failure ");
    }
    if (functionName=="resume") {
      helpString.push_back("bool resume(const std::string& objName) ");
      helpString.push_back("Resumes a specific stacking thread. This will ");
      helpString.push_back("resume the required tracking thread with the name ");
      helpString.push_back("provided by the user. ");
      helpString.push_back("@param objName specifies the label of the tracking thread ");
      helpString.push_back("to be resumed ");
      helpString.push_back("@return true/false on success/failure ");
    }
    if (functionName=="getColorHist") {
      helpString.push_back("yarp::os::Bottle getColorHist(const int32_t u, const int32_t v) ");
      helpString.push_back("Get the color histogram of the object requested by the user. ");
      helpString.push_back("@param u specifies the u coordinate of the object ");
      helpString.push_back("@param v specifies the v coordinate of the object ");
      helpString.push_back("@return Bottle containing color histogram ");
    }
    if (functionName=="quit") {
      helpString.push_back("bool quit() ");
      helpString.push_back("Quit the module. ");
      helpString.push_back("@return true/false on success/failure ");
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


