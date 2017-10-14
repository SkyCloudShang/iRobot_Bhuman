/* 
 * Copyright:Hust_iRobot
 * File:   Robot1.h
 * Author: Shangyunfei
 * Description:This file is used for RelayCompetition.
 * Created on October 4, 2017, 4:56 PM
 */

#ifndef ROBOT1_H
#define ROBOT1_H
option(Robot1)
{
    float p = -0.9f;
    common_transition
    {
    	if (theLibCodeRelease.odometryXSum >= 1000.f)
        {
            theBehaviorStatus.firstRobotArrived = 1;
        }
        else
            theBehaviorStatus.firstRobotArrived = 0;
    }

    initial_state(walkStraight)
    {
        transition
        {
            if(theLibCodeRelease.odometryXSum >= 1000.f)
        	goto finish;
        }
        action
        {
            WalkAtRelativeSpeed(Pose2f(p * theLibCodeRelease.odometryRSum, 1.0f, 0.f));
        }
    }

    state(finish)
    {
        action
        {
            Stand();
        }
    }
}

#endif /* ROBOT1_H */

