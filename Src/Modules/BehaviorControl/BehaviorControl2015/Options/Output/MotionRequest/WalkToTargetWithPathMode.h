/* 
 * Copyright:Hust_iRobot
 * File:   WalkToTargetWithPathMode.h
 * Author: Shangyunfei
 * Description:
 * Created on 2017?10?1?, ??9:44
 */

#ifndef WALKTOTARGETWITHPATHMODE_H
#define WALKTOTARGETWITHPATHMODE_H

option(WalkToTargetWithPathMode,(const Pose2f&) speed, (const Pose2f&) target)
{
  initial_state(setRequest)
  {
    transition
    {
      if(theMotionInfo.motion == MotionRequest::walk)
        goto requestIsExecuted;
    }
    action
    {
      theMotionRequest.motion = MotionRequest::walk;
      theMotionRequest.walkRequest.mode = WalkRequest::pathMode;
      theMotionRequest.walkRequest.target = target;
      theMotionRequest.walkRequest.speed = speed;
      theMotionRequest.walkRequest.walkKickRequest = WalkRequest::WalkKickRequest();
    }
  }

  /** The motion process has started executing the request. */
  target_state(requestIsExecuted)
  {
    transition
    {
      if(theMotionInfo.motion != MotionRequest::walk)
        goto setRequest;
    }
    action
    {
      theMotionRequest.motion = MotionRequest::walk;
      theMotionRequest.walkRequest.mode = WalkRequest::pathMode;
      theMotionRequest.walkRequest.target = target;
      theMotionRequest.walkRequest.speed = speed;
      theMotionRequest.walkRequest.walkKickRequest = WalkRequest::WalkKickRequest();
    }
  }
}


#endif /* WALKTOTARGETWITHPATHMODE_H */

