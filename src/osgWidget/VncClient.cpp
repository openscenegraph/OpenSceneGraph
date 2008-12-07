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

VncClient::VncClient(const std::string& hostname, GeometryHints hints)
{
    connect(hostname, hints);
}

bool VncClient::assign(VncImage* vncImage, GeometryHints hints)
{
    if (!vncImage) return false;
    
    _vncImage = vncImage;

    bool flip = _vncImage->getOrigin()==osg::Image::TOP_LEFT;

    osg::Geometry* pictureQuad = osg::createTexturedQuadGeometry(hints.position, hints.widthVec, hints.heightVec,
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

bool VncClient::connect(const std::string& hostname, GeometryHints hints)
{
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(hostname+".vnc");
    return assign(dynamic_cast<VncImage*>(image.get()));
}

void VncClient::close()
{
    if (!_vncImage) return;
    
    _vncImage->close();
}
