/* OpenSceneGraph example, osgphotoalbum.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/


#include "ImageReaderWriter.h"

#include <osg/Texture2D>
#include <osg/Geometry>
#include <osg/Geode>

#include <sstream>


ImageReaderWriter::DataReference::DataReference():
    _fileName(),
    _resolutionX(256),
    _resolutionY(256),
    _center(0.625f,0.0f,0.0f),
    _maximumWidth(1.25f,0.0f,0.0f),
    _maximumHeight(0.0f,0.0f,1.0f),
    _numPointsAcross(10),
    _numPointsUp(10),
    _backPage(false) {}

ImageReaderWriter::DataReference::DataReference(const std::string& fileName, unsigned int res, float width, float height,bool backPage):
    _fileName(fileName),
    _resolutionX(res),
    _resolutionY(res),
    _center(width*0.5f,0.0f,height*0.5f),
    _maximumWidth(width,0.0f,0.0f),
    _maximumHeight(0.0f,0.0f,height),
    _numPointsAcross(10),
    _numPointsUp(10),
    _backPage(backPage) {}

ImageReaderWriter::DataReference::DataReference(const DataReference& rhs):
    _fileName(rhs._fileName),
    _resolutionX(rhs._resolutionX),
    _resolutionY(rhs._resolutionY),
    _center(rhs._center),
    _maximumWidth(rhs._maximumWidth),
    _maximumHeight(rhs._maximumHeight),
    _numPointsAcross(rhs._numPointsAcross),
    _numPointsUp(rhs._numPointsUp),
    _backPage(rhs._backPage) {}



ImageReaderWriter::ImageReaderWriter()
{
}

std::string ImageReaderWriter::local_insertReference(const std::string& fileName, unsigned int res, float width, float height, bool backPage)
{
    std::stringstream ostr;
    ostr<<"res_"<<res<<"_"<<fileName;

    std::string myReference = ostr.str();
    _dataReferences[myReference] = DataReference(fileName,res,width,height,backPage);
    return myReference;
}

osg::ref_ptr<osg::Image> ImageReaderWriter::readImage_Archive(DataReference& dr, float& s,float& t)
{
    for(PhotoArchiveList::iterator itr=_photoArchiveList.begin();
        itr!=_photoArchiveList.end();
        ++itr)
    {
        osg::ref_ptr<osg::Image> image = (*itr)->readImage(dr._fileName,dr._resolutionX,dr._resolutionY,s,t);
        if (image) return image;
    }
    return 0;
}

osg::ref_ptr<osg::Image> ImageReaderWriter::readImage_DynamicSampling(DataReference& dr, float& s,float& t)
{
    // record previous options.
    osg::ref_ptr<osgDB::ReaderWriter::Options> previousOptions = osgDB::Registry::instance()->getOptions();

    osg::ref_ptr<osgDB::ImageOptions> options = new osgDB::ImageOptions;
    options->_destinationImageWindowMode = osgDB::ImageOptions::PIXEL_WINDOW;
    options->_destinationPixelWindow.set(0,0,dr._resolutionX,dr._resolutionY);

    osgDB::Registry::instance()->setOptions(options.get());

    osg::ref_ptr<osg::Image> image = osgDB::readRefImageFile(dr._fileName);

    // restore previous options.
    osgDB::Registry::instance()->setOptions(previousOptions.get());

    s = options.valid()?options->_sourcePixelWindow.windowWidth:1.0f;
    t = options.valid()?options->_sourcePixelWindow.windowHeight:1.0f;

    return image;

}


osgDB::ReaderWriter::ReadResult ImageReaderWriter::local_readNode(const std::string& fileName, const Options*)
{
    DataReferenceMap::iterator itr = _dataReferences.find(fileName);
    if (itr==_dataReferences.end()) return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;

    DataReference& dr = itr->second;

    osg::ref_ptr<osg::Image> image;
    float s=1.0f,t=1.0f;

    // try to load photo from any loaded PhotoArchives
    if (!_photoArchiveList.empty())
        image = readImage_Archive(dr,s,t);

    // not loaded yet, so try to load it directly.
    if (!image)
        image = readImage_DynamicSampling(dr,s,t);


    if (image)
    {


        float photoWidth = 0.0f;
        float photoHeight = 0.0f;
        float maxWidth = dr._maximumWidth.length();
        float maxHeight = dr._maximumHeight.length();


        if ((s/t)>(maxWidth/maxHeight))
        {
            // photo wider than tall relative to the required pictures size.
            // so need to clamp the width to the maximum width and then
            // set the height to keep the original photo aspect ratio.

            photoWidth = maxWidth;
            photoHeight = photoWidth*(t/s);
        }
        else
        {
            // photo tall than wide relative to the required pictures size.
            // so need to clamp the height to the maximum height and then
            // set the width to keep the original photo aspect ratio.

            photoHeight = maxHeight;
            photoWidth = photoHeight*(s/t);
        }

        photoWidth*=0.95;
        photoHeight*=0.95;

        osg::Vec3 halfWidthVector(dr._maximumWidth*(photoWidth*0.5f/maxWidth));
        osg::Vec3 halfHeightVector(dr._maximumHeight*(photoHeight*0.5f/maxHeight));


        // set up the texture.
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);
        texture->setResizeNonPowerOfTwoHint(false);
        texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
        texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);

        // set up the drawstate.
        osg::StateSet* dstate = new osg::StateSet;
        dstate->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
        dstate->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON);

        // set up the geoset.
        osg::Geometry* geom = new osg::Geometry;
        geom->setStateSet(dstate);

        osg::Vec3Array* coords = new osg::Vec3Array(4);

        if (!dr._backPage)
        {
            (*coords)[0] = dr._center - halfWidthVector + halfHeightVector;
            (*coords)[1] = dr._center - halfWidthVector - halfHeightVector;
            (*coords)[2] = dr._center + halfWidthVector - halfHeightVector;
            (*coords)[3] = dr._center + halfWidthVector + halfHeightVector;
        }
        else
        {
            (*coords)[3] = dr._center - halfWidthVector + halfHeightVector;
            (*coords)[2] = dr._center - halfWidthVector - halfHeightVector;
            (*coords)[1] = dr._center + halfWidthVector - halfHeightVector;
            (*coords)[0] = dr._center + halfWidthVector + halfHeightVector;
        }
        geom->setVertexArray(coords);

        osg::Vec2Array* tcoords = new osg::Vec2Array(4);
        (*tcoords)[0].set(0.0f,1.0f);
        (*tcoords)[1].set(0.0f,0.0f);
        (*tcoords)[2].set(1.0f,0.0f);
        (*tcoords)[3].set(1.0f,1.0f);
        geom->setTexCoordArray(0,tcoords);

        osg::Vec4Array* colours = new osg::Vec4Array(1);
        (*colours)[0].set(1.0f,1.0f,1.0,1.0f);
        geom->setColorArray(colours, osg::Array::BIND_OVERALL);

        geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));

        // set up the geode.
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(geom);

        return geode;

    }
    else
    {
        return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;
    }


}
