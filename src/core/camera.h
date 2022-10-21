/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#pragma once

#include <object.h>
#include <rfilter.h>
#include <nanogui/vector.h>

/**
 * \brief Generic camera interface
 * 
 * This class provides an abstract interface to cameras in Nori and
 * exposes the ability to sample their response function. By default, only
 * a perspective camera implementation exists, but you may choose to
 * implement other types (e.g. an environment camera, or a physically-based 
 * camera model that simulates the behavior actual lenses)
 */
class Camera : public Object {
public:
    /**
     * \brief Importance sample a ray according to the camera's response function
     *
     * \param ray
     *    A ray data structure to be filled with a position 
     *    and direction value
     *
     * \param samplePosition
     *    Denotes the desired sample position on the film
     *    expressed in fractional pixel coordinates
     *
     */
    virtual void sampleRay(Ray &ray, const Point2f &samplePosition) const = 0;

    /// Return the size of the output image in pixels
    const Vector2i &getOutputSize() const { return m_outputSize; }

    /// Return the camera's reconstruction filter in image space
    const ReconstructionFilter *getReconstructionFilter() const { return m_rfilter; }

    /**
     * \brief Return the type of object (i.e. Mesh/Camera/etc.) 
     * provided by this instance
     * */
    EClassType getClassType() const { return ECamera; }
protected:
    Vector2i m_outputSize;
    ReconstructionFilter *m_rfilter;
};
