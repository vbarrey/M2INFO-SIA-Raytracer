#pragma once

#include "common.h"
#include "vector.h"

class Trackball {
public:
    using Quaternionf = Eigen::Quaternion<float, Eigen::DontAlign>;

    Trackball(float speedFactor = 2.0f)
        : m_active(false), m_lastPos(Vector2i::Zero()), m_size(Vector2i::Zero()),
          m_quat(Quaternionf::Identity()),
          m_incr(Quaternionf::Identity()),
          m_speedFactor(speedFactor) { }

    void setSize(const Vector2i &size) { m_size = size; }

    const Vector2i &size() const { return m_size; }

    void button(const Vector2i &pos, bool pressed) {
        m_active = pressed;
        m_lastPos = pos;
        if (!m_active)
            m_quat = (m_incr * m_quat).normalized();
        m_incr = Quaternionf::Identity();
    }

    bool motion(Vector2i pos) {
        if (!m_active)
            return false;

        /* Based on the rotation controller from AntTweakBar */
        float invMinDim = 1.0f / m_size.minCoeff();
        float w = (float) m_size.x(), h = (float) m_size.y();

        float ox = (m_speedFactor * (2*m_lastPos.x() - w) + w) - w - 1.0f;
        float tx = (m_speedFactor * (2*pos.x()      - w) + w) - w - 1.0f;
        float oy = (m_speedFactor * (h - 2*m_lastPos.y()) + h) - h - 1.0f;
        float ty = (m_speedFactor * (h - 2*pos.y())      + h) - h - 1.0f;

        ox *= invMinDim; oy *= invMinDim;
        tx *= invMinDim; ty *= invMinDim;

        ::Vector3f v0(ox, oy, 1.0f), v1(tx, ty, 1.0f);
        if (v0.squaredNorm() > 1e-4f && v1.squaredNorm() > 1e-4f) {
            v0.normalize(); v1.normalize();
            ::Vector3f axis = v0.cross(v1);
            float sa = std::sqrt(axis.dot(axis)),
                  ca = v0.dot(v1),
                  angle = std::atan2(sa, ca);
            if (tx*tx + ty*ty > 1.0f)
                angle *= 1.0f + 0.2f * (std::sqrt(tx*tx + ty*ty) - 1.0f);
            m_incr = Eigen::AngleAxisf(angle, axis.normalized());
            if (!std::isfinite(m_incr.norm()))
                m_incr = Quaternionf::Identity();
        }
        return true;
    }

    Eigen::Matrix4f matrix() const {
        Eigen::Matrix4f result2 = Eigen::Matrix4f::Identity();
        result2.block<3,3>(0, 0) = (m_incr * m_quat).toRotationMatrix();
        return result2;
    }


private:
    /// Whether or not this Arcball is currently active.
    bool m_active;

    /// The last click position (which triggered the Arcball to be active / non-active).
    Vector2i m_lastPos;

    /// The size of this Arcball.
    Vector2i m_size;

    /**
     * The current stable state.  When this Arcball is active, represents the
     * state of this Arcball when \ref Arcball::button was called with
     * ``down = true``.
     */
    Quaternionf m_quat;

    /// When active, tracks the overall update to the state.  Identity when non-active.
    Quaternionf m_incr;

    /**
     * The speed at which this Arcball rotates.  Smaller values mean it rotates
     * more slowly, higher values mean it rotates more quickly.
     */
    float m_speedFactor;

public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};
