/* 
 * Copyright:Hust_iRobot
 * File:   Robot2.h
 * Author: Shangyunfei
 * Description:This file is used for RelayCompetition.
 * Created on October 4, 2017, 4:56 PM
 */

#ifndef ROBOT2_H
#define ROBOT2_H
option(Robot2)
{ 
    float p = -0.9f;

    initial_state(stand)
    {
	transition
	{
            if(!theTeammateData.teammates.empty() && theTeammateData.teammates[0].firstRobotArrived)
                goto walkStraight;
	}
	action
	{
            Stand();
	}
    }

    state(walkStraight)
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


#endif /* ROBOT2_H */

