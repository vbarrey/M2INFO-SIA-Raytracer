#pragma once

#include "bsdf.h"
#include "common.h"
#include "object.h"
#include "ray.h"

class AreaLight;

/** represents a shape (geometry and BSDF)
 */
class Shape : public Object {
public:
  Shape() : m_transformation(Eigen::Affine3f::Identity().matrix()) {}

  Shape(const PropertyList &)
      : m_transformation(Eigen::Affine3f::Identity().matrix()) {}

  virtual void activate();

  /** Search the nearest intersection between the ray and the shape.
   * It must be implemented in the derived class. */
  virtual bool intersect(const Ray &ray, Hit &hit) const {
    throw RTException(
        "Shape::intersect must be implemented in the derived class");
  }

  /** Return the axis-aligned bounding box of the geometry.
   * It must be implemented in the derived class. */
  virtual const BoundingBox3f &getBoundingBox() const {
    throw RTException("Shape::AABB must be implemented in the derived class");
  }

  virtual const BSDF *bsdf() const { return m_bsdf; }
  virtual void setBsdf(const BSDF *bsdf) { m_bsdf = bsdf; }

  virtual void setTransformation(const Eigen::Matrix4f &mat) {
    m_transformation = mat;
  }
  virtual const Transform &transformation() const { return m_transformation; }

  virtual bool isAreaLight() const { return m_light != nullptr; }
  virtual const AreaLight *areaLight() const { return m_light; }

  virtual float area() const {
    throw RTException("Shape::area must be implemented in the derived class");
  };

  /** Sample a point on the shape
   * \param sample two uniform random numbers
   * \param p      sampled point position in world space
   * \param n      its normal
   * \param pdf    associated PDF
   */
  virtual void sample(const Point2f &sample, Point3f &p, Normal3f &n,
                      float &pdf) const {
    throw RTException("Shape::sample must be implemented in the derived class");
  }

  /// Register a child object (e.g. a BSDF) with the shape
  virtual void addChild(Object *child);
  /// Register a parent object (e.g. a light) with the shape
  virtual void setParent(Object *parent);

  /// \brief Return the type of object provided by this instance
  EClassType getClassType() const { return EShape; }

  virtual std::string toString() const {
    throw RTException(
        "Shape::toString must be implemented in the derived class");
  }

protected:
  const BSDF *m_bsdf = nullptr;
  const AreaLight *m_light = nullptr;
  Transform m_transformation;
};
