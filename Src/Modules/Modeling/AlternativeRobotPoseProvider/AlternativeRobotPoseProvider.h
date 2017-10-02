/**
 * @file AlternativeRobotPoseProvider.h
 *
 * Declaration of a module that uses recent field feature obervations
 * and combines them to an alternative pose of the robot.
 *
 * @author <A href="mailto:tlaue@uni-bremen.de">Tim Laue</A>
 */

#pragma once

#include "Tools/Module/Module.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Infrastructure/GameInfo.h"
#include "Representations/Modeling/AlternativeRobotPoseHypothesis.h"
#include "Representations/Modeling/Odometer.h"
#include "Representations/Modeling/OwnSideModel.h"
#include "Representations/MotionControl/MotionInfo.h"
#include "Representations/Perception/FieldFeatures/GoalFeature.h"
#include "Representations/Perception/FieldFeatures/GoalFrame.h"
#include "Representations/Perception/FieldFeatures/MidCircle.h"
#include "Representations/Perception/FieldFeatures/MidCorner.h"
#include "Representations/Perception/FieldFeatures/OuterCorner.h"
#include "Representations/Perception/FieldFeatures/PenaltyArea.h"
#include "Representations/Sensing/GroundContactState.h"
#include "Tools/Debugging/DebugDrawings.h"
#include "Tools/RingBuffer.h"

MODULE(AlternativeRobotPoseProvider,
{,
  REQUIRES(FieldDimensions),
  REQUIRES(FrameInfo),
  REQUIRES(GameInfo),
  REQUIRES(GoalFeature),
  REQUIRES(GoalFrame),
  REQUIRES(GroundContactState),
  REQUIRES(MidCircle),
  REQUIRES(MidCorner),
  REQUIRES(MotionInfo),
  REQUIRES(OuterCorner),
  REQUIRES(Odometer),
  REQUIRES(OwnSideModel),
  REQUIRES(PenaltyArea),
  PROVIDES(AlternativeRobotPoseHypothesis),
  DEFINES_PARAMETERS(
  {,
    (int)(20000) maxTimeToKeepObservation,  /**< What the name says (in ms) */
    (float)(800.f) translationDifference,   /**< Maximum translational difference for assigning a pose to a cluster */
    (float)(0.5f) rotationDifference,       /**< Maximum rotational difference for assigning a pose to a cluster */
    (unsigned)(6) maxClusters,              /**< Do not try to find more than this number of clusters */
  }),
});

/**
 * @class AlternativeRobotPoseProvider
 *
 * Computes a new robot pose
 */
class AlternativeRobotPoseProvider : public AlternativeRobotPoseProviderBase
{
  /** All necessary information about a recently determined pose */
  struct PoseObservation
  {
    Pose2f pose;                    /**< The robot's pose */
    unsigned int timeOfObservation; /**< The point of time when the pose was computed */
    bool stillInOwnHalf;            /**< The robot knows that it must be in its own half during observation */
  };

  /** A cluster of poses */
  struct Cluster
  {
    int numOfPoses;                        /**< The number of poses that have been clustered */
    Pose2f pose;                           /**< The center of the cluster (i.e. average of all poses) */
    unsigned int timeOfNewestObservation;  /**< Point of time, the newest pose of this cluster was computed */
    bool isInOwnHalf;                      /**< At least one of the poses was definitely in the own half (given OwnSideModel) */

    /** Default constructor, initializes all members */
    Cluster()
    {
      numOfPoses = 0;
      pose.rotation = 0;
      pose.translation = Vector2f(0.f, 0.f);
      timeOfNewestObservation = 0;
      isInOwnHalf = false;
    }
  };

  RingBuffer<PoseObservation, 32> observations;  /**< The most recent pose observations */
  std::vector<Cluster> clusters;                 /**< The list of clusters */

  /** Computes the representation.
   * @param alternativeRobotPoseHypothesis The hypothesis
   */
  void update(AlternativeRobotPoseHypothesis& alternativeRobotPoseHypothesis);

  /** Converts and adds a feature to the ringbuffer.
   * @param ff A pointer to a field feature, e.g. PenaltyArea, MidCircle, ...
   */
  void addFieldFeatureToBuffer(const FieldFeature* ff);

  /** During each execution cycle, all observations that are older than maxTimeToKeepObservation will be removed. */
  void removeOldObservations();

  /** During each execution cycle, the observations are clustered again. */
  void clusterObservations();

  /** Observations made in previous execution cycles require an odometry update. */
  void odometryUpdate();

  /** Draw the content of the observation buffer. */
  void drawObservations();
};
