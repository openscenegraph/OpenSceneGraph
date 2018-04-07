//info : osgSSBO example,testing ShaderStorageBufferObjects ,Markus Hein,  2014, osg-3.2.1
//required hardware and driver must support GL >=  GL 4.3 or GL ES 3.1 (GL ES not tested, would be nice if someone will test it on a small device)

//testing osg support for Shader Storage Buffer Objects

//version: "first take" from last night session..



#include <osg/StateAttributeCallback>
#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/DispatchCompute>
#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osg/Node>
#include <osg/PositionAttitudeTransform>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/Stencil>
#include <osg/Depth>
#include <osg/Billboard>
#include <osg/Material>
#include <osg/AnimationPath>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgUtil/SmoothingVisitor>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgViewer/Renderer>


#include <osg/Array>
#include <osg/BoundingSphere>
#include <osg/BufferIndexBinding>
#include <osg/BufferObject>
#include <osg/Group>
#include <osg/Math>
#include <osg/MatrixTransform>
#include <osg/Program>
#include <osg/Shader>
#include <osg/Drawable>
#include <osg/CopyOp>
#include <osg/State>
#include <osg/Matrix>
#include <osg/ShapeDrawable>
#include <osg/GL>
#include <osg/StateSet>
#include <osg/Texture2D>
#include <osg/BlendFunc>
#include <osg/TexEnv>
#include <osg/Material>
#include <osg/PointSprite>
#include <osg/Program>
#include <osg/Notify>
#include <osg/Point>
#include <osg/io_utils>
#include <osg/VertexProgram>

#include <osgText/Font>
#include <osgText/Text>


#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osgUtil/Optimizer>
#include <iostream>
#include <typeinfo>


using namespace osg;


//todo .. #define COMPUTATION_IN_SEPARATE_THREAD

#define WORK_GROUP_SIZE 16


#define PRERENDER_ANTIALIASINGMULTISAMPLES 16
#define PRERENDER_HIGH_QUALITY_ANTIALIASING
#define PRERENDER_WIDTH 1920
#define PRERENDER_HEIGHT 1080


#define SUB_PLACEMENT_OFFSET_HORIZONTAL  0.5
#define SUB_PLACEMENT_OFFSET_VERTICAL  0.5

enum BufferOffset
{
    POSITION_NOW_OFFSET,
    POSITION_OLD_OFFSET,
    POSITION_INIT_OFFSET,

    VELOCITY_NOW_OFFSET,
    VELOCITY_OLD_OFFSET,
    VELOCITY_INIT_OFFSET,

    ACCELERATION_OFFSET,
    PROPERTIES_OFFSET,

    OFFSET_END
};


const int __numDataValuesPerChannel = OFFSET_END;
const int __numChannels = 4;

//512x512x4x7 = 7.340.032 floats in SSBO on GPU
const int NUM_ELEMENTS_X = 512;
const int NUM_ELEMENTS_Y = 512;

float random(float min, float max) { return min + (max - min)*(float)rand() / (float)RAND_MAX; }


enum Channel
{
    RED_CHANNEL,
    GREEN_CHANNEL,
    BLUE_CHANNEL,
    ALPHA_CHANNEL,
    RGB_CHANNEL,
    RGBA_CHANNEL
};



class ShaderStorageBufferCallback : public osg::StateAttributeCallback
{
public:
    void operator() (osg::StateAttribute* /*attr*/, osg::NodeVisitor* /*nv*/)
    {
        //if you need to process the data in your app-code , better leaving it on GPU and processing there, uploading per frame will make it slow
#if 0
        osg::ShaderStorageBufferBinding* ssbb = static_cast<osg::ShaderStorageBufferBinding*>(attr);
        osg::ShaderStorageBufferObject* ssbo
            = static_cast<osg::ShaderStorageBufferObject*>(ssbb->getBufferObject());

        osg::FloatArray* array = static_cast<osg::FloatArray*>(ssbo->getBufferData(0));

        float someValue = array->at(0);
        //std::cout << "someValue now: " << someValue << std::endl;
        //data transfer performance test
        //    array->dirty();
#endif
    }
};


//do not forget to set OSG_FILE_PATH to default OSG-Data and make sure the new shaders are copied there under"shaders"
class ComputeNode : public osg::PositionAttitudeTransform
{

public:

    osg::ref_ptr<osg::DispatchCompute>                _DispatchCompute;
    osg::ref_ptr<osg::Program>                        _computeProgram;
    osg::ref_ptr<osg::Shader>                        _computeShader;        //compute and write position data in SSBO

    osg::ref_ptr<osg::Shader>                        _vertexShader;        //reading position data from SSBO (OBS!: make sure glMemoryBuffer() is syncing this)
    osg::ref_ptr<osg::Shader>                        _geometryShader;    //building a quad looking to the camera
    osg::ref_ptr<osg::Shader>                        _fragmentShader;    //use false-colors etc. for making your data visible

    osg::ref_ptr<osg::Node>                            _helperNode;        // coordinate system node

    ref_ptr<osg::ShaderStorageBufferObject>            _ssbo;
    ref_ptr<osg::ShaderStorageBufferBinding>        _ssbb;

    GLfloat*                                        _data;        // some data we upload to GPU, initialised with random values
    osg::ref_ptr<FloatArray>                        _dataArray; //

    osg::ref_ptr<osg::Group>                        _computationResultsRenderGroup;
    osg::ref_ptr<osg::Program>                        _computationResultsRenderProgram;
    osg::ref_ptr<osg::StateSet>                        _computationResultsRenderStateSet;


    std::string                                        _computeShaderSourcePath;
    std::string                                        _vertexShaderSourcePath;
    std::string                                        _geometryShaderSourcePath;
    std::string                                        _fragmentShaderSourcePath;


    void                addHelperGeometry();
    void                addDataMonitor(osg::Vec3 placement, osg::Vec3 relativePlacement, float scale, Channel channel, BufferOffset shaderBufferOffset, std::string labelcaption, float minDataRange, float maxDataRange);
    void                addComputationResultsRenderTree();
    void                initComputingSetup();

    ComputeNode()
    {
        _computeShaderSourcePath = "shaders/osgssboComputeShader.cs";
        _vertexShaderSourcePath = "shaders/osgssboVertexShader.vs";
        _geometryShaderSourcePath = "shaders/osgssboGeometryShader.gs";
        _fragmentShaderSourcePath = "shaders/osgssboFragmentShader.fs";
	_DispatchCompute=new osg::DispatchCompute();
        addChild(_DispatchCompute);
    }

};


class ComputeNodeUpdateCallback : public osg::NodeCallback
{
public:

    ComputeNode* _computeNode;
    osg::Timer_t _prevShaderUpdateTime;
    osg::Timer _timer;

    ComputeNodeUpdateCallback(){}

    ComputeNodeUpdateCallback(ComputeNode* computeNode)
    {
        _computeNode = computeNode;
        _prevShaderUpdateTime = 0;
    }

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        osg::Timer_t currTime = _timer.tick();

        if (_timer.delta_s(_prevShaderUpdateTime, currTime) > 1.0) //one  second interval for shader-changed-do-reload check
        {
            osg::ref_ptr<osg::Shader> reloadedshader;
            std::string runningSource;
            std::string reloadedstring;

            if (_computeNode->_computeShader.valid())
            {
                runningSource = _computeNode->_computeShader->getShaderSource();
                reloadedshader = osgDB::readRefShaderFile(osg::Shader::COMPUTE, _computeNode->_computeShaderSourcePath);

                reloadedstring = reloadedshader->getShaderSource();
                if (!osgDB::equalCaseInsensitive(runningSource.c_str(), reloadedstring.c_str()))
                {
                    _computeNode->_computeProgram->removeShader(_computeNode->_computeShader.get());
                    _computeNode->_computeShader = reloadedshader.get();
                    _computeNode->_computeProgram->addShader(_computeNode->_computeShader.get());
                }
            }

            if (_computeNode->_vertexShader.valid())
            {

                runningSource = _computeNode->_vertexShader->getShaderSource();
                reloadedshader = osgDB::readRefShaderFile(osg::Shader::VERTEX, _computeNode->_vertexShaderSourcePath);

                reloadedstring = reloadedshader->getShaderSource();
                if (!osgDB::equalCaseInsensitive(runningSource.c_str(), reloadedstring.c_str()))
                {
                    _computeNode->_computationResultsRenderProgram->removeShader(_computeNode->_vertexShader.get());
                    _computeNode->_vertexShader = reloadedshader.get();
                    _computeNode->_computationResultsRenderProgram->addShader(_computeNode->_vertexShader.get());
                }
            }



            if (_computeNode->_geometryShader.valid())
            {
                runningSource = _computeNode->_geometryShader->getShaderSource();
                reloadedshader = osgDB::readRefShaderFile(osg::Shader::GEOMETRY, _computeNode->_geometryShaderSourcePath);

                reloadedstring = reloadedshader->getShaderSource();
                if (!osgDB::equalCaseInsensitive(runningSource.c_str(), reloadedstring.c_str()))
                {
                    _computeNode->_computationResultsRenderProgram->removeShader(_computeNode->_geometryShader.get());
                    _computeNode->_geometryShader = reloadedshader.get();
                    _computeNode->_computationResultsRenderProgram->addShader(_computeNode->_geometryShader.get());
                }
            }

            if (_computeNode->_fragmentShader.valid())
            {
                runningSource = _computeNode->_fragmentShader->getShaderSource();
                reloadedshader = osgDB::readRefShaderFile(osg::Shader::FRAGMENT, _computeNode->_fragmentShaderSourcePath);

                reloadedstring = reloadedshader->getShaderSource();
                if (!osgDB::equalCaseInsensitive(runningSource.c_str(), reloadedstring.c_str()))
                {
                    _computeNode->_computationResultsRenderProgram->removeShader(_computeNode->_fragmentShader.get());
                    _computeNode->_fragmentShader = reloadedshader.get();
                    _computeNode->_computationResultsRenderProgram->addShader(_computeNode->_fragmentShader.get());
                }
            }


            _prevShaderUpdateTime = _timer.tick();
        }

        traverse(node, nv);

    }
};

//set  OSG_FILE_PATH for loading axes.osgt
void ComputeNode::addHelperGeometry()
{
    _helperNode = osgDB::readRefNodeFile("axes.osgt");

    if (_helperNode.valid())
    {
        addChild(_helperNode.get());
    }

    //osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
    //pat->setPosition(osg::Vec3(0.5, 0, 0.5));
    //osg::Geode *sphereGeode = new osg::Geode;
    //float radius = 0.5f;
    //osg::TessellationHints* hints = new osg::TessellationHints;
    //hints->setDetailRatio(0.9f);
    //osg::ShapeDrawable* sphere = new osg::ShapeDrawable(new osg::Sphere(osg::Vec3(0.0f, 0.0f, 0.0f), radius), hints);
    //sphereGeode->addDrawable(sphere);
    //sphere->setColor(osg::Vec4(0, 1, 0, 0.1));
    //osg::StateSet* stateset = sphereGeode->getOrCreateStateSet();
    //osg::BlendFunc *blend = new osg::BlendFunc;
    //blend->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    //stateset->setAttributeAndModes(blend, osg::StateAttribute::ON);
    //pat->addChild(sphereGeode);
    //addChild(pat);
}






void ComputeNode::addDataMonitor(osg::Vec3 placement, osg::Vec3 relativePlacement, float scale, Channel colorchannel, BufferOffset shaderStorageBufferOffset, std::string labelCaption, float minDataRange, float maxDataRange)
{
    osg::PositionAttitudeTransform* pat = new osg::PositionAttitudeTransform;
    pat->setPosition(relativePlacement);
    addChild(pat);
    osg::Geometry* geom;

    if (NUM_ELEMENTS_X >= NUM_ELEMENTS_Y)
    {
        float ratio = (float)((float)NUM_ELEMENTS_Y / (float)NUM_ELEMENTS_X);
        geom = osg::createTexturedQuadGeometry(placement, osg::Vec3(1.0f*scale, 0.0f, 0.0f), osg::Vec3(0.0f, 0.0f, ratio*1.0f*scale));
    }
    else
    {
        float ratio = (float)((float)NUM_ELEMENTS_X / (float)NUM_ELEMENTS_Y);
        geom = osg::createTexturedQuadGeometry(placement, osg::Vec3(ratio*1.0f*scale, 0.0f, 0.0f), osg::Vec3(0.0f, 0.0f, 1.0f*scale));

    }

    geom->setVertexAttribArray(1, geom->getTexCoordArray(0), osg::Array::BIND_PER_VERTEX);

    osg::ref_ptr<osg::Geode> quad = new osg::Geode;
    quad->addDrawable(geom);
    quad->setStateSet(getOrCreateStateSet());
    pat->addChild(quad.get());

    static const char* vertexShaderSrcChannelMonitor = {

        "#version 430  \n"

        "uniform int numRows;\n"
        "uniform int numCols;\n"
        "uniform float osg_FrameTime;\n"
        "uniform mat4 osg_ProjectionMatrix;\n"
        "uniform mat4 osg_ModelViewMatrix;\n"
        "out vec2 texCoordFromVertexShader;\n"
        "struct particle{ float    x; float y; float z; float w;};"
        "layout (location = 0) in vec3 vertexpos;\n"
        "attribute vec2 tex_coords;\n"
        "void main() {\n"
        "texCoordFromVertexShader.xy = tex_coords.xy;  gl_Position = ( osg_ProjectionMatrix * osg_ModelViewMatrix * vec4(vertexpos.x,vertexpos.y,vertexpos.z,1) ); \n"
        "}\n"
    };




    std::stringstream fragmentshaderstringstreamChannelMonitor;
    fragmentshaderstringstreamChannelMonitor << "#version 430\n";
    fragmentshaderstringstreamChannelMonitor << "uniform int numRows;\n";
    fragmentshaderstringstreamChannelMonitor << "uniform int numCols;\n";
    fragmentshaderstringstreamChannelMonitor << "uniform float dataRangeMin;\n";
    fragmentshaderstringstreamChannelMonitor << "uniform float dataRangeMax;\n";
    fragmentshaderstringstreamChannelMonitor << "in vec2 texCoordFromVertexShader;\n";
    fragmentshaderstringstreamChannelMonitor << "struct particle{ float    x; float y; float z; float w;};";
    fragmentshaderstringstreamChannelMonitor << "layout(std140, binding=0) coherent buffer particles{particle p[];}; ";
    fragmentshaderstringstreamChannelMonitor << "\n";
    fragmentshaderstringstreamChannelMonitor << "void main(void)\n";
    fragmentshaderstringstreamChannelMonitor << "{\n";
    fragmentshaderstringstreamChannelMonitor << "ivec2 storePos = ivec2(numRows*texCoordFromVertexShader.x, numCols*texCoordFromVertexShader.y);  particle particleData = p[" << shaderStorageBufferOffset * NUM_ELEMENTS_X*NUM_ELEMENTS_Y << " + (storePos.x*numRows + storePos.y)]; ";

    //fragmentshaderstringstreamChannelMonitor << " memoryBarrierBuffer(); \n";
    fragmentshaderstringstreamChannelMonitor << " float dataRangeMultiplier = 1.0 / abs(dataRangeMax - dataRangeMin); \n";

    switch (colorchannel)
    {
    case RED_CHANNEL:
    {
        fragmentshaderstringstreamChannelMonitor << "   vec4 color; color.x =  0.5+dataRangeMultiplier*particleData.x; color.y =0.0; color.z = 0.0; color.w = 1.0; gl_FragColor = color;\n";

        break;
    }
    case GREEN_CHANNEL:
    {
        fragmentshaderstringstreamChannelMonitor << "   vec4 color; color.x = 0.0; color.y = 0.5+dataRangeMultiplier*particleData.y; color.z = 0.0; color.w = 1.0; gl_FragColor = color;\n";
        break;
    }
    case BLUE_CHANNEL:
    {
        fragmentshaderstringstreamChannelMonitor << "   vec4 color; color.x = 0.0; color.y = 0.0; color.z = 0.5+dataRangeMultiplier*particleData.z;  color.w = 0.0 ; gl_FragColor = color;\n";
        break;
    }
    case ALPHA_CHANNEL:
    {
        fragmentshaderstringstreamChannelMonitor << "   vec4 color; color.x = 0.5+dataRangeMultiplier*particleData.w; color.y = 0.5+dataRangeMultiplier*particleData.w; color.z = 0.5+dataRangeMultiplier*particleData.w; color.w = 0.5+0.5*particleData.w; gl_FragColor = color;\n";
        break;
    }

    case RGB_CHANNEL:
    {
        fragmentshaderstringstreamChannelMonitor << "   vec4 color; color.x = 0.5+dataRangeMultiplier*particleData.x; color.y = 0.5+dataRangeMultiplier*particleData.y; color.z = 0.5+dataRangeMultiplier*particleData.z; color.w = 1.0; gl_FragColor = color;\n";
        break;
    }

    case RGBA_CHANNEL:
    {
        fragmentshaderstringstreamChannelMonitor << "   vec4 color; color.x = 0.5+dataRangeMultiplier*particleData.x; color.y = 0.5+dataRangeMultiplier*particleData.y; color.z = 0.5+dataRangeMultiplier*particleData.z; color.w = 0.5+0.5*particleData.w; gl_FragColor = color;\n";
        break;
    }

    }

    fragmentshaderstringstreamChannelMonitor << "}\n";




    osg::Program * program = new osg::Program;
    program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShaderSrcChannelMonitor));
    program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentshaderstringstreamChannelMonitor.str().c_str()));
    program->addBindAttribLocation("tex_coords", 1);

    osg::StateSet* ss = geom->getOrCreateStateSet();
    ss->setAttributeAndModes(program, osg::StateAttribute::ON);
    ss->addUniform(new osg::Uniform("numRows", (int)NUM_ELEMENTS_X));
    ss->addUniform(new osg::Uniform("numCols", (int)NUM_ELEMENTS_Y));

    ss->addUniform(new osg::Uniform("dataRangeMin", (float)minDataRange));
    ss->addUniform(new osg::Uniform("dataRangeMax", (float)maxDataRange));


    ss->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    //add a label
    osg::ref_ptr<osgText::Text> text = new osgText::Text;
    osg::ref_ptr<osgText::Font> font = osgText::readRefFontFile("fonts/arial.ttf");
    text->setFont(font);
    text->setColor(osg::Vec4(1, 1, 1, 1));
    text->setCharacterSize(0.1*scale);
    text->setPosition(placement + osg::Vec3(0.05, 0.05, 0));
    pat->setName(labelCaption);
    text->setText(pat->getName());
    text->setBackdropType(osgText::Text::OUTLINE);
    text->setBackdropOffset(0.05f);
    text->setBackdropColor(osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));

    quad->addDrawable(text);

    pat->addChild(quad.get());

}

//compute texture image , taken from osgspotlight
osg::Image* createSpotLightImage(const osg::Vec4& centerColour, const osg::Vec4& backgroudColour, unsigned int size, float power)
{
    osg::Image* image = new osg::Image;
    image->allocateImage(size, size, 1,
        GL_RGBA, GL_UNSIGNED_BYTE);


    float mid = (float(size) - 1)*0.5f;
    float div = 2.0f / float(size);
    for (unsigned int r = 0; r < size; ++r)
    {
        unsigned char* ptr = image->data(0, r, 0);
        for (unsigned int c = 0; c < size; ++c)
        {
            float dx = (float(c) - mid)*div;
            float dy = (float(r) - mid)*div;
            float pr = powf(1.0f - sqrtf(dx*dx + dy*dy), power);
            if (pr < 0.0f) pr = 0.0f;
            osg::Vec4 color = centerColour*pr + backgroudColour*(1.0f - pr);
            *ptr++ = (unsigned char)((color[0])*255.0f);
            *ptr++ = (unsigned char)((color[1])*255.0f);
            *ptr++ = (unsigned char)((color[2])*255.0f);
            *ptr++ = (unsigned char)((color[3])*255.0f);
        }
    }
    return image;
}


void ComputeNode::addComputationResultsRenderTree()
{

    _computationResultsRenderProgram = new osg::Program;

    _vertexShader = osgDB::readRefShaderFile(osg::Shader::VERTEX, _vertexShaderSourcePath);
    _computationResultsRenderProgram->addShader(_vertexShader.get());

    _geometryShader = osgDB::readRefShaderFile(osg::Shader::GEOMETRY, _geometryShaderSourcePath);
    _computationResultsRenderProgram->addShader(_geometryShader.get());

    _fragmentShader = osgDB::readRefShaderFile(osg::Shader::FRAGMENT, _fragmentShaderSourcePath);
    _computationResultsRenderProgram->addShader(_fragmentShader.get());


    _computationResultsRenderProgram->addBindAttribLocation("tex_coords", 1);

    _computationResultsRenderGroup = new osg::Group;
    _computationResultsRenderGroup->setDataVariance(osg::Object::DYNAMIC);
    _computationResultsRenderStateSet = _computationResultsRenderGroup->getOrCreateStateSet();
    _computationResultsRenderStateSet->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    osg::PointSprite *sprite = new osg::PointSprite;
    int texture_unit = 0;
    _computationResultsRenderStateSet->setTextureAttributeAndModes(texture_unit, sprite, osg::StateAttribute::ON);
    _computationResultsRenderStateSet->setAttributeAndModes(_computationResultsRenderProgram.get(), osg::StateAttribute::ON);
    _computationResultsRenderStateSet->addUniform(new osg::Uniform("particleTexture", texture_unit));
    _computationResultsRenderStateSet->addUniform(new osg::Uniform("numRows", (int)NUM_ELEMENTS_X));
    _computationResultsRenderStateSet->addUniform(new osg::Uniform("numCols", (int)NUM_ELEMENTS_Y));


    _computationResultsRenderStateSet->setMode(GL_POINT_SMOOTH, osg::StateAttribute::ON);
    _computationResultsRenderStateSet->setMode(GL_VERTEX_PROGRAM_POINT_SIZE_ARB, osg::StateAttribute::ON);
    _computationResultsRenderStateSet->setMode(GL_ALPHA_TEST, osg::StateAttribute::ON);
    _computationResultsRenderStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    osg::Texture2D *tex = new osg::Texture2D();

    osg::Image* particleImage = createSpotLightImage(osg::Vec4(1, 0, 0, 1), osg::Vec4(0.5, 0, 0, 0.0), 32, 0.7);
    if (particleImage)
    {
        tex->setImage(particleImage);
    }
    _computationResultsRenderStateSet->setTextureAttributeAndModes(texture_unit, tex, osg::StateAttribute::ON);


    osg::BlendFunc *blend = new osg::BlendFunc;
    if (false) //emissive particles
    {
        blend->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE);
    }
    else
    {
        blend->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    }

    _computationResultsRenderStateSet->setAttributeAndModes(blend, osg::StateAttribute::ON);


    osg::Depth* depth = new osg::Depth;
    depth->setRange(0.0f, 0.0f);
    depth->setFunction(osg::Depth::ALWAYS);
    depth->setWriteMask(false);
    depth->setFunction(osg::Depth::ALWAYS);

    _computationResultsRenderStateSet->setAttributeAndModes(depth, osg::StateAttribute::OFF);


    osg::Geode* particleGeode = new osg::Geode;
    unsigned int numVertices = NUM_ELEMENTS_X*NUM_ELEMENTS_Y;

    osg::Geometry* particleGeometry = new osg::Geometry;
    particleGeometry->setUseDisplayList(false);
    particleGeometry->setUseVertexBufferObjects(true);

    osg::Vec3Array* vertexarray = new osg::Vec3Array;
    osg::Vec2Array* tcoords = new osg::Vec2Array;

    osg::Vec2 bottom_texcoord(0.0f, 0.0f);

    osg::Vec2 dx_texcoord(1.0f / (float)(NUM_ELEMENTS_X), 0.0f);
    osg::Vec2 dy_texcoord(0.0f, 1.0f / (float)(NUM_ELEMENTS_Y));



    for (int i = 0; i < NUM_ELEMENTS_X; i++)
    {
        osg::Vec2 texcoord = bottom_texcoord + dy_texcoord*(float)i;

        for (int j = 0; j < NUM_ELEMENTS_Y; j++)
        {
            vertexarray->push_back(osg::Vec3(texcoord.x(), texcoord.y(), 0.0));
            tcoords->push_back(osg::Vec2(texcoord.x(), texcoord.y()));
            texcoord += dx_texcoord;
        }
    }

    particleGeometry->setVertexArray(vertexarray);
    particleGeometry->addPrimitiveSet(new osg::DrawArrays(GL_POINTS, 0, numVertices));
    particleGeometry->setTexCoordArray(0, tcoords);
    //this glMemoryBarrier thing... not sure if we could better do instanced drawing?  all the data is in Shader Storage Buffer..
    particleGeometry->setVertexAttribArray(1, particleGeometry->getTexCoordArray(0), osg::Array::BIND_PER_VERTEX);

    _computationResultsRenderGroup->addChild(particleGeode);
    particleGeode->addDrawable(particleGeometry);

    addChild(_computationResultsRenderGroup.get());

}


void ComputeNode::initComputingSetup()
{

    _computeProgram = new osg::Program;
    _DispatchCompute->setComputeGroups((NUM_ELEMENTS_X / WORK_GROUP_SIZE) <= 1 ? 1 : (NUM_ELEMENTS_X / WORK_GROUP_SIZE), (NUM_ELEMENTS_Y / WORK_GROUP_SIZE) <= 1 ? 1 : (NUM_ELEMENTS_Y / WORK_GROUP_SIZE), 1);
    _computeShader = osgDB::readRefShaderFile(osg::Shader::COMPUTE, _computeShaderSourcePath);
    _computeProgram->addShader(_computeShader.get());

    setDataVariance(osg::Object::DYNAMIC);
    osg::StateSet* statesetComputation = getOrCreateStateSet();
    statesetComputation->setAttributeAndModes(_computeProgram.get());
    statesetComputation->addUniform(new osg::Uniform("numCols", (int)NUM_ELEMENTS_X));
    statesetComputation->addUniform(new osg::Uniform("numRows", (int)NUM_ELEMENTS_Y));
    statesetComputation->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    //blocksize
    int  numParticles = NUM_ELEMENTS_X * NUM_ELEMENTS_Y;
    const unsigned blockSize = numParticles * __numChannels * __numDataValuesPerChannel* sizeof(GLfloat);

    //init all the particle data array
    int idx = 0;
    _data = new GLfloat[NUM_ELEMENTS_X  * NUM_ELEMENTS_Y  * __numChannels * __numDataValuesPerChannel];
    _dataArray = new FloatArray;

    //init the data array  somehow, this way all is stored in one BufferObject. maybe better using multiple buffers instead? not sure what is faster and better for threading
    for (int d = 0; d < __numDataValuesPerChannel; ++d)
    {
        for (int i = 0; i < NUM_ELEMENTS_X; ++i)
        {

            for (int j = 0; j < NUM_ELEMENTS_Y; ++j)
            {

                for (int k = 0; k < __numChannels; ++k)
                {
                    switch (k)
                    {

                    case (RED_CHANNEL) :
                    {
                        if ((d == POSITION_NOW_OFFSET) || (d == POSITION_OLD_OFFSET) || (d == POSITION_INIT_OFFSET))//position
                        {
                            *_data = random(0.25, 0.75);
                        }
                        if ((d == VELOCITY_NOW_OFFSET) || (d == VELOCITY_OLD_OFFSET) || (d == VELOCITY_INIT_OFFSET))//velocity
                        {
                            *_data = random(-2.4, 2.4);
                        }
                        if (d == ACCELERATION_OFFSET) //acceleration
                        {
                            *_data = random(-3.0, 3.0);
                        }

                        if (d == PROPERTIES_OFFSET) //property particle mass (compute shader is computing sphere mass from radius instead)
                        {
                            *_data = random(0.2, 15.0);
                        }

                        break;
                    }

                    case (GREEN_CHANNEL) :
                    {
                        if ((d == POSITION_NOW_OFFSET) || (d == POSITION_OLD_OFFSET) || (d == POSITION_INIT_OFFSET))//position
                        {
                            *_data = random(0.25, 0.75);
                        }
                        if ((d == VELOCITY_NOW_OFFSET) || (d == VELOCITY_OLD_OFFSET) || (d == VELOCITY_INIT_OFFSET))//velocity
                        {
                            *_data = random(-2.4, 2.4);
                        }

                        if (d == ACCELERATION_OFFSET)//acceleration
                        {
                            *_data = random(-3.0, 3.0);
                        }
                        if (d == PROPERTIES_OFFSET) //property particle radius
                        {
                            *_data = random(0.07, 0.219);
                        }

                        break;
                    }

                    case (BLUE_CHANNEL) :
                    {
                        if ((d == POSITION_NOW_OFFSET) || (d == POSITION_OLD_OFFSET) || (d == POSITION_INIT_OFFSET))//position
                        {
                            *_data = random(0.25, 0.75);
                        }
                        if ((d == VELOCITY_NOW_OFFSET) || (d == VELOCITY_OLD_OFFSET) || (d == VELOCITY_INIT_OFFSET))//velocity
                        {
                            *_data = random(-2.4, 2.4);
                        }

                        if (d == ACCELERATION_OFFSET)//acceleration
                        {
                            *_data = random(-3.0, 3.0);
                        }


                        if (d == PROPERTIES_OFFSET)  //place for some other property
                        {
                            *_data = random(0.0, 0.0);
                        }

                        break;
                    }

                    case (ALPHA_CHANNEL) :
                    {
                        if ((d == POSITION_NOW_OFFSET) || (d == POSITION_OLD_OFFSET) || (d == POSITION_INIT_OFFSET))//position
                        {
                            *_data = random(1.0, 1.0);
                        }
                        if ((d == VELOCITY_NOW_OFFSET) || (d == VELOCITY_OLD_OFFSET) || (d == VELOCITY_INIT_OFFSET))//velocity
                        {
                            *_data = random(-2.4, 2.4);
                        }

                        if (d == ACCELERATION_OFFSET) //acceleration
                        {
                            //*_data = random(1.0, 1.0);
                            *_data = random(0.0, 0.0);
                        }

                        if (d == PROPERTIES_OFFSET) //place for some other property
                        {
                            *_data = random(0.3, 0.3);
                        }

                        break;
                    }



                    }
                    _dataArray->push_back(*_data);
                    _data++;
                    idx++;
                }
            }
        }
    }

    _ssbo = new osg::ShaderStorageBufferObject;
    _dataArray->setBufferObject(_ssbo.get());


    _ssbb = new osg::ShaderStorageBufferBinding(0, _dataArray.get(), 0, blockSize);
    statesetComputation->setAttributeAndModes(_ssbb.get(), osg::StateAttribute::ON);


    //option, do something useful with data or test the transfer speed
    //_ssbb->setUpdateCallback(new ShaderStorageBufferCallback);

    //adding a quad , visualizing data in buffer
    addDataMonitor(osg::Vec3(0, -1, 0), osg::Vec3(SUB_PLACEMENT_OFFSET_HORIZONTAL * 0, -SUB_PLACEMENT_OFFSET_VERTICAL * -2.0, SUB_PLACEMENT_OFFSET_HORIZONTAL * 0), 1.0, RGB_CHANNEL, POSITION_NOW_OFFSET, "X,Y,Z - PositionNow", -1.0, 1.0);

    //the coord from default dataset
    addHelperGeometry();


    addComputationResultsRenderTree();

}


//taken from osgdistorsion example for getting it nice on screen with antialiasing
osg::Node* createPrerenderSubgraph(osg::Node* subgraph, const osg::Vec4& clearColour)
{
    osg::Group* prerenderNode = new osg::Group;

    unsigned int tex_width = PRERENDER_WIDTH;
    unsigned int tex_height = PRERENDER_HEIGHT;

    osg::Texture2D* texture = new osg::Texture2D;
    texture->setTextureSize(tex_width, tex_height);
    texture->setInternalFormat(GL_RGBA);
    texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);

    {
        osg::Camera* prerenderCamera = new osg::Camera;
        prerenderCamera->setClearColor(clearColour);
        prerenderCamera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        prerenderCamera->setReferenceFrame(osg::Transform::RELATIVE_RF);
        prerenderCamera->setProjectionMatrix(osg::Matrixd::identity());
        prerenderCamera->setViewMatrix(osg::Matrixd::identity());
        prerenderCamera->setViewport(0, 0, tex_width, tex_height);
        prerenderCamera->setRenderOrder(osg::Camera::PRE_RENDER);
        prerenderCamera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);
        prerenderCamera->attach(osg::Camera::COLOR_BUFFER0, texture, 0, 0, false, PRERENDER_ANTIALIASINGMULTISAMPLES, PRERENDER_ANTIALIASINGMULTISAMPLES);
        prerenderCamera->addChild(subgraph);
        prerenderNode->addChild(prerenderCamera);

    }

    {
        osg::Geometry* polyGeom = new osg::Geometry();

        polyGeom->setSupportsDisplayList(false);

        osg::Vec3 origin(0.0f, 0.0f, 0.0f);
        osg::Vec3 xAxis(1.0f, 0.0f, 0.0f);
        osg::Vec3 yAxis(0.0f, 1.0f, 0.0f);

        float height = 1024.0f;
        float width = 1280.0f;
        int noSteps = 3;

        osg::Vec3Array* vertices = new osg::Vec3Array;
        osg::Vec2Array* texcoords = new osg::Vec2Array;
        osg::Vec4Array* colors = new osg::Vec4Array;

        osg::Vec3 bottom = origin;
        osg::Vec3 dx = xAxis*(width / ((float)(noSteps - 1)));
        osg::Vec3 dy = yAxis*(height / ((float)(noSteps - 1)));

        osg::Vec2 bottom_texcoord(0.0f, 0.0f);
        osg::Vec2 dx_texcoord(1.0f / (float)(noSteps - 1), 0.0f);
        osg::Vec2 dy_texcoord(0.0f, 1.0f / (float)(noSteps - 1));

        int i, j;
        for (i = 0; i < noSteps; ++i)
        {
            osg::Vec3 cursor = bottom + dy*(float)i;
            osg::Vec2 texcoord = bottom_texcoord + dy_texcoord*(float)i;
            for (j = 0; j < noSteps; ++j)
            {
                vertices->push_back(cursor);
                texcoords->push_back(osg::Vec2((sin(texcoord.x()*osg::PI - osg::PI*0.5) + 1.0f)*0.5f, (sin(texcoord.y()*osg::PI - osg::PI*0.5) + 1.0f)*0.5f));
                colors->push_back(osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
                cursor += dx;
                texcoord += dx_texcoord;
            }
        }

        polyGeom->setVertexArray(vertices);
        polyGeom->setColorArray(colors, osg::Array::BIND_PER_VERTEX);
        polyGeom->setTexCoordArray(0, texcoords);

        for (i = 0; i < noSteps - 1; ++i)
        {
            osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(osg::PrimitiveSet::QUAD_STRIP);
            for (j = 0; j < noSteps; ++j)
            {
                elements->push_back(j + (i + 1)*noSteps);
                elements->push_back(j + (i)*noSteps);
            }
            polyGeom->addPrimitiveSet(elements);
        }

        osg::StateSet* stateset = polyGeom->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0, texture, osg::StateAttribute::ON);
        stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

        osg::Geode* geode = new osg::Geode();
        geode->addDrawable(polyGeom);

        osg::Camera* nestedRenderCamera = new osg::Camera;
        nestedRenderCamera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
        nestedRenderCamera->setViewMatrix(osg::Matrix::identity());
        nestedRenderCamera->setProjectionMatrixAsOrtho2D(0, 1280, 0, 1024);
        nestedRenderCamera->setRenderOrder(osg::Camera::NESTED_RENDER);
        nestedRenderCamera->addChild(geode);

        prerenderNode->addChild(nestedRenderCamera);
    }

    return prerenderNode;
}




int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    osgViewer::Viewer viewer;

    osg::ref_ptr<osg::Group> scene = new osg::Group;


    viewer.addEventHandler(new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()));
    viewer.addEventHandler(new osgViewer::StatsHandler);
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);
    viewer.addEventHandler(new osgViewer::ThreadingHandler);
    viewer.getCamera()->setProjectionMatrixAsPerspective(60.0f, 1.33333, 0.01, 100.0);
    viewer.setCameraManipulator(new osgGA::TrackballManipulator());

    viewer.setUpViewInWindow(11, 11, 800 + 11, 600 + 11);
    //viewer.setUpViewOnSingleScreen(0); // !!

    viewer.getCamera()->setClearColor(osg::Vec4(0.3, 0.3, 0.3, 1.0));
    viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);// we can play with threading models later

    osg::ref_ptr<ComputeNode> computeNode = new ComputeNode();
    computeNode->setPosition(osg::Vec3(0, 0, 0));
    computeNode->setUpdateCallback(new ComputeNodeUpdateCallback(computeNode.get())); // on-the-fly reloading the shaders if shader source on disk is changed
    computeNode->initComputingSetup();



    scene->addChild(computeNode.get());
    scene->addChild(computeNode->_computationResultsRenderGroup.get());


#ifdef PRERENDER_HIGH_QUALITY_ANTIALIASING
    viewer.setSceneData(createPrerenderSubgraph(scene.get(), osg::Vec4(0.3, 0.4, 0.6, 1)));
#else
    viewer.setSceneData(scene.get());
#endif

    viewer.realize();

    viewer.getCamera()->getGraphicsContext()->getState()->setUseModelViewAndProjectionUniforms(true);

    viewer.run();

    return 1;
}
