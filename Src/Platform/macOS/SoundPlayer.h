/**
 * @file  Platform/macOS/SoundPlayer.h
 *
 * Declaration of class SoundPlayer.
 */

#pragma once

#include <deque>
#include <string>
#include <vector>
#include "Platform/Thread.h"
#include "Platform/Semaphore.h"

class SoundPlayer : Thread
{
private:
  static SoundPlayer soundPlayer; /**< The only instance of this class. */
  DECLARE_SYNC;
  std::deque<std::string> queue;
  std::string filePrefix;
  Semaphore sem;
  bool isPlaying = false;

public:
  /**
   * Put a filename into play sound queue.
   * If you want to play Config/Sounds/bla.wav use play("bla.wav");
   * @param name The filename of the sound file.
   * @return The number of files in the play sound queue.
   */
  static int play(const std::string& name);

  static int playSamples(std::vector<short>& samples);

private:
  ~SoundPlayer();

  /**
   * The main function of this thread.
   */
  void main();

  /**
   * Starts the thread that plays the sounds.
   */
  void start();

  /**
   * Plays a single sound.
   * @param basename The filename of the sound file.
   */
  void playDirect(const std::string& basename);
};
