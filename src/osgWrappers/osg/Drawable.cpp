#include <osgIntrospection/ReflectionMacros>
#include <osgIntrospection/TypedMethodInfo>
#include <osgIntrospection/Attributes>

#include <osg/Drawable>

BEGIN_ABSTRACT_OBJECT_REFLECTOR(osg::Drawable)
	BaseType(osg::Object);
	Property(osg::StateSet *, StateSet);
	ReadOnlyProperty(osg::Drawable::ParentList, Parents);
END_REFLECTOR

STD_CONTAINER_REFLECTOR(osg::Drawable::ParentList);
