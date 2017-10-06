/** All option files that belong to the current behavior have to be included by this file. */

#include "Options/Soccer.h"
#include "Options/GameControl/HandleGameState.h"
#include "Options/GameControl/HandlePenaltyState.h"
#include "Options/GameControl/PlayingState.h"
#include "Options/GameControl/ReadyState.h"

#include "Options/HeadControl/LookForward.h"
#include "Options/HeadControl/LookAtBall.h"
#include "Options/HeadControl/LookLeftAndRight.h"
#include "Options/HeadControl/LookUpAndDown.h"

#include "Options/Output/ArmMotionRequest/PointAt.h"
#include "Options/Output/ArmMotionRequest/KeyFrameArms.h"

#include "Options/Output/HeadMotionRequest/SetHeadPanTilt.h"
#include "Options/Output/HeadMotionRequest/SetHeadTargetSpeedRequest.h"

#include "Options/Output/MotionRequest/SpecialAction.h"
#include "Options/Output/MotionRequest/Stand.h"
#include "Options/Output/MotionRequest/WalkAtSpeedPercentage.h"
#include "Options/Output/MotionRequest/WalkToTarget.h"
#include "Options/Output/MotionRequest/InWalkKick.h"
#include "Options/Output/MotionRequest/GetUpEngine.h"
#include"Options/Output/MotionRequest/WalkToTargetWithPathMode.h"
#include "Options/Output/Annotation.h"
#include "Options/Output/PlaySound.h"
#include "Options/Output/PlaySamples.h"

#include "Options/Skills/GetUp.h"
#include "Options/Skills/ArmContact.h"

#include "Options/Roles/Striker.h"
#include "Options/Roles/Keeper.h"
#include "Options/Roles/Supporter.h"
#include "Options/Roles/Robot1.h"
#include "Options/Roles/Robot2.h"

#include "Options/DemoOptions/Demo.h"
#include "Options/DemoOptions/Waving.h"

#include "Options/Tools/ButtonPressedAndReleased.h"
