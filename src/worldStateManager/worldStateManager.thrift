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
  * Steps playback once. 
  * @return true/false on success/failure
  */
  bool step();
  
  /**
  * Quit the module.
  * @return true/false on success/failure
  */
  bool quit();  
}
