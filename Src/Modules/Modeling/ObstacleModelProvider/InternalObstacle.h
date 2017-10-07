/**
 *
 * @author Florian Maaß
 */
#pragma once

#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Modeling/ObstacleModel.h"
#include "Representations/Infrastructure/CameraInfo.h"
#include "Representations/Perception/ImagePreprocessing/CameraMatrix.h"
#include "Representations/Perception/ImagePreprocessing/FieldBoundary.h"
#include "OrientationKalmanFilter.h"
#include <functional>

class ObstacleModelProvider;

class InternalObstacle : public Obstacle
{
  friend class ObstacleModelProvider;

private:
  InternalObstacle(const Matrix2f& covariance,
                   const Vector2f& center,
                   const Vector2f& left,
                   const Vector2f& right,
                   const unsigned lastmeasurement,
                   const unsigned seenCount,
                   const Type type);
  InternalObstacle(const Type type);

  bool isBehind(const InternalObstacle& other) const; /**< is this obstacle behind another one */
  bool isBetween(const float leftAngle, const float rightAngle) const /**< is the left and the right vector between the two given parameters left and right angle (radian)*/
  {
    return leftAngle > left.angle() && right.angle() > rightAngle;
  }

  void dynamic(const float odometryRotation, const Vector2f odometryTranslation, const Matrix2f odometryJacobian,
               const float odometryNoiseX, const float odometryNoiseY, const float oKFDynamicNoise, const float dKFDynamicNoise);
  void measurement(const InternalObstacle& measurement, const float weightedSum,
                   const FieldDimensions& theFieldDimensions, const float oKFMeasureNoise, const float dKFDynamicNoise);
  void considerType(const InternalObstacle& measurement, const int colorThreshold, const int uprightThreshold);

  bool isInImage(Vector2f& centerInImage, const CameraInfo& theCameraInfo, const CameraMatrix& theCameraMatrix) const; /**< is an obstacle in the image */
  bool fieldBoundaryFurtherAsObstacle(const Vector2f& centerInImage, const unsigned notSeenThreshold,
                                      const CameraInfo& theCameraInfo, const CameraMatrix& theCameraMatrix, const FieldBoundary& theFieldBoundary);

  int color = 0;                          /**< negative value is for teammates */
  int upright = 0;                        /**< negative value is for fallen robots */
  unsigned seenCount = 0;                 /**< Somewhat validity (between minPercepts and maxPercepts in ObstacleModelProvider) */
  unsigned notSeenButShouldSeenCount = 0; /**< how many times the obstacle was not seen but could be measured */

  OrientationKalmanFilter oKF = OrientationKalmanFilter(5.f, 0.f); /**< Kalman-Filter for the orientation value */
  OrientationKalmanFilter dKF = OrientationKalmanFilter(200.f, 0.5f); /**< Kalman-Filter for the orientation direction (forwards/backwards) */
  /**^ 0: correct direction, 1: incorrect direction*/
};
