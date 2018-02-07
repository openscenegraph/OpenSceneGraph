/* -*-c++-*- OpenSceneGraph - Copyright (C) 2012-2012 David Callu
 *                                          2017-2018 Julien Valentin
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commercial and non commercial applications,
 * as long as this copyright notice is maintained.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

#include <osg/BufferIndexBinding>
#include <osg/BindImageTexture>
#include <osg/TextureBuffer>
#include <osg/BufferObject>
#include <osg/Camera>
#include <osg/Program>

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>
#include <osgGA/SphericalManipulator>

class ReadBackAndResetCallback : public osg::SyncBufferDataCallback
{
    osg::Uniform *_invNumPixelUniform;
public:
    ReadBackAndResetCallback(osg::BufferData*bd, osg::Uniform* invNumPixelUniform):
        osg::SyncBufferDataCallback(bd),
        _invNumPixelUniform(invNumPixelUniform)
    {
        setHostAccess(GL_READ_WRITE);
        setDeviceAccess(GL_READ_WRITE);
    }

    virtual bool synctraversal(osg::Node *,osg::NodeVisitor*)
    {
        osg::UIntArray * array = dynamic_cast<  osg::UIntArray*>(_bd.get());
        unsigned int numPixel =  array->front();
        _invNumPixelUniform->set( 1.0f / static_cast<float>( std::max(1u, numPixel)) );
        // if ((renderInfo.getView()->getFrameStamp()->getFrameNumber() % 10) == 0)
        {
            OSG_WARN << "osgatomiccounter : draw " << numPixel << " pixels." << std::endl;
        }
        (*array)[0] = 0;
        _bd->dirty();
        return true;
    }
};

enum AtomicImplementation
{
    ATOMIC_COUNTER,
    SSBO,
    IMAGE_API
};
osg::Program * createProgram(AtomicImplementation atomicImpl)
{

    std::stringstream vp;
    vp << "#version 420 compatibility\n"
       << "\n"
       << "void main(void)\n"
       << "{\n"
       << "    gl_Position = ftransform();\n"
       << "}\n";
    osg::Shader * vpShader = new osg::Shader( osg::Shader::VERTEX, vp.str() );

    std::stringstream fp;
    switch(atomicImpl)
    {
    case ATOMIC_COUNTER:
        fp<< "#version 420 compatibility\n"
          << "layout(binding = 0) uniform atomic_uint acRed;\n"
          << "layout(binding = 0, offset = 4) uniform atomic_uint acGreen;\n"
          << "layout(binding = 2) uniform atomic_uint acBlue;\n"
          << "\n"
          << "uniform float invNumPixel;\n"
          << "\n"
          << "void main(void)\n"
          << "{\n"
          << "    float r = float(atomicCounterIncrement(acRed)) * invNumPixel;\n"
          << "    float g = float(atomicCounterIncrement(acGreen)) * invNumPixel;\n"
          << "    float b = float(atomicCounterIncrement(acBlue)) * invNumPixel;\n"
          << "    gl_FragColor = vec4(r, g, b, 1.0);\n"
          << "}\n"
          << "\n";
        break;
    case SSBO:
        fp << "#version 430 compatibility\n"
           << "layout(std430, binding=0) buffer acRedAndGreen{uint acRedGreen[];}; "
           << "layout(std430, binding=2) buffer acBlueOnly{uint acBlue[];}; "
           << "\n"
           << "uniform float invNumPixel;\n"
           << "\n"
           << "void main(void)\n"
           << "{\n"
           << "    float r = float(atomicAdd(acRedGreen[0],1)) * invNumPixel;\n"
           ///atomicAdd(acRedGreen[1],1) does nothing on Linux 4.5.0 NVIDIA 381.22
           << "    float g = float(atomicAdd(acRedGreen[1],1)) * invNumPixel;\n"
           << "    float b = float(atomicAdd(acBlue[0],1)) * invNumPixel;\n"
           << "    gl_FragColor = vec4(r, g, b, 1.0);\n"
           << "}\n"
           << "\n";
        break;
    case IMAGE_API:
        fp << "#version 430 compatibility\n"
           << "uniform layout(binding = 0, r32ui )  uimageBuffer acRedGreen;\n"
           << "uniform layout(binding = 2, r32ui )  uimageBuffer acBlue;\n"
           << "\n"
           << "uniform float invNumPixel;\n"
           << "\n"
           << "void main(void)\n"
           << "{\n"
           << "    float r = float(imageAtomicAdd(acRedGreen,0,1)) * invNumPixel;\n"
           << "    float g = float(imageAtomicAdd(acRedGreen,1,1)) * invNumPixel;\n"
           << "    float b = float(imageAtomicAdd(acBlue,0,1)) * invNumPixel;\n"
           << "    gl_FragColor = vec4(r, g, b, 1.0);\n"
           << "}\n"
           << "\n";
        break;
    }
    osg::Shader * fpShader = new osg::Shader( osg::Shader::FRAGMENT, fp.str() );

    osg::Program * program = new osg::Program;
    program->addShader(vpShader);
    program->addShader(fpShader);

    return program;
}

class ResetAtomicCounter : public osg::StateAttributeCallback
{
public:
    virtual void operator () (osg::StateAttribute* sa, osg::NodeVisitor*)
    {
        osg::BufferIndexBinding * acbb = dynamic_cast<osg::BufferIndexBinding *>(sa);
        if (acbb)
        {
            osg::BufferData * acbd = acbb->getBufferData();
            if (acbd)
            {
                acbd->dirty();
            }
        }
    }
};
class ResetImageTexture : public osg::StateAttributeCallback
{
public:
    virtual void operator () (osg::StateAttribute* sa, osg::NodeVisitor*)
    {
         //   OSG_WARN<<"///ResetImageTexture"<<std::endl;
        osg::BindImageTexture * bim=dynamic_cast<osg::BindImageTexture *>(sa);
        if(bim)
        {
        osg::TextureBuffer * acbb = dynamic_cast<osg::TextureBuffer *>(bim->getTexture());
        if (acbb)
        {
            osg::BufferData * acbd =const_cast<osg::BufferData*>(acbb->getBufferData());
            if (acbd)
            {
                acbd->dirty();
            }
        }
        }
    }
};


class  MemBarrierDraw :public osg::Drawable::DrawCallback
{
public:
    GLenum _flags;
    MemBarrierDraw(GLenum f):osg::Drawable::DrawCallback(),_flags(f) {}
    virtual void drawImplementation(osg::RenderInfo& renderInfo,const osg::Drawable* dr) const
    {
        dr->drawImplementation(renderInfo);
        renderInfo.getState()->get<osg::GLExtensions>()->glMemoryBarrier( _flags );
    }
};
int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is a simple example which show draw order of pixel with differents ways.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [ --impl ATOMIC_COUNTER | SSBO | IMAGE_API].");

    osgViewer::Viewer viewer(arguments);
    AtomicImplementation impl=ATOMIC_COUNTER;
    std::string implem="";
    while(arguments.read("--impl",implem)) {}
    unsigned int helpType = 0;
    if ((helpType = arguments.readHelpType()))
    {
        arguments.getApplicationUsage()->write(std::cout, helpType);
        return 1;
    }
    if(!implem.empty())
    {
        if(implem=="ATOMIC_COUNTER")impl= ATOMIC_COUNTER;
        if(implem=="SSBO")impl= SSBO;
        if(implem=="IMAGE_API")impl= IMAGE_API;

    }
    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    // set up the camera manipulators.
    viewer.setCameraManipulator( new osgGA::TrackballManipulator() );

    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    // add the thread model handler
    viewer.addEventHandler(new osgViewer::ThreadingHandler);

    // add the window size toggle handler
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    // add the help handler
    viewer.addEventHandler(new osgViewer::HelpHandler(arguments.getApplicationUsage()));

    // add the screen capture handler
    viewer.addEventHandler(new osgViewer::ScreenCaptureHandler);

    // load the data
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);
    if (!loadedModel)
    {
        osg::Geometry * quad = osg::createTexturedQuadGeometry(osg::Vec3f(-2.0f, 0.0f, -2.0f),
                               osg::Vec3f(2.0f, 0.0f, 0.0f),
                               osg::Vec3f(0.0f, 0.0f, 2.0f) );
        quad->setUseDisplayList(false);
        quad->setUseVertexBufferObjects(true);
        if(impl==IMAGE_API) //ensure sync with image api
            quad->setDrawCallback(new MemBarrierDraw(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT));
        osg::Geode * geode = new osg::Geode;
        geode->addDrawable(quad);
        loadedModel = geode;
    }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }


    osg::StateSet * ss = loadedModel->asGeode()->getDrawable(0)->getOrCreateStateSet();
    ss->setAttributeAndModes( createProgram(impl), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE | osg::StateAttribute::PROTECTED );

    ss = loadedModel->getOrCreateStateSet();
    osg::ref_ptr<osg::UIntArray> atomicCounterArrayRedAndGreen = new osg::UIntArray;
    atomicCounterArrayRedAndGreen->push_back(0);
    atomicCounterArrayRedAndGreen->push_back(0);
    atomicCounterArrayRedAndGreen->push_back(0);
    atomicCounterArrayRedAndGreen->push_back(0);

    osg::ref_ptr<osg::UIntArray> atomicCounterArrayBlue = new osg::UIntArray;
    atomicCounterArrayBlue->push_back(0);
    atomicCounterArrayBlue->push_back(0);
    atomicCounterArrayBlue->push_back(0);
    atomicCounterArrayBlue->push_back(0);
    switch(impl)
    {
    case ATOMIC_COUNTER:
    {
        osg::ref_ptr<osg::AtomicCounterBufferObject> acboRedAndGreen = new osg::AtomicCounterBufferObject;
        acboRedAndGreen->setUsage(GL_STREAM_COPY);
        atomicCounterArrayRedAndGreen->setBufferObject(acboRedAndGreen.get());

        osg::ref_ptr<osg::AtomicCounterBufferObject> acboBlue = new osg::AtomicCounterBufferObject;
        acboBlue->setUsage(GL_STREAM_COPY);
        atomicCounterArrayBlue->setBufferObject(acboBlue.get());

        osg::ref_ptr<osg::AtomicCounterBufferBinding> acbbRedAndGreen = new osg::AtomicCounterBufferBinding(0, atomicCounterArrayRedAndGreen.get());
        ss->setAttributeAndModes(acbbRedAndGreen.get());

        osg::ref_ptr<osg::AtomicCounterBufferBinding> acbbBlue = new osg::AtomicCounterBufferBinding(2, atomicCounterArrayBlue.get(), 0, sizeof(GLuint));
        ss->setAttributeAndModes(acbbBlue.get());

        acbbRedAndGreen->setUpdateCallback(new ResetAtomicCounter);
        acbbBlue->setUpdateCallback(new ResetAtomicCounter);
    }
    break;
    case SSBO:
    {
        osg::ref_ptr<osg::ShaderStorageBufferObject> acboRedAndGreen = new osg::ShaderStorageBufferObject;
        acboRedAndGreen->setUsage(GL_DYNAMIC_DRAW);
        atomicCounterArrayRedAndGreen->setBufferObject(acboRedAndGreen.get());

        osg::ref_ptr<osg::ShaderStorageBufferObject> acboBlue = new osg::ShaderStorageBufferObject;
        acboBlue->setUsage(GL_DYNAMIC_DRAW);
        atomicCounterArrayBlue->setBufferObject(acboBlue.get());

        osg::ref_ptr<osg::ShaderStorageBufferBinding> acbbRedAndGreen = new osg::ShaderStorageBufferBinding(0, atomicCounterArrayRedAndGreen.get(), 0, sizeof(GLuint)*3);
        ss->setAttributeAndModes(acbbRedAndGreen.get());

        osg::ref_ptr<osg::ShaderStorageBufferBinding> acbbBlue = new osg::ShaderStorageBufferBinding(2, atomicCounterArrayBlue.get(), 0, sizeof(GLuint));
        ss->setAttributeAndModes(acbbBlue.get());

        acbbRedAndGreen->setUpdateCallback(new ResetAtomicCounter);
        acbbBlue->setUpdateCallback(new ResetAtomicCounter);
    }
    break;
    case IMAGE_API:
    {
        osg::ref_ptr<osg::TextureBuffer> atomicCounterArrayRedAndGreenTex=new osg::TextureBuffer(atomicCounterArrayRedAndGreen);
        atomicCounterArrayRedAndGreenTex->setInternalFormat(GL_R32UI);
        osg::ref_ptr<osg::BindImageTexture> acbbRedAndGreen = new osg::BindImageTexture(0, atomicCounterArrayRedAndGreenTex,osg::BindImageTexture::READ_WRITE);
        ss->setAttributeAndModes(acbbRedAndGreen.get());

        osg::ref_ptr<osg::TextureBuffer> atomicCounterArrayBlueTex=new osg::TextureBuffer(atomicCounterArrayBlue);
        atomicCounterArrayBlueTex->setInternalFormat(GL_R32UI);
        osg::ref_ptr<osg::BindImageTexture> acbbBlue = new osg::BindImageTexture(2, atomicCounterArrayBlueTex,osg::BindImageTexture::READ_WRITE);
        ss->setAttributeAndModes(acbbBlue.get());

        ///just in order to force apply on each frame (BindImage can't check )
        ss->setTextureAttribute(0,atomicCounterArrayRedAndGreenTex);
        ss->setTextureAttribute(1,atomicCounterArrayBlueTex);

        acbbBlue->setUpdateCallback(new ResetImageTexture);
        acbbRedAndGreen->setUpdateCallback(new ResetImageTexture);

    }

    }

    osg::ref_ptr<osg::Uniform> invNumPixelUniform = new osg::Uniform("invNumPixel", 1.0f/(800.0f*600.0f));
    ss->addUniform( invNumPixelUniform.get() );

    loadedModel->addUpdateCallback(new ReadBackAndResetCallback(atomicCounterArrayBlue, invNumPixelUniform));

    // optimize the scene graph, remove redundant nodes and state etc.
    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get());

    viewer.setSceneData( loadedModel.get() );

    viewer.realize();

    return viewer.run();

}
