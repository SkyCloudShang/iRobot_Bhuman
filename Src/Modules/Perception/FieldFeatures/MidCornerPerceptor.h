/**
 * @file MidCornerPerceptor.h
 * profides MidCorner
 * @author <a href="mailto:jesse@tzi.de">Jesse Richter-Klug</a>
 */

#pragma once

#include "Tools/Module/Module.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Perception/FieldPercepts/FieldLineIntersections.h"
#include "Representations/Perception/FieldPercepts/FieldLines.h"
#include "Representations/Perception/FieldFeatures/FieldRelations.h"
#include "Representations/Perception/FieldFeatures/MidCorner.h"

MODULE(MidCornerPerceptor,
{,
  REQUIRES(FieldLineIntersections),
  REQUIRES(IntersectionRelations),
  REQUIRES(FieldDimensions),
  REQUIRES(FieldLines),
  PROVIDES(MidCorner),
  DEFINES_PARAMETERS(
  {,
    (float)(600.f) distrustAreaXRadius,
    (float)(500.f) distrustAreaYRadius,
    (Vector2f)(Vector2f(200.f,0.f)) distrustAreaOffset,
  }),
});

class MidCornerPerceptor : public MidCornerPerceptorBase
{
  void update(MidCorner& midCorner);
private:
  bool searchForBigT(MidCorner& midCorner) const;
};
