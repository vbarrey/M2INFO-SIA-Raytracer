/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include "bsdf.h"
#include "frame.h"
#include "texture.h"

/// Ideal mirror BRDF
class Mirror : public BSDF {
public:
  Mirror(const PropertyList &propList) {
    m_albedo = propList.getColor("albedo", Color3f(1.0f));

    // Load texture if any, otherwise use albedo
    m_texture = new Texture(m_albedo, propList);
  }

  ~Mirror() {
    if (m_texture)
      delete m_texture;
  }

  Color3f eval(const BSDFQueryRecord &) const {
    /* Discrete BRDFs always evaluate to zero */
    return Color3f(0.0f);
  }

  float pdf(const BSDFQueryRecord &) const {
    /* Discrete BRDFs always evaluate to zero */
    return 0.0f;
  }

  Color3f sample(BSDFQueryRecord &bRec, const Point2f &) const {
    if (Frame::cosTheta(bRec.wi) <= 0)
      return Color3f(0.0f);

    // Reflection in local coordinates
    bRec.wo = Frame::reflect(bRec.wi);
    bRec.measure = EDiscrete;

    /* Relative index of refraction: no change */
    bRec.eta = 1.0f;

    return m_texture->lookUp(bRec.uv);
  }

  /// Return a human-readable summary
  std::string toString() const {
    return tfm::format("Mirror[\n"
                       "  albedo = %s\n"
                       "]",
                       m_albedo.toString());
  }

private:
  Color3f m_albedo;
  Texture *m_texture;
};

REGISTER_CLASS(Mirror, "mirror");
