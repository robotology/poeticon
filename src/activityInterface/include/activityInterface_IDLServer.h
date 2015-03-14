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
   * Get the 2D position of the object requested by the user.
   * @param objName specifies the name of the object
   * @return Bottle containing 2D position
   */
  virtual yarp::os::Bottle get2D(const std::string& objName);
  /**
   * Get the tool offset position of the object requested by the user.
   * @param objName specifies the name of the object (typically tool)
   * @return Bottle containing 3D offset
   */
  virtual yarp::os::Bottle getOffset(const std::string& objName);
  /**
   * Perform the take action on the particular object with the particular hand
   * @param objName specifies the name of the object in question
   * @param handName specifies the name of the hand in question
   * @return true/false on taking or not
   */
  virtual bool take(const std::string& objName, const std::string& handName);
  /**
   * Perform the drops action on the particular object with the particular hand
   * @param objName specifies the name of the object in question
   * @return true/false on droping or not
   */
  virtual bool drop(const std::string& objName);
  /**
   * Perform the put action on the particular object with the particular hand
   * @param objName specifies the name of the object in question
   * @param targetName specifies the name of target object to drop onto.
   * @return true/false on putting or not
   */
  virtual bool put(const std::string& objName, const std::string& targetName);
  /**
   * Perform the push action on the particular object with the particular tool
   * @param objName specifies the name of the object in question
   * @param toolName specifies the name of target tool.
   * @return true/false on pushing or not
   */
  virtual bool push(const std::string& objName, const std::string& toolName);
  /**
   * Perform the pull action on the particular object with the particular tool
   * @param objName specifies the name of the object in question
   * @param toolName specifies the name of target tool.
   * @return true/false on pulling or not
   */
  virtual bool pull(const std::string& objName, const std::string& toolName);
  /**
   * Perform the take action on the particular tool with the particular hand
   * @param handName specifies the name of the hand to use
   * @param xpos specifies the 2D position of the object on the X axis
   * @param ypos specifies the 2D position of the object on the Y axis
   * @return true/false on taking or not
   */
  virtual bool askForTool(const std::string& handName, const int32_t xpos, const int32_t ypos);
  /**
   * Ask for the list of labels that are under ojbName
   * @param objName specifies the name of the object in question
   * @return Bottle containing list of labels that are under objName
   */
  virtual yarp::os::Bottle underOf(const std::string& objName);
  /**
   * Ask with what the object in question is reacheable with
   * @param objName specifies the name of the object in question
   * @return Bottle containing list of labels that can be used to reach the objName
   */
  virtual yarp::os::Bottle reachableWith(const std::string& objName);
  /**
   * Ask which objects can be used to pull object in question
   * @param objName specifies the name of the object in question
   * @return Bottle containing list of labels that can be used to reach the objName
   */
  virtual yarp::os::Bottle pullableWith(const std::string& objName);
  /**
   * Get objects list that are currently recognized.
   * @return Bottle containing list of labels that are currently recognized
   */
  virtual yarp::os::Bottle getNames();
  /**
   * Get all known object that are available in the opc.
   * @return Bottle containing list of all known object
   */
  virtual yarp::os::Bottle getOPCNames();
  /**
   * Get the speech instruction and sends it to the praxicon
   * @param request specifies the request to be asked to the praxicon
   * @return Bottle containing list of goals to achieve
   */
  virtual yarp::os::Bottle askPraxicon(const std::string& request);
  /**
   * Return to home position
   * @return true/false on homeing or not
   */
  virtual bool goHome();
  /**
   * Ask to pause all trackers
   * @return true/false on pausing or not
   */
  virtual bool pauseAllTrackers();
  /**
   * Ask to resume position
   * @return true/false on homeing or not
   */
  virtual bool resumeAllTrackers();
  /**
   * Quit the module.
   * @return true/false on success/failure
   */
  virtual bool quit();
  virtual bool read(yarp::os::ConnectionReader& connection);
  virtual std::vector<std::string> help(const std::string& functionName="--all");
};

#endif

