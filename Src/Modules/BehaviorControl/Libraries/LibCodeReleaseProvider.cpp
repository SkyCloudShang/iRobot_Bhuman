/**
 * @file LibCodeRelease.cpp
 */

#include "LibCodeReleaseProvider.h"

MAKE_MODULE(LibCodeReleaseProvider, behaviorControl);

using namespace Transformation;

void LibCodeReleaseProvider::update(LibCodeRelease& libCodeRelease)
{
  libCodeRelease.timeSinceBallWasSeen = theFrameInfo.getTimeSince(theBallModel.timeWhenLastSeen);
  libCodeRelease.angleToGoal = (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundline, 0.f)).angle();
  libCodeRelease.between = [&](float value, float min, float max) -> bool
  {
    return value >= min && value <= max;
  };
  
   libCodeRelease.angleToCenter=(theRobotPose.inversePose * Vector2f(0.f, 0.f)).angle();
   libCodeRelease.angleToBall=(theRobotPose.inversePose * Vector2f(robotToField(theRobotPose,theBallModel.estimate.position))).angle();
  
        static float odometryR = 0.f;
	static float odometryX = 0.f;
	odometryR = static_cast<float>(theOdometer.odometryOffset.rotation);
	odometryX = theOdometer.odometryOffset.translation.x();
	if (theGameInfo.state == STATE_PLAYING && theRobotInfo.penalty == PENALTY_NONE)
	{
		libCodeRelease.odometryRSum += odometryR;
		libCodeRelease.odometryXSum += odometryX;
	}
	else
	{
		libCodeRelease.odometryRSum = 0.f;
		libCodeRelease.odometryXSum = 0.f;
	}
	if ((theRobotInfo.number == 2) && (theTeamData.teammates.empty() || theTeamData.teammates[0].theBehaviorStatus.firstRobotArrived==0))
	{
		libCodeRelease.odometryRSum = 0.f;
		libCodeRelease.odometryXSum = 0.f;
	}
}
