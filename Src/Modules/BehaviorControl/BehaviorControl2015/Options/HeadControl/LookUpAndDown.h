/* 
 * Copyright:Hust_iRobot
 * File:   LookUpAndDown.h
 * Author: Shangyunfei
 * Description:Just as the name say.
 * Created on 2017?10?1?, ??10:14
 */

#ifndef LOOKUPANDDOWN_H
#define LOOKUPANDDOWN_H
option(LookUpAndDown, (float)(0.f)pan,(float)(0.38f)tilt)
{
    initial_state(lookUp)
    {
        transition
        {
            if (state_time > 700)
                goto lookDown;
        }
        action
        {
            SetHeadPanTilt(pan,tilt,(float)(pi),(HeadMotionRequest::lowerCamera));
        }
    }
    
    state(lookDown)
    {
        transition
        {
            if (state_time > 700)
                goto lookUp;
        }
        action
        {
             SetHeadPanTilt(pan,-tilt,(float)(pi),(HeadMotionRequest::lowerCamera));
        }
    }
}        


#endif /* LOOKUPANDDOWN_H */

