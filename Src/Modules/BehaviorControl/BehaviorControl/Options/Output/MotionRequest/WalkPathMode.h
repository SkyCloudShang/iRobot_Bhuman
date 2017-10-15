/* 
 * Copyright:Hust_iRobot
 * File:   WalkPathMode.h
 * Author: Shangyunfei
 * Description:
 * Created on October 15, 2017, 10:53 AM
 */

#ifndef WALKPATHMODE_H
#define WALKPATHMODE_H

option(WalkPathMode,(const Pose2f&) target, (const Pose2f&)(Pose2f(1.f,1.f,1.f)) speed, (bool)(true)excludePArea)
{
    initial_state(setAction)
    {
        action
        {
            theMotionRequest=thePathPlanner.plan(target,speed,excludePArea);
        }
    }
}

#endif /* WALKPATHMODE_H */

