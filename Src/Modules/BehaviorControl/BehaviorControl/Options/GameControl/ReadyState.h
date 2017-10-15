/* 
 * Copyright:Hust_iRobot
 * File:   ReadyState.h
 * Author: Shangyunfei
 * Description:
 * Created on October 15, 2017, 10:58 AM
 */

#ifndef READYSTATE_H
#define READYSTATE_H

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
          if((theLibCodeRelease.between(theRobotPose.translation.x(),-4550.f,-4450.f))
                  &&(theLibCodeRelease.between(theRobotPose.translation.y(),-50.f,50.f)))
          {
              goto end;
          }
      }
      action
      {
          LookForward();
          WalkPathMode(Pose2f(theLibCodeRelease.angleToGoal,-4500.f,0.f));
      }
  }
  
  state(walkToStrikerPoint)
  {
      transition
      {
          if(theLibCodeRelease.between(theRobotPose.translation.x(),-755.f,-745.f) && 
                  theLibCodeRelease.between(theRobotPose.translation.y(),-5.f,5.f) )
          {
              goto end;
          }
      }
      action
      {
          LookForward();
          WalkPathMode(Pose2f(theLibCodeRelease.angleToCenter,-750.f,0.f));
      }
  }
  
  state(walkToStrikerKickPoint)
  {
      transition
      {
          if(theLibCodeRelease.between(theRobotPose.translation.x(),-5.f,5.f) && 
                  theLibCodeRelease.between(theRobotPose.translation.y(),-5.f,5.f) )
          {
              goto end;
          }
      }
      action
      {
          LookForward();
          WalkPathMode(Pose2f(theLibCodeRelease.angleToCenter,0.f,0.f));
      }
  }
  
  state(walkToDefenderPoint)
  {
      transition
      {
          if((theLibCodeRelease.between(theRobotPose.translation.x(),-3050.f,-2950.f))
                  &&(theLibCodeRelease.between(theRobotPose.translation.y(),-1050.f,-950.f)))
          {
              goto end;
          }
      }
      action
      {
          LookForward();
          WalkPathMode(Pose2f(theLibCodeRelease.angleToGoal,-3000.f,-1000.f));
      }
  }
  
  state(walkToBreakingSupporterPoint)
  {
      transition
      {
          if((theLibCodeRelease.between(theRobotPose.translation.x(),-3050.f,-2950.f))
                  &&(theLibCodeRelease.between(theRobotPose.translation.y(),950.f,1050.f)))
          {
              goto end;
          }
      }
      action
      {
         LookForward();
          WalkPathMode(Pose2f(theLibCodeRelease.angleToGoal,-3000.f,1000.f));
      }
  }
  
  state(walkToSupporterPoint)
  {
      transition
      {
          if((theLibCodeRelease.between(theRobotPose.translation.x(),-2050.f,-1950.f))
                  &&(theLibCodeRelease.between(theRobotPose.translation.y(),-1050.f,-950.f)))
          {
              goto end;
          }
      }
      action
      {
          LookForward();
          WalkPathMode(Pose2f(theLibCodeRelease.angleToGoal,-2000.f,-1000.f));
      }
  }
  
  
  state(end)
  {
      transition
      {
          if(std::abs(theLibCodeRelease.angleToCenter)<5_deg)
              goto endstand;
      }
      action
      {
          WalkToTarget(Pose2f(100.f,100.f,100.f),Pose2f(theLibCodeRelease.angleToCenter,0.f,0.f));
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

