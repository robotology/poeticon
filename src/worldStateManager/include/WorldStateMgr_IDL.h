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
   * Pauses a specific stacking thread. This will
   * pause the required tracking thread with the name
   * provided by the user.
   * @param objName specifies the label of the tracking thread
   * to be paused
   * @return true/false on success/failure
   */
  virtual bool pause(const std::string& objName);
  /**
   * Resumes a specific stacking thread. This will
   * resume the required tracking thread with the name
   * provided by the user.
   * @param objName specifies the label of the tracking thread
   * to be resumed
   * @return true/false on success/failure
   */
  virtual bool resume(const std::string& objName);
  /**
   * Quit the module.
   * @return true/false on success/failure
   */
  virtual bool quit();
  virtual bool read(yarp::os::ConnectionReader& connection);
  virtual std::vector<std::string> help(const std::string& functionName="--all");
};

#endif

