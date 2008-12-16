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

#include <osg/Notify>
#include <osgDB/ReadFile>
#include <osgViewer/ViewerEventHandlers>

#include <osgWidget/Browser>

#include <osg/io_utils>

using namespace osgWidget;


osg::ref_ptr<BrowserManager>& BrowserManager::instance()
{
    static osg::ref_ptr<BrowserManager> s_BrowserManager = new BrowserManager;
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

BrowserImage* BrowserManager::createBrowserImage(const std::string& url, int width, int height)
{
    osg::notify(osg::NOTICE)<<"Cannot created browser"<<std::endl;
    return 0;
}

Browser::Browser(const std::string& url, const GeometryHints& hints)
{
    open(url, hints);
}

bool Browser::assign(BrowserImage* browserImage, const GeometryHints& hints)
{
    if (!browserImage) return false;
    
    _browserImage = browserImage;

    bool flip = _browserImage->getOrigin()==osg::Image::TOP_LEFT;

    float aspectRatio = (_browserImage->t()>0 && _browserImage->s()>0) ? float(_browserImage->t()) / float(_browserImage->s()) : 1.0;

    osg::Vec3 widthVec(hints.widthVec);
    osg::Vec3 heightVec(hints.heightVec);
    
    switch(hints.aspectRatioPolicy)
    {
        case(GeometryHints::RESIZE_HEIGHT_TO_MAINTAINCE_ASPECT_RATIO):
            heightVec *= aspectRatio;
            break;
        case(GeometryHints::RESIZE_WIDTH_TO_MAINTAINCE_ASPECT_RATIO):
            widthVec /= aspectRatio;
            break;
        default:
            // no need to adjust aspect ratio
            break;
    }
    
    osg::Geometry* pictureQuad = osg::createTexturedQuadGeometry(hints.position, widthVec, heightVec,
                                       0.0f, flip ? 1.0f : 0.0f , 1.0f, flip ? 0.0f : 1.0f);

    osg::Texture2D* texture = new osg::Texture2D(_browserImage.get());
    texture->setResizeNonPowerOfTwoHint(false);
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

    pictureQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                texture,
                osg::StateAttribute::ON);

    osg::ref_ptr<osgViewer::InteractiveImageHandler> handler = new osgViewer::InteractiveImageHandler(_browserImage.get());

    pictureQuad->setEventCallback(handler.get());
    pictureQuad->setCullCallback(handler.get());

    addDrawable(pictureQuad);

    return true;
}

bool Browser::open(const std::string& hostname, const GeometryHints& hints)
{
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(hostname+".gecko");
    return assign(dynamic_cast<BrowserImage*>(image.get()), hints);
}

void Browser::navigateTo(const std::string& url)
{
    if (_browserImage.valid()) _browserImage->navigateTo(url);
}
