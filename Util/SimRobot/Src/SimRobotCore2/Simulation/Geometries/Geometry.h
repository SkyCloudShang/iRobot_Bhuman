/**
* @file Simulation/Geometries/Geometry.h
* Declaration of class Geometry
* @author Colin Graf
*/

#pragma once

#include <ode/ode.h>
#include <unordered_map>
#include "Simulation/PhysicalObject.h"

/**
* @class Geometry
* Abstract class for geometries of physical objects
*/
class Geometry : public PhysicalObject, public SimRobotCore2::Geometry
{
public:
  /**
  * @class Material
  * Describes friction properties of the material a geometry is made of
  */
  class Material : public Element
  {
  public:
    std::string name; /**< The name of the material */
    std::unordered_map<std::string, float> frictions; /**< The friction of the material on another material */
    std::unordered_map<std::string, float> rollingFrictions; /**< The rolling friction of the material on another material */

    /**
    * Looks up the friction on another material
    * @param other The other material
    * @param friction The friction on the other material
    * @return Whether there is a friction specified or not
    */
    bool getFriction(const Material& other, float& friction) const;

    /**
    * Looks up the rolling friction on another material
    * @param other The other material
    * @param rollingFriction The rolling friction on the other material
    * @return Whether there is a rolling friction specified or not
    */
    bool getRollingFriction(const Material& other, float& rollingFriction) const;

  private:
    mutable std::unordered_map<const Material*, float> materialToFriction; /**< A pointer map to speed up friction lookups */
    mutable std::unordered_map<const Material*, float> materialToRollingFriction; /**< A pointer map to speed up rolling friction lookups */

    /**
    * Registers an element as parent
    * @param element The element to register
    */
    virtual void addParent(Element& element);
  };

  bool immaterial; /**< Whether this geometry will not be used to generate contact points when it collides with another geometry */
  float innerRadius; /**< The radius of a sphere that is enclosed by the geometry. This radius is used for implementing a fuzzy but fast distance sensor */
  float innerRadiusSqr; /**< precomputed square of \c innerRadius */
  float outerRadius; /**< The radius of a sphere that covers the geometry. */

  float color[4]; /**< A color for drawing the geometry */
  Material* material; /**< The material the surface of the geometry is made of */
  std::list<SimRobotCore2::CollisionCallback*>* collisionCallbacks; /**< Collision callback functions registered by another SimRobot module */

  /** Default constructor */
  Geometry();

  /** Virtual destructor */
  virtual ~Geometry();

  /**
  * Creates the geometry (not including \c translation and \c rotation)
  * @param space A space to create the geometry in
  * @return The created geometry
  */
  virtual dGeomID createGeometry(dSpaceID space);

private:
  bool created;

  /**
  * Draws physical primitives of the object (including children) on the currently selected OpenGL context
  * @param flags Flags to enable or disable certain features
  */
  virtual void drawPhysics(unsigned int flags) const;

  /**
  * Registers an element as parent
  * @param element The element to register
  */
  virtual void addParent(Element& element);

  friend class CollisionSensor;

private:
  // API
  virtual const QString& getFullName() const {return SimObject::getFullName();}
  virtual SimRobot::Widget* createWidget() {return SimObject::createWidget();}
  virtual const QIcon* getIcon() const {return SimObject::getIcon();}
  virtual SimRobotCore2::Renderer* createRenderer() {return SimObject::createRenderer();}
  virtual bool registerDrawing(SimRobotCore2::Controller3DDrawing& drawing) {return ::PhysicalObject::registerDrawing(drawing);}
  virtual bool unregisterDrawing(SimRobotCore2::Controller3DDrawing& drawing) {return ::PhysicalObject::unregisterDrawing(drawing);}
  virtual SimRobotCore2::Body* getParentBody() {return ::PhysicalObject::getParentBody();}
  virtual bool registerCollisionCallback(SimRobotCore2::CollisionCallback& collisionCallback);
  virtual bool unregisterCollisionCallback(SimRobotCore2::CollisionCallback& collisionCallback);
};
