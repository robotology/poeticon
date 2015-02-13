// This is an automatically-generated file.
// It could get re-generated if the ALLOW_IDL_GENERATION flag is on.

#ifndef YARP_THRIFT_GENERATOR_WorldStateMgr_IDL
#define YARP_THRIFT_GENERATOR_WorldStateMgr_IDL

#include <yarp/os/Wire.h>
#include <yarp/os/idl/WireTypes.h>

class WorldStateMgr_IDL;


class WorldStateMgr_IDL : public yarp::os::Wire {
public:
  WorldStateMgr_IDL();
  /**
   * Print information the current world state on the screen.
   * @return true/false on success/failure
   */
  virtual bool dump();
  /**
   * Update the world state database.
   * If the module was started in perception mode (default), the new state will
   * be created from robot perception. If the module was started in playback
   * mode, the new state will be created from the next time instant in the
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

