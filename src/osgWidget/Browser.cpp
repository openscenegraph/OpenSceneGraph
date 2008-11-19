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

#include <osgWidget/Browser>

#include <osg/Notify>

using namespace osgWidget;


osg::ref_ptr<BrowserManager>& BrowserManager::instance()
{
    static osg::ref_ptr<BrowserManager> s_BrowserManager;
    return s_BrowserManager;
}

BrowserManager::BrowserManager()
{
    osg::notify(osg::NOTICE)<<"Constructing base BrowserManager"<<std::endl;
}

BrowserManager::~BrowserManager()
{
    osg::notify(osg::NOTICE)<<"Destructing base BrowserManager"<<std::endl;
}

void BrowserManager::init(const std::string& application)
{
    _application = application;
}

BrowserImage* BrowserManager::createBrowserImage(const std::string& url)
{
    return createBrowserImage(url, 1024, 1024);
}

BrowserImage* BrowserManager::createBrowserImage(const std::string& url, int width, int height)
{
    osg::notify(osg::NOTICE)<<"Cannot created browser"<<std::endl;
    return 0;
}
