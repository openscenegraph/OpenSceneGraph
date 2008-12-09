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

            unsigned int width = 1024;
            unsigned int height = 1024;
            
            return osgWidget::BrowserManager::instance()->createBrowserImage(osgDB::getNameLessExtension(file), width, height);
        }

        
        virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            osgDB::ReaderWriter::ReadResult result = readImage(fileName, options);
            if (!result.validImage()) return result;
            
            osg::ref_ptr<osgWidget::Browser> browser = new osgWidget::Browser();
            if (browser->assign(dynamic_cast<osgWidget::BrowserImage*>(result.getImage())))
            {
                return browser.release();
            }
            else
            {
                return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;
            }
        }

        bool _initialized;
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(ubrowser, ReaderWriterUBrowser)
