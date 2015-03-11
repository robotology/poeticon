// This is an automatically-generated file.
// It could get re-generated if the ALLOW_IDL_GENERATION flag is on.

#include <activityInterface_IDLServer.h>
#include <yarp/os/idl/WireTypes.h>



class activityInterface_IDLServer_getManip : public yarp::os::Portable {
public:
  std::string objName;
  std::string handName;
  double _return;
  void init(const std::string& objName, const std::string& handName);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activityInterface_IDLServer_handStat : public yarp::os::Portable {
public:
  std::string handName;
  bool _return;
  void init(const std::string& handName);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activityInterface_IDLServer_getLabel : public yarp::os::Portable {
public:
  int32_t xpos;
  int32_t ypos;
  std::string _return;
  void init(const int32_t xpos, const int32_t ypos);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activityInterface_IDLServer_inHand : public yarp::os::Portable {
public:
  std::string objName;
  std::string _return;
  void init(const std::string& objName);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activityInterface_IDLServer_get3D : public yarp::os::Portable {
public:
  std::string objName;
  yarp::os::Bottle _return;
  void init(const std::string& objName);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activityInterface_IDLServer_getOffset : public yarp::os::Portable {
public:
  std::string objName;
  yarp::os::Bottle _return;
  void init(const std::string& objName);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activityInterface_IDLServer_take : public yarp::os::Portable {
public:
  std::string objName;
  std::string handName;
  bool _return;
  void init(const std::string& objName, const std::string& handName);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activityInterface_IDLServer_drop : public yarp::os::Portable {
public:
  std::string objName;
  std::string targetName;
  bool _return;
  void init(const std::string& objName, const std::string& targetName);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activityInterface_IDLServer_geto : public yarp::os::Portable {
public:
  std::string handName;
  int32_t xpos;
  int32_t ypos;
  bool _return;
  void init(const std::string& handName, const int32_t xpos, const int32_t ypos);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activityInterface_IDLServer_underOf : public yarp::os::Portable {
public:
  std::string objName;
  yarp::os::Bottle _return;
  void init(const std::string& objName);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class activityInterface_IDLServer_quit : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

bool activityInterface_IDLServer_getManip::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(3)) return false;
  if (!writer.writeTag("getManip",1,1)) return false;
  if (!writer.writeString(objName)) return false;
  if (!writer.writeString(handName)) return false;
  return true;
}

bool activityInterface_IDLServer_getManip::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readDouble(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activityInterface_IDLServer_getManip::init(const std::string& objName, const std::string& handName) {
  _return = (double)0;
  this->objName = objName;
  this->handName = handName;
}

bool activityInterface_IDLServer_handStat::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(2)) return false;
  if (!writer.writeTag("handStat",1,1)) return false;
  if (!writer.writeString(handName)) return false;
  return true;
}

bool activityInterface_IDLServer_handStat::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activityInterface_IDLServer_handStat::init(const std::string& handName) {
  _return = false;
  this->handName = handName;
}

bool activityInterface_IDLServer_getLabel::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(3)) return false;
  if (!writer.writeTag("getLabel",1,1)) return false;
  if (!writer.writeI32(xpos)) return false;
  if (!writer.writeI32(ypos)) return false;
  return true;
}

bool activityInterface_IDLServer_getLabel::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readString(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activityInterface_IDLServer_getLabel::init(const int32_t xpos, const int32_t ypos) {
  _return = "";
  this->xpos = xpos;
  this->ypos = ypos;
}

bool activityInterface_IDLServer_inHand::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(2)) return false;
  if (!writer.writeTag("inHand",1,1)) return false;
  if (!writer.writeString(objName)) return false;
  return true;
}

bool activityInterface_IDLServer_inHand::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readString(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activityInterface_IDLServer_inHand::init(const std::string& objName) {
  _return = "";
  this->objName = objName;
}

bool activityInterface_IDLServer_get3D::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(2)) return false;
  if (!writer.writeTag("get3D",1,1)) return false;
  if (!writer.writeString(objName)) return false;
  return true;
}

bool activityInterface_IDLServer_get3D::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.read(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activityInterface_IDLServer_get3D::init(const std::string& objName) {
  this->objName = objName;
}

bool activityInterface_IDLServer_getOffset::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(2)) return false;
  if (!writer.writeTag("getOffset",1,1)) return false;
  if (!writer.writeString(objName)) return false;
  return true;
}

bool activityInterface_IDLServer_getOffset::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.read(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activityInterface_IDLServer_getOffset::init(const std::string& objName) {
  this->objName = objName;
}

bool activityInterface_IDLServer_take::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(3)) return false;
  if (!writer.writeTag("take",1,1)) return false;
  if (!writer.writeString(objName)) return false;
  if (!writer.writeString(handName)) return false;
  return true;
}

bool activityInterface_IDLServer_take::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activityInterface_IDLServer_take::init(const std::string& objName, const std::string& handName) {
  _return = false;
  this->objName = objName;
  this->handName = handName;
}

bool activityInterface_IDLServer_drop::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(3)) return false;
  if (!writer.writeTag("drop",1,1)) return false;
  if (!writer.writeString(objName)) return false;
  if (!writer.writeString(targetName)) return false;
  return true;
}

bool activityInterface_IDLServer_drop::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activityInterface_IDLServer_drop::init(const std::string& objName, const std::string& targetName) {
  _return = false;
  this->objName = objName;
  this->targetName = targetName;
}

bool activityInterface_IDLServer_geto::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(4)) return false;
  if (!writer.writeTag("geto",1,1)) return false;
  if (!writer.writeString(handName)) return false;
  if (!writer.writeI32(xpos)) return false;
  if (!writer.writeI32(ypos)) return false;
  return true;
}

bool activityInterface_IDLServer_geto::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activityInterface_IDLServer_geto::init(const std::string& handName, const int32_t xpos, const int32_t ypos) {
  _return = false;
  this->handName = handName;
  this->xpos = xpos;
  this->ypos = ypos;
}

bool activityInterface_IDLServer_underOf::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(2)) return false;
  if (!writer.writeTag("underOf",1,1)) return false;
  if (!writer.writeString(objName)) return false;
  return true;
}

bool activityInterface_IDLServer_underOf::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.read(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activityInterface_IDLServer_underOf::init(const std::string& objName) {
  this->objName = objName;
}

bool activityInterface_IDLServer_quit::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("quit",1,1)) return false;
  return true;
}

bool activityInterface_IDLServer_quit::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void activityInterface_IDLServer_quit::init() {
  _return = false;
}

activityInterface_IDLServer::activityInterface_IDLServer() {
  yarp().setOwner(*this);
}
double activityInterface_IDLServer::getManip(const std::string& objName, const std::string& handName) {
  double _return = (double)0;
  activityInterface_IDLServer_getManip helper;
  helper.init(objName,handName);
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","double activityInterface_IDLServer::getManip(const std::string& objName, const std::string& handName)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool activityInterface_IDLServer::handStat(const std::string& handName) {
  bool _return = false;
  activityInterface_IDLServer_handStat helper;
  helper.init(handName);
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","bool activityInterface_IDLServer::handStat(const std::string& handName)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
std::string activityInterface_IDLServer::getLabel(const int32_t xpos, const int32_t ypos) {
  std::string _return = "";
  activityInterface_IDLServer_getLabel helper;
  helper.init(xpos,ypos);
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","std::string activityInterface_IDLServer::getLabel(const int32_t xpos, const int32_t ypos)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
std::string activityInterface_IDLServer::inHand(const std::string& objName) {
  std::string _return = "";
  activityInterface_IDLServer_inHand helper;
  helper.init(objName);
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","std::string activityInterface_IDLServer::inHand(const std::string& objName)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
yarp::os::Bottle activityInterface_IDLServer::get3D(const std::string& objName) {
  yarp::os::Bottle _return;
  activityInterface_IDLServer_get3D helper;
  helper.init(objName);
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","yarp::os::Bottle activityInterface_IDLServer::get3D(const std::string& objName)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
yarp::os::Bottle activityInterface_IDLServer::getOffset(const std::string& objName) {
  yarp::os::Bottle _return;
  activityInterface_IDLServer_getOffset helper;
  helper.init(objName);
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","yarp::os::Bottle activityInterface_IDLServer::getOffset(const std::string& objName)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool activityInterface_IDLServer::take(const std::string& objName, const std::string& handName) {
  bool _return = false;
  activityInterface_IDLServer_take helper;
  helper.init(objName,handName);
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","bool activityInterface_IDLServer::take(const std::string& objName, const std::string& handName)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool activityInterface_IDLServer::drop(const std::string& objName, const std::string& targetName) {
  bool _return = false;
  activityInterface_IDLServer_drop helper;
  helper.init(objName,targetName);
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","bool activityInterface_IDLServer::drop(const std::string& objName, const std::string& targetName)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool activityInterface_IDLServer::geto(const std::string& handName, const int32_t xpos, const int32_t ypos) {
  bool _return = false;
  activityInterface_IDLServer_geto helper;
  helper.init(handName,xpos,ypos);
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","bool activityInterface_IDLServer::geto(const std::string& handName, const int32_t xpos, const int32_t ypos)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
yarp::os::Bottle activityInterface_IDLServer::underOf(const std::string& objName) {
  yarp::os::Bottle _return;
  activityInterface_IDLServer_underOf helper;
  helper.init(objName);
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","yarp::os::Bottle activityInterface_IDLServer::underOf(const std::string& objName)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool activityInterface_IDLServer::quit() {
  bool _return = false;
  activityInterface_IDLServer_quit helper;
  helper.init();
  if (!yarp().canWrite()) {
    fprintf(stderr,"Missing server method '%s'?\n","bool activityInterface_IDLServer::quit()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}

bool activityInterface_IDLServer::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  reader.expectAccept();
  if (!reader.readListHeader()) { reader.fail(); return false; }
  yarp::os::ConstString tag = reader.readTag();
  bool direct = (tag=="__direct__");
  if (direct) tag = reader.readTag();
  while (!reader.isError()) {
    // TODO: use quick lookup, this is just a test
    if (tag == "getManip") {
      std::string objName;
      std::string handName;
      if (!reader.readString(objName)) {
        reader.fail();
        return false;
      }
      if (!reader.readString(handName)) {
        reader.fail();
        return false;
      }
      double _return;
      _return = getManip(objName,handName);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeDouble(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "handStat") {
      std::string handName;
      if (!reader.readString(handName)) {
        reader.fail();
        return false;
      }
      bool _return;
      _return = handStat(handName);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "getLabel") {
      int32_t xpos;
      int32_t ypos;
      if (!reader.readI32(xpos)) {
        reader.fail();
        return false;
      }
      if (!reader.readI32(ypos)) {
        reader.fail();
        return false;
      }
      std::string _return;
      _return = getLabel(xpos,ypos);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeString(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "inHand") {
      std::string objName;
      if (!reader.readString(objName)) {
        reader.fail();
        return false;
      }
      std::string _return;
      _return = inHand(objName);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeString(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "get3D") {
      std::string objName;
      if (!reader.readString(objName)) {
        reader.fail();
        return false;
      }
      yarp::os::Bottle _return;
      _return = get3D(objName);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.write(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "getOffset") {
      std::string objName;
      if (!reader.readString(objName)) {
        reader.fail();
        return false;
      }
      yarp::os::Bottle _return;
      _return = getOffset(objName);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.write(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "take") {
      std::string objName;
      std::string handName;
      if (!reader.readString(objName)) {
        reader.fail();
        return false;
      }
      if (!reader.readString(handName)) {
        reader.fail();
        return false;
      }
      bool _return;
      _return = take(objName,handName);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "drop") {
      std::string objName;
      std::string targetName;
      if (!reader.readString(objName)) {
        reader.fail();
        return false;
      }
      if (!reader.readString(targetName)) {
        reader.fail();
        return false;
      }
      bool _return;
      _return = drop(objName,targetName);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "geto") {
      std::string handName;
      int32_t xpos;
      int32_t ypos;
      if (!reader.readString(handName)) {
        reader.fail();
        return false;
      }
      if (!reader.readI32(xpos)) {
        reader.fail();
        return false;
      }
      if (!reader.readI32(ypos)) {
        reader.fail();
        return false;
      }
      bool _return;
      _return = geto(handName,xpos,ypos);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.writeBool(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "underOf") {
      std::string objName;
      if (!reader.readString(objName)) {
        reader.fail();
        return false;
      }
      yarp::os::Bottle _return;
      _return = underOf(objName);
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

std::vector<std::string> activityInterface_IDLServer::help(const std::string& functionName) {
  bool showAll=(functionName=="--all");
  std::vector<std::string> helpString;
  if(showAll) {
    helpString.push_back("*** Available commands:");
    helpString.push_back("getManip");
    helpString.push_back("handStat");
    helpString.push_back("getLabel");
    helpString.push_back("inHand");
    helpString.push_back("get3D");
    helpString.push_back("getOffset");
    helpString.push_back("take");
    helpString.push_back("drop");
    helpString.push_back("geto");
    helpString.push_back("underOf");
    helpString.push_back("quit");
    helpString.push_back("help");
  }
  else {
    if (functionName=="getManip") {
      helpString.push_back("double getManip(const std::string& objName, const std::string& handName) ");
      helpString.push_back("Asks for the manipulability percentage ");
      helpString.push_back("@param objName specifies the name of the target object ");
      helpString.push_back("@param handName specifies the name of hand to query ");
      helpString.push_back("@return double of best manipulability ");
    }
    if (functionName=="handStat") {
      helpString.push_back("bool handStat(const std::string& handName) ");
      helpString.push_back("Get the status of the left or right hand ");
      helpString.push_back("@param handName specifies the name of the hand in question ");
      helpString.push_back("@return true/false on holding or not ");
    }
    if (functionName=="getLabel") {
      helpString.push_back("std::string getLabel(const int32_t xpos, const int32_t ypos) ");
      helpString.push_back("Get the label of the object located in the vicinity of xpo and ypos ");
      helpString.push_back("@param xpos specifies the 2D position of the object on the X axis ");
      helpString.push_back("@param ypos specifies the 2D position of the object on the Y axis ");
      helpString.push_back("@return string with the name of the object ");
    }
    if (functionName=="inHand") {
      helpString.push_back("std::string inHand(const std::string& objName) ");
      helpString.push_back("Figure out if the requested object is actually located in the hand of the robot ");
      helpString.push_back("@param objName specifies the name of the object ");
      helpString.push_back("@return string with the name of the hand left, right or none if not located in any hand. ");
    }
    if (functionName=="get3D") {
      helpString.push_back("yarp::os::Bottle get3D(const std::string& objName) ");
      helpString.push_back("Get the 3D position of the object requested by the user. ");
      helpString.push_back("@param objName specifies the name of the object ");
      helpString.push_back("@return Bottle containing 3D position ");
    }
    if (functionName=="getOffset") {
      helpString.push_back("yarp::os::Bottle getOffset(const std::string& objName) ");
      helpString.push_back("Get the tool offset position of the object requested by the user. ");
      helpString.push_back("@param objName specifies the name of the object (typically tool) ");
      helpString.push_back("@return Bottle containing 3D offset ");
    }
    if (functionName=="take") {
      helpString.push_back("bool take(const std::string& objName, const std::string& handName) ");
      helpString.push_back("Perform the take action on the particular object with the particular hand ");
      helpString.push_back("@param objName specifies the name of the object in question ");
      helpString.push_back("@param handName specifies the name of the hand in question ");
      helpString.push_back("@return true/false on taking or not ");
    }
    if (functionName=="drop") {
      helpString.push_back("bool drop(const std::string& objName, const std::string& targetName) ");
      helpString.push_back("Perform the drop action on the particular object with the particular hand ");
      helpString.push_back("@param objName specifies the name of the object in question ");
      helpString.push_back("@param targetName specifies the name of target object to drop onto. ");
      helpString.push_back("@return true/false on droping or not ");
    }
    if (functionName=="geto") {
      helpString.push_back("bool geto(const std::string& handName, const int32_t xpos, const int32_t ypos) ");
      helpString.push_back("Perform the take action on the particular tool with the particular hand ");
      helpString.push_back("@param objName specifies the name of the object in question ");
      helpString.push_back("@param handName specifies the name of the hand in question ");
      helpString.push_back("@return true/false on taking or not ");
    }
    if (functionName=="underOf") {
      helpString.push_back("yarp::os::Bottle underOf(const std::string& objName) ");
      helpString.push_back("Ask for the list of labels that are under ojbName ");
      helpString.push_back("@param objName specifies the name of the object in question ");
      helpString.push_back("@return Bottle containing list of labels that are under objName ");
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


