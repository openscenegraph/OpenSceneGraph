#include <osgIntrospection/ReflectionMacros>
#include <osgIntrospection/TypedMethodInfo>
#include <osgIntrospection/Attributes>

#include <osg/Geode>

using namespace osgIntrospection;

BEGIN_OBJECT_REFLECTOR(osg::Geode)
	BaseType(osg::Node);
	ArrayPropertyWithReturnType(osg::Drawable *, Drawable, Drawables, bool);
END_REFLECTOR
