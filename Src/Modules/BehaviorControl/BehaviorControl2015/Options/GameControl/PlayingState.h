/* 
 * Copyright:Hust_iRobot
 * File:   PlayingState.h
 * Author: wangyidi
 * Description: test version for the following function: 
 * Add logic to make robot return to its original point according to player number 
 * if it cannot see the ball after being free from penalized state 
 * in case that the robot continued rotating outside of the field until it is penalized again
 * Created on Oct 1, 2017, 2:56 PM
 */

#ifndef PLAYINGSTATE_H
#define PLAYINGSTATE_H
/** behavior for the playing state */
option(PlayingState)
{
  initial_state(getReady)
  {
    transition
    {
        if(state_time > 500)
            goto searchForBall;
    }
    action
    {
      LookForward();
      Stand();
    }
  }
  
  state(searchForBall)
  {
      transition
      {
          if(libCodeRelease.timeSinceBallWasSeen() > theBehaviorParameters.ballNotSeenTimeOut && state_time > 5000)
              goto identifyPlayerNumber;
          if(libCodeRelease.timeSinceBallWasSeen() < 300)
              goto playPlayer5;     // if the robot sees the ball immediately, it play as a striker 
      }
      action
      {
          WalkAtSpeedPercentage(Pose2f(1.f, 0.f, 0.f));
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
              goto playKeeper;
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
              goto playPlayer2;
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
              goto playPlayer3;
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
              goto playPlayer4;
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
              goto playPlayer5;
          }
      }
      action
      {
          LookForward();
          WalkToTargetWithPathMode(Pose2f(100.f,100.f,100.f),Pose2f(libCodeRelease.angleToGoal,-2000.f,-1000.f));
      }
  }
  
  state(playKeeper)
  {
      action
      {
          Keeper();
      }
  }
  
  state(playPlayer2)
  {
      action
      {
          Striker();
      }
  }
  
  state(playPlayer3)
  {
      action
      {
          Striker();
      }
  }
  
  state(playPlayer4)
  {
      action
      {
          Striker();
      }
  }
  
  state(playPlayer5)
  {
      action
      {
          Striker();
      }
  }
  
}

#endif /* PLAYINGSTATE_H */
