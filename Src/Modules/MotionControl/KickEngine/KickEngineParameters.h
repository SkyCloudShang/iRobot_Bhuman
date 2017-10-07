/**
 * @file KickEngineParameters.h
 * @author <a href="mailto:judy@informatik.uni-bremen.de">Judith Müller</a>
 */

#pragma once

#include <vector>

#include "Tools/Math/Eigen.h"
#include "Tools/Streams/Enum.h"

class Phase : public Streamable
{
public:
  ENUM(Limb,
  {,
    leftFootTra,
    leftFootRot,
    rightFootTra,
    rightFootRot,
    leftArmTra,
    leftHandRot,
    rightArmTra,
    rightHandRot,
  });

  enum { numOfPoints = 3 };

  unsigned int duration;

  Vector3f controlPoints[Phase::numOfLimbs][numOfPoints];
  Vector2f comTra[numOfPoints];
  Vector2f headTra[numOfPoints];

  Vector3f originPos[Phase::numOfLimbs];
  Vector2f comOriginPos = Vector2f::Zero();
  Vector2f comOriginOffset = Vector2f::Zero();
  Vector2f headOrigin = Vector2f::Zero();
  Vector3f odometryOffset = Vector3f::Zero();

protected:
  virtual void serialize(In* in, Out* out);
};

STREAMABLE(KickEngineParameters,
{
  int numberOfPhases = 0;
  char name[260];

  void calcControlPoints();

  Vector3f getPosition(const float& phase, const int& phaseNumber, const int& limb);

  Vector2f getComRefPosition(const float& phase, const int& phaseNumber);
  Vector2f getHeadRefPosition(const float& phase, const int& phaseNumber);

  void initFirstPhase();
  void initFirstPhase(const Vector3f* origins, const Vector2f& head);
  void initFirstPhaseLoop(const Vector3f* origins, const Vector2f& lastCom, const Vector2f& head);

  void onRead(),

  (Vector3f)(Vector3f::Zero()) footOrigin,
  (Vector3f)(Vector3f::Zero()) footRotOrigin,
  (Vector3f)(Vector3f::Zero()) armOrigin,
  (Vector3f)(Vector3f::Zero()) handRotOrigin,
  (Vector2f)(Vector2f::Zero()) comOrigin,
  (Vector2f)(Vector2f::Zero()) headOrigin,

  (float)(0.f) kpx,
  (float)(0.f) kix,
  (float)(0.f) kdx,
  (float)(0.f) kpy,
  (float)(0.f) kiy,
  (float)(0.f) kdy,

  (bool)(false) loop,
  (bool)(true)  standLeft,
  (bool)(false) ignoreHead,

  (std::vector<Phase>) phaseParameters,
});
