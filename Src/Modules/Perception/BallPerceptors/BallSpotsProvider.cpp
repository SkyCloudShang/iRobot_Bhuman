/**
 * @file BallSpotsProvider.cpp
 * @author <a href="mailto:jesse@tzi.de">Jesse Richter-Klug</a>
 */

#include "BallSpotsProvider.h"
#include "Tools/Debugging/DebugDrawings.h"
#include "Tools/ImageProcessing/InImageSizeCalculations.h"
#include <algorithm>

void BallSpotsProvider::update(BallSpots& ballSpots)
{
  DECLARE_DEBUG_DRAWING("module:BallSpotsProvider:scanLines", "drawingOnImage");

  ballSpots.ballSpots.clear();
  searchScanLines(ballSpots);
}

void BallSpotsProvider::searchScanLines(BallSpots& ballSpots) const
{
  //todo body and fieldline
  const unsigned step = theColorScanlineRegionsVerticalClipped.lowResStep > 1 ? theColorScanlineRegionsVerticalClipped.lowResStep / 2 : 1;
  const unsigned start = theColorScanlineRegionsVerticalClipped.lowResStart >= step ? theColorScanlineRegionsVerticalClipped.lowResStart - step : theColorScanlineRegionsVerticalClipped.lowResStart;
  Geometry::Circle circle;

  for(unsigned scanLineIndex = start; scanLineIndex < theColorScanlineRegionsVerticalClipped.scanlines.size(); scanLineIndex += step)
  {
    int lowestYOfCurrentArea = 0;
    int currentLengthNeeded = 0;
    for(const ScanlineRegion& region : theColorScanlineRegionsVerticalClipped.scanlines[scanLineIndex].regions)
    {
      if(!region.is(FieldColors::field))
      {
        if(currentLengthNeeded == 0)
        {
          lowestYOfCurrentArea = region.range.lower;
          currentLengthNeeded = static_cast<int>(getNeededLengthFor(theColorScanlineRegionsVerticalClipped.scanlines[scanLineIndex].x, region.range.lower, circle));
          if(!currentLengthNeeded)
            break;
        }
      }
      else
      {
        if(currentLengthNeeded != 0)
        {
          Vector2i spot;
          bool foundSpot = true;

          if(lowestYOfCurrentArea - region.range.lower > currentLengthNeeded)
            ballSpots.ballSpots.emplace_back(circle.center.cast<int>());
          else if(currentLengthNeeded != 0 && lowestYOfCurrentArea == theColorScanlineRegionsVerticalClipped.scanlines[scanLineIndex].regions.front().range.lower &&
                  (scanLineIndex - start) % step == 0 && lowestYOfCurrentArea - region.range.lower > currentLengthNeeded / 2)
            ballSpots.ballSpots.emplace_back(theColorScanlineRegionsVerticalClipped.scanlines[scanLineIndex].x, (lowestYOfCurrentArea + region.range.lower) / 2);
          else
            foundSpot = false;

          if(foundSpot &&
             (isLastDuplicative(ballSpots, static_cast<int>(circle.radius * minAllowedDistanceRadiusRelation))
              || isSpotClearlyInsideARobot(ballSpots.ballSpots.back(), circle.radius)
              || !correctWithScanLeftAndRight(ballSpots.ballSpots.back(), circle)
              || (currentLengthNeeded < minRadiusOfWantedRegion
                  && !checkGreenAround(ballSpots.ballSpots.back(), circle.radius))
              || isSpotClearlyInsideARobot(ballSpots.ballSpots.back(), circle.radius)))
            ballSpots.ballSpots.erase(ballSpots.ballSpots.end() - 1);
        }
        // if(theCameraInfo.camera == CameraInfo::lower && )

        //TODO if head reached
        lowestYOfCurrentArea = 0;
        currentLengthNeeded = 0;
      }
    }

    if(allowScanlineTopSpotFitting
       && currentLengthNeeded > sqr(minRadiusOfWantedRegion)
       && lowestYOfCurrentArea - theColorScanlineRegionsVerticalClipped.scanlines[scanLineIndex].regions.back().range.upper > std::sqrt(currentLengthNeeded)
       && !isSpotClearlyInsideARobot(circle.center.cast<int>(), circle.radius))
    {
      ballSpots.ballSpots.emplace_back(circle.center.cast<int>());

      if(isLastDuplicative(ballSpots, static_cast<int>(circle.radius * minAllowedDistanceRadiusRelation)))
        ballSpots.ballSpots.erase(ballSpots.ballSpots.end() - 1);
    }
  }
}

bool BallSpotsProvider::correctWithScanLeftAndRight(Vector2i& initialPoint, const Geometry::Circle& circle) const
{
  const int maxScanLength = static_cast<int>(circle.radius * scanLengthRadiusFactor);

  int leftMaximum(0), rightMaximum(theECImage.colored.width);
  theBodyContour.clipLeft(leftMaximum, initialPoint.y());
  theBodyContour.clipRight(rightMaximum, initialPoint.y());

  const int maxLeftScanLength = std::min(maxScanLength, initialPoint.x() - leftMaximum);
  const int maxRightScanLength = std::min(maxScanLength, rightMaximum - initialPoint.x());

  unsigned foundGoodPixel = 0;
  unsigned foundNeutralPixel = 0;
  int leftScanLength = 0;
  scanBallSpotOneDirection(initialPoint, leftScanLength, maxLeftScanLength, foundGoodPixel, foundNeutralPixel,
  [](const Vector2i& spot, const int currentLength) {return int(spot.x() - currentLength); },
  [](const Vector2i& spot, const int currentLength) {return int(spot.y()); });

  int rightScanLength = 0;
  scanBallSpotOneDirection(initialPoint, rightScanLength, maxRightScanLength, foundGoodPixel, foundNeutralPixel,
  [](const Vector2i& spot, const int currentLength) {return int(spot.x() + currentLength); },
  [](const Vector2i& spot, const int currentLength) {return int(spot.y()); });

  initialPoint.x() -= (leftScanLength - rightScanLength) / 2;
  const float noise = 1.f - static_cast<float>(foundGoodPixel) / static_cast<float>(leftScanLength + rightScanLength - foundNeutralPixel);
  const float goodNeutralRatio = static_cast<float>(foundGoodPixel) / static_cast<float>(foundNeutralPixel + foundGoodPixel);
  const float foundDiameterPercentage = static_cast<float>(leftScanLength + rightScanLength) / static_cast<float>(2 * circle.radius);

  DRAWTEXT("module:BallSpotsProvider:scanLines", initialPoint.x() + 10, initialPoint.y() - 4, 6, ColorRGBA::red, "noise:" << noise);
  DRAWTEXT("module:BallSpotsProvider:scanLines", initialPoint.x() + 10, initialPoint.y() + 5, 6, ColorRGBA::red, "foundDiameterPercentage:" << foundDiameterPercentage);
  DRAWTEXT("module:BallSpotsProvider:scanLines", initialPoint.x() + 10, initialPoint.y() + 9, 6, ColorRGBA::red, "goodNeutralRatio:" << goodNeutralRatio);

  return noise < noiseThreshold && foundDiameterPercentage > minFoundDiameterPercentage && goodNeutralRatio > minGoodNeutralRatio;
}

void BallSpotsProvider::scanBallSpotOneDirection(const Vector2i& spot, int& currentLength, const int& maxLength,
  unsigned& goodPixelCounter, unsigned& neutralPixelCounter,
  int(*getX) (const Vector2i& spot, const int currentLength),
  int(*getY) (const Vector2i& spot, const int currentLength)) const
{
  unsigned currentSkipped = 0;
  unsigned currentSkippedGreen = 0;
  while(checkPixel(theECImage.colored[getY(spot, currentLength)][getX(spot, currentLength)], goodPixelCounter, neutralPixelCounter, currentSkippedGreen, currentSkipped)
        && ++currentLength <= maxLength);
  currentLength -= currentSkipped;

  LINE("module:BallSpotsProvider:scanLines", spot.x(), spot.y(), getX(spot, maxLength), getY(spot, maxLength),
       1, Drawings::solidPen, ColorRGBA::yellow);
  LINE("module:BallSpotsProvider:scanLines", spot.x(), spot.y(), getX(spot, currentLength), getY(spot, currentLength),
       1, Drawings::solidPen, ColorRGBA::red);
}

bool BallSpotsProvider::checkPixel(const PixelTypes::ColoredPixel& pixel, unsigned& goodPixelCounter, unsigned& neutralPixelCounter, unsigned& currentSkippedGreen, unsigned& currentSkipped) const
{
  if(pixel == FieldColors::white)
  {
    currentSkipped = 0;
    currentSkippedGreen = 0;
    ++goodPixelCounter;
  }
  else if(pixel == FieldColors::black)
  {
    if(!blackPixelsAreNeutral)
    {
      currentSkipped = 0;
      currentSkippedGreen = 0;
      ++goodPixelCounter;
    }
    else
      ++neutralPixelCounter;
  }
  else if(pixel == FieldColors::field)
  {
    ++currentSkipped;
    ++currentSkippedGreen;
  }
  else if(!allowColorNon)
    ++currentSkipped;
  else
    ++neutralPixelCounter;

  return currentSkipped < maxNumberOfSkippablePixel && currentSkippedGreen < maxNumberOfSkippableGreenPixel;
}

bool BallSpotsProvider::isLastDuplicative(const BallSpots& ballSpots, const int minAllowedDistanz) const
{
  if(ballSpots.ballSpots.size() < 2)
    return false;

  const int sqaredAllowdDistanz = sqr(minAllowedDistanz);
  const Vector2i& spotToCheck = ballSpots.ballSpots.back();
  for(auto ptr = ballSpots.ballSpots.begin(); ptr < ballSpots.ballSpots.end() - 1; ptr++)
    if((*ptr - spotToCheck).squaredNorm() < sqaredAllowdDistanz)
      return true;

  return false;
}

bool BallSpotsProvider::checkGreenAround(const Vector2i& spot, const float radius) const
{
  int useRadius = additionalRadiusForGreenCheck + static_cast<int>(radius);
  if(useRadius >= spot.x() - 1 || useRadius >= spot.y() - 1 ||
     spot.x() + 1 + useRadius >= theECImage.colored.width ||
     spot.y() + 1 + useRadius >= theECImage.colored.height)
    return false;

  int count(0);
  const int lastX = spot.x() + useRadius - 1;

  const int y1 = spot.y() - useRadius;
  const int y12 = spot.y() - useRadius - 1;
  const int y2 = spot.y() + useRadius;
  const int y22 = spot.y() + useRadius + 1;

  for(int x = spot.x() - useRadius + 1; x <= lastX; x++)
    if(theECImage.colored[y1][x] == FieldColors::field)
      count++;

  for(int x = spot.x() - useRadius + 1; x <= lastX; x++)
    if(theECImage.colored[y12][x] == FieldColors::field)
      count++;

  for(int x = spot.x() - useRadius + 1; x <= lastX; x++)
    if(theECImage.colored[y2][x] == FieldColors::field)
      count++;

  for(int x = spot.x() - useRadius + 1; x <= lastX; x++)
    if(theECImage.colored[y22][x] == FieldColors::field)
      count++;

  const int lastY = spot.y() + useRadius - 1;

  const int x1 = spot.x() - useRadius;
  const int x12 = spot.x() - useRadius;
  const int x2 = spot.x() + useRadius;
  const int x22 = spot.x() + useRadius;

  for(int y = spot.y() - useRadius + 1; y <= lastY; y++)
    if(theECImage.colored[y][x1] == FieldColors::field)
      count++;

  for(int y = spot.y() - useRadius + 1; y <= lastY; y++)
    if(theECImage.colored[y][x12] == FieldColors::field)
      count++;

  for(int y = spot.y() - useRadius + 1; y <= lastY; y++)
    if(theECImage.colored[y][x2] == FieldColors::field)
      count++;

  for(int y = spot.y() - useRadius + 1; y <= lastY; y++)
    if(theECImage.colored[y][x22] == FieldColors::field)
      count++;

  const int allPixel = 8 * (2 * useRadius - 1);
  const float percentGreen = static_cast<float>(count) / static_cast<int>(allPixel);

  return percentGreen > greenPercent;
}

bool BallSpotsProvider::isSpotClearlyInsideARobot(const Vector2i& spot, const float estimatedRadius) const
{
  for(const PlayersImagePercept::PlayerInImage& player : thePlayersImagePercept.players)
    if(spot.x() > player.x1FeetOnly && spot.x() < player.x2FeetOnly)
      if(spot.y() < player.y2 - estimatedRadius - IISC::getImageLineDiameterByLowestPoint(spot.cast<float>(), theCameraInfo, theCameraMatrix, theFieldDimensions))
        return true;

  return false;
}

float BallSpotsProvider::getNeededLengthFor(const int x, const int y, Geometry::Circle& circle) const
{
  const Vector2f startPoint = Vector2i(x, y).cast<float>();
  if(!IISC::calcPossibleVisibleBallByLowestPoint(startPoint, circle, theCameraInfo, theCameraMatrix, theBallSpecification, greenEdge)
    )//|| circle.radius < minRadiusOfWantedRegion)
    return 0;
  else
    return circle.radius + (startPoint - circle.center).norm() * ballSpotDistUsage;
}

MAKE_MODULE(BallSpotsProvider, perception)
