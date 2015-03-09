#activeParticleTrack.thrift

/**
* activeParticleTrack_IDLServer
*
* Interface. 
*/

struct Bottle { }
(
yarp.name = "yarp::os::Bottle"
yarp.includefile="yarp/os/Bottle.h"
)

service activeParticleTrack_IDLServer
{
  /**
  * Displays all the traking particles.
  * @param value specifies the boolean value for 
  * displaying the particle or not. Use on/off.
  * @return true/false on success/failure
  */
  bool display(1:string value);

  /**
  * Gets the list of ID being tracked
  * @return true/false on success/failure
  */
  Bottle getIDs();
  
  /**
   * Gets the list of ID actually being paused
   * @return true/false on success/failure
   */
  Bottle getPausedIDs();

  /**
  * Track a fixation point. This initializes the
  * segmentation of the area of interest for subsequent
  * tracking.
  * @param fix_x specifies the x coordinate of the 
  * required fixation point for segmentation
  * @param fix_x specifies the y coordinate of the
  * required fixation point for segmentation
  * @return i32 of thread index
  */
  i32 track(1:double fix_x, 2:double fix_y);
  
  /**
  * Deletes a specific stacking thread. This will
  * delete the required tracking thread with the id
  * provided by the user.
  * @param id specifies the id of the tracking thread 
  * to be deleted
  * @return true/false on success/failure
  */
  bool untrack(1:i32 id);
  
  /**
   * Pauses a specific stacking thread. This will
   * pause the required tracking thread with the id
   * provided by the user.
   * @param id specifies the id of the tracking thread
   * to be paused
   * @return true/false on success/failure
   */
  bool pause(1:i32 id);
  
  /**
   * Resumes a specific stacking thread. This will
   * resume the required tracking thread with the id
   * provided by the user.
   * @param id specifies the id of the tracking thread
   * to be resumed
   * @return true/false on success/failure
   */
  bool resume(1:i32 id);
  
  /**
  * Sets the tracker to the user desired index. 
  * Usesul for synchronisation issues.
  * provided by the user.
  * @param id specifies the index to start incrementing from
  * @return true/false on success/failure
  */
  bool countFrom(1:i32 id);
  
  /**
  * Reset all the trakers. This is used in case the user
  * would like to reset the whole tracking process.
  * @return true/false on success/failure
  */
  bool reset();

  /**
  * Quit the module.
  * @return true/false on success/failure
  */
  bool quit();  
}
