/* -*-c++-*- OpenSceneGraph - Copyright (C) 2007 Cedric Pinson
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial
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
    osg::GraphicsContext::Traits* trait = new osg::GraphicsContext::Traits;
    for (std::vector<VisualChooser::VisualAttribute>::iterator it = vc._visual_attributes.begin(); 
        it != vc._visual_attributes.end(); 
        it++)
    {
        if (it->_attribute == VisualChooser::DoubleBuffer)
            trait->doubleBuffer = true;
        else if (it->_attribute == VisualChooser::DepthSize)
            trait->depth = it->_parameter;
        else if (it->_attribute == VisualChooser::RedSize)
            trait->red = it->_parameter;
        else if (it->_attribute == VisualChooser::BlueSize)
            trait->blue = it->_parameter;
        else if (it->_attribute == VisualChooser::GreenSize)
            trait->green = it->_parameter;
        else if (it->_attribute == VisualChooser::AlphaSize)
            trait->alpha = it->_parameter;
        else
            std::cout << it->_attribute << " value " << it->_parameter << std::endl;
    }

    trait->windowName = rs.getWindowName();
    trait->x = rs.getWindowOriginX();
    trait->y = rs.getWindowOriginY();
    trait->width = rs.getWindowWidth();
    trait->height = rs.getWindowHeight();
    trait->windowDecoration = rs.usesBorder();
    trait->sharedContext = 0;

    return trait;
 }

static osgViewer::View* load(const std::string& file, const osgDB::ReaderWriter::Options* option)
{
    osg::ref_ptr<CameraConfig> config = new CameraConfig;
    std::cout << "Parse file " << file << std::endl;
    config->parseFile(file);

    RenderSurface* rs = 0;
    Camera* cm = 0;
    std::map<RenderSurface*,osg::ref_ptr<osg::GraphicsContext> > surfaces;
    osgViewer::View* _view = new osgViewer::View;

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
            traits = gc->getTraits();
        }
        else
        {
            osg::GraphicsContext::Traits* newtraits = buildTrait(*rs);
            
            newtraits->readDISPLAY();
            if (newtraits->displayNum<0) newtraits->displayNum = 0;
            
            gc = osg::GraphicsContext::createGraphicsContext(newtraits);
            
            surfaces[rs] = gc.get();
            traits = gc.valid() ? gc->getTraits() : 0;
        }

        std::cout << rs->getWindowName() << " " << rs->getWindowOriginX() << " " << rs->getWindowOriginY() << " " << rs->getWindowWidth() << " " << rs->getWindowHeight() << std::endl;

        if (gc.valid())
        {
            osg::notify(osg::INFO)<<"  GraphicsWindow has been created successfully."<<std::endl;

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
            #if 1
            std::cout << "Matrix Projection " << projection << std::endl;
            std::cout << "Matrix Projection master " << _view->getCamera()->getProjectionMatrix() << std::endl;
            // will work only if it's a post multyply in the producer camera
            std::cout << "Matrix View " << view << std::endl;
            #endif
            // setup projection from parent
            osg::Matrix offsetProjection = osg::Matrix::inverse(_view->getCamera()->getProjectionMatrix()) * projection;
            std::cout << _view->getCamera()->getProjectionMatrix() * offsetProjection << std::endl;
            //    std::cout << "setViewByMatrix " << offset << std::endl;

            _view->addSlave(camera.get(), offsetProjection, view);
            //     _view->getCamera()->setProjectionMatrix(projection); //osg::Matrix::identity());
            //     _view->addSlave(camera.get(), osg::Matrix::identity(), view);
        }
        else
        {
            osg::notify(osg::NOTICE)<<"  GraphicsWindow has not been created successfully."<<std::endl;
        }
        
    }

#if 0
    if (_view->getNumberOfCameras() == 1)
    {
        _view->addSlave(camera.get());
    }
#endif

    std::cout << "done" << std::endl;
    return _view;
}

//
// OSG interface to read/write from/to a file.
//
class ReaderWriterProducerCFG : public osgDB::ReaderWriter 
{
public:

    virtual const char* className() { return "Producer cfg object reader"; }

    virtual bool acceptsExtension(const std::string& extension) const
    {
        return osgDB::equalCaseInsensitive(extension, "cfg");
    }

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
            result = ReadResult("failed to load " + path);
        else
            result = ReadResult(view.get());

        if(options && filePathList)
            filePathList->pop_back();

        return result;
    }

};


REGISTER_OSGPLUGIN(cfg, ReaderWriterProducerCFG)
