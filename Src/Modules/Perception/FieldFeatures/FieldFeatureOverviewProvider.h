/**
 * @file FieldFeatureOverviewProvider.h
 * profides FieldFeatureOverview
 * @author <a href="mailto:jesse@tzi.de">Jesse Richter-Klug</a>
 */

#pragma once

#include "Tools/Module/Module.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Perception/FieldFeatures/FieldFeatureOverview.h"
#include "Representations/Perception/FieldFeatures/GoalFeature.h"
#include "Representations/Perception/FieldFeatures/GoalFrame.h"
#include "Representations/Perception/FieldFeatures/MidCircle.h"
#include "Representations/Perception/FieldFeatures/MidCorner.h"
#include "Representations/Perception/FieldFeatures/OuterCorner.h"
#include "Representations/Perception/FieldFeatures/PenaltyArea.h"

MODULE(FieldFeatureOverviewProvider,
{,
  REQUIRES(FrameInfo),
  REQUIRES(GoalFeature),
  REQUIRES(GoalFrame),
  REQUIRES(MidCircle),
  REQUIRES(MidCorner),
  REQUIRES(OuterCorner),
  REQUIRES(PenaltyArea),

  PROVIDES(FieldFeatureOverview),
  DEFINES_PARAMETERS(
  {,
  }),
});

class FieldFeatureOverviewProvider : public FieldFeatureOverviewProviderBase
{
  void update(FieldFeatureOverview& fieldFeatureOverview);
};
