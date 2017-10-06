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
    	if (libCodeRelease.odometryXSum >= 3100.f)
        {
            theBehaviorStatus.firstRobotArrived = true;
        }
        else
            theBehaviorStatus.firstRobotArrived = false;
    }

    initial_state(walkStraight)
    {
        transition
        {
            if(libCodeRelease.odometryXSum >= 3500.f)
        	goto finish;
        }
        action
        {
            WalkAtSpeedPercentage(Pose2f(p * libCodeRelease.odometryRSum, 0.8f, 0.f));
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

