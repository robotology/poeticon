#planningCycle.thrift

service planningCycle_IDLserver
{
    bool quit();
    bool startPlanning();
    bool pausePlanner();
    bool resumePlanner();
    bool goBack();
    bool goForward();
    bool updateState();
    bool updateGoals();
    bool plan();
    bool resetRules();
    bool resetConfig();
    bool resetLevel();
    bool ground();
    bool compileGoal();
    bool increaseHorizon();
    bool executePlannedAction();
    string checkGoalCompleted();
    bool run1Step();
    string showPlannedAction();
    string showCurrentState();
    string showCurrentGoal();
}
