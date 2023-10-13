#include "bsdf.h"
#include "core/warp.h"
#include "integrator.h"
#include "scene.h"

class AO : public Integrator {
public:
  AO(const PropertyList &props) {
    m_sampleCount = props.getInteger("sampleCount", 32);
    m_cosineWeighted = props.getBoolean("cosineWeighted", true);
  }

  Color3f Li(const Scene *scene, Sampler *sampler, const Ray &ray) const {
    /* Find the surface that is visible in the requested direction */
    Hit hit;
    scene->intersect(ray, hit);
    if (!hit.foundIntersection())
      return scene->backgroundColor();

    Normal3f normal = hit.localFrame.n;
    Point3f pos = ray.at(hit.t);
    
    float integSum = 0.f;
    for(int i=0; i< m_sampleCount; i++){
      // Envoie de rayon sur l'émisphere
      Vector3f wkLocal = (m_cosineWeighted ? Warp::squareToCosineHemisphere(sampler->next2D()) : Warp::squareToUniformHemisphere(sampler->next2D()));
      // Passage de repère local au repère monde
      Vector3f wkWorld = hit.toWorld(wkLocal).normalized();
      Ray r = Ray(pos + normal * Epsilon,  wkWorld,true);
      Hit shadow;
      scene->intersect(r,shadow);
      if(!shadow.foundIntersection()){ 
        float cosTerm = std::max(0.f, normal.dot(wkWorld));
        float pdf = ( m_cosineWeighted ? Warp::squareToCosineHemispherePdf(wkLocal) : Warp::squareToUniformHemispherePdf(wkLocal));
        integSum +=  cosTerm / pdf;
      }
    }

    return Color3f(integSum/(M_PI*m_sampleCount));
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
