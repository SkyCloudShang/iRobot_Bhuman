/**
 * @file LibCodeReleaseProvider.h
 */

#include "Tools/Module/Module.h"
#include "Tools/Math/Transformation.h"
#include "Representations/BehaviorControl/Libraries/LibCodeRelease.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Modeling/BallModel.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Modeling/Odometer.h"
#include "Representations/Infrastructure/GameInfo.h"
#include "Representations/Infrastructure/RobotInfo.h"
#include "Representations/Communication/TeamData.h"
#include "Representations/Configuration/FieldDimensions.h"

MODULE(LibCodeReleaseProvider,
{,
  REQUIRES(BallModel),
  REQUIRES(FieldDimensions),
  REQUIRES(FrameInfo),
  REQUIRES(RobotPose),
  REQUIRES(Odometer),
  REQUIRES(GameInfo),
  REQUIRES(RobotInfo),
  REQUIRES(TeamData),
  PROVIDES(LibCodeRelease),
});

class LibCodeReleaseProvider : public LibCodeReleaseProviderBase
{
  void update(LibCodeRelease& libCodeRelease);
};
