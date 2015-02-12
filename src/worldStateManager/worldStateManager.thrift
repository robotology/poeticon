# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# Copyright: (C) 2014 VisLab, Institute for Systems and Robotics,
#                Istituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
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
  * Quit the module.
  * @return true/false on success/failure
  */
  bool quit();  
}
