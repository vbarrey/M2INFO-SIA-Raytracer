#pragma once

#include "shape.h"

/**
 * @brief Quad (potentially infinite) whose normal is facing +Z
 */
class Quad : public Shape {
public:
  Quad(const PropertyList &propList);

  Vector2f size() const { return m_size; }

  virtual bool intersect(const Ray &ray, Hit &hit) const;

  virtual void sample(const Point2f &sample, Point3f &p, Normal3f &n,
                      float &pdf) const;

  virtual float area() const { return m_size.x() * m_size.y(); }

  /// Return a human-readable summary
  std::string toString() const {
    return tfm::format("Quad[\n"
                       "  frame = %s,\n"
                       "  size = %s,\n"
                       "  BSDF = %s\n]",
                       indent(m_transformation.toString(), 10),
                       m_size.toString(),
                       m_bsdf ? indent(m_bsdf->toString())
                              : std::string("null"));
  }

protected:
  Vector2f m_size;
  bool m_infinite;
};
