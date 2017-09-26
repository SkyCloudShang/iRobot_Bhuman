/**
 * @file Representations/BehaviorControl/BehaviorLEDRequest.h
 * This file contains the BehaviorLEDRequest struct.
 * @author jeff
 */

#pragma once

#include "Representations/Infrastructure/LEDRequest.h"

STREAMABLE(BehaviorLEDRequest,
{
  ENUM(BehaviorLED,
  {,
    leftEye,
    rightEye,
    leftEar,
    rightEar,
  });

  ENUM(EyeColor,
  {,
    defaultColor,
    red,
    green,
    blue,
    white,
    magenta,
    yellow,
    cyan,
  });

  BehaviorLEDRequest()
  {
    FOREACH_ENUM(BehaviorLED, i)
      modifiers[i] = LEDRequest::on;
  }

  bool operator==(const BehaviorLEDRequest& other) const
  {
    FOREACH_ENUM(BehaviorLED, i)
      if(modifiers[i] != other.modifiers[i])
        return false;
    return true;
  }

  bool operator!=(const BehaviorLEDRequest& other) const
  {
    return !(*this == other);
  },

  ((LEDRequest) LEDState[numOfBehaviorLEDs]) modifiers,
  (EyeColor)(defaultColor) leftEyeColor,
  (EyeColor)(defaultColor) rightEyeColor,
});
