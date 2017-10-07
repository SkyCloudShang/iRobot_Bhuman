/*
 * Copyright 2010 The University of New South Wales (UNSW).
 *
 * This file is part of the 2010 team rUNSWift RoboCup entry. You may
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version as
 * modified below. As the original licensors, we add the following
 * conditions to that license:
 *
 * In paragraph 2.b), the phrase "distribute or publish" should be
 * interpreted to include entry into a competition, and hence the source
 * of any derived work entered into a competition must be made available
 * to all parties involved in that competition under the terms of this
 * license.
 *
 * In addition, if the authors of a derived work publish any conference
 * proceedings, journal articles or other academic papers describing that
 * derived work, then appropriate academic citations to the original work
 * must be included in that publication.
 *
 * This rUNSWift source is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this source code; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/**
 * @file Walk2014Generator.cpp
 *
 * This file implements the UNSW 2014 walk generator. It was refactored to fit into the B-Human framework.
 *
 * The period of each foot-step is set by T. T generates the forcing function by alternatively lifting each foot.
 * Control:
 * The change of support foot is driven by the ZMP switching sign, but must be > T/2. If > 3*T we switch to try
 * to revive.
 * The front-back sway is controlled by ankle tilts proportional to the y gyro.
 * The user specifies forward(m), left(m), turn(radians) to activate the walk.
 * If these values are all zero the robot stands.
 *
 * @author Bernhard Hengst
 * @date 18 Jan 2014
 *
 * @author Thomas Röfer
 * @date 28 Aug 2017
 */

#include "Walk2014Generator.h"
#include "Platform/BHAssert.h"
#include "Platform/SystemCall.h"
#include "Representations/Sensing/RobotModel.h"
#include "Tools/Debugging/Annotation.h"
#include "Tools/Math/Constants.h"
#include "Tools/Math/Rotation.h"
#include "Tools/Motion/InverseKinematic.h"
#include "Tools/Range.h"
#include <algorithm>

MAKE_MODULE(Walk2014Generator, motionControl);

static const float mmPerM = 1000.f;

void Walk2014Generator::update(WalkGenerator& generator)
{
  // Use other parameters in demo games to take care of the robots
  if(theGlobalOptions.slowWalk)
  {
    maxSpeed.translation.x() = std::min(maxSpeed.translation.x(), slowMaxSpeed.translation.x());
    maxSpeed.translation.y() = std::min(maxSpeed.translation.y(), slowMaxSpeed.translation.y());
    maxSpeed.rotation = std::min(maxSpeed.rotation, slowMaxSpeed.rotation);
    maxSpeedBackwards = std::min(maxSpeedBackwards, slowMaxSpeedBackwards);
    maxAcceleration.x() = std::min(maxAcceleration.x(), slowMaxAcceleration.x());
    maxAcceleration.y() = std::min(maxAcceleration.y(), slowMaxAcceleration.y());
  }

  generator.reset = [this, &generator]() { reset(generator); };
  generator.calcJoints =
    [this, &generator](const Pose2f& speed, const Pose2f& target, WalkGenerator::WalkMode walkMode,
                       const std::function<Pose3f(float phase)>& getKickFootOffset)
  {
    calcJoints(generator, speed, target, walkMode, getKickFootOffset);
  };
  generator.maxSpeed = Pose2f(maxSpeed.rotation / speedScale.rotation,
                              maxSpeed.translation.x() / speedScale.translation.x(),
                              maxSpeed.translation.y() / speedScale.translation.y());
}

void Walk2014Generator::reset(WalkGenerator& generator)
{
  generator.stepDuration = 0.f;
  generator.t = 0.f;
  walkState = standing;
  forward = lastForward = 0.f;
  forwardL = forwardL0 = 0.f;
  forwardR = forwardR0 = 0.f;
  left = lastLeft = 0.f;
  leftL = leftR = 0_deg;
  turnRL = turnRL0 = 0_deg;
  swingAngle = 0_deg;
  switchPhase = 0.f;
  maxFootHeight = maxFootHeight0 = 0.f;
  weightShiftStatus = weightDidNotShift;
  filteredGyroX = filteredGyroY = 0_deg;
  prevForwardL = prevForwardR = 0.f;
  prevLeftL = prevLeftR = 0_deg;
  prevTurn = 0_deg;
  weightShiftMisses = 0;
  slowWeightShifts = 0;
}

void Walk2014Generator::calcJoints(WalkGenerator& generator,
                                   Pose2f speed,
                                   const Pose2f& target,
                                   WalkGenerator::WalkMode walkMode,
                                   const std::function<Pose3f(float phase)>& getKickFootOffset)
{
  // 1. Read in new walk values (forward, left, turn, power) only at the start of a walk step cycle, ie when t = 0
  if(generator.t == 0)
  {
    Pose2f request = speed;
    Pose2f maxSpeed = this->maxSpeed;
    float maxSpeedBackwards = this->maxSpeedBackwards;
    Pose2f returnOffset((generator.isLeftPhase ? -turnRL0 : turnRL0) / speedScale.rotation,
                        (generator.isLeftPhase ? -forwardR0 : -forwardL0) * mmPerM / speedScale.translation.x(),
                        (walkHipHeight - theRobotDimensions.footHeight) * std::tan(swingAngle) / speedScale.translation.y());

    if(theFrameInfo.getTimeSince(timeWhenSlowWeightShiftsDetected) <= slowWaitShiftStandDelay)
    {
      request = Pose2f();
      walkMode = WalkGenerator::stepSizeMode;
    }
    else if(weightShiftStatus == emergencyStep)
    {
      request = Pose2f(0_deg, 0, generator.isLeftPhase ? emergencyStepSize : -emergencyStepSize);
      walkMode = WalkGenerator::stepSizeMode;
      weightShiftStatus = weightDidShift;
    }

    if(walkMode == WalkGenerator::targetMode)
    {
      ASSERT(speed.rotation > 0.f && speed.translation.x() > 0.f && speed.translation.y() > 0.f);

      maxSpeed = Pose2f(std::min(speed.rotation * speedScale.rotation, static_cast<float>(this->maxSpeed.rotation)),
                        std::min(speed.translation.x() * speedScale.translation.x(), this->maxSpeed.translation.x()),
                        std::min(speed.translation.y() * speedScale.translation.y(), this->maxSpeed.translation.y()));
      maxSpeedBackwards = std::min(speed.translation.x() * speedScale.translation.x(), this->maxSpeedBackwards);
      // Remove the offset that will be covered just by returning the swing leg
      forward = (target.translation.x() - returnOffset.translation.x()) * speedScale.translation.x();
      left = (target.translation.y() - returnOffset.translation.y()) * speedScale.translation.y();
      turn = (target.rotation - returnOffset.rotation) * speedScale.rotation;
      generator.stepDuration = (baseWalkPeriod + sidewaysWalkPeriodIncreaseFactor * std::abs(left)) / 1000.f;
      generator.speed = Pose2f(turn / generator.stepDuration, forward / generator.stepDuration, left / generator.stepDuration);
      if(ellipsoidClampWalk(maxSpeed, maxSpeedBackwards, generator.speed.translation.x(), generator.speed.translation.y(), generator.speed.rotation))
      {
        walkMode = WalkGenerator::speedMode;
        request = Pose2f(target.rotation * targetModeSpeedFactor, target.translation * targetModeSpeedFactor);
      }
      else
      {
        // Consider in the speed that half of the step is returning to origin
        generator.speed.rotation = 0.5f * generator.speed.rotation + 0.5f / generator.stepDuration * returnOffset.rotation;
        generator.speed.translation = 0.5f * generator.speed.translation + 0.5f / generator.stepDuration * returnOffset.translation;
      }
    }

    if(walkMode == WalkGenerator::speedMode)
    {
      forward = request.translation.x() * speedScale.translation.x();
      left = request.translation.y() * speedScale.translation.y();
      turn = request.rotation * speedScale.rotation;

      // Scale back values to try to ensure stability.
      ellipsoidClampWalk(maxSpeed, maxSpeedBackwards, forward, left, turn);

      // If switching direction, first stop if new speed is not reachable through acceleration
      if(lastForward * forward < 0.f && std::abs(lastForward) > std::abs(maxAcceleration.x()))
        forward = 0.f;

      // Limit acceleration and deceleration of forward movement
      if(lastForward > 0.f || (lastForward == 0.f && forward > 0.f))
        forward = lastForward + Rangef(-maxDeceleration.x(), maxAcceleration.x()).limit(forward - lastForward);
      else
        forward = lastForward + Rangef(-maxAcceleration.x(), maxDeceleration.x()).limit(forward - lastForward);

      // If switching direction, first stop if new speed is not reachable through acceleration
      if(lastLeft * left < 0.f && std::abs(lastLeft) > std::abs(maxAcceleration.y()))
        left = 0.f;

      // Limit acceleration and deceleration of sideways movement
      if(lastLeft > 0.f || (lastLeft == 0.f && left > 0.f))
        left = lastLeft + Rangef(-maxDeceleration.y(), maxAcceleration.y()).limit(left - lastLeft);
      else
        left = lastLeft + Rangef(-maxAcceleration.y(), maxDeceleration.y()).limit(left - lastLeft);

      generator.stepDuration = (baseWalkPeriod + sidewaysWalkPeriodIncreaseFactor * std::abs(left)) / 1000.f;

      // Consider in the speed that half of the step is returning to origin
      generator.speed = Pose2f(0.5f * turn / speedScale.rotation + 0.5f / generator.stepDuration * returnOffset.rotation,
                               0.5f * forward / speedScale.translation.x() + 0.5f / generator.stepDuration * returnOffset.translation.x(),
                               0.5f * left / speedScale.translation.y() + 0.5f / generator.stepDuration * returnOffset.translation.y());
    }
    else if(walkMode == WalkGenerator::stepSizeMode)
    {
      forward = request.translation.x();
      left = request.translation.y();
      turn = request.rotation;
      generator.stepDuration = (baseWalkPeriod + sidewaysWalkPeriodIncreaseFactor * std::abs(left)) / 1000.f;

      // Consider in the speed that half of the step is returning to origin
      generator.speed = Pose2f((0.5f * turn + 0.5f * returnOffset.rotation) / generator.stepDuration,
                               (0.5f * forward + 0.5f * returnOffset.translation.x()) / generator.stepDuration,
                               (0.5f * left + 0.5f * returnOffset.translation.y()) / generator.stepDuration);
    }

    lastForward = forward;
    lastLeft = left;

    if(walkMode == WalkGenerator::speedMode)
    {
      // 1.6 Walk Calibration
      // The definition of forward, left and turn is the actual distance/angle traveled in one second.
      // It is scaled down to the duration of a single step.
      forward *= generator.stepDuration;
      left *= generator.stepDuration;
      turn *= generator.stepDuration;
    }

    forward /= mmPerM; // in m/s
    left /= mmPerM; // in m/s

    // 5.1 Calculate the height to lift each swing foot
    maxFootHeight = baseFootLift / mmPerM + std::abs(forward) * footLiftIncreaseFactor.x() + std::abs(left) * footLiftIncreaseFactor.y();
  }

  // 2. Update timer
  generator.t += Constants::motionCycleTime;

  // 3. Determine Walk2014 Option
  if(walkState != standing && forward == 0 && left == 0 && turn == 0)
    walkState = stopping;
  else if(walkState != walking && (forward != 0 || left != 0 || turn != 0))
    walkState = starting;

  // 5. Determine walk variables throughout the walk step phase
  float foothL;
  float foothR;
  if(walkState == standing)
  {
    generator.stepDuration = generator.t = 0.f;
    foothL = foothR = 0;
  }
  else
  {
    // 5.3 Calculate intra-walkphase forward, left and turn at time-step dt
    if(generator.isLeftPhase)
      calcFootOffsets(generator, 1.f, forwardL0, forwardR0, forwardL, forwardR, leftL, leftR, foothL, foothR);
    else
      calcFootOffsets(generator, -1.f, forwardR0, forwardL0, forwardR, forwardL, leftR, leftL, foothR, foothL);

    // 5.4 Special conditions when priming the walk
    if(walkState == starting)
    {
      foothL *= footLiftFirstStepFactor; // reduce max lift due to short duration
      foothR *= footLiftFirstStepFactor;
      forwardL = forwardR = 0; // don't move on starting
      leftR = leftL = 0;
      turnRL = 0;
      generator.speed = Pose2f();
      if(left != 0.f) // make first real step in direction of movement
        generator.isLeftPhase = left < 0;
    }
  }

  // 6. Changing Support Foot. Note isLeftPhase means left foot is swing foot.
  // t>0.75*T tries to avoid bounce, especially when side-stepping
  // lastZMPL*ZMPL<0.0 indicates that support foot has changed
  // t>3*T tires to get out of "stuck" situations
  if((generator.t > supportSwitchPhaseRange.min * generator.stepDuration && theFootSupport.switched)
     || generator.t > supportSwitchPhaseRange.max * generator.stepDuration)
  {
    switchPhase = generator.t;
    maxFootHeight0 = maxFootHeight;
    weightShiftStatus = generator.isLeftPhase != (theFootSupport.support < 0) ? weightDidShift : weightDidNotShift;
    generator.isLeftPhase = theFootSupport.support < 0;

    if(weightShiftStatus == weightDidNotShift)
    {
      lastForward = lastLeft = 0;
      if(++weightShiftMisses > maxWeightShiftMisses)
      {
        ANNOTATION("Walk2014Generator", "Too many weight shift misses");
        weightShiftStatus = emergencyStep;
        generator.isLeftPhase = !generator.isLeftPhase;
      }
    }
    else
    {
      weightShiftMisses = 0;
      if(switchPhase > minSlowWeightShiftRatio * generator.stepDuration)
      {
        if(++slowWeightShifts > maxSlowWeightShifts)
        {
          ANNOTATION("Walk2014Generator", "Too many slow weight shifts");
          timeWhenSlowWeightShiftsDetected = theFrameInfo.time;
        }
      }
      else
        slowWeightShifts = 0;
    }

    if(walkState != standing)
    {
      // 6.1 Recover previous "left" swing angle
      swingAngle = generator.isLeftPhase ? leftL : leftR;

      // 6.2 Decide on timing of next walk step phase
      if(walkState != walking)
        walkState = static_cast<WalkState>((walkState + 1) & 3);

      // 6.3 reset step phase time
      generator.t = 0;

      // 6.4 backup values
      forwardL0 = forwardL;
      forwardR0 = forwardR;
      turnRL0 = turnRL;
    }
  } // end of changing support foot

  // 8. Odometry update for localization
  generator.odometryOffset = calcOdometryOffset(generator.isLeftPhase);

  // 9.1 Foot poses
  Pose3f leftFoot = Pose3f(0, theRobotDimensions.yHipOffset, 0)
                    .rotateX(-leftL)
                    .translate(-forwardL * mmPerM - torsoOffset, 0, -(walkHipHeight - theRobotDimensions.footHeight - foothL * mmPerM) / std::cos(leftL))
                    .rotateX(leftL).rotateZ(turnRL).translate(0, 0, -theRobotDimensions.footHeight);
  Pose3f rightFoot = Pose3f(0, -theRobotDimensions.yHipOffset, 0)
                     .rotateX(-leftR)
                     .translate(-forwardR * mmPerM - torsoOffset, 0, -(walkHipHeight - theRobotDimensions.footHeight - foothR * mmPerM) / std::cos(leftR))
                     .rotateX(leftR).rotateZ(-turnRL).translate(0, 0, -theRobotDimensions.footHeight);

  // 9.2 Walk kicks
  if(getKickFootOffset)
    (generator.isLeftPhase ? leftFoot : rightFoot).conc(getKickFootOffset(std::min(generator.t / generator.stepDuration, 1.f)));

  // 9.3 Inverse kinematics
  VERIFY(InverseKinematic::calcLegJoints(leftFoot, rightFoot, Vector2f::Zero(), generator.jointRequest, theRobotDimensions) || SystemCall::getMode() == SystemCall::logfileReplay);

  // 10. Set joint values and stiffness
  int stiffness = walkState == standing ? StiffnessData::useDefault : walkStiffness;
  for(uint8_t i = Joints::firstLegJoint; i < Joints::numOfJoints; ++i)
    generator.jointRequest.stiffnessData.stiffnesses[i] = stiffness;

  // 10.1 Arms
  for(int joint = Joints::lShoulderPitch; joint < Joints::firstLegJoint; ++joint)
  {
    generator.jointRequest.angles[joint] = 0_deg;
    generator.jointRequest.stiffnessData.stiffnesses[joint] = StiffnessData::useDefault;
  }

  // 5.5 "natural" arm swing while walking to counterbalance foot swing
  generator.jointRequest.angles[Joints::lShoulderPitch] = 90_deg - forwardL * armShoulderPitchFactor;
  generator.jointRequest.angles[Joints::rShoulderPitch] = 90_deg - forwardR * armShoulderPitchFactor;
  generator.jointRequest.angles[Joints::lShoulderRoll] = armShoulderRoll + std::abs(left) * armShoulderRollIncreaseFactor;
  generator.jointRequest.angles[Joints::rShoulderRoll] = -generator.jointRequest.angles[Joints::lShoulderRoll];

  // Compensate arm position's effect on COM by tilting torso
  compensateArmPosition(leftFoot, rightFoot, generator.jointRequest);

  // 7. Sagittal balance
  filteredGyroY = gyroLowPassRatio * filteredGyroY + (1.f - gyroLowPassRatio) * theInertialSensorData.gyro.y();
  Angle balanceAdjustment = walkState == standing ? 0.f : filteredGyroY *
                            (filteredGyroY > 0 ? gyroForwardBalanceFactor : gyroBackwardBalanceFactor); // adjust ankle tilt in proportion to filtered gryoY
  generator.jointRequest.angles[generator.isLeftPhase ? Joints::rAnklePitch : Joints::lAnklePitch] += balanceAdjustment;

  // Lateral balance
  filteredGyroX = gyroLowPassRatio * filteredGyroX + (1.f - gyroLowPassRatio) * theInertialSensorData.gyro.x();
  if(walkState == standing)
  {
    balanceAdjustment = filteredGyroX * gyroSidewaysBalanceFactor;
    generator.jointRequest.angles[Joints::lAnkleRoll] += balanceAdjustment;
    generator.jointRequest.angles[Joints::rAnkleRoll] += balanceAdjustment;
  }

  // Head can move freely
  generator.jointRequest.angles[Joints::headPitch] = generator.jointRequest.angles[Joints::headYaw] = JointAngles::ignore;
}

void Walk2014Generator::calcFootOffsets(WalkGenerator& generator,
                                        float swingFootSign, float forwardSwing0, float forwardSupport0,
                                        float& forwardSwing, float& forwardSupport, Angle& leftSwing, Angle& leftSupport,
                                        float& footHeightSwing, float& footHeightSupport)
{
  if(weightShiftStatus == weightDidShift)
  {
    // 5.3.1 forward: steps from (previous) -forward/2 to +forward/2, i.e. the target is forward/2
    forwardSupport = forwardSupport0 + (forward / 2.f - forwardSupport0) * Rangef::ZeroOneRange().limit(generator.t / generator.stepDuration);
    forwardSwing = forwardSwing0 + (-forward / 2.f - forwardSwing0) * parabolicStep(generator.t, generator.stepDuration); // swing-foot follow-through

    // 5.3.4 left: steps from left0 to +left in one step and from (previous) -left to 0 in the next
    float legLength = (walkHipHeight - theRobotDimensions.footHeight) / mmPerM;
    float left0 = std::tan(-swingAngle) * legLength;
    leftSupport = std::atan2(left0 + ((left * swingFootSign > 0 ? left : 0.f) - left0) * Rangef::ZeroOneRange().limit(generator.t / generator.stepDuration), legLength);
    leftSwing = -std::atan2(left0 + ((left * swingFootSign > 0 ? left : 0.f) - left0) * parabolicStep(generator.t, generator.stepDuration), legLength);

    // 5.3.5 turn
    turnRL = turnRL0 + ((turn * swingFootSign > 0 ? 1.f - insideTurnRatio : insideTurnRatio) * swingFootSign * turn - turnRL0) * Rangef::ZeroOneRange().limit(generator.t / generator.stepDuration);
  }

  // 5.3.6 determine how high to lift the swing foot off the ground
  footHeightSwing = maxFootHeight * parabolicReturn(generator.t / generator.stepDuration); // lift swing foot
  footHeightSupport = maxFootHeight0 * parabolicReturn((switchPhase + generator.t) / generator.stepDuration); // return support foot to 0 if it was still lifted
}

Pose2f Walk2014Generator::calcOdometryOffset(bool isLeftSwingFoot)
{
  // Work out incremental forward, left, and turn values for next time step
  Pose2f offset((turnRL - prevTurn) * (isLeftSwingFoot ? 1.f : -1.f) * odometryScale.rotation,
                (isLeftSwingFoot ? forwardR - prevForwardR : forwardL - prevForwardL) * mmPerM * odometryScale.translation.x(),
                (walkHipHeight - theRobotDimensions.footHeight) * (isLeftSwingFoot ? std::tan(leftR) - std::tan(prevLeftR) : std::tan(leftL) - std::tan(prevLeftL)) * odometryScale.translation.y());

  // backup values for next computation
  prevTurn = turnRL;
  prevLeftL = leftL;
  prevLeftR = leftR;
  prevForwardL = forwardL;
  prevForwardR = forwardR;

  return offset;
}

float Walk2014Generator::calcWalkVolume(float forward, float left, float turn) const
{
  return std::pow(std::pow(forward, walkVolumeTranslationExponent) + std::pow(left, walkVolumeTranslationExponent),
                  (walkVolumeRotationExponent / walkVolumeTranslationExponent))
         + std::pow(turn, walkVolumeRotationExponent);
}

bool Walk2014Generator::ellipsoidClampWalk(const Pose2f& maxSpeed, float maxBackwardsSpeed, float& forward, float& left, Angle& turn) const
{
  // Values in range [-1..1]
  float forwardAmount = forward / (forward >= 0.f ? maxSpeed.translation.x() : maxSpeedBackwards);
  float leftAmount = left / maxSpeed.translation.y();
  float turnAmount = turn / maxSpeed.rotation;
  float factor = std::max(std::max(std::abs(forwardAmount), std::abs(leftAmount)), std::abs(turnAmount));
  bool clamp = factor > 1.f;
  if(clamp)
  {
    forwardAmount /= factor;
    leftAmount /= factor;
    turnAmount /= factor;
  }

  // see if the point we are given is already inside the allowed walk params volume
  if(calcWalkVolume(std::abs(forwardAmount), std::abs(leftAmount), std::abs(turnAmount)) > 1.f)
  {
    clamp = true;
    float scale = 0.5f;
    float high = 1.f;
    float low = 0.f;

    // This is basically a binary search to find the point on the surface.
    for(unsigned i = 0; i < 10; ++i)
    {
      // give priority to turn. keep it the same
      if(calcWalkVolume(std::abs(forwardAmount) * scale, std::abs(leftAmount) * scale, std::abs(turnAmount)) > 1.f)
        high = scale;
      else
        low = scale;
      scale = (low + high) / 2.f;
    }

    forwardAmount *= scale;
    leftAmount *= scale;
  }
  forward = (forward >= 0.f ? maxSpeed.translation.x() : maxSpeedBackwards) * forwardAmount;
  left = maxSpeed.translation.y() * leftAmount;
  turn = maxSpeed.rotation * turnAmount;
  return clamp;
}

float Walk2014Generator::parabolicReturn(float f) const
{
  Rangef::ZeroOneRange().clamp(f);

  if(f < 0.25f)
    return 8.f * f * f;
  else if(f < 0.75f)
  {
    float x = f - 0.5f;
    return 1.f - 8.f * x * x;
  }
  else
  {
    float x = 1.f - f;
    return 8.f * x * x;
  }
}

float Walk2014Generator::parabolicStep(float time, float period) const
{
  float timeFraction = Rangef::ZeroOneRange().limit(time / period);
  if(timeFraction < 0.5f)
    return 2.f * timeFraction * timeFraction;
  else
    return 4.f * timeFraction - 2.f * timeFraction * timeFraction - 1.f;
}

void Walk2014Generator::compensateArmPosition(const Pose3f& leftFoot, const Pose3f& rightFoot, JointRequest& jointRequest) const
{
  JointRequest temp = jointRequest;
  for(int joint = 0; joint < Joints::firstArmJoint; ++joint)
    temp.angles[joint] = theJointRequest.angles[joint] != JointRequest::off
                         ? theJointRequest.angles[joint] : theJointAngles.angles[joint];
  RobotModel withWalkGeneratorArms(temp, theRobotDimensions, theMassCalibration);
  for(int joint = Joints::firstArmJoint; joint < Joints::firstLegJoint; ++joint)
    temp.angles[joint] = theJointRequest.angles[joint] != JointRequest::off
                         ? theJointRequest.angles[joint] : theJointAngles.angles[joint];
  RobotModel balanced(temp, theRobotDimensions, theMassCalibration);

  float torsoTilt = 0.f;
  for(int i = 0; i < numOfComIterations; ++i)
  {
    Quaternionf torsoRotation = Rotation::aroundY(-torsoTilt);
    VERIFY(InverseKinematic::calcLegJoints(leftFoot, rightFoot, torsoRotation, temp, theRobotDimensions) || SystemCall::getMode() == SystemCall::logfileReplay);
    ForwardKinematic::calculateLegChain(Legs::left, temp, theRobotDimensions, balanced.limbs);
    ForwardKinematic::calculateLegChain(Legs::right, temp, theRobotDimensions, balanced.limbs);
    balanced.updateCenterOfMass(theMassCalibration);
    Vector3f balancedCom = torsoRotation * balanced.centerOfMass;
    torsoTilt += (balancedCom.x() - withWalkGeneratorArms.centerOfMass.x()) * pComFactor;
  }
  float tiltIncrease = mmPerM * (forward >= 0 ? forward * comTiltForwardIncreaseFactor : forward * comTiltBackwardIncreaseFactor);
  VERIFY(InverseKinematic::calcLegJoints(leftFoot, rightFoot, Vector2f(0.f, tiltIncrease - torsoTilt * comTiltFactor), jointRequest, theRobotDimensions) || SystemCall::getMode() == SystemCall::logfileReplay);
}
