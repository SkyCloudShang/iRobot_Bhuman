/**
 * @file OuterCorner.cpp
 * @author <a href="mailto:jesse@tzi.de">Jesse Richter-Klug</a>
 */

#include "../ImagePreprocessing/CameraMatrix.h"
#include "OuterCorner.h"
#include "Representations/Infrastructure/CameraInfo.h"
#include "Tools/Debugging/DebugDrawings.h"
#include "Tools/Module/Blackboard.h"
#include "Tools/Math/Transformation.h"
#include "Representations/Configuration/FieldDimensions.h"

void OuterCorner::draw() const
{
  FieldFeature::draw();
  DECLARE_DEBUG_DRAWING("representation:OuterCorner:image", "drawingOnImage");
  DECLARE_DEBUG_DRAWING("representation:OuterCorner:field", "drawingOnField");
  if(!isValid)
    return;

  static const float size = 1000.;
  COMPLEX_DRAWING("representation:OuterCorner:field")
  {
    const Vector2f a = (*this) * Vector2f(0.f, isRightCorner ? size : -size);
    const Vector2f a2 = (*this) * Vector2f(0.f, isRightCorner ? (size / 2.f) : -(size / 2.f));
    const Vector2f b = this->translation;
    const Vector2f c = (*this) * Vector2f(size, 0.f);
    LINE("representation:OuterCorner:field", a.x(), a.y(), b.x(), b.y(), 10, Drawings::solidPen, ColorRGBA::blue);
    LINE("representation:OuterCorner:field", a.x(), a.y(), c.x(), c.y(), 10, Drawings::solidPen, ColorRGBA::blue);
    LINE("representation:OuterCorner:field", b.x(), b.y(), c.x(), c.y(), 10, Drawings::solidPen, ColorRGBA::blue);
    LINE("representation:OuterCorner:field", a2.x(), a2.y(), c.x(), c.y(), 10, Drawings::solidPen, ColorRGBA::blue);
  }
  COMPLEX_DRAWING("representation:OuterCorner:image")
  {
    if(Blackboard::getInstance().exists("CameraMatrix") && Blackboard::getInstance().exists("CameraInfo"))
    {
      const CameraMatrix& theCameraMatrix = static_cast<const CameraMatrix&>(Blackboard::getInstance()["CameraMatrix"]);
      const CameraInfo& theCameraInfo = static_cast<const CameraInfo&>(Blackboard::getInstance()["CameraInfo"]);

      const Vector2f a = (*this) * Vector2f(0.f, isRightCorner ? size : -size);
      const Vector2f a2 = (*this) * Vector2f(0.f, isRightCorner ? (size / 2.f) : -(size / 2.f));
      const Vector2f b = this->translation;
      const Vector2f c = (*this) * Vector2f(size, 0.f);
      Vector2f aImage, a2Image, bImage, cImage;

      if(Transformation::robotToImage(a, theCameraMatrix, theCameraInfo, aImage) &&
         Transformation::robotToImage(a2, theCameraMatrix, theCameraInfo, a2Image) &&
         Transformation::robotToImage(b, theCameraMatrix, theCameraInfo, bImage) &&
         Transformation::robotToImage(c, theCameraMatrix, theCameraInfo, cImage))
      {
        LINE("representation:OuterCorner:image", aImage.x(), aImage.y(), bImage.x(), bImage.y(), 2, Drawings::solidPen, ColorRGBA::blue);
        LINE("representation:OuterCorner:image", aImage.x(), aImage.y(), cImage.x(), cImage.y(), 2, Drawings::solidPen, ColorRGBA::blue);
        LINE("representation:OuterCorner:image", bImage.x(), bImage.y(), cImage.x(), cImage.y(), 2, Drawings::solidPen, ColorRGBA::blue);
        LINE("representation:OuterCorner:image", a2Image.x(), a2Image.y(), cImage.x(), cImage.y(), 2, Drawings::solidPen, ColorRGBA::blue);
      }
    }
  }
}

const Pose2f OuterCorner::getGlobalFeaturePosition() const
{
  ASSERT(isValid);
  ASSERT(Blackboard::getInstance().exists("FieldDimensions"));
  const FieldDimensions& theFieldDimensions = static_cast<const FieldDimensions&>(Blackboard::getInstance()["FieldDimensions"]);

  return Pose2f(isRightCorner ? pi_2 : -pi_2, theFieldDimensions.xPosOpponentGroundline, isRightCorner ? theFieldDimensions.yPosRightSideline : theFieldDimensions.yPosLeftSideline);

}
