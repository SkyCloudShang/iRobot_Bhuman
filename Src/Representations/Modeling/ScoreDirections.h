/* 
 * Copyright:Hust_iRobot
 * File:   ScoreDirections.h
 * Author: Shangyunfei
 * Description:Declaration of a representation that lists sectors of directions in which the ball
 * can be kicked and hit the goal. The directions are specified as the global positions
 * of obstacle borders that limit the free ranges to the left or right as seen from the
 * position of the ball, that is the global direction can be determined by
 * (limit - globalBall).angle().
 * Created on October 6, 2017, 9:48 AM
 */

#ifndef SCOREDIRECTIONS_H
#define SCOREDIRECTIONS_H

#include "Tools/Math/Eigen.h"
#include "Tools/Math/Pose2f.h"
#include "Tools/Math/BHMath.h"
#include "Tools/Streams/AutoStreamable.h"

STREAMABLE(ScoreDirections,
{
public:
  STREAMABLE(Sector,
  {
  public:
    Sector() = default;
    Sector(const Vector2f& leftLimit, const Vector2f& rightLimit),

    (Vector2f) leftLimit, /** Global position of left limiting object border. */
    (Vector2f) rightLimit, /** Global position of right limiting object border. */
  });

  void draw() const,

  (std::vector<Sector>) sectors, /**< All sectors sorted by their angular size (biggest first). */
});

#endif /* SCOREDIRECTIONS_H */

