#include "shapes/mesh.h"
#include "accelerators/bvh.h"
#include "warp.h"

#include <filesystem/resolver.h>
#include <fstream>
#include <iostream>
#include <limits>
#include <tiny_obj_loader.h>

Mesh::Mesh(const PropertyList &propList) : m_BVH(nullptr) {
  m_transformation = propList.getTransform("toWorld", ::Transform());
  std::string filename = propList.getString("filename");
  loadFromFile(filename);
  buildBVH();
}

void Mesh::activate() {
  if (!m_bsdf) {
    /* If no material was assigned, instantiate a diffuse BRDF */
    m_bsdf = static_cast<BSDF *>(
        ObjectFactory::createInstance("diffuse", PropertyList()));
  }
  m_PDF = DiscretePDF(m_faces.size());
  for (uint32_t i = 0; i < m_faces.size(); i++) {
    Point3f p0 = m_transformation * vertexOfFace(i, 0).position;
    Point3f p1 = m_transformation * vertexOfFace(i, 1).position;
    Point3f p2 = m_transformation * vertexOfFace(i, 2).position;
    float area = Vector3f(0.5f * (p1 - p0).cross(p2 - p0)).norm();
    m_PDF.append(area);
  }
  m_area = m_PDF.normalize();
}

void Mesh::loadFromFile(const std::string &filename) {
  filesystem::path filepath = getFileResolver()->resolve(filename);
  std::ifstream is(filepath.str());
  if (is.fail())
    throw RTException("Unable to open mesh file \"%s\"!", filepath.str());

  const std::string ext = filepath.extension();
  if (ext == "off" || ext == "OFF")
    loadOFF(filepath.str());
  else if (ext == "obj" || ext == "OBJ")
    loadOBJ(filepath.str());
  else
    std::cerr << "Mesh: extension \'" << ext << "\' not supported."
              << std::endl;
}

void Mesh::loadOFF(const std::string &filename) {
  std::ifstream in(filename.c_str(), std::ios::in);
  if (!in) {
    std::cerr << "File not found " << filename << std::endl;
    return;
  }

  std::string header;
  in >> header;

  // check the header file
  if (header != "OFF") {
    std::cerr << "Wrong header = " << header << std::endl;
    return;
  }

  int nofVertices, nofFaces, inull;
  int nb, id0, id1, id2;
  Vector3f v;

  in >> nofVertices >> nofFaces >> inull;

  for (int i = 0; i < nofVertices; ++i) {
    in >> v.x() >> v.y() >> v.z();
    m_vertices.push_back(Vertex(v));
  }

  for (int i = 0; i < nofFaces; ++i) {
    in >> nb >> id0 >> id1 >> id2;
    assert(nb == 3);
    m_faces.push_back(FaceIndex(id0, id1, id2));
  }

  in.close();

  computeNormals();
  computeBoundingBox();
}

void Mesh::loadOBJ(const std::string &filename) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string err, warn;
  bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                                  filename.c_str());

  if (!success) {
    throw RTException("Mesh::loadObj: error loading file %s: %s", filename,
                      err);
  }

  bool needNormals = false;

  int face_idx = 0;

  // Loop over shapes
  for (size_t s = 0; s < shapes.size(); s++) {
    // Loop over faces
    size_t index_offset = 0;
    for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++) {
      size_t fv = shapes[s].mesh.num_face_vertices[f];
      assert(fv == 3);

      // Loop over vertices in the face.
      for (size_t v = 0; v < fv; v++) {
        // access to vertex
        tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

        Point3f pos;
        pos[0] = attrib.vertices[3 * idx.vertex_index + 0];
        pos[1] = attrib.vertices[3 * idx.vertex_index + 1];
        pos[2] = attrib.vertices[3 * idx.vertex_index + 2];

        Vector3f n;
        if (attrib.normals.size() > 0) {
          n[0] = attrib.normals[3 * idx.normal_index + 0];
          n[1] = attrib.normals[3 * idx.normal_index + 1];
          n[2] = attrib.normals[3 * idx.normal_index + 2];
        } else {
          needNormals = true;
        }

        Vector2f tc = Vector2f::Zero();
        if (attrib.texcoords.size() > 0) {
          tc[0] = attrib.texcoords[2 * idx.texcoord_index + 0];
          tc[1] = attrib.texcoords[2 * idx.texcoord_index + 1];
        }
        m_vertices.push_back(Vertex(pos, n, tc));
      }

      m_faces.push_back(FaceIndex(face_idx, face_idx + 1, face_idx + 2));
      face_idx += 3;
      index_offset += fv;
    }
  }

  if (needNormals) {
    computeNormals();
  }
  computeBoundingBox();
}

Mesh::~Mesh() { delete m_BVH; }

void Mesh::makeUnitary() {
  Eigen::Vector3f lowest, highest;
  lowest.fill(std::numeric_limits<float>::max()); 
  highest.fill(-std::numeric_limits<float>::max());

  for (VertexArray::iterator v_iter = m_vertices.begin();
       v_iter != m_vertices.end(); ++v_iter) {
    lowest = lowest.array().min(v_iter->position.array());
    highest = highest.array().max(v_iter->position.array());
  }

  Point3f center = (lowest + highest) / 2.0;
  float m = (highest - lowest).maxCoeff();
  for (VertexArray::iterator v_iter = m_vertices.begin();
       v_iter != m_vertices.end(); ++v_iter)
    v_iter->position = (v_iter->position - center) / m;

  computeBoundingBox();
}

void Mesh::computeNormals() {
  // pass 1: set the normal to 0
  for (VertexArray::iterator v_iter = m_vertices.begin();
       v_iter != m_vertices.end(); ++v_iter)
    v_iter->normal.setZero();

  // pass 2: compute face normals and accumulate
  for (FaceIndexArray::iterator f_iter = m_faces.begin();
       f_iter != m_faces.end(); ++f_iter) {
    const Point3f v0 = m_vertices[(*f_iter)(0)].position;
    const Point3f v1 = m_vertices[(*f_iter)(1)].position;
    const Point3f v2 = m_vertices[(*f_iter)(2)].position;

    Normal3f n = (v1 - v0).cross(v2 - v0);

    m_vertices[(*f_iter)(0)].normal += n;
    m_vertices[(*f_iter)(1)].normal += n;
    m_vertices[(*f_iter)(2)].normal += n;
  }

  // pass 3: normalize
  for (VertexArray::iterator v_iter = m_vertices.begin();
       v_iter != m_vertices.end(); ++v_iter)
    v_iter->normal.normalize();
}

void Mesh::computeBoundingBox() {
  m_AABB.reset();
  for (VertexArray::iterator v_iter = m_vertices.begin();
       v_iter != m_vertices.end(); ++v_iter)
    m_AABB.expandBy(v_iter->position);
}

void Mesh::buildBVH() {
  if (m_BVH)
    delete m_BVH;
  m_BVH = new BVH;
  m_BVH->build(this, 10, 100);
}

long int Mesh::ms_itersection_count = 0;

bool Mesh::intersectFace(const Ray &ray, Hit &hit, int faceId) const {
  ms_itersection_count++;
  const Vertex &v0 = vertexOfFace(faceId, 0);
  const Vertex &v1 = vertexOfFace(faceId, 1);
  const Vertex &v2 = vertexOfFace(faceId, 2);
  Vector3f e1 = v1.position - v0.position;
  Vector3f e2 = v2.position - v0.position;
  Eigen::Matrix3f M;
  M << -ray.direction, e1, e2;
  Vector3f tuv = M.inverse() * (ray.origin - v0.position);
  float t = tuv(0), u = tuv(1), v = tuv(2);
  if (t > 0 && u >= 0 && v >= 0 && (u + v) <= 1 && t < hit.t) {
    hit.t = t;

    if (ray.shadowRay)
      return true;

    Vector3f n = u * v1.normal + v * v2.normal +
                 (1. - u - v) * v0.normal;
    hit.localFrame = Frame(n.normalized());
    hit.uv = u * v1.texcoord + v * v2.texcoord + (1. - u - v) * v0.texcoord;

    return true;
  }
  return false;
}

bool Mesh::intersect(const Ray &ray, Hit &hit) const {
  if (m_BVH) {
    // use the BVH !!
    return m_BVH->intersect(ray, hit);
  } else {
    // brute force !!
    bool ret = false;
    float tMin, tMax;
    if ((!m_AABB.rayIntersect(ray, tMin, tMax)) || tMin > hit.t)
      return false;

    for (unsigned int i = 0; i < m_faces.size(); ++i) {
      ret = ret | intersectFace(ray, hit, i);
    }
    return ret;
  }
}

void Mesh::sample(const Point2f &sample, Point3f &p, Normal3f &n,
                  float &pdf) const {
  float u = sample.x(), v = sample.y();
  int faceId = m_PDF.sampleReuse(u);
  Point2f b = Warp::squareToUniformTriangle(Point2f(u, v));
  const Vertex &v0 = vertexOfFace(faceId, 0);
  const Vertex &v1 = vertexOfFace(faceId, 1);
  const Vertex &v2 = vertexOfFace(faceId, 2);
  p = m_transformation * Point3f(b[0] * v0.position + b[1] * v1.position +
                                 (1 - b[0] - b[1]) * v2.position);
  n = m_transformation *
      Normal3f(b[0] * v0.normal + b[1] * v1.normal +
               (1. - b[0] - b[1]) * v2.normal);
  n.normalize();
  pdf = 1.f / area();
}

std::string Mesh::toString() const {
  return tfm::format("Mesh[\n"
                     "  vertexCount = %i,\n"
                     "  triangleCount = %i,\n"
                     "  BSDF = %s\n"
                     "]",
                     m_vertices.size(), m_faces.size(),
                     m_bsdf ? indent(m_bsdf->toString()) : std::string("null"));
}

REGISTER_CLASS(Mesh, "mesh")
