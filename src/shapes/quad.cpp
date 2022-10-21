#include "quad.h"

Quad::Quad(const PropertyList &propList) {
  m_transformation = propList.getTransform("toWorld", Transform());
  m_infinite = false;
  try {
    Vector3f s = propList.getVector("size");
    m_size = s.head<2>();
  } catch (const RTException &e) {
    m_infinite = true;
  }
}

bool Quad::intersect(const Ray &ray, Hit &hit) const {
  // dot product close to zero => ray parallel to the plane
  if (ray.direction.z() == 0.f) 
    return false;

  float t = -ray.origin.z() / ray.direction.z();

  if (t <= 0.f)
    return false;

  Point3f pos = ray.at(t);
  float u = pos.dot(Vector3f::UnitX());
  float v = pos.dot(Vector3f::UnitY());
  if (!m_infinite && (u < -m_size[0] * 0.5f || v < -m_size[1] * 0.5f ||
                      u > m_size[0] * 0.5f || v > m_size[1] * 0.5f))
    return false;

  hit.t = t;
  
  if (ray.shadowRay)
      return true;

  hit.localFrame = Frame(Normal3f(0.0, 0.0, 1.0));
  if (m_infinite)
    hit.uv = Point2f(pos.x(), pos.y());
  else
    hit.uv = Point2f(u / m_size[0] + 0.5f, v / m_size[1] + 0.5f);
  return true;
}

void Quad::sample(const Point2f &sample, Point3f &p, Normal3f &n,
                  float &pdf) const {
  if (m_infinite) {
    RTException("Quad::sample: cannot sample an infinite plane");
  }
  p = m_transformation * Point3f((sample.x() - 0.5f) * size().x(),
                                 (sample.y() - 0.5f) * size().y(), 0.f);
  n = m_transformation * Normal3f(0, 0, 1);
  pdf = 1.f / area();
}

REGISTER_CLASS(Quad, "quad")
