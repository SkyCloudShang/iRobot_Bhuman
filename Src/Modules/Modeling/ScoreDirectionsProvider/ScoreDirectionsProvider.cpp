/* 
 * Copyright:Hust_iRobot
 * File:   ScoreDirectionsProvider.cpp
 * Author: Shangyunfei
 * Description:
 * Created on October 6, 2017, 11:21 AM
 */

#include "ScoreDirectionsProvider.h"
#include <algorithm>

#define DRAWING_EDGE Vector2f(12000.f,0)

void ScoreDirectionsProvider::update(ScoreDirections& scoreDirections)
{
    //DECLARE_DEBUG_DRAWING("module:ScoreDirectionsProvider::sectors","drawingOnField");
    
    const Vector2f ballPosition=theRobotPose*theBallModel.estimate.position;
    const Vector2f leftGoalPostOffset(Vector2f(theFieldDimensions.xPosOpponentGoalPost,theFieldDimensions.yPosLeftGoal)-ballPosition);
    const Vector2f rightGoalPostOffset(Vector2f(theFieldDimensions.xPosOpponentGoalPost,theFieldDimensions.yPosRightGoal)-ballPosition);
    const float angleToLeftGoalPost=leftGoalPostOffset.angle();
    const float angleToRightGoalPost=rightGoalPostOffset.angle();
    const Pose2f origin(theRobotPose.rotation);
    
    // Describes one of the two edges of an obstacle sector.
    struct Edge
    {
        float distance;
        float direction;
        int step;
        
        Edge()=default;
        Edge(float distance,float direction,int step):distance(distance),direction(direction),
        step(step){}
    };
    
    
    std::vector<Edge>edges;
    
    // Insert obstacles including seen goal posts
   for(const auto& obstacle : theObstacleModel.obstacles)
   {
    Vector2f position = theRobotPose * obstacle.center;
    if(position.x() < (theFieldDimensions.xPosOpponentGoal + theFieldDimensions.xPosOpponentGoalPost) / 2.f)
    {
      const Vector2f offsetToBall = position - ballPosition;
      const float direction = offsetToBall.angle();
      const float width = (obstacle.type == Obstacle::goalpost
                          ? theFieldDimensions.goalPostRadius * 2.f
                          : (obstacle.left - obstacle.right).norm()) + 2.f * theFieldDimensions.ballRadius;
      const float distance = std::sqrt(std::max(offsetToBall.squaredNorm() - sqr(width / 2.f), 1.f));
      const float radius = std::atan2(width / 2.f, distance);
      const float left = direction + radius;
      const float right = direction - radius;
      if(right < angleToLeftGoalPost && left > angleToRightGoalPost)
      {
        edges.push_back(Edge(distance, right, 1));
        edges.push_back(Edge(distance, left, -1));
//        COMPLEX_DRAWING("module:ScoreDirectionsProvider:sectors",
//        {
//          const Vector2f points[3] =
//          {
//            theBallModel.estimate.position,
//            Pose2D(right - theRobotPose.rotation) * DRAWING_EDGE + theBallModel.estimate.position,
//            Pose2D(left - theRobotPose.rotation) * DRAWING_EDGE + theBallModel.estimate.position
//          };
//          POLYGON("module:ScoreDirectionsProvider:sectors",
//                  3, points, 0, Drawings::PenStyle::noPen, ColorRGBA(), Drawings::PenStyle::solidPen, ColorRGBA(255, 0, 0, 64));
//        });
      }
    }
  }
    
 // Insert localization-based goal posts
  const float leftGoalPostDistance = std::sqrt(std::max(leftGoalPostOffset.squaredNorm() - sqr(theFieldDimensions.goalPostRadius + theFieldDimensions.ballRadius), 1.f));
  const float leftGoalPostRadius = std::atan2(theFieldDimensions.goalPostRadius + theFieldDimensions.ballRadius, leftGoalPostDistance);
  edges.push_back(Edge(leftGoalPostDistance, angleToLeftGoalPost - leftGoalPostRadius, 1));
  edges.push_back(Edge(leftGoalPostDistance, angleToLeftGoalPost + leftGoalPostRadius, -1));
    
  const float rightGoalPostDistance = std::sqrt(std::max(rightGoalPostOffset.squaredNorm() - sqr(theFieldDimensions.goalPostRadius + theFieldDimensions.ballRadius), 1.f));
  const float rightGoalPostRadius = std::atan2(theFieldDimensions.goalPostRadius + theFieldDimensions.ballRadius, rightGoalPostDistance);
  edges.push_back(Edge(rightGoalPostDistance, angleToRightGoalPost - rightGoalPostRadius, 1));
  edges.push_back(Edge(rightGoalPostDistance, angleToRightGoalPost + rightGoalPostRadius, -1));

  
  
  float lastCenter = 1000.f;
  if(!scoreDirections.sectors.empty())
  {
    const ScoreDirections::Sector& s(scoreDirections.sectors.front());
    const float left = (s.leftLimit - ballPosition).angle();
    const float right = (s.rightLimit - ballPosition).angle();
    lastCenter = (left + right) / 2.f;
  }

// Fill representation
  scoreDirections.sectors.clear();

  
   if(!edges.empty())
  {
    // Sort by angle. When two directions are equal, the closer obstacle is started first,
    // but ended last.
    std::sort(edges.begin(), edges.end(),
              [](const Edge& e1, const Edge& e2) -> bool
    {
      return e1.direction < e2.direction ||
             (e1.direction == e2.direction && e1.distance * e1.step > e2.distance * e2.step);
    });

    // Sweep through obstacles. Whenever the counter is zero, the sector is free.
    int counter = edges.front().step;
    for(auto i = edges.begin(), j = i++; i != edges.end(); ++i, ++j)
    {
      ASSERT(counter >= 0);
      float range = i->direction - j->direction;
      if(counter == 0 && range > 0)
        scoreDirections.sectors.push_back(ScoreDirections::Sector(
          Pose2f(i->direction, ballPosition) * Vector2f(i->distance, 0),
          Pose2f(j->direction, ballPosition) * Vector2f(j->distance, 0)));
      counter += i->step;
    }

        // Sort by angular size in decending order
    std::sort(scoreDirections.sectors.begin(), scoreDirections.sectors.end(),
              [&](const ScoreDirections::Sector& s1, const ScoreDirections::Sector& s2) -> bool
    {
      const float left1 = (s1.leftLimit - ballPosition).angle();
      const float right1 = (s1.rightLimit - ballPosition).angle();
      const float left2 = (s2.leftLimit - ballPosition).angle();
      const float right2 = (s2.rightLimit - ballPosition).angle();
      const float range1 = (left1 - right1) * (left1 >= lastCenter && right1 <= lastCenter ? bonusRatio : 1.f);
      const float range2 = (left2 - right2) * (left2 >= lastCenter && right2 <= lastCenter ? bonusRatio : 1.f);
      return range1 > range2;
    });
   }

}