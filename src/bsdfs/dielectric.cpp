/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include "bsdf.h"
#include "frame.h"
#include "texture.h"

/// Ideal dielectric BSDF
class Dielectric : public BSDF {
public:
  Dielectric(const PropertyList &propList) {
    /* Interior IOR (default: BK7 borosilicate optical glass) */
    m_intIOR = propList.getFloat("intIOR", 1.5046f);

    /* Exterior IOR (default: air) */
    m_extIOR = propList.getFloat("extIOR", 1.000277f);

    m_albedo = propList.getColor("albedo", Color3f(1.0f));

    // Load texture if any, otherwise use albedo
    m_texture = new Texture(m_albedo, propList);
  }

  ~Dielectric() {
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

  Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample) const {
    float eta = m_intIOR / m_extIOR;
    float cosThetaT;
    float f = fresnel(Frame::cosTheta(bRec.wi), m_extIOR, m_intIOR, cosThetaT);
    float pdf;

    if (sample.x() <= f) { // reflection
        bRec.wo = Frame::reflect(bRec.wi);
        bRec.eta = 1.0f;
        pdf = f;
    } else { // transmission
        bRec.wo = Frame::refract(bRec.wi, cosThetaT, eta);
        bRec.eta = cosThetaT < 0 ? eta : 1.f / eta;
        pdf = 1.f - f;
    }
    bRec.measure = EDiscrete;

    if(pdf < Epsilon)
        return Color3f(0.f);
    return m_texture->lookUp(bRec.uv);
  }

  std::string toString() const {
    return tfm::format("Dielectric[\n"
                       "  intIOR = %f,\n"
                       "  extIOR = %f,\n"
                       "  albedo = %s\n"
                       "]",
                       m_intIOR, m_extIOR, m_albedo.toString());
  }

private:
  float m_intIOR, m_extIOR;
  Color3f m_albedo;
  Texture *m_texture;
};

REGISTER_CLASS(Dielectric, "dielectric");
