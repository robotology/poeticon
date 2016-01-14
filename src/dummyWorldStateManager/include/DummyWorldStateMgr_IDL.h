// This is an automatically-generated file.
// It could get re-generated if the ALLOW_IDL_GENERATION flag is on.

#ifndef YARP_THRIFT_GENERATOR_DummyWorldStateMgr_IDL
#define YARP_THRIFT_GENERATOR_DummyWorldStateMgr_IDL

#include <yarp/os/Wire.h>
#include <yarp/os/idl/WireTypes.h>

class DummyWorldStateMgr_IDL;


class DummyWorldStateMgr_IDL : public yarp::os::Wire {
public:
  DummyWorldStateMgr_IDL();
  /**
   * Update the world state database.
   * The new state will be created from the next time instant in the
   * world state text file.
   * @return true/false on success/failure
   */
  virtual bool update();
  /**
   * Quit the module.
   * @return true/false on success/failure
   */
  virtual bool quit();
  virtual bool read(yarp::os::ConnectionReader& connection);
  virtual std::vector<std::string> help(const std::string& functionName="--all");
};

#endif
