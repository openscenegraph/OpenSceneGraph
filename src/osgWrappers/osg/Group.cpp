#include <osgIntrospection/ReflectionMacros>
#include <osgIntrospection/TypedMethodInfo>
#include <osgIntrospection/Attributes>

#include <osg/Group>

using namespace osgIntrospection;

BEGIN_OBJECT_REFLECTOR(osg::Group)
	BaseType(osg::Node);
	ArrayPropertyWithReturnType(osg::Node *, Child, Children, bool);
END_REFLECTOR
