/**
 * @file Modules/MotionControl/LegMotionCombinator.cpp
 * This file declares a module that combines the leg motions created by the different modules.
 * @author <a href="mailto:jesse@tzi.de">Jesse Richter-Klug</a>
 * based on a module created by @author <A href="mailto:Thomas.Roefer@dfki.de">Thomas Röfer</A>
 */

#include "LegMotionCombinator.h"
#include "MotionCombinator.h"
#include "Tools/RobotParts/Joints.h"

MAKE_MODULE(LegMotionCombinator, motionControl)

void LegMotionCombinator::update(LegJointRequest& jointRequest)
{
  const JointRequest* jointRequests[MotionRequest::numOfMotions];
  jointRequests[MotionRequest::walk] = &theWalkLegRequest;
  jointRequests[MotionRequest::kick] = &theKickEngineOutput;
  jointRequests[MotionRequest::specialAction] = &theSpecialActionsOutput;
  jointRequests[MotionRequest::stand] = &theStandLegRequest;
  jointRequests[MotionRequest::getUp] = &theGetUpEngineOutput;

  jointRequest.angles[Joints::headYaw] = theHeadJointRequest.pan;
  jointRequest.angles[Joints::headPitch] = theHeadJointRequest.tilt;
  MotionCombinator::copy(*jointRequests[theLegMotionSelection.targetMotion], jointRequest, theStiffnessSettings, Joints::headYaw, Joints::headPitch);
  MotionCombinator::copy(*jointRequests[theLegMotionSelection.targetMotion], jointRequest, theStiffnessSettings, Joints::firstLegJoint, Joints::rAnkleRoll);

  ASSERT(jointRequest.isValid());

  if(theLegMotionSelection.ratios[theLegMotionSelection.targetMotion] == 1.f)
  {
    lastJointAngles = theJointAngles;
  }
  else // interpolate motions
  {
    const bool interpolateStiffness = !(theLegMotionSelection.targetMotion != MotionRequest::specialAction &&
                                        theLegMotionSelection.specialActionRequest.specialAction == SpecialActionRequest::playDead &&
                                        theLegMotionSelection.ratios[MotionRequest::specialAction] > 0.f); // do not interpolate from play_dead
    for(int i = 0; i < MotionRequest::numOfMotions; ++i)
      if(i != theLegMotionSelection.targetMotion && theLegMotionSelection.ratios[i] > 0.f)
      {
        MotionCombinator::interpolate(*jointRequests[i], *jointRequests[theLegMotionSelection.targetMotion], theLegMotionSelection.ratios[i],
                                      jointRequest, interpolateStiffness, theStiffnessSettings, lastJointAngles, Joints::headYaw, Joints::headYaw);

        MotionCombinator::interpolate(*jointRequests[i], *jointRequests[theLegMotionSelection.targetMotion], theLegMotionSelection.ratios[i],
                                      jointRequest, interpolateStiffness, theStiffnessSettings, lastJointAngles, Joints::firstLegJoint, Joints::rAnkleRoll);
      }
  }

  ASSERT(jointRequest.isValid());
}
