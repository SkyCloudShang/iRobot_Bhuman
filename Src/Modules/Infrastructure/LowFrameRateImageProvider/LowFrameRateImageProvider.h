/**
 * @file Modules/Infrastructure/LowFrameRateImageProvider.h
 * This file declares a module that provides an image that is only rarely updated.
 * The image is intended for logging purposes.
 * @author Arne Böckmann
 * @author Jonas Kuball
 */

#pragma once

#include "Tools/Module/Module.h"
#include "Representations/Infrastructure/LowFrameRateImage.h"
#include "Representations/Infrastructure/Image.h"
#include "Representations/Infrastructure/FrameInfo.h"
#include "Representations/Infrastructure/CameraInfo.h"
#include "Representations/Perception/BallPercepts/BallPercept.h"
#include "Representations/Perception/BallPercepts/BallSpots.h"
#include "Representations/Perception/FieldPercepts/CirclePercept.h"
#include "Representations/Perception/FieldPercepts/FieldLines.h"
#include "Representations/Perception/FieldPercepts/GoalPostPercept.h"
#include "Representations/Perception/FieldPercepts/PenaltyMarkPercept.h"
#include "Representations/Perception/PlayersPercepts/PlayersImagePercept.h"

#include <map>

MODULE(LowFrameRateImageProvider,
{,
  REQUIRES(Image),
  REQUIRES(CameraInfo),
  REQUIRES(FrameInfo),
  REQUIRES(BallPercept),
  REQUIRES(BallSpots),
  REQUIRES(CirclePercept),
  REQUIRES(FieldLines),
  REQUIRES(GoalPostPercept),
  REQUIRES(PenaltyMarkPercept),
  REQUIRES(PlayersImagePercept),
  PROVIDES_WITHOUT_MODIFY(LowFrameRateImage),
  LOADS_PARAMETERS(
  {,
    (int) frameRate, /**< Frames per minute. */
    (int) conditionFrameRate, /**< Frames per minute, used when onlyLogConditions is true. */
    (bool) onlyLogConditions, /**< If true, only images where any of the given condition is matched are logged */
    (std::vector<std::string>) conditions, /**< conditions are pre-defined in LowFrameRateImageProvider::generateConditions(). */
  }),
});

class LowFrameRateImageProvider : public LowFrameRateImageProviderBase
{
public:
  LowFrameRateImageProvider();
  void update(LowFrameRateImage& image);

private:
  void updateImage(LowFrameRateImage& lfrImage) const;

  std::map<std::string, std::function<bool(const LowFrameRateImageProvider&)>> conditionMap;
  void generateConditions();
  void checkForInvalidConditions();

  bool recognizesPercept() const;

  unsigned lastUpdateTime; /**< Time of last update. */
  bool storeNextImage;
};
