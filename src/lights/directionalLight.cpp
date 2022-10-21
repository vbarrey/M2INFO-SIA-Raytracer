#include "light.h"

class DirectionalLight : public Light {
public:
  DirectionalLight(const PropertyList &propList) {
    m_intensity = propList.getColor("radiance", Color3f(1.f));
    m_transformation = propList.getTransform("toWorld", Transform());
    Vector3f direction =
        propList.getVector("direction", Vector3f(1.f, 0.f, 0.f)).normalized();
    m_direction = m_transformation * direction;
    m_flags = (int)LightFlags::DeltaDirection;
  }

  Color3f sample(const Point3f &x, const Point2f &u, float &pdf,
                 Vector3f &wi, float &dist) const {
    wi = -m_direction;
    dist = std::numeric_limits<float>::max();
    pdf = 1.f;
    return m_intensity;
  }

  std::string toString() const {
    return tfm::format("DirectionalLight[\n"
                       "  intensity = %s\n"
                       "  direction = %s\n"
                       "]",
                       m_intensity.toString(), ::toString(m_direction));
  }

protected:
  Color3f m_intensity;
  Vector3f m_direction;
};

REGISTER_CLASS(DirectionalLight, "directionalLight")
