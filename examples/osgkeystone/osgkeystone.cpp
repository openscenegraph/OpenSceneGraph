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
#include <osg/Stencil>
#include <osg/PolygonStipple>

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
        gridColour(1.0f,1.0f,1.0f,1.0f),
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
        gridColour = rhs.gridColour;
        bottom_left = rhs.bottom_left;
        bottom_right = rhs.bottom_right;
        top_left = rhs.top_left;
        top_right = rhs.top_right;
        return *this;
    }

    bool        keystoneEditingEnabled;

    osg::Vec4   gridColour;

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

    bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object* obj, osg::NodeVisitor* nv);

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

bool KeystoneHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object* obj, osg::NodeVisitor* nv)
{
    osg::Camera* camera = dynamic_cast<osg::Camera*>(obj);
    osg::Viewport* viewport = camera ?  camera->getViewport() : 0;

    if (!viewport) return false;

    bool haveCameraMatch = false;
    float x = ea.getXnormalized();
    float y = ea.getYnormalized();
    for(unsigned int i=0; i<ea.getNumPointerData(); ++i)
    {
        const osgGA::PointerData* pd = ea.getPointerData(i);
        if (pd->object==obj)
        {
            haveCameraMatch = true;
            x = pd->getXnormalized();
            y = pd->getYnormalized();
            break;
        }
    }


    if (!haveCameraMatch) return false;

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
                    _startPosition.set(x,y);
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
                    osg::Vec2d currentPosition(x, y);
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
                    _currentControlPoints->top_left.set(x, y);
                }
                else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_9 || ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Page_Up)
                {
                    _currentControlPoints->top_right.set(x, y);
                }
                else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_3 || ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_Page_Down)
                {
                    _currentControlPoints->bottom_right.set(x, y);
                }
                else if (ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_1 || ea.getKey()==osgGA::GUIEventAdapter::KEY_KP_End)
                {
                    _currentControlPoints->bottom_left.set(x, y);
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
    colours->push_back(keystone->gridColour);
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

osg::Texture* createKestoneDistortionTexture(int width, int height)
{
    osg::ref_ptr<osg::TextureRectangle> texture = new osg::TextureRectangle;

    texture->setTextureSize(width, height);
    texture->setInternalFormat(GL_RGB);
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_EDGE);

    return texture.release();
}

osg::Camera* assignKeystoneRenderToTextureCamera(osgViewer::View* view, osg::GraphicsContext* gc, int width, int height, osg::Texture* texture)
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setName("Render to texture camera");
    camera->setGraphicsContext(gc);
    camera->setViewport(new osg::Viewport(0,0,width, height));
    camera->setDrawBuffer(GL_FRONT);
    camera->setReadBuffer(GL_FRONT);
    camera->setAllowEventFocus(false);
    camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

    // attach the texture and use it as the color buffer.
    camera->attach(osg::Camera::COLOR_BUFFER, texture);

    view->addSlave(camera.get(), osg::Matrixd(), osg::Matrixd());

    return camera.release();
}

osg::Camera* assignKeystoneDistortionCamera(osgViewer::View* view, osg::DisplaySettings* ds, osg::GraphicsContext* gc, int x, int y, int width, int height, GLenum buffer, osg::Texture* texture, Keystone* keystone)
{
    double screenDistance = ds->getScreenDistance();
    double screenWidth = ds->getScreenWidth();
    double screenHeight = ds->getScreenHeight();
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
    camera->setGraphicsContext(gc);
    camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
    camera->setClearColor( osg::Vec4(0.0,0.0,0.0,1.0) );
    camera->setViewport(new osg::Viewport(x, y, width, height));
    camera->setDrawBuffer(buffer);
    camera->setReadBuffer(buffer);
    camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
    camera->setInheritanceMask(camera->getInheritanceMask() & ~osg::CullSettings::CLEAR_COLOR & ~osg::CullSettings::COMPUTE_NEAR_FAR_MODE);
    //camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

    camera->setViewMatrix(osg::Matrix::identity());
    camera->setProjectionMatrixAsPerspective(fovy, aspectRatio, 0.1, 1000.0);

    // add subgraph to render
    camera->addChild(geode);

    camera->addChild(createGrid(keystone, osg::Vec4(1.0,1.0,1.0,1.0)));

    camera->setName("DistortionCorrectionCamera");

    // camera->addEventCallback(new KeystoneHandler(keystone));

    view->addSlave(camera.get(), osg::Matrixd(), osg::Matrixd(), false);

    return camera.release();
}



struct StereoSlaveCallback : public osg::View::Slave::UpdateSlaveCallback
{
    StereoSlaveCallback(osg::DisplaySettings* ds, double eyeScale):_ds(ds), _eyeScale(eyeScale) {}

    virtual void updateSlave(osg::View& view, osg::View::Slave& slave)
    {
        osg::Camera* camera = slave._camera.get();
        osgViewer::View* viewer_view = dynamic_cast<osgViewer::View*>(&view);

        if (_ds.valid() && camera && viewer_view)
        {

            // set projection matrix
            if (_eyeScale<0.0)
            {
                camera->setProjectionMatrix(_ds->computeLeftEyeProjectionImplementation(view.getCamera()->getProjectionMatrix()));
            }
            else
            {
                camera->setProjectionMatrix(_ds->computeRightEyeProjectionImplementation(view.getCamera()->getProjectionMatrix()));
            }

            double sd = _ds->getScreenDistance();
            double fusionDistance = sd;
            switch(viewer_view->getFusionDistanceMode())
            {
                case(osgUtil::SceneView::USE_FUSION_DISTANCE_VALUE):
                    fusionDistance = viewer_view->getFusionDistanceValue();
                    break;
                case(osgUtil::SceneView::PROPORTIONAL_TO_SCREEN_DISTANCE):
                    fusionDistance *= viewer_view->getFusionDistanceValue();
                    break;
            }
            double eyeScale = osg::absolute(_eyeScale) * (fusionDistance/sd);

            if (_eyeScale<0.0)
            {
                camera->setViewMatrix(_ds->computeLeftEyeViewImplementation(view.getCamera()->getViewMatrix(), eyeScale));
            }
            else
            {
                camera->setViewMatrix(_ds->computeRightEyeViewImplementation(view.getCamera()->getViewMatrix(), eyeScale));
            }
        }
        else
        {
            slave.updateSlaveImplementation(view);
        }
    }

    osg::ref_ptr<osg::DisplaySettings> _ds;
    double _eyeScale;
};

osg::Camera* assignStereoCamera(osgViewer::View* view, osg::DisplaySettings* ds, osg::GraphicsContext* gc, int x, int y, int width, int height, GLenum buffer, double eyeScale)
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;

    camera->setGraphicsContext(gc);
    camera->setViewport(new osg::Viewport(x,y, width, height));
    camera->setDrawBuffer(buffer);
    camera->setReadBuffer(buffer);

    // add this slave camera to the viewer, with a shift left of the projection matrix
    view->addSlave(camera.get(), osg::Matrixd::identity(), osg::Matrixd::identity());

    // assign update callback to maintain the correct view and projection matrices
    osg::View::Slave& slave = view->getSlave(view->getNumSlaves()-1);
    slave._updateSlaveCallback =  new StereoSlaveCallback(ds, eyeScale);

    return camera.release();
}

static const GLubyte patternVertEven[] = {
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55,
    0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55};

static const GLubyte patternVertOdd[] = {
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA};

static const GLubyte patternHorzEven[] = {
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00,
    0xFF, 0xFF, 0xFF, 0xFF, 0x00, 0x00, 0x00, 0x00};

// 32 x 32 bit array every row is a horizontal line of pixels
//  and the (bitwise) columns a vertical line
//  The following is a checkerboard pattern
static const GLubyte patternCheckerboard[] = {
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA,
    0x55, 0x55, 0x55, 0x55,
    0xAA, 0xAA, 0xAA, 0xAA};


void setUpViewForStereo(osgViewer::View* view, osg::DisplaySettings* ds)
{
    if (!ds->getStereo()) return;

    ds->setUseSceneViewForStereoHint(false);


    osg::ref_ptr<Keystone> keystone = new Keystone;

   
    // set up view's main camera
    {
        double height = osg::DisplaySettings::instance()->getScreenHeight();
        double width = osg::DisplaySettings::instance()->getScreenWidth();
        double distance = osg::DisplaySettings::instance()->getScreenDistance();
        double vfov = osg::RadiansToDegrees(atan2(height/2.0f,distance)*2.0);

        view->getCamera()->setProjectionMatrixAsPerspective( vfov, width/height, 1.0f,10000.0f);
    }
    

    int screenNum = 0;

    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
    {
        OSG_NOTICE<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }

    // unsigned int numScreens = wsi->getNumScreens(si);
    
    osg::GraphicsContext::ScreenIdentifier si;
    si.readDISPLAY();

    // displayNum has not been set so reset it to 0.
    if (si.displayNum<0) si.displayNum = 0;

    si.screenNum = screenNum;

    unsigned int width, height;
    wsi->getScreenResolution(si, width, height);

//    width/=2; height/=2;

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits(ds);
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

    OSG_NOTICE<<"traits->stencil="<<traits->stencil<<std::endl;


    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    if (!gc)
    {
        OSG_NOTICE<<"GraphicsWindow has not been created successfully."<<std::endl;
        return;
    }

    switch(ds->getStereoMode())
    {
        case(osg::DisplaySettings::QUAD_BUFFER):
        {
            // left Camera left buffer
            {
                osg::ref_ptr<osg::Camera> camera = assignStereoCamera(view, ds, gc, 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK_LEFT : GL_FRONT_LEFT, -1.0);
                camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
                camera->setRenderOrder(osg::Camera::NESTED_RENDER, 0);
            }

            // right Camera right buffer
            {
                osg::ref_ptr<osg::Camera> camera = assignStereoCamera(view, ds, gc, 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK_RIGHT : GL_FRONT_RIGHT, 1.0);
                camera->setClearMask(GL_DEPTH_BUFFER_BIT);
                camera->setRenderOrder(osg::Camera::NESTED_RENDER, 1);
            }

            // for keystone:
            // left camera to render to left texture
            // right camera to render to right texture
            // left keystone camera to render to left buffer
            // left keystone camera to render to right buffer
            // one keystone and editing for the one window
            
            break;
        }
        case(osg::DisplaySettings::ANAGLYPHIC):
        {
            // left Camera red
            osg::ref_ptr<osg::Camera> left_camera = assignStereoCamera(view, ds, gc, 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, -1.0);
            left_camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
            left_camera->getOrCreateStateSet()->setAttribute(new osg::ColorMask(true, false, false, true));
            left_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 0);

            // right Camera cyan
            osg::ref_ptr<osg::Camera> right_camera = assignStereoCamera(view, ds, gc, 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, 1.0);
            right_camera->setClearMask(GL_DEPTH_BUFFER_BIT);
            right_camera->getOrCreateStateSet()->setAttribute(new osg::ColorMask(false, true, true, true));
            right_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 1);

            if (keystone.valid())
            {
                // for keystone:
                // left camera to render to texture using red colour mask
                // right camera to render to same texture using cyan colour mask
                // keystone camera to render to whole screen without colour masks
                // one keystone and editing for the one window

                // create distortion texture
                osg::ref_ptr<osg::Texture> texture = createKestoneDistortionTexture(traits->width, traits->height);

                // convert to RTT Camera
                left_camera->setDrawBuffer(GL_FRONT);
                left_camera->setReadBuffer(GL_FRONT);
                left_camera->setAllowEventFocus(false);
                left_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                // attach the texture and use it as the color buffer.
                left_camera->attach(osg::Camera::COLOR_BUFFER, texture.get());


                // convert to RTT Camera
                right_camera->setDrawBuffer(GL_FRONT);
                right_camera->setReadBuffer(GL_FRONT);
                right_camera->setAllowEventFocus(false);
                right_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                // attach the texture and use it as the color buffer.
                right_camera->attach(osg::Camera::COLOR_BUFFER, texture.get());


                // create Keystone distortion camera
                osg::ref_ptr<osg::Camera> camera = assignKeystoneDistortionCamera(view, ds, gc.get(),
                                                                                0, 0, traits->width, traits->height,
                                                                                traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                texture, keystone.get());

                camera->setRenderOrder(osg::Camera::NESTED_RENDER, 2);
                
                // attach Keystone editing event handler.
                camera->addEventCallback(new KeystoneHandler(keystone.get()));
            }

            break;
        }
        case(osg::DisplaySettings::HORIZONTAL_SPLIT):
        {
            // left viewport Camera
            osg::ref_ptr<osg::Camera> left_camera = assignStereoCamera(view, ds, gc,
                               0, 0, traits->width/2, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT,
                               (ds->getSplitStereoHorizontalEyeMapping()==osg::DisplaySettings::LEFT_EYE_LEFT_VIEWPORT) ? -1.0 : 1.0);

            // right viewport Camera
            osg::ref_ptr<osg::Camera> right_camera = assignStereoCamera(view, ds, gc,
                               traits->width/2,0, traits->width/2, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT,
                               (ds->getSplitStereoHorizontalEyeMapping()==osg::DisplaySettings::LEFT_EYE_RIGHT_VIEWPORT) ? -1.0 : 1.0);

            if (keystone.valid())
            {
                // for keystone:
                // left camera to render to left texture using whole viewport of left texture
                // right camera to render to right texture using whole viewport of right texture
                // left keystone camera to render to left viewport/window
                // right keystone camera to render to right viewport/window
                // two keystone, one for each of the left and right viewports/windows

                // create distortion texture
                osg::ref_ptr<osg::Texture> left_texture = createKestoneDistortionTexture(traits->width/2, traits->height);

                // convert to RTT Camera
                left_camera->setViewport(0, 0, traits->width/2, traits->height);
                left_camera->setDrawBuffer(GL_FRONT);
                left_camera->setReadBuffer(GL_FRONT);
                left_camera->setAllowEventFocus(true);
                left_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                // attach the texture and use it as the color buffer.
                left_camera->attach(osg::Camera::COLOR_BUFFER, left_texture.get());


                // create distortion texture
                osg::ref_ptr<osg::Texture> right_texture = createKestoneDistortionTexture(traits->width/2, traits->height);

                // convert to RTT Camera
                right_camera->setViewport(0, 0, traits->width/2, traits->height);
                right_camera->setDrawBuffer(GL_FRONT);
                right_camera->setReadBuffer(GL_FRONT);
                right_camera->setAllowEventFocus(true);
                right_camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

                // attach the texture and use it as the color buffer.
                right_camera->attach(osg::Camera::COLOR_BUFFER, right_texture.get());
                

                // create Keystone left distortion camera
                keystone->gridColour.set(1.0f,0.0f,0.0,1.0);
                osg::ref_ptr<osg::Camera> left_keystone_camera = assignKeystoneDistortionCamera(view, ds, gc.get(),
                                                                                0, 0, traits->width/2, traits->height,
                                                                                traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                left_texture, keystone.get());

                left_keystone_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 2);

                // attach Keystone editing event handler.
                left_keystone_camera->addEventCallback(new KeystoneHandler(keystone.get()));


                osg::ref_ptr<Keystone> right_keystone = new Keystone;
                right_keystone->gridColour.set(0.0f,1.0f,0.0,1.0);
                
                // create Keystone right distortion camera
                osg::ref_ptr<osg::Camera> right_keystone_camera = assignKeystoneDistortionCamera(view, ds, gc.get(),
                                                                                traits->width/2, 0, traits->width/2, traits->height,
                                                                                traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                                right_texture, right_keystone.get());

                right_keystone_camera->setRenderOrder(osg::Camera::NESTED_RENDER, 3);

                // attach Keystone editing event handler.
                right_keystone_camera->addEventCallback(new KeystoneHandler(right_keystone.get()));

                view->getCamera()->setAllowEventFocus(false);
                
            }
            
            break;
        }
        case(osg::DisplaySettings::VERTICAL_SPLIT):
        {
            // bottom viewport Camera
            assignStereoCamera(view, ds, gc,
                               0, 0, traits->width, traits->height/2, traits->doubleBuffer ? GL_BACK : GL_FRONT,
                               (ds->getSplitStereoVerticalEyeMapping()==osg::DisplaySettings::LEFT_EYE_BOTTOM_VIEWPORT) ? -1.0 : 1.0);

            // top vieport camera
            assignStereoCamera(view, ds, gc,
                               0,traits->height/2, traits->width, traits->height/2, traits->doubleBuffer ? GL_BACK : GL_FRONT,
                               (ds->getSplitStereoVerticalEyeMapping()==osg::DisplaySettings::LEFT_EYE_TOP_VIEWPORT) ? -1.0 : 1.0);

            // for keystone:
            // left camera to render to left texture using whole viewport of left texture
            // right camera to render to right texture using whole viewport of right texture
            // left keystone camera to render to left viewport/window
            // right keystone camera to render to right viewport/window
            // two keystone, one for each of the left and right viewports/windows

            break;
        }
        case(osg::DisplaySettings::LEFT_EYE):
        {
            // single window, whole window, just left eye offsets
            osg::ref_ptr<osg::Camera> camera = assignStereoCamera(view, ds, gc, 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, -1.0);

            // for keystone:
            // treat as standard keystone correction.
            // left eye camera to render to texture
            // keystone camera then render to window
            // one keystone and editing for window

            break;
        }
        case(osg::DisplaySettings::RIGHT_EYE):
        {
            // single window, whole window, just right eye offsets
            osg::ref_ptr<osg::Camera> camera = assignStereoCamera(view, ds, gc, 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, 1.0);

            // for keystone:
            // treat as standard keystone correction.
            // left eye camera to render to texture
            // keystone camera then render to window
            // one keystone and editing for window

            break;
        }
        case(osg::DisplaySettings::HORIZONTAL_INTERLACE):
        case(osg::DisplaySettings::VERTICAL_INTERLACE):
        case(osg::DisplaySettings::CHECKERBOARD):
        {
            // set up the stencil buffer
            {
                osg::ref_ptr<osg::Camera> camera = new osg::Camera;
                camera->setGraphicsContext(gc.get());
                camera->setViewport(0, 0, traits->width, traits->height);
                camera->setDrawBuffer(traits->doubleBuffer ? GL_BACK : GL_FRONT);
                camera->setReadBuffer(camera->getDrawBuffer());
                camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
                camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);
                camera->setClearStencil(0);
                camera->setRenderOrder(osg::Camera::NESTED_RENDER, 0);
                view->addSlave(camera.get(), false);

                osg::ref_ptr<osg::Geometry> geometry = osg::createTexturedQuadGeometry(osg::Vec3(-1.0f,-1.0f,0.0f), osg::Vec3(2.0f,0.0f,0.0f), osg::Vec3(0.0f,2.0f,0.0f), 0.0f, 0.0f, 1.0f, 1.0f);
                osg::ref_ptr<osg::Geode> geode = new osg::Geode;
                geode->addDrawable(geometry.get());
                camera->addChild(geode.get());

                geode->setCullingActive(false);
                
                osg::ref_ptr<osg::StateSet> stateset = geode->getOrCreateStateSet();

                // set up stencil
                osg::ref_ptr<osg::Stencil> stencil = new osg::Stencil;
                stencil->setFunction(osg::Stencil::ALWAYS, 1, ~0u);
                stencil->setOperation(osg::Stencil::REPLACE, osg::Stencil::REPLACE, osg::Stencil::REPLACE);
                stencil->setWriteMask(~0u);
                stateset->setAttributeAndModes(stencil.get(), osg::StateAttribute::ON);

                // set up polygon stipple
                if(ds->getStereoMode() == osg::DisplaySettings::VERTICAL_INTERLACE)
                {
                    stateset->setAttributeAndModes(new osg::PolygonStipple(patternVertEven), osg::StateAttribute::ON);
                }
                else if(ds->getStereoMode() == osg::DisplaySettings::HORIZONTAL_INTERLACE)
                {
                    stateset->setAttributeAndModes(new osg::PolygonStipple(patternHorzEven), osg::StateAttribute::ON);
                }
                else
                {
                    stateset->setAttributeAndModes(new osg::PolygonStipple(patternCheckerboard), osg::StateAttribute::ON);
                }

                stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
                stateset->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);

            }

            OSG_NOTICE<<"view->getNumSlaves()="<<view->getNumSlaves()<<std::endl;
            // left Camera
            {
                osg::ref_ptr<osg::Camera> camera = assignStereoCamera(view, ds, gc, 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, -1.0);
                camera->setClearMask(0);
                camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
                camera->setRenderOrder(osg::Camera::NESTED_RENDER, 1);

                osg::ref_ptr<osg::Stencil> stencil = new osg::Stencil;
                stencil->setFunction(osg::Stencil::EQUAL, 0, ~0u);
                stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
                camera->getOrCreateStateSet()->setAttributeAndModes(stencil.get(), osg::StateAttribute::ON);
            }

            // right Camera cyan
            {
                osg::ref_ptr<osg::Camera> camera = assignStereoCamera(view, ds, gc, 0, 0, traits->width, traits->height, traits->doubleBuffer ? GL_BACK : GL_FRONT, 1.0);
                camera->setClearMask(GL_DEPTH_BUFFER_BIT);
                camera->setRenderOrder(osg::Camera::NESTED_RENDER, 2);

                osg::ref_ptr<osg::Stencil> stencil = new osg::Stencil;
                stencil->setFunction(osg::Stencil::NOTEQUAL, 0, ~0u);
                stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
                camera->getOrCreateStateSet()->setAttributeAndModes(stencil.get(), osg::StateAttribute::ON);
            }
            break;
        }
    }
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

    osg::DisplaySettings* ds = osg::DisplaySettings::instance();


    // create distortion texture
    osg::ref_ptr<osg::Texture> texture = createKestoneDistortionTexture(width, height);

    // create RTT Camera
    assignKeystoneRenderToTextureCamera(view, gc.get(), width, height, texture);

    // create Keystone distortion camera
    osg::ref_ptr<osg::Camera> camera = assignKeystoneDistortionCamera(view, ds, gc.get(),
                                                                      0, 0, width, height,
                                                                      traits->doubleBuffer ? GL_BACK : GL_FRONT,
                                                                      texture, keystone);
    // attach Keystone editing event handler.
    camera->addEventCallback(new KeystoneHandler(keystone));
    
}

int main( int argc, char **argv )
{
    osg::ArgumentParser arguments(&argc,argv);
    
    // initialize the viewer.
    osgViewer::Viewer viewer(arguments);

    osg::DisplaySettings* ds = viewer.getDisplaySettings() ? viewer.getDisplaySettings() : osg::DisplaySettings::instance().get();
    ds->readCommandLine(arguments);

    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles(arguments);

    if (!model)
    {
        OSG_NOTICE<<"No models loaded, please specify a model file on the command line"<<std::endl;
        return 1;
    }


    OSG_NOTICE<<"Stereo "<<ds->getStereo()<<std::endl;
    OSG_NOTICE<<"StereoMode "<<ds->getStereoMode()<<std::endl;

    viewer.setSceneData(model.get());
    
    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    // add camera manipulator
    viewer.setCameraManipulator(new osgGA::TrackballManipulator());

    if (ds->getStereo())
    {
        setUpViewForStereo(&viewer, ds);
    }
    else
    {
        setUpViewForKeystone(&viewer, new Keystone);
    }
    
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
