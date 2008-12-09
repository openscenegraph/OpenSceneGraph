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

#include <osg/Geode>
#include <osgDB/ReadFile>
#include <osgViewer/ViewerEventHandlers>

#include <osgWidget/PdfReader>

#include <osg/io_utils>

using namespace osgWidget;

PdfReader::PdfReader(const std::string& filename, const GeometryHints& hints)
{
    open(filename, hints);
}
        
bool PdfReader::assign(PdfImage* pdfImage, const GeometryHints& hints)
{
    if (!pdfImage) return false;

    _pdfImage = pdfImage;
    _pdfImage->setBackgroundColor(hints.backgroundColor);

    bool flip = _pdfImage->getOrigin()==osg::Image::TOP_LEFT;

    float aspectRatio = (_pdfImage->t()>0 && _pdfImage->s()>0) ? float(_pdfImage->t()) / float(_pdfImage->s()) : 1.0;

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

    osg::Texture2D* texture = new osg::Texture2D(_pdfImage.get());
    texture->setResizeNonPowerOfTwoHint(false);
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);

    pictureQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                texture,
                osg::StateAttribute::ON);

    pictureQuad->setEventCallback(new osgViewer::InteractiveImageHandler(_pdfImage.get()));

    addDrawable(pictureQuad);
    
    return true;
}

bool PdfReader::open(const std::string& filename, const GeometryHints& hints)
{
    osg::ref_ptr<osg::Image> image = osgDB::readImageFile(filename);
    return assign(dynamic_cast<PdfImage*>(image.get()), hints);
}

bool PdfReader::page(int pageNum)
{
    if (!_pdfImage) return false;
    
    return _pdfImage->page(pageNum);
}

bool PdfReader::previous()
{
    if (!_pdfImage) return false;
    
    return _pdfImage->previous();
}

bool PdfReader::next()
{
    if (!_pdfImage) return false;
    
    return _pdfImage->next();
}
