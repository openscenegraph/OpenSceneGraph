/*
 * 3D Heat map using vertex displacement mapping
 * Rendered using depth peeling in fragment shader.
 */

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Vec3>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/LightModel>
#include <osg/io_utils>
#include <osg/Material>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>
#include <osg/Math>
#include <iostream>

#define _USE_MATH_DEFINES
#include <math.h>

#include <osg/TexEnv>
#include <osg/TexMat>
#include <osg/Depth>
#include <osg/ShapeDrawable>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/PolygonMode>
#include <osg/PolygonOffset>

#include "HeatMap.h"
#include "DepthPeeling.h"

///////////////////////////////////////////////////////////////////////////
// in-line GLSL source code

static const char *VertexShader = {
    "#version 120\n"
    "uniform float maximum;\n"
    "uniform float maxheight;\n"
    "uniform float transparency;\n"
    "uniform sampler1D colortex;\n"
    "uniform sampler2D datatex;\n"
    "in vec2  xypos;\n"
    "void main(void)\n"
    "{\n"
    "    float foo;\n"
    "    float tmp     = min(texture2D(datatex, xypos).x / maximum, 1.0);\n"
    "    gl_Position   = gl_ModelViewProjectionMatrix * (gl_Vertex + vec4(0.0, 0.0, maxheight * tmp, 0.0));\n"
    "    vec4 color    = texture1D(colortex, tmp);\n"
    "    color.w       = color.w * transparency;\n"
    "    gl_FrontColor = color;\n"
    "    gl_BackColor  = color;\n"
    "}\n"
};

static const char *FragmentShader =
{
    "#version 120\n"
    "bool depthpeeling();\n"
    "void main(void)\n"
    "{\n"
    "  if( depthpeeling() ) discard;\n"
    "  gl_FragColor = gl_Color;\n"
    "}\n"
};

/**
 * Overloaded Geometry class to return predefined bounds
 */
class MyGeometry : public osg::Geometry
{
public:
    MyGeometry(osg::BoundingBox bounds)
    {
        m_bounds = bounds;
        m_bsphere = osg::BoundingSphere(bounds);
    }

    // an attempt to return a reasonable bounding box. Still does not prevent clipping of the heat map.
    virtual const osg::BoundingBox& getBoundingBox() const {return m_bounds;}
    virtual osg::BoundingBox computeBound() const {return m_bounds;}
    virtual const osg::BoundingSphere& getBound() const {return m_bsphere;}

protected:
    osg::BoundingBox m_bounds;
    osg::BoundingSphere m_bsphere;
};

Heatmap::Heatmap(float width, float depth, float maxheight, unsigned int K, unsigned int N, float maximum, float transparency)
{
    m_K = K;
    m_N = N;
    const int O = 4;

    // create Geometry object to store all the vertices primitives.
    osg::Geometry *meshGeom  = new MyGeometry(osg::BoundingBox(osg::Vec3(-width/2, -depth/2, 0), osg::Vec3(width/2, depth/2, maxheight)));

    // we use a float attribute array storing texcoords
    osg::Vec2Array* xypositions = new osg::Vec2Array();
    xypositions->setName("xypos");

    // create vertex coordinates
    osg::Vec3Array* vertices = new osg::Vec3Array();
    osg::Vec3 off(-width/2, -depth/2, 0);
    for (unsigned int y=0; y < O*N; y++) {
        if (y % 2 == 0)
        {
            for (unsigned int x=0; x < O*K; x++) {
                vertices->push_back(osg::Vec3(width*x/(O*K-1), depth*y/(O*N-1), 0.0)+off);
                xypositions->push_back(osg::Vec2(((float)x+0.5f)/(O*K),((float)y+0.5f)/(O*N)));
            }
        }
        else
        {
            vertices->push_back(osg::Vec3(0, depth*y/(O*N-1), 0.0)+off);
            xypositions->push_back(osg::Vec2(0.5f/(O*K),((float)y+0.5f)/(O*N)));
            for (unsigned int x=0; x < O*K-1; x++) {
                vertices->push_back(osg::Vec3(width*(0.5+x)/(O*K-1), depth*y/(O*N-1), 0.0)+off);
                xypositions->push_back(osg::Vec2(((float)(x+0.5f)+0.5f)/(O*K),((float)y+0.5f)/(O*N)));
            }
            vertices->push_back(osg::Vec3(width, depth*y/(O*N-1), 0.0)+off);
            xypositions->push_back(osg::Vec2(1.0f-0.5f/(O*K),((float)y+0.5f)/(O*N)));
        }
    }
    xypositions->setBinding(osg::Array::BIND_PER_VERTEX);
    xypositions->setNormalize(false);

    meshGeom->setVertexAttribArray(6, xypositions);
    meshGeom->setVertexArray(vertices);

    // generate several tri strips to form a mesh
    GLuint *indices = new GLuint[4*O*K];
    for (unsigned int y=0; y < O*N-1; y++) {
        if (y % 2 == 0)
        {
            int base  = (y/2) * (O*K+O*K+1);
            int base2 = (y/2) * (O*K+O*K+1) + O*K;
            int i=0; for (unsigned int x=0; x < O*K; x++) { indices[i++] = base2+x; indices[i++] = base+x;}
            indices[i++] = base2+O*K;
            meshGeom->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_STRIP, i, indices));
        }
        else
        {
            int base = (y/2) * (O*K+O*K+1) + O*K;
            int base2 = (y/2) * (O*K+O*K+1) + O*K + O*K+1;
            int i=0; for (unsigned int x=0; x < O*K; x++) { indices[i++] = base+x; indices[i++] = base2+x;}
            indices[i++] = base+O*K;
            meshGeom->addPrimitiveSet(new osg::DrawElementsUInt(osg::PrimitiveSet::TRIANGLE_STRIP, i, indices));
        }
    }
    delete[] indices;

    // create vertex and fragment shader
    osg::Program* program = new osg::Program;
    program->setName( "mesh" );
    program->addBindAttribLocation("xypos", 6);
    program->addShader( new osg::Shader( osg::Shader::VERTEX, VertexShader ) );
    program->addShader( new osg::Shader( osg::Shader::FRAGMENT, DepthPeeling::PeelingShader ) );
    program->addShader( new osg::Shader( osg::Shader::FRAGMENT, FragmentShader ) );

    // create a 1D texture for color lookups
    colorimg = new osg::Image();
    colorimg->allocateImage(5, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE);
    unsigned char *data = colorimg->data();
    *data++ =   0; *data++ =   0; *data++ = 255; *data++ =   0;  // fully transparent blue
    *data++ =   0; *data++ = 255; *data++ = 255; *data++ = 255;  // turquoise
    *data++ =   0; *data++ = 255; *data++ =   0; *data++ = 255;  // green
    *data++ = 255; *data++ = 255; *data++ =   0; *data++ = 255;  // yellow
    *data++ = 255; *data++ =   0; *data++ =   0; *data++ = 255;  // red
    colortex = new osg::Texture1D(colorimg.get());
    colortex->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    colortex->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    colortex->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    colortex->setResizeNonPowerOfTwoHint(false);

    // create a 2D texture for data lookups
    m_img2 = new osg::Image();
    m_img2->allocateImage(K, N, 1, GL_LUMINANCE, GL_FLOAT);
    m_img2->setInternalTextureFormat(GL_RGB32F_ARB);
    m_data = (float*)m_img2.get()->data();
    m_tex2 = new osg::Texture2D(m_img2.get());
    m_tex2.get()->setResizeNonPowerOfTwoHint(false);
    m_tex2.get()->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
    m_tex2.get()->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
    m_tex2.get()->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    m_tex2.get()->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

    // set render states
    osg::StateSet *meshstate = meshGeom->getOrCreateStateSet();
    meshstate->setMode(GL_BLEND,  osg::StateAttribute::ON);
    meshstate->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    meshstate->setAttributeAndModes(program, osg::StateAttribute::ON);
    meshstate->setTextureAttributeAndModes(0,colortex.get(),osg::StateAttribute::ON);
    meshstate->setTextureAttributeAndModes(1,m_tex2.get(),osg::StateAttribute::ON);

    // uniforms for height and color scaling
    maximumUniform = new osg::Uniform( "maximum", (float)maximum );
    maxheightUniform = new osg::Uniform( "maxheight", (float)maxheight );
    transparencyUniform = new osg::Uniform( "transparency", (float)transparency);

    osg::Uniform* texUniform = new osg::Uniform(osg::Uniform::SAMPLER_1D, "colortex");
    texUniform->set(0);
    osg::Uniform* texUniform2 = new osg::Uniform(osg::Uniform::SAMPLER_2D, "datatex");
    texUniform2->set(1);
    meshstate->addUniform(texUniform);
    meshstate->addUniform(texUniform2);
    meshstate->addUniform(maximumUniform);
    meshstate->addUniform(maxheightUniform);
    meshstate->addUniform(transparencyUniform);

    // add the geometries to the geode.
    meshGeom->setUseDisplayList(false);
    addDrawable(meshGeom);
}

void Heatmap::setData(float *buffer, float maxheight, float maximum, float transparency)
{
    memcpy(m_data, buffer, m_N*m_K*sizeof(float));

    maximumUniform->set( maximum );
    maxheightUniform->set( maxheight );
    transparencyUniform->set ( transparency );

    m_img2.get()->dirty();
}

Heatmap::~Heatmap()
{
}
