#pragma once

#include "bitmap.h"
#include "object.h"

enum class LightFlags : int {
  DeltaPosition = 1,
  DeltaDirection = 2,
  Area = 4,
  Infinite = 8
};

class Light : public Object {
public:
  Light() : m_transformation(Eigen::Affine3f::Identity().matrix()) {}

  /** Returns the intensity emitted along \param direction (for infinite
   * lights)
   */
  virtual Color3f intensity(const Vector3f &direction) const {
    return Color3f(0.f);
  }

  /** Return the intensity emitted at \c x
   *   \param x       world space position of a reference point in the scene
   *   \param sample  2D sample value for sampling area lights
   *   \param pdf     PDF for sampling the chosen direction
   *   \param wi      incident direction to the light source
   *   \param dist    distance to the light
   */
  virtual Color3f sample(const Point3f &x, const Point2f &sample, float &pdf,
                         Vector3f &wi, float &dist) const = 0;

  /** Return the intensity emitted at coordinates \param uv of the light with
   * normal \param n in the outgoing direction \param w (for area lights)
   */
  virtual Color3f intensity(const Vector2f &uv, const Normal3f &n,
                            const Vector3f &w) const {
    return Color3f(0.f);
  }

  /// Return the associated shape (for area lights)
  virtual Shape *shape() const { return nullptr; }

  virtual void setTransformation(const Eigen::Matrix4f &mat) {
    m_transformation = mat;
  }
  virtual const Transform &transformation() const { return m_transformation; }

  inline bool isDeltaLight() const {
    return m_flags & (int)LightFlags::DeltaPosition ||
           m_flags & (int)LightFlags::DeltaDirection;
  }

  int flags() const { return m_flags; }

  EClassType getClassType() const { return ELight; }

protected:
  Transform m_transformation;
  int m_flags;
};

typedef std::vector<Light *> LightList;
