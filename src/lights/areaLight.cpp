#include "lights/areaLight.h"
#include "core/bsdf.h"
#include "core/texture.h"
#include "core/warp.h"

AreaLight::AreaLight(const PropertyList &propList) {
  m_intensity = propList.getColor("radiance", Color3f(1.f));
  m_twoSided = propList.getBoolean("twoSided", false);
  m_texture = new Texture(m_intensity, propList);
  m_flags = (int)LightFlags::Area;
}

void AreaLight::addChild(Object *obj) {
  switch (obj->getClassType()) {
  case EShape:
    if (m_shape)
      throw RTException(
          "AreaLight: tried to register multiple shape instances!");
    m_shape = static_cast<Shape *>(obj);
    break;
  default:
    throw RTException("AreaLight::addChild(<%s>) is not supported!",
                      classTypeName(obj->getClassType()));
  }
}

Color3f AreaLight::intensity(const Vector2f &uv, const Normal3f &n,
                             const Vector3f &w) const {
  if(n.dot(w) <= 0 && !m_twoSided)
    return Color3f(0.f);

  return m_texture->lookUp(uv);
}

Color3f AreaLight::sample(const Point3f &x, const Point2f &sample, float &pdf,
                          Vector3f &wi, float &dist) const {
  
  Point3f p;
  Normal3f n;
  float pdf;
  
  m_shape->sample(sample, p, n, pdf);
}

std::string AreaLight::toString() const {
  return tfm::format("AreaLight[\n"
                     "  intensity = %s,\n"
                     "  two-sided = %f,\n"
                     "  shape = %f,\n"
                     "]",
                     m_intensity.toString(), m_twoSided ? "true" : "false",
                     indent(m_shape->toString()));
}

REGISTER_CLASS(AreaLight, "areaLight")
