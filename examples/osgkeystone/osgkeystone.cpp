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

#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>


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

protected:

    osg::ref_ptr<Keystone>      _keystone;

    osg::Vec2d                  _startPosition;
    osg::ref_ptr<ControlPoints> _startControlPoints;
    
    Region                      _selectedRegion;
    osg::ref_ptr<ControlPoints> _currentControlPoints;

};

KeystoneHandler::KeystoneHandler(Keystone* keystone):
    _keystone(keystone),
    _selectedRegion(NONE_SELECTED)
{
    _startControlPoints = new ControlPoints;
    _currentControlPoints = new ControlPoints;
}

bool KeystoneHandler::handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa)
{
    switch(ea.getEventType())
    {
        case(osgGA::GUIEventAdapter::PUSH):
        {
            if (ea.getModKeyMask()==osgGA::GUIEventAdapter::MODKEY_LEFT_CTRL || ea.getModKeyMask()==osgGA::GUIEventAdapter::MODKEY_RIGHT_CTRL )
            {
                float x = ea.getXnormalized();
                float y = ea.getYnormalized();
                if (x<-0.33)
                {
                    // left side
                    if (y<-0.33) _selectedRegion = BOTTOM_LEFT;
                    else if (y<0.33) _selectedRegion = LEFT;
                    else _selectedRegion = TOP_LEFT;
                }
                else if (x<0.33)
                {
                    // center side
                    if (y<-0.33) _selectedRegion = BOTTOM;
                    else if (y<0.33) _selectedRegion = CENTER;
                    else _selectedRegion = TOP;
                }
                else
                {
                    // right side
                    if (y<-0.33) _selectedRegion = BOTTOM_RIGHT;
                    else if (y<0.33) _selectedRegion = RIGHT;
                    else _selectedRegion = TOP_RIGHT;
                }
                (*_startControlPoints) = (*_currentControlPoints);
                _startPosition.set(x,y);
                OSG_NOTICE<<"Region "<<_selectedRegion<<std::endl;
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
                OSG_NOTICE<<"Moving region "<<_selectedRegion<<", moving by "<<currentPosition-_startPosition<<std::endl;
                switch(_selectedRegion)
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
                return true;
            }

            return false;
        }
        case(osgGA::GUIEventAdapter::RELEASE):
        {
            OSG_NOTICE<<"Mouse released "<<std::endl;
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
            return false;
        }
        default:
            return false;
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
    

    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles(arguments);

    if (!model)
    {
        OSG_NOTICE<<"No models loaded, please specify a model file on the command line"<<std::endl;
        return 1;
    }

    viewer.setSceneData(model.get());

    viewer.setCameraManipulator(new osgGA::TrackballManipulator());

    viewer.realize();

    viewer.getCamera()->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);

    osg::Matrixd original_pm = viewer.getCamera()->getProjectionMatrix();

    viewer.addEventHandler(new KeystoneHandler(keystone));

    while(!viewer.done())
    {
        viewer.advance();
        viewer.eventTraversal();
        viewer.updateTraversal();
        
        if (keystone.valid())
        {
            viewer.getCamera()->setProjectionMatrix(original_pm * keystone->computeKeystoneMatrix());
        }
        viewer.renderingTraversals();
    }
    return 0;
}
