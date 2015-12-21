# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
#                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
# Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
# CopyPolicy: Released under the terms of the GNU GPL v2.0
#
# worldStateManager.thrift

struct Bottle { }
(
yarp.name = "yarp::os::Bottle"
yarp.includefile="yarp/os/Bottle.h"
)

service WorldStateMgr_IDL
{
  /**
  * Check if initialization phase has been completed. This is accomplished when
  * tracker, short-term memory model and WSOPC database possess all entries.
  * @return true/false on initialized/uninitialized
  */
  bool isInitialized();

  /**
  * Print information the current world state on the screen.
  * @return true/false on success/failure
  */
  bool dump();

  /**
  * Update the world state database.
  * If the module was started in perception mode (default), the new state will
  * be created from robot perception. If the module was started in playback
  * mode, the new state will be created from the next time instant in the
  * world state text file.
  * @return true/false on success/failure
  */
  bool update();

  /**
  * Reset the world state database, initializing it from scratch.
  * NOTE: you still have to manually restart this module:
  * objectsPropertiesCollector --name wsopc --context poeticon --db dbhands.ini --nosave --async_bc
  * @return true/false on success/failure
  */
  bool reset();

  /**
   * Pauses a specific stacking thread. This will
   * pause the required tracking thread with the _name_
   * provided by the user.
   * @param objName specifies the label of the tracking thread
   * to be paused
   * @return true/false on success/failure
   */
  bool pause(1:string objName);

  /**
   * Resumes a specific stacking thread. This will
   * resume the required tracking thread with the _name_
   * provided by the user.
   * @param objName specifies the label of the tracking thread
   * to be resumed
   * @return true/false on success/failure
   */
  bool resume(1:string objName);

  /**
   * Pauses a specific stacking thread. This will
   * pause the required tracking thread with the _ID_
   * provided by the user.
   * @param objID specifies the numeric identifier of the tracking thread
   * to be paused
   * @return true/false on success/failure
   */
  bool pauseID(1:i32 objID);

  /**
   * Resumes a specific stacking thread. This will
   * resume the required tracking thread with the _ID_
   * provided by the user.
   * @param objID specifies the numeric identifier of the tracking thread
   * to be resumed
   * @return true/false on success/failure
   */
  bool resumeID(1:i32 objID);

  /**
   * Get the color histogram of the object requested by the user.
   * @param u specifies the u coordinate of the object
   * @param v specifies the v coordinate of the object
   * @return Bottle containing color histogram
   **/
  Bottle getColorHist(1:i32 u, 2:i32 v);

  /**
  * Quit the module.
  * @return true/false on success/failure
  */
  bool quit();  
}
