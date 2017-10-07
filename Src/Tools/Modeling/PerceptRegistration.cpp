/**
 * @file PerceptRegistration.cpp
 *
 * Implementation of a class that uses recent field feature obervations
 * and associates them with their real world pendants.
 *
 * @author <A href="mailto:tlaue@uni-bremen.de">Tim Laue</A>
 */

#include "PerceptRegistration.h"
#include "UKFPose2D.h"
#include "Tools/Debugging/DebugDrawings.h"

PerceptRegistration::PerceptRegistration(const CameraMatrix& cameraMatrix,
    const CirclePercept& circlePercept,
    const FieldDimensions& fieldDimensions,
    const FrameInfo& frameInfo,
    const GameInfo& gameInfo,
    const OwnTeamInfo& ownTeamInfo,
    const GoalFeature& goalFeature,
    const GoalFrame& goalFrame,
    const FieldLineIntersections& fieldLineIntersections,
    const FieldLines& fieldLines,
    const MidCircle& midCircle,
    const MidCorner& midCorner,
    const OuterCorner& outerCorner,
    const PenaltyArea& penaltyArea,
    const PenaltyMarkPercept& penaltyMarkPercept,
    const GoalPostPercept& theGoalPostPercept,
    bool         goalFrameIsPerceivedAsLines,
    const float& lineAssociationCorridor,
    const float& longLineAssociationCorridor,
    const float& centerCircleAssociationDistance,
    const float& penaltyMarkAssociationDistance,
    const float& intersectionAssociationDistance,
    const float& globalPoseAssociationMaxDistanceDeviation,
    const Angle& globalPoseAssociationMaxAngularDeviation) :
  theCameraMatrix(cameraMatrix), theCirclePercept(circlePercept), theFieldDimensions(fieldDimensions),
  theFrameInfo(frameInfo), theGameInfo(gameInfo), theOwnTeamInfo(ownTeamInfo), theGoalFeature(goalFeature),
  theGoalFrame(goalFrame), theFieldLineIntersections(fieldLineIntersections),
  theFieldLines(fieldLines), theMidCircle(midCircle), theMidCorner(midCorner), theOuterCorner(outerCorner),
  thePenaltyArea(penaltyArea), thePenaltyMarkPercept(penaltyMarkPercept), theGoalPostPercept(theGoalPostPercept),
  lineAssociationCorridor(lineAssociationCorridor), longLineAssociationCorridor(longLineAssociationCorridor),
  centerCircleAssociationDistance(centerCircleAssociationDistance),
  penaltyMarkAssociationDistance(penaltyMarkAssociationDistance), intersectionAssociationDistance(intersectionAssociationDistance),
  globalPoseAssociationMaxDistanceDeviation(globalPoseAssociationMaxDistanceDeviation),
  globalPoseAssociationMaxAngularDeviation(globalPoseAssociationMaxAngularDeviation)
{
  // Initialize goal posts
  goalPosts[0] = Vector2f(theFieldDimensions.xPosOwnGoalPost, theFieldDimensions.yPosRightGoal);
  goalPosts[1] = Vector2f(theFieldDimensions.xPosOwnGoalPost, theFieldDimensions.yPosLeftGoal);
  goalPosts[2] = Vector2f(theFieldDimensions.xPosOpponentGoalPost, theFieldDimensions.yPosLeftGoal);
  goalPosts[3] = Vector2f(theFieldDimensions.xPosOpponentGoalPost, theFieldDimensions.yPosRightGoal);
  penaltyAreaWidth = 2.f * std::abs(theFieldDimensions.yPosLeftPenaltyArea);
  goalAcceptanceThreshold = (2.f * theFieldDimensions.yPosLeftGoal) / 3.f;

  // Initialize penalty marks
  ownPenaltyMark =      Vector2f(theFieldDimensions.xPosOwnPenaltyMark, 0.f);
  opponentPenaltyMark = Vector2f(theFieldDimensions.xPosOpponentPenaltyMark, 0.f);

  // Initialize list of relevant field lines
  for(size_t i = 0, count = theFieldDimensions.fieldLines.lines.size(); i < count; ++i)
  {
    const FieldDimensions::LinesTable::Line& fieldLine = theFieldDimensions.fieldLines.lines[i];
    if(!fieldLine.isPartOfCircle && (fieldLine.to - fieldLine.from).norm() > 300.f)
    {
      FieldLine relevantFieldLine;
      relevantFieldLine.start = fieldLine.from;
      relevantFieldLine.end = fieldLine.to;
      relevantFieldLine.dir = relevantFieldLine.end - relevantFieldLine.start;
      relevantFieldLine.dir.normalize();
      relevantFieldLine.length = (fieldLine.to - fieldLine.from).norm();
      relevantFieldLine.isLong = relevantFieldLine.length > penaltyAreaWidth;
      relevantFieldLine.vertical = std::abs(fieldLine.from.y() - fieldLine.to.y()) < 0.001f;
      if(relevantFieldLine.vertical)
        verticalFieldLines.push_back(relevantFieldLine);
      else
        horizontalFieldLines.push_back(relevantFieldLine);
    }
  }
  if(goalFrameIsPerceivedAsLines)
  {
    for(size_t i = 0, count = theFieldDimensions.goalFrameLines.lines.size(); i < count; ++i)
    {
      const FieldDimensions::LinesTable::Line& fieldLine = theFieldDimensions.goalFrameLines.lines[i];
      FieldLine relevantFieldLine;
      relevantFieldLine.start = fieldLine.from;
      relevantFieldLine.end = fieldLine.to;
      relevantFieldLine.dir = relevantFieldLine.end - relevantFieldLine.start;
      relevantFieldLine.dir.normalize();
      relevantFieldLine.length = (fieldLine.to - fieldLine.from).norm();
      relevantFieldLine.isLong = relevantFieldLine.length > penaltyAreaWidth;
      relevantFieldLine.vertical = std::abs(fieldLine.from.y() - fieldLine.to.y()) < 0.001f;
      if(relevantFieldLine.vertical)
        verticalFieldLines.push_back(relevantFieldLine);
      else
        horizontalFieldLines.push_back(relevantFieldLine);
    }
  }
  // Seach center line in list of all lines:
  centerLine = 0;
  for(auto& fieldLine : horizontalFieldLines)
  {
    if(fieldLine.start.x() != theFieldDimensions.xPosHalfWayLine)
      continue;
    if(fieldLine.end.x() != theFieldDimensions.xPosHalfWayLine)
      continue;
    centerLine = &fieldLine;
    return;
  }
  ASSERT(centerLine != 0);

  // Initialize corner lists:
  // X
  xIntersections.push_back(Vector2f(theFieldDimensions.xPosHalfWayLine, theFieldDimensions.centerCircleRadius));
  xIntersections.push_back(Vector2f(theFieldDimensions.xPosHalfWayLine, -theFieldDimensions.centerCircleRadius));
  // T
  tIntersections = xIntersections;
  tIntersections.push_back(Vector2f(theFieldDimensions.xPosHalfWayLine, theFieldDimensions.yPosRightSideline));
  tIntersections.push_back(Vector2f(theFieldDimensions.xPosHalfWayLine, theFieldDimensions.yPosLeftSideline));
  tIntersections.push_back(Vector2f(theFieldDimensions.xPosOwnGroundline, theFieldDimensions.yPosLeftPenaltyArea));
  tIntersections.push_back(Vector2f(theFieldDimensions.xPosOwnGroundline, theFieldDimensions.yPosRightPenaltyArea));
  tIntersections.push_back(Vector2f(theFieldDimensions.xPosOpponentGroundline, theFieldDimensions.yPosLeftPenaltyArea));
  tIntersections.push_back(Vector2f(theFieldDimensions.xPosOpponentGroundline, theFieldDimensions.yPosRightPenaltyArea));
  // L
  lIntersections = tIntersections;
  lIntersections.push_back(Vector2f(theFieldDimensions.xPosOpponentGroundline, theFieldDimensions.yPosRightSideline));
  lIntersections.push_back(Vector2f(theFieldDimensions.xPosOpponentGroundline, theFieldDimensions.yPosLeftSideline));
  lIntersections.push_back(Vector2f(theFieldDimensions.xPosOwnGroundline, theFieldDimensions.yPosRightSideline));
  lIntersections.push_back(Vector2f(theFieldDimensions.xPosOwnGroundline, theFieldDimensions.yPosLeftSideline));
  lIntersections.push_back(Vector2f(theFieldDimensions.xPosOwnPenaltyArea, theFieldDimensions.yPosRightPenaltyArea));
  lIntersections.push_back(Vector2f(theFieldDimensions.xPosOwnPenaltyArea, theFieldDimensions.yPosLeftPenaltyArea));
  lIntersections.push_back(Vector2f(theFieldDimensions.xPosOpponentPenaltyArea, theFieldDimensions.yPosRightPenaltyArea));
  lIntersections.push_back(Vector2f(theFieldDimensions.xPosOpponentPenaltyArea, theFieldDimensions.yPosLeftPenaltyArea));
  if(goalFrameIsPerceivedAsLines)
  {
    lIntersections.push_back(Vector2f(theFieldDimensions.xPosOwnGoal, theFieldDimensions.yPosLeftGoal));
    lIntersections.push_back(Vector2f(theFieldDimensions.xPosOwnGoal, theFieldDimensions.yPosRightGoal));
    lIntersections.push_back(Vector2f(theFieldDimensions.xPosOpponentGoal, theFieldDimensions.yPosLeftGoal));
    lIntersections.push_back(Vector2f(theFieldDimensions.xPosOpponentGoal, theFieldDimensions.yPosRightGoal));
  }

  // Initialize time stamps
  lastGoalPostCovarianceUpdate = 0;
  lastPenaltyMarkCovarianceUpdate = 0;
  lastCirclePerceptCovarianceUpdate = 0;
}

void PerceptRegistration::update(const Pose2f& theRobotPose, RegisteredPercepts& registeredPercepts,
                                 const Pose3f& inverseCameraMatrix, const Vector2f& currentRotationDeviation)
{
  robotPose = theRobotPose;
  this->inverseCameraMatrix = inverseCameraMatrix;
  this->currentRotationDeviation = currentRotationDeviation;

  if(theFieldLines.lines.size() > lineCovarianceUpdates.size())
  {
    lineCovarianceUpdates.resize(theFieldLines.lines.size(), 0);
    lineCovariances.resize(theFieldLines.lines.size());
  }
  if(theFieldLineIntersections.intersections.size() > intersectionCovarianceUpdates.size())
  {
    intersectionCovarianceUpdates.resize(theFieldLineIntersections.intersections.size(), 0);
    intersectionCovariances.resize(theFieldLineIntersections.intersections.size());
  }

  registeredPercepts.lines.clear();
  registeredPercepts.landmarks.clear();
  registeredPercepts.poses.clear();
  registeredPercepts.totalNumberOfPerceivedLines     = registerLines(registeredPercepts.lines);
  registeredPercepts.totalNumberOfPerceivedLandmarks = registerLandmarks(registeredPercepts.landmarks);
  registeredPercepts.totalNumberOfPerceivedPoses     = registerPoses(registeredPercepts.poses);
  draw();
}

int PerceptRegistration::registerPoses(std::vector<RegisteredPose>& poses)
{
  return registerPose(poses, theMidCircle) +
         registerPose(poses, theMidCorner) +
         registerPose(poses, theGoalFrame) +
         registerPose(poses, theOuterCorner) +
         registerPose(poses, thePenaltyArea) +
         registerPose(poses, theGoalFeature);
}

int PerceptRegistration::registerPose(std::vector<RegisteredPose>& poses, const FieldFeature& feature)
{
  if(feature.isValid)
  {
    RegisteredPose newPose;
    if(pickPoseFromFieldFeature(robotPose, feature, newPose.pose))
    {
      if((robotPose.translation - newPose.pose.translation).norm() < globalPoseAssociationMaxDistanceDeviation)
      {
        Angle angularDifference(robotPose.rotation - newPose.pose.rotation);
        angularDifference.normalize();
        if(std::abs(angularDifference) < globalPoseAssociationMaxAngularDeviation)
        {
          newPose.p = feature;
          poses.push_back(newPose);
        }
      }
    }
    return 1;
  }
  return 0;
}

bool PerceptRegistration::pickPoseFromFieldFeature(const Pose2f& robotPose, const FieldFeature& fieldFeature, Pose2f& pickedPose) const
{
  const Pose2f& p1 = fieldFeature.getGlobalRobotPosition().pos1;
  const Pose2f& p2 = fieldFeature.getGlobalRobotPosition().pos2;
  const float sqrDst1 = (robotPose.translation - p1.translation).squaredNorm();
  const float sqrDst2 = (robotPose.translation - p2.translation).squaredNorm();
  float angle1 = robotPose.rotation - p1.rotation;
  float angle2 = robotPose.rotation - p2.rotation;
  angle1 = std::abs(Angle::normalize(angle1));
  angle2 = std::abs(Angle::normalize(angle2));
  if(angle1 < angle2 && sqrDst1 < sqrDst2)
  {
    pickedPose = p1;
    return true;
  }
  else if(angle2 < angle1 && sqrDst2 < sqrDst1)
  {
    pickedPose = p2;
    return true;
  }
  return false;
}

int PerceptRegistration::registerLandmarks(std::vector<RegisteredLandmark>& landmarks)
{
  int numOfLandmarks(0);
  if(thePenaltyMarkPercept.wasSeen)
  {
    Vector2f penaltyMarkInWorld;
    if(getAssociatedPenaltyMark(thePenaltyMarkPercept.positionOnField, penaltyMarkInWorld))
    {
      RegisteredLandmark newLandmark;
      newLandmark.w = penaltyMarkInWorld;
      newLandmark.p = thePenaltyMarkPercept.positionOnField;
      if(lastPenaltyMarkCovarianceUpdate != theFrameInfo.time)
      {
        penaltyMarkCovariance = UKFPose2D::getCovOfPointInWorld(newLandmark.p, 0.f, theCameraMatrix, inverseCameraMatrix, currentRotationDeviation);
        lastPenaltyMarkCovarianceUpdate = theFrameInfo.time;
      }
      newLandmark.cov = penaltyMarkCovariance;
      landmarks.push_back(newLandmark);
    }
    numOfLandmarks++;
  }
  if(theGoalPostPercept.wasSeen)
  {
    Vector2f goalPostInWorld;
    if(getAssociatedGoalPost(theGoalPostPercept.positionOnField, goalPostInWorld))
    {
      RegisteredLandmark newLandmark;
      newLandmark.w = goalPostInWorld;
      newLandmark.p = theGoalPostPercept.positionOnField;
      if(lastGoalPostCovarianceUpdate != theFrameInfo.time)
      {
        goalPostCovariance = UKFPose2D::getCovOfPointInWorld(newLandmark.p, 0.f, theCameraMatrix, inverseCameraMatrix, currentRotationDeviation);
        lastGoalPostCovarianceUpdate = theFrameInfo.time;
      }
      newLandmark.cov = goalPostCovariance;
      landmarks.push_back(newLandmark);
    }
    numOfLandmarks++;
  }
  if(theCirclePercept.wasSeen && !theMidCircle.isValid)
  {
    const Vector2f circleInWorld = robotPose * theCirclePercept.pos;
    if(circleInWorld.norm() <= centerCircleAssociationDistance)
    {
      RegisteredLandmark newLandmark;
      newLandmark.w = Vector2f(0.f, 0.f);
      newLandmark.p = theCirclePercept.pos;
      if(lastCirclePerceptCovarianceUpdate != theFrameInfo.time)
      {
        circlePerceptCovariance = UKFPose2D::getCovOfPointInWorld(newLandmark.p, 0.f, theCameraMatrix, inverseCameraMatrix, currentRotationDeviation);
        lastCirclePerceptCovarianceUpdate = theFrameInfo.time;
      }
      newLandmark.cov = circlePerceptCovariance;
      landmarks.push_back(newLandmark);
    }
    numOfLandmarks++;
  }
  // If we detected some of the special corner constellations (which will be used differently),
  // we do not need any of the "normal" intersections anymore.
  if(!theGoalFrame.isValid && !theMidCorner.isValid && !theOuterCorner.isValid && !thePenaltyArea.isValid)
  {
    for(unsigned int i = 0; i < theFieldLineIntersections.intersections.size(); ++i)
    {
      const auto& intersection = theFieldLineIntersections.intersections[i];
      Vector2f intersectionInWorld;
      if(getAssociatedIntersection(intersection, intersectionInWorld))
      {
        RegisteredLandmark newLandmark;
        newLandmark.w = intersectionInWorld;
        newLandmark.p = intersection.pos;
        if(intersectionCovarianceUpdates[i] < theFrameInfo.time)
        {
          intersectionCovariances[i] = UKFPose2D::getCovOfPointInWorld(newLandmark.p, 0.f, theCameraMatrix, inverseCameraMatrix, currentRotationDeviation);
          intersectionCovarianceUpdates[i] = theFrameInfo.time;
        }
        newLandmark.cov = intersectionCovariances[i];
        landmarks.push_back(newLandmark);
      }
    }
    numOfLandmarks += static_cast<int>(theFieldLineIntersections.intersections.size());
  }
  return numOfLandmarks;
}

int PerceptRegistration::registerLines(std::vector<RegisteredLine>& lines)
{
  // If we have some of the high level percepts, do not register the low level ones
  if(theGoalFrame.isValid || theMidCorner.isValid || theOuterCorner.isValid || thePenaltyArea.isValid || theMidCircle.isValid)
    return 0;

  // Iterate over all observed lines:
  for(unsigned int i = 0; i < theFieldLines.lines.size(); ++i)
  {
    const auto& line = theFieldLines.lines[i];
    const FieldLine* fieldLine = getPointerToAssociatedLine(line.first, line.last);
    if(fieldLine)
    {
      RegisteredLine newLine;
      newLine.pStart    = line.first;
      newLine.pEnd      = line.last;
      newLine.pCenter   = (line.first + line.last) * 0.5f;
      newLine.pDir      = line.last - line.first;
      newLine.pDir.normalize();
      newLine.wStart    = (*fieldLine).start;
      newLine.wEnd      = (*fieldLine).end;
      newLine.vertical  = (*fieldLine).vertical;
      if(lineCovarianceUpdates[i] < theFrameInfo.time)
      {
        lineCovariances[i] = UKFPose2D::getCovOfPointInWorld(newLine.pCenter, 0.f, theCameraMatrix, inverseCameraMatrix, currentRotationDeviation);
        lineCovarianceUpdates[i] = theFrameInfo.time;
      }
      newLine.covCenter = lineCovariances[i];
      lines.push_back(newLine);
    }
  }
  return static_cast<int>(theFieldLines.lines.size());
}

bool PerceptRegistration::getAssociatedIntersection(const FieldLineIntersections::Intersection& intersection, Vector2f& associatedIntersection) const
{
  const std::vector< Vector2f >* corners = &lIntersections;
  if(intersection.type == FieldLineIntersections::Intersection::T)
    corners = &tIntersections;
  else if(intersection.type == FieldLineIntersections::Intersection::X)
    corners = &xIntersections;
  const Vector2f pointWorld = robotPose * intersection.pos;
  const float sqrThresh = intersectionAssociationDistance * intersectionAssociationDistance;
  for(unsigned int i = 0; i < corners->size(); ++i)
  {
    const Vector2f& c = corners->at(i);
    // simple implementation for testing:
    if((pointWorld - c).squaredNorm() < sqrThresh)
    {
      associatedIntersection = c;
      return true;
    }
  }
  return false;
}

bool PerceptRegistration::getAssociatedPenaltyMark(const Vector2f& penaltyMarkPercept, Vector2f& associatedPenaltyMark) const
{
  const Vector2f penaltyMarkInWorld = robotPose * penaltyMarkPercept;
  associatedPenaltyMark = penaltyMarkInWorld.x() <= 0.f ? ownPenaltyMark : opponentPenaltyMark;
  const float differenceBetweenPerceptAndModel = (penaltyMarkInWorld - associatedPenaltyMark).norm();
  return differenceBetweenPerceptAndModel < penaltyMarkAssociationDistance;
}

bool PerceptRegistration::getAssociatedGoalPost(const Vector2f& goalPostPercept, Vector2f& associatedGoalPost) const
{
  const Vector2f goalPostPerceptionInWorld = robotPose * goalPostPercept;
  int idxOfClosestPost = 0;
  float distToClosestPost = (goalPostPerceptionInWorld - goalPosts[0]).norm();
  for(int i = 1; i < 4; ++i)
  {
    const float distToPost = (goalPostPerceptionInWorld - goalPosts[i]).norm();
    if(distToPost < distToClosestPost)
    {
      distToClosestPost = distToPost;
      idxOfClosestPost = i;
    }
  }
  associatedGoalPost = goalPosts[idxOfClosestPost];
  return distToClosestPost <= goalAcceptanceThreshold;
}

const PerceptRegistration::FieldLine* PerceptRegistration::getPointerToAssociatedLine(const Vector2f& start, const Vector2f& end) const
{
  const Vector2f startOnField = robotPose * start;
  const Vector2f endOnField = robotPose * end;
  Vector2f dirOnField = endOnField - startOnField;
  dirOnField.normalize();
  const Vector2f orthogonalOnField(dirOnField.y(), -dirOnField.x());
  bool isVertical = std::abs(dirOnField.x()) > std::abs(dirOnField.y());
  const float lineLength = (start - end).norm();
  const bool perceivedLineIsLong = lineLength > penaltyAreaWidth;
  // Throw away lines on center circle (as they are currently not in the field lines)
  // To save computing time, do check for lines of up to medium length, only.
  // To avoid deleting short penalty area side lines, only consider lines that start somewhere near the field center
  if(lineLength < 2000.f && startOnField.norm() < 2.f * theFieldDimensions.centerCircleRadius
     && end.norm() < 2.f * theFieldDimensions.centerCircleRadius &&
     lineCouldBeOnCenterCircle(startOnField, dirOnField))
    return nullptr;
  // Throw away short lines before kickoff (false positives on center circle that are confused with the center line)
  // when the other team has kickoff
  if(lineLength < 1000.f && iAmBeforeKickoffInTheCenterOfMyHalfLookingForward() &&
     theGameInfo.kickOffTeam != theOwnTeamInfo.teamNumber)
    return nullptr;
  const float sqrLineAssociationCorridor     = sqr(lineAssociationCorridor);
  const float sqrLongLineAssociationCorridor = sqr(longLineAssociationCorridor);
  Vector2f intersection;
  const std::vector<FieldLine>& fieldLines = isVertical ? verticalFieldLines : horizontalFieldLines;

  for(unsigned int i = 0; i < fieldLines.size(); ++i)
  {
    const FieldLine& fieldLine = fieldLines[i];
    const float currentSqrCorridor = (fieldLine.isLong && perceivedLineIsLong) ? sqrLongLineAssociationCorridor : sqrLineAssociationCorridor;
    if(getSqrDistanceToLine(fieldLine.start, fieldLine.dir, fieldLine.length, startOnField) > currentSqrCorridor ||
       getSqrDistanceToLine(fieldLine.start, fieldLine.dir, fieldLine.length, endOnField) > currentSqrCorridor)
      continue;
    if(!intersectLineWithLine(startOnField, orthogonalOnField, fieldLine.start, fieldLine.dir, intersection))
      continue;
    if(getSqrDistanceToLine(startOnField, dirOnField, intersection) > currentSqrCorridor)
      continue;
    if(!intersectLineWithLine(endOnField, orthogonalOnField, fieldLine.start, fieldLine.dir, intersection))
      continue;
    if(getSqrDistanceToLine(startOnField, dirOnField, intersection) > currentSqrCorridor)
      continue;
    return &(fieldLines[i]);
  }

  // If this point has been reached, no matching line has been found.
  // However, in READY and SET, we try to consider a wide tolerance to match the center line. This
  // might solve some rarely occurring localization imprecisions before kickoff.
  if(!isVertical && iAmBeforeKickoffAndTheLineIsProbablyTheCenterLine(startOnField, endOnField, dirOnField, orthogonalOnField, lineLength))
  {
    return centerLine;
  }
  return nullptr;
}

// THIS IS SOMEHOW HACKED. COULD BE IMPROVED IN THE FUTURE. T.L.
bool PerceptRegistration::iAmBeforeKickoffAndTheLineIsProbablyTheCenterLine(const Vector2f& lineStart,
                                                                            const Vector2f& lineEnd,
                                                                            const Vector2f& direction,
                                                                            const Vector2f& orthogonal,
                                                                            float length) const
{
  if(!iAmBeforeKickoffInTheCenterOfMyHalfLookingForward())
    return false;
  // Check minimum length
  if(length < 2500.f)
    return false;
  // Check compatibility of line:
  Vector2f intersection;
  const float sqrLineAssociationCorridor = sqr(2000.f);
  if(getSqrDistanceToLine(centerLine->start, centerLine->dir, centerLine->length, lineStart) > sqrLineAssociationCorridor ||
     getSqrDistanceToLine(centerLine->start, centerLine->dir, centerLine->length, lineEnd) > sqrLineAssociationCorridor)
    return false;
  if(!intersectLineWithLine(lineStart, orthogonal, centerLine->start, centerLine->dir, intersection))
    return false;
  if(getSqrDistanceToLine(lineStart, direction, intersection) > sqrLineAssociationCorridor)
    return false;
  if(!intersectLineWithLine(lineEnd, orthogonal, centerLine->start, centerLine->dir, intersection))
    return false;
  if(getSqrDistanceToLine(lineStart, direction, intersection) > sqrLineAssociationCorridor)
    return false;
  return true;
}

bool PerceptRegistration::iAmBeforeKickoffInTheCenterOfMyHalfLookingForward() const
{
  // Game state must be correct:
  if(theGameInfo.state != STATE_SET && theGameInfo.state != STATE_READY)
    return false;
  // Robot must be in own half and look forward:
  if(robotPose.translation.x() >= 0.f || std::abs(robotPose.rotation) >= 25_deg ||
     robotPose.translation.x() <= theFieldDimensions.xPosOwnPenaltyArea)
    return false;
  return true;
}

float PerceptRegistration::getSqrDistanceToLine(const Vector2f& base, const Vector2f& dir, float length, const Vector2f& point) const
{
  float l = (point.x() - base.x()) * dir.x() + (point.y() - base.y()) * dir.y();
  if(l < 0)
    l = 0;
  else if(l > length)
    l = length;
  return ((base + dir * l) - point).squaredNorm();
}

bool PerceptRegistration::intersectLineWithLine(const Vector2f& lineBase1, const Vector2f& lineDir1,
    const Vector2f& lineBase2, const Vector2f& lineDir2, Vector2f& intersection) const
{
  const float h = lineDir1.x() * lineDir2.y() - lineDir1.y() * lineDir2.x();
  if(h == 0.f)
    return false;
  float scale = ((lineBase2.x() - lineBase1.x()) * lineDir1.y() - (lineBase2.y() - lineBase1.y()) * lineDir1.x()) / h;
  intersection.x() = lineBase2.x() + lineDir2.x() * scale;
  intersection.y() = lineBase2.y() + lineDir2.y() * scale;
  return true;
}

float PerceptRegistration::getSqrDistanceToLine(const Vector2f& base, const Vector2f& dir, const Vector2f& point) const
{
  const float l = (point.x() - base.x()) * dir.x() + (point.y() - base.y()) * dir.y();
  return ((base + dir * l) - point).squaredNorm();
}

bool PerceptRegistration::lineCouldBeOnCenterCircle(const Vector2f& lineStart, const Vector2f& direction) const
{
  Geometry::Line line;
  line.base = lineStart;
  line.direction = direction;
  const float distToCenter = Geometry::getDistanceToLine(line, Vector2f(0.f, 0.f));
  const float centerCircleCorridorInner = theFieldDimensions.centerCircleRadius * 0.4f;
  const float centerCircleCorridorOuter = theFieldDimensions.centerCircleRadius * 1.7f;
  return distToCenter > centerCircleCorridorInner && distToCenter < centerCircleCorridorOuter;
}

void PerceptRegistration::draw()
{
  DEBUG_DRAWING("module:PerceptRegistration:fieldModel", "drawingOnField")
  {
    for(const auto& line : verticalFieldLines)
    {
      LINE("module:PerceptRegistration:fieldModel", line.start.x(), line.start.y(), line.end.x(), line.end.y(),
           40, Drawings::solidPen, ColorRGBA(128, 0, 255, 100));
    }
    for(const auto& line : horizontalFieldLines)
    {
      LINE("module:PerceptRegistration:fieldModel", line.start.x(), line.start.y(), line.end.x(), line.end.y(),
           40, Drawings::solidPen, ColorRGBA(255, 0, 128, 100));
    }
  }
}
