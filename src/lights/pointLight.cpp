#include "light.h"

class PointLight : public Light {
public:
  PointLight(const PropertyList &propList) {
    m_intensity = propList.getColor("radiance", Color3f(1.f));
    m_transformation = propList.getTransform("toWorld", Transform());
    m_position = m_transformation * Point3f(0, 0, 0);
    m_flags = (int)LightFlags::DeltaPosition;
  }

  Color3f sample(const Point3f &x, const Point2f &u, float &pdf, Vector3f &wi,
                 float &dist) const {
    wi = m_position - x;
    float d2 = wi.squaredNorm();
    dist = std::sqrt(d2);
    wi /= dist;
    pdf = 1.f;
    return m_intensity / d2;
  }

  std::string toString() const {
    return tfm::format("PointLight[\n"
                       "  intensity = %s\n"
                       "  position = %s\n"
                       "]",
                       m_intensity.toString(), ::toString(m_position));
  }

protected:
  Color3f m_intensity;
  Point3f m_position;
};

REGISTER_CLASS(PointLight, "pointLight")
