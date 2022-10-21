#include "bsdf.h"
#include "integrator.h"
#include "lights/areaLight.h"
#include "scene.h"
#include "warp.h"

#define STRATIFIED

class Whitted : public Integrator {
public:
  Whitted(const PropertyList &props) {
    m_maxRecursion = props.getInteger("maxRecursion", 4);
  }

  Color3f Li(const Scene *scene, Sampler *sampler, const Ray &ray) const {

    Color3f radiance = Color3f::Zero();

    // stopping criteria:
    if (ray.recursionLevel >= m_maxRecursion) {
      return radiance;
    }

    // Find the surface that is visible in the requested direction
    Hit hit;
    scene->intersect(ray, hit);
    if (!hit.foundIntersection()) {
      return scene->backgroundColor(ray.direction);
    }

    Normal3f normal = hit.localFrame.n;
    Point3f pos = ray.at(hit.t);

    const BSDF *bsdf = hit.shape->bsdf();

    // Recursively trace a ray for mirror and dielectric materials
    if (!bsdf->isDiffuse()) {
      BSDFQueryRecord query(hit.toLocal(-ray.direction));
      query.uv = hit.uv;
      Color3f weighted_brdf = bsdf->sample(query, sampler->next2D());
      Ray r;
      Vector3f sampleDir = hit.toWorld(query.wo);
      if (sampleDir.dot(normal) < 0) { // transmission
        r = Ray(pos - normal * Epsilon, sampleDir);
      } else { // reflection
        r = Ray(pos + normal * Epsilon, sampleDir);
      }
      r.recursionLevel = ray.recursionLevel + 1;
      return weighted_brdf * Li(scene, sampler, r);
    }

    // Compute direct lighting
    for (Light *light : scene->lightList()) {
      Vector3f lightDir;
      float dist, pdf;
      Color3f intensity =
          light->sample(pos, sampler->next2D(), pdf, lightDir, dist);
      if (pdf <= Epsilon)
        continue;
      Ray shadowRay(pos + normal * Epsilon, lightDir, true);
      Hit shadowHit;
      scene->intersect(shadowRay, shadowHit);
      if (shadowHit.shape != light->shape() && shadowHit.t < dist) {
        continue;
      }
      float cos_term = std::max(0.f, lightDir.dot(normal));
      Color3f brdf = bsdf->eval(BSDFQueryRecord(hit.toLocal(-ray.direction),
                                                hit.toLocal(lightDir),
                                                ESolidAngle, hit.uv));
      radiance += intensity * cos_term * brdf / pdf;
    }
    return radiance;
  }

  std::string toString() const {
    return tfm::format("Whitted[\n"
                       "  max recursion = %f\n"
                       " ]",
                       m_maxRecursion);
  }

private:
  int m_maxRecursion;
};

REGISTER_CLASS(Whitted, "whitted")
