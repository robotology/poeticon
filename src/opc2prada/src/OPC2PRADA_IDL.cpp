/*
 * Copyright (C) 2006-2018 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

// This is an automatically generated file.
// It could get re-generated if the ALLOW_IDL_GENERATION flag is on.

#include <OPC2PRADA_IDL.h>
#include <yarp/os/idl/WireTypes.h>



class OPC2PRADA_IDL_update : public yarp::os::Portable {
public:
  bool _return;
  void init();
  bool write(yarp::os::ConnectionWriter& connection) const override;
  bool read(yarp::os::ConnectionReader& connection) override;
};

class OPC2PRADA_IDL_loadObjects : public yarp::os::Portable {
public:
  yarp::os::Bottle _return;
  void init();
  bool write(yarp::os::ConnectionWriter& connection) const override;
  bool read(yarp::os::ConnectionReader& connection) override;
};

class OPC2PRADA_IDL_query2d : public yarp::os::Portable {
public:
  std::int32_t id;
  yarp::os::Bottle _return;
  void init(const std::int32_t id);
  bool write(yarp::os::ConnectionWriter& connection) const override;
  bool read(yarp::os::ConnectionReader& connection) override;
};

class OPC2PRADA_IDL_querytool2d : public yarp::os::Portable {
public:
  std::int32_t id;
  yarp::os::Bottle _return;
  void init(const std::int32_t id);
  bool write(yarp::os::ConnectionWriter& connection) const override;
  bool read(yarp::os::ConnectionReader& connection) override;
};

class OPC2PRADA_IDL_quit : public yarp::os::Portable {
public:
  bool _return;
  void init();
  bool write(yarp::os::ConnectionWriter& connection) const override;
  bool read(yarp::os::ConnectionReader& connection) override;
};

bool OPC2PRADA_IDL_update::write(yarp::os::ConnectionWriter& connection) const {
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

bool OPC2PRADA_IDL_loadObjects::write(yarp::os::ConnectionWriter& connection) const {
  yarp::os::idl::WireWriter writer(connection);
  if (!writer.writeListHeader(1)) return false;
  if (!writer.writeTag("loadObjects",1,1)) return false;
  return true;
}

bool OPC2PRADA_IDL_loadObjects::read(yarp::os::ConnectionReader& connection) {
  yarp::os::idl::WireReader reader(connection);
  if (!reader.readListReturn()) return false;
  if (!reader.read(_return)) {
    reader.fail();
    return false;
  }
  return true;
}

void OPC2PRADA_IDL_loadObjects::init() {
}

bool OPC2PRADA_IDL_query2d::write(yarp::os::ConnectionWriter& connection) const {
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

void OPC2PRADA_IDL_query2d::init(const std::int32_t id) {
  this->id = id;
}

bool OPC2PRADA_IDL_querytool2d::write(yarp::os::ConnectionWriter& connection) const {
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

void OPC2PRADA_IDL_querytool2d::init(const std::int32_t id) {
  this->id = id;
}

bool OPC2PRADA_IDL_quit::write(yarp::os::ConnectionWriter& connection) const {
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
yarp::os::Bottle OPC2PRADA_IDL::loadObjects() {
  yarp::os::Bottle _return;
  OPC2PRADA_IDL_loadObjects helper;
  helper.init();
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","yarp::os::Bottle OPC2PRADA_IDL::loadObjects()");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
yarp::os::Bottle OPC2PRADA_IDL::query2d(const std::int32_t id) {
  yarp::os::Bottle _return;
  OPC2PRADA_IDL_query2d helper;
  helper.init(id);
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","yarp::os::Bottle OPC2PRADA_IDL::query2d(const std::int32_t id)");
  }
  bool ok = yarp().write(helper,helper);
  return ok?helper._return:_return;
}
yarp::os::Bottle OPC2PRADA_IDL::querytool2d(const std::int32_t id) {
  yarp::os::Bottle _return;
  OPC2PRADA_IDL_querytool2d helper;
  helper.init(id);
  if (!yarp().canWrite()) {
    yError("Missing server method '%s'?","yarp::os::Bottle OPC2PRADA_IDL::querytool2d(const std::int32_t id)");
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
  std::string tag = reader.readTag();
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
    if (tag == "loadObjects") {
      yarp::os::Bottle _return;
      _return = loadObjects();
      yarp::os::idl::WireWriter writer(reader);
      if (!writer.isNull()) {
        if (!writer.writeListHeader(1)) return false;
        if (!writer.write(_return)) return false;
      }
      reader.accept();
      return true;
    }
    if (tag == "query2d") {
      std::int32_t id;
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
      std::int32_t id;
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

std::vector<std::string> OPC2PRADA_IDL::help(const std::string& functionName) {
  bool showAll=(functionName=="--all");
  std::vector<std::string> helpString;
  if(showAll) {
    helpString.emplace_back("*** Available commands:");
    helpString.emplace_back("update");
    helpString.emplace_back("loadObjects");
    helpString.emplace_back("query2d");
    helpString.emplace_back("querytool2d");
    helpString.emplace_back("quit");
    helpString.emplace_back("help");
  }
  else {
    if (functionName=="update") {
      helpString.emplace_back("bool update() ");
      helpString.emplace_back("Update the state text file. ");
      helpString.emplace_back("@return true/false on success/failure ");
    }
    if (functionName=="loadObjects") {
      helpString.emplace_back("yarp::os::Bottle loadObjects() ");
      helpString.emplace_back("Load correspondence between ID and Labels in the DataBase ");
      helpString.emplace_back("@return Bottle with ID and Objects names ");
    }
    if (functionName=="query2d") {
      helpString.emplace_back("yarp::os::Bottle query2d(const std::int32_t id) ");
      helpString.emplace_back("Bottle ");
      helpString.emplace_back("@return Bottle with 2d features ");
    }
    if (functionName=="querytool2d") {
      helpString.emplace_back("yarp::os::Bottle querytool2d(const std::int32_t id) ");
      helpString.emplace_back("Bottle ");
      helpString.emplace_back("@return Bottle with 2d features of the tool ");
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


