/**
 * @file Modules/Infrastructure/OracledWorldModelProvider.h
 *
 * This file implements a module that provides models based on simulated data.
 *
 * @author <a href="mailto:tlaue@uni-bremen.de">Tim Laue</a>
 */

#include "OracledWorldModelProvider.h"
#include "Tools/Global.h"
#include "Tools/Settings.h"
#include "Tools/Math/Pose3f.h"
#include "Tools/Debugging/DebugDrawings.h"

OracledWorldModelProvider::OracledWorldModelProvider():
  lastBallModelComputation(0), lastRobotPoseComputation(0)
{}

void OracledWorldModelProvider::computeRobotPose()
{
  if(lastRobotPoseComputation == theFrameInfo.time)
    return;
  DRAW_ROBOT_POSE("module:OracledWorldModelProvider:realRobotPose", theGroundTruthWorldState.ownPose, ColorRGBA::magenta);
  theRobotPose = theGroundTruthWorldState.ownPose + robotPoseOffset;
  theRobotPose.deviation = 1.f;
  theRobotPose.validity = 1.f;
  theRobotPose.timeOfLastConsideredFieldFeature = theFrameInfo.time;
  lastRobotPoseComputation = theFrameInfo.time;
}

void OracledWorldModelProvider::computeBallModel()
{
  if(lastBallModelComputation == theFrameInfo.time || theGroundTruthWorldState.balls.size() == 0)
    return;
  computeRobotPose();
  Vector2f ballPosition = theGroundTruthWorldState.balls[0].topRows(2);

  Vector2f velocity((ballPosition - lastBallPosition) / float(theFrameInfo.getTimeSince(theBallModel.timeWhenLastSeen)) * 1000.f);
  theBallModel.estimate.position = theRobotPose.inversePose * ballPosition;
  theBallModel.estimate.velocity = velocity.rotate(-theRobotPose.rotation);
  theBallModel.lastPerception = theBallModel.estimate.position;
  theBallModel.timeWhenLastSeen = theFrameInfo.time;
  theBallModel.timeWhenDisappeared = theFrameInfo.time;

  lastBallPosition = ballPosition;
  lastBallModelComputation = theFrameInfo.time;
}

void OracledWorldModelProvider::update(BallModel& ballModel)
{
  computeBallModel();
  ballModel = theBallModel;
}

void OracledWorldModelProvider::update(GroundTruthBallModel& groundTruthBallModel)
{
  computeBallModel();
  groundTruthBallModel.lastPerception = theBallModel.lastPerception;
  groundTruthBallModel.estimate = theBallModel.estimate;
  groundTruthBallModel.timeWhenDisappeared = theBallModel.timeWhenDisappeared;
  groundTruthBallModel.timeWhenLastSeen = theBallModel.timeWhenLastSeen;
}

void OracledWorldModelProvider::update(BallModel3D& ballModel)
{
  if(theGroundTruthWorldState.balls.size() == 0)
    return;
  computeRobotPose();
  Vector3f ballPosition = theGroundTruthWorldState.balls[0];
  ballPosition.z() *= -1.f;

  Vector3f velocity((ballPosition - lastBallPosition3D) / float(theFrameInfo.getTimeSince(ballModel.timeWhenLastSeen)) * 1000.f);
  ballModel.estimate.position = Pose3f(theRobotPose.translation.x(), theRobotPose.translation.y(), 0.f).inverse() * ballPosition;
  ballModel.estimate.velocity = Pose3f().rotateZ(-theRobotPose.rotation) * velocity;
  ballModel.lastPerception = ballModel.estimate.position;
  ballModel.timeWhenLastSeen = theFrameInfo.time;
  ballModel.timeWhenDisappeared = theFrameInfo.time;

  lastBallPosition3D = ballPosition;
}

void OracledWorldModelProvider::update(ObstacleModel& obstacleModel)
{
  computeRobotPose();
  obstacleModel.obstacles.clear();
  if(!Global::settingsExist())
    return;

  const bool teammate = Global::getSettings().teamNumber == 1;
  for(unsigned int i = 0; i < theGroundTruthWorldState.firstTeamPlayers.size(); ++i)
    playerToObstacle(theGroundTruthWorldState.firstTeamPlayers[i], obstacleModel, teammate);
  for(unsigned int i = 0; i < theGroundTruthWorldState.secondTeamPlayers.size(); ++i)
    playerToObstacle(theGroundTruthWorldState.secondTeamPlayers[i], obstacleModel, !teammate);

  //add goal posts
  float squaredObstacleModelMaxDistance = sqr(obstacleModelMaxDistance);
  Vector2f goalPost = theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGoalPost, theFieldDimensions.yPosLeftGoal);
  if(goalPost.squaredNorm() < squaredObstacleModelMaxDistance)
    obstacleModel.obstacles.emplace_back(Matrix2f::Identity(), goalPost, theFrameInfo.time, Obstacle::goalpost);
  goalPost = theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGoalPost, theFieldDimensions.yPosRightGoal);
  if(goalPost.squaredNorm() < squaredObstacleModelMaxDistance)
    obstacleModel.obstacles.emplace_back(Matrix2f::Identity(), goalPost, theFrameInfo.time, Obstacle::goalpost);
  goalPost = theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOwnGoalPost, theFieldDimensions.yPosLeftGoal);
  if(goalPost.squaredNorm() < squaredObstacleModelMaxDistance)
    obstacleModel.obstacles.emplace_back(Matrix2f::Identity(), goalPost, theFrameInfo.time, Obstacle::goalpost);
  goalPost = theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOwnGoalPost, theFieldDimensions.yPosRightGoal);
  if(goalPost.squaredNorm() < squaredObstacleModelMaxDistance)
    obstacleModel.obstacles.emplace_back(Matrix2f::Identity(), goalPost, theFrameInfo.time, Obstacle::goalpost);
}

void OracledWorldModelProvider::playerToObstacle(const GroundTruthWorldState::GroundTruthPlayer& player, ObstacleModel& obstacleModel, const bool isTeammate) const
{
  Vector2f center(theRobotPose.inversePose * player.pose.translation);
  if(center.squaredNorm() >= sqr(obstacleModelMaxDistance))
    return;
  Obstacle obstacle(Matrix2f::Identity(), center, theFrameInfo.time,
                    isTeammate ? (player.upright ? Obstacle::teammate : Obstacle::fallenTeammate)
                               : player.upright ? Obstacle::opponent : Obstacle::fallenOpponent);
  obstacle.setLeftRight(Obstacle::getRobotDepth());
  obstacleModel.obstacles.emplace_back(obstacle);
}

void OracledWorldModelProvider::update(RobotPose& robotPose)
{
  DECLARE_DEBUG_DRAWING("module:OracledWorldModelProvider:realRobotPose", "drawingOnField");
  computeRobotPose();
  robotPose = theRobotPose;
}

void OracledWorldModelProvider::update(GroundTruthRobotPose& groundTruthRobotPose)
{
  computeRobotPose();
  static_cast<RobotPose&>(groundTruthRobotPose) = theRobotPose;
  groundTruthRobotPose.timestamp = theFrameInfo.time;
}

MAKE_MODULE(OracledWorldModelProvider, cognitionInfrastructure)
