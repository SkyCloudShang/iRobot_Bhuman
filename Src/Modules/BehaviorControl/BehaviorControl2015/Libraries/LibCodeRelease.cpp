/**
* @file LibCodeRelease.cpp
*/

#include "../LibraryBase.h"

using namespace Transformation;

namespace Behavior2015
{
  #include "LibCodeRelease.h"
  
  LibCodeRelease::LibCodeRelease():
    angleToGoal(0.f)
  {}
  
  void LibCodeRelease::preProcess()
  {
    angleToGoal = (theRobotPose.inversePose * Vector2f(theFieldDimensions.xPosOpponentGroundline, 0.f)).angle();
    angleToCenter=(theRobotPose.inversePose * Vector2f(0.f, 0.f)).angle();
    angleToBall=(theRobotPose.inversePose * Vector2f(robotToField(theRobotPose,theBallModel.estimate.position))).angle();
  
        static float odometryR = 0.f;
	static float odometryX = 0.f;
	odometryR = static_cast<float>(theOdometer.odometryOffset.rotation);
	odometryX = theOdometer.odometryOffset.translation.x();
	if (theGameInfo.state == STATE_PLAYING && theRobotInfo.penalty == PENALTY_NONE)
	{
		odometryRSum += odometryR;
		odometryXSum += odometryX;
	}
	else
	{
		odometryRSum = 0.f;
		odometryXSum = 0.f;
	}
	if ((theRobotInfo.number == 2) && (theTeammateData.teammates.empty() || !theTeammateData.teammates[0].firstRobotArrived))
	{
		odometryRSum = 0.f;
		odometryXSum = 0.f;
	}
  }

  void LibCodeRelease::postProcess()
  {
  }
  
  int LibCodeRelease::timeSinceBallWasSeen()
  {
    return theFrameInfo.getTimeSince(theBallModel.timeWhenLastSeen);
  }
  
  bool LibCodeRelease::between(float value, float min, float max)
  {
    return value >= min && value <= max;
  }  
}