#include <osgIntrospection/ReflectionMacros>
#include <osgIntrospection/TypedMethodInfo>
#include <osgIntrospection/Attributes>

#include <osg/Node>
#include <osg/Group>

using namespace osgIntrospection;

BEGIN_OBJECT_REFLECTOR(osg::NodeCallback)
	BaseType(osg::Object);
	Property(osg::NodeCallback *, NestedCallback);
END_REFLECTOR

BEGIN_ABSTRACT_OBJECT_REFLECTOR(osg::Node)
	BaseType(osg::Object);
	Property(const std::string &, Name);
	Property(osg::NodeCallback *, UpdateCallback);
	Property(osg::StateSet *, StateSet);
	ReadOnlyProperty(osg::Node::ParentList, Parents);
END_REFLECTOR

STD_CONTAINER_REFLECTOR(osg::Node::ParentList);
