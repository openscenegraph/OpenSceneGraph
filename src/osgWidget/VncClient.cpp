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

#include <osgDB/ReadFile>
#include <osgViewer/ViewerEventHandlers>
#include <osgWidget/VncClient>

using namespace osgWidget;

VncClient::VncClient(const std::string& hostname, const GeometryHints& hints)
{
    connect(hostname, hints);
}

bool VncClient::assign(VncImage* vncImage, const GeometryHints& hints)
{
    if (!vncImage) return false;
    
    _vncImage = vncImage;

    bool flip = _vncImage->getOrigin()==osg::Image::TOP_LEFT;

    float aspectRatio = (_vncImage->t()>0 && _vncImage->s()>0) ? float(_vncImage->t()) / float(_vncImage->s()) : 1.0;

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

    osg::Texture2D* texture = new osg::Texture2D(_vncImage.get());
    texture->setResizeNonPowerOfTwoHint(false);
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

    pictureQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                texture,
                osg::StateAttribute::ON);

    pictureQuad->setEventCallback(new osgViewer::InteractiveImageHandler(_vncImage.get()));

    addDrawable(pictureQuad);

    return true;
}

bool VncClient::connect(const std::string& hostname, const GeometryHints& hints)
{
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(hostname+".vnc");
    return assign(dynamic_cast<VncImage*>(image.get()), hints);
}

void VncClient::close()
{
    if (!_vncImage) return;
    
    _vncImage->close();
}
