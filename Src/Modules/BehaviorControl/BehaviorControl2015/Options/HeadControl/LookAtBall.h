/* 
 * Copyright:Hust_iRobot
 * File:   LookAtBall.h
 * Author: Shangyunfei
 * Description:Just as the name say.
 * Created on 2017?10?1?, ??10:12
 */

#ifndef LOOKATBALL_H
#define LOOKATBALL_H
option(LookAtBall)
{
    initial_state(lookAtBall)
    {
        transition
        {
            if(libCodeRelease.timeSinceBallWasSeen()> 2500)
                goto lookForBall;
        }
        action
        {
            Vector3f theBall(theBallModel.estimate.position.x(),theBallModel.estimate.position.y(),50.0f);
            SetHeadTargetSpeedRequest(theBall);
        }       
    }
    
    state(lookForBall)
    {
        transition
        {
            if(theBallModel.seenPercentage>0.8)
                goto lookAtBall;
        }
        action
        {
            LookUpAndDown();
        }
    }
}



#endif /* LOOKATBALL_H */

