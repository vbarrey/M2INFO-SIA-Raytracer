#include "disk.h"
#include "warp.h"

Disk::Disk(const PropertyList &propList) {
  m_radius = propList.getFloat("radius", 1.f);
  m_transformation = propList.getTransform("toWorld", Transform());
}

bool Disk::intersect(const Ray &ray, Hit &hit) const {
  // dot product close to zero => ray parallel to the plane
  if (ray.direction.z() == 0.f) 
    return false;

  float t = -ray.origin.z() / ray.direction.z();

  if (t <= 0.f)
    return false;

  Point3f pos = ray.at(t);

  float dist2 = pos.x() * pos.x() + pos.y() * pos.y();
  if (dist2 > m_radius * m_radius)
    return false;

  hit.t = t;

  if (ray.shadowRay)
      return true;

  hit.localFrame = Frame(Normal3f(0.0, 0.0, 1.0));

  float phi = std::atan2(pos.y(), pos.x());
  if (phi < 0)
    phi += 2.f * M_PI;
  float u = phi * INV_TWOPI;
  float rHit = std::sqrt(dist2);
  float v = (m_radius - rHit) / m_radius;

  hit.uv = Point2f(u, v);

  return true;
}

void Disk::sample(const Point2f &sample, Point3f &p, Normal3f &n,
                  float &pdf) const {
  Point2f pos = Warp::squareToUniformDisk(sample);
  pdf = Warp::squareToUniformDiskPdf(pos) / area();
  p = m_transformation * Point3f(pos.x() * m_radius, pos.y() * m_radius, 0);
  n = m_transformation * Normal3f(0, 0, 1);
}

REGISTER_CLASS(Disk, "disk")