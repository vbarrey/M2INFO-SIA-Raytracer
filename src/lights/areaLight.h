#pragma once

#include "light.h"
#include "shape.h"

class Texture;

class AreaLight : public Light {
public:
  AreaLight(const PropertyList &propList);

  /// See light.h
  Color3f sample(const Point3f &x, const Point2f &sample, float &pdf,
                 Vector3f &wi, float &dist) const override;

  /// See light.h
  Color3f intensity(const Vector2f &uv, const Normal3f &n,
                    const Vector3f &w) const override;

  Shape *shape() const override { return m_shape; }

  /// Return a human-readable summary
  std::string toString() const override;

  void addChild(Object *child) override;

protected:
  Color3f m_intensity;
  bool m_twoSided;
  Shape *m_shape = nullptr;
  Texture *m_texture = nullptr;
};
