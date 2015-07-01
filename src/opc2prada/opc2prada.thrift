# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# Copyright: (C) 2015 VisLab, Institute for Systems and Robotics,
#                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
# Author: Pedro Vicente <pvicente@isr.ist.utl.pt>
# CopyPolicy: Released under the terms of the GNU GPL v2.0
#
# opc2prada.thrift

struct Bottle { }
(
yarp.name = "yarp::os::Bottle"
yarp.includefile="yarp/os/Bottle.h"
)

service OPC2PRADA_IDL
{

  /**
  * Update the state text file.
  * 
  * @return true/false on success/failure
  */
  bool update();
  
  /**
  * Load correspondence between ID and Labels in the DataBase
  * 
  * @return Bottle with ID and Objects names
  */
  Bottle loadObjects();

  /**
  * Bottle
  * 
  * @return Bottle with 2d features
  */
  Bottle query2d(1:i32 id);

  /**
  * Bottle
  * 
  * @return Bottle with 2d features of the tool
  */
  Bottle querytool2d(1:i32 id);

  /**
  * Quit the module.
  * @return true/false on success/failure
  */
  bool quit();  
}
