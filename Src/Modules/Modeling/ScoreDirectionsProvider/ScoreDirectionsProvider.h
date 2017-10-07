/* 
 * Copyright:Hust_iRobot
 * File:   ScoreDirectionsProvider.h
 * Author: Shangyunfei
 * Description:The file declares a module that determines sectors of directions in which the ball can be
 * kicked and hit the goal. This is based on the obstacle model. The problem is solved using
 * a sweep line method. All obstacles are described as angular sectors surrounding the ball.
 * The two edges of each sector, i.e. the direction in which an obstacle begins and the
 * direction in which it ends, are added to an array that is then sorted by the angles. The
 * goal posts are added as well. After that, the array is traversed while updating an
 * obstacle counter. Whenever a starting edge is encountered, the counter is increased, and
 * when an ending edge is found, the counter is lowered. Whenever the counter is zero, a
 * free sector was found.
 * Created on October 6, 2017, 11:21 AM
 */

#ifndef SCOREDIRECTIONSPROVIDER_H
#define SCOREDIRECTIONSPROVIDER_H

#include "Tools/Module/Module.h"
#include "Representations/Configuration/FieldDimensions.h"
#include "Representations/Modeling/BallModel.h"
#include "Representations/Modeling/ObstacleModel.h"
#include "Representations/Modeling/RobotPose.h"
#include "Representations/Modeling/ScoreDirections.h"

MODULE(ScoreDirectionsProvider,
{,
   REQUIRES(BallModel),
   REQUIRES(ObstacleModel),
   REQUIRES(FieldDimensions),
   REQUIRES(RobotPose),
   PROVIDES(ScoreDirections),
   DEFINES_PARAMETERS(
    {,
     (float)(1.0f)bonusRatio,
    }),
});

class ScoreDirectionsProvider:public ScoreDirectionsProviderBase
{
    void update(ScoreDirections& scoreDirections);
};


#endif /* SCOREDIRECTIONSPROVIDER_H */

