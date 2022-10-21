#pragma once

#include "light.h"
#include "shape.h"

class Texture;

class AreaLight : public Light {
public:
  AreaLight(const PropertyList &propList);

  Color3f sample(const Point3f &x, const Point2f &u, float &pdf, Vector3f &wi,
                 float &dist) const override;

  Color3f intensity(const Hit &hit, const Vector3f &w) const override;

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
