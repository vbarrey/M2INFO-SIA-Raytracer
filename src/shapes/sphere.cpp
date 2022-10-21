#include "sphere.h"
#include "warp.h"

Sphere::Sphere(float radius) : m_radius(radius) {}

Sphere::Sphere(const PropertyList &propList) {
  m_radius = propList.getFloat("radius", 1.f);
  m_transformation = propList.getTransform("toWorld", Transform());
}

bool Sphere::intersect(const Ray &ray, Hit &hit) const {
  float b = 2. * ray.direction.dot(ray.origin);
  float c = (ray.origin).squaredNorm() - m_radius * m_radius;
  float discr = b * b - 4. * c;
  if (discr >= 0) {
    discr = std::sqrt(discr);
    float t = 0.5 * (-b - discr);
    if (t < Epsilon)
      t = 0.5 * (-b + discr);
    if (t < Epsilon || t > hit.t)
      return false;

    hit.t = t;

    if (ray.shadowRay)
      return true;

    Point3f point = ray.at(t);
    Normal3f n = point.normalized();
    Vector3f x = Vector3f(0, 1, 0) - Vector3f(0, 1, 0).dot(n) * n;
    x.normalize();
    Vector3f y = n.cross(x);
    hit.localFrame = Frame(x, y, n);

    // Texture coordinates
    float phi = std::atan2(point.y(), point.x());
    if (phi < 0.f) phi += 2.f * M_PI;
    float theta = std::acos(clamp(point.z() / m_radius, -1.f, 1.f));
    hit.uv = Point2f(phi / (2.f * M_PI), theta / M_PI);
    return true;
  }
  return false;
}

void Sphere::sample(const Point2f &sample, Point3f &p, Normal3f &n,
                    float &pdf) const {
  Point3f pos = Warp::squareToUniformSphere(sample);
  pdf = 1.f / area();
  pos *= m_radius;
  p = m_transformation * pos;
  n = (m_transformation * Normal3f(pos.x(), pos.y(), pos.z())).normalized();
}

REGISTER_CLASS(Sphere, "sphere")
