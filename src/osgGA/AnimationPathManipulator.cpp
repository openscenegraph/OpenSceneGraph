#include <osgGA/AnimationPathManipulator>
#include <stdio.h>

using namespace osgGA;

AnimationPathManipulator::AnimationPathManipulator(osg::AnimationPath* animationPath) 
{
    _animationPath = animationPath;
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

bool AnimationPathManipulator::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us)
{
    if( !valid() ) return false;

    us = us;

    bool retval = false;
    switch( ea.getEventType() )
    {
	case GUIEventAdapter::FRAME:
	    handleFrame( ea.time() );

	    break;
	case GUIEventAdapter::KEYBOARD:
	    switch( ea.getKey())
	    {
		default:
		    retval =  false;
	    }
	    break;
        default:
            break;
    }
    return retval;
}

void AnimationPathManipulator::handleFrame( double time )
{
    osg::AnimationPath::ControlPoint cp;
    _animationPath->getInterpolatedControlPoint( time, cp );

    osg::Matrix mat;
    cp.getMatrix( mat );

    osg::Vec3 eye(mat(3,0), mat(3,1), mat(3,2));
    mat(3,0) = 0.0;
    mat(3,1) = 0.0;
    mat(3,2) = 0.0;
    osg::Vec3 look = eye + (osg::Vec3(0,1,0) * mat);
    osg::Vec3 up = osg::Vec3(0,0,1) * mat;
    if( _camera.valid() )
	_camera->setView( eye, look, up );
}
