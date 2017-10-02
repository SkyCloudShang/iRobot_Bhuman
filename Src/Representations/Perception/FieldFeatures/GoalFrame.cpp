/**
 * @file MidCorner.cpp
 * @author <a href="mailto:jesse@tzi.de">Jesse Richter-Klug</a>
 */

#include "../ImagePreprocessing/CameraMatrix.h"
#include "GoalFrame.h"
#include "Representations/Infrastructure/CameraInfo.h"
#include "Tools/Debugging/DebugDrawings.h"
#include "Tools/Module/Blackboard.h"
#include "Tools/Math/Transformation.h"
#include "Representations/Configuration/FieldDimensions.h"

void GoalFrame::draw() const
{
  FieldFeature::draw();
  DECLARE_DEBUG_DRAWING("representation:GoalFrame:image", "drawingOnImage");
  DECLARE_DEBUG_DRAWING("representation:GoalFrame:field", "drawingOnField");
  if(!isValid)
    return;

  static const float size = 1000.;
  COMPLEX_DRAWING("representation:GoalFrame:field")
  {
    const Vector2f a = (*this) * Vector2f(0.f, size);
    const Vector2f b = (*this) * Vector2f(0.f, -size);
    const Vector2f c = (*this) * Vector2f(size, 0.f);
    const Vector2f d = (*this) * Vector2f(size, size);
    const Vector2f e = (*this) * Vector2f(size, -size);
    LINE("representation:GoalFrame:field", a.x(), a.y(), b.x(), b.y(), 10, Drawings::solidPen, ColorRGBA::blue);
    LINE("representation:GoalFrame:field", a.x(), a.y(), c.x(), c.y(), 10, Drawings::solidPen, ColorRGBA::blue);
    LINE("representation:GoalFrame:field", b.x(), b.y(), c.x(), c.y(), 10, Drawings::solidPen, ColorRGBA::blue);
    LINE("representation:GoalFrame:field", d.x(), d.y(), e.x(), e.y(), 10, Drawings::solidPen, ColorRGBA::blue);
    LINE("representation:GoalFrame:field", b.x(), b.y(), e.x(), e.y(), 10, Drawings::solidPen, ColorRGBA::blue);
    LINE("representation:GoalFrame:field", d.x(), d.y(), a.x(), a.y(), 10, Drawings::solidPen, ColorRGBA::blue);
    DRAWTEXT("representation:GoalFrame:field", this->translation.x(), this->translation.y(), 40, ColorRGBA::blue, "GF");
  }
  COMPLEX_DRAWING("representation:GoalFrame:image")
  {
    if(Blackboard::getInstance().exists("CameraMatrix") && Blackboard::getInstance().exists("CameraInfo"))
    {
      const CameraMatrix& theCameraMatrix = static_cast<const CameraMatrix&>(Blackboard::getInstance()["CameraMatrix"]);
      const CameraInfo& theCameraInfo = static_cast<const CameraInfo&>(Blackboard::getInstance()["CameraInfo"]);

      const Vector2f a = (*this) * Vector2f(0.f, size);
      const Vector2f b = (*this) * Vector2f(0.f, -size);
      const Vector2f c = (*this) * Vector2f(size, 0.f);
      const Vector2f d = (*this) * Vector2f(size, size);
      const Vector2f e = (*this) * Vector2f(size, -size);
      Vector2f aImage, bImage, cImage, dImage, eImage;

      if(Transformation::robotToImage(a, theCameraMatrix, theCameraInfo, aImage) &&
         Transformation::robotToImage(b, theCameraMatrix, theCameraInfo, bImage) &&
         Transformation::robotToImage(c, theCameraMatrix, theCameraInfo, cImage) &&
         Transformation::robotToImage(d, theCameraMatrix, theCameraInfo, dImage) &&
         Transformation::robotToImage(e, theCameraMatrix, theCameraInfo, eImage))
      {
        LINE("representation:GoalFrame:image", aImage.x(), aImage.y(), bImage.x(), bImage.y(), 2, Drawings::solidPen, ColorRGBA::blue);
        LINE("representation:GoalFrame:image", aImage.x(), aImage.y(), cImage.x(), cImage.y(), 2, Drawings::solidPen, ColorRGBA::blue);
        LINE("representation:GoalFrame:image", bImage.x(), bImage.y(), cImage.x(), cImage.y(), 2, Drawings::solidPen, ColorRGBA::blue);
        LINE("representation:GoalFrame:image", dImage.x(), dImage.y(), eImage.x(), eImage.y(), 2, Drawings::solidPen, ColorRGBA::blue);
        LINE("representation:GoalFrame:image", bImage.x(), bImage.y(), eImage.x(), eImage.y(), 2, Drawings::solidPen, ColorRGBA::blue);
        LINE("representation:GoalFrame:image", dImage.x(), dImage.y(), aImage.x(), aImage.y(), 2, Drawings::solidPen, ColorRGBA::blue);
      }
    }
  }
}

const Pose2f GoalFrame::getGlobalFeaturePosition() const
{
  ASSERT(isValid);
  ASSERT(Blackboard::getInstance().exists("FieldDimensions"));
  const FieldDimensions& theFieldDimensions = static_cast<const FieldDimensions&>(Blackboard::getInstance()["FieldDimensions"]);

  return Pose2f(theFieldDimensions.xPosOpponentGroundline, 0.f);
}
