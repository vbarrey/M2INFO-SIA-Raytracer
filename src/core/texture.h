#pragma once

#include "bitmap.h"
#include "object.h"

class Texture {
public:
  Texture(const Color3f &albedo, const PropertyList &propList)
      : m_albedo(albedo) {
    m_bitmap = nullptr;
    std::string texturePath = propList.getString("texture", "");
    if (texturePath.size() > 0) {
      filesystem::path filepath = getFileResolver()->resolve(texturePath);
      if (filepath.exists()) {
        m_bitmap = new Bitmap(filepath);
        m_scale = propList.getVector("scale", Vector3f::Ones()).head<2>();
        m_mode = TextureMode(propList.getInteger("mode", 0));
        m_filter = propList.getBoolean("filter", true);
        if (m_mode == REPLACE)
          m_albedo = Color3f(1.f);
      }
    }
  }

  ~Texture() {
    if (m_bitmap)
      delete m_bitmap;
  }

  Color3f lookUp(const Point2f &coords) const {
    if (m_bitmap == nullptr)
      return m_albedo;

    float scaled_u = coords.x() / m_scale.x();
    float scaled_v = coords.y() / m_scale.y();

    float float_u = (scaled_u - std::floor(scaled_u)) * (m_bitmap->rows() - 1);
    float float_v = (scaled_v - std::floor(scaled_v)) * (m_bitmap->cols() - 1);

    int u = float_u;
    int v = float_v;

    // less than two pixels
    if(float_u < 1.f && float_v < 1.f) {
      u = float_u < 0.5 ? 0 : 1;
      v = float_v < 0.5 ? 0 : 1;
      return (*m_bitmap)(u, v) * m_albedo;
    }

    Color3f fColor;
    // in corners and on borders
    if (!m_filter || u >= m_bitmap->rows() - 1 || v >= m_bitmap->cols() - 1) {
      return (*m_bitmap)(u, v) * m_albedo;
    } 

    return bilerp(float_u - u, float_v - v, (*m_bitmap)(u, v),
                    (*m_bitmap)(u, v + 1), (*m_bitmap)(u + 1, v),
                    (*m_bitmap)(u + 1, v + 1)) * m_albedo;
  }

protected:
  /// Linear interpolation
  inline Color3f lerp(float t, const Color3f &v1, const Color3f &v2) const {
    return (1 - t) * v1 + t * v2;
  }
  // Bilinear interpolation
  inline Color3f bilerp(float tx, float ty, const Color3f &v00,
                        const Color3f &v01, const Color3f &v10,
                        const Color3f &v11) const {
    Color3f up = lerp(tx, v00, v01);
    Color3f bottom = lerp(tx, v10, v11);
    return lerp(ty, up, bottom);
  }

  enum TextureMode { MODULATE, REPLACE };
  TextureMode m_mode;

  Bitmap *m_bitmap;
  Color3f m_albedo;
  Vector2f m_scale;
  bool m_filter;
};
