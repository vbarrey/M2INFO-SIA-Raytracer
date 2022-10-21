#include "shape.h"
#include "lights/areaLight.h"

void Shape::activate() {
  if (!m_bsdf) {
    /* If no material was assigned, instantiate a diffuse BRDF */
    m_bsdf = static_cast<BSDF *>(
        ObjectFactory::createInstance("diffuse", PropertyList()));
  }
}

void Shape::setParent(Object *parent)
{
    if(parent->getClassType() == ELight) {
        m_light = dynamic_cast<AreaLight *>(parent);
    }
}

void Shape::addChild(Object *obj) {
    switch (obj->getClassType()) {
        case EBSDF:
            if (m_bsdf)
                throw RTException(
                    "Shape: tried to register multiple BSDF instances!");
            m_bsdf = static_cast<BSDF *>(obj);
            break;
        default:
            throw RTException("Shape::addChild(<%s>) is not supported!",
                                classTypeName(obj->getClassType()));
    }
}
