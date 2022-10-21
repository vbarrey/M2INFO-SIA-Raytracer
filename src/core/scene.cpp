#include "scene.h"
#include "lights/areaLight.h"
#include "shapes/mesh.h"
#include "shapes/sphere.h"

#include <Eigen/Geometry>

Scene::Scene(const PropertyList &props) {
  m_backgroundColor = props.getColor("background", Color3f(0.0));
  m_envMap = nullptr;
}

void Scene::clear() {
  m_shapeList.clear();
  m_lightList.clear();
  if (m_camera)
    delete m_camera;
  m_camera = nullptr;
  if (m_sampler)
    delete m_sampler;
  m_sampler = nullptr;
  if (m_integrator)
    delete m_integrator;
  m_integrator = nullptr;
  if (m_envMap)
    delete m_envMap;
  m_envMap = nullptr;
}

void Scene::activate() {
  if (!m_integrator)
    throw RTException("No integrator was specified!");
  if (!m_camera)
    throw RTException("No camera was specified!");

  if (!m_sampler) {
    /* Create a default (independent) sampler */
    m_sampler = static_cast<Sampler *>(
        ObjectFactory::createInstance("independent", PropertyList()));
  }

  for (AreaLight *al : m_areaLightList) {
    addChild(al->shape());
  }

  cout << endl;
  cout << "Configuration: " << toString() << endl;
  cout << endl;
}

Color3f Scene::backgroundColor(const Vector3f &direction) const {
  if (m_envMap)
    return m_envMap->intensity(direction);
  return m_backgroundColor;
}

/** Search for the nearest intersection between the ray and the object list */
void Scene::intersect(const Ray &ray, Hit &hit) const {
  for (uint32_t i = 0; i < m_shapeList.size(); ++i) {
    // apply transformation
    Transform invTransform = m_shapeList[i]->transformation().inverse();
    Ray local_ray = invTransform * ray;
    float old_t = hit.t;
    if (hit.foundIntersection()) {
      // If previous intersection found, transform intersection point
      Point3f x = ray.at(hit.t);
      hit.t = (invTransform * x - local_ray.origin).norm();
    }
    Hit h;
    h.t = hit.t;
    m_shapeList[i]->intersect(local_ray, h);

    if (h.t < hit.t) {
      // we found a new closest intersection point for this object, record it:
      hit.shape = m_shapeList[i];
      Point3f x = local_ray.at(h.t);
      hit.localFrame = Frame(
          (m_shapeList[i]->transformation() * h.localFrame.s).normalized(),
          (m_shapeList[i]->transformation() * h.localFrame.t).normalized(),
          Normal3f((m_shapeList[i]->transformation() * h.localFrame.n)
                       .normalized()));
      hit.t = (m_shapeList[i]->transformation() * x - ray.origin).norm();
      hit.uv = h.uv;
    } else {
      hit.t = old_t;
    }
  }
}

void Scene::addChild(Object *obj) {
  switch (obj->getClassType()) {
  case EShape: {
    Shape *shape = static_cast<Shape *>(obj);
    m_shapeList.push_back(shape);
  } break;

  case ELight: {
    Light *light = static_cast<Light *>(obj);
    m_lightList.push_back(light);
    if (light->flags() & (int)LightFlags::Area)
      m_areaLightList.push_back(dynamic_cast<AreaLight *>(light));
    if (light->flags() & (int)LightFlags::Infinite) {
      if (m_envMap)
        throw RTException("There can only be one env. map per scene!");
      m_envMap = light;
    }
  } break;

  case ESampler:
    if (m_sampler)
      throw RTException("There can only be one sampler per scene!");
    m_sampler = static_cast<Sampler *>(obj);
    break;

  case ECamera:
    if (m_camera)
      throw RTException("There can only be one camera per scene!");
    m_camera = static_cast<Camera *>(obj);
    break;

  case EIntegrator:
    if (m_integrator)
      throw RTException("There can only be one integrator per scene!");
    m_integrator = static_cast<Integrator *>(obj);
    break;

  default:
    throw RTException("Scene::addChild(<%s>) is not supported!",
                      classTypeName(obj->getClassType()));
  }
}

std::string Scene::toString() const {
  std::string shapes;
  for (size_t i = 0; i < m_shapeList.size(); ++i) {
    shapes += std::string("  ") + indent(m_shapeList[i]->toString(), 2);
    if (i + 1 < m_shapeList.size())
      shapes += ",";
    shapes += "\n";
  }
  std::string lights;
  for (size_t i = 0; i < m_lightList.size(); ++i) {
    lights += std::string("  ") + indent(m_lightList[i]->toString(), 2);
    if (i + 1 < m_lightList.size())
      lights += ",";
    lights += "\n";
  }

  return tfm::format(
      "Scene[\n"
      "  background = %s,\n"
      "  integrator = %s,\n"
      "  sampler = %s,\n"
      "  camera = %s,\n"
      "  shapes = {\n"
      "  %s  }\n"
      "  lights = {\n"
      "  %s  }\n"
      "]",
      m_envMap ? "envmap" : m_backgroundColor.toString(),
      indent(m_integrator->toString()), indent(m_sampler->toString()),
      indent(m_camera->toString()), indent(shapes, 2), indent(lights, 2));
}

REGISTER_CLASS(Scene, "scene")
