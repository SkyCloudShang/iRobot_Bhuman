/**
 * @file Modules/Infrastructure/LowFrameRateImageProvider.h
 * This file implements a module that provides an image that is only rarely updated.
 * The image is intended for logging purposes.
 * @author Arne Böckmann
 * @author Jonas Kuball
 */

#include "LowFrameRateImageProvider.h"

LowFrameRateImageProvider::LowFrameRateImageProvider() : lastUpdateTime(0), storeNextImage(false)
{
  generateConditions();
  checkForInvalidConditions();
}

void LowFrameRateImageProvider::generateConditions()
{
  // Conditions are defined below. "Provider" is the current instance of this LowFrameRateImageProvider.
  conditionMap.emplace("BallSpots", [](const auto& provider) { return !provider.theBallSpots.ballSpots.empty(); });
  conditionMap.emplace("BallPercept", [](const auto& provider) { return provider.theBallPercept.status == BallPercept::Status::seen; });
  conditionMap.emplace("CirclePercept", [](const auto& provider) { return provider.theCirclePercept.wasSeen; });
  conditionMap.emplace("FieldLines", [](const auto& provider) { return provider.theFieldLines.lines.size() > 0; });
  conditionMap.emplace("GoalPostPercept", [](const auto& provider) { return provider.theGoalPostPercept.wasSeen; });
  conditionMap.emplace("PenaltyMarkPercept", [](const auto& provider) { return provider.thePenaltyMarkPercept.wasSeen; });
  conditionMap.emplace("PlayersPercept", [](const auto& provider) { return provider.thePlayersImagePercept.players.size() > 0; });
}

void LowFrameRateImageProvider::checkForInvalidConditions()
{
  // Warn the user exactly one time in the case that an invalid condition is given via config file.
  auto it = conditions.begin();
  while(it != conditions.end())
  {
    if(conditionMap.find(*it) == conditionMap.end())
    {
      OUTPUT_WARNING("LowFrameRateImageProvider: Condition " + *it + " is invalid.");
      it = conditions.erase(it);
    }
    else
    {
      it++;
    }
  }
}

void LowFrameRateImageProvider::update(LowFrameRateImage& lowFrameRateImage)
{
  lowFrameRateImage.imageUpdated = false;

  if(storeNextImage)
  {
    updateImage(lowFrameRateImage);
    storeNextImage = false;
  }
  else if((!onlyLogConditions && theFrameInfo.getTimeSince(lastUpdateTime) >= 60000 / frameRate)
          || (onlyLogConditions && theFrameInfo.getTimeSince(lastUpdateTime) >= 60000 / conditionFrameRate && recognizesPercept()))
  {
    // Generate new image
    lastUpdateTime = theFrameInfo.time;
    updateImage(lowFrameRateImage);
    storeNextImage = true; // Store next image as well to make sure to get both upper and lower cam images
  }
}

bool LowFrameRateImageProvider::recognizesPercept() const
{
  for(const std::string& condition : conditions)
  {
    const auto& conditionPairIterator = conditionMap.find(condition);
    if(conditionPairIterator != conditionMap.end() && conditionPairIterator->second.operator()(*this))
    {
      return true;
    }
  }
  return false;
}

void LowFrameRateImageProvider::updateImage(LowFrameRateImage& lfrImage) const
{
  lfrImage.image.setImage(const_cast<Image::Pixel*>(theImage[0]));
  lfrImage.image.setResolution(theImage.width, theImage.height, theImage.isFullSize);
  lfrImage.image.timeStamp = theImage.timeStamp;
  lfrImage.imageUpdated = true;
}

MAKE_MODULE(LowFrameRateImageProvider, cognitionInfrastructure)
