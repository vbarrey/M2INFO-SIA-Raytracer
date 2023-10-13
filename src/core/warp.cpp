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

    //transformation des coordonnées dans le carré unitaire d'un point sample dans l'espace disque unitaire (disque de rayon 1 centré en (0,0)).
    float r = sqrt(sample.x());
    float phi = 2.f * M_PI * sample.y();
    float x = r * cos(phi);
    float y = r * sin(phi);
    return Point2f(x, y);
}

float Warp::squareToUniformDiskPdf(const Point2f &p) {
  //Calcul de la probabilité qu'un point du carré soit contenu dans le disque après transformation, 
  // si le rayon > 1 (par rapport au centre du cercle en (0,0)) alors proba=0 sinon proba = 1/pi (constante, voir cours).
  float proba = (sqrt(p.x()*p.x()+p.y()*p.y()) > 1)? 0.f : 1/M_PI;
  return proba;
}

Vector3f Warp::squareToUniformHemisphere(const Point2f &sample) {
  float theta1 = 2.f * M_PI * sample.x();
  float theta2 = acos(sample.y());

  return Vector3f(sin(theta2)*cos(theta1), sin(theta1)*sin(theta2), cos(theta2));
}

float Warp::squareToUniformHemispherePdf(const Vector3f &v) {
  return (v.z() < 0)? 0.f : 1/(2*M_PI);
}

Vector3f Warp::squareToCosineHemisphere(const Point2f &sample) {
  float theta1 = 2.f * M_PI * sample.x();
  float theta2 = acos(sqrt(1-sample.y()));

  return Vector3f(sin(theta2)*cos(theta1), sin(theta1)*sin(theta2), cos(theta2));
}

float Warp::squareToCosineHemispherePdf(const Vector3f &v) {
  return (v.z() < 0)? 0.f : v.z()/(M_PI);
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
