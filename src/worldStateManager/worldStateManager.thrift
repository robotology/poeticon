# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
#                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
# Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
# CopyPolicy: Released under the terms of the GNU GPL v2.0
#
# worldStateManager.thrift

service WorldStateMgr_IDL
{
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
   * Pauses a specific stacking thread. This will
   * pause the required tracking thread with the name
   * provided by the user.
   * @param objName specifies the label of the tracking thread
   * to be paused
   * @return true/false on success/failure
   */
  bool pause(1:string objName);
  
  /**
   * Resumes a specific stacking thread. This will
   * resume the required tracking thread with the name
   * provided by the user.
   * @param objName specifies the label of the tracking thread
   * to be resumed
   * @return true/false on success/failure
   */
  bool resume(1:string objName);

  /**
  * Quit the module.
  * @return true/false on success/failure
  */
  bool quit();  
}
