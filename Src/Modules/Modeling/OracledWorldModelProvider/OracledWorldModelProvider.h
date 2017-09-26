/**
 * @file Modules/Infrastructure/OracledWorldModelProvider.h
 *
 * This file implements a module that provides models based on simulated data.
 *
 * @author <a href="mailto:tlaue@uni-bremen.de">Tim Laue</a>
 */

#pragma once

#include "Tools/Module/Module.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Infrastructure/GroundTruthWorldState.h"
#include "Representations/Modeling/BallModel.h"
#include "Representations/Modeling/ObstacleModel.h"
#include "Representations/Modeling/RobotPose.h"

MODULE(OracledWorldModelProvider,
{,
  REQUIRES(GroundTruthWorldState),
  REQUIRES(FieldDimensions),
  REQUIRES(FrameInfo),
  PROVIDES(BallModel),
  PROVIDES(BallModel3D),
  PROVIDES(GroundTruthBallModel),
  PROVIDES(ObstacleModel),
  PROVIDES(RobotPose),
  PROVIDES(GroundTruthRobotPose),
  LOADS_PARAMETERS(
  {,
    (Pose2f) robotPoseOffset, /**< Offset that will be added to the robot pose. Useful for testing */
    (float) obstacleModelMaxDistance, /**< Only obstacles (players, goalposts) will be entered in the obstacle model if their distance to the robot is closer that this parameter value */
  }),
});

/**
 * @class OracledWorldModelProvider
 * A module that provides several models
 */
class OracledWorldModelProvider: public OracledWorldModelProviderBase
{
public:
  /** Constructor*/
  OracledWorldModelProvider();

private:
  /** The function that actually computes the ball model*/
  void computeBallModel();

  /** The function that actually computes the robot pose*/
  void computeRobotPose();

  /** One main function, might be called every cycle
   * @param ballModel The data struct to be filled
   */
  void update(BallModel& ballModel);

  /** One main function, might be called every cycle
   * @param ballModel3D The data struct to be filled
   */
  void update(BallModel3D& ballModel);

  /** One main function, might be called every cycle
   * @param groundTruthBallModel The data struct to be filled
   */
  void update(GroundTruthBallModel& groundTruthBallModel);

  /** One main function, might be called every cycle
   * @param obstacleModel The data struct to be filled
   */
  void update(ObstacleModel& obstacleModel);

  /** One main function, might be called every cycle
   * @param robotPose The data struct to be filled
   */
  void update(RobotPose& robotPose);

  /** One main function, might be called every cycle
   * @param groundTruthRobotPose The data struct to be filled
   */
  void update(GroundTruthRobotPose& groundTruthRobotPose);

  /** Converts ground truth player data to an obstacle
   * @param player A player
   * @param obstacleModel The model to which the player will be added
   * @param isRed Whether a player is in team red or not
   */
  void playerToObstacle(const GroundTruthWorldState::GroundTruthPlayer& player, ObstacleModel& obstacleModel, const bool isTeammate) const;

  unsigned int lastBallModelComputation;  /*< Time of last ball model computation*/
  unsigned int lastRobotPoseComputation;  /*< Time of last robot pose computation*/
  Vector2f     lastBallPosition = Vector2f::Zero(); /*< The ball position after the last computation*/
  Vector3f     lastBallPosition3D = Vector3f::Zero(); /*< The ball position after the last computation*/
  BallModel    theBallModel;              /*< The current ball model*/
  RobotPose    theRobotPose;              /*< The current robot pose*/
};
