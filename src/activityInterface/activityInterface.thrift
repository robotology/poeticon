# Copyright: (C) 2013 iCub Facility - Istituto Italiano di Tecnologia
# Authors: Vadim Tikhanoff
# CopyPolicy: Released under the terms of the GNU GPL v2.0.
#
#activityInterface.thrift

struct Bottle { }
(
yarp.name = "yarp::os::Bottle"
yarp.includefile="yarp/os/Bottle.h"
)
/**
* activityInterface_IDLServer
*
* Interface. 
*/

service activityInterface_IDLServer
{
    /**
    * Asks for the manipulability percentage
    * @param target specifies the name of the target object
    * @return double of best manipulability
    **/
    double getManip(1:string target)
    
    /**
    * Get the status of the left or right hand
    * @param handName specifies the name of the hand in question
    * @return true/false on holding or not
    **/
    bool handStat(1:string handName)
    
    /**
     * Get the label of the object located in the vicinity of xpo and ypos
     * @param xpos specifies the 2D position of the object on the X axis
     * @param ypos specifies the 2D position of the object on the Y axis
     * @return string with the name of the object
     **/
    string getLabel(1:i32 xpos, 2:i32 ypos)
    
    /**
     * Figure out if the requested object is actually located in the hand of the robot
     * @param objName specifies the name of the object
     * @return string with the name of the hand left, right or none if not located in any hand.
     **/
    string inHand(1:string objName)
    
    /**
     * Get the 3D position of the object requested by the user.
     * @param objName specifies the name of the object
     * @return Bottle containing 3D position
     **/
    Bottle get3D(1:string objName)
    
    /**
     * Perform the take action on the particular object with the particular hand
     * @param objName specifies the name of the object in question
     * @param handName specifies the name of the hand in question
     * @return true/false on taking or not
     **/
    bool take(1:string objName, 2:string handName)
    
    /**
     * Perform the drop action on the particular object with the particular hand
     * @param objName specifies the name of the object in question
     * @param handName specifies the name of the hand in question
     * @return true/false on droping or not
     **/
    bool drop(1:string objName, 2:string handName)
    
    /**
     * Quit the module.
     * @return true/false on success/failure
     */
    bool quit();

}
