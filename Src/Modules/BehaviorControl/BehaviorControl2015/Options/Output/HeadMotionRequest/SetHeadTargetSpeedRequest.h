/* 
 * Copyright:Hust_iRobot
 * File:   SetHeadTargetSpeedRequest.h
 * Author: Shangyunfei
 * Description:This mode is used in speed mode.
 * Created on 2017?10?1?, ??10:05
 */

#ifndef SETHEADTARGETSPEEDREQUEST_H
#define SETHEADTARGETSPEEDREQUEST_H
option(SetHeadTargetSpeedRequest,(Vector3f) target,(float)(pi) speed, ((HeadMotionRequest) CameraControlMode)(autoCamera) camera, (bool)(false) stopAndGoMode)
{
    initial_state(setRequest)
    {
        transition
        {
            if(state_time > 200 && !theHeadJointRequest.moving)
            goto targetReached;
        }
        action
        {
            theHeadMotionRequest.mode = HeadMotionRequest::targetOnGroundMode;
            theHeadMotionRequest.target=target;
            theHeadMotionRequest.speed = speed;
            theHeadMotionRequest.cameraControlMode = camera;
            theHeadMotionRequest.stopAndGoMode = stopAndGoMode;
        } 
    }
    
    state(targetReached)
    {
        transition
        {
            goto setRequest;
        }
        action
        {
            theHeadMotionRequest.mode = HeadMotionRequest::targetOnGroundMode;
            theHeadMotionRequest.target=target;
            theHeadMotionRequest.speed = speed;
        }
    }
}


#endif /* SETHEADTARGETSPEEDREQUEST_H */

