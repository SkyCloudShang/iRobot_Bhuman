/**
 * @file Modules/MotionControl/MotionCombinator.cpp
 * This file implements a module that combines the motions created by the different modules.
 * @author <A href="mailto:Thomas.Roefer@dfki.de">Thomas Röfer</A>
 * @author <a href="mailto:jesse@tzi.de">Jesse Richter-Klug</a>
 */

#include "MotionCombinator.h"
#include "Tools/Motion/SensorData.h"
#include "Tools/Debugging/DebugDrawings.h"
#include "Tools/Math/Rotation.h"
#include "Tools/Math/RotationMatrix.h"

MAKE_MODULE(MotionCombinator, motionControl)

void MotionCombinator::update(JointRequest& jointRequest)
{
  specialActionOdometry += theSpecialActionsOutput.odometryOffset;

  MotionUtilities::copy(theHeadJointRequest, jointRequest, theStiffnessSettings, Joints::headYaw, Joints::headPitch);
  MotionUtilities::copy(theArmJointRequest, jointRequest, theStiffnessSettings, Joints::firstArmJoint, Joints::rHand);
  MotionUtilities::copy(theLegJointRequest, jointRequest, theStiffnessSettings, Joints::firstLegJoint, Joints::rAnkleRoll);

  ASSERT(jointRequest.isValid());

  Pose2f odometryOffset;

  // Find fully active motion and set MotionInfo
  if(theLegMotionSelection.ratios[theLegMotionSelection.targetMotion] == 1.f)
  {
    // default values
    motionInfo.motion = theLegMotionSelection.targetMotion;
    motionInfo.isMotionStable = true;
    motionInfo.upcomingOdometryOffset = Pose2f();

    switch(theLegMotionSelection.targetMotion)
    {
      case MotionRequest::walk:
        odometryOffset = theWalkingEngineOutput.odometryOffset;
        motionInfo.walkRequest = theWalkingEngineOutput.executedWalk;
        motionInfo.upcomingOdometryOffset = theWalkingEngineOutput.upcomingOdometryOffset;
        break;
      case MotionRequest::kick:
        odometryOffset = theKickEngineOutput.odometryOffset;
        motionInfo.kickRequest = theKickEngineOutput.executedKickRequest;
        motionInfo.isMotionStable = theKickEngineOutput.isStable;
        break;
      case MotionRequest::specialAction:
        odometryOffset = specialActionOdometry;
        specialActionOdometry = Pose2f();
        motionInfo.specialActionRequest = theSpecialActionsOutput.executedSpecialAction;
        motionInfo.isMotionStable = theSpecialActionsOutput.isMotionStable;
        break;
      case MotionRequest::getUp:
        motionInfo.isMotionStable = false;
        odometryOffset = theGetUpEngineOutput.odometryOffset;
        break;
      case MotionRequest::fall:
        motionInfo.isMotionStable = false;
        odometryOffset = Pose2f();
        break;
      case MotionRequest::stand:
      default:
        break;
    }
  }

  if(theRobotInfo.hasFeature(RobotInfo::zGyro))
  {
    if(theFallDownState.state == FallDownState::falling || theFallDownState.state == FallDownState::fallen)
      odometryOffset.rotation = 0_deg; // postpone rotation change until being upright again
    else
      odometryOffset.rotation = Angle::normalize(Rotation::Euler::getZAngle(theInertialData.orientation3D) - odometryData.rotation);
  }
  else
    odometryOffset.rotation = Angle::normalize(odometryOffset.rotation + theFallDownState.odometryRotationOffset);

  if(useAccFusion && theLegMotionSelection.targetMotion == MotionRequest::walk)
    estimateOdometryOffset(odometryOffset.translation);

  odometryData += odometryOffset;

  if(!useStiffnessDirectly)
    applyDynamicStiffness(jointRequest);

  if(debugArms)
    debugReleaseArms(jointRequest);
  applyDamageConfig(jointRequest);

  float sum = 0;
  int count = 0;
  for(int i = Joints::lHipYawPitch; i < Joints::numOfJoints; i++)
  {
    if(jointRequest.angles[i] != JointAngles::off && jointRequest.angles[i] != JointAngles::ignore &&
       lastJointRequest.angles[i] != JointAngles::off && lastJointRequest.angles[i] != JointAngles::ignore)
    {
      sum += std::abs(jointRequest.angles[i] - lastJointRequest.angles[i]);
      count++;
    }
  }
  PLOT("module:MotionCombinator:deviations:JointRequest:legsOnly", sum / count);
  for(int i = 0; i < Joints::lHipYawPitch; i++)
  {
    if(jointRequest.angles[i] != JointAngles::off && jointRequest.angles[i] != JointAngles::ignore &&
       lastJointRequest.angles[i] != JointAngles::off && lastJointRequest.angles[i] != JointAngles::ignore)
    {
      sum += std::abs(jointRequest.angles[i] - lastJointRequest.angles[i]);
      count++;
    }
  }
  PLOT("module:MotionCombinator:deviations:JointRequest:all", sum / count);

  sum = 0;
  count = 0;
  for(int i = Joints::lHipYawPitch; i < Joints::numOfJoints; i++)
  {
    if(lastJointRequest.angles[i] != JointAngles::off && lastJointRequest.angles[i] != JointAngles::ignore)
    {
      sum += std::abs(lastJointRequest.angles[i] - theJointAngles.angles[i]);
      count++;
    }
  }
  PLOT("module:MotionCombinator:differenceToJointData:legsOnly", sum / count);

  for(int i = 0; i < Joints::lHipYawPitch; i++)
  {
    if(lastJointRequest.angles[i] != JointAngles::off && lastJointRequest.angles[i] != JointAngles::ignore)
    {
      sum += std::abs(lastJointRequest.angles[i] - theJointAngles.angles[i]);
      count++;
    }
  }
  lastJointRequest = jointRequest;
  PLOT("module:MotionCombinator:differenceToJointData:all", sum / count);

#ifndef NDEBUG
  if(!jointRequest.isValid())
  {
    {
      std::string logDir = "";
#ifdef TARGET_ROBOT
      logDir = "../logs/";
#endif
      OutMapFile stream(logDir + "jointRequest.log");
      stream << jointRequest;
      OutMapFile stream2(logDir + "motionSelection.log");
      stream2 << theLegMotionSelection;
    }
    FAIL("Joint request invalid.");
  }
#endif
}

void MotionCombinator::update(OdometryData& odometryData)
{
  odometryData = this->odometryData;

  Pose2f odometryOffset = odometryData;
  odometryOffset -= lastOdometryData;
  PLOT("module:MotionCombinator:odometryOffsetX", odometryOffset.translation.x());
  PLOT("module:MotionCombinator:odometryOffsetY", odometryOffset.translation.y());
  PLOT("module:MotionCombinator:odometryOffsetRotation", odometryOffset.rotation.toDegrees());
  lastOdometryData = odometryData;
}

void MotionCombinator::applyDynamicStiffness(JointRequest& jointRequest) const
{
  auto dynamicStiffnessFunction = [&](Joints::Joint joint)
  {
    ASSERT(jointRequest.stiffnessData.stiffnesses[joint] <= 100);
    if(jointRequest.stiffnessData.stiffnesses[joint] == 0)
      return 0.f;

    const Angle dist = std::abs(jointRequest.angles[joint] - theJointAngles.angles[joint]);

    const Angle maxPos = static_cast<float>(101 - jointRequest.stiffnessData.stiffnesses[joint]) * 1_deg;
    const Angle maxPos_2 = maxPos / 2.f;

    static const float minStiffness = 0.1f;
    static const float stiffnessWorkspace = 1.f - 0.1f;
    static const float graphCompression = stiffnessWorkspace / 2.f;

    if(dist < maxPos_2)
      return minStiffness + sqr(dist / maxPos_2) * graphCompression;
    else if(dist < maxPos)
      return 1.f - sqr((dist - maxPos) / maxPos_2) * graphCompression;
    else
      return 1.f;
  };

  FOREACH_ENUM((Joints) Joint, joint)
    jointRequest.stiffnessData.stiffnesses[joint] = static_cast<int>(100.f * dynamicStiffnessFunction(joint));
}

void MotionCombinator::debugReleaseArms(JointRequest& jointRequest) const
{
  /*if(theKeyStates.pressed[theKeyStates.lHandBack] ||
    theKeyStates.pressed[theKeyStates.lHandLeft] ||
    theKeyStates.pressed[theKeyStates.lHandRight])
    for(unsigned i = Joints::firstLeftArmJoint; i <= Joints::lHand; i++)
      jointRequest.stiffnessData.stiffnesses[i] = 0;

  if(theKeyStates.pressed[theKeyStates.rHandBack] ||
    theKeyStates.pressed[theKeyStates.rHandLeft] ||
    theKeyStates.pressed[theKeyStates.rHandRight])
    for(unsigned i = Joints::firstRightArmJoint; i <= Joints::rHand; i++)
      jointRequest.stiffnessData.stiffnesses[i] = 0;*/

  if(theKeyStates.pressed[theKeyStates.headFront]
     || theKeyStates.pressed[theKeyStates.headMiddle]
     || theKeyStates.pressed[theKeyStates.headRear])
    for(unsigned i = Joints::firstLeftArmJoint; i <= Joints::rHand; i++)
      jointRequest.stiffnessData.stiffnesses[i] = 0;
}

void MotionCombinator::applyDamageConfig(JointRequest& jointRequest) const
{
  for(Joints::Joint j : theDamageConfigurationBody.jointsToEraseStiffness)
    jointRequest.stiffnessData.stiffnesses[j] = 0;
}

void MotionCombinator::estimateOdometryOffset(Vector2f& offset)
{
  auto dynamicModel = [&](Vector3f& state)
  {
    Vector3f accCorrected = (theInertialData.orientation2D * (useFilteredAcc ? theInertialData.filteredAcc : theInertialData.acc));
    state += (accCorrected - Vector3f(0, 0, Constants::g_1000)) * Constants::motionCycleTime * 1000;
  };
  auto pseudoMeasurement = [&](const Vector3f& state)
  {
    return state.z();
  };
  auto realMeasurement = [&](const Vector3f& state)
  {
    return (state * Constants::motionCycleTime).head<2>();
  };
  odometryUKF.predict(dynamicModel, dynamicVariance.asDiagonal());
  odometryUKF.update(0.f, pseudoMeasurement, pseudoVariance);
  odometryUKF.update<2>(offset, realMeasurement, measurementVariance.asDiagonal());

  offset = odometryUKF.mean.head<2>() * Constants::motionCycleTime;
}
