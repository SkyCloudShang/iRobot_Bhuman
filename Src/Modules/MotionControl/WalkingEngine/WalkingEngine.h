/**
 * @file WalkingEngine.h
 *
 * This file declares a module that is a wrapper for the UNSW walk generator.
 *
 * @author Thomas Röfer
 */

#pragma once

#include "Platform/SystemCall.h"
#include "Representations/Configuration/WalkKicks.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Infrastructure/JointRequest.h"
#include "Representations/MotionControl/LegMotionSelection.h"
#include "Representations/MotionControl/MotionRequest.h"
#include "Representations/MotionControl/WalkingEngineOutput.h"
#include "Representations/MotionControl/WalkGenerator.h"
#include "Representations/Sensing/FallDownState.h"
#include "Representations/Sensing/GroundContactState.h"
#include "Representations/Sensing/OdometryOffset.h"
#include "Representations/Sensing/RobotModel.h"
#include "Tools/Debugging/Annotation.h"
#include "Tools/Math/Rotation.h"
#include "Tools/Module/Module.h"
#include "Tools/Motion/WalkKickEngine.h"
#ifdef __INTELLISENSE__
#define INTELLISENSE_PREFIX WalkingEngine::
#endif
#include "Tools/Cabsl.h"

MODULE(WalkingEngine,
{,
  REQUIRES(FallDownState),
  REQUIRES(FrameInfo),
  REQUIRES(GroundContactState),
  REQUIRES(LegMotionSelection),
  REQUIRES(MotionRequest),
  REQUIRES(OdometryOffset),
  REQUIRES(WalkGenerator),
  REQUIRES(WalkKicks),
  REQUIRES(WalkingEngineOutput),
  PROVIDES(WalkingEngineOutput),
  PROVIDES(WalkArmRequest),
  PROVIDES(WalkLegRequest),
  PROVIDES(StandArmRequest),
  PROVIDES(StandLegRequest),
  LOADS_PARAMETERS(
  {,
    (bool) playKickSounds, /**< Say which kick is currently executed. */
    (bool) useMeasuredOdometryOffset, /**< Use the OdometryOffset instead of the theWalkGenerator.odometryOffset. */
    (int) minTimeInStandBeforeLeaving, /**< The minimum time in stand before leaving is possible. */
  }),
});

/**
 * The wrapper for the UNSW walk generator. It is mainly implemented as a state machine,
 * because the original dribble engine is implemented here as well.
 */
class WalkingEngine : public WalkingEngineBase, public Cabsl<WalkingEngine>
{
public:
  WalkingEngine();

private:
  WalkKickEngine walkKickEngine; /**< The engine used to create foot trajectories for in-walk kicks. Cannot be copied. */
  JointRequest jointRequest; /**< The target joint request is provided by the walk and stand update methods. */
  Pose2f speed; /**< The request for the UNSW walk generator. */
  WalkGenerator::WalkMode walkMode = WalkGenerator::speedMode; /**< How is the request interpreted? */
  WalkRequest walkRequest; /**< The currently executed walkRequest. */
  std::function<Pose3f(float)> getKickFootOffset; /**< If set, provides an offset to add to the pose of the swing foot to create a kick motion. */
  Pose2f target; /**< The relative walk target in target mode. */
  Pose2f lastTarget; /**< The last request processed. */
  unsigned lastTimeWalking = 0; /**< The last time the engine was walking in ms. */

  void update(WalkingEngineOutput& walkingEngineOutput) override;
  void update(WalkArmRequest& walkArmRequest) override { static_cast<JointRequest&>(walkArmRequest) = jointRequest; }
  void update(WalkLegRequest& walkLegRequest) override { static_cast<JointRequest&>(walkLegRequest) = jointRequest; }
  void update(StandArmRequest& standArmRequest) override { static_cast<JointRequest&>(standArmRequest) = jointRequest; }
  void update(StandLegRequest& standLegRequest) override { static_cast<JointRequest&>(standLegRequest) = jointRequest; }

  /**
   * Fills the walking engine output from the values computed by the UNSW walk generator.
   * @param stepStarted A new step has just begun.
   * @param walkingEngineOutput The output of this module that is filled.
   */
  void updateOutput(bool stepStarted, WalkingEngineOutput& walkingEngineOutput);

  /**
   * Fills in the walking speeds in the request for the Walk2014Generator.
   * @param speed The speed or step size to walk with.
   * @param walkMode Speed or step size?
   * @param getKickFootOffset If set, provides an offset to add to the pose of the swing foot to
   *                          create a kick motion.
   */
  void walk(const Pose2f& speed, WalkGenerator::WalkMode walkMode = WalkGenerator::speedMode,
            const std::function<Pose3f(float)>& getKickFootOffset = std::function<Pose3f(float)>());

  /**
   * Fills in the walking target in the request for the Walk2014Generator.
   * @param speed The speed or step size to walk with. If everything is zero, the robot stands.
   * @param target The target to walk to if in target mode.
   */
  void walk(const Pose2f& speed, const Pose2f& target);

  /**
   * Sets the request for the Walk2014Generator to make the robot stand.
   */
  void stand();

  /**
   * The method updates the walk request, but keeps the walk kick request
   * inside it.
   */
  void updateWalkRequestWithoutKick();

  /**
   * Is the left leg swinging during a prestep for kicking?
   * The symbol is a macro to always reflect the current state of the walkKickRequest.
   */
#define leftPrestepPhase (walkRequest.walkKickRequest.kickLeg == Legs::right)

  // Behavior options
#include "Options/Root.h"
#include "Options/UpdateRequest.h"
#include "Options/Walking.h"
#include "Options/InWalkKick.h"
};
