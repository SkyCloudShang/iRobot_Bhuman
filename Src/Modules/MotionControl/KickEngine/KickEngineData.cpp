/*
 * @file KickEngineData.cpp
 * This file implements a module that creates motions.
 * @author <A href="mailto:judy@tzi.de">Judith Müller</A>
 */

#include <cstring>

#include "KickEngineData.h"
#include "Representations/Configuration/JointLimits.h"
#include "Tools/Debugging/DebugDrawings.h"
#include "Tools/Debugging/Modify.h"
#include "Tools/Math/Pose3f.h"
#include "Tools/Math/RotationMatrix.h"
#include "Tools/Motion/InverseKinematic.h"
#include "Platform/SystemCall.h"

bool KickEngineData::getMotionIDByName(const MotionRequest& motionRequest, const std::vector<KickEngineParameters>& params)
{
  motionID = -1;

  for(unsigned int i = 0; i < params.size(); ++i)
    if(motionRequest.kickRequest.getKickMotionFromName(&params[i].name[0]) == motionRequest.kickRequest.kickMotionType)
    {
      motionID = i;
      return true;
    }

  return false;
}

void KickEngineData::calculateOrigins(const KickRequest& kr, const JointAngles& ja, const TorsoMatrix& to, const RobotDimensions& robotDimensions)
{
  if(!wasActive)
  {
    if(!kr.mirror)
    {
      origins[Phase::leftFootTra] = robotModel.limbs[Limbs::footLeft].rotation.inverse() * robotModel.soleLeft.translation;
      origins[Phase::rightFootTra] = robotModel.limbs[Limbs::footRight].rotation.inverse() * robotModel.soleRight.translation;

      origins[Phase::leftArmTra] = robotModel.limbs[Limbs::foreArmLeft].translation;
      origins[Phase::rightArmTra] = robotModel.limbs[Limbs::foreArmRight].translation;

      origins[Phase::leftFootRot] = Vector3f(0, 0, robotModel.limbs[Limbs::footLeft].rotation.getZAngle());
      origins[Phase::rightFootRot] = Vector3f(0, 0, robotModel.limbs[Limbs::footRight].rotation.getZAngle());
      origins[Phase::leftHandRot] = Vector3f(ja.angles[Joints::lElbowYaw], ja.angles[Joints::lElbowRoll], ja.angles[Joints::lWristYaw]);
      origins[Phase::rightHandRot] = Vector3f(ja.angles[Joints::rElbowYaw], ja.angles[Joints::rElbowRoll], ja.angles[Joints::rWristYaw]);
    }
    else
    {
      origins[Phase::leftFootTra] = robotModel.limbs[Limbs::footRight].rotation.inverse() * robotModel.soleRight.translation;
      origins[Phase::rightFootTra] = robotModel.limbs[Limbs::footLeft].rotation.inverse() * robotModel.soleLeft.translation;
      origins[Phase::leftArmTra] = robotModel.limbs[Limbs::foreArmRight].translation;
      origins[Phase::rightArmTra] = robotModel.limbs[Limbs::foreArmLeft].translation;

      origins[Phase::rightHandRot] = Vector3f(-ja.angles[Joints::lElbowYaw], -ja.angles[Joints::lElbowRoll], -ja.angles[Joints::lWristYaw]);
      origins[Phase::leftHandRot] = Vector3f(-ja.angles[Joints::rElbowYaw], -ja.angles[Joints::rElbowRoll], -ja.angles[Joints::rWristYaw]);

      origins[Phase::leftFootRot] = Vector3f(0, 0, ja.angles[Joints::rHipYawPitch]);//robotModel.limbs[Limbs::footLeft].rotation.getPackedAngleAxisFaulty();
      origins[Phase::rightFootRot] = Vector3f(0, 0, ja.angles[Joints::lHipYawPitch]);// robotModel.limbs[Limbs::footRight].rotation.getPackedAngleAxisFaulty();

      origins[Phase::leftFootTra].y() *= -1;
      origins[Phase::rightFootTra].y() *= -1;
      origins[Phase::leftArmTra].y() *= -1;
      origins[Phase::rightArmTra].y() *= -1;
    }
  }
  else
  {
    for(int i = 0; i < Phase::numOfLimbs; ++i)
      origins[i] = currentParameters.phaseParameters[currentParameters.numberOfPhases - 1].controlPoints[i][2];
  }
}

void KickEngineData::calcPhaseState()
{
  phase = static_cast<float>(timeSinceTimeStamp) / static_cast<float>(currentParameters.phaseParameters[phaseNumber].duration);
}

bool KickEngineData::checkPhaseTime(const FrameInfo& frame, const JointAngles& ja, const TorsoMatrix& torsoMatrix)
{
  timeSinceTimeStamp = frame.getTimeSince(timeStamp);

  if(motionID < 0)
    return false;

  if(phaseNumber < currentParameters.numberOfPhases)
  {
    if(static_cast<unsigned int>(timeSinceTimeStamp) > currentParameters.phaseParameters[phaseNumber].duration)
    {
      phaseNumber++;
      timeStamp = frame.time;
      timeSinceTimeStamp = frame.getTimeSince(timeStamp);
      if(phaseNumber < currentParameters.numberOfPhases)
      {
        if(currentKickRequest.armsBackFix)
        {
          if(lElbowFront)
          {
            Vector3f inverse = currentParameters.phaseParameters[phaseNumber].controlPoints[Phase::leftHandRot][2];
            inverse.x() *= -1.f;
            addDynPoint(DynPoint(Phase::leftHandRot, phaseNumber, inverse), torsoMatrix);
          }
          if(rElbowFront)
          {
            Vector3f inverse = currentParameters.phaseParameters[phaseNumber].controlPoints[Phase::rightHandRot][2];
            inverse.x() *= -1.f;
            addDynPoint(DynPoint(Phase::rightHandRot, phaseNumber, inverse), torsoMatrix);
          }
        }

        for(unsigned int i = 0; i < currentKickRequest.dynPoints.size(); i++)
          if(currentKickRequest.dynPoints[i].phaseNumber == phaseNumber)
            addDynPoint(currentKickRequest.dynPoints[i], torsoMatrix);
      }
    }
  }
  else if(currentParameters.loop && phaseNumber == currentParameters.numberOfPhases)
  {
    phaseNumber = 0;
    //calculateOrigins(currentKickRequest, ja, torsoMatrix);
    currentParameters.initFirstPhaseLoop(origins, currentParameters.phaseParameters[currentParameters.numberOfPhases - 1].comTra[2], Vector2f(ja.angles[Joints::headPitch], ja.angles[Joints::headYaw]));

    for(unsigned int i = 0; i < currentKickRequest.dynPoints.size(); i++)
      if(currentKickRequest.dynPoints[i].phaseNumber == phaseNumber)
        addDynPoint(currentKickRequest.dynPoints[i], torsoMatrix);
  }

  return phaseNumber < currentParameters.numberOfPhases;
}

bool KickEngineData::calcJoints(JointRequest& jointRequest, const RobotDimensions& rd)
{
  //Calculate Legs
  if(motionID > -1)
  {
    if(!currentParameters.ignoreHead)
    {
      jointRequest.angles[Joints::headPitch] = head.x();
      jointRequest.angles[Joints::headYaw] = head.y();
    }
    else
    {
      jointRequest.angles[Joints::headYaw] = JointAngles::ignore;
      jointRequest.angles[Joints::headPitch] = JointAngles::ignore;
    }

    calcLegJoints(Joints::lHipYawPitch, jointRequest, rd);
    calcLegJoints(Joints::rHipYawPitch, jointRequest, rd);

    simpleCalcArmJoints(Joints::lShoulderPitch, jointRequest, rd, positions[Phase::leftArmTra], positions[Phase::leftHandRot]);
    simpleCalcArmJoints(Joints::rShoulderPitch, jointRequest, rd, positions[Phase::rightArmTra], positions[Phase::rightHandRot]);

    return true;
  }
  else //just set the angles from init
  {
    for(int i = Joints::lShoulderPitch; i < Joints::numOfJoints; ++i)
      jointRequest.angles[i] = lastBalancedJointRequest.angles[i];

    return false;
  }
}

void KickEngineData::calcLegJoints(const Joints::Joint& joint, JointRequest& jointRequest, const RobotDimensions& theRobotDimensions)
{
  const int sign = joint == Joints::lHipYawPitch ? 1 : -1;

  const Vector3f& footPos = (sign > 0) ? positions[Phase::leftFootTra] : positions[Phase::rightFootTra];
  const Vector3f& footRotAng = (sign > 0) ? positions[Phase::leftFootRot] : positions[Phase::rightFootRot];

  const RotationMatrix rotateBodyTilt = RotationMatrix::aroundX(bodyAngle.x()).rotateY(bodyAngle.y());
  Vector3f anklePos = rotateBodyTilt * (footPos - Vector3f(0.f, 0, -theRobotDimensions.footHeight));
  anklePos -= Vector3f(0.f, sign * (theRobotDimensions.yHipOffset), 0);

  //const RotationMatrix zRot = RotationMatrix::aroundZ(footRotAng.z()).rotateX(sign * pi_4);
  //anklePos = zRot * anklePos;
  const float leg0 = 0;//std::atan2(-anklePos.x(), anklePos.y());
  const float diagonal = anklePos.norm();

  // upperLegLength, lowerLegLength, and diagonal form a triangle, use cosine theorem
  float a1 = (theRobotDimensions.upperLegLength * theRobotDimensions.upperLegLength -
              theRobotDimensions.lowerLegLength * theRobotDimensions.lowerLegLength + diagonal * diagonal) /
             (2 * theRobotDimensions.upperLegLength * diagonal);
  //if(std::abs(a1) > 1.f) OUTPUT_TEXT("clipped a1");
  a1 = std::abs(a1) > 1.f ? 0.f : std::acos(a1);

  float a2 = (theRobotDimensions.upperLegLength * theRobotDimensions.upperLegLength +
              theRobotDimensions.lowerLegLength * theRobotDimensions.lowerLegLength - diagonal * diagonal) /
             (2 * theRobotDimensions.upperLegLength * theRobotDimensions.lowerLegLength);
  //if(std::abs(a2) > 1.f) OUTPUT_TEXT("clipped a2");
  a2 = std::abs(a2) > 1.f ? pi : std::acos(a2);

  const float leg2 = -a1 - std::atan2(anklePos.x(), Vector2f(anklePos.y(), anklePos.z()).norm() * -sgn(anklePos.z()));
  const float leg1 = anklePos.z() == 0.0f ? 0.0f : (std::atan(anklePos.y() / -anklePos.z()));
  const float leg3 = pi - a2;

  const RotationMatrix rotateBecauseOfHip = RotationMatrix::aroundZ(0).rotateX(bodyAngle.x()).rotateY(bodyAngle.y());
  //calculate inverse foot rotation so that they are flat to the ground
  RotationMatrix footRot = RotationMatrix::aroundX(leg1).rotateY(leg2 + leg3);
  footRot = footRot.inverse() /* * zRot*/ * rotateBecauseOfHip;

  //and add additonal foot rotation (which is probably not flat to the ground)
  const float leg4 = std::atan2(footRot(0, 2), footRot(2, 2)) + footRotAng.y();
  const float leg5 = std::asin(-footRot(1, 2)) + footRotAng.x() ;

  jointRequest.angles[joint] = leg0;
  jointRequest.angles[joint + 1] = (/*-pi_4 * sign + */leg1);
  jointRequest.angles[joint + 2] = leg2;
  jointRequest.angles[joint + 3] = leg3;
  jointRequest.angles[joint + 4] = leg4;
  jointRequest.angles[joint + 5] = leg5;
}

void KickEngineData::simpleCalcArmJoints(const Joints::Joint& joint, JointRequest& jointRequest, const RobotDimensions& theRobotDimensions,
    const Vector3f& armPos, const Vector3f& handRotAng)
{
  float sign = joint == Joints::lShoulderPitch ? 1.f : -1.f;

  const Vector3f target = armPos - Vector3f(theRobotDimensions.armOffset.x(), theRobotDimensions.armOffset.y() * sign, theRobotDimensions.armOffset.z());

  jointRequest.angles[joint + 0] = std::atan2(target.z(), target.x());
  jointRequest.angles[joint + 1] = std::atan2(target.y() * sign, std::sqrt(sqr(target.x()) + sqr(target.z())));

  float length2ElbowJoint = Vector3f(theRobotDimensions.upperArmLength, theRobotDimensions.yOffsetElbowToShoulder, 0.f).norm();
  float angle = std::asin(theRobotDimensions.upperArmLength / length2ElbowJoint);

  Pose3f elbow;
  elbow.rotateY(-jointRequest.angles[joint + 0])
  .rotateZ(jointRequest.angles[joint + 1])
  .translate(length2ElbowJoint, 0.f, 0.f)
  .rotateZ(-angle)
  .translate(theRobotDimensions.yOffsetElbowToShoulder, 0.f, 0.f);

  jointRequest.angles[joint + 0] = std::atan2(elbow.translation.z(), elbow.translation.x());
  jointRequest.angles[joint + 1] = std::atan2(elbow.translation.y(), std::sqrt(sqr(elbow.translation.x()) + sqr(elbow.translation.z())));
  jointRequest.angles[joint + 0] = (jointRequest.angles[joint + 0] < pi) ? jointRequest.angles[joint + 0] : 0_deg;  //clip special

  jointRequest.angles[joint + 0] *= -1.f;
  jointRequest.angles[joint + 1] *= sign;
  jointRequest.angles[joint + 2] = handRotAng.x();
  jointRequest.angles[joint + 3] = handRotAng.y();
  jointRequest.angles[joint + 4] = handRotAng.z();
  jointRequest.angles[joint + 5] = 0.f;
}

void KickEngineData::balanceCOM(JointRequest& joints, const RobotDimensions& rd, const MassCalibration& mc)
{
  const Pose3f& torso = toLeftSupport ? comRobotModel.limbs[Limbs::footLeft] : comRobotModel.limbs[Limbs::footRight];
  comRobotModel.setJointData(joints, rd, mc);
  const Vector3f com = torso.rotation.inverse() * comRobotModel.centerOfMass;

  actualDiff = com - ref;

  balanceSum += actualDiff.head<2>();

  float height = comRobotModel.centerOfMass.z() - ref.z();

  const Vector2f balance(
    currentParameters.kpy * (actualDiff.x()) + currentParameters.kiy * balanceSum.x() + currentParameters.kdy * ((actualDiff.x() - lastCom.x()) / cycletime),
    -currentParameters.kpx * (actualDiff.y()) + -currentParameters.kix * balanceSum.y() + -currentParameters.kdx * ((actualDiff.y() - lastCom.y()) / cycletime));

  if(height != 0.f)
  {
    bodyAngle.x() = (balance.y() != 0) ? std::atan2((balance.y()), height) : 0;
    bodyAngle.y() = (balance.x() != 0) ? std::atan2((balance.x()), height) : 0;
  }

  lastCom = actualDiff;
}

void KickEngineData::mirrorIfNecessary(JointRequest& joints)
{
  if(currentKickRequest.mirror)
  {
    const JointRequest old = joints;
    for(int i = 0; i < Joints::numOfJoints; ++i)
    {
      if(i == Joints::headPitch)
        continue;

      joints.angles[i] = old.mirror(static_cast<Joints::Joint>(i));
    }
  }
}

void KickEngineData::addGyroBalance(JointRequest& jointRequest, const JointLimits& jointLimits, const InertialData& id, const float& ratio)
{
  if(id.gyro.y() != 0 && id.gyro.x() != 0 && !willBeLeft)
  {
    //Predict next gyrodata
    gyro = id.gyro.head<2>().cast<float>() * 0.3f + 0.7f * gyro;

    //some clipping
    for(int i = Joints::lHipYawPitch; i < Joints::numOfJoints; i++)
    {
      jointLimits.limits[i].clamp(jointRequest.angles[i]);
    }

    const JointRequest balancedJointRequest = jointRequest;

    //calculate the commandedVelocity
    float commandedVelocity[4];
    //y-velocity if left leg is support
    commandedVelocity[0] = (balancedJointRequest.angles[Joints::lHipPitch] - lastBalancedJointRequest.angles[Joints::lHipPitch]) / cycletime;
    //y-velocity if right leg is support
    commandedVelocity[1] = (balancedJointRequest.angles[Joints::rHipPitch] - lastBalancedJointRequest.angles[Joints::rHipPitch]) / cycletime;
    //x-velcocity if left leg is support
    commandedVelocity[2] = (balancedJointRequest.angles[Joints::lHipRoll] - lastBalancedJointRequest.angles[Joints::lHipRoll]) / cycletime;
    //x-velocity if right leg is support
    commandedVelocity[3] = (balancedJointRequest.angles[Joints::rHipRoll] - lastBalancedJointRequest.angles[Joints::rHipRoll]) / cycletime;

    //calculate disturbance from meseaured velocity and commanded velocity
    // y-velocity if left leg is support
    float gyroVelyLeft = (gyro.y() + commandedVelocity[0] - lastGyroLeft.y()) / cycletime;
    lastGyroLeft.y() = gyro.y() + commandedVelocity[0];
    //y-velocity if right leg is support
    float gyroVelyRight = (gyro.y() + commandedVelocity[1] - lastGyroRight.y()) / cycletime;
    lastGyroRight.y() = gyro.y() + commandedVelocity[1];
    //x-velocity if left leg is support
    float gyroVelxLeft = (gyro.x() + commandedVelocity[2] - lastGyroLeft.x()) / cycletime;
    lastGyroLeft.x() = gyro.x() + commandedVelocity[2];
    //x-velocity if right leg is support
    float gyroVelxRight = (gyro.x() + commandedVelocity[3] - lastGyroRight.x()) / cycletime;
    lastGyroRight.x() = gyro.x() + commandedVelocity[3];

    //calculate control variable with PID-Control
    float calcVelocity[4];
    //y if left supprt
    calcVelocity[0] = -gyroP.y() * (gyro.y() + commandedVelocity[0]) - gyroD.y() * gyroVelyLeft - gyroI.y() * (gyroErrorLeft.y());
    //y if right support
    calcVelocity[1] = -gyroP.y() * (gyro.y() + commandedVelocity[1]) - gyroD.y() * gyroVelyRight - gyroI.y() * (gyroErrorRight.y());
    //x if left support
    calcVelocity[2] = -gyroP.x() * (gyro.x() + commandedVelocity[2]) + gyroD.x() * gyroVelxLeft + gyroI.x() * gyroErrorLeft.x();
    //x if right support
    calcVelocity[3] = -gyroP.x() * (gyro.x() - commandedVelocity[3]) + gyroD.x() * gyroVelxRight + gyroI.x() * gyroErrorRight.x();

    bool supp = (currentKickRequest.mirror) ? !toLeftSupport : toLeftSupport;

    if(supp)  //last support Leg was left
    {
      //y
      jointRequest.angles[Joints::rHipPitch] += calcVelocity[0] * cycletime * ratio;
      jointRequest.angles[Joints::lHipPitch] += calcVelocity[0] * cycletime * ratio;
      jointRequest.angles[Joints::lAnklePitch] += calcVelocity[0] * cycletime * ratio;
      jointRequest.angles[Joints::rAnklePitch] += calcVelocity[0] * cycletime * ratio;
      //x
      jointRequest.angles[Joints::lHipRoll] += calcVelocity[2] * cycletime * ratio;
      jointRequest.angles[Joints::rHipRoll] += calcVelocity[2] * cycletime * ratio;
      jointRequest.angles[Joints::lAnkleRoll] -= calcVelocity[2] * cycletime * ratio;
      jointRequest.angles[Joints::rAnkleRoll] -= calcVelocity[2] * cycletime * ratio;
    }
    else //if(toRightSupport)
    {
      //y
      jointRequest.angles[Joints::rHipPitch] += calcVelocity[1] * cycletime * ratio;
      jointRequest.angles[Joints::lHipPitch] += calcVelocity[1] * cycletime * ratio;
      jointRequest.angles[Joints::lAnklePitch] += calcVelocity[1] * cycletime * ratio;
      jointRequest.angles[Joints::rAnklePitch] += calcVelocity[1] * cycletime * ratio;

      //x
      jointRequest.angles[Joints::lHipRoll] += calcVelocity[3] * cycletime * ratio;
      jointRequest.angles[Joints::rHipRoll] += calcVelocity[3] * cycletime * ratio;
      jointRequest.angles[Joints::lAnkleRoll] -= calcVelocity[3] * cycletime * ratio;
      jointRequest.angles[Joints::rAnkleRoll] -= calcVelocity[3] * cycletime * ratio;
    }
    gyroErrorLeft += lastGyroLeft;
    gyroErrorRight += lastGyroRight;
    lastBalancedJointRequest = balancedJointRequest;
  }
}

void KickEngineData::addDynPoint(const DynPoint& dynPoint, const TorsoMatrix& torsoMatrix)
{
  Vector3f d = dynPoint.translation;
  if(dynPoint.limb == Phase::leftFootTra || dynPoint.limb == Phase::rightFootTra)
    transferDynPoint(d, torsoMatrix);

  const int phaseNumber = dynPoint.phaseNumber;
  const int limb = dynPoint.limb;

  if(dynPoint.duration > 0)
    currentParameters.phaseParameters[phaseNumber].duration = dynPoint.duration;

  currentParameters.phaseParameters[phaseNumber].odometryOffset = dynPoint.odometryOffset;

  const Vector3f& cubePoint = currentParameters.phaseParameters[phaseNumber].controlPoints[limb][2];
  const Vector3f diff = cubePoint - d;

  currentParameters.phaseParameters[phaseNumber].controlPoints[limb][2] -= diff;
  currentParameters.phaseParameters[phaseNumber].controlPoints[limb][1] -= diff;

  const Vector3f point1 = currentParameters.phaseParameters[phaseNumber].controlPoints[limb][2] -
                          currentParameters.phaseParameters[phaseNumber].controlPoints[limb][1];

  const Vector3f absAngle(std::atan2(point1.y(), point1.z()),
                          std::atan2(point1.x(), point1.z()),
                          std::atan2(point1.x(), point1.y()));

  const RotationMatrix rot = RotationMatrix::aroundX(dynPoint.angle.x() - absAngle.x())
                             .rotateY(dynPoint.angle.y() - absAngle.y())
                             .rotateZ(dynPoint.angle.z() - absAngle.z());

  currentParameters.phaseParameters[phaseNumber].controlPoints[limb][1] =
    (rot * point1) + currentParameters.phaseParameters[phaseNumber].controlPoints[limb][1];

  if(phaseNumber < currentParameters.numberOfPhases - 1)
  {
    currentParameters.phaseParameters[phaseNumber + 1].controlPoints[limb][0] =
      currentParameters.phaseParameters[phaseNumber].controlPoints[limb][2] -
      currentParameters.phaseParameters[phaseNumber].controlPoints[limb][1];

    float factor = static_cast<float>(currentParameters.phaseParameters[phaseNumber + 1].duration) /
                   static_cast<float>(currentParameters.phaseParameters[phaseNumber].duration);
    currentParameters.phaseParameters[phaseNumber + 1].controlPoints[limb][0] *= factor;

    currentParameters.phaseParameters[phaseNumber + 1].controlPoints[limb][0] +=
      currentParameters.phaseParameters[phaseNumber].controlPoints[limb][2];
  }
  if(phaseNumber == 0 && limb == Phase::rightFootRot)
    SystemCall::playSound("greetings.wav");
}

void KickEngineData::transferDynPoint(Vector3f& d, const TorsoMatrix& torsoMatrix)
{
  const Pose3f& left = positions[Phase::leftFootTra];
  const Pose3f& right = positions[Phase::rightFootTra];

  const Vector3f& foot = toLeftSupport ? left.translation : right.translation;

  Pose3f left2 = torsoMatrix.rotation * robotModel.soleLeft;
  Pose3f right2 = torsoMatrix.rotation * robotModel.soleRight;

  if(currentKickRequest.mirror)
  {
    Pose3f temp = left2;
    left2 = right2;
    right2 = temp;
    left2.translation.y() *= -1;
    right2.translation.y() *= -1;
  }

  const Vector3f& foot2 = toLeftSupport ? left2.translation : right2.translation;
  d += foot - foot2;
}

void KickEngineData::ModifyData(const KickRequest& br, JointRequest& kickEngineOutput, std::vector<KickEngineParameters>& params)
{
  auto& p = params.back();
  MODIFY("module:KickEngine:newKickMotion", p);
  strcpy(p.name, "newKick");

  MODIFY("module:KickEngine:px", gyroP.x());
  MODIFY("module:KickEngine:dx", gyroD.x());
  MODIFY("module:KickEngine:ix", gyroI.x());
  MODIFY("module:KickEngine:py", gyroP.y());
  MODIFY("module:KickEngine:dy", gyroD.y());
  MODIFY("module:KickEngine:iy", gyroI.y());

  MODIFY("module:KickEngine:formMode", formMode);
  MODIFY("module:KickEngine:lFootTraOff", limbOff[Phase::leftFootTra]);
  MODIFY("module:KickEngine:rFootTraOff", limbOff[Phase::rightFootTra]);
  MODIFY("module:KickEngine:lFootRotOff", limbOff[Phase::leftFootRot]);
  MODIFY("module:KickEngine:rFootRotOff", limbOff[Phase::rightFootRot]);
  MODIFY("module:KickEngine:lHandTraOff", limbOff[Phase::leftArmTra]);
  MODIFY("module:KickEngine:rHandTraOff", limbOff[Phase::rightArmTra]);
  MODIFY("module:KickEngine:lHandRotOff", limbOff[Phase::leftHandRot]);
  MODIFY("module:KickEngine:rHandRotOff", limbOff[Phase::rightHandRot]);

  //Plot com stabilizing
  PLOT("module:KickEngine:comy", robotModel.centerOfMass.y());
  PLOT("module:KickEngine:diffy", actualDiff.y());
  PLOT("module:KickEngine:refy", ref.y());

  PLOT("module:KickEngine:comx", robotModel.centerOfMass.x());
  PLOT("module:KickEngine:diffx", actualDiff.x());
  PLOT("module:KickEngine:refx", ref.x());

  PLOT("module:KickEngine:lastdiffy", toDegrees(lastBody.y()));
  PLOT("module:KickEngine:bodyErrory", toDegrees(bodyError.y()));

  for(int i = 0; i < Phase::numOfLimbs; i++)
  {
    const int stiffness = limbOff[i] ? 0 : 100;

    switch(static_cast<Phase::Limb>(i))
    {
      case Phase::leftFootTra:
        kickEngineOutput.stiffnessData.stiffnesses[Joints::lHipRoll] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::lHipPitch] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::lKneePitch] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::lAnklePitch] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::lAnkleRoll] = stiffness;
        break;
      case Phase::rightFootTra:
        kickEngineOutput.stiffnessData.stiffnesses[Joints::rHipRoll] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::rHipPitch] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::rKneePitch] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::rAnklePitch] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::rAnkleRoll] = stiffness;
        break;
      case Phase::leftFootRot:
        kickEngineOutput.stiffnessData.stiffnesses[Joints::lAnklePitch] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::lAnkleRoll] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::rHipYawPitch] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::lHipYawPitch] = stiffness;
        break;
      case Phase::rightFootRot:
        kickEngineOutput.stiffnessData.stiffnesses[Joints::rAnklePitch] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::rAnkleRoll] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::rHipYawPitch] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::lHipYawPitch] = stiffness;
        break;
      case Phase::leftArmTra:
        kickEngineOutput.stiffnessData.stiffnesses[Joints::lShoulderPitch] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::lShoulderRoll] = stiffness;
        break;
      case Phase::rightArmTra:
        kickEngineOutput.stiffnessData.stiffnesses[Joints::rShoulderPitch] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::rShoulderRoll] = stiffness;
        break;
      case Phase::leftHandRot:
        kickEngineOutput.stiffnessData.stiffnesses[Joints::lElbowRoll] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::lElbowYaw] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::lWristYaw] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::lHand] = stiffness;
        break;
      case Phase::rightHandRot:
        kickEngineOutput.stiffnessData.stiffnesses[Joints::rElbowRoll] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::rElbowYaw] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::rWristYaw] = stiffness;
        kickEngineOutput.stiffnessData.stiffnesses[Joints::rHand] = stiffness;
        break;
    }
  }
}

void KickEngineData::setCycleTime(float time)
{
  cycletime = time;
}

void KickEngineData::calcPositions()
{
  for(int i = 0; i < Phase::numOfLimbs; ++i)
    positions[i] = currentParameters.getPosition(phase, phaseNumber, i);

  if(!currentParameters.ignoreHead)
    head = currentParameters.getHeadRefPosition(phase, phaseNumber);

  ref << currentParameters.getComRefPosition(phase, phaseNumber),
      (toLeftSupport) ? positions[Phase::leftFootTra].z() : positions[Phase::rightFootTra].z();
}

void KickEngineData::setRobotModel(const RobotModel& rm)
{
  robotModel = rm;
}

void KickEngineData::setCurrentKickRequest(const MotionRequest& mr)
{
  currentKickRequest = mr.kickRequest;
}

void KickEngineData::setExecutedKickRequest(KickRequest& br)
{
  br.mirror = currentKickRequest.mirror;
  br.armsBackFix = currentKickRequest.armsBackFix;
  br.kickMotionType = currentKickRequest.kickMotionType;
}

void KickEngineData::initData(const FrameInfo& frame, const MotionRequest& mr, std::vector<KickEngineParameters>& params,
                              const JointAngles& ja, const TorsoMatrix& torsoMatrix, JointRequest& jointRequest, const RobotDimensions& rd, const MassCalibration& mc)
{
  if(getMotionIDByName(mr, params))
  {
    phase = 0.f;
    phaseNumber = 0;
    timeStamp = frame.time;
    currentParameters = params[motionID];
    toLeftSupport = currentParameters.standLeft;

    ref = Vector3f::Zero();
    actualDiff = ref;
    calculateOrigins(mr.kickRequest, ja, torsoMatrix, rd);
    currentParameters.initFirstPhase(origins, Vector2f(ja.angles[Joints::headPitch], (mr.kickRequest.mirror) ? -ja.angles[Joints::headYaw] : ja.angles[Joints::headYaw]));
    calcPositions();

    float angleY = (toLeftSupport) ? -robotModel.limbs[Limbs::footLeft].rotation.inverse().getYAngle() : -robotModel.limbs[Limbs::footRight].rotation.inverse().getYAngle();
    float angleX = (toLeftSupport) ? -robotModel.limbs[Limbs::footLeft].rotation.inverse().getXAngle() : -robotModel.limbs[Limbs::footRight].rotation.inverse().getXAngle();
    if(mr.kickRequest.mirror)
      angleX *= -1.f;

    bodyAngle = Vector2f(angleX, angleY);
    calcJoints(jointRequest, rd);
    comRobotModel.setJointData(jointRequest, rd, mc);
    const Pose3f& torso = toLeftSupport ? comRobotModel.limbs[Limbs::footLeft] : comRobotModel.limbs[Limbs::footRight];
    const Vector3f com = torso.rotation.inverse() * comRobotModel.centerOfMass;

    //this calculates inverse of pid com control -> getting com to max pos at start -> beeing rotated as before engine
    float foot = toLeftSupport ? origins[Phase::leftFootTra].z() : origins[Phase::rightFootTra].z();
    float height = comRobotModel.centerOfMass.z() - foot;

    balanceSum.x() = std::tan(angleY - Constants::pi) * height;
    balanceSum.x() /= currentParameters.kiy;
    balanceSum.y() = std::tan(angleX - Constants::pi) * height;
    balanceSum.y() /= -currentParameters.kix;

    currentParameters.initFirstPhaseLoop(origins, Vector2f(com.x(), com.y()), Vector2f(ja.angles[Joints::headPitch], (mr.kickRequest.mirror) ? -ja.angles[Joints::headYaw] : ja.angles[Joints::headYaw]));

    if(!wasActive)
    {
      // origin = Vector2f::Zero();
      gyro = Vector2f::Zero();
      lastGyroLeft = Vector2f::Zero();
      lastGyroRight = Vector2f::Zero();
      gyroErrorLeft = Vector2f::Zero();
      gyroErrorRight = Vector2f::Zero();
      bodyError = Vector2f::Zero();
      lastBody = Vector2f::Zero();
      lastCom = Vector3f::Zero();

      for(int i = 0; i < Joints::numOfJoints; i++)
      {
        lastBalancedJointRequest.angles[i] = ja.angles[i];
      }
    }
    for(unsigned int i = 0; i < mr.kickRequest.dynPoints.size(); i++)
      if(mr.kickRequest.dynPoints[i].phaseNumber == phaseNumber)
        addDynPoint(mr.kickRequest.dynPoints[i], torsoMatrix);

    lElbowFront = origins[Phase::leftHandRot].x() > pi_4;
    rElbowFront = origins[Phase::rightHandRot].x() < -pi_4;

    if(mr.kickRequest.armsBackFix) //quick hack to not break arms while they are on the back
    {
      if(lElbowFront)
        addDynPoint(DynPoint(Phase::leftHandRot, 0, Vector3f(pi_2, -pi_4, 0)), torsoMatrix);

      if(rElbowFront)
        addDynPoint(DynPoint(Phase::rightHandRot, 0, Vector3f(-pi_2, pi_4, 0)), torsoMatrix);
    }
  }
}

void KickEngineData::setEngineActivation(const float& ratio)
{
  willBeLeft = (ratio < 1.f && lastRatio > ratio);
  wasActive = (ratio != 0.f && motionID > -1);
  startComp = (ratio != 0.f && lastRatio <= ratio);
  lastRatio = ratio;
}

bool KickEngineData::activateNewMotion(const KickRequest& br, const bool& isLeavingPossible)
{
  if(!wasActive || (br.kickMotionType != currentKickRequest.kickMotionType && isLeavingPossible))
    return true;
  else if(br.kickMotionType == currentKickRequest.kickMotionType && br.mirror == currentKickRequest.mirror)
    currentKickRequest = br; // update KickRequest when it is compatible to the current motion

  return false;
}

bool KickEngineData::sitOutTransitionDisturbance(bool& compensate, bool& compensated, const InertialData& id, KickEngineOutput& kickEngineOutput, const JointAngles& ja, const FrameInfo& frame)
{
  if(compensate)
  {
    if(!startComp)
    {
      timeStamp = frame.time;
      gyro = Vector2f::Zero();
      lastGyroLeft = Vector2f::Zero();
      lastGyroRight = Vector2f::Zero();
      gyroErrorLeft = Vector2f::Zero();
      gyroErrorRight = Vector2f::Zero();
      bodyError = Vector2f::Zero();
      lastBody = Vector2f::Zero();
      lastCom = Vector3f::Zero();
      motionID = -1;

      kickEngineOutput.isLeavingPossible = false;

      for(int i = 0; i < Joints::numOfJoints; i++)
      {
        lastBalancedJointRequest.angles[i] = ja.angles[i];
        compenJoints.angles[i] = ja.angles[i];
      }
    }

    for(int i = 0; i < Joints::numOfJoints; i++)
    {
      kickEngineOutput.angles[i] = compenJoints.angles[i];
      kickEngineOutput.stiffnessData.stiffnesses[i] = 100;
    }

    int time = frame.getTimeSince(timeStamp);
    if((std::abs(id.gyro.x()) < 0.1f && std::abs(id.gyro.y()) < 0.1f && time > 200) || time > 1000)
    {
      compensate = false;
      compensated = true;
      return true;
    }
    else
      return false;
  }

  return true;
}
