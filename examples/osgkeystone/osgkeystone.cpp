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


class ControlPoints : public osg::Referenced
{
public:
    ControlPoints():
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

    ControlPoints& operator = (const ControlPoints& rhs)
    {
        if (&rhs==this) return *this;
        bottom_left = rhs.bottom_left;
        bottom_right = rhs.bottom_right;
        top_left = rhs.top_left;
        top_right = rhs.top_right;
        return *this;
    }

    osg::Vec2d bottom_left;
    osg::Vec2d bottom_right;
    osg::Vec2d top_left;
    osg::Vec2d top_right;
};

class Keystone : public osg::Referenced
{
public:
    Keystone():
        translate(0.0,0.0),
        scale(1.0,1.0),
        taper(1.0,1.0),
        angle(0.0)
    {
    }

    double angleBetweenVectors(const osg::Vec2d& v1, const osg::Vec2d& v2) const
    {
        osg::Vec2d v3(-v2.y(), v2.x());
        double p1 = v1*v2;
        double p2 = v1*v3;
        double a = atan2(p2, p1);
        return a;
    }

    osg::Vec2d rotateVector(osg::Vec2d& v, double s, double c) const
    {
        return osg::Vec2d(v.x()*c-v.y()*s, v.y()*c+v.x()*s);
    }
    

    void updateKeystone(ControlPoints cp)
    {
        // compute translation
        translate = (cp.bottom_left+cp.bottom_right+cp.top_left+cp.top_right)*0.25;

        // adjust control points to fit translation
        cp.top_left -= translate;
        cp.top_right -= translate;
        cp.bottom_right -= translate;
        cp.bottom_left -= translate;

        angle = (angleBetweenVectors(cp.top_left, osg::Vec2d(-1.0,1.0)) +
                   angleBetweenVectors(cp.top_right, osg::Vec2d(1.0,1.0)) +
                   angleBetweenVectors(cp.bottom_right, osg::Vec2d(1.0,-1.0)) +
                   angleBetweenVectors(cp.bottom_left, osg::Vec2d(-1.0,-1.0)))*0.25;

        OSG_NOTICE<<"cp.top_left="<<cp.top_left<<std::endl;
        OSG_NOTICE<<"cp.top_right="<<cp.top_right<<std::endl;
        OSG_NOTICE<<"cp.bottom_right="<<cp.bottom_right<<std::endl;
        OSG_NOTICE<<"cp.bottom_left="<<cp.bottom_left<<std::endl;

        double s = sin(angle);
        double c = cos(angle);
        cp.top_left = rotateVector(cp.top_left, s, c);
        cp.top_right = rotateVector(cp.top_right, s, c);
        cp.bottom_right = rotateVector(cp.bottom_right, s, c);
        cp.bottom_left = rotateVector(cp.bottom_left, s, c);

        OSG_NOTICE<<"after rotate cp.top_left="<<cp.top_left<<std::endl;
        OSG_NOTICE<<"             cp.top_right="<<cp.top_right<<std::endl;
        OSG_NOTICE<<"             cp.bottom_right="<<cp.bottom_right<<std::endl;
        OSG_NOTICE<<"             cp.bottom_left="<<cp.bottom_left<<std::endl;
        

        // compute scaling
        scale.x() = ( (cp.top_right.x()-cp.top_left.x()) + (cp.bottom_right.x()-cp.bottom_left.x()) )/4.0;
        scale.y() = ( (cp.top_right.y()-cp.bottom_right.y()) + (cp.top_left.y()-cp.bottom_left.y()) )/4.0;

        // adjust control points to fit scaling
        cp.top_left.x() *= scale.x();
        cp.top_right.x() *= scale.x();
        cp.bottom_right.x() *= scale.x();
        cp.bottom_left.x() *= scale.x();
        cp.top_left.y() *= scale.y();
        cp.top_right.y() *= scale.y();
        cp.bottom_right.y() *= scale.y();
        cp.bottom_left.y() *= scale.y();

        taper.x() = (cp.top_left-cp.bottom_left).length() / (cp.top_right-cp.bottom_right).length();
        taper.y() = (cp.bottom_right-cp.bottom_left).length() / (cp.top_right-cp.top_left).length();
        OSG_NOTICE<<"translate="<<translate<<std::endl;
        OSG_NOTICE<<"scale="<<scale<<std::endl;
        OSG_NOTICE<<"taper="<<taper<<std::endl;
        OSG_NOTICE<<"angle="<<osg::RadiansToDegrees(angle)<<std::endl;
    }

    osg::Matrixd computeKeystoneMatrix() const
    {
        osg::Matrixd pm;
        pm.postMultRotate(osg::Quat(angle, osg::Vec3d(0.0,0.0,1.0)));
        pm.postMultScale(osg::Vec3d(scale.x(),scale.y(),1.0));
        pm.postMultTranslate(osg::Vec3d(translate.x(),translate.y(),0.0));

        if (taper.x()!=1.0)
        {
            double x0 = (1.0+taper.x())/(1-taper.x());
            pm.postMult(osg::Matrixd(1.0-x0, 0.0,    0.0,    1.0,
                                    0.0,    1.0-x0, 0.0,    0.0,
                                    0.0,    0.0,    (1.0-x0)*0.25, 0.0,
                                    0.0,    0.0,    0.0,    -x0));
        }
        if (taper.y()!=1.0)
        {
            double y0 = (1.0+taper.y())/(1-taper.y());
            pm.postMult(osg::Matrixd(1.0-y0, 0.0,    0.0,    0.0,
                                    0.0,    1.0-y0, 0.0,    1.0,
                                    0.0,    0.0,    (1.0-y0)*0.25, 0.0,
                                    0.0,    0.0,    0.0,    -y0));
        }
        return pm;
    }
    
    osg::Vec2d translate;
    osg::Vec2d scale;
    osg::Vec2d taper;
    double angle;
};


class KeystoneHandler : public osgGA::GUIEventHandler
{
public:

    KeystoneHandler(Keystone* keystone);

    ~KeystoneHandler() {}

    bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa);

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
    osg::ref_ptr<ControlPoints> _startControlPoints;
    
    Region                      _selectedRegion;
    osg::ref_ptr<ControlPoints> _currentControlPoints;

};

KeystoneHandler::KeystoneHandler(Keystone* keystone):
    _keystone(keystone),
    _defaultIncrement(0.0,0.0),
    _ctrlIncrement(1.0,1.0),
    _shiftIncrement(0.1,0.1),
    _keyIncrement(0.005, 0.005),
    _selectedRegion(NONE_SELECTED)
{
    _startControlPoints = new ControlPoints;
    _currentControlPoints = new ControlPoints;
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
    _keystone->updateKeystone(*_currentControlPoints);
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
            return false;
        }
        case(osgGA::GUIEventAdapter::DRAG):
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

            return false;
        }
        case(osgGA::GUIEventAdapter::RELEASE):
        {
            _selectedRegion = NONE_SELECTED;
            return false;
        }
        case(osgGA::GUIEventAdapter::KEYDOWN):
        {
            if (ea.getKey()=='r')
            {
                _selectedRegion = NONE_SELECTED;
                _startControlPoints->reset();
                _currentControlPoints->reset();
                _keystone->updateKeystone(*_currentControlPoints);
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
            return false;
        }
        default:
            return false;
    }
}

osg::Node* createGrid(const osg::Vec3& origin, const osg::Vec3& widthVector, const osg::Vec3& heightVector, const osg::Vec4& colour)
{
    osg::ref_ptr<osg::Geode> geode = new osg::Geode;
    
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geode->addDrawable(geometry.get());

    osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array;
    colours->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    geometry->setColorArray(colours.get());
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices.get());
    
    // border line
    {
        unsigned int vi = vertices->size();
        vertices->push_back(origin);
        vertices->push_back(origin+widthVector);
        vertices->push_back(origin+widthVector+heightVector);
        vertices->push_back(origin+heightVector);
        geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINE_LOOP, vi, 4));
    }
    
    // cross lines
    {
        unsigned int vi = vertices->size();
        vertices->push_back(origin);
        vertices->push_back(origin+widthVector+heightVector);
        vertices->push_back(origin+heightVector);
        vertices->push_back(origin+widthVector);
        geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINES, vi, 4));
    }
    
    // vertices lines
    {
        unsigned int vi = vertices->size();
        osg::Vec3 dv = widthVector/6.0;
        osg::Vec3 bv = origin+dv;
        osg::Vec3 tv = bv+heightVector;
        for(unsigned int i=0; i<5; ++i)
        {
            vertices->push_back(bv);
            vertices->push_back(tv);
            bv += dv;
            tv += dv;
        }
        geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINES, vi, 10));
    }

    // horizontal lines
    {
        unsigned int vi = vertices->size();
        osg::Vec3 dv = heightVector/6.0;
        osg::Vec3 bv = origin+dv;
        osg::Vec3 tv = bv+widthVector;
        for(unsigned int i=0; i<5; ++i)
        {
            vertices->push_back(bv);
            vertices->push_back(tv);
            bv += dv;
            tv += dv;
        }
        geometry->addPrimitiveSet(new osg::DrawArrays(GL_LINES, vi, 10));
    }

    geometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
    
    return geode.release();
}

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

        
        
        double screenDistance = osg::DisplaySettings::instance()->getScreenDistance();
        double screenWidth = osg::DisplaySettings::instance()->getScreenWidth();
        double screenHeight = osg::DisplaySettings::instance()->getScreenHeight();

        double tr = _keystone->taper.x();
        double r_right = sqrt(tr);
        double r_left = r_right/tr;

        double sx = _keystone->scale.x();
        double sy = _keystone->scale.y();

        double tx = _keystone->translate.x();
        double ty = _keystone->translate.y();

        osg::Matrixd pm;
        pm.postMultRotate(osg::Quat(_keystone->angle, osg::Vec3d(0.0,0.0,1.0)));

        (*vertices)[0] = osg::Vec3(-screenWidth*sx*0.5*r_left + tx*screenWidth*0.5*r_left, screenHeight*sy*0.5 + ty*screenHeight*0.5 ,-screenDistance*r_left) * pm;
        (*vertices)[1] = osg::Vec3(screenWidth*sx*0.5*r_right + tx*screenWidth*0.5*r_right, screenHeight*sy*0.5 + ty*screenHeight*0.5,-screenDistance*r_right) * pm;
        (*vertices)[2] = osg::Vec3(screenWidth*sx*0.5*r_right + tx*screenWidth*0.5*r_right,-screenHeight*sy*0.5 + ty*screenHeight*0.5,-screenDistance*r_right) * pm;
        (*vertices)[3] = osg::Vec3(-screenWidth*sx*0.5*r_left + tx*screenWidth*0.5*r_left,-screenHeight*sy*0.5 + ty*screenHeight*0.5,-screenDistance*r_left) * pm;

        geometry->dirtyBound();
    }
    

    osg::ref_ptr<Keystone> _keystone;
};



osg::Geometry* createKeystoneDistortionMesh(Keystone* cp)
{
    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setUseDisplayList(false);

    osg::ref_ptr<KeystoneUpdateCallback> kuc = new KeystoneUpdateCallback(cp);
    geometry->setUpdateCallback(kuc.get());

    osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array;
    colours->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    geometry->setColorArray(colours.get());
    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);

    osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
    geometry->setVertexArray(vertices.get());


    unsigned int vi = vertices->size();
    vertices->resize(4);

    osg::ref_ptr<osg::Vec2Array> texcoords = new osg::Vec2Array;
    geometry->setTexCoordArray(0, texcoords.get());

    texcoords->push_back(osg::Vec2(0.0f,1.0f));
    texcoords->push_back(osg::Vec2(1.0f,1.0f));
    texcoords->push_back(osg::Vec2(1.0f,0.0f));
    texcoords->push_back(osg::Vec2(0.0f,0.0f));

    geometry->addPrimitiveSet(new osg::DrawArrays(GL_QUADS, vi, 4));

    geometry->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    kuc->update(geometry.get());

    return geometry.release();
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

        
        osg::Geode* geode = new osg::Geode();
        geode->addDrawable(createKeystoneDistortionMesh(keystone));

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

        camera->setName("DistortionCorrectionCamera");

        view->addSlave(camera.get(), osg::Matrixd(), osg::Matrixd(), false);
    }
}

int main( int argc, char **argv )
{
    osg::ArgumentParser arguments(&argc,argv);
    
    // initialize the viewer.
    osgViewer::Viewer viewer(arguments);

    osg::ref_ptr<Keystone> keystone = new Keystone;

    double distance, width, view_angle;
    if (arguments.read("-p",distance, width, view_angle))
    {
        keystone->taper.x() = (2.0 + (width/distance) * sin(osg::inDegrees(view_angle))) / (2.0 - (width/distance) * sin(osg::inDegrees(view_angle)));
        //scale.x() = 1.0/cos(osg::inDegrees(view_angle));
        OSG_NOTICE<<"scale "<<keystone->scale<<std::endl;
        OSG_NOTICE<<"taper "<<keystone->taper<<std::endl;
    }

    if (arguments.read("-a",keystone->angle)) { OSG_NOTICE<<"angle = "<<keystone->angle<<std::endl; keystone->angle = osg::inDegrees(keystone->angle); }
    if (arguments.read("-t",keystone->translate.x(), keystone->translate.y())) { OSG_NOTICE<<"translate = "<<keystone->translate<<std::endl;}
    if (arguments.read("-s",keystone->scale.x(), keystone->scale.y())) { OSG_NOTICE<<"scale = "<<keystone->scale<<std::endl;}
    if (arguments.read("-k",keystone->taper.x(), keystone->taper.y())) { OSG_NOTICE<<"taper = "<<keystone->taper<<std::endl;}
    
    bool oldStyleKeystone = false;
    if (arguments.read("--old")) oldStyleKeystone = true;
    
    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles(arguments);

    if (!model)
    {
        OSG_NOTICE<<"No models loaded, please specify a model file on the command line"<<std::endl;
        return 1;
    }

    osg::ref_ptr<osg::Group> group = new osg::Group;
    group->addChild(model.get());

    double screenWidth = osg::DisplaySettings::instance()->getScreenWidth();
    double screenHeight = osg::DisplaySettings::instance()->getScreenHeight();
    double screenDistance = osg::DisplaySettings::instance()->getScreenDistance();
    double fovy = osg::RadiansToDegrees(2.0*atan2(screenHeight/2.0,screenDistance));
    double aspectRatio = screenWidth/screenHeight;
    
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setClearMask(0x0);
    camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
    camera->setRenderOrder(osg::Camera::NESTED_RENDER);
    camera->setProjectionMatrixAsPerspective(fovy, aspectRatio, 0.1, 1000.0);
    camera->addChild(createGrid(osg::Vec3(-screenWidth*0.5, -screenHeight*0.5, -screenDistance), osg::Vec3(screenWidth, 0.0, 0.0), osg::Vec3(0.0, screenHeight, 0.0), osg::Vec4(1.0,0.0,0.0,1.0)));

    viewer.setSceneData(group.get());

    
    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    // add camera manipulator
    viewer.setCameraManipulator(new osgGA::TrackballManipulator());

    
    if (!oldStyleKeystone)
    {
        setUpViewForKeystone(&viewer, keystone);
    }
    
    viewer.realize();

    viewer.getCamera()->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);

    osg::Matrixd original_pm = viewer.getCamera()->getProjectionMatrix();
    osg::Matrixd original_grid_pm = camera->getProjectionMatrix();

    group->addChild(camera.get());

    // Add keystone handler
    viewer.addEventHandler(new KeystoneHandler(keystone));

    while(!viewer.done())
    {
        viewer.advance();
        viewer.eventTraversal();
        viewer.updateTraversal();
        
        if (oldStyleKeystone && keystone.valid())
        {
            viewer.getCamera()->setProjectionMatrix(original_pm * keystone->computeKeystoneMatrix());
            camera->setProjectionMatrix(original_grid_pm * keystone->computeKeystoneMatrix());
        }
        viewer.renderingTraversals();
    }
    return 0;
}
