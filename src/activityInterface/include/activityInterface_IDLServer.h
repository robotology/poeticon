// This is an automatically-generated file.
// It could get re-generated if the ALLOW_IDL_GENERATION flag is on.

#ifndef YARP_THRIFT_GENERATOR_activityInterface_IDLServer
#define YARP_THRIFT_GENERATOR_activityInterface_IDLServer

#include <yarp/os/Wire.h>
#include <yarp/os/idl/WireTypes.h>
#include <yarp/os/Bottle.h>

class activityInterface_IDLServer;


/**
 * activityInterface_IDLServer
 * Interface.
 */
class activityInterface_IDLServer : public yarp::os::Wire {
public:
  activityInterface_IDLServer();
  /**
   * Asks for the manipulability percentage
   * @param objName specifies the name of the target object
   * @param handName specifies the name of hand to query
   * @return double of best manipulability
   */
  virtual double getManip(const std::string& objName, const std::string& handName);
  /**
   * Get the status of the left or right hand
   * @param handName specifies the name of the hand in question
   * @return true/false on holding or not
   */
  virtual bool handStat(const std::string& handName);
  /**
   * Get the label of the object located in the vicinity of xpo and ypos
   * @param xpos specifies the 2D position of the object on the X axis
   * @param ypos specifies the 2D position of the object on the Y axis
   * @return string with the name of the object
   */
  virtual std::string getLabel(const int32_t xpos, const int32_t ypos);
  /**
   * Figure out if the requested object is actually located in the hand of the robot
   * @param objName specifies the name of the object
   * @return string with the name of the hand left, right or none if not located in any hand.
   */
  virtual std::string inHand(const std::string& objName);
  /**
   * Get the 3D position of the object requested by the user.
   * @param objName specifies the name of the object
   * @return Bottle containing 3D position
   */
  virtual yarp::os::Bottle get3D(const std::string& objName);
  /**
   * Get the tool offset position of the object requested by the user.
   * @param objName specifies the name of the object (typically tool)
   * @return Bottle containing 3D offset
   */
  virtual yarp::os::Bottle getTooltipOffset(const std::string& objName);
  /**
   * Perform the take action on the particular object with the particular hand
   * @param objName specifies the name of the object in question
   * @param handName specifies the name of the hand in question
   * @return true/false on taking or not
   */
  virtual bool take(const std::string& objName, const std::string& handName);
  /**
   * Perform the drop action on the particular object with the particular hand
   * @param objName specifies the name of the object in question
   * @param targetName specifies the name of target object to drop onto.
   * @return true/false on droping or not
   */
  virtual bool drop(const std::string& objName, const std::string& targetName);
  /**
   * Ask for the list of labels that are under ojbName
   * @param objName specifies the name of the object in question
   * @return Bottle containing list of labels that are under objName
   */
  virtual yarp::os::Bottle underOf(const std::string& objName);
  /**
   * Quit the module.
   * @return true/false on success/failure
   */
  virtual bool quit();
  virtual bool read(yarp::os::ConnectionReader& connection);
  virtual std::vector<std::string> help(const std::string& functionName="--all");
};

#endif

