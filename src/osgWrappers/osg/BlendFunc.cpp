#include <osgIntrospection/ReflectionMacros>
#include <osgIntrospection/TypedMethodInfo>
#include <osgIntrospection/Attributes>

#include <osg/BlendFunc>

using namespace osgIntrospection;

BEGIN_ENUM_REFLECTOR(osg::BlendFunc::BlendFuncMode)
	EnumLabel(osg::BlendFunc::DST_ALPHA);
	EnumLabel(osg::BlendFunc::DST_COLOR);
	EnumLabel(osg::BlendFunc::ONE);
	EnumLabel(osg::BlendFunc::ONE_MINUS_DST_ALPHA);
	EnumLabel(osg::BlendFunc::ONE_MINUS_DST_COLOR);
	EnumLabel(osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
	EnumLabel(osg::BlendFunc::ONE_MINUS_SRC_COLOR);
	EnumLabel(osg::BlendFunc::SRC_ALPHA);
	EnumLabel(osg::BlendFunc::SRC_ALPHA_SATURATE);
	EnumLabel(osg::BlendFunc::SRC_COLOR);
	EnumLabel(osg::BlendFunc::CONSTANT_COLOR);
	EnumLabel(osg::BlendFunc::ONE_MINUS_CONSTANT_COLOR);
	EnumLabel(osg::BlendFunc::CONSTANT_ALPHA);
	EnumLabel(osg::BlendFunc::ONE_MINUS_CONSTANT_ALPHA);
	EnumLabel(osg::BlendFunc::ZERO);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::BlendFunc)
	BaseType(osg::StateAttribute)
	Method2(void, setFunction, IN, GLenum, source, IN, GLenum, destination);
	
	Property(GLenum, Source);
		Attribute(PropertyTypeAttribute(typeof(osg::BlendFunc::BlendFuncMode)));

	Property(GLenum, Destination);
		Attribute(PropertyTypeAttribute(typeof(osg::BlendFunc::BlendFuncMode)));

END_REFLECTOR
