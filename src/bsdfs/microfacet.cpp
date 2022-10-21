/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include "bsdf.h"
#include "core/warp.h"
#include "frame.h"

class Microfacet : public BSDF {
public:
  Microfacet(const PropertyList &propList) {
    /* RMS surface roughness */
    m_alpha = propList.getFloat("alpha", 0.1f);

    /* Interior IOR (default: BK7 borosilicate optical glass) */
    m_intIOR = propList.getFloat("intIOR", 1.5046f);

    /* Exterior IOR (default: air) */
    m_extIOR = propList.getFloat("extIOR", 1.000277f);

    /* Albedo of the diffuse base material (a.k.a "kd") */
    m_kd = propList.getColor("kd", Color3f(0.5f));

    /* To ensure energy conservation, we must scale the
       specular component by 1-kd.

       While that is not a particularly realistic model of what
       happens in reality, this will greatly simplify the
       implementation. Please see the course staff if you're
       interested in implementing a more realistic version
       of this BRDF. */
    m_ks = 1 - m_kd.maxCoeff();
  }

  /// Evaluate the BRDF for the given pair of directions
  Color3f eval(const BSDFQueryRecord &bRec) const {
    throw RTException("MicrofacetBRDF::eval not implemented yet");
    return Color3f(0.f);
  }

  /// Evaluate the sampling density of \ref sample() wrt. solid angles
  float pdf(const BSDFQueryRecord &bRec) const {
    throw RTException("MicrofacetBRDF::pdf not implemented yet");
    return 0.f;
  }

  /// Sample the BRDF
  Color3f sample(BSDFQueryRecord &bRec, const Point2f &sample) const {
    throw RTException("MicrofacetBRDF::sample not implemented yet");
    return Color3f(0.f);
  }

  bool isDiffuse() const {
    /* While microfacet BRDFs are not perfectly diffuse, they can be
       handled by sampling techniques for diffuse/non-specular materials,
       hence we return true here */
    return true;
  }

  std::string toString() const {
    return tfm::format("Microfacet[\n"
                       "  alpha = %f,\n"
                       "  intIOR = %f,\n"
                       "  extIOR = %f,\n"
                       "  kd = %s,\n"
                       "  ks = %f\n"
                       "]",
                       m_alpha, m_intIOR, m_extIOR, m_kd.toString(), m_ks);
  }

private:
  float m_alpha;
  float m_intIOR, m_extIOR;
  float m_ks;
  Color3f m_kd;
};

REGISTER_CLASS(Microfacet, "microfacet");
