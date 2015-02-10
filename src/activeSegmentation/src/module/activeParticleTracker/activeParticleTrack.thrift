#activeParticleTrack.thrift

/**
* activeParticleTrack_IDLServer
*
* Interface. 
*/

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
  * Track a fixation point. This initializes the 
  * segmentation of the area of interest for subsequent
  * tracking.
  * @param fix_x specifies the x coordinate of the 
  * required fixation point for segmentation
  * @param fix_x specifies the y coordinate of the
  * required fixation point for segmentation
  * @return i32 of thread index
  */
  i32 track(1:i32 fix_x, 2:i32 fix_y);
  
  /**
  * Stops a specific stacking thread. This will 
  * delete the required tracking thread with the id 
  * provided by the user.
  * @param id specifies the id of the tracking thread 
  * to be stopped
  * @return true/false on success/failure
  */
  bool untrack(1:i32 id);
  
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
