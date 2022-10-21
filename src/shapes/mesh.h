#pragma once

#include "accelerators/bvh.h"
#include "common.h"
#include "shape.h"
#include "dpdf.h"

#include <string>
#include <vector>

/** \class Mesh
 * A class to represent a 3D triangular mesh
 */
class Mesh : public Shape {
public:
  static long int ms_itersection_count;

  /** Represents a vertex of the mesh */
  struct Vertex {
    Vertex()
        : position(Point3f::Zero()), normal(Normal3f::Zero()),
          texcoord(Vector2f::Zero()) {}

    Vertex(const Point3f &pos)
        : position(pos), normal(Vector3f::Zero()), texcoord(Vector2f::Zero()) {}

    Vertex(const Point3f &pos, const Vector3f &n, const Vector2f &tc)
        : position(pos), normal(n), texcoord(tc) {}

    Point3f position;
    Normal3f normal;
    Vector2f texcoord;
  };

  Mesh(const PropertyList &propList);

  /** Destructor */
  virtual ~Mesh();

  virtual void activate();

  void loadFromFile(const std::string &filename);

  /** Loads a triangular mesh in the OFF format */
  void loadOFF(const std::string &filename);

  /** Loads a triangular mesh in the OBJ format */
  void loadOBJ(const std::string &filename);

  /** Compute the intersection between a ray and the mesh */
  virtual bool intersect(const Ray &ray, Hit &hit) const;

  /** Compute the intersection between a ray and a given triangular face */
  bool intersectFace(const Ray &ray, Hit &hit, int faceId) const;

  void makeUnitary();
  void computeNormals();
  void computeBoundingBox();
  void buildBVH();

  /// \returns  the number of faces
  int nbFaces() const { return m_faces.size(); }

  virtual float area() const { return m_area; };

  virtual void sample(const Point2f &sample, Point3f &p, Normal3f &n,
                      float &pdf) const;

  /// \returns a const references to the \a vertexId -th vertex of the \a faceId
  /// -th face. vertexId must be between 0 and 2 !!
  const Vertex &vertexOfFace(int faceId, int vertexId) const {
    return m_vertices[m_faces[faceId](vertexId)];
  }

  virtual const BoundingBox3f &getBoundingBox() const { return m_AABB; }

  /// \returns a human-readable summary of this instance
  std::string toString() const;

  BVH *bvh() { return m_BVH; }

protected:
  /** Represent a triangular face via its 3 vertex indices. */
  typedef Eigen::Vector3i FaceIndex;

  /** Represents a sequential list of vertices */
  typedef std::vector<Vertex> VertexArray;

  /** Represents a sequential list of triangles */
  typedef std::vector<FaceIndex> FaceIndexArray;

  /** The list of vertices */
  VertexArray m_vertices;
  /** The list of face indices */
  FaceIndexArray m_faces;

  /** The bounding box of the mesh */
  BoundingBox3f m_AABB;

  /** Area of the mesh **/
  float m_area;

  /** Bounding Volume Hierarchy **/
  BVH *m_BVH;

  DiscretePDF m_PDF;
};
