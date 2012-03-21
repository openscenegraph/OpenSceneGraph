/* -*-c++-*- OpenSceneGraph - Copyright (C) 2007 Cedric Pinson
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commercial and non commercial
 * applications, as long as this copyright notice is maintained.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Authors:
 *  Cedric Pinson <mornifle@plopbyte.net>
 *
 */


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifndef DATADIR
#define DATADIR "."
#endif // DATADIR

#include <osg/GraphicsContext>
#include "RenderSurface.h"
#include "CameraConfig.h"
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osg/io_utils>

using namespace osgProducer;
static osg::GraphicsContext::Traits* buildTrait(RenderSurface& rs)
{
    VisualChooser& vc = *rs.getVisualChooser();

    osg::GraphicsContext::Traits* traits = new osg::GraphicsContext::Traits;

    for (std::vector<VisualChooser::VisualAttribute>::iterator it = vc._visual_attributes.begin();
        it != vc._visual_attributes.end();
        it++)
    {
        switch(it->_attribute)
        {
            case(VisualChooser::UseGL):            break; // on by default in osgViewer
            case(VisualChooser::BufferSize):       break; // no present mapping
            case(VisualChooser::Level):            traits->level = it->_parameter; break;
            case(VisualChooser::RGBA):             break; // automatically set in osgViewer
            case(VisualChooser::DoubleBuffer):     traits->doubleBuffer = true; break;
            case(VisualChooser::Stereo):           traits->quadBufferStereo = true; break;
            case(VisualChooser::AuxBuffers):       break; // no present mapping
            case(VisualChooser::RedSize):          traits->red = it->_parameter; break;
            case(VisualChooser::GreenSize):        traits->green = it->_parameter; break;
            case(VisualChooser::BlueSize):         traits->blue = it->_parameter; break;
            case(VisualChooser::AlphaSize):        traits->alpha = it->_parameter; break;
            case(VisualChooser::DepthSize):        traits->depth = it->_parameter; break;
            case(VisualChooser::StencilSize):      traits->stencil = it->_parameter; break;
            case(VisualChooser::AccumRedSize):     break; // no present mapping
            case(VisualChooser::AccumGreenSize):   break; // no present mapping
            case(VisualChooser::AccumBlueSize):    break; // no present mapping
            case(VisualChooser::AccumAlphaSize):   break; // no present mapping
            case(VisualChooser::Samples):          traits->samples = it->_parameter; break;
            case(VisualChooser::SampleBuffers):    traits->sampleBuffers = 1; break;
        }
    }

    OSG_INFO<<"ReaderWriterCFG buildTrait traits->depth="<<traits->depth<<std::endl;
    OSG_INFO<<"ReaderWriterCFG buildTrait traits->samples="<<traits->samples<<std::endl;
    OSG_INFO<<"ReaderWriterCFG buildTrait traits->sampleBuffers="<<traits->sampleBuffers<<std::endl;


    traits->hostName = rs.getHostName();
    traits->displayNum = rs.getDisplayNum();
    traits->screenNum = rs.getScreenNum();
    traits->windowName = rs.getWindowName();
    traits->x = rs.getWindowOriginX();
    traits->y = rs.getWindowOriginY();
    traits->width = rs.getWindowWidth();
    traits->height = rs.getWindowHeight();
    traits->windowDecoration = rs.usesBorder();
    traits->sharedContext = 0;
    traits->pbuffer = (rs.getDrawableType()==osgProducer::RenderSurface::DrawableType_PBuffer);

    traits->overrideRedirect = rs.usesOverrideRedirect();

    return traits;
 }

static osgViewer::View* load(const std::string& file, const osgDB::ReaderWriter::Options* option)
{
    osg::ref_ptr<CameraConfig> config = new CameraConfig;
    //std::cout << "Parse file " << file << std::endl;
    config->parseFile(file);

    RenderSurface* rs = 0;
    Camera* cm = 0;
    std::map<RenderSurface*,osg::ref_ptr<osg::GraphicsContext> > surfaces;
    osg::ref_ptr<osgViewer::View> _view = new osgViewer::View;

    for (int i = 0; i < (int)config->getNumberOfCameras(); i++)
    {
        cm = config->getCamera(i);
        rs = cm->getRenderSurface();
        if (rs->getDrawableType() != osgProducer::RenderSurface::DrawableType_Window)
            continue;

        osg::ref_ptr<const osg::GraphicsContext::Traits> traits;
        osg::ref_ptr<osg::GraphicsContext> gc;
        if (surfaces.find(rs) != surfaces.end())
        {
            gc = surfaces[rs];
            traits = gc.valid() ? gc->getTraits() : 0;
        }
        else
        {
            osg::GraphicsContext::Traits* newtraits = buildTrait(*rs);

#if 0
            osg::GraphicsContext::ScreenIdentifier si;
            si.readDISPLAY();

            if (si.displayNum>=0) newtraits->displayNum = si.displayNum;
            if (si.screenNum>=0) newtraits->screenNum = si.screenNum;
#endif

            gc = osg::GraphicsContext::createGraphicsContext(newtraits);

            surfaces[rs] = gc.get();
            traits = gc.valid() ? gc->getTraits() : 0;
        }

        // std::cout << rs->getWindowName() << " " << rs->getWindowOriginX() << " " << rs->getWindowOriginY() << " " << rs->getWindowWidth() << " " << rs->getWindowHeight() << std::endl;

        if (gc.valid())
        {
            OSG_INFO<<"  GraphicsWindow has been created successfully."<<std::endl;

            osg::ref_ptr<osg::Camera> camera = new osg::Camera;
            camera->setGraphicsContext(gc.get());


            int x,y;
            unsigned int width,height;
            cm->applyLens();
            cm->getProjectionRectangle(x, y, width, height);
            camera->setViewport(new osg::Viewport(x, y, width, height));

            GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
            camera->setDrawBuffer(buffer);
            camera->setReadBuffer(buffer);

            osg::Matrix projection(cm->getProjectionMatrix());

            osg::Matrix offset = osg::Matrix::identity();
            cm->setViewByMatrix(offset);
            osg::Matrix view = osg::Matrix(cm->getPositionAndAttitudeMatrix());

            // setup projection from parent
            osg::Matrix offsetProjection = osg::Matrix::inverse(_view->getCamera()->getProjectionMatrix()) * projection;
            _view->addSlave(camera.get(), offsetProjection, view);

            #if 0
            std::cout << "Matrix Projection " << projection << std::endl;
            std::cout << "Matrix Projection master " << _view->getCamera()->getProjectionMatrix() << std::endl;
            // will work only if it's a post multyply in the producer camera
            std::cout << "Matrix View " << view << std::endl;
            std::cout << _view->getCamera()->getProjectionMatrix() * offsetProjection << std::endl;
            #endif
        }
        else
        {
            OSG_INFO<<"  GraphicsWindow has not been created successfully."<<std::endl;
            return 0;
        }

    }

    // std::cout << "done" << std::endl;
    return _view.release();
}

//
// OSG interface to read/write from/to a file.
//
class ReaderWriterProducerCFG : public osgDB::ReaderWriter
{
public:

    ReaderWriterProducerCFG()
    {
        supportsExtension("cfg","Producer camera configuration file");
    }

    virtual const char* className() { return "Producer cfg object reader"; }


    virtual ReadResult readObject(const std::string& fileName, const Options* options = NULL) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(fileName);
        if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

        osgDB::FilePathList* filePathList = 0;
        if(options)
        {
            filePathList = const_cast<osgDB::FilePathList*>(&(options->getDatabasePathList()));
            filePathList->push_back(DATADIR);
        }

        std::string path = osgDB::findDataFile(fileName);
        if(path.empty()) return ReadResult::FILE_NOT_FOUND;

        ReadResult result;
        osg::ref_ptr<osg::View> view = load(path, options);
        if(! view.valid())
            result = ReadResult("Error: could not load " + path);
        else
            result = ReadResult(view.get());

        if(options && filePathList)
            filePathList->pop_back();

        return result;
    }

};


REGISTER_OSGPLUGIN(cfg, ReaderWriterProducerCFG)
