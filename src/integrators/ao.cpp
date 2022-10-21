#include "bsdf.h"
#include "core/warp.h"
#include "integrator.h"
#include "scene.h"

class AO : public Integrator {
public:
  AO(const PropertyList &props) {
    m_sampleCount = props.getInteger("sampleCount", 10);
    m_cosineWeighted = props.getBoolean("cosineWeighted", true);
  }

  Color3f Li(const Scene *scene, Sampler *sampler, const Ray &ray) const {
    throw(RTException("AO::Li not implemented yet"));

    return Color3f(0.f);
  }

  std::string toString() const {
    return tfm::format("AO[\n"
                       "  samples = %f\n"
                       "  cosine-weighted = %s\n"
                       " ]",
                       m_sampleCount, m_cosineWeighted ? "true" : "false");
  }

private:
  int m_sampleCount;
  bool m_cosineWeighted;
};

REGISTER_CLASS(AO, "ao")
