# Copyright: (C) 2012-2015 POETICON++, European Commission FP7 project ICT-288382
# Copyright: (C) 2018 VisLab, Institute for Systems and Robotics,
#                Instituto Superior Técnico, Universidade de Lisboa, Lisbon, Portugal
# Author: Giovanni Saponaro <gsaponaro@isr.tecnico.ulisboa.pt>
# CopyPolicy: Released under the terms of the GNU GPL v2.0
#
# dummyActivityInterface.thrift

struct Bottle { }
(
yarp.name = "yarp::os::Bottle"
yarp.includefile="yarp/os/Bottle.h"
)

service DummyActivityInterface_IDL
{
  /**
  * Point at a tool and asks for it with speech synthesis
  * @param handName specifies the name of the hand to use
  * @param xpos specifies the 2D position of the target on the X axis
  * @param ypos specifies the 2D position of the target on the Y axis
  * @return true/false on success or not
  **/
  bool askForTool(1:string handName, 2:i32 xpos, 3:i32 ypos);

  /**
   * Get the speech instruction and send it to the PRAXICON
   * @param request specifies the request to be asked to the PRAXICON
   * @return Bottle containing list of goals to achieve
   **/
  Bottle askPraxicon(1:string request);

  /**
  * Perform the drops action on the particular object with the particular hand
  * @param objName specifies the name of the object in question
  * @return true/false on dropping or not
  **/
  bool drop(1:string objName);

  /**
  * Print information about the current progress on screen.
  * @return true/false on success/failure
  */
  bool dump();

  /**
  * Get the 2D position of the object requested by the user.
  * @param objName specifies the name of the object
  * @return Bottle containing 2D position
  **/
  Bottle get2D(1:string objName);

  /**
  * Get the 3D position of the object requested by the user.
  * @param objName specifies the name of the object
  * @return Bottle containing 3D position
  **/
  Bottle get3D(1:string objName);

  /**
  *Get the label of the object located in the vicinity of xpos and ypos
  * @param xpos specifies the 2D position of the object on the X axis
  * @param ypos specifies the 2D position of the object on the Y axis
  * @return string with the name of the object
  **/
  string getLabel(1:i32 xpos, 2:i32 ypos);

  /**
  * Get objects list that are currently recognized.
  * @return Bottle containing list of labels that are currently recognized
  **/
  Bottle getNames();

  /**
  * Return to home position
  * @return true/false on homeing or not
  **/
  bool goHome();

  /**
  * Get the status of the left or right hand
  * @param handName specifies the name of the hand in question
  * @return true/false on holding or not
  **/
  bool handStat(1:string handName);

  /**
  * Figure out if the requested object is actually located in one of the robot's hand
  * @param objName specifies the name of the object
  * @return string with the name of the hand left, right or none if not located in any hand.
  **/
  string inHand(1:string objName);

  /**
  * Remove the top object from the object stack.
  * @return true/false on success or not
  **/
  bool pop();

  /**
  * Perform the pull action on the particular object with the particular tool
  * @param objName specifies the name of the object in question
  * @param toolName specifies the name of target tool.
  * @return true/false on pulling or not
  **/
  bool pull(1:string objName, 2:string toolName);

  /**
  * Ask which objects can be used to pull the object in question
  * @param objName specifies the name of the object in question
  * @return Bottle containing list of labels that can be used to reach the objName
  **/
  Bottle pullableWith(1:string objName);

  /**
  * Perform the push action on the particular object with the particular tool
  * @param objName specifies the name of the object in question
  * @param toolName specifies the name of target tool.
  * @return true/false on pushing or not
  **/
  bool push(1:string objName, 2:string toolName);

  /**
  * Perform the put action on the particular object with the particular hand
  * @param objName specifies the name of the object in question
  * @param targetName specifies the name of target object to drop onto.
  * @return true/false on putting or not
  **/
  bool put(1:string objName, 2:string targetName);

  /**
  * Ask with what the object in question is reachable with
  * @param objName specifies the name of the object in question
  * @return Bottle containing list of labels that can be used to reach the objName
  **/
  Bottle reachableWith(1:string objName);

  /**
   * Run simulated experiment once
   * @return string containing the result of the experiment
   **/
  string simulate();

  /**
  * Perform the take action on the particular object with the particular hand
  * @param objName specifies the name of the object in question
  * @param handName specifies the name of the hand in question
  * @return true/false on taking or not
  **/
  bool take(1:string objName, 2:string handName);

  /**
  * Ask for the list of labels that are under ojbName
  * @param objName specifies the name of the object in question
  * @return Bottle containing list of labels that are under objName
  **/
  Bottle underOf(1:string objName);

  /**
  * Quit the module.
  * @return true/false on success/failure
  */
  bool quit();
}
