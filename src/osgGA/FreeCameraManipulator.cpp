#include <osgGA/FreeCameraManipulator>


using namespace osgGA;


FreeCameraManipulator::FreeCameraManipulator() : 
	_position(osg::Vec3(0.0, 0.0, 2.0)),
	_rotation(osg::Quat()),
	_x(0.0),
	_y(osg::PI_2),
	_speed(2.5),
	_sensitivity(5.0),
	_move(osg::Vec3d(0.0,0.0,0.0))
{
}


FreeCameraManipulator::FreeCameraManipulator(const FreeCameraManipulator& m, const osg::CopyOp& copyOp) : osgGA::StandardManipulator(m, copyOp)
{
}


void FreeCameraManipulator::setTransformation(const osg::Vec3d& eye, const osg::Quat& rotation)
{
}


void FreeCameraManipulator::setTransformation(const osg::Vec3d& eye, const osg::Vec3d& center, const osg::Vec3d& up)
{
}


void FreeCameraManipulator::getTransformation(osg::Vec3d& eye, osg::Quat& rotation) const
{
}


void FreeCameraManipulator::getTransformation(osg::Vec3d& eye, osg::Vec3d& center, osg::Vec3d& up) const
{
}


void FreeCameraManipulator::home(double h)
{
}


void FreeCameraManipulator::home(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
}


osg::Matrixd FreeCameraManipulator::getMatrix() const
{
	osg::Matrixd matrix;
	matrix.postMultTranslate(_position);
	matrix.postMultRotate(_rotation);
	return matrix;
}


osg::Matrixd FreeCameraManipulator::getInverseMatrix() const
{
    osg::Matrixd matrix;
    matrix.postMultTranslate(-_position);
	matrix.postMultRotate(-_rotation);
    return matrix;
}


void FreeCameraManipulator::setByMatrix(const osg::Matrixd& matrix)
{
	_position = matrix.getTrans();
	_rotation = matrix.getRotate();
}


void FreeCameraManipulator::setByInverseMatrix(const osg::Matrixd& matrix)
{
	setByMatrix(osg::Matrixd::inverse(matrix));
}


bool FreeCameraManipulator::handleMouseMove(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
//	addMouseEvent(ea);
	centerMousePointer(ea, us);

	_x += _sensitivity * ea.getXnormalized();
	_y += _sensitivity * ea.getYnormalized();

	if(_y < 0.0)
	{
		_y = 0.0;
	}
	else if(_y > osg::PI)
	{
		_y = osg::PI;
	}

	_rotation.makeRotate(_x, osg::Vec3d(0.0,0.0,1.0));
	_rotation *= osg::Quat(-_y, getSideVector(getCoordinateFrame(_position)));

	return true;
}


bool FreeCameraManipulator::handleKeyDown(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& us)
{
	switch(ea.getKey())
	{
		case osgGA::GUIEventAdapter::KEY_W:
			_move = _rotation.inverse() * osg::Vec3d(0.0, 0.0, -_speed);
		break;

		case osgGA::GUIEventAdapter::KEY_S:
			_move = _rotation.inverse() * osg::Vec3d(0.0, 0.0, _speed);
		break;

		case osgGA::GUIEventAdapter::KEY_A:
			_move = _rotation.inverse() * osg::Vec3d(-_speed, 0.0, 0.0);
		break;

		case osgGA::GUIEventAdapter::KEY_D:
			_move = _rotation.inverse() * osg::Vec3d(_speed, 0.0, 0.0);
		break;

		default:
			_move = osg::Vec3d(0.0,0.0,0.0);
		break;
	}

	_position += _move;

	return false;
}


void FreeCameraManipulator::setPosition(const osg::Vec3& position)
{
	_position = position;
}


void FreeCameraManipulator::setSpeed(double speed)
{
	_speed = speed;
}


void FreeCameraManipulator::setMouseSensitivity(double sens)
{
	_sensitivity = sens;
}
