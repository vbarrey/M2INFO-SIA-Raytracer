/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include "bsdf.h"
#include "frame.h"
#include "texture.h"
#include "warp.h"

/**
 * \brief Phong BRDF model (energy conservative)
 */
class Phong : public BSDF {
public:
  Phong(const PropertyList &propList) {
    /* Albedo of the diffuse base material (a.k.a "kd") */
    m_kd = propList.getColor("kd", Color3f(0.5f));
    /* Albedo of the specular base material (a.k.a "ks") */
    m_ks = propList.getColor("ks", Color3f(0.5f));
    /* Roughness of the material (a.k.a "exponent") */
    m_exponent = propList.getFloat("exponent", 0.1f);

    // Load texture if any, otherwise use diffuse albedo
    m_texture = new Texture(m_kd, propList);

    m_specularSamplingWeight = m_ks.mean() / (m_kd.mean() + m_ks.mean());
  }

  ~Phong() {
    if (m_texture)
      delete m_texture;
  }

  /// Evaluate the BRDF model
  Color3f eval(const BSDFQueryRecord &bRec) const {
    /* This is a smooth BRDF -- return zero if the measure
       is wrong, or when queried for illumination on the backside */
    if (bRec.measure != ESolidAngle || Frame::cosTheta(bRec.wi) <= 0 ||
        Frame::cosTheta(bRec.wo) <= 0)
      return Color3f(0.0f);

    float alpha = bRec.wo.dot(Frame::reflect(bRec.wi));

    Color3f color = m_texture->lookUp(bRec.uv) * INV_PI;

    if (alpha > 0.0f) {
      color += m_ks * (m_exponent + 2.f) * std::pow(alpha, m_exponent)
                       * INV_TWOPI;
    }

    return color;
  }

  /// Compute the density of \ref sample() wrt. solid angles
  float pdf(const BSDFQueryRecord &bRec) const {
    /* This is a smooth BRDF -- return zero if the measure
       is wrong, or when queried for illumination on the backside */
    if (bRec.measure != ESolidAngle || Frame::cosTheta(bRec.wi) <= 0 ||
        Frame::cosTheta(bRec.wo) <= 0)
      return 0.0f;

    float alpha = bRec.wo.dot(Frame::reflect(bRec.wi));
    float specProb = 0.0f;
    if (alpha > 0)
      specProb =
          std::pow(alpha, m_exponent) * (m_exponent + 1.0f) * INV_TWOPI;

    float diffuseProb = Warp::squareToCosineHemispherePdf(bRec.wo);
    return m_specularSamplingWeight * specProb +
           (1 - m_specularSamplingWeight) * diffuseProb;
  }

  /// Draw a sample from the BRDF model
  Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample) const {
    if (Frame::cosTheta(bRec.wi) <= 0)
      return Color3f(1.f, 0.f, 0.0f);

    bRec.measure = ESolidAngle;

    /* Relative index of refraction: no change */
    bRec.eta = 1.0f;

    float u = sample.x();
    float v = sample.y();
    bool chooseSpecular = true;
    if (u <= m_specularSamplingWeight) {
      u /= m_specularSamplingWeight;
    } else {
      u = (u - m_specularSamplingWeight) / (1.f - m_specularSamplingWeight);
      chooseSpecular = false;
    }

    if (chooseSpecular) {
      Vector3f R = Frame::reflect(bRec.wi);
      float phi = 2.f * M_PI * u;
      float cos_theta = std::pow(v, 1 / (m_exponent + 1.f));
      float sin_theta = std::sqrt(1 - cos_theta * cos_theta);
      Vector3f localDir(std::cos(phi) * sin_theta, std::sin(phi) * sin_theta, cos_theta);
      bRec.wo = Frame(R).toWorld(localDir);
    } else {
      bRec.wo = Warp::squareToCosineHemisphere(Vector2f(u,v));
    }

    Color3f m_eval = eval(bRec);
    if ((m_eval == 0).all())
      return Color3f(0.f);

    return m_eval * Frame::cosTheta(bRec.wo) / pdf(bRec);
  }

  bool isDiffuse() const { return true; }

  /// Return a human-readable summary
  std::string toString() const {
    return tfm::format("Phong[\n"
                       "  kd = %s\n"
                       "  ks = %s\n"
                       "  exponent = %f\n"
                       "]",
                       m_kd.toString(), m_ks.toString(), m_exponent);
  }

  EClassType getClassType() const { return EBSDF; }

private:
  Color3f m_kd, m_ks;
  float m_exponent, m_specularSamplingWeight;
  Texture *m_texture;
};

REGISTER_CLASS(Phong, "phong");
