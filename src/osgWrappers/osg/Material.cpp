#include <osgIntrospection/ReflectionMacros>
#include <osgIntrospection/TypedMethodInfo>
#include <osgIntrospection/Attributes>

#include <osg/Material>

using namespace osgIntrospection;

BEGIN_ENUM_REFLECTOR(osg::Material::Face)
	EnumLabel(osg::Material::FRONT);
	EnumLabel(osg::Material::BACK);
	EnumLabel(osg::Material::FRONT_AND_BACK);
END_REFLECTOR

BEGIN_ENUM_REFLECTOR(osg::Material::ColorMode)
	EnumLabel(osg::Material::AMBIENT);
	EnumLabel(osg::Material::SPECULAR);
	EnumLabel(osg::Material::DIFFUSE);
	EnumLabel(osg::Material::EMISSION);
	EnumLabel(osg::Material::AMBIENT_AND_DIFFUSE);
	EnumLabel(osg::Material::OFF);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::Material)
	BaseType(osg::StateAttribute);
	Property(osg::Material::ColorMode, ColorMode);
	IndexedProperty(const osg::Vec4 &, Ambient, osg::Material::Face, face);
	IndexedProperty(const osg::Vec4 &, Diffuse, osg::Material::Face, face);
	IndexedProperty(const osg::Vec4 &, Specular, osg::Material::Face, face);
END_REFLECTOR

