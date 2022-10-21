/*
    This file is part of Nori, a simple educational ray tracer

    Copyright (c) 2015 by Wenzel Jakob
*/

#include <scene.h>
#include <bsdf.h>
#include <camera.h>
#include <integrator.h>
#include <sampler.h>
#include <hypothesis.h>
#include <pcg32.h>

/*
 * =======================================================================
 *   WARNING    WARNING    WARNING    WARNING    WARNING    WARNING
 * =======================================================================
 *   Remember to put on SAFETY GOGGLES before looking at this file. You
 *   are most certainly not expected to read or understand any of it.
 * =======================================================================
 */


/**
 * Student's t-test for the equality of means
 *
 * This test analyzes whether the expected value of a random variable matches a
 * certain known value. When there is significant statistical "evidence"
 * against this hypothesis, the test fails.
 *
 * This is useful in checking whether a Monte Carlo method method converges
 * against the right value. Because statistical tests are able to handle the
 * inherent noise of these methods, they can be used to construct statistical
 * test suites not unlike the traditional unit tests used in software engineering.
 *
 * This implementation can be used to test two things:
 *
 * 1. that the illumination scattered by a BRDF model under uniform illumination
 *    into a certain direction matches a given value (modulo noise).
 *
 * 2. that the average radiance received by a camera within some scene
 *    matches a given value (modulo noise).
 */
class StudentsTTest : public Object {
public:
    StudentsTTest(const PropertyList &propList) {
        /* The null hypothesis will be rejected when the associated
           p-value is below the significance level specified here. */
        m_significanceLevel = propList.getFloat("significanceLevel", 0.01f);

        /* This parameter specifies a list of incidence angles that will be tested */
        std::vector<std::string> angles = tokenize(propList.getString("angles", ""));
        for (auto angle : angles)
            m_angles.push_back(toFloat(angle));

        /* This parameter specifies a list of reference values, one for each angle */
        std::vector<std::string> references = tokenize(propList.getString("references", ""));
        for (auto angle : references)
            m_references.push_back(toFloat(angle));

        /* Number of BSDF samples that should be generated (default: 100K) */
        m_sampleCount = propList.getInteger("sampleCount", 100000);
    }

    virtual ~StudentsTTest() {
        for (auto bsdf : m_bsdfs)
            delete bsdf;
        for (auto scene : m_scenes)
            delete scene;
    }

    void addChild(Object *obj) {
        switch (obj->getClassType()) {
            case EBSDF:
                m_bsdfs.push_back(static_cast<BSDF *>(obj));
                break;

            case EScene:
                m_scenes.push_back(static_cast<Scene *>(obj));
                break;

            default:
                throw RTException("StudentsTTest::addChild(<%s>) is not supported!",
                    classTypeName(obj->getClassType()));
        }
    }

    /// Invoke a series of t-tests on the provided input
    void activate() {
        int total = 0, passed = 0;
        pcg32 random;

        if (!m_bsdfs.empty()) {
            if (m_references.size() * m_bsdfs.size() != m_angles.size())
                throw RTException("Specified a different number of angles and reference values!");
            if (!m_scenes.empty())
                throw RTException("Cannot test BSDFs and scenes at the same time!");

            /* Test each registered BSDF */
            int ctr = 0;
            for (auto bsdf : m_bsdfs) {
                for (size_t i=0; i<m_references.size(); ++i) {
                    float angle = m_angles[i], reference = m_references[ctr++];

                    cout << "------------------------------------------------------" << endl;
                    cout << "Testing (angle=" << angle << "): " << bsdf->toString() << endl;
                    ++total;

                    BSDFQueryRecord bRec(sphericalDirection(degToRad(angle), 0));

                    cout << "Drawing " << m_sampleCount << " samples .. " << endl;
                    double mean=0, variance = 0;
                    for (int k=0; k<m_sampleCount; ++k) {
                        Point2f sample(random.nextFloat(), random.nextFloat());
                        double result = (double) bsdf->sample(bRec, sample).getLuminance();

                        /* Numerically robust online variance estimation using an
                           algorithm proposed by Donald Knuth (TAOCP vol.2, 3rd ed., p.232) */
                        double delta = result - mean;
                        mean += delta / (double) (k+1);
                        variance += delta * (result - mean);
                    }
                    variance /= m_sampleCount - 1;
                    std::pair<bool, std::string>
                        result = hypothesis::students_t_test(mean, variance, reference,
                            m_sampleCount, m_significanceLevel, (int) m_references.size());

                    if (result.first)
                        ++passed;
                    cout << result.second << endl;
                }
            }
        } else {
            if (m_references.size() != m_scenes.size())
                throw RTException("Specified a different number of scenes and reference values!");

            Sampler *sampler = static_cast<Sampler *>(
                ObjectFactory::createInstance("independent", PropertyList()));

            int ctr = 0;
            for (auto scene : m_scenes) {
                const Integrator *integrator = scene->integrator();
                const Camera *camera = scene->camera();
                float reference = m_references[ctr++];

                cout << "------------------------------------------------------" << endl;
                cout << "Testing scene: " << scene->toString() << endl;
                ++total;

                cout << "Generating " << m_sampleCount << " paths.. " << endl;

                double mean = 0, variance = 0;
                for (int k=0; k<m_sampleCount; ++k) {
                    /* Sample a ray from the camera */
                    Ray ray;
                    Point2f pixelSample = (sampler->next2D().array()
                        * camera->getOutputSize().cast<float>().array()).matrix();                    
                    camera->sampleRay(ray, pixelSample);

                    /* Compute the incident radiance */
                    Color3f value = integrator->Li(scene, sampler, ray);

                    /* Numerically robust online variance estimation using an
                       algorithm proposed by Donald Knuth (TAOCP vol.2, 3rd ed., p.232) */
                    double result = (double) value.getLuminance();
                    double delta = result - mean;
                    mean += delta / (double) (k+1);
                    variance += delta * (result - mean);
                }
                variance /= m_sampleCount - 1;

                std::pair<bool, std::string>
                    result = hypothesis::students_t_test(mean, variance, reference,
                        m_sampleCount, m_significanceLevel, (int) m_references.size());

                if (result.first)
                    ++passed;
                cout << result.second << endl;
            }
        }
        cout << "Passed " << passed << "/" << total << " tests." << endl;
    }

    std::string toString() const {
        return tfm::format(
            "StudentsTTest[\n"
            "  significanceLevel = %f,\n"
            "  sampleCount= %i\n"
            "]",
            m_significanceLevel,
            m_sampleCount
        );
    }

    EClassType getClassType() const { return ETest; }
private:
    std::vector<BSDF *> m_bsdfs;
    std::vector<Scene *> m_scenes;
    std::vector<float> m_angles;
    std::vector<float> m_references;
    float m_significanceLevel;
    int m_sampleCount;
};

REGISTER_CLASS(StudentsTTest, "ttest");