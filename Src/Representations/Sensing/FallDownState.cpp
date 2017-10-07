/**
 * @file Representations/Sensing/FallDownState.cpp
 *
 * Implementation of a debug drawing for the FallDownState.
 *
 * @author Carsten Könemann
 */

#include "FallDownState.h"
#include "Tools/Debugging/DebugDrawings.h"

void FallDownState::draw() const
{
  DEBUG_DRAWING("representation:FallDownState", "drawingOnImage")
  {
    // text-shadow for better visibility
    DRAWTEXT("representation:FallDownState", 26, 26, 35, ColorRGBA::black, "State: " << getName(state));
    DRAWTEXT("representation:FallDownState", 26, 51, 35, ColorRGBA::black, "Direction: " << getName(direction));
    // text
    DRAWTEXT("representation:FallDownState", 25, 25, 35, ColorRGBA::white, "State: " << getName(state));
    DRAWTEXT("representation:FallDownState", 25, 50, 35, ColorRGBA::white, "Direction: " << getName(direction));
  }
}
