/* 
 * Copyright:Hust_iRobot
 * File:   ArmContact.h
 * Author: Shangyunfei
 * Description:Used for reactions to arm contact.
 * Created on 2017?10?1?, ??10:24
 */

#ifndef ARMCONTACT_H
#define ARMCONTACT_H
option(ArmContact)
{
    initial_state(mainDo)
    {
        ArmContactLeft();
        ArmContactRight();
    }
}

option(ArmContactLeft,(const int)(3000)actionDelay, (const int)(2000)targetTime)
{
    initial_state(judgeContact)
    {
        transition
        {
            if(theArmContactModel.contactLeft)
            {
                switch(theArmContactModel.pushDirectionLeft)
                {
                    case ArmContactModel::S:
                    case ArmContactModel::SE:
                    case ArmContactModel::SW:
                        goto handelLeftContact;
                    default:
                        break;
                }
            }       
        }
    }
    
    state(handelLeftContact)
    {
        transition
        {
            if(state_time>targetTime)
                goto actionDelayTime;
        }
        action
        {
            KeyFrameLeftArm(ArmKeyFrameRequest::back);
        }
    }
    
    state(actionDelayTime)
    {
        transition
        {
            if(state_time>actionDelay)
            {
                goto judgeContact;
            }
        }
    }
}

option(ArmContactRight,(const int)(3000)actionDelay, (const int)(2000)targetTime)
{
     initial_state(judgeContact)
    {
        transition
        {
            if(theArmContactModel.contactRight)
            {
                switch(theArmContactModel.pushDirectionRight)
                {
                    case ArmContactModel::S:
                    case ArmContactModel::SE:
                    case ArmContactModel::SW:
                        goto handelRightContact;
                    default:
                        break;
                }
            }       
        }
    }
    
    state(handelRightContact)
    {
        transition
        {
            if(state_time>targetTime)
                goto actionDelayTime;
        }
        action
        {
            KeyFrameRightArm(ArmKeyFrameRequest::back);
        }
    }
    
    state(actionDelayTime)
    {
        transition
        {
            if(state_time>actionDelay)
            {
                goto judgeContact;
            }
        }
    }
}


#endif /* ARMCONTACT_H */

