/**
 * @file FallEngine.h
 * A minimized motion engine for falling.
 * @author Bernd Poppinga
 */

#pragma once
#include "Representations/MotionControl/ArmMotionRequest.h"
#include "Representations/MotionControl/MotionInfo.h"
#include "Representations/MotionControl/FallEngineOutput.h"
#include "Representations/Infrastructure/JointAngles.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Infrastructure/GameInfo.h"
#include "Representations/Sensing/FallDownState.h"
#include "Representations/Sensing/InertialData.h"
#include "Tools/Module/Module.h"
#include "Tools/Motion/MotionUtilities.h"

MODULE(FallEngine,
{,
  REQUIRES(ArmMotionRequest),
  REQUIRES(FallDownState),
  REQUIRES(JointAngles),
  REQUIRES(StiffnessSettings),
  REQUIRES(FrameInfo),
  REQUIRES(InertialData),
  REQUIRES(GameInfo),
  USES(MotionRequest),
  USES(MotionInfo),
  USES(JointRequest),
  PROVIDES(FallEngineOutput),
  DEFINES_PARAMETERS(
  {,
    (int)(2000) waitAfterFalling,
    (int)(10000) noGameControllerThreshold,
    (Angle)(20_deg) forwardThreshold,
    (Angle)(-10_deg) backwardsThreshold,
  }),
});

class FallEngine : public FallEngineBase
{
public:
  void update(FallEngineOutput& output);

private:
  bool waitingForGetup = false;

  bool headYawInSafePosition = false;
  bool headPitchInSafePosition = false;
  bool shoulderInSafePosition = false;
  JointAngles lastJointsBeforeFall;

  bool specialActionFall() const;

  void safeFall(FallEngineOutput& output) const;
  void safeArms(FallEngineOutput& output) const;
  void centerHead(FallEngineOutput& jointRequest);

  void calcDirection(FallEngineOutput& output);

  void interpolate(const JointAngles& from, const JointRequest& to, float& ratio, JointRequest& target) const;
};
