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
            if(!theTeamData.teammates.empty() && theTeamData.teammates[0].theBehaviorStatus.firstRobotArrived==1)
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


#endif /* ROBOT2_H */

