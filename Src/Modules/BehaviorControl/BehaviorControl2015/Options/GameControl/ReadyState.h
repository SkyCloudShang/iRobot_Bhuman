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
              if(theGameInfo.kickOffTeam==theOwnTeamInfo.teamNumber)
                  goto walkToStrikerPoint;
              else
                  goto walkToStrikerKickPoint;
          }
          else if(theRobotInfo.number==3)
          {
              goto walkToDefenderPoint;
          }
          else if(theRobotInfo.number==4)
          {
              goto walkToBreakingSupporterPoint;
          }
          else if(theRobotInfo.number==5)
          {
              goto walkToSupporterPoint;
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
  
  state(walkToStrikerPoint)
  {
      transition
      {
          if(libCodeRelease.between(theRobotPose.translation.x(),-755.f,-745.f) && 
                  libCodeRelease.between(theRobotPose.translation.y(),-5.f,5.f) )
          {
              goto end;
          }
      }
      action
      {
          LookForward();
          WalkToTargetWithPathMode(Pose2f(100.f,100.f,100.f),Pose2f(libCodeRelease.angleToCenter,-750.f,0.f));
      }
  }
  
  state(walkToStrikerKickPoint)
  {
      transition
      {
          if(libCodeRelease.between(theRobotPose.translation.x(),-5.f,5.f) && 
                  libCodeRelease.between(theRobotPose.translation.y(),-5.f,5.f) )
          {
              goto end;
          }
      }
      action
      {
          LookForward();
          WalkToTargetWithPathMode(Pose2f(100.f,100.f,100.f),Pose2f(libCodeRelease.angleToCenter,0.f,0.f));
      }
  }
  
  state(walkToDefenderPoint)
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
  
  state(walkToBreakingSupporterPoint)
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
  
  state(walkToSupporterPoint)
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

