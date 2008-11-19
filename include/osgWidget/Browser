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

#ifndef OSGWIDGET_BROWSER
#define OSGWIDGET_BROWSER

#include <osg/Image>
#include <osgWidget/Export>

namespace osgWidget {


class BrowserImage;

class BrowserManager : public osg::Object
{
    public:
    
        static osg::ref_ptr<BrowserManager>& instance();
        
        virtual void init(const std::string& application);

        virtual BrowserImage* createBrowserImage(const std::string& url);
        virtual BrowserImage* createBrowserImage(const std::string& url, int width, int height);

    protected:

        BrowserManager();
        BrowserManager(const BrowserManager& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY) {}
        virtual ~BrowserManager();
        
        META_Object(osgWidget,BrowserManager)

        std::string _application;
};


class BrowserImage : public osg::Image
{
    public:
    
        virtual void navigateTo(const std::string& url) = 0;
        
    
};


}

#endif
