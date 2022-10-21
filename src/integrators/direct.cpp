#include "bsdf.h"
#include "integrator.h"
#include "scene.h"
#include "frame.h"

class Direct : public Integrator {
public:
  Direct(const PropertyList &props) {}

  Color3f Li(const Scene *scene, Sampler *sampler, const Ray &ray) const {
    /* Find the surface that is visible in the requested direction */
    Hit hit;
    scene->intersect(ray, hit);
    if (!hit.foundIntersection())
      return scene->backgroundColor();

    Color3f radiance = Color3f::Zero();
    const BSDF *bsdf = hit.shape->bsdf();

    Normal3f normal = hit.localFrame.n;
    Point3f pos = ray.at(hit.t);

    const LightList &lights = scene->lightList();
    for (LightList::const_iterator it = lights.begin(); it != lights.end();
         ++it) {
      float dist, pdf;
      Vector3f lightDir;
      Color3f intensity = (*it)->sample(pos, sampler->next2D(), pdf, lightDir, dist);
      Ray shadowRay(pos + normal * Epsilon, lightDir, true);
      Hit shadowHit;
      scene->intersect(shadowRay, shadowHit);
      if (!shadowHit.foundIntersection() || shadowHit.t > dist) {
        float cos_term = std::max(0.f, lightDir.dot(normal));
        Color3f brdf =
            bsdf->eval(BSDFQueryRecord(hit.toLocal(-ray.direction),
                                       hit.toLocal(lightDir),
                                       ESolidAngle, hit.uv));
        radiance += intensity * cos_term * brdf;
      }
    }

    return radiance;
  }

  std::string toString() const { return "Direct[]"; }
};

REGISTER_CLASS(Direct, "direct")
