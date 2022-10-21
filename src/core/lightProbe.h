#pragma once

#include "texture.h"
#include "vector.h"

class LightProbe : Texture {
public:
  LightProbe(const Color3f &background, const PropertyList &propList)
      : Texture(background, propList) {}

  Color3f intensity(const Point2f &uv) const {
    float float_u = uv.x() * (m_bitmap->rows() - 1);
    float float_v = uv.y() * (m_bitmap->cols() - 1);
    int u = float_u;
    int v = float_v;

    if (u >= m_bitmap->rows() - 1 || v >= m_bitmap->cols() - 1) {
      return (*m_bitmap)(u, v);
    }

    return bilerp(float_u - u, float_v - v, (*m_bitmap)(u, v),
                  (*m_bitmap)(u, v + 1), (*m_bitmap)(u + 1, v),
                  (*m_bitmap)(u + 1, v + 1));
  }

  Color3f intensity(const Vector3f &dir) const {
    Vector3f d = dir.normalized();

    float p = std::atan2(d.x(), d.z());
    if(p<0) p += 2.f * M_PI;

    Point2f uv(std::acos(d.y()) / M_PI,
               p / (2.f * M_PI));
    return intensity(uv);
  }
};
