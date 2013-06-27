/* OpenSceneGraph example, osgshaderterrain.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osg/AlphaFunc>
#include <osg/Billboard>
#include <osg/BlendFunc>
#include <osg/Depth>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/GL2Extensions>
#include <osg/Material>
#include <osg/Math>
#include <osg/MatrixTransform>
#include <osg/PolygonOffset>
#include <osg/Program>
#include <osg/Projection>
#include <osg/Shader>
#include <osg/ShapeDrawable>
#include <osg/StateSet>
#include <osg/Switch>
#include <osg/Texture2D>
#include <osg/Uniform>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include <osgUtil/SmoothingVisitor>

#include <osgText/Text>

#include <osgViewer/Viewer>

#include <iostream>

// for the grid data..
#include "../osghangglide/terrain_coords.h"

osg::Node* createScene()
{
    osg::Group* scene = new osg::Group;

    unsigned int numColumns = 38;
    unsigned int numRows = 39;
    unsigned int r;
    unsigned int c;

    osg::Vec3 origin(0.0f,0.0f,0.0f);
    osg::Vec3 size(1000.0f,1000.0f,250.0f);
    osg::Vec3 scaleDown(1.0f/size.x(),1.0f/size.y(),1.0f/size.z());

    // ---------------------------------------
    // Set up a StateSet to texture the objects
    // ---------------------------------------
    osg::StateSet* stateset = new osg::StateSet();


    osg::Uniform* originUniform = new osg::Uniform("terrainOrigin",origin);
    stateset->addUniform(originUniform);

    osg::Uniform* sizeUniform = new osg::Uniform("terrainSize",size);
    stateset->addUniform(sizeUniform);

    osg::Uniform* scaleDownUniform = new osg::Uniform("terrainScaleDown",scaleDown);
    stateset->addUniform(scaleDownUniform);

    osg::Uniform* terrainTextureSampler = new osg::Uniform("terrainTexture",0);
    stateset->addUniform(terrainTextureSampler);

    osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",1);
    stateset->addUniform(baseTextureSampler);

    osg::Uniform* treeTextureSampler = new osg::Uniform("treeTexture",1);
    stateset->addUniform(treeTextureSampler);


    // compute z range of z values of grid data so we can scale it.
    float min_z = FLT_MAX;
    float max_z = -FLT_MAX;
    for(r=0;r<numRows;++r)
    {
        for(c=0;c<numColumns;++c)
        {
            min_z = osg::minimum(min_z,vertex[r+c*numRows][2]);
            max_z = osg::maximum(max_z,vertex[r+c*numRows][2]);
        }
    }

    float scale_z = size.z()/(max_z-min_z);

    osg::Image* terrainImage = new osg::Image;
    terrainImage->allocateImage(numColumns,numRows,1,GL_LUMINANCE, GL_FLOAT);
    terrainImage->setInternalTextureFormat(GL_LUMINANCE_FLOAT32_ATI);
    for(r=0;r<numRows;++r)
    {
        for(c=0;c<numColumns;++c)
        {
            *((float*)(terrainImage->data(c,r))) = (vertex[r+c*numRows][2]-min_z)*scale_z;
        }
    }

    osg::Texture2D* terrainTexture = new osg::Texture2D;
    terrainTexture->setImage(terrainImage);
    terrainTexture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::NEAREST);
    terrainTexture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
    terrainTexture->setResizeNonPowerOfTwoHint(false);
    stateset->setTextureAttributeAndModes(0,terrainTexture,osg::StateAttribute::ON);


    osg::Image* image = osgDB::readImageFile("Images/lz.rgb");
    if (image)
    {
        osg::Texture2D* texture = new osg::Texture2D;

        texture->setImage(image);
        stateset->setTextureAttributeAndModes(1,texture,osg::StateAttribute::ON);
    }

    {
        std::cout<<"Creating terrain...";

        osg::Geode* geode = new osg::Geode();
        geode->setStateSet( stateset );


        {
            osg::Program* program = new osg::Program;
            stateset->setAttribute(program);

#if 1
            // use inline shaders

            ///////////////////////////////////////////////////////////////////
            // vertex shader using just Vec4 coefficients
            char vertexShaderSource[] =
               "uniform sampler2D terrainTexture;\n"
               "uniform vec3 terrainOrigin;\n"
               "uniform vec3 terrainScaleDown;\n"
               "\n"
               "varying vec2 texcoord;\n"
               "\n"
               "void main(void)\n"
               "{\n"
               "    texcoord = gl_Vertex.xy - terrainOrigin.xy;\n"
               "    texcoord.x *= terrainScaleDown.x;\n"
               "    texcoord.y *= terrainScaleDown.y;\n"
               "\n"
               "    vec4 position;\n"
               "    position.x = gl_Vertex.x;\n"
               "    position.y = gl_Vertex.y;\n"
               "    position.z = texture2D(terrainTexture, texcoord).r;\n"
               "    position.w = 1.0;\n"
               " \n"
               "    gl_Position     = gl_ModelViewProjectionMatrix * position;\n"
                "   gl_FrontColor = vec4(1.0,1.0,1.0,1.0);\n"
               "}\n";

            //////////////////////////////////////////////////////////////////
            // fragment shader
            //
            char fragmentShaderSource[] =
                "uniform sampler2D baseTexture; \n"
                "varying vec2 texcoord;\n"
                "\n"
                "void main(void) \n"
                "{\n"
                "    gl_FragColor = texture2D( baseTexture, texcoord); \n"
                "}\n";

            program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShaderSource));
            program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource));

#else

            // get shaders from source
            program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("shaders/terrain.vert")));
            program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("shaders/terrain.frag")));

#endif

            // get shaders from source
        }


        {
            osg::Geometry* geometry = new osg::Geometry;

            osg::Vec3Array& v = *(new osg::Vec3Array(numColumns*numRows));
            osg::Vec4ubArray& color = *(new osg::Vec4ubArray(1));

            color[0].set(255,255,255,255);

            float rowCoordDelta = size.y()/(float)(numRows-1);
            float columnCoordDelta = size.x()/(float)(numColumns-1);

            float rowTexDelta = 1.0f/(float)(numRows-1);
            float columnTexDelta = 1.0f/(float)(numColumns-1);

            osg::Vec3 pos = origin;
            osg::Vec2 tex(0.0f,0.0f);
            int vi=0;
            for(r=0;r<numRows;++r)
            {
                pos.x() = origin.x();
                tex.x() = 0.0f;
                for(c=0;c<numColumns;++c)
                {
                    v[vi].set(pos.x(),pos.y(),pos.z());
                    pos.x()+=columnCoordDelta;
                    tex.x()+=columnTexDelta;
                    ++vi;
                }
                pos.y() += rowCoordDelta;
                tex.y() += rowTexDelta;
            }

            geometry->setVertexArray(&v);
            geometry->setColorArray(&color, osg::Array::BIND_OVERALL);

            for(r=0;r<numRows-1;++r)
            {
                osg::DrawElementsUShort& drawElements = *(new osg::DrawElementsUShort(GL_QUAD_STRIP,2*numColumns));
                geometry->addPrimitiveSet(&drawElements);
                int ei=0;
                for(c=0;c<numColumns;++c)
                {
                    drawElements[ei++] = (r+1)*numColumns+c;
                    drawElements[ei++] = (r)*numColumns+c;
                }
            }

            geometry->setInitialBound(osg::BoundingBox(origin, origin+size));

            geode->addDrawable(geometry);

            scene->addChild(geode);
        }
    }

    std::cout<<"done."<<std::endl;

    return scene;
}

class TestSupportOperation: public osg::GraphicsOperation
{
public:

    TestSupportOperation():
        osg::GraphicsOperation("TestSupportOperation",false),
        _supported(true),
        _errorMessage() {}

    virtual void operator () (osg::GraphicsContext* gc)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);

        unsigned int contextID = gc->getState()->getContextID();
        osg::GL2Extensions* gl2ext = osg::GL2Extensions::Get(contextID,true);
        if( gl2ext )
        {
            if( !gl2ext->isGlslSupported() )
            {
                _supported = false;
                _errorMessage = "ERROR: GLSL not supported by OpenGL driver.";
            }

            GLint numVertexTexUnits = 0;
            glGetIntegerv( GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, &numVertexTexUnits );
            if( numVertexTexUnits <= 0 )
            {
                _supported = false;
                _errorMessage = "ERROR: vertex texturing not supported by OpenGL driver.";
            }
        }
        else
        {
            _supported = false;
            _errorMessage = "ERROR: GLSL not supported.";
        }
    }

    OpenThreads::Mutex  _mutex;
    bool                _supported;
    std::string         _errorMessage;
};

int main(int, char **)
{
    // construct the viewer.
    osgViewer::Viewer viewer;

    osg::Node* node = createScene();

    // add model to viewer.
    viewer.setSceneData( node );

    viewer.setUpViewAcrossAllScreens();

    osg::ref_ptr<TestSupportOperation> testSupportOperation = new TestSupportOperation;
#if 0
    // temporily commenting out as its causing the viewer to crash... no clue yet to why
    viewer.setRealizeOperation(testSupportOperation.get());
#endif
    // create the windows and run the threads.
    viewer.realize();

    if (!testSupportOperation->_supported)
    {
        osg::notify(osg::WARN)<<testSupportOperation->_errorMessage<<std::endl;

        return 1;
    }

    return viewer.run();
}
