// This is an automatically-generated file.
// It could get re-generated if the ALLOW_IDL_GENERATION flag is on.

#ifndef YARP_THRIFT_GENERATOR_activeParticleTrack_IDLServer
#define YARP_THRIFT_GENERATOR_activeParticleTrack_IDLServer

#include <yarp/os/Wire.h>
#include <yarp/os/idl/WireTypes.h>
#include <yarp/os/Bottle.h>

class activeParticleTrack_IDLServer;


class activeParticleTrack_IDLServer : public yarp::os::Wire {
public:
  activeParticleTrack_IDLServer();
  /**
   * Displays all the traking particles.
   * @param value specifies the boolean value for
   * displaying the particle or not. Use on/off.
   * @return true/false on success/failure
   */
  virtual bool display(const std::string& value);
  /**
   * Gets the list of ID being tracked
   * @return true/false on success/failure
   */
  virtual yarp::os::Bottle getIDs();
  /**
   * Gets the list of ID actually being paused
   * @return true/false on success/failure
   */
  virtual yarp::os::Bottle getPausedIDs();
  /**
   * Track a fixation point. This initializes the
   * segmentation of the area of interest for subsequent
   * tracking.
   * @param fix_x specifies the x coordinate of the
   * required fixation point for segmentation
   * @param fix_x specifies the y coordinate of the
   * required fixation point for segmentation
   * @return i32 of thread index
   */
  virtual int32_t track(const double fix_x, const double fix_y);
  /**
   * Deletes a specific stacking thread. This will
   * delete the required tracking thread with the id
   * provided by the user.
   * @param id specifies the id of the tracking thread
   * to be deleted
   * @return true/false on success/failure
   */
  virtual bool untrack(const int32_t id);
  /**
   * Pauses a specific stacking thread. This will
   * pause the required tracking thread with the id
   * provided by the user.
   * @param id specifies the id of the tracking thread
   * to be paused
   * @return true/false on success/failure
   */
  virtual bool pause(const int32_t id);
  /**
   * Resumes a specific stacking thread. This will
   * resume the required tracking thread with the id
   * provided by the user.
   * @param id specifies the id of the tracking thread
   * to be resumed
   * @return true/false on success/failure
   */
  virtual bool resume(const int32_t id);
  /**
   * Sets the tracker to the user desired index.
   * Usesul for synchronisation issues.
   * provided by the user.
   * @param id specifies the index to start incrementing from
   * @return true/false on success/failure
   */
  virtual bool countFrom(const int32_t id);
  /**
   * Reset all the trakers. This is used in case the user
   * would like to reset the whole tracking process.
   * @return true/false on success/failure
   */
  virtual bool reset();
  /**
   * Quit the module.
   * @return true/false on success/failure
   */
  virtual bool quit();
  virtual bool read(yarp::os::ConnectionReader& connection);
  virtual std::vector<std::string> help(const std::string& functionName="--all");
};

#endif

