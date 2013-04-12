/* OpenSceneGraph example, osganimate.
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

#include <osg/Notify>
#include <osg/io_utils>
#include <osg/TextureRectangle>
#include <osg/TexMat>

#include <osgDB/ReadFile>
#include <osgGA/StateSetManipulator>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>


class Keystone : public osg::Referenced
{
public:
    Keystone():
        keystoneEditingEnabled(false),
        bottom_left(-1.0,-1.0),
        bottom_right(1.0,-1.0),
        top_left(-1.0,1.0),
        top_right(1.0,1.0) {}

    void reset()
    {
        bottom_left.set(-1.0,-1.0);
        bottom_right.set(1.0,-1.0);
        top_left.set(-1.0,1.0);
        top_right.set(1.0,1.0);
    }

    Keystone& operator = (const Keystone& rhs)
    {
        if (&rhs==this) return *this;
        keystoneEditingEnabled = rhs.keystoneEditingEnabled;
        bottom_left = rhs.bottom_left;
        bottom_right = rhs.bottom_right;
        top_left = rhs.top_left;
        top_right = rhs.top_right;
        return *this;
    }

    bool        keystoneEditingEnabled;

    osg::Vec2d  bottom_left;
    osg::Vec2d  bottom_right;
    osg::Vec2d  top_left;
    osg::Vec2d  top_right;

    void compute3DPositions(osg::DisplaySettings* ds, osg::Vec3& tl, osg::Vec3& tr, osg::Vec3& br, osg::Vec3& bl) const
    {
        double tr_x = ((top_right-bottom_right).length()) / ((top_left-bottom_left).length());
        double r_left = sqrt(tr_x);
        double r_right = r_left/tr_x;

        double tr_y = ((top_right-top_left).length()) / ((bottom_right-bottom_left).length());
        double r_bottom = sqrt(tr_y);
        double r_top = r_bottom/tr_y;

        double screenDistance = ds->getScreenDistance();
        double screenWidth = ds->getScreenWidth();
        double screenHeight = ds->getScreenHeight();

        tl = osg::Vec3(screenWidth*0.5*top_left.x(), screenHeight*0.5*top_left.y(), -screenDistance)*r_left*r_top;
        tr = osg::Vec3(screenWidth*0.5*top_right.x(), screenHeight*0.5*top_right.y(), -screenDistance)*r_right*r_top;
        br = osg::Vec3(screenWidth*0.5*bottom_right.x(), screenHeight*0.5*bottom_right.y(), -screenDistance)*r_right*r_bottom;
        bl = osg::Vec3(screenWidth*0.5*bottom_left.x(), screenHeight*0.5*bottom_left.y(), -screenDistance)*r_left*r_bottom;
    }
};



class KeystoneHandler : public osgGA::GUIEventHandler
{
public:

    KeystoneHandler(Keystone* keystone);

    ~KeystoneHandler() {}

    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa);

    void setKeystoneEditingEnabled(bool enabled) { if (_currentControlPoints.valid()) _currentControlPoints->keystoneEditingEnabled = enabled; }
    bool getKeystoneEditingEnabled() const { return _currentControlPoints.valid() ? _currentControlPoints->keystoneEditingEnabled : false; }

    enum Region
    {
        NONE_SELECTED,
        TOP_LEFT,
        TOP,
        TOP_RIGHT,
        RIGHT,
        BOTTOM_RIGHT,
        BOTTOM,
        BOTTOM_LEFT,
        LEFT,
        CENTER
    };

    osg::Vec2d incrementScale(const osgGA::GUIEventAdapter& ea) const;
    Region computeRegion(const osgGA::GUIEventAdapter& ea) const;
    void move(Region region, const osg::Vec2d& delta);
    
protected:


    osg::ref_ptr<Keystone>      _keystone;

    osg::Vec2d                  _defaultIncrement;
    osg::Vec2d                  _ctrlIncrement;
    osg::Vec2d                  _shiftIncrement;
    osg::Vec2d                  _keyIncrement;

    osg::Vec2d                  _startPosition;
    osg::ref_ptr<Keystone>      _startControlPoints;
    
    Region                      _selectedRegion;
    osg::ref_ptr<Keystone>      _currentControlPoints;

};

KeystoneHandler::KeystoneHandler(Keystone* keystone):
    _keystone(keystone),
    _defaultIncrement(0.0,0.0),
    _ctrlIncrement(1.0,1.0),
    _shiftIncrement(0.1,0.1),
    _keyIncrement(0.005, 0.005),
    _selectedRegion(NONE_SELECTED)
{
    _startControlPoints = new Keystone;
    _currentControlPoints = keystone; //new Keystone;
}

KeystoneHandler::Region KeystoneHandler::computeRegion(const osgGA::GUIEventAdapter& ea) const
{
    float x = ea.getXnormalized();
    float y = ea.getYnormalized();
    if (x<-0.33)
    {
        // left side
        if (y<-0.33) return BOTTOM_LEFT;
        else if (y<0.33) return LEFT;
        else return TOP_LEFT;
    }
    else if (x<0.33)
    {
        // center side
        if (y<-0.33) return BOTTOM;
        else if (y<0.33) return CENTER;
        else return TOP;
    }
    else
    {
        // right side
        if (y<-0.33) return BOTTOM_RIGHT;
        else if (y<0.33) return RIGHT;
        else return TOP_RIGHT;
    }
    return NONE_SELECTED;
}

void KeystoneHandler::move(Region region, const osg::Vec2d& delta)
{
    switch(region)
    {
        case(TOP_LEFT):
            _currentControlPoints->top_left += delta;
            break;
        case(TOP):
            _currentControlPoints->top_left += delta;
            _currentControlPoints->top_right += delta;
            break;
        case(TOP_RIGHT):
            _currentControlPoints->top_right += delta;
            break;
        case(RIGHT):
            _currentControlPoints->top_right += delta;
            _currentControlPoints->bottom_right += delta;
            break;
        case(BOTTOM_RIGHT):
            _currentControlPoints->bottom_right += delta;
            break;
        case(BOTTOM):
            _currentControlPoints->bottom_right += delta;
            _currentControlPoints->bottom_left += delta;
            break;
        case(BOTTOM_LEFT):
            _currentControlPoints->bottom_left += delta;
            break;
        case(LEFT):
            _currentControlPoints->bottom_left += delta;
            _currentControlPoints->top_left += delta;
            break;
        case(CENTER):
            _currentControlPoints->bottom_left += delta;
            _currentControlPoints->top_left += delta;
            _currentControlPoints->bottom_right += delta;
            _currentControlPoints->top_right += delta;
            break;
        case(NONE_SELECTED):
            break;
    }
}

osg::Vec2d KeystoneHandler::incrementScale(const osgGA::GUIEventAdapter& ea) const
{
    if (_ctrlIncrement!=osg::Vec2d(0.0,0.0) && (ea.getModKeyMask()==osgGA::GUIEventAdapter::MODKEY_LEFT_CTRL || ea.getModKeyMask()==osgGA::GUIEventAdapter::MODKEY_RIGHT_CTRL )) return _ctrlIncrement;
    if (_shiftIncrement!=osg::Vec2d(0.0,0.0) && (ea.getModKeyMask()==osgGA::GUIEventAdapter::MODKEY_LEFT_SHIFT || ea.getModKeyMask()==osgGA::GUIEventAdapter::MODKEY_RIGHT_SHIFT )) return _shiftIncrement;
    return _defaultIncrement;
}

bool KeystoneHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::PUSH):
        {
            if (getKeystoneEditingEnabled())
            {
                osg::Vec2d scale = incrementScale(ea);
                if (scale.length2()!=0.0)
                {
                    _selectedRegion = computeRegion(ea);
                    (*_startControlPoints) = (*_currentControlPoints);
                    _startPosition.set(ea.getXnormalized(),ea.getYnormalized());
                }
                else
                {
                    _selectedRegion = NONE_SELECTED;
                }
            }
            return false;
        }
        case(osgGA::GUIEventAdapter::DRAG):
        {
            if (getKeystoneEditingEnabled())
            {
                if (_selectedRegion!=NONE_SELECTED)
                {
                    (*_currentControlPoints) = (*_startControlPoints);
                    osg::Vec2d currentPosition(ea.getXnormalized(), ea.getYnormalized());
                    osg::Vec2d delta(currentPosition-_startPosition);
                    osg::Vec2d scale = incrementScale(ea);
                    move(_selectedRegion, osg::Vec2d(delta.x()*scale.x(), delta.y()*scale.y()) );
                    return true;
                }
            }
            return false;
        }
        case(osgGA::GUIEventAdapter::RELEASE):
        {
            if (getKeystoneEditingEnabled())
            {
                _selectedRegion = NONE_SELECTED;
            }
            return false;
        }
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (getKeystoneEditingEnabled())
            {
                if (ea.getUnmodifiedKey()=='g' && (ea.getModKeyMask()==osgGA::GUIEventAdapter::MODKEY_LEFT_CTRL || ea.getModKeyMask()==osgGA::GUIEventAdapter::MODKEY_RIGHT_CTRL))
                {
                    setKeystoneEditingEnabled(false);
                }
                else if (ea.getKey()=='r')
                {
                    _selectedRegion = NONE_SELECTED;
                    _startControlPoints->reset();
                    _currentControlPoints->reset();
                }
                else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Up)
                {
                    move(computeRegion(ea), osg::Vec2d(0.0, _keyIncrement.y()*incrementScale(ea).y()) );
                }
                else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Down)
                {
                    move(computeRegion(ea), osg::Vec2d(0.0, -_keyIncrement.y()*incrementScale(ea).y()) );
                }
                else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Left)
                {
                    move(computeRegion(ea), osg::Vec2d(-_keyIncrement.x()*incrementScale(ea).x(), 0.0) );
                }
                else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_Right)
                {
                    move(computeRegion(ea), osg::Vec2d(_keyIncrement.x()*incrementScale(ea).x(), 0.0) );
                }
                else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_7 || ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Home)
                {
                    _currentControlPoints->top_left.set(ea.getXnormalized(), ea.getYnormalized());
                }
                else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_9 || ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Page_Up)
                {
                    _currentControlPoints->top_right.set(ea.getXnormalized(), ea.getYnormalized());
                }
                else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_3 || ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Page_Down)
                {
                    _currentControlPoints->bottom_right.set(ea.getXnormalized(), ea.getYnormalized());
                }
                else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_1 || ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_End)
                {
                    _currentControlPoints->bottom_left.set(ea.getXnormalized(), ea.getYnormalized());
                }
            }
            else if (ea.getUnmodifiedKey()=='g' && (ea.getModKeyMask()==osgGA::GUIEventAdapter::MODKEY_LEFT_CTRL || ea.getModKeyMask()==osgGA::GUIEventAdapter::MODKEY_RIGHT_CTRL))
            {
                setKeystoneEditingEnabled(true);
            }
            return false;
        }
        default:
            return false;
    }
}
struct KeystoneCullCallback : public osg::Drawable::CullCallback
{
    KeystoneCullCallback(Keystone* keystone=0):_keystone(keystone) {}
    KeystoneCullCallback(const KeystoneCullCallback&, const osg::CopyOp&) {}

    META_Object(osg,KeystoneCullCallback);

    /** do customized cull code, return true if drawable should be culled.*/
    virtual bool cull(osg::NodeVisitor* nv, osg::Drawable* drawable, osg::RenderInfo* renderInfo) const
    {
        return _keystone.valid() ? !_keystone->keystoneEditingEnabled : true;
    }

    osg::ref_ptr<Keystone> _keystone;
};


struct KeystoneUpdateCallback : public osg::Drawable::UpdateCallback
{
    KeystoneUpdateCallback(Keystone* keystone=0):_keystone(keystone) {}
    KeystoneUpdateCallback(const KeystoneUpdateCallback&, const osg::CopyOp&) {}

    META_Object(osg,KeystoneUpdateCallback);

    /** do customized update code.*/
    virtual void update(osg::NodeVisitor*, osg::Drawable* drawable)
    {
        update(dynamic_cast<osg::Geometry*>(drawable));
    }

    void update(osg::Geometry* geometry)
    {
        if (!geometry) return;

        osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
        if (!vertices) return;

        osg::Vec2Array* texcoords = dynamic_cast<osg::Vec2Array*>(geometry->getTexCoordArray(0));
        if (!texcoords) return;

        osg::Vec3 tl, tr, br, bl;

        _keystone->compute3DPositions(osg::DisplaySettings::instance().get(), tl, tr, br, bl);

        for(unsigned int i=0; i<vertices->size(); ++i)
        {
            osg::Vec3& v = (*vertices)[i];
            osg::Vec2& t = (*texcoords)[i];
            v = bl * ((1.0f-t.x())*(1.0f-t.y())) +
                br * ((t.x())*(1.0f-t.y())) +
                tl * ((1.0f-t.x())*(t.y())) +
                tr * ((t.x())*(t.y()));
        }
        geometry->dirtyBound();
    }
    
    osg::ref_ptr<Keystone> _keystone;
};



osg::Geode* createKeystoneDistortionMesh(Keystone* keystone)
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geode->addDrawable(geometry.get());

    geometry->setUseDisplayList(false);

    osg::ref_ptr<KeystoneUpdateCallback> kuc = new KeystoneUpdateCallback(keystone);
    geometry->setUpdateCallback(kuc.get());

    osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array;
    colours->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    geometry->setColorArray(colours.get());
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices.get());

    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
    geometry->setTexCoordArray(0, texcoords.get());

    unsigned int numRows = 7;
    unsigned int numColumns = 7;
    unsigned int numVertices = numRows*numColumns;

    vertices->resize(numVertices);
    texcoords->resize(numVertices);

    for(unsigned j=0; j<numRows; j++)
    {
        for(unsigned i=0; i<numColumns; i++)
        {
            osg::Vec2& t = (*texcoords)[j*numColumns+i];
            t.set(static_cast<float>(i)/static_cast<float>(numColumns-1), static_cast<float>(j)/static_cast<float>(numRows-1));
        }
    }

    osg::ref_ptr<osg::DrawElementsUShort> elements = new osg::DrawElementsUShort(GL_TRIANGLES);
    geometry->addPrimitiveSet(elements.get());
    for(unsigned j=0; j<numRows-1; j++)
    {
        for(unsigned i=0; i<numColumns-1; i++)
        {
            unsigned int vi = j*numColumns+i;
            
            elements->push_back(vi+numColumns);
            elements->push_back(vi);
            elements->push_back(vi+1);

            elements->push_back(vi+numColumns);
            elements->push_back(vi+1);
            elements->push_back(vi+1+numColumns);
        }
    }
    
    geometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    geometry->getOrCreateStateSet()->setRenderBinDetails(0, "RenderBin");

    kuc->update(geometry.get());

    return geode.release();
}

osg::Node* createGrid(Keystone* keystone, const osg::Vec4& colour)
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;

    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geode->addDrawable(geometry.get());

    geometry->setUseDisplayList(false);

    osg::ref_ptr<KeystoneUpdateCallback> kuc = new KeystoneUpdateCallback(keystone);
    geometry->setUpdateCallback(kuc.get());

    geometry->setCullCallback(new KeystoneCullCallback(keystone));

    osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array;
    colours->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    geometry->setColorArray(colours.get());
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices.get());

    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
    geometry->setTexCoordArray(0, texcoords.get());

    osg::Vec2 origin(0.0f, 0.0f);
    osg::Vec2 widthVector(1.0f, 0.0f);
    osg::Vec2 heightVector(0.0f, 1.0f);

    unsigned int numIntervals = 7;
    
    // border line
    {
        unsigned int vi = texcoords->size();
        texcoords->push_back(origin);
        texcoords->push_back(origin+widthVector);
        texcoords->push_back(origin+widthVector+heightVector);
        texcoords->push_back(origin+heightVector);
        geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_LOOP, vi, 4));
    }

    // cross lines
    {
        unsigned int vi = texcoords->size();
        osg::Vec2 v = origin;
        osg::Vec2 dv = (widthVector+heightVector)/static_cast<float>(numIntervals-1);
        for(unsigned int i=0; i<numIntervals; ++i)
        {
            texcoords->push_back(v);
            v += dv;
        }
        geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, vi, numIntervals));

        vi = texcoords->size();
        v = origin+heightVector;
        dv = (widthVector-heightVector)/static_cast<float>(numIntervals-1);
        for(unsigned int i=0; i<numIntervals; ++i)
        {
            texcoords->push_back(v);
            v += dv;
        }
        geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, vi, numIntervals));
    }

    // vertices lines
    {
        unsigned int vi = texcoords->size();
        osg::Vec2 dv = widthVector/6.0;
        osg::Vec2 bv = origin+dv;
        osg::Vec2 tv = bv+heightVector;
        for(unsigned int i=0; i<5; ++i)
        {
            texcoords->push_back(bv);
            texcoords->push_back(tv);
            bv += dv;
            tv += dv;
        }
        geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINES, vi, 10));
    }

    // horizontal lines
    {
        unsigned int vi = texcoords->size();
        osg::Vec2 dv = heightVector/6.0;
        osg::Vec2 bv = origin+dv;
        osg::Vec2 tv = bv+widthVector;
        for(unsigned int i=0; i<5; ++i)
        {
            texcoords->push_back(bv);
            texcoords->push_back(tv);
            bv += dv;
            tv += dv;
        }
        geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINES, vi, 10));
    }

    vertices->resize(texcoords->size());

    geometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    geometry->getOrCreateStateSet()->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    geometry->getOrCreateStateSet()->setRenderBinDetails(1, "RenderBin");

    kuc->update(geometry.get());

    return geode.release();
}


void setUpViewForKeystone(osgViewer::View* view, Keystone* keystone)
{
    int screenNum = 0;
    
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
    {
        OSG_NOTICE<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }

    osg::GraphicsContext::ScreenIdentifier si;
    si.readDISPLAY();

    // displayNum has not been set so reset it to 0.
    if (si.displayNum<0) si.displayNum = 0;

    si.screenNum = screenNum;

    unsigned int width, height;
    wsi->getScreenResolution(si, width, height);

//    width/=2; height/=2;

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->hostName = si.hostName;
    traits->displayNum = si.displayNum;
    traits->screenNum = si.screenNum;
    traits->x = 0;
    traits->y = 0;
    traits->width = width;
    traits->height = height;
    traits->windowDecoration = false;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;


    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    if (!gc)
    {
        OSG_NOTICE<<"GraphicsWindow has not been created successfully."<<std::endl;
        return;
    }

    int tex_width = width;
    int tex_height = height;

    int camera_width = tex_width;
    int camera_height = tex_height;

    osg::TextureRectangle* texture = new osg::TextureRectangle;

    texture->setTextureSize(tex_width, tex_height);
    texture->setInternalFormat(GL_RGB);
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_EDGE);

#if 0
    osg::Camera::RenderTargetImplementation renderTargetImplementation = osg::Camera::SEPERATE_WINDOW;
    GLenum buffer = GL_FRONT;
#else
    osg::Camera::RenderTargetImplementation renderTargetImplementation = osg::Camera::FRAME_BUFFER_OBJECT;
    GLenum buffer = GL_FRONT;
#endif

    // front face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName("Render to texture camera");
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(0,0,camera_width, camera_height));
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setAllowEventFocus(false);
        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderTargetImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture);

        view->addSlave(camera.get(), osg::Matrixd(), osg::Matrixd());
    }

    // distortion correction set up.
    {

        double screenDistance = osg::DisplaySettings::instance()->getScreenDistance();
        double screenWidth = osg::DisplaySettings::instance()->getScreenWidth();
        double screenHeight = osg::DisplaySettings::instance()->getScreenHeight();
        double fovy = osg::RadiansToDegrees(2.0*atan2(screenHeight/2.0,screenDistance));
        double aspectRatio = screenWidth/screenHeight;

        osg::Geode* geode = createKeystoneDistortionMesh(keystone);

        // new we need to add the texture to the mesh, we do so by creating a
        // StateSet to contain the Texture StateAttribute.
        osg::StateSet* stateset = geode->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON);
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

        osg::TexMat* texmat = new osg::TexMat;
        texmat->setScaleByTextureRectangleSize(true);
        stateset->setTextureAttributeAndModes(0, texmat, osg::StateAttribute::ON);

        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(gc.get());
        camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
        camera->setClearColor( osg::Vec4(0.0,0.0,0.0,1.0) );
        camera->setViewport(new osg::Viewport(0, 0, width, height));
        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        camera->setAllowEventFocus(false);
        camera->setInheritanceMask(camera->getInheritanceMask() & ~osg::CullSettings::CLEAR_COLOR & ~osg::CullSettings::COMPUTE_NEAR_FAR_MODE);
        //camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

        camera->setViewMatrix(osg::Matrix::identity());
        camera->setProjectionMatrixAsPerspective(fovy, aspectRatio, 0.1, 1000.0);

        // add subgraph to render
        camera->addChild(geode);

        camera->addChild(createGrid(keystone, osg::Vec4(1.0,1.0,1.0,1.0)));

        camera->setName("DistortionCorrectionCamera");

        camera->addEventCallback(new KeystoneHandler(keystone));

        view->addSlave(camera.get(), osg::Matrixd(), osg::Matrixd(), false);
    }
}

int main( int argc, char **argv )
{
    osg::ArgumentParser arguments(&argc,argv);
    
    // initialize the viewer.
    osgViewer::Viewer viewer(arguments);

    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles(arguments);

    if (!model)
    {
        OSG_NOTICE<<"No models loaded, please specify a model file on the command line"<<std::endl;
        return 1;
    }

    viewer.setSceneData(model.get());
    
    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    // add camera manipulator
    viewer.setCameraManipulator(new osgGA::TrackballManipulator());

    setUpViewForKeystone(&viewer, new Keystone);
    
    viewer.realize();

    while(!viewer.done())
    {
        viewer.advance();
        viewer.eventTraversal();
        viewer.updateTraversal();
        viewer.renderingTraversals();
    }
    return 0;
}
