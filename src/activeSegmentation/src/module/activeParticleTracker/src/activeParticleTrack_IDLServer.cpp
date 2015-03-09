// This is an automatically-generated file.
// It could get re-generated if the ALLOW_IDL_GENERATION flag is on.

#include <activeParticleTrack_IDLServer.h>
#include <yarp/os/idl/WireTypes.h>



class activeParticleTrack_IDLServer_display : public yarp::os::Portable {
public:
  std::string value;
  bool _return;
  void init(const std::string& value);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activeParticleTrack_IDLServer_getIDs : public yarp::os::Portable {
public:
  yarp::os::Bottle _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activeParticleTrack_IDLServer_getPausedIDs : public yarp::os::Portable {
public:
  yarp::os::Bottle _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activeParticleTrack_IDLServer_track : public yarp::os::Portable {
public:
  double fix_x;
  double fix_y;
  int32_t _return;
  void init(const double fix_x, const double fix_y);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activeParticleTrack_IDLServer_untrack : public yarp::os::Portable {
public:
  int32_t id;
  bool _return;
  void init(const int32_t id);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activeParticleTrack_IDLServer_pause : public yarp::os::Portable {
public:
  int32_t id;
  bool _return;
  void init(const int32_t id);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activeParticleTrack_IDLServer_resume : public yarp::os::Portable {
public:
  int32_t id;
  bool _return;
  void init(const int32_t id);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activeParticleTrack_IDLServer_countFrom : public yarp::os::Portable {
public:
  int32_t id;
  bool _return;
  void init(const int32_t id);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activeParticleTrack_IDLServer_reset : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activeParticleTrack_IDLServer_quit : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

bool activeParticleTrack_IDLServer_display::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(2)) return false;
  if (!writer.writeTag("display",1,1)) return false;
  if (!writer.writeString(value)) return false;
  return true;
}

bool activeParticleTrack_IDLServer_display::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activeParticleTrack_IDLServer_display::init(const std::string& value) {
  _return = false;
  this->value = value;
}

bool activeParticleTrack_IDLServer_getIDs::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("getIDs",1,1)) return false;
  return true;
}

bool activeParticleTrack_IDLServer_getIDs::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.read(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activeParticleTrack_IDLServer_getIDs::init() {
}

bool activeParticleTrack_IDLServer_getPausedIDs::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("getPausedIDs",1,1)) return false;
  return true;
}

bool activeParticleTrack_IDLServer_getPausedIDs::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.read(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activeParticleTrack_IDLServer_getPausedIDs::init() {
}

bool activeParticleTrack_IDLServer_track::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(3)) return false;
  if (!writer.writeTag("track",1,1)) return false;
  if (!writer.writeDouble(fix_x)) return false;
  if (!writer.writeDouble(fix_y)) return false;
  return true;
}

bool activeParticleTrack_IDLServer_track::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readI32(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activeParticleTrack_IDLServer_track::init(const double fix_x, const double fix_y) {
  _return = 0;
  this->fix_x = fix_x;
  this->fix_y = fix_y;
}

bool activeParticleTrack_IDLServer_untrack::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(2)) return false;
  if (!writer.writeTag("untrack",1,1)) return false;
  if (!writer.writeI32(id)) return false;
  return true;
}

bool activeParticleTrack_IDLServer_untrack::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activeParticleTrack_IDLServer_untrack::init(const int32_t id) {
  _return = false;
  this->id = id;
}

bool activeParticleTrack_IDLServer_pause::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(2)) return false;
  if (!writer.writeTag("pause",1,1)) return false;
  if (!writer.writeI32(id)) return false;
  return true;
}

bool activeParticleTrack_IDLServer_pause::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activeParticleTrack_IDLServer_pause::init(const int32_t id) {
  _return = false;
  this->id = id;
}

bool activeParticleTrack_IDLServer_resume::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(2)) return false;
  if (!writer.writeTag("resume",1,1)) return false;
  if (!writer.writeI32(id)) return false;
  return true;
}

bool activeParticleTrack_IDLServer_resume::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activeParticleTrack_IDLServer_resume::init(const int32_t id) {
  _return = false;
  this->id = id;
}

bool activeParticleTrack_IDLServer_countFrom::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(2)) return false;
  if (!writer.writeTag("countFrom",1,1)) return false;
  if (!writer.writeI32(id)) return false;
  return true;
}

bool activeParticleTrack_IDLServer_countFrom::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activeParticleTrack_IDLServer_countFrom::init(const int32_t id) {
  _return = false;
  this->id = id;
}

bool activeParticleTrack_IDLServer_reset::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("reset",1,1)) return false;
  return true;
}

bool activeParticleTrack_IDLServer_reset::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activeParticleTrack_IDLServer_reset::init() {
  _return = false;
}

bool activeParticleTrack_IDLServer_quit::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("quit",1,1)) return false;
  return true;
}

bool activeParticleTrack_IDLServer_quit::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activeParticleTrack_IDLServer_quit::init() {
  _return = false;
}

activeParticleTrack_IDLServer::activeParticleTrack_IDLServer() {
  yarp().setOwner(*this);
}
bool activeParticleTrack_IDLServer::display(const std::string& value) {
  bool _return = false;
  activeParticleTrack_IDLServer_display helper;
  helper.init(value);
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","bool activeParticleTrack_IDLServer::display(const std::string& value)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
yarp::os::Bottle activeParticleTrack_IDLServer::getIDs() {
  yarp::os::Bottle _return;
  activeParticleTrack_IDLServer_getIDs helper;
  helper.init();
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","yarp::os::Bottle activeParticleTrack_IDLServer::getIDs()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
yarp::os::Bottle activeParticleTrack_IDLServer::getPausedIDs() {
  yarp::os::Bottle _return;
  activeParticleTrack_IDLServer_getPausedIDs helper;
  helper.init();
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","yarp::os::Bottle activeParticleTrack_IDLServer::getPausedIDs()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
int32_t activeParticleTrack_IDLServer::track(const double fix_x, const double fix_y) {
  int32_t _return = 0;
  activeParticleTrack_IDLServer_track helper;
  helper.init(fix_x,fix_y);
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","int32_t activeParticleTrack_IDLServer::track(const double fix_x, const double fix_y)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool activeParticleTrack_IDLServer::untrack(const int32_t id) {
  bool _return = false;
  activeParticleTrack_IDLServer_untrack helper;
  helper.init(id);
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","bool activeParticleTrack_IDLServer::untrack(const int32_t id)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool activeParticleTrack_IDLServer::pause(const int32_t id) {
  bool _return = false;
  activeParticleTrack_IDLServer_pause helper;
  helper.init(id);
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","bool activeParticleTrack_IDLServer::pause(const int32_t id)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool activeParticleTrack_IDLServer::resume(const int32_t id) {
  bool _return = false;
  activeParticleTrack_IDLServer_resume helper;
  helper.init(id);
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","bool activeParticleTrack_IDLServer::resume(const int32_t id)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool activeParticleTrack_IDLServer::countFrom(const int32_t id) {
  bool _return = false;
  activeParticleTrack_IDLServer_countFrom helper;
  helper.init(id);
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","bool activeParticleTrack_IDLServer::countFrom(const int32_t id)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool activeParticleTrack_IDLServer::reset() {
  bool _return = false;
  activeParticleTrack_IDLServer_reset helper;
  helper.init();
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","bool activeParticleTrack_IDLServer::reset()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool activeParticleTrack_IDLServer::quit() {
  bool _return = false;
  activeParticleTrack_IDLServer_quit helper;
  helper.init();
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","bool activeParticleTrack_IDLServer::quit()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}

bool activeParticleTrack_IDLServer::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  reader.expectAccept();
  if (!reader.readListHeader()) { reader.fail(); return false; }
  yarp::os::ConstString tag = reader.readTag();
  bool direct = (tag=="__direct__");
  if (direct) tag = reader.readTag();
  while (!reader.isError()) {
    // TODO: use quick lookup, this is just a test
    if (tag == "display") {
      std::string value;
      if (!reader.readString(value)) {
        reader.fail();
        return false;
      }
      bool _return;
      _return = display(value);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "getIDs") {
      yarp::os::Bottle _return;
      _return = getIDs();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.write(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "getPausedIDs") {
      yarp::os::Bottle _return;
      _return = getPausedIDs();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.write(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "track") {
      double fix_x;
      double fix_y;
      if (!reader.readDouble(fix_x)) {
        reader.fail();
        return false;
      }
      if (!reader.readDouble(fix_y)) {
        reader.fail();
        return false;
      }
      int32_t _return;
      _return = track(fix_x,fix_y);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeI32(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "untrack") {
      int32_t id;
      if (!reader.readI32(id)) {
        reader.fail();
        return false;
      }
      bool _return;
      _return = untrack(id);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "pause") {
      int32_t id;
      if (!reader.readI32(id)) {
        reader.fail();
        return false;
      }
      bool _return;
      _return = pause(id);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "resume") {
      int32_t id;
      if (!reader.readI32(id)) {
        reader.fail();
        return false;
      }
      bool _return;
      _return = resume(id);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "countFrom") {
      int32_t id;
      if (!reader.readI32(id)) {
        reader.fail();
        return false;
      }
      bool _return;
      _return = countFrom(id);
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

std::vector<std::string> activeParticleTrack_IDLServer::help(const std::string& functionName) {
  bool showAll=(functionName=="--all");
  std::vector<std::string> helpString;
  if(showAll) {
    helpString.push_back("*** Available commands:");
    helpString.push_back("display");
    helpString.push_back("getIDs");
    helpString.push_back("getPausedIDs");
    helpString.push_back("track");
    helpString.push_back("untrack");
    helpString.push_back("pause");
    helpString.push_back("resume");
    helpString.push_back("countFrom");
    helpString.push_back("reset");
    helpString.push_back("quit");
    helpString.push_back("help");
  }
  else {
    if (functionName=="display") {
      helpString.push_back("bool display(const std::string& value) ");
      helpString.push_back("Displays all the traking particles. ");
      helpString.push_back("@param value specifies the boolean value for ");
      helpString.push_back("displaying the particle or not. Use on/off. ");
      helpString.push_back("@return true/false on success/failure ");
    }
    if (functionName=="getIDs") {
      helpString.push_back("yarp::os::Bottle getIDs() ");
      helpString.push_back("Gets the list of ID being tracked ");
      helpString.push_back("@return true/false on success/failure ");
    }
    if (functionName=="getPausedIDs") {
      helpString.push_back("yarp::os::Bottle getPausedIDs() ");
      helpString.push_back("Gets the list of ID actually being paused ");
      helpString.push_back("@return true/false on success/failure ");
    }
    if (functionName=="track") {
      helpString.push_back("int32_t track(const double fix_x, const double fix_y) ");
      helpString.push_back("Track a fixation point. This initializes the ");
      helpString.push_back("segmentation of the area of interest for subsequent ");
      helpString.push_back("tracking. ");
      helpString.push_back("@param fix_x specifies the x coordinate of the ");
      helpString.push_back("required fixation point for segmentation ");
      helpString.push_back("@param fix_x specifies the y coordinate of the ");
      helpString.push_back("required fixation point for segmentation ");
      helpString.push_back("@return i32 of thread index ");
    }
    if (functionName=="untrack") {
      helpString.push_back("bool untrack(const int32_t id) ");
      helpString.push_back("Deletes a specific stacking thread. This will ");
      helpString.push_back("delete the required tracking thread with the id ");
      helpString.push_back("provided by the user. ");
      helpString.push_back("@param id specifies the id of the tracking thread ");
      helpString.push_back("to be deleted ");
      helpString.push_back("@return true/false on success/failure ");
    }
    if (functionName=="pause") {
      helpString.push_back("bool pause(const int32_t id) ");
      helpString.push_back("Pauses a specific stacking thread. This will ");
      helpString.push_back("pause the required tracking thread with the id ");
      helpString.push_back("provided by the user. ");
      helpString.push_back("@param id specifies the id of the tracking thread ");
      helpString.push_back("to be paused ");
      helpString.push_back("@return true/false on success/failure ");
    }
    if (functionName=="resume") {
      helpString.push_back("bool resume(const int32_t id) ");
      helpString.push_back("Resumes a specific stacking thread. This will ");
      helpString.push_back("resume the required tracking thread with the id ");
      helpString.push_back("provided by the user. ");
      helpString.push_back("@param id specifies the id of the tracking thread ");
      helpString.push_back("to be resumed ");
      helpString.push_back("@return true/false on success/failure ");
    }
    if (functionName=="countFrom") {
      helpString.push_back("bool countFrom(const int32_t id) ");
      helpString.push_back("Sets the tracker to the user desired index. ");
      helpString.push_back("Usesul for synchronisation issues. ");
      helpString.push_back("provided by the user. ");
      helpString.push_back("@param id specifies the index to start incrementing from ");
      helpString.push_back("@return true/false on success/failure ");
    }
    if (functionName=="reset") {
      helpString.push_back("bool reset() ");
      helpString.push_back("Reset all the trakers. This is used in case the user ");
      helpString.push_back("would like to reset the whole tracking process. ");
      helpString.push_back("@return true/false on success/failure ");
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


