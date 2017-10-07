/**
 * This file declares a module that calculates the regions around interesting spots that must
 * be searched by CNS-based image processing modules.
 * @author Thomas Röfer
 */

#pragma once

#include "Representations/Configuration/BallSpecification.h"
#include "Representations/Infrastructure/CameraInfo.h"
#include "Representations/Modeling/WorldModelPrediction.h"
#include "Representations/Perception/BallPercepts/BallSpots.h"
#include "Representations/Perception/ImagePreprocessing/CameraMatrix.h"
#include "Representations/Perception/ImagePreprocessing/ImageRegions.h"
#include "Representations/Perception/PlayersPercepts/PlayersImagePercept.h"
#include "Tools/Module/Module.h"

MODULE(CNSRegionsProvider,
{,
  REQUIRES(BallSpecification),
  REQUIRES(BallSpots),
  REQUIRES(CameraInfo),
  REQUIRES(CameraMatrix),
  REQUIRES(CNSPenaltyMarkRegions),
  REQUIRES(BallRegions),
  REQUIRES(WorldModelPrediction),
  REQUIRES(PlayersImagePercept),
  PROVIDES(BallRegions),
  PROVIDES(CNSRegions),
  LOADS_PARAMETERS(
  {,
    (bool) dontUseUpperImageButWholeLower, /** Will set one big region over the whole image if it is the lower, else zero regions are produced */
    (bool) usePrediction, /**< Include block around predicted ball position. */
    (bool) playersFeet, /**< Create CNS-Regions at players feet */
    (float) sizeFactor, /**< Factor to increase search space around ball spot radius. */
    (float) predictedFactor, /**< Factor to increase search space around the predicted ball position. */
    (int) blockSizeX, /**< Horizontal block size. Must be a multiple of 16. */
    (int) blockSizeY, /**< Vertical block size. Must be a multiple of 16. */
    (float) searchFootHeightByHeight, /*< OrientationDetermination: Height in mm for search area */
    (float) searchFootHeightByDepth, /*< OrientationDetermination: Depth in mm for search area */
    (float) robotExpectedWidth, /*< OrientationDetermination: Maximal width of a robot in mm */
  }),
});

class CNSRegionsProvider : public CNSRegionsProviderBase
{
  /**
   * Marks the blockSizeX*blockSizeY cells the CNS should be computed for.
   * There is a one block border around the actual image, i.e. the upper left corner is at (1, 1).
   */
  char searchGrid[32][42];
  std::vector<Vector2i> stack; /**< The stack used by the flood fill algorithm to group blocks. */

  void update(BallRegions& ballRegions);
  void update(CNSRegions& cnsRegions);

  /** Adds the regions occupied by assumed player feet to the search grid. */
  void addPlayersFeetRegions();

  /**
   * Determines the size of connected region in the search grid.
   * @param x The x coordinate of a point in the regions.
   * @param y The y coordinate of a point in the regions.
   * @param xRange The horizontal extent of the regions is reported here. The coordinates are
   *               zero-based, i.e. one less than the coordinate system in which x is provided.
   * @param xRange The vertical extent of the regions is reported here. The coordinates are
   *               zero-based, i.e. one less than the coordinate system in which y is provided.
   */
  void floodFill(int x, int y, Rangei& xRange, Rangei& yRange);

public:
  CNSRegionsProvider() {stack.reserve(sizeof(searchGrid));}
};
