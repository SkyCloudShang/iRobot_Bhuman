/**
 * @file Controller/RoboCupCtrl.h
 *
 * This file declares the class RoboCupCtrl.
 *
 * @author <a href="mailto:Thomas.Roefer@dfki.de">Thomas Röfer</a>
 */

#pragma once

#include "SimulatedRobot.h"
#include "GameController.h"

#include <QList>
#include <QString>
#include <list>

class Robot;

/**
 * The class implements the SimRobot controller for RoboCup.
 */
class RoboCupCtrl : public SimRobot::Module, public SimRobotCore2::CollisionCallback
{
public:
  static RoboCupCtrl* controller; /**< A pointer to the SimRobot controller. */
  static SimRobot::Application* application; /**< The interface to the SimRobot GUI */
  GameController gameController;

protected:
  const char* robotName; /**< The name of the robot currently constructed. */
  std::list<Robot*> robots; /**< The list of all robots. */
  int simStepLength; /**< The length of one simulation step (in ms) */
  bool simTime = false; /**< Switches between simulation time mode and real time mode. */
  float delayTime = 0.f; /**< Delay simulation to reach this duration of a step. */
  int time; /**< The simulation time. */
  float lastTime = 0.f; /**< The last time execute was called. */
  QString statusText; /**< The text to be printed in the status bar. */

private:
  QList<SimRobot::Object*> views; /**< List of registered views */

public:
  RoboCupCtrl(SimRobot::Application& application);

protected:
  virtual ~RoboCupCtrl();

public:
  /**
   * Adds a scene graph object to the scene graph displayed in SimRobot
   * @param object The scene graph object to add
   * @param parent The parent scene graph object (e.g. a category or null)
   * @param flags Some flags for registering the scene graph object (see SimRobot::Flag)
   */
  void addView(SimRobot::Object* object, const SimRobot::Object* parent = nullptr, int flags = 0);

  /**
   * Adds a scene graph object to the scene graph displayed in SimRobot
   * @param object The scene graph object to add
   * @param categoryName The full name of the parent categroy
   * @param flags Some flags for registering the scene graph object (see SimRobot::Flag)
   */
  void addView(SimRobot::Object* object, const QString& categoryName, int flags = 0);

  /**
   * Removes a view that was previously added with addView(). Note that no parent object
   * is required here, due to asymmetrical registerObject()/unregisterObject() definitions
   * @param object View to remove from scene graph
   */
  void removeView(SimRobot::Object* object);

  /**
   * Adds a category to the scene graph displayed in SimRobot that can be used for grouping views
   * @param name The name of the category
   * @param parent The parent scene graph object (e.g. a category or null)
   * @param icon The icon used to list the category in the scene graph
   * @return The category
   */
  SimRobot::Object* addCategory(const QString& name, const SimRobot::Object* parent = nullptr, const char* icon = nullptr);

  /**
   * Adds a category to the scene graph displayed in SimRobot that can be used for grouping views
   * @param name The name of the category
   * @param parentName The name of the parent scene graph object (e.g. a category)
   * @return The category
   */
  SimRobot::Object* addCategory(const QString& name, const QString& parentName);

  /**
   * Removes a category that was previously added with addCategory(). Note that no parent object
   * is required here, due to asymmetrical registerObject()/unregisterObject() definitions
   * @param object Category to remove from scene graph
   */
  void removeCategory(SimRobot::Object* object);

  /**
   * The function returns the full name of the robot currently constructed.
   * @return The name of the robot.
   */
  static const char* getRobotFullName() { return controller->robotName; }

  /**
   * The function returns the name of the robot associated to the current thread.
   * @return The name of the robot.
   */
  std::string getRobotName() const;

  /**
   * The function returns the simulation time.
   * @return The pseudo-time in milliseconds.
   */
  unsigned getTime() const;

protected:
  /**
   * The function is called to initialize the module.
   */
  virtual bool compile();

  /**
   * The function is called in each simulation step.
   */
  virtual void update();

  /**
   * The callback function.
   * Called whenever the geometry at which this interface is registered collides with another geometry.
   * @param geom1 The geometry at which the interface has been registered
   * @param geom2 The other geometry
   */
  virtual void collided(SimRobotCore2::Geometry& geom1, SimRobotCore2::Geometry& geom2);

  /**
   * Has to be called by derived class to start processes.
   */
  void start();

  /**
   * Has to be called by derived class to stop processes.
   */
  void stop();
};
