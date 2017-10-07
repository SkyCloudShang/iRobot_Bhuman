/**
 * @file MidCorner.cpp
 * @author <a href="mailto:jesse@tzi.de">Jesse Richter-Klug</a>
 */

#include "../ImagePreprocessing/CameraMatrix.h"
#include "MidCorner.h"
#include "Representations/Infrastructure/CameraInfo.h"
#include "Tools/Debugging/DebugDrawings.h"
#include "Tools/Module/Blackboard.h"
#include "Tools/Math/Transformation.h"
#include "Representations/Configuration/FieldDimensions.h"

void MidCorner::draw() const
{
  FieldFeature::draw();
  DECLARE_DEBUG_DRAWING("representation:MidCorner:image", "drawingOnImage");
  DECLARE_DEBUG_DRAWING("representation:MidCorner:field", "drawingOnField");
  if(!isValid)
    return;

  static const float size = 1000.;
  COMPLEX_DRAWING("representation:MidCorner:field")
  {
    const Vector2f a = (*this) * Vector2f(0.f, size);
    const Vector2f b = (*this) * Vector2f(0.f, -size);
    const Vector2f c = (*this) * Vector2f(size, 0.f);
    LINE("representation:MidCorner:field", a.x(), a.y(), b.x(), b.y(), 10, Drawings::solidPen, ColorRGBA::blue);
    LINE("representation:MidCorner:field", a.x(), a.y(), c.x(), c.y(), 10, Drawings::solidPen, ColorRGBA::blue);
    LINE("representation:MidCorner:field", b.x(), b.y(), c.x(), c.y(), 10, Drawings::solidPen, ColorRGBA::blue);
    //DRAWTEXT("representation:MidCorner:field", this->translation.x(), this->translation.y(), 40, ColorRGBA::blue, "MC");
  }
  COMPLEX_DRAWING("representation:MidCorner:image")
  {
    if(Blackboard::getInstance().exists("CameraMatrix") && Blackboard::getInstance().exists("CameraInfo"))
    {
      const CameraMatrix& theCameraMatrix = static_cast<const CameraMatrix&>(Blackboard::getInstance()["CameraMatrix"]);
      const CameraInfo& theCameraInfo = static_cast<const CameraInfo&>(Blackboard::getInstance()["CameraInfo"]);

      const Vector2f a = (*this) * Vector2f(0.f, size);
      const Vector2f b = (*this) * Vector2f(0.f, -size);
      const Vector2f c = (*this) * Vector2f(size, 0.f);
      Vector2f aImage, bImage, cImage;

      if(Transformation::robotToImage(a, theCameraMatrix, theCameraInfo, aImage) &&
         Transformation::robotToImage(b, theCameraMatrix, theCameraInfo, bImage) &&
         Transformation::robotToImage(c, theCameraMatrix, theCameraInfo, cImage))
      {
        LINE("representation:MidCorner:image", aImage.x(), aImage.y(), bImage.x(), bImage.y(), 2, Drawings::solidPen, ColorRGBA::blue);
        LINE("representation:MidCorner:image", aImage.x(), aImage.y(), cImage.x(), cImage.y(), 2, Drawings::solidPen, ColorRGBA::blue);
        LINE("representation:MidCorner:image", bImage.x(), bImage.y(), cImage.x(), cImage.y(), 2, Drawings::solidPen, ColorRGBA::blue);
      }
    }
  }
}

const Pose2f MidCorner::getGlobalFeaturePosition() const
{
  ASSERT(isValid);
  ASSERT(Blackboard::getInstance().exists("FieldDimensions"));
  const FieldDimensions& theFieldDimensions = static_cast<const FieldDimensions&>(Blackboard::getInstance()["FieldDimensions"]);

  const Vector2f dirPoint = (*this) * Vector2f(1.f, 0.f);

  bool isRightMidCorner(((dirPoint.x() - this->translation.x()) * (0.f - this->translation.y()) - (dirPoint.y() - this->translation.y()) * (0.f - this->translation.x())) > 0.f);
  return Pose2f(isRightMidCorner ? pi_2 : -pi_2, 0.f, isRightMidCorner ? theFieldDimensions.yPosRightSideline : theFieldDimensions.yPosLeftSideline);
}
