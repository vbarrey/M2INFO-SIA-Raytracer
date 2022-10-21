/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob

    Nori is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License Version 3
    as published by the Free Software Foundation.

    Nori is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "warp.h"
#include "frame.h"
#include "vector.h"

Point2f Warp::squareToUniformSquare(const Point2f &sample) { return sample; }

float Warp::squareToUniformSquarePdf(const Point2f &sample) {
  return ((sample.array() >= 0).all() && (sample.array() <= 1).all()) ? 1.0f
                                                                      : 0.0f;
}

Vector3f Warp::squareToUniformSphere(const Point2f &sample) {
  float z = 1.f - 2.f * sample.x();
  float r = std::sqrt(std::max(0.f, 1.f - z * z));
  float phi = 2.f * M_PI * sample.y();
  return Vector3f(r * std::cos(phi), r * std::sin(phi), z);
}

float Warp::squareToUniformSpherePdf(const Vector3f &v) { return INV_FOURPI; }

Point2f Warp::squareToUniformDisk(const Point2f &sample) {
  throw(RTException("Warp::squareToUniformDisk not implemented yet"));
  return Point2f::Zero();
}

float Warp::squareToUniformDiskPdf(const Point2f &p) {
  throw(RTException("Warp::squareToUniformDiskPdf not implemented yet"));
  return 0.f;
}

Vector3f Warp::squareToUniformHemisphere(const Point2f &sample) {
  throw(RTException("Warp::squareToUniformHemisphere not implemented yet"));
  return Vector3f::Zero();
}

float Warp::squareToUniformHemispherePdf(const Vector3f &v) {
  throw(RTException("Warp::squareToUniformHemispherePdf not implemented yet"));
  return 0.f;
}

Vector3f Warp::squareToCosineHemisphere(const Point2f &sample) {
  throw(RTException("Warp::squareToCosineHemisphere not implemented yet"));
  return Vector3f::Zero();
}

float Warp::squareToCosineHemispherePdf(const Vector3f &v) {
  throw(RTException("Warp::squareToCosineHemispherePdf not implemented yet"));
  return 0.f;
}

Point2f Warp::squareToUniformTriangle(const Point2f &sample) {
  throw(RTException("Warp::squareToUniformTriangle not implemented yet"));
  return Point2f::Zero();
}

float Warp::squareToUniformTrianglePdf(const Point2f &p) {
  throw(RTException("Warp::squareToUniformTrianglePdf not implemented yet"));
  return 0.f;
}

Vector3f Warp::squareToBeckmann(const Point2f &sample, float alpha) {
  throw(RTException("Warp::squareToBeckmann not implemented yet"));
  return Vector3f::Zero();
}

float Warp::squareToBeckmannPdf(const Vector3f &m, float alpha) {
  throw(RTException("Warp::squareToBeckmannPdf not implemented yet"));
  return 0.f;
}
