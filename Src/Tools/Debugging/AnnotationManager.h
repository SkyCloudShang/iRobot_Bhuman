/**
 * @file AnnotationManager.h
 * @author Andreas Stolpmann
 */

#pragma once

#include "Tools/MessageQueue/MessageQueue.h"

#include <vector>
#include <unordered_map>

class AnnotationManager
{
private:
  MessageQueue outData;
  unsigned currentFrame = 0;
  unsigned annotationCounter = 0;
  unsigned lastGameState;

  friend class Process;

  AnnotationManager(); // private so only Process can access it.

public:
  void signalProcessStart();
  void clear();

  void addAnnotation();
  MessageQueue& getOut();
};
