# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# Copyright: (C) 2016 VisLab, Institute for Systems and Robotics,
#                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
# Author: Giovanni Saponaro <gsaponaro@isr.ist.utl.pt>
# CopyPolicy: Released under the terms of the GNU GPL v2.0
#
# dummyWorldStateManager.thrift

service DummyWorldStateMgr_IDL
{
  /**
  * Update the world state database.
  * The new state will be created from the next time instant in the
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
