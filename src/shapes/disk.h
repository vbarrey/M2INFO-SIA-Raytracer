#pragma once

#include "shape.h"

/**
 * @brief Disk whose normal is facing +Z
 */
class Disk : public Shape {
public:
  Disk(const PropertyList &propList);

  float radius() const { return m_radius; }

  virtual bool intersect(const Ray &ray, Hit &hit) const;

  virtual void sample(const Point2f &sample, Point3f &p, Normal3f &n,
                      float &pdf) const;

  virtual float area() const { return M_PI * m_radius * m_radius; }

 /// Return a human-readable summary
  std::string toString() const {
    return tfm::format("Disk[\n"
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
