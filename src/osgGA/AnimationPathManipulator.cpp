#include <osgGA/AnimationPathManipulator>
#include <stdio.h>

using namespace osgGA;

AnimationPathManipulator::AnimationPathManipulator(osg::AnimationPath* animationPath) 
{
    _animationPath = animationPath;
    _timeOffset = 0.0f;
    _timeScale = 1.0f;
}

AnimationPathManipulator::AnimationPathManipulator( const std::string& filename ) 
{
    _animationPath = osgNew osg::AnimationPath;
    _animationPath->setLoopMode(osg::AnimationPath::LOOP);

    FILE *fp = fopen( filename.c_str(), "r" );
    if( fp == NULL )
    {
	osg::notify(osg::WARN) << "AnimationPathManipulator: Cannot open animation path file \"" << filename << "\".\n";
	_valid = false;
	return;
    }
    while( !feof( fp ))
    {
	double time;
        osg::Vec3 position;
        osg::Quat rotation;
	fscanf( fp, "%lf %f %f %f %f %f %f %f\n",
	    &time, &position[0], &position[1], &position[2],
	    &rotation[0], &rotation[1], &rotation[2], &rotation[3] );

	if( !feof(fp))
            _animationPath->insert(time,osg::AnimationPath::ControlPoint(position,rotation));
    }
    fclose(fp);
}

void AnimationPathManipulator::home(const GUIEventAdapter& ea,GUIActionAdapter&)
{
    if (_animationPath.valid())
    {
        _timeOffset = _animationPath->getFirstTime()-ea.time();
    }
}

void AnimationPathManipulator::init(const GUIEventAdapter& ea,GUIActionAdapter& aa)
{
    home(ea,aa);
}

bool AnimationPathManipulator::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us)
{
    if( !valid() ) return false;

    us = us;

    bool retval = false;
    switch( ea.getEventType() )
    {
	case GUIEventAdapter::FRAME:
	    handleFrame( ea.time() );
            retval =  true;
	    break;
	case GUIEventAdapter::KEYBOARD:
            if (ea.getKey()==' ')
            {
                home(ea,us);
                us.requestRedraw();
                us.requestContinuousUpdate(false);
                return true;
            } 
            return false;

	    retval =  false;
	    break;
        default:
            break;
    }
    return retval;
}

void AnimationPathManipulator::handleFrame( double time )
{
    osg::AnimationPath::ControlPoint cp;
    _animationPath->getInterpolatedControlPoint( (time+_timeOffset)*_timeScale, cp );

    osg::Matrix matrix;
    cp.getMatrix( matrix );
    
    if (_camera.valid())
    {
        _camera->home();
        _camera->transformLookAt(matrix);
    }
}
