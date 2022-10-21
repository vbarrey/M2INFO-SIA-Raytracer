#include "bsdf.h"
#include "frame.h"
#include "integrator.h"
#include "scene.h"

class FlatIntegrator : public Integrator {
public:
  FlatIntegrator(const PropertyList &props) { /* No parameters this time */
  }

  Color3f Li(const Scene *scene, Sampler *sampler, const Ray &ray) const {
    /* Find the surface that is visible in the requested direction */
    Hit hit;
    scene->intersect(ray, hit);
    if (!hit.foundIntersection())
      return Color3f(0.0f);

    /* Return the object albedo*/
    BSDFQueryRecord query = BSDFQueryRecord(hit.toLocal(-ray.direction));
    query.uv = hit.uv;
    Color3f albedo = hit.shape->bsdf()->sample(query, Point2f::Zero());
    return albedo;
  }

  std::string toString() const { return "FlatIntegrator[]"; }
};

REGISTER_CLASS(FlatIntegrator, "flat")
