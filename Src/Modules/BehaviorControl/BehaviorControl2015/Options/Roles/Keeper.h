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
        if((robotToField(theRobotPose,theBallModel.estimate.position)).x()<1000.f)
        {
            if(theBallModel.estimate.velocity.x()<-200.f && theBallModel.estimate.position.x()<2000.f)
            {
                if(BallPhysics::getEndPosition(theBallModel.estimate.position,theBallModel.estimate.velocity,-0.6f).y()>0.f)
                {
                   // SpecialAction(SpecialActionRequest::keeperBlockLeft);
                    SpecialAction(SpecialActionRequest::keeperJumpLeftPenalty);
                }
                else
                {
                    //SpecialAction(SpecialActionRequest::keeperBlockLeft,true);
                    SpecialAction(SpecialActionRequest::keeperJumpLeftPenalty,true);
                }
                // SpecialAction(SpecialActionRequest::keeperJumpLeftPenalty);
            }
                
        } 
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
    
    state(judge)
    {
        transition
        {
            if(theBallModel.estimate.position.norm()<1500.f && theBallModel.estimate.velocity.norm()<100.f)
                goto startToKick;
            else if(robotToField(theRobotPose,theBallModel.estimate.position).x()<0.f)
                goto walkToKeeperPoint;
            else 
                goto initPoint;         
        }
    }
    
    state(startToKick)
    {
        transition
        {
            if(state_time>300&&action_done)
            {
                goto kickBallAway;
            }           
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
                goto alignBehindBall;
        }
        action
        {
            LookUpAndDown();
            WalkToTarget(Pose2f(50.f, 50.f, 50.f), theBallModel.estimate.position);
        }
    }
    

    state(alignBehindBall)
    {
        transition
        {
          if(libCodeRelease.between(theBallModel.estimate.position.y(), 20.f, 50.f)
          && libCodeRelease.between(theBallModel.estimate.position.x(), 140.f, 170.f))
//          && std::abs(libCodeRelease.angleToCenter) < 2_deg)
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
          goto standAfterKick;
        }
        action
        {
          LookForward();
          InWalkKick(WalkKickVariant(WalkKicks::forward, Legs::left), Pose2f(libCodeRelease.angleToGoal, theBallModel.estimate.position.x() - 160.f, theBallModel.estimate.position.y() - 55.f));
        }
    }
    
    state(standAfterKick)
    {
        transition
        {
            if(state_time>2000 && action_done)
                goto judgeAfterKick;
        }
        action
        {
            LookAtBall();
            Stand();
        }
    }
    
    state(judgeAfterKick)
    {
        transition
        {
            if(theBallModel.estimate.position.norm()<1500.f)
                goto startToKick;
            else 
                goto walkBack;
        }
    }
    
    state(walkBack)
    {
        transition
        {
          if(theBallModel.estimate.position.norm()<1500.f && theBallModel.estimate.velocity.norm()<100.f)
              goto startToKick;
          else if((libCodeRelease.between(theRobotPose.translation.x(),-4005.f,-3995.f))
                  &&(libCodeRelease.between(theRobotPose.translation.y(),-5.f,5.f)))
              goto stand;
        }
        action
        {
          LookAtBall();
          WalkToTarget(Pose2f(100.f,100.f,100.f),Pose2f(-theRobotPose.rotation,-4000-theRobotPose.translation.x(),-theRobotPose.translation.y()));
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
    
    
    state(walkToKeeperPoint)
    {
        transition
        {
            if(theBallModel.estimate.position.norm()<1500.f && theBallModel.estimate.velocity.norm()<100.f)
                goto startToKick;
            else if(libCodeRelease.between(theRobotPose.translation.x(),-4005.f,-3995.f) &&
                libCodeRelease.between(theRobotPose.translation.y(),robotToField(theRobotPose,theBallModel.estimate.position).y()/9.0f-5.f,
                    robotToField(theRobotPose,theBallModel.estimate.position).y()/9.0f+5.f))
                goto turnToBall;
        }
        action
        {
            WalkToTargetWithPathMode(Pose2f(100.f,100.f,100.f),Pose2f(libCodeRelease.angleToGoal,-4000.f,robotToField(theRobotPose,theBallModel.estimate.position).y()/9));
        }
    }
    
    state(turnToBall)
    {
        transition
        {
            if(theBallModel.estimate.position.norm()<1500.f && theBallModel.estimate.velocity.norm()<100.f)
                goto startToKick;
            else if(std::abs(theBallModel.estimate.position.angle()) < 5_deg)
                goto stand;
        }
        action
        {
            LookAtBall();
            WalkToTarget(Pose2f(50.f, 50.f, 50.f), Pose2f(theBallModel.estimate.position.angle(), 0.f, 0.f));
        }
    }
    
    state(initPoint)
    {
        transition
        {
          if(theBallModel.estimate.position.norm()<1500.f && theBallModel.estimate.velocity.norm()<100.f)
              goto startToKick;
          else if((libCodeRelease.between(theRobotPose.translation.x(),-4005.f,-3995.f))
                  &&(libCodeRelease.between(theRobotPose.translation.y(),-5.f,5.f)))
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

