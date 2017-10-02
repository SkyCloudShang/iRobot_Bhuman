#include "ObstacleModel.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Infrastructure/TeamInfo.h"
#include "Tools/Debugging/DebugDrawings.h"
#include "Tools/Debugging/DebugDrawings3D.h"
#include "Tools/Math/Approx.h"
#include "Tools/Math/Random.h"
#include "Tools/Modeling/Obstacle.h"
#include "Tools/Module/Blackboard.h"

void ObstacleModel::verify() const
{
#ifndef NDEBUG
  for(const auto& obstacle : obstacles)
  {
    ASSERT(std::isfinite(obstacle.center.x()));
    ASSERT(std::isfinite(obstacle.center.y()));

    ASSERT(std::isfinite(obstacle.left.x()));
    ASSERT(std::isfinite(obstacle.left.y()));

    ASSERT(std::isfinite(obstacle.right.x()));
    ASSERT(std::isfinite(obstacle.right.y()));

    ASSERT(std::isfinite(obstacle.velocity.x()));
    ASSERT(std::isfinite(obstacle.velocity.y()));

    //ASSERT((obstacle.left - obstacle.right).squaredNorm() < sqr(2000.f));

    ASSERT(std::isnormal(obstacle.covariance(0, 0)));
    ASSERT(std::isnormal(obstacle.covariance(1, 1)));
    ASSERT(std::isfinite(obstacle.covariance(0, 1)));
    ASSERT(std::isfinite(obstacle.covariance(1, 0)));
    ASSERT(Approx::isEqual(obstacle.covariance(0, 1), obstacle.covariance(1, 0), 1e-20f)); 
  }
#endif
}

void ObstacleModel::draw() const
{
  DECLARE_DEBUG_DRAWING("representation:ObstacleModel:rectangle", "drawingOnField");
  DECLARE_DEBUG_DRAWING("representation:ObstacleModel:centerCross", "drawingOnField");
  DECLARE_DEBUG_DRAWING("representation:ObstacleModel:leftRight", "drawingOnField");
  DECLARE_DEBUG_DRAWING("representation:ObstacleModel:circle", "drawingOnField");
  DECLARE_DEBUG_DRAWING("representation:ObstacleModel:covariance", "drawingOnField");
  DECLARE_DEBUG_DRAWING("representation:ObstacleModel:velocity", "drawingOnField");
  DECLARE_DEBUG_DRAWING("representation:ObstacleModel:fallen", "drawingOnField");
  DECLARE_DEBUG_DRAWING3D("representation:ObstacleModel", "robot");

  static const ColorRGBA colors[] =
  {
    ColorRGBA::blue,
    ColorRGBA::red,
    ColorRGBA::yellow,
    ColorRGBA::black
  };

  const ColorRGBA ownColor = colors[Blackboard::getInstance().exists("OwnTeamInfo") ?
      static_cast<const OwnTeamInfo&>(Blackboard::getInstance()["OwnTeamInfo"]).teamColor : TEAM_BLUE];

  const ColorRGBA opponentColor = colors[Blackboard::getInstance().exists("OpponentTeamInfo") ?
      static_cast<const OpponentTeamInfo&>(Blackboard::getInstance()["OpponentTeamInfo"]).teamColor : TEAM_RED];

  ColorRGBA color;
  for(const auto& obstacle : obstacles)
  {
    switch(obstacle.type)
    {
      case Obstacle::goalpost:
      {
        color = ColorRGBA::white;
        break;
      }
      case Obstacle::fallenTeammate:
      case Obstacle::teammate:
      {
        color = ownColor;
        break;
      }
      case Obstacle::fallenOpponent:
      case Obstacle::opponent:
      {
        color = opponentColor;
        break;
      }
      case Obstacle::fallenSomeRobot:
      case Obstacle::someRobot:
      {
        color = ColorRGBA::orange;
        break;
      }
      default:
      {
        color = ColorRGBA::violet;
        break;
      }
    }
    const Vector2f& center = obstacle.center;
    const Vector2f& left = obstacle.left;
    const Vector2f& right = obstacle.right;

    CYLINDER3D("representation:ObstacleModel", center.x(), center.y(), -210, 0, 0, 0, (left - right).norm(), 10, color);
    CROSS("representation:ObstacleModel:centerCross", center.x(), center.y(), Obstacle::getRobotDepth(), 10, Drawings::solidPen, color);

    float obstacleRadius = (left - right).norm() * .5f;
    Angle robotRotation = Blackboard::getInstance().exists("RobotPose") ? static_cast<const RobotPose&>(Blackboard::getInstance()["RobotPose"]).rotation : Angle();
    Vector2f frontRight(-Obstacle::getRobotDepth(), -obstacleRadius);
    frontRight = center + frontRight;
    RECTANGLE2("representation:ObstacleModel:rectangle", frontRight, obstacleRadius * 2, obstacleRadius * 2, -robotRotation, 16, Drawings::PenStyle::solidPen, ColorRGBA::black, Drawings::solidBrush, color);

    LINE("representation:ObstacleModel:leftRight", center.x(), center.y(), left.x(), left.y(), 20, Drawings::dottedPen, color);
    LINE("representation:ObstacleModel:leftRight", center.x(), center.y(), right.x(), right.y(), 20, Drawings::dottedPen, color);
    CIRCLE("representation:ObstacleModel:circle", center.x(), center.y(), obstacleRadius, 10, Drawings::dottedPen, color, Drawings::noBrush, color);
    COVARIANCE2D("representation:ObstacleModel:covariance", obstacle.covariance, center);

    if(obstacle.velocity.squaredNorm() > 0)
      ARROW("representation:ObstacleModel:velocity", center.x(), center.y(),
            center.x() + 2 * obstacle.velocity.x(), center.y() + 2 * obstacle.velocity.y(), 10, Drawings::solidPen, ColorRGBA::black);

    if(obstacle.type >= Obstacle::fallenSomeRobot)
    {
      DRAWTEXT("representation:ObstacleModel:fallen", center.x(), center.y(), 100, color, "FALLEN");
    }
  }
}

ObstacleModelCompressed::ObstacleModelCompressed(const ObstacleModel& other, size_t maxNumberOfObstacles)
{
  const size_t numOfInputObstacles = other.obstacles.size();
  if(numOfInputObstacles <= maxNumberOfObstacles)
  {
    obstacles.reserve(numOfInputObstacles);
    for(const auto& obstacle : other.obstacles)
    {
      obstacles.emplace_back(obstacle);
    }
  }
  else
  {
    size_t offset = Random::uniformInt(numOfInputObstacles - 1);
    obstacles.reserve(maxNumberOfObstacles);
    for(size_t i = 0; i < maxNumberOfObstacles; i++)
      obstacles.emplace_back(other.obstacles[(offset + i) % numOfInputObstacles]);
  }
}

ObstacleModelCompressed::ObstacleCompressed::ObstacleCompressed(const Obstacle& other) :
  covXX(other.covariance(0, 0)), covYY(other.covariance(1, 1)), covXY(other.covariance(0, 1)),
  center(other.center.cast<short>()), left(other.left.cast<short>()), right(other.right.cast<short>()), lastSeen(other.lastSeen), type(other.type)
{}
