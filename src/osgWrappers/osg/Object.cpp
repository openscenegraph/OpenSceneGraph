#include <osgIntrospection/ReflectionMacros>
#include <osgIntrospection/TypedMethodInfo>
#include <osgIntrospection/Attributes>

#include <osg/Object>

using namespace osgIntrospection;

// simple reflectors for types that are considered 'atomic'
ABSTRACT_OBJECT_REFLECTOR(osg::Referenced)

BEGIN_ENUM_REFLECTOR(osg::Object::DataVariance)
	EnumLabel(osg::Object::STATIC);
	EnumLabel(osg::Object::DYNAMIC);
END_REFLECTOR

BEGIN_ABSTRACT_OBJECT_REFLECTOR(osg::Object)
	BaseType(osg::Referenced);
	Property(osg::Object::DataVariance, DataVariance);	
		
	Property(osg::Referenced *, UserData);
		Attribute(DefaultValueAttribute(0));
END_REFLECTOR
