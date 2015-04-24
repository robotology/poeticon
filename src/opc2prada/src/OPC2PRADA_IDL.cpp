// This is an automatically-generated file.
// It could get re-generated if the ALLOW_IDL_GENERATION flag is on.

#include <OPC2PRADA_IDL.h>
#include <yarp/os/idl/WireTypes.h>



class OPC2PRADA_IDL_update : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class OPC2PRADA_IDL_query2d : public yarp::os::Portable {
public:
  int32_t id;
  yarp::os::Bottle _return;
  void init(const int32_t id);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class OPC2PRADA_IDL_querytool2d : public yarp::os::Portable {
public:
  int32_t id;
  yarp::os::Bottle _return;
  void init(const int32_t id);
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

class OPC2PRADA_IDL_quit : public yarp::os::Portable {
public:
  bool _return;
  void init();
  virtual bool write(yarp::os::ConnectionWriter& connection);
  virtual bool read(yarp::os::ConnectionReader& connection);
};

bool OPC2PRADA_IDL_update::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("update",1,1)) return false;
  return true;
}

bool OPC2PRADA_IDL_update::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void OPC2PRADA_IDL_update::init() {
  _return = false;
}

bool OPC2PRADA_IDL_query2d::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(2)) return false;
  if (!writer.writeTag("query2d",1,1)) return false;
  if (!writer.writeI32(id)) return false;
  return true;
}

bool OPC2PRADA_IDL_query2d::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.read(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void OPC2PRADA_IDL_query2d::init(const int32_t id) {
  this->id = id;
}

bool OPC2PRADA_IDL_querytool2d::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(2)) return false;
  if (!writer.writeTag("querytool2d",1,1)) return false;
  if (!writer.writeI32(id)) return false;
  return true;
}

bool OPC2PRADA_IDL_querytool2d::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.read(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void OPC2PRADA_IDL_querytool2d::init(const int32_t id) {
  this->id = id;
}

bool OPC2PRADA_IDL_quit::write(yarp::os::ConnectionWriter& connection) {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("quit",1,1)) return false;
  return true;
}

bool OPC2PRADA_IDL_quit::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.readBool(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void OPC2PRADA_IDL_quit::init() {
  _return = false;
}

OPC2PRADA_IDL::OPC2PRADA_IDL() {
  yarp().setOwner(*this);
}
bool OPC2PRADA_IDL::update() {
  bool _return = false;
  OPC2PRADA_IDL_update helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool OPC2PRADA_IDL::update()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
yarp::os::Bottle OPC2PRADA_IDL::query2d(const int32_t id) {
  yarp::os::Bottle _return;
  OPC2PRADA_IDL_query2d helper;
  helper.init(id);
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","yarp::os::Bottle OPC2PRADA_IDL::query2d(const int32_t id)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
yarp::os::Bottle OPC2PRADA_IDL::querytool2d(const int32_t id) {
  yarp::os::Bottle _return;
  OPC2PRADA_IDL_querytool2d helper;
  helper.init(id);
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","yarp::os::Bottle OPC2PRADA_IDL::querytool2d(const int32_t id)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
bool OPC2PRADA_IDL::quit() {
  bool _return = false;
  OPC2PRADA_IDL_quit helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","bool OPC2PRADA_IDL::quit()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}

bool OPC2PRADA_IDL::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  reader.expectAccept();
  if (!reader.readListHeader()) { reader.fail(); return false; }
  yarp::os::ConstString tag = reader.readTag();
  bool direct = (tag=="__direct__");
  if (direct) tag = reader.readTag();
  while (!reader.isError()) {
    // TODO: use quick lookup, this is just a test
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
    if (tag == "query2d") {
      int32_t id;
      if (!reader.readI32(id)) {
        reader.fail();
        return false;
      }
      yarp::os::Bottle _return;
      _return = query2d(id);
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.write(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "querytool2d") {
      int32_t id;
      if (!reader.readI32(id)) {
        reader.fail();
        return false;
      }
      yarp::os::Bottle _return;
      _return = querytool2d(id);
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

std::vector<std::string> OPC2PRADA_IDL::help(const std::string& functionName) {
  bool showAll=(functionName=="--all");
  std::vector<std::string> helpString;
  if(showAll) {
    helpString.push_back("*** Available commands:");
    helpString.push_back("update");
    helpString.push_back("query2d");
    helpString.push_back("querytool2d");
    helpString.push_back("quit");
    helpString.push_back("help");
  }
  else {
    if (functionName=="update") {
      helpString.push_back("bool update() ");
      helpString.push_back("Update the state text file. ");
      helpString.push_back("@return true/false on success/failure ");
    }
    if (functionName=="query2d") {
      helpString.push_back("yarp::os::Bottle query2d(const int32_t id) ");
      helpString.push_back("Bottle ");
      helpString.push_back("@return Bottle with 2d features ");
    }
    if (functionName=="querytool2d") {
      helpString.push_back("yarp::os::Bottle querytool2d(const int32_t id) ");
      helpString.push_back("Bottle ");
      helpString.push_back("@return Bottle with 2d features of the tool ");
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


