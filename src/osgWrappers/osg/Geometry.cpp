#include <osgIntrospection/ReflectionMacros>
#include <osgIntrospection/TypedMethodInfo>
#include <osgIntrospection/Attributes>

#include <osg/Drawable>
#include <osg/Geometry>

BEGIN_ABSTRACT_OBJECT_REFLECTOR(osg::Drawable)
	BaseType(osg::Object);
	Property(osg::StateSet *, StateSet);
	ReadOnlyProperty(osg::Drawable::ParentList, Parents);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::Geometry)
	BaseType(osg::Drawable);
END_REFLECTOR
