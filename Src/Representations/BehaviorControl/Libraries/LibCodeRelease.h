/**
 * @file LibCodeRelease.h
 */

#pragma once
#include "Tools/Function.h"

STREAMABLE(LibCodeRelease,
{
  FUNCTION(bool(float value, float min, float max)) between,
  
  (float) angleToGoal,
  (int) timeSinceBallWasSeen,
          
  //add by Shangyunfei        
  (float)angleToCenter,
         
  (float)angleToBall,        
          
  (float)odometryRSum,
  (float)odometryXSum,
});
