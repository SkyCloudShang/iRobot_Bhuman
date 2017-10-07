/**
 * @file TeamData.cpp
 */

#include "TeamData.h"
#include "Representations/Infrastructure/FrameInfo.h"

#define HANDLE_PARTICLE(particle) case id##particle: return the##particle.handleArbitraryMessage(message, [this](unsigned u){return this->toLocalTimestamp(u);})
bool Teammate::handleMessage(InMessage& message)
{
  switch(message.getMessageID())
  {
      HANDLE_PARTICLE(RobotPose);
      HANDLE_PARTICLE(BallModel);
      HANDLE_PARTICLE(ObstacleModel);
      HANDLE_PARTICLE(BehaviorStatus);
      HANDLE_PARTICLE(SPLStandardBehaviorStatus);
      HANDLE_PARTICLE(Whistle);
      HANDLE_PARTICLE(TeammateRoles);
      HANDLE_PARTICLE(SideConfidence);
      HANDLE_PARTICLE(FieldCoverage);
      HANDLE_PARTICLE(RobotHealth);
    default:
      return false;
  }
}

void TeamData::draw() const
{
  DECLARE_DEBUG_DRAWING("representation:TeamData", "drawingOnField");
  for(auto const& teammate : teammates)
  {
    ColorRGBA posCol;
    if(teammate.status == Teammate::PLAYING)
      posCol = ColorRGBA::green;
    else if(teammate.status == Teammate::FALLEN)
      posCol = ColorRGBA::yellow;
    else
      posCol = ColorRGBA::red;

    const Vector2f& rPos = teammate.theRobotPose.translation;
    const float radius = std::max(50.f, teammate.theRobotPose.deviation);
    Vector2f dirPos = teammate.theRobotPose * Vector2f(radius, 0.f);

    // Circle around Player
    CIRCLE("representation:TeamData", rPos.x(), rPos.y(), radius, 20, Drawings::solidPen,
           posCol, Drawings::noBrush, ColorRGBA::white);
    // Direction of the Robot
    LINE("representation:TeamData", rPos.x(), rPos.y(), dirPos.x(), dirPos.y(), 20,
         Drawings::solidPen, posCol);
    // Player number
    DRAWTEXT("representation:TeamData", rPos.x() + 100, rPos.y(), 100, ColorRGBA::black, teammate.number);
    // Role
    DRAWTEXT("representation:TeamData", rPos.x() + 100, rPos.y() - 150, 100,
             ColorRGBA::black, Role::getName(teammate.theBehaviorStatus.role));

    // Time to reach ball
    int ttrb = teammate.theBehaviorStatus.role == Role::striker
               ? static_cast<int>(teammate.theBehaviorStatus.timeToReachBall.timeWhenReachBallStriker)
               : static_cast<int>(teammate.theBehaviorStatus.timeToReachBall.timeWhenReachBall);
    if(Blackboard::getInstance().exists("FrameInfo"))
    {
      const FrameInfo& theFrameInfo = (const FrameInfo&) Blackboard::getInstance()["FrameInfo"];
      ttrb = theFrameInfo.getTimeSince(ttrb);
    }
    DRAWTEXT("representation:TeamData", rPos.x() + 100, rPos.y() - 300, 100, ColorRGBA::black, "TTRB: " << ttrb);

    //Line from Robot to WalkTarget
    LINE("representation:TeamData", rPos.x(), rPos.y(),
         teammate.theSPLStandardBehaviorStatus.walkingTo.x(),
         teammate.theSPLStandardBehaviorStatus.walkingTo.y(),
         10, Drawings::dashedPen, ColorRGBA::magenta);

    // Ball position
    const Vector2f ballPos = teammate.theRobotPose * teammate.theBallModel.estimate.position;
    CIRCLE("representation:TeamData", ballPos.x(), ballPos.y(), 50, 20, Drawings::solidPen, ColorRGBA::yellow, Drawings::solidBrush, ColorRGBA::yellow);
  }
}
