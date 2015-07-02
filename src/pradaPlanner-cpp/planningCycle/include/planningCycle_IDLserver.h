// This is an automatically-generated file.
// It could get re-generated if the ALLOW_IDL_GENERATION flag is on.

#ifndef YARP_THRIFT_GENERATOR_planningCycle_IDLserver
#define YARP_THRIFT_GENERATOR_planningCycle_IDLserver

#include <yarp/os/Wire.h>
#include <yarp/os/idl/WireTypes.h>
#include <yarp/os/Bottle.h>

class planningCycle_IDLserver;


class planningCycle_IDLserver : public yarp::os::Wire {
public:
  planningCycle_IDLserver();
  /**
   * Quit the module.
   * Abruptly terminates the module and the rpc port
   */
  virtual bool quit();
  /**
   * Start fully-automated planner.
   * The planner may be paused, unpaused or stopped.
   * It will update the state, ground the rules, compile the goals, and plan the necessary steps to achieve the goals.
   * @returns ok upon receiving/understanding the command
   */
  virtual bool startPlanner();
  /**
   * Stop fully-automated planner.
   * Rules and symbols will have to be grounded again, goals will have to be recompiled.
   * @returns ok upon receiving/understanding the command
   */
  virtual bool stopPlanner();
  /**
   * Pauses the fully-automated planner.
   * It will execute the last instruction started before the pause command.
   * Planning variables may be changed while paused.
   * @returns ok upon receiving/understanding the command
   */
  virtual bool pausePlanner();
  /**
   * Resumes the fully-automated planner.
   * Resumes the planner. It will start from where it last stopped.
   * @returns ok upon receiving/understanding the command
   */
  virtual bool resumePlanner();
  /**
   * Goes back one step in the subgoal list.
   * @returns ok upon receiving/understanding the command
   */
  virtual bool goBack();
  /**
   * Goes forward one step in the subgoal list.
   * @returns ok upon receiving/understanding the command
   */
  virtual bool goForward();
  /**
   * Updates the planner world state.
   * Sends an update command to both the world state manager and opc2prada
   * @returns ok/fail if successful/not.
   */
  virtual bool updateState();
  /**
   * Updates and loads the object IDs and labels into the planner.
   * Sends an update command to the opc2prada
   * @returns ok/fail if successful/not.
   */
  virtual bool loadObjects();
  /**
   * Prints the last loaded objects on the planner.
   * Does NOT send an update command to the opc2prada
   * @returns bottle of bottles with (IDs, labels), or fail.
   */
  virtual yarp::os::Bottle printObjects();
  /**
   * Loads the world state to the planner.
   * Completes the state file with missing symbols (negated), and loads it into the planner.
   * @returns ok/fail if successful/not.
   */
  virtual bool loadState();
  /**
   * Prints world state on the terminal.
   * Prints the list of symbols on the current world state on the terminal.
   * Make sure to loadState if the state changed.
   * @returns bottle with the world state symbol-list, or fail if not successful.
   */
  virtual std::string showCurrentState();
  /**
   * Compiles subgoal list from Praxicon instructions
   * Waits for a praxicon instruction bottle and translates it into robot symbols.
   * The compiler will timeout if no bottle is received in 5 minutes
   * @returns ok/fail upon successful compiling/not.
   */
  virtual bool compileGoal();
  /**
   * Loads the current subgoal.
   * Loads the current subgoal according to the planning step.
   * @returns ok/fail upon success/not.
   */
  virtual bool updateGoals();
  /**
   * Prints current planner goal to the terminal.
   * Prints the list of symbols of the current planner goal to the terminal.
   * Make sure you have updated the goals if the plan level changed.
   * @returns bottle with the planner goal symbol-list, or fail if not.
   */
  virtual std::string showCurrentGoal();
  /**
   * Grounds rules.
   * Creates a list of rules grounded for the available objects, with affordances.
   * Requires an updated world state to get the object list.
   * @returns ok/fail upon success/not.
   */
  virtual bool ground();
  /**
   * Plans the next action.
   * Runs PRADA to find the next best action to achieve the current subgoal.
   * Make sure all rules have been grounded, the state updated, the goals compiled and updated, and that it is in the correct plan level.
   * @returns ok/fail if an action is found/not.
   */
  virtual bool plan();
  /**
   * Runs a full loop of the planning cycle.
   * Updates the world state, checks the subgoals, plans the next action, codes and executes it.
   * Make sure the rules have been grounded, the goals compiled and updated, and that it is in the correct plan level.
   * @returns ok/fail upon completion/failure.
   */
  virtual bool run1Step();
  /**
   * Prints the planned action on the terminal.
   * Translates the planned action to object labels and prints it on the terminal.
   * Only the last planned action will be printed.
   * @returns last planned action. If no action has been planned, returns an empty string.
   */
  virtual std::string showPlannedAction();
  /**
   * Executes the last planned action.
   * Translates the planned action to objects labels and sends the instruction to ActivityInterface for execution.
   * @returns ok/fail upon success/failure of the action.
   */
  virtual bool executePlannedAction();
  /**
   * Checks if the current subgoal has been achieved.
   * Compares the current subgoal with the world state to verify if it has been achieved.
   * Make sure the correct plan level is set, and that the goals are updated.
   * @returns goal achieved/not achieved.
   */
  virtual std::string checkGoalCompleted();
  /**
   * Resets rules.
   * Returns the rule list to the point of the last grounding.
   * @returns ok upon receiving/understanding the command.
   */
  virtual bool resetRules();
  /**
   * Resets config file.
   * Returns the config file for PRADA to the default values.
   * Sets the planning horizon back to 5.
   * @returns ok/fail upon writing the file/failing.
   */
  virtual bool resetConfig();
  /**
   * Resets plan level.
   * Sets the plan level back to 0.
   * @returns ok upon receiving/understanding the command.
   */
  virtual bool resetLevel();
  /**
   * Increases planning horizon.
   * Increases the planning horizon by 1. The config file will be written over.
   * Horizon won't exceed 15.
   * @returns ok/fail upon writing the file/failure
   */
  virtual bool increaseHorizon();
  virtual bool read(yarp::os::ConnectionReader& connection);
  virtual std::vector<std::string> help(const std::string& functionName="--all");
};

#endif

