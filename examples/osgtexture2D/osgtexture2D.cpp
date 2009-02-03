/* OpenSceneGraph example, osgtexture2D.
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

#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/DrawPixels>
#include <osg/PolygonOffset>
#include <osg/Geode>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgText/Text>

#include <osgViewer/Viewer>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// filter wall and animation callback.
//

class FilterCallback : public osg::NodeCallback
{
public:

    FilterCallback(osg::Texture2D* texture,osgText::Text* text,double delay=1.0):
        _texture(texture),
        _text(text),
        _delay(delay),
        _currPos(0),
        _prevTime(0.0)
    {
        // start with a mip mapped mode to ensure that
        _minFilterList.push_back(osg::Texture2D::LINEAR_MIPMAP_LINEAR);
        _magFilterList.push_back(osg::Texture2D::LINEAR);
        _textList.push_back("Tri-linear mip mapping (default filtering)\nsetFilter(MIN_FILTER,LINEAR_MIP_LINEAR)\nsetFilter(MAG_FILTER,LINEAR)");

        _minFilterList.push_back(osg::Texture2D::NEAREST);
        _magFilterList.push_back(osg::Texture2D::NEAREST);
        _textList.push_back("Nearest filtering\nsetFilter(MIN_FILTER,NEAREST)\nsetFilter(MAG_FILTER,NEAREST)");
        
        _minFilterList.push_back(osg::Texture2D::LINEAR);
        _magFilterList.push_back(osg::Texture2D::LINEAR);
        _textList.push_back("Linear filtering\nsetFilter(MIN_FILTER,LINEAR)\nsetFilter(MAG_FILTER,LINEAR)");
        
        _minFilterList.push_back(osg::Texture2D::NEAREST_MIPMAP_NEAREST);
        _magFilterList.push_back(osg::Texture2D::LINEAR);
        _textList.push_back("nearest mip mapping (default filtering)\nsetFilter(MIN_FILTER,)\nsetFilter(MAG_FILTER,LINEAR)");

        _minFilterList.push_back(osg::Texture2D::LINEAR_MIPMAP_NEAREST);
        _magFilterList.push_back(osg::Texture2D::LINEAR);
        _textList.push_back("bi-linear mip mapping\nsetFilter(MIN_FILTER,LINEAR_MIPMAP_NEAREST)\nsetFilter(MAG_FILTER,LINEAR)");

        _minFilterList.push_back(osg::Texture2D::NEAREST_MIPMAP_LINEAR);
        _magFilterList.push_back(osg::Texture2D::LINEAR);
        _textList.push_back("bi-linear mip mapping\nsetFilter(MIN_FILTER,NEAREST_MIPMAP_LINEAR)\nsetFilter(MAG_FILTER,LINEAR)");

        
        setValues();
    }

    virtual void operator()(osg::Node*, osg::NodeVisitor* nv)
    {
        if (nv->getFrameStamp())
        {
            double currTime = nv->getFrameStamp()->getSimulationTime();
            if (currTime-_prevTime>_delay) 
            {
                // update filter modes and text.
                setValues();
                
                // advance the current positon, wrap round if required.
                _currPos++;
                if (_currPos>=_minFilterList.size()) _currPos=0;                    
                
                // record time
                _prevTime = currTime;
            }
        }
    }
    
    void setValues()
    {
        _texture->setFilter(osg::Texture2D::MIN_FILTER,_minFilterList[_currPos]);
        _texture->setFilter(osg::Texture2D::MAG_FILTER,_magFilterList[_currPos]);

        _text->setText(_textList[_currPos]);
    }

protected:

    typedef std::vector<osg::Texture2D::FilterMode> FilterList;
    typedef std::vector<std::string>                TextList;

    osg::ref_ptr<osg::Texture2D>    _texture;
    osg::ref_ptr<osgText::Text>     _text;
    double                          _delay;
    
    FilterList                      _minFilterList;
    FilterList                      _magFilterList;
    TextList                        _textList;
    
    unsigned int                    _currPos;
    double                          _prevTime;
    
};

osg::Node* createFilterWall(osg::BoundingBox& bb,const std::string& filename)
{
    osg::Group* group = new osg::Group;
    
    // left hand side of bounding box.
    osg::Vec3 top_left(bb.xMin(),bb.yMin(),bb.zMax());
    osg::Vec3 bottom_left(bb.xMin(),bb.yMin(),bb.zMin());
    osg::Vec3 bottom_right(bb.xMin(),bb.yMax(),bb.zMin());
    osg::Vec3 top_right(bb.xMin(),bb.yMax(),bb.zMax());
    osg::Vec3 center(bb.xMin(),(bb.yMin()+bb.yMax())*0.5f,(bb.zMin()+bb.zMax())*0.5f);    
    float height = bb.zMax()-bb.zMin();
    
    // create the geometry for the wall.
    osg::Geometry* geom = new osg::Geometry;
    
    osg::Vec3Array* vertices = new osg::Vec3Array(4);
    (*vertices)[0] = top_left;
    (*vertices)[1] = bottom_left;
    (*vertices)[2] = bottom_right;
    (*vertices)[3] = top_right;
    geom->setVertexArray(vertices);
    
    osg::Vec2Array* texcoords = new osg::Vec2Array(4);
    (*texcoords)[0].set(0.0f,1.0f);
    (*texcoords)[1].set(0.0f,0.0f);
    (*texcoords)[2].set(1.0f,0.0f);
    (*texcoords)[3].set(1.0f,1.0f);
    geom->setTexCoordArray(0,texcoords);

    osg::Vec3Array* normals = new osg::Vec3Array(1);
    (*normals)[0].set(1.0f,0.0f,0.0f);
    geom->setNormalArray(normals);
    geom->setNormalBinding(osg::Geometry::BIND_OVERALL);
    
    osg::Vec4Array* colors = new osg::Vec4Array(1);
    (*colors)[0].set(1.0f,1.0f,1.0f,1.0f);
    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);

    geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));
    
    osg::Geode* geom_geode = new osg::Geode;
    geom_geode->addDrawable(geom);
    group->addChild(geom_geode);
    
    
    // set up the texture state.    
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setDataVariance(osg::Object::DYNAMIC); // protect from being optimized away as static state.
    texture->setImage(osgDB::readImageFile(filename));
    
    osg::StateSet* stateset = geom->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    
    // create the text label.
    
    osgText::Text* text = new osgText::Text;
    text->setFont("fonts/arial.ttf");
    text->setPosition(center);
    text->setCharacterSize(height*0.03f);
    text->setAlignment(osgText::Text::CENTER_CENTER);
    text->setAxisAlignment(osgText::Text::YZ_PLANE);
    
    osg::Geode* text_geode = new osg::Geode;
    text_geode->addDrawable(text);
    
    osg::StateSet* text_stateset = text_geode->getOrCreateStateSet();
    text_stateset->setAttributeAndModes(new osg::PolygonOffset(-1.0f,-1.0f),osg::StateAttribute::ON);
    
    group->addChild(text_geode);

    // set the update callback to cycle through the various min and mag filter modes.
    group->setUpdateCallback(new FilterCallback(texture,text));
    
    return group;
    
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// anisotropic wall and animation callback.
//

class AnisotropicCallback : public osg::NodeCallback
{
public:

    AnisotropicCallback(osg::Texture2D* texture,osgText::Text* text,double delay=1.0):
        _texture(texture),
        _text(text),
        _delay(delay),
        _currPos(0),
        _prevTime(0.0)
    {

        _maxAnisotropyList.push_back(1.0f);
        _textList.push_back("No anisotropic filtering (default)\nsetMaxAnisotropy(1.0f)");
        
        _maxAnisotropyList.push_back(2.0f);
        _textList.push_back("Anisotropic filtering\nsetMaxAnisotropy(2.0f)");
        
        _maxAnisotropyList.push_back(4.0f);
        _textList.push_back("Anisotropic filtering\nsetMaxAnisotropy(4.0f)");
        
        _maxAnisotropyList.push_back(8.0f);
        _textList.push_back("Anisotropic filtering\nsetMaxAnisotropy(8.0f)");
        
        _maxAnisotropyList.push_back(16.0f);
        _textList.push_back("Higest quality anisotropic filtering\nsetMaxAnisotropy(16.0f)");

        setValues();
    }

    virtual void operator()(osg::Node*, osg::NodeVisitor* nv)
    {
        if (nv->getFrameStamp())
        {
            double currTime = nv->getFrameStamp()->getSimulationTime();
            if (currTime-_prevTime>_delay) 
            {
                // update filter modes and text.
                setValues();
                
                // advance the current positon, wrap round if required.
                _currPos++;
                if (_currPos>=_maxAnisotropyList.size()) _currPos=0;                    
                
                // record time
                _prevTime = currTime;
            }
        }
    }
    
    void setValues()
    {
        _texture->setMaxAnisotropy(_maxAnisotropyList[_currPos]);

        _text->setText(_textList[_currPos]);
    }

protected:

    typedef std::vector<float>          AnisotropyList;
    typedef std::vector<std::string>    TextList;

    osg::ref_ptr<osg::Texture2D>    _texture;
    osg::ref_ptr<osgText::Text>     _text;
    double                          _delay;
    
    AnisotropyList                  _maxAnisotropyList;
    TextList                        _textList;
    
    unsigned int                    _currPos;
    double                          _prevTime;
    
};

osg::Node* createAnisotripicWall(osg::BoundingBox& bb,const std::string& filename)
{
    osg::Group* group = new osg::Group;
    
    // left hand side of bounding box.
    osg::Vec3 top_left(bb.xMin(),bb.yMax(),bb.zMin());
    osg::Vec3 bottom_left(bb.xMin(),bb.yMin(),bb.zMin());
    osg::Vec3 bottom_right(bb.xMax(),bb.yMin(),bb.zMin());
    osg::Vec3 top_right(bb.xMax(),bb.yMax(),bb.zMin());
    osg::Vec3 center((bb.xMin()+bb.xMax())*0.5f,(bb.yMin()+bb.yMax())*0.5f,bb.zMin());    
    float height = bb.yMax()-bb.yMin();
    
    // create the geometry for the wall.
    osg::Geometry* geom = new osg::Geometry;
    
    osg::Vec3Array* vertices = new osg::Vec3Array(4);
    (*vertices)[0] = top_left;
    (*vertices)[1] = bottom_left;
    (*vertices)[2] = bottom_right;
    (*vertices)[3] = top_right;
    geom->setVertexArray(vertices);
    
    osg::Vec2Array* texcoords = new osg::Vec2Array(4);
    (*texcoords)[0].set(0.0f,1.0f);
    (*texcoords)[1].set(0.0f,0.0f);
    (*texcoords)[2].set(1.0f,0.0f);
    (*texcoords)[3].set(1.0f,1.0f);
    geom->setTexCoordArray(0,texcoords);

    osg::Vec3Array* normals = new osg::Vec3Array(1);
    (*normals)[0].set(0.0f,0.0f,1.0f);
    geom->setNormalArray(normals);
    geom->setNormalBinding(osg::Geometry::BIND_OVERALL);
    
    osg::Vec4Array* colors = new osg::Vec4Array(1);
    (*colors)[0].set(1.0f,1.0f,1.0f,1.0f);
    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);

    geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));
    
    osg::Geode* geom_geode = new osg::Geode;
    geom_geode->addDrawable(geom);
    group->addChild(geom_geode);
    
    
    // set up the texture state.    
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setDataVariance(osg::Object::DYNAMIC); // protect from being optimized away as static state.
    texture->setImage(osgDB::readImageFile(filename));
    
    osg::StateSet* stateset = geom->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    
    // create the text label.
    
    osgText::Text* text = new osgText::Text;
    text->setFont("fonts/arial.ttf");
    text->setPosition(center);
    text->setCharacterSize(height*0.03f);
    text->setColor(osg::Vec4(1.0f,0.0f,1.0f,1.0f));
    text->setAlignment(osgText::Text::CENTER_CENTER);
    text->setAxisAlignment(osgText::Text::XY_PLANE);
    
    osg::Geode* text_geode = new osg::Geode;
    text_geode->addDrawable(text);
    
    osg::StateSet* text_stateset = text_geode->getOrCreateStateSet();
    text_stateset->setAttributeAndModes(new osg::PolygonOffset(-1.0f,-1.0f),osg::StateAttribute::ON);
    
    group->addChild(text_geode);

    // set the update callback to cycle through the various min and mag filter modes.
    group->setUpdateCallback(new AnisotropicCallback(texture,text));
    
    return group;
    
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// wrap wall and animation callback.
//
class WrapCallback : public osg::NodeCallback
{
public:

    WrapCallback(osg::Texture2D* texture,osgText::Text* text,double delay=1.0):
        _texture(texture),
        _text(text),
        _delay(delay),
        _currPos(0),
        _prevTime(0.0)
    {

        _wrapList.push_back(osg::Texture2D::CLAMP);
        _textList.push_back("Default tex coord clamp\nsetWrap(WRAP_S,CLAMP)");
        
        _wrapList.push_back(osg::Texture2D::CLAMP_TO_EDGE);
        _textList.push_back("Clamp to edge extension\nsetWrap(WRAP_S,CLAMP_TO_EDGE)");
        
        _wrapList.push_back(osg::Texture2D::CLAMP_TO_BORDER);
        _textList.push_back("Clamp to border color extension\nsetWrap(WRAP_S,CLAMP_TO_BORDER)");
        
        _wrapList.push_back(osg::Texture2D::REPEAT);
        _textList.push_back("Repeat wrap\nsetWrap(WRAP_S,REPEAT)");
        
        _wrapList.push_back(osg::Texture2D::MIRROR);
        _textList.push_back("Mirror wrap extension\nsetWrap(WRAP_S,MIRROR)");

        setValues();
    }

    virtual void operator()(osg::Node*, osg::NodeVisitor* nv)
    {
        if (nv->getFrameStamp())
        {
            double currTime = nv->getFrameStamp()->getSimulationTime();
            if (currTime-_prevTime>_delay) 
            {
                // update filter modes and text.
                setValues();
                
                // advance the current positon, wrap round if required.
                _currPos++;
                if (_currPos>=_wrapList.size()) _currPos=0;                    
                
                // record time
                _prevTime = currTime;
            }
        }
    }
    
    void setValues()
    {
        _texture->setWrap(osg::Texture2D::WRAP_S,_wrapList[_currPos]);
        _texture->setWrap(osg::Texture2D::WRAP_T,_wrapList[_currPos]);

        _text->setText(_textList[_currPos]);
    }

protected:

    typedef std::vector<osg::Texture2D::WrapMode> WrapList;
    typedef std::vector<std::string>              TextList;

    osg::ref_ptr<osg::Texture2D>    _texture;
    osg::ref_ptr<osgText::Text>     _text;
    double                          _delay;
    
    WrapList                        _wrapList;
    TextList                        _textList;
    
    unsigned int                    _currPos;
    double                          _prevTime;
    
};

osg::Node* createWrapWall(osg::BoundingBox& bb,const std::string& filename)
{
    osg::Group* group = new osg::Group;
    
    // left hand side of bounding box.
    osg::Vec3 top_left(bb.xMax(),bb.yMax(),bb.zMax());
    osg::Vec3 bottom_left(bb.xMax(),bb.yMax(),bb.zMin());
    osg::Vec3 bottom_right(bb.xMax(),bb.yMin(),bb.zMin());
    osg::Vec3 top_right(bb.xMax(),bb.yMin(),bb.zMax());
    osg::Vec3 center(bb.xMax(),(bb.yMin()+bb.yMax())*0.5f,(bb.zMin()+bb.zMax())*0.5f);    
    float height = bb.zMax()-bb.zMin();
    
    // create the geometry for the wall.
    osg::Geometry* geom = new osg::Geometry;
    
    osg::Vec3Array* vertices = new osg::Vec3Array(4);
    (*vertices)[0] = top_left;
    (*vertices)[1] = bottom_left;
    (*vertices)[2] = bottom_right;
    (*vertices)[3] = top_right;
    geom->setVertexArray(vertices);
    
    osg::Vec2Array* texcoords = new osg::Vec2Array(4);
    (*texcoords)[0].set(-1.0f,2.0f);
    (*texcoords)[1].set(-1.0f,-1.0f);
    (*texcoords)[2].set(2.0f,-1.0f);
    (*texcoords)[3].set(2.0f,2.0f);
    geom->setTexCoordArray(0,texcoords);

    osg::Vec3Array* normals = new osg::Vec3Array(1);
    (*normals)[0].set(-1.0f,0.0f,0.0f);
    geom->setNormalArray(normals);
    geom->setNormalBinding(osg::Geometry::BIND_OVERALL);
    
    osg::Vec4Array* colors = new osg::Vec4Array(1);
    (*colors)[0].set(1.0f,1.0f,1.0f,1.0f);
    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);

    geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));
    
    osg::Geode* geom_geode = new osg::Geode;
    geom_geode->addDrawable(geom);
    group->addChild(geom_geode);
    
    
    // set up the texture state.    
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setDataVariance(osg::Object::DYNAMIC); // protect from being optimized away as static state.
    texture->setBorderColor(osg::Vec4(1.0f,1.0f,1.0f,0.5f)); // only used when wrap is set to CLAMP_TO_BORDER
    texture->setImage(osgDB::readImageFile(filename));
    
    osg::StateSet* stateset = geom->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
    
    // create the text label.
    
    osgText::Text* text = new osgText::Text;
    text->setFont("fonts/arial.ttf");
    text->setPosition(center);
    text->setCharacterSize(height*0.03f);
    text->setAlignment(osgText::Text::CENTER_CENTER);
    text->setAxisAlignment(osgText::Text::YZ_PLANE);
    
    osg::Geode* text_geode = new osg::Geode;
    text_geode->addDrawable(text);
    
    osg::StateSet* text_stateset = text_geode->getOrCreateStateSet();
    text_stateset->setAttributeAndModes(new osg::PolygonOffset(-1.0f,-1.0f),osg::StateAttribute::ON);
    
    group->addChild(text_geode);

    // set the update callback to cycle through the various min and mag filter modes.
    group->setUpdateCallback(new WrapCallback(texture,text));
    
    return group;
    
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// sublooad wall and animation callback.
//

class ImageUpdateCallback : public osg::NodeCallback
{
public:

    ImageUpdateCallback(osg::Texture2D* texture,osgText::Text* text,double delay=1.0):
        _texture(texture),
        _text(text),
        _delay(delay),
        _currPos(0),
        _prevTime(0.0)
    {

        
        _imageList.push_back(osgDB::readImageFile("Images/dog_left_eye.jpg"));
        _textList.push_back("Subloaded Image 1 - dog_left_eye.jpg");
        
        _imageList.push_back(osgDB::readImageFile("Images/dog_right_eye.jpg"));
        _textList.push_back("Subloaded Image 2 - dog_right_eye.jpg");
        
        setValues();
    }

    virtual void operator()(osg::Node*, osg::NodeVisitor* nv)
    {
        if (nv->getFrameStamp())
        {
            double currTime = nv->getFrameStamp()->getSimulationTime();
            if (currTime-_prevTime>_delay) 
            {
                // update filter modes and text.
                setValues();
                
                // advance the current positon, wrap round if required.
                _currPos++;
                if (_currPos>=_imageList.size()) _currPos=0;                    
                
                // record time
                _prevTime = currTime;
            }
        }
    }
    
    void setValues()
    {
        // Note, as long as the images are the same dimensions subloading will be used
        // to update the textures.  If dimensions change then the texture objects have
        // to be deleted and re-recreated.
        //
        // The load/subload happens during the draw traversal so doesn't happen on
        // the setImage which just updates internal pointers and modifed flags.

        _texture->setImage(_imageList[_currPos].get());
        
        //_texture->dirtyTextureObject();

        _text->setText(_textList[_currPos]);
    }

protected:

    typedef std::vector< osg::ref_ptr<osg::Image> > ImageList;
    typedef std::vector<std::string>                TextList;

    osg::ref_ptr<osg::Texture2D>    _texture;
    osg::ref_ptr<osgText::Text>     _text;
    double                          _delay;
    
    ImageList                       _imageList;
    TextList                        _textList;
    
    unsigned int                    _currPos;
    double                          _prevTime;
    
};

osg::Node* createSubloadWall(osg::BoundingBox& bb)
{
    osg::Group* group = new osg::Group;
    
    // left hand side of bounding box.
    osg::Vec3 top_left(bb.xMin(),bb.yMax(),bb.zMax());
    osg::Vec3 bottom_left(bb.xMin(),bb.yMax(),bb.zMin());
    osg::Vec3 bottom_right(bb.xMax(),bb.yMax(),bb.zMin());
    osg::Vec3 top_right(bb.xMax(),bb.yMax(),bb.zMax());
    osg::Vec3 center((bb.xMax()+bb.xMin())*0.5f,bb.yMax(),(bb.zMin()+bb.zMax())*0.5f);    
    float height = bb.zMax()-bb.zMin();
    
    // create the geometry for the wall.
    osg::Geometry* geom = new osg::Geometry;
    
    osg::Vec3Array* vertices = new osg::Vec3Array(4);
    (*vertices)[0] = top_left;
    (*vertices)[1] = bottom_left;
    (*vertices)[2] = bottom_right;
    (*vertices)[3] = top_right;
    geom->setVertexArray(vertices);
    
    osg::Vec2Array* texcoords = new osg::Vec2Array(4);
    (*texcoords)[0].set(0.0f,1.0f);
    (*texcoords)[1].set(0.0f,0.0f);
    (*texcoords)[2].set(1.0f,0.0f);
    (*texcoords)[3].set(1.0f,1.0f);
    geom->setTexCoordArray(0,texcoords);

    osg::Vec3Array* normals = new osg::Vec3Array(1);
    (*normals)[0].set(0.0f,-1.0f,0.0f);
    geom->setNormalArray(normals);
    geom->setNormalBinding(osg::Geometry::BIND_OVERALL);
    
    osg::Vec4Array* colors = new osg::Vec4Array(1);
    (*colors)[0].set(1.0f,1.0f,1.0f,1.0f);
    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);

    geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));
    
    osg::Geode* geom_geode = new osg::Geode;
    geom_geode->addDrawable(geom);
    group->addChild(geom_geode);
    
    
    // set up the texture state.    
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setDataVariance(osg::Object::DYNAMIC); // protect from being optimized away as static state.
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    
    osg::StateSet* stateset = geom->getOrCreateStateSet();
    stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    
    // create the text label.
    
    osgText::Text* text = new osgText::Text;
    text->setDataVariance(osg::Object::DYNAMIC);
    text->setFont("fonts/arial.ttf");
    text->setPosition(center);
    text->setCharacterSize(height*0.03f);
    text->setAlignment(osgText::Text::CENTER_CENTER);
    text->setAxisAlignment(osgText::Text::XZ_PLANE);
    
    osg::Geode* text_geode = new osg::Geode;
    text_geode->addDrawable(text);
    
    osg::StateSet* text_stateset = text_geode->getOrCreateStateSet();
    text_stateset->setAttributeAndModes(new osg::PolygonOffset(-1.0f,-1.0f),osg::StateAttribute::ON);
    
    group->addChild(text_geode);

    // set the update callback to cycle through the various min and mag filter modes.
    group->setUpdateCallback(new ImageUpdateCallback(texture,text));
    
    return group;
    
}


osg::Node* createModel()
{

    // create the root node which will hold the model.
    osg::Group* root = new osg::Group();
    
    // turn off lighting 
    root->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    osg::BoundingBox bb(0.0f,0.0f,0.0f,1.0f,1.0f,1.0f);

    root->addChild(createFilterWall(bb,"Images/lz.rgb"));
    root->addChild(createAnisotripicWall(bb,"Images/primitives.gif"));
    root->addChild(createWrapWall(bb,"Images/tree0.rgba"));
    root->addChild(createSubloadWall(bb));
    
    return root;
}

int main(int , char **)
{
    // construct the viewer.
    osgViewer::Viewer viewer;

    // add model to viewer.
    viewer.setSceneData( createModel() );

    return viewer.run();
}
