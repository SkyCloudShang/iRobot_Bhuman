/* 
 * Copyright:Hust_iRobot
 * File:   LookLeftAndRight.h
 * Author: Shangyunfei
 * Description:Just as the name say.
 * Created on 2017?10?1?, ??10:13
 */

#ifndef LOOKLEFTANDRIGHT_H
#define LOOKLEFTANDRIGHT_H
option(LookLeftAndRight,(float) (0.38f) pan,(float) (0.f) tilt)
{
    initial_state(lookLeft)
    {
        transition
        {
            if(action_done && state_time>1000.f)
                goto lookRight;
        }
        action
        {
            SetHeadPanTilt(-pan, tilt, 150_deg);
        }
    }
    
    state(lookRight)
    {
        transition
        {
            if(action_done && state_time>1000.f)
                goto lookLeft;
        }
        action
        {
            SetHeadPanTilt(pan, tilt, 150_deg);
        }
    }
}


#endif /* LOOKLEFTANDRIGHT_H */

