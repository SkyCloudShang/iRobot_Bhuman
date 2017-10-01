/* 
 * Copyright:Hust_iRobot
 * File:   KeyFrameArms.h
 * Author: Shangyunfei
 * Description:This mode is used for nao to control his arms such as let his one arm back when
 *      his arm is pushed by another robot.
 * Created on 2017?10?1?, ??10:01
 */

#ifndef KEYFRAMEARMS_H
#define KEYFRAMEARMS_H
option(KeyFrameArms,((ArmKeyFrameRequest) ArmKeyFrameId)motion,(bool)(false)fast)
{
    initial_state(setRequest)
    {
        transition
        {
            if(theArmMotionSelection.targetArmMotion[Arms::left]==ArmMotionSelection::keyFrameS &&
                  theArmKeyFrameEngineOutput.arms[Arms::left].motion==motion &&
                  theArmMotionSelection.targetArmMotion[Arms::right]==ArmMotionSelection::keyFrameS &&
                  theArmKeyFrameEngineOutput.arms[Arms::right].motion==motion)
                goto requestIsExecuted;
        }
        action
        {
            KeyFrameLeftArm(motion,fast);
            KeyFrameRightArm(motion,fast);
        }
    }
    
    target_state(requestIsExecuted)
    {
        transition
        {
            if(theArmMotionSelection.targetArmMotion[Arms::left]!=ArmMotionSelection::keyFrameS  ||
                  theArmKeyFrameEngineOutput.arms[Arms::left].motion!=motion || 
                  theArmMotionSelection.targetArmMotion[Arms::right]!=ArmMotionSelection::keyFrameS  ||
                  theArmKeyFrameEngineOutput.arms[Arms::right].motion!=motion )
                goto setRequest;
        }
        action
        {
           KeyFrameLeftArm(motion,fast);
           KeyFrameRightArm(motion,fast);
        }
    }
    
    
}


option(KeyFrameLeftArm,((ArmKeyFrameRequest) ArmKeyFrameId)motion,(bool)(false)fast)
{
    initial_state(setRequest)
    {
        transition
        {
            if(theArmMotionSelection.targetArmMotion[Arms::left]==ArmMotionSelection::keyFrameS &&
                  theArmKeyFrameEngineOutput.arms[Arms::left].motion==motion)
                goto requestIsExecuted;
        }
        action
        {
            theArmMotionRequest.armMotion[Arms::left]=ArmMotionRequest::keyFrame;
            theArmMotionRequest.armKeyFrameRequest.arms[Arms::left].motion=motion;
            theArmMotionRequest.armKeyFrameRequest.arms[Arms::left].fast=fast;
        }
    }
    
    target_state(requestIsExecuted)
    {
        transition
        {
            if(theArmMotionSelection.targetArmMotion[Arms::left]!=ArmMotionSelection::keyFrameS ||
                  theArmKeyFrameEngineOutput.arms[Arms::left].motion!=motion)
                goto setRequest;
        }
        action
        {
            theArmMotionRequest.armMotion[Arms::left]=ArmMotionRequest::keyFrame;
            theArmMotionRequest.armKeyFrameRequest.arms[Arms::left].motion=motion;
            theArmMotionRequest.armKeyFrameRequest.arms[Arms::left].fast=fast;
        }
    }
}

option(KeyFrameRightArm,((ArmKeyFrameRequest) ArmKeyFrameId)motion,(bool)(false)fast)
{
    initial_state(setRequest)
    {
        transition
        {
            if(theArmMotionSelection.targetArmMotion[Arms::right]==ArmMotionSelection::keyFrameS &&
                  theArmKeyFrameEngineOutput.arms[Arms::right].motion==motion)
                goto requestIsExecuted;
        }
        action
        {
            theArmMotionRequest.armMotion[Arms::right]=ArmMotionRequest::keyFrame;
            theArmMotionRequest.armKeyFrameRequest.arms[Arms::right].motion=motion;
            theArmMotionRequest.armKeyFrameRequest.arms[Arms::right].fast=fast;
        }
    }
    
    target_state(requestIsExecuted)
    {
        transition
        {
            if(theArmMotionSelection.targetArmMotion[Arms::right]!=ArmMotionSelection::keyFrameS||
                  theArmKeyFrameEngineOutput.arms[Arms::right].motion!=motion)
                goto setRequest;
        }
        action
        {
            theArmMotionRequest.armMotion[Arms::right]=ArmMotionRequest::keyFrame;
            theArmMotionRequest.armKeyFrameRequest.arms[Arms::right].motion=motion;
            theArmMotionRequest.armKeyFrameRequest.arms[Arms::right].fast=fast;
        }
    }
}



#endif /* KEYFRAMEARMS_H */

