/**
 * @file SelfLocator.h
 *
 * Declares a class that performs self-localization
 *
 * @author <a href="mailto:tlaue@uni-bremen.de">Tim Laue</a>
 */

#pragma once

#include "UKFSample.h"
#include "Tools/Modeling/PerceptRegistration.h"
#include "Tools/Modeling/SampleSet.h"
#include "Tools/Module/Module.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Infrastructure/CameraInfo.h"
#include "Representations/Infrastructure/CognitionStateChanges.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Infrastructure/GameInfo.h"
#include "Representations/Infrastructure/RobotInfo.h"
#include "Representations/Infrastructure/TeamInfo.h"
#include "Representations/Modeling/AlternativeRobotPoseHypothesis.h"
#include "Representations/Modeling/BallModel.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Modeling/Odometer.h"
#include "Representations/Modeling/OwnSideModel.h"
#include "Representations/Modeling/SelfLocalizationHypotheses.h"
#include "Representations/Modeling/SideConfidence.h"
#include "Representations/MotionControl/MotionInfo.h"
#include "Representations/MotionControl/MotionRequest.h"
#include "Representations/MotionControl/OdometryData.h"
#include "Representations/Perception/ImagePreprocessing/CameraMatrix.h"
#include "Representations/Perception/ImagePreprocessing/FieldBoundary.h"
#include "Representations/Perception/FieldFeatures/GoalFeature.h"
#include "Representations/Perception/FieldFeatures/GoalFrame.h"
#include "Representations/Perception/FieldFeatures/MidCircle.h"
#include "Representations/Perception/FieldFeatures/MidCorner.h"
#include "Representations/Perception/FieldFeatures/OuterCorner.h"
#include "Representations/Perception/FieldFeatures/PenaltyArea.h"
#include "Representations/Perception/FieldPercepts/FieldLines.h"
#include "Representations/Perception/FieldPercepts/FieldLineIntersections.h"
#include "Representations/Perception/FieldPercepts/CirclePercept.h"
#include "Representations/Perception/FieldPercepts/PenaltyMarkPercept.h"

MODULE(SelfLocator,
{,
  REQUIRES(AlternativeRobotPoseHypothesis),
  REQUIRES(CognitionStateChanges),
  REQUIRES(Odometer),
  REQUIRES(OdometryData),
  REQUIRES(OwnSideModel),
  REQUIRES(OwnTeamInfo),
  REQUIRES(GameInfo),
  REQUIRES(GoalFeature),
  REQUIRES(GoalFrame),
  REQUIRES(RobotInfo),
  REQUIRES(FieldLines),
  REQUIRES(FieldLineIntersections),
  REQUIRES(CirclePercept),
  REQUIRES(FieldDimensions),
  REQUIRES(FrameInfo),
  REQUIRES(MidCircle),
  REQUIRES(MidCorner),
  REQUIRES(MotionInfo),
  REQUIRES(CameraMatrix),
  REQUIRES(CameraInfo),
  REQUIRES(BallModel),
  REQUIRES(FieldBoundary),
  REQUIRES(OuterCorner),
  REQUIRES(PenaltyArea),
  REQUIRES(RobotPose),
  REQUIRES(SideConfidence),
  REQUIRES(PenaltyMarkPercept),
  USES(MotionRequest),
  PROVIDES(RobotPose),
  PROVIDES(SelfLocalizationHypotheses),
  LOADS_PARAMETERS(
  {,
    (int) numberOfSamples,                          /**< The number of samples used by the self-locator */
    (Pose2f) defaultPoseDeviation,                  /**< Standard deviation used for creating new hypotheses */
    (Pose2f) filterProcessDeviation,                /**< The process noise for estimating the robot pose. */
    (Pose2f) odometryDeviation,                     /**< The percentage inaccuracy of the odometry. */
    (Vector2f) odometryRotationDeviation,           /**< A rotation deviation of each walked mm. */
    (float) goalAssociationMaxAngle,                /**< Angle threshold for associating a seen goal post and a goal post in the model */
    (float) goalAssociationMaxAngularDistance,      /**< Distance threshold (distance as angle) for associating a seen goal post and a goal post in the model */
    (float) centerCircleAssociationDistance,        /**< Distance threshold (metric) for associating a seen center circle and a center circle in the model */
    (float) penaltyMarkAssociationDistance,         /**< Distance threshold (metric) for associating a seen penalty mark and a penalty mark in the model */
    (float) globalPoseAssociationDistance,          /**< Distance threshold (metric) for associating a computed pose (by field feature) and the currently estimated pose */
    (float) lineAssociationCorridor,                /**< The corridor (metric distance) used for relating seen lines with field lines. */
    (float) intersectionAssociationDistance,        /**< Distance threshold (metric) for associating a seen intersection and an intersection in the model */
    (Vector2f) robotRotationDeviation,              /**< Deviation of the rotation of the robot's torso */
    (Vector2f) robotRotationDeviationInStand,       /**< Deviation of the rotation of the robot's torso when it is standing. */
    (float) validityThreshold,                      /**< if(robotPose.validity >= validityThreshold) robotPose.validity = 1.f; */
    (float) minTranslationNoise,                    /**< The minimum translation noise (in mm) that is added to each sample's translational direction during each motion update */
    (float) minRotationNoise,                       /**< The minimum rotation noise (in radian) that is added to each sample's rotation during each motion update */
    (float) movedDistWeightRotationNoise,           /**< Weighting for considering walked distance when computing a sample's rotational error */
    (float) movedAngleWeightRotationNoise,          /**< Weighting for considering rotation when computing a sample's rotational error */
    (float) movedAngleWeightRotationNoiseNotWalking,/**< Weighting for considering rotation when computing a sample's rotational error, special handling for situations in which the robot is not walking and odometry might not be continuous */
    (float) majorDirTransWeight,                    /**< Weighting for considering the major direction when computing a sample's translational error (major means, for instance, the movement in x direction when computing the translational error in x direction) */
    (float) minorDirTransWeight,                    /**< Weighting for considering the minor direction when computing a sample's translational error (minor means, for instance, the movement in y direction when computing the translational error in x direction) */
    (bool) goalFrameIsPerceivedAsLines,             /**< Set to true, if the field has goals with a white frame on the floor */
    (bool) alwaysAssumeOpponentHalf,                /**< When making a short demo with one goal, the robot should always assume that this is the goal to score at.*/
    (float) baseValidityWeighting,                  /**< Each particle has at least this weight. */
    (float) goalieFieldBorderDistanceThreshold,     /**< Workaround for 180 degree turned goalie: Definition of distance for a "far" field border */
    (float) goalieNoPerceptsThreshold,              /**< Workaround for 180 degree turned goalie: Timeout for not having seen major percepts (ball, field features) */
    (float) goalieNoFarFieldBorderThreshold,        /**< Workaround for 180 degree turned goalie: Timeout for not having seen the "far" field border */
    (float) positionJumpNotificationDistance,       /**< Threshold to leave some time between two "JUMP!" sounds. */
    (int) numberOfConsideredFramesForValidity,      /**< Robot pose validity is filtered over this number of frames. */
    (int) minNumberOfObservationsForResetting,      /**< To accept an alternative robot pose, it must be based on at least this many observations of field features. */
    (float) translationalDeviationForResetting,     /**< To insert a new particle, the current alternative pose must be farther away from the current robot pose than this threshold. */
    (float) rotationalDeviationForResetting,        /**< To insert a new particle, the current alternative pose rotation must be more different from the current robot pose rotation than this threshold. */
  }),
});


/**
 * @class SelfLocator
 *
 * A module for self-localization, based on a particle filter with each particle having 
 * an Unscented Kalman Filter.
 */
class SelfLocator : public SelfLocatorBase
{
private:
  SampleSet<UKFSample>* samples;             /**< Container for all samples. */
  PerceptRegistration perceptRegistration;   /**< Subcomponent for associating percepts to the field model */
  RegisteredPercepts registeredPercepts;     /**< Collection of percepts that have been associated with elements on the field */
  Pose2f lastRobotPose;                      /**< The result of the last computation */
  unsigned lastTimeFarFieldBorderSeen;       /**< Timestamp for checking goalie localization */
  unsigned lastTimeJumpSound;                /**< When has the last sound been played? Avoid to flood the sound player in some situations */
  unsigned timeOfLastReturnFromPenalty;      /**< Point of time when the last penalty of this robot was over */
  bool sampleSetHasBeenResetted;             /**< Flag indicating that all samples have been replaced in the current frame */
  int nextSampleNumber;                      /**< Unique sample identifiers */
  int idOfLastBestSample;                    /**< Identifier of the best sample of the last frame */
  float averageWeighting;                    /**< The average of the weightings of all samples in the sample set */
  unsigned lastAlternativePoseTimestamp;     /**< Last time an alternative pose was valid */
  std::vector<Pose2f> walkInPositions;       /**< List of poses at which our robots stand at the beginning of a half */
  unsigned int nextWalkInPoseNumber;         /**< Counter for uniform walk-in pose selection, only needed for robots with a number > 5 */
  bool nextReturnFromPenaltyIsLeft;          /**< Flag that is used to assure that after a penalty both alternative poses get the same number of samples */

  /**
   * The method provides the robot pose
   *
   * @param robotPose The robot pose representation that is updated by this module.
   */
  void update(RobotPose& robotPose);

  /**
   * The method provides the list of currently evaluated hypotheses.
   *
   * @param selfLocalizationHypotheses The representation that is updated by this module.
   */
  void update(SelfLocalizationHypotheses& selfLocalizationHypotheses);

  /** Integrate odometry offset into hypotheses */
  void motionUpdate();

  /** Perform UKF measurement step for all samples */
  void sensorUpdate();

  /** Particle filter resampling step */
  void resampling();

  /** Tries to replace a sample, if current robot pose is much different from the alternative hypothesis
   * @param robotPose The current robot pose estimate
   * @return true, if a sample has been replaced
   */
  bool sensorResetting(const RobotPose& robotPose);

  /** Special handling for initializing samples at some game state transitions and in penalty shootout
   * @param propagatedRobotPose The current position fo the robot
   */
  void handleGameStateChanges(const Pose2f& propagatedRobotPose);

  /** Handle mirror information from SideConfidence */
  void handleSideConfidence();

  /** Computes all elements needed to fill RobotPose
   * @param robotPose The pose
   */
  void computeModel(RobotPose& robotPose);

  /** Method for hacks. Currently: The 180 degree goalie turn problem */
  void domainSpecificSituationHandling();

  /** Returns a reference to the sample that has the highest validity
   * @return A reference sample
   */
  UKFSample& getMostValidSample();

  /** Check to avoid samples with the same ID
   * @return Always true ;-)
   */
  bool allSamplesIDsAreUnique();

  /** Returns a pose at one of the two possible positions when returning from a penalty
   * @return A robot pose
   */
  Pose2f getNewPoseReturnFromPenaltyPosition();

  /** Returns the preconfigured pose for a robot to enter the field
   * @return A robot pose
   */
  Pose2f getNewPoseAtWalkInPosition();

  /** Returns a pose on the manual placement line or inside the own penalty area (if the robot is the goalie)
   * @return A robot pose
   */
  Pose2f getNewPoseAtManualPlacementPosition();

  /** Computes a new pose based on the AlternativeRobotPoseHypothesis
   * @param forceOwnHalf The new pose must be inside the own half
   * @param lastRobotPose The previous robot pose
   * @return A new pose
   */
  Pose2f getNewPoseBasedOnObservations(bool forceOwnHalf, const Pose2f& lastRobotPose) const;

  /** Checks, if a new pose is closer to the last pose or its mirrored version
   * @param true, if the mirrored version is a better fit
   */
  bool isMirrorCloser(const Pose2f& currentPose, const Pose2f& lastPose) const;

  /** Draw debug information 
   * @param robotPose The current robot pose
   */
  void draw(const RobotPose& robotPose);

public:
  /** Default constructor */
  SelfLocator();

  /** Destructor */
  ~SelfLocator();
};
