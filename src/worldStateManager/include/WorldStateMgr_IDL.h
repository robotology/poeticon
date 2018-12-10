/*
 * Copyright (C) 2006-2018 Istituto Italiano di Tecnologia (IIT)
 * All rights reserved.
 *
 * This software may be modified and distributed under the terms of the
 * BSD-3-Clause license. See the accompanying LICENSE file for details.
 */

// This is an automatically generated file.
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
   * Initialize the world state database.
   * NOTE: before launching this command, make sure that segmentation and
   *       object recognition are stable.
   * @return true/false on success/failure
   */
  virtual bool init();
  /**
   * Check if initialization phase has been completed. This is accomplished when
   * tracker, short-term memory model and WSOPC database possess all entries.
   * @return true/false on initialized/uninitialized
   */
  virtual bool isInitialized();
  /**
   * Print information the current world state on the screen.
   * @return true/false on success/failure
   */
  virtual bool dump();
  /**
   * Update the world state database.
   * The new state will be created from robot perception.
   * @return true/false on success/failure
   */
  virtual bool update();
  /**
   * Reset the world state database, initializing it from scratch.
   * In the WSOPC database, with respect to the previous experiment hand entries
   * will get their fields cleared (but their IDs preserved), whereas object
   * entries will be deleted and new ones will be created with new IDs, except
   * for special object entries that are protected from deletion (objects that
   * are currently hidden in a stack, and objects that are currently grasped).
   * NOTE: before launching this command, make sure that segmentation and
   *       object recognition are stable.
   * @return true/false on success/failure
   */
  virtual bool reset();
  /**
   * DEPRECATED
   * Pauses a specific stacking thread. This will
   * pause the required tracking thread with the _name_
   * provided by the user.
   * @param objName specifies the label of the tracking thread
   * to be paused
   * @return true/false on success/failure
   */
  virtual bool pause(const std::string& objName);
  /**
   * DEPRECATED
   * Resumes a specific stacking thread. This will
   * resume the required tracking thread with the _name_
   * provided by the user.
   * @param objName specifies the label of the tracking thread
   * to be resumed
   * @return true/false on success/failure
   */
  virtual bool resume(const std::string& objName);
  /**
   * DEPRECATED
   * Pauses a specific stacking thread. This will
   * pause the required tracking thread with the _ID_
   * provided by the user.
   * @param objID specifies the numeric identifier of the tracking thread
   * to be paused
   * @return true/false on success/failure
   */
  virtual bool pauseID(const std::int32_t objID);
  /**
   * DEPRECATED
   * Resumes a specific stacking thread. This will
   * resume the required tracking thread with the _ID_
   * provided by the user.
   * @param objID specifies the numeric identifier of the tracking thread
   * to be resumed
   * @return true/false on success/failure
   */
  virtual bool resumeID(const std::int32_t objID);
  /**
   * Quit the module.
   * @return true/false on success/failure
   */
  virtual bool quit();
  bool read(yarp::os::ConnectionReader& connection) override;
  virtual std::vector<std::string> help(const std::string& functionName="--all");
};

#endif
