/* 
 * Copyright:Hust_iRobot
 * File:   Supporter.h
 * Author: Shangyunfei
 * Description:A test Supporter created by shangyunfei using in SimRobot.Have not test in real Robot.So 
 *  you would be better not to use it in real competition.
 * Created on 2017?10?1?, ??10:34
 */

#ifndef SUPPORTER_H
#define SUPPORTER_H
option(Supporter)
{   
    initial_state(start)
    {
        transition
        {
             if(state_time > 1000)
                 goto walkToOpponentSide;
        }
        action
        {
            LookForward();
            Stand();
        }
    }

    state(walkToOpponentSide)
    {
        transition
        {
            Vector2f robotFieldPos=Transformation::robotToField(theRobotPose,Vector2f(0.f,0.f));
            if(libCodeRelease.between(robotFieldPos.x(),1950.f,2050.f) &&
                libCodeRelease.between(robotFieldPos.y(),-1050.f,-950.f))
               goto turnRobotToCenter;     
        }
        action
        {
            LookForward();
            WalkToTargetWithPathMode(Pose2f(100.f,100.f,100.f),Pose2f(libCodeRelease.angleToGoal,2000.f,-1000.f));
        }
    }
    
    state(turnRobotToCenter)
    {
        transition
        {
            if(std::abs(libCodeRelease.angleToCenter)<5_deg)
                goto lookAtball;
        }
        action
        {
            LookForward();
            WalkToTarget(Pose2f(100.f,100.f,100.f),Pose2f(libCodeRelease.angleToCenter,0.f,0.f));
        }
    }
    
    state(lookAtball)
    {
        transition
        {
            if(theBallModel.estimate.position.norm() < 2000.f)           
                Striker();
        }
        action
        {
            LookAtBall();
        }
    } 
}



#endif /* SUPPORTER_H */

