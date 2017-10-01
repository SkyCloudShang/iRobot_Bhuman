/* 
 * Copyright:Hust_iRobot
 * File:   Keeper.h
 * Author: Shangyunfei
 * Description: A test keeper created by shangyunfei using in SimRobot.Have not test in real Robot.So 
 *  you would be better not to use it in real competition.
 * Created on 2017?10?1?, ??10:31
 */

#ifndef KEEPER_H
#define KEEPER_H
option(Keeper)
{
    common_transition
    {
        if(theBallModel.estimate.position.norm()<1500.f && theBallModel.estimate.velocity.norm()<100.f)
            goto kickBallAway;
    }
    
    initial_state(start)
    {
        transition
        {
             if(state_time > 1000)
                 goto judge;
        }
        action
        {
            LookAtBall();
            Stand();
        }
    }
    
    state(kickBallAway)
    {
        transition
        {
            if(theBallModel.estimate.position.norm() < 500.f)
                goto alignToCenter;
        }
        action
        {
            WalkToTarget(Pose2f(50.f, 50.f, 50.f), theBallModel.estimate.position);
        }
    }
    
    state(alignToCenter)
    {
        transition
        {
          if(std::abs(libCodeRelease.angleToCenter) < 10_deg && std::abs(theBallModel.estimate.position.y()) < 100.f)
            goto alignBehindBall;
        }
        action
        {
             WalkToTarget(Pose2f(100.f, 100.f, 100.f), Pose2f(libCodeRelease.angleToCenter, theBallModel.estimate.position.x() - 400.f, theBallModel.estimate.position.y()));
        }       
    }
    
    state(alignBehindBall)
    {
    transition
    {
      if(libCodeRelease.between(theBallModel.estimate.position.y(), 20.f, 50.f)
          && libCodeRelease.between(theBallModel.estimate.position.x(), 140.f, 170.f)
          && std::abs(libCodeRelease.angleToCenter) < 2_deg)
        goto kick;
    }
    action
    {
      LookForward();
      WalkToTarget(Pose2f(80.f, 80.f, 80.f), Pose2f(libCodeRelease.angleToGoal, theBallModel.estimate.position.x() - 150.f, theBallModel.estimate.position.y() - 30.f));
    }
    }
    
    state(kick)
    {
    transition
    {
      if(state_time > 3000 || (state_time > 10 && action_done))
        goto judge;
    }
    action
    {
      LookForward();
      InWalkKick(WalkKickVariant(WalkKicks::forward, Legs::left), Pose2f(libCodeRelease.angleToGoal, theBallModel.estimate.position.x() - 160.f, theBallModel.estimate.position.y() - 55.f));
    }
  }
    
    
    state(judge)
    {
        transition
        {
            Vector2f ballFieldPosition=Transformation::robotToField(theRobotPose,theBallModel.estimate.position);
            
            if(ballFieldPosition.x()<0.f)
                goto walkToKeeperPoint;
            else 
                goto initPoint;
                
        }
    }
    
    state(walkToKeeperPoint)
    {
        transition
        {
            Vector2f robotFieldPos=Transformation::robotToField(theRobotPose,Vector2f(0.f,0.f));
            Vector2f ballFieldPosition=Transformation::robotToField(theRobotPose,theBallModel.estimate.position);
            
            if(libCodeRelease.between(robotFieldPos.x(),-4005.f,-3995.f) &&
                libCodeRelease.between(robotFieldPos.y(),ballFieldPosition.y()/9.0f-5.f,ballFieldPosition.y()/9.0f+5.f))
            {
                goto turnToBall;
            }
        }
        action
        {
            Vector2f ballFieldPosition=Transformation::robotToField(theRobotPose,theBallModel.estimate.position);
            WalkToTargetWithPathMode(Pose2f(100.f,100.f,100.f),Pose2f(libCodeRelease.angleToGoal,-4000.f,ballFieldPosition.y()/9));
        }
    }
    
    state(turnToBall)
    {
        transition
        {
            if(std::abs(theBallModel.estimate.position.angle()) < 5_deg)
            goto stand;
        }
        action
        {
            LookAtBall();
            WalkToTarget(Pose2f(50.f, 50.f, 50.f), Pose2f(theBallModel.estimate.position.angle(), 0.f, 0.f));
        }
    }
    
    state(stand)
    {
        transition
        {
            if(action_done)
            {
                goto judge;
            }
        }
        action
        {
            LookAtBall();
            Stand();
        }
    }
    
    state(initPoint)
    {
        transition
        {
          Vector2f robotFieldPos=Transformation::robotToField(theRobotPose,Vector2f(0.f,0.f));
          if((libCodeRelease.between(robotFieldPos.x(),-4050.f,-3950.f))
                  &&(libCodeRelease.between(robotFieldPos.y(),-50.f,50.f)))
              goto stand;
        }
        action
        {
          LookAtBall();
          WalkToTargetWithPathMode(Pose2f(100.f,100.f,100.f),Pose2f(libCodeRelease.angleToGoal,-4000.f,0.f));
        }
    }   
}


#endif /* KEEPER_H */

