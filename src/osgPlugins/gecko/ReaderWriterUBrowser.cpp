/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2008 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/

#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#include <osgViewer/ViewerEventHandlers>

#include "UBrowser.h"

class ReaderWriterUBrowser : public osgDB::ReaderWriter
{
    public:
    
        ReaderWriterUBrowser()
        {        
             osg::notify(osg::NOTICE)<<"ReaderWriterUBrowser::ReaderWriterUBrowser()"<<std::endl;

             supportsExtension("gecko","browser image");
             supportsExtension("browser","browser image");

             osg::ref_ptr<osgWidget::BrowserManager> previousManager = osgWidget::BrowserManager::instance();

            _initialized = false;

             osgWidget::BrowserManager::instance() = new UBrowserManager;
             
             if (previousManager.valid() && !(previousManager->getApplication().empty()))
             {
                osgWidget::BrowserManager::instance()->setApplication(previousManager->getApplication());
             }
             
             osg::notify(osg::NOTICE)<<"ReaderWriterUBrowser::ReaderWriterUBrowser() done"<<std::endl;
        }
    
        virtual ~ReaderWriterUBrowser()
        {
            // should we restore the previous value?
            osgWidget::BrowserManager::instance() = 0;
        }
        
        virtual const char* className() const { return "Browser Reader/Writer"; }
        
        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options =NULL) const
        {
            return readImage(file, options);
        }

        virtual ReadResult readImage(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            if (!_initialized)
            {
                if (osgWidget::BrowserManager::instance()->getApplication().empty())
                {
                    osgWidget::BrowserManager::instance()->setApplication(osg::DisplaySettings::instance()->getApplication());
                }
            
                osgWidget::BrowserManager::instance()->init(osgWidget::BrowserManager::instance()->getApplication());
            }

            
            return osgWidget::BrowserManager::instance()->createBrowserImage(osgDB::getNameLessExtension(file));
        }

        
        virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            osgDB::ReaderWriter::ReadResult result = readImage(fileName, options);
            if (!result.validImage()) return result;
            
            osg::Image* image = result.getImage();
            
            bool xyPlane = false;
            bool flip = image->getOrigin()==osg::Image::TOP_LEFT;
            osg::Vec3 origin = osg::Vec3(0.0f,0.0f,0.0f);
            float width = 1.0;
            float height = float(image->t())/float(image->s());
            osg::Vec3 widthAxis = osg::Vec3(width,0.0f,0.0f);
            osg::Vec3 heightAxis = xyPlane ? osg::Vec3(0.0f,height,0.0f) : osg::Vec3(0.0f,0.0f,height);

            osg::Geometry* pictureQuad = osg::createTexturedQuadGeometry(origin, widthAxis, heightAxis,
                                               0.0f, flip ? 1.0f : 0.0f , 1.0f, flip ? 0.0f : 1.0f);

            osg::Texture2D* texture = new osg::Texture2D(image);
            texture->setResizeNonPowerOfTwoHint(false);
            texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
            texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
            texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

            pictureQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                        texture,
                        osg::StateAttribute::ON);

            pictureQuad->setEventCallback(new osgViewer::InteractiveImageHandler(image));

            osg::Geode* geode = new osg::Geode;
            geode->addDrawable(pictureQuad);

            return geode;
        }

        bool _initialized;
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(ubrowser, ReaderWriterUBrowser)
