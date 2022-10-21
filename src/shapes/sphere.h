#pragma once

#include "shape.h"
#include "shapes/mesh.h"

#include <Eigen/Core>
#include <map>

/** Represents a sphere
 */
class Sphere : public Shape {
public:
  Sphere(float radius);
  Sphere(const PropertyList &propList);

  virtual bool intersect(const Ray &ray, Hit &hit) const;

  float radius() const { return m_radius; }
  
  virtual void sample(const Point2f &sample, Point3f &p, Normal3f &n,
                      float &pdf) const;

  virtual float area() const { return 4.f * M_PI * m_radius * m_radius; }

  /// Return a human-readable summary
  std::string toString() const {
    return tfm::format("Sphere[\n"
                       "  frame = %s,\n"
                       "  radius = %f\n"
                       "  BSDF = %s,\n]"
                       "]",
                       indent(m_transformation.toString(), 10), m_radius,
                       m_bsdf ? indent(m_bsdf->toString())
                              : std::string("null"));
  }

protected:
  float m_radius;
};
