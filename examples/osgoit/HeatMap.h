
#ifndef HEATMAP_H
#define HEATMAP_H

#include <osg/Geode>
#include <osg/Uniform>
#include <osg/Texture2D>
#include <osg/Texture1D>

class Heatmap : public osg::Geode
{
public:
    Heatmap(float width, float depth, float maxheight, unsigned int K, unsigned int N, float maximum, float transparency);
    ~Heatmap();

    void setData(float *buffer, float maxheight, float maximum, float transparency);

protected:
    unsigned int m_K;
    unsigned int m_N;
    float *m_data;
    osg::ref_ptr<osg::Image> m_img2;
    osg::ref_ptr<osg::Texture2D> m_tex2;

    osg::ref_ptr<osg::Image> colorimg;
    osg::ref_ptr<osg::Texture1D> colortex;

    osg::Uniform *maximumUniform;
    osg::Uniform *maxheightUniform;
    osg::Uniform *transparencyUniform;
};

#endif // #ifndef HEATMAP_H
