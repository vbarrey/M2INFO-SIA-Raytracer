/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include "block.h"
#include "sampler.h"
#include <pcg32.h>

/**
 * Stratified sampling
 */
class Stratified : public Sampler {
public:
  Stratified(const PropertyList &propList) {
    m_xPixelSamples = (size_t)propList.getInteger("xPixelSamples", 4);
    m_yPixelSamples = (size_t)propList.getInteger("yPixelSamples", 4);

    m_sampleCount = m_xPixelSamples * m_yPixelSamples;
    m_jitterSamples = (size_t)propList.getBoolean("jitter", true);

    /* Dimension, up to which which stratified samples are guaranteed to be
     * available. */
    m_maxDimension = propList.getInteger("maxDimension", 2);

    /* Allocate sample vectors */
    for (int i = 0; i < m_maxDimension; ++i) {
        m_samples1D.push_back(std::vector<float>(m_sampleCount));
        m_samples2D.push_back(std::vector<Point2f>(m_sampleCount));
    }
  }

  virtual ~Stratified() {}

  std::unique_ptr<Sampler> clone() const {
    std::unique_ptr<Stratified> cloned(new Stratified());
    cloned->m_sampleCount = m_sampleCount;
    cloned->m_xPixelSamples = m_xPixelSamples;
    cloned->m_yPixelSamples = m_yPixelSamples;
    cloned->m_jitterSamples = m_jitterSamples;
    cloned->m_maxDimension = m_maxDimension;
    cloned->m_random = m_random;
    for (int i = 0; i < m_maxDimension; ++i) {
      cloned->m_samples1D.push_back(std::vector<float>(m_sampleCount));
      cloned->m_samples2D.push_back(std::vector<Point2f>(m_sampleCount));
    }
    return std::move(cloned);
  }

  void prepare(const ImageBlock &block) {
    m_random.seed(block.getOffset().x(), block.getOffset().y());
  }

  void stratifiedSample1D(std::vector<float> &samp) {
    float invNSamples = 1.f / m_sampleCount;
    for (size_t i = 0; i < m_sampleCount; ++i) {
      float delta = m_jitterSamples ? m_random.nextFloat() : 0.5f;
      samp[i] = std::min((i + delta) * invNSamples, OneMinusEpsilon);
    }
  }

  void stratifiedSample2D(std::vector<Point2f> &samp) {
    float dx = 1.f / m_xPixelSamples, dy = 1.f / m_yPixelSamples;
    int i = 0;
    for (int y = 0; y < m_yPixelSamples; ++y)
      for (int x = 0; x < m_xPixelSamples; ++x) {
        float jx = m_jitterSamples ? m_random.nextFloat() : 0.5f;
        float jy = m_jitterSamples ? m_random.nextFloat() : 0.5f;
        samp[i].x() = std::min((x + jx) * dx, OneMinusEpsilon);
        samp[i].y() = std::min((y + jy) * dy, OneMinusEpsilon);
        ++i;
      }
  }

  template <typename Iterator> void shuffle(Iterator begin, Iterator end) {
    for (Iterator it = end - 1; it > begin; --it)
      std::iter_swap(it, begin + m_random.nextUInt((uint32_t)(it - begin + 1)));
  }

  void generate() {
    // Generate single stratified samples for the pixel
    for (size_t i = 0; i < m_samples1D.size(); ++i) {
      stratifiedSample1D(m_samples1D[i]);
      if (m_jitterSamples)
        shuffle(m_samples1D[i].begin(), m_samples1D[i].end());
    }
    for (size_t i = 0; i < m_samples2D.size(); ++i) {
      stratifiedSample2D(m_samples2D[i]);
      if (m_jitterSamples)
        shuffle(m_samples2D[i].begin(), m_samples2D[i].end());
    }

    m_sampleIndex = 0;
    m_dimension1D = m_dimension2D = 0;
  }

  void advance() {
    m_sampleIndex++;
    m_dimension1D = m_dimension2D = 0;
  }

  float next1D() {
    assert(m_sampleIndex < m_sampleCount);
    if (m_dimension1D < m_maxDimension) {
      return m_samples1D[m_dimension1D++][m_sampleIndex];
    } else {
      return m_random.nextFloat();
    }
  }

  Point2f next2D() {
    assert(m_sampleIndex < m_sampleCount);
    if (m_dimension2D < m_maxDimension) {
      return m_samples2D[m_dimension2D++][m_sampleIndex];
    } else {
      return Point2f(m_random.nextFloat(), m_random.nextFloat());
    }
  }

  std::string toString() const {
    return tfm::format("Stratified[\n"
                       "  sample count = %i,\n"
                       "  max. dimension = %i\n"
                       " ]",
                       m_sampleCount, m_maxDimension);
  }

protected:
  Stratified() {}

private:
  pcg32 m_random;
  int m_maxDimension;
  bool m_jitterSamples;
  std::vector<std::vector<float>> m_samples1D;
  std::vector<std::vector<Point2f>> m_samples2D;
  int m_dimension1D, m_dimension2D;
  int m_xPixelSamples, m_yPixelSamples;
};

REGISTER_CLASS(Stratified, "stratified");
