# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# Copyright: (C) 2016 VisLab, Institute for Systems and Robotics,
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
  * Initialize the world state database.
  * NOTE: before launching this command, make sure that segmentation and
  *       object recognition are stable.
  * @return true/false on success/failure
  */
  bool init();

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
  * The new state will be created from robot perception.
  * @return true/false on success/failure
  */
  bool update();

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
