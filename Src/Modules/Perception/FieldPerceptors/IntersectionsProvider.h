/**
 * @author Arne Böckmann
 * @author Felix Thielke
 */

#pragma once

#include "Tools/Module/Module.h"
#include "Representations/Infrastructure/CameraInfo.h"
#include "Representations/Perception/ImagePreprocessing/CameraMatrix.h"
#include "Representations/Perception/FieldPercepts/CirclePercept.h"
#include "Representations/Perception/FieldPercepts/LinesPercept.h"
#include "Representations/Perception/FieldPercepts/IntersectionsPercept.h"

MODULE(IntersectionsProvider,
{,
  REQUIRES(CameraInfo),
  REQUIRES(CameraMatrix),
  REQUIRES(CirclePercept), // Just to make sure that lines on the circle are marked as such
  REQUIRES(LinesPercept),
  PROVIDES(IntersectionsPercept),
  LOADS_PARAMETERS(
  {,
    (float)(0.15f) maxAllowedIntersectionAngleDifference, /**<The angle between two intersecting lines should not differ more from 90° than this number (in rad) */
    (float)(0.8f) maxLengthUnrecognizedProportion,  /**< the length of the recognized line multiplied by this value could maximal imagine */
    (float)(20.f) maxOverheadToDecleareAsEnd,  /**< the max of pixel a end can be farther away to declear as end*/
  }),
});

class IntersectionsProvider : public IntersectionsProviderBase
{
public:
  void update(IntersectionsPercept& intersectionsPercept);

  /**
  * returns the distance of the closer point to target
  * @param[out] closer the point closer to the target
  * @param[out] further the point further away from the target
  */
  float getCloserPoint(const Vector2f& a, const Vector2f& b, const Vector2f target, Vector2f& closer, Vector2f& further) const;

  /**Determines whether the point is in the line segment or not*/
  bool isPointInSegment(const LinesPercept::Line& line, const Vector2f& point) const;

  void addIntersection(IntersectionsPercept& intersectionsPercept, IntersectionsPercept::Intersection::IntersectionType type,
                       const Vector2f& intersection, const Vector2f& dir1, const Vector2f& dir2, unsigned line1, unsigned line2) const;

  /**enforces that horizontal is +90° of vertical*/
  void enforceTIntersectionDirections(const Vector2f& vertical, Vector2f& horizontal) const;
};
