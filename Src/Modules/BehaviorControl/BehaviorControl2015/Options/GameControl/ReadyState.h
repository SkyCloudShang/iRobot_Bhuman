/* 
 * Copyright:Hust_iRobot
 * File:   ReadyState.h
 * Author: Shangyunfei
 * Description: Make robot go to different point decides on its player number.
 * Created on 2017?10?1?, ??9:11
 */

#ifndef READYSTATE_H
#define READYSTATE_H
/** behavior for the ready state */
option(ReadyState)
{
  /* position has been reached -> stand and wait */
  initial_state(stand)
  {
    transition
    {
        if(action_done)
        {
            goto identifyPlayerNumber;
        }
    }
    action
    {
      LookForward();
      Stand();
    }
  }
  
  state(identifyPlayerNumber)
  {
      transition
      {
          if(theRobotInfo.number==1)
          {
              goto walkToKeeperPoint;
          }
          else if(theRobotInfo.number==2)
          {
              goto walkToPlayer2Point;
          }
          else if(theRobotInfo.number==3)
          {
              goto walkToPlayer3Point;
          }
          else if(theRobotInfo.number==4)
          {
              goto walkToPlayer4Point;
          }
          else if(theRobotInfo.number==5)
          {
              goto walkToPlayer5Point;
          }
      }
  }
  
  
  state(walkToKeeperPoint)
  {
      transition
      {
          if((libCodeRelease.between(theRobotPose.translation.x(),-4050.f,-3950.f))
                  &&(libCodeRelease.between(theRobotPose.translation.y(),-50.f,50.f)))
          {
              goto end;
          }
      }
      action
      {
          LookForward();
          WalkToTargetWithPathMode(Pose2f(100.f,100.f,100.f),Pose2f(libCodeRelease.angleToGoal,-4000.f,0.f));
      }
  }
  
  state(walkToPlayer2Point)
  {
      transition
      {
          if(libCodeRelease.between(theRobotPose.translation.x(),-2050.f,-1950.f) && 
                  libCodeRelease.between(theRobotPose.translation.y(),950.f,1050.f) )
          {
              goto end;
          }
      }
      action
      {
          LookForward();
          WalkToTargetWithPathMode(Pose2f(100.f,100.f,100.f),Pose2f(libCodeRelease.angleToCenter,-2000.f,1000.f));
      }
  }
  
  state(walkToPlayer3Point)
  {
      transition
      {
          if((libCodeRelease.between(theRobotPose.translation.x(),-3050.f,-2950.f))
                  &&(libCodeRelease.between(theRobotPose.translation.y(),-1050.f,-950.f)))
          {
              goto end;
          }
      }
      action
      {
          LookForward();
          WalkToTargetWithPathMode(Pose2f(100.f,100.f,100.f),Pose2f(libCodeRelease.angleToGoal,-3000.f,-1000.f));
      }
  }
  
  state(walkToPlayer4Point)
  {
      transition
      {
          if((libCodeRelease.between(theRobotPose.translation.x(),-3050.f,-2950.f))
                  &&(libCodeRelease.between(theRobotPose.translation.y(),950.f,1050.f)))
          {
              goto end;
          }
      }
      action
      {
         LookForward();
          WalkToTargetWithPathMode(Pose2f(100.f,100.f,100.f),Pose2f(libCodeRelease.angleToGoal,-3000.f,1000.f));
      }
  }
  
  state(walkToPlayer5Point)
  {
      transition
      {
          if((libCodeRelease.between(theRobotPose.translation.x(),-2050.f,-1950.f))
                  &&(libCodeRelease.between(theRobotPose.translation.y(),-1050.f,-950.f)))
          {
              goto end;
          }
      }
      action
      {
          LookForward();
          WalkToTargetWithPathMode(Pose2f(100.f,100.f,100.f),Pose2f(libCodeRelease.angleToGoal,-2000.f,-1000.f));
      }
  }
  
  
  state(end)
  {
      transition
      {
          if(std::abs(libCodeRelease.angleToCenter)<5_deg)
              goto endstand;
      }
      action
      {
          WalkToTarget(Pose2f(100.f,100.f,100.f),Pose2f(libCodeRelease.angleToCenter,0.f,0.f));
      }
  }
  
  state(endstand)
  {
      action
      {
          Stand();
      }
  }
  
}


#endif /* READYSTATE_H */

