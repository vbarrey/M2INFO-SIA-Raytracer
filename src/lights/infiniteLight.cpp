#include "light.h"
#include "lightProbe.h"

class InfiniteLight : public Light {
public:
  InfiniteLight(const PropertyList &propList) {
    m_transformation = propList.getTransform("toWorld", Transform());
    m_envmap = new LightProbe(Color3f(0), propList);
    m_filename = propList.getString("texture", "");
    m_flags = (int)LightFlags::Infinite;
  }

  ~InfiniteLight() {
    if (m_envmap)
      delete m_envmap;
    m_envmap = nullptr;
  }

  virtual Color3f intensity(const Vector3f &direction) const {
    return m_envmap->intensity(m_transformation.inverse() * direction);
  }

  Color3f sample(const Point3f &x, const Point2f &u, float &pdf,
                 Vector3f &wi, float &dist) const {
    dist = std::numeric_limits<float>::max();
    // Convert infinite light sample point to direction
    float theta = u.y() * M_PI, phi = u.x() * 2.f * M_PI;
    wi = m_transformation * sphericalDirection(theta, phi);
    pdf = 0.f;
    float sinTheta = std::sin(theta);
    if (sinTheta > Epsilon)
      pdf = 1.f / (2.f * M_PI * M_PI * sinTheta);
    return m_envmap->intensity(u);
  }

  std::string toString() const {
    return tfm::format("InfiniteLight[\n"
                       "  frame = %s,\n"
                       "  envmap = %s\n"
                       "]",
                       indent(m_transformation.toString(), 10),
                       m_filename);
  }

private:
  LightProbe *m_envmap;
  std::string m_filename;
};

REGISTER_CLASS(InfiniteLight, "infiniteLight")
