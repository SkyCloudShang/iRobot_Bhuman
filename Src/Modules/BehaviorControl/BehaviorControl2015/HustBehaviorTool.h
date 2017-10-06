/* 
 * Copyright:Hust_iRobot
 * File:   HustBehaviorTool.h
 * Author: Shangyunfei
 * Description:
 * Created on October 5, 2017, 10:11 AM
 */
#ifndef HUSTBEHAVIORTOOL_H
#define HUSTBEHAVIORTOOL_H

#include "BehaviorControl2015.h"

namespace Behavior2015
{
    class HustBehaviorTool:public BehaviorControl2015Base
    {
        
        /**
   * Computes the position where a rolling ball is expected to stop rolling.
   * @param p The ball position (in mm)
   * @param v The ball velocity (in mm/s)
   * @param ballFriction The ball friction (negative force) (in m/s^2)
   * @return The position relative to the robot (in mm)
   */
        /********************************************************
         * Description:
         * @param:
         * @param:
         * @return
         * Author:
         * Date:
        ********************************************************/
        static float estimatedDropInTimeToReachBall(TeammateData& teammateData, int robotNumber);
        
    };
}



#endif /* HUSTBEHAVIORTOOL_H */

