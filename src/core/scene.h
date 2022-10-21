#pragma once

#include "camera.h"
#include "integrator.h"
#include "light.h"
#include "sampler.h"
#include "shape.h"

class AreaLight;

typedef std::vector<Shape *> ShapeList;
typedef std::vector<Light *> LightList;

class Scene : public Object {
public:
  Scene(const PropertyList &props);

  /// \return a pointer to the scene's camera
  Camera *camera() { return m_camera; }
  const Camera *camera() const { return m_camera; }
  void setCamera(Camera *camera) { m_camera = camera; }

  /// \return a pointer to the scene's integrator
  Integrator *integrator() { return m_integrator; }
  const Integrator *integrator() const { return m_integrator; }

  /// Return a pointer to the scene's sample generator
  const Sampler *getSampler() const { return m_sampler; }
  Sampler *getSampler() { return m_sampler; }

  /// \return a reference to an array containing all the shapes
  const ShapeList &shapeList() const { return m_shapeList; }

  /// \return a reference to an array containing all the lights
  const LightList &lightList() const { return m_lightList; }

  /// \return the background color
  Color3f backgroundColor(const Vector3f &direction = Vector3f::UnitZ()) const;

  /// Search the nearest intersection between the ray and the shape list
  void intersect(const Ray &ray, Hit &hit) const;

  /**
   * \brief Inherited from \ref NoriObject::activate()
   *
   * Initializes the internal data structures (kd-tree,
   * emitter sampling data structures, etc.)
   */
  void activate();

  /// Register a child object (e.g. a material) with the shape
  virtual void addChild(Object *child);

  /// \brief Return the type of object provided by this instance
  EClassType getClassType() const { return EScene; }

  /// Clear the scene and release the memory
  void clear();

  /// Return a string summary of the scene (for debugging purposes)
  std::string toString() const;

private:
  Integrator *m_integrator = nullptr;
  Sampler *m_sampler = nullptr;
  Camera *m_camera = nullptr;

  ShapeList m_shapeList;

  LightList m_lightList;
  std::vector<AreaLight *> m_areaLightList;

  Color3f m_backgroundColor;
  Light *m_envMap = nullptr;
};
