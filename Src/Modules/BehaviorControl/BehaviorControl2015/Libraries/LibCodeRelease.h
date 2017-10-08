/**
* @file LibCodeRelease.h
*/

class LibCodeRelease : public LibraryBase
{
public:
  /** Constructor for initializing all members*/
  LibCodeRelease();

  void preProcess() override;

  void postProcess() override;
  
  
  bool between(float value, float min, float max);
    
  int timeSinceBallWasSeen();
  
   //angle to oppenent goal
  float angleToGoal;
  
   //angle to the center point
  float angleToCenter;
  
  //angle to ball
  float angleToBall;
  
  //
  float odometryRSum;
  float odometryXSum;
};