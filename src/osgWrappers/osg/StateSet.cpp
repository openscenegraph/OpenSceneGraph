#include <osgIntrospection/ReflectionMacros>
#include <osgIntrospection/TypedMethodInfo>
#include <osgIntrospection/Attributes>

#include <osg/StateSet>

using namespace osgIntrospection;

BEGIN_ABSTRACT_OBJECT_REFLECTOR(osg::StateAttribute)
	BaseType(osg::Object);
END_REFLECTOR

BEGIN_OBJECT_REFLECTOR(osg::StateSet)
	BaseType(osg::Object)
END_REFLECTOR

BEGIN_ENUM_REFLECTOR(osg::StateSet::RenderingHint)
	EnumLabel(osg::StateSet::DEFAULT_BIN);
	EnumLabel(osg::StateSet::OPAQUE_BIN);
	EnumLabel(osg::StateSet::TRANSPARENT_BIN);
END_REFLECTOR

BEGIN_ENUM_REFLECTOR(osg::StateSet::RenderBinMode)
	EnumLabel(osg::StateSet::ENCLOSE_RENDERBIN_DETAILS);
	EnumLabel(osg::StateSet::INHERIT_RENDERBIN_DETAILS);
	EnumLabel(osg::StateSet::OVERRIDE_RENDERBIN_DETAILS);
	EnumLabel(osg::StateSet::USE_RENDERBIN_DETAILS);
END_REFLECTOR

STD_MAP_REFLECTOR_WITH_TYPES(osg::StateSet::ModeList, unsigned int, osg::StateAttribute::Values);
STD_MAP_REFLECTOR(osg::StateSet::AttributeList);
STD_CONTAINER_REFLECTOR(osg::StateSet::TextureAttributeList);
STD_CONTAINER_REFLECTOR(osg::StateSet::TextureModeList);

BEGIN_OBJECT_REFLECTOR(osg::StateSet)
	BaseType(osg::Object)
	Property(int, RenderingHint);
		Attribute(PropertyTypeAttribute(typeof(osg::StateSet::RenderingHint)));

	ReadOnlyProperty(const osg::StateSet::ModeList &, ModeList);
	ReadOnlyProperty(const osg::StateSet::AttributeList &, AttributeList);
	ReadOnlyProperty(const osg::StateSet::TextureModeList &, TextureModeList);
	ReadOnlyProperty(const osg::StateSet::TextureAttributeList &, TextureAttributeList);

END_REFLECTOR


typedef osg::ref_ptr<osg::StateAttribute> RefStateAttribute;
STD_VALUE_REFLECTOR(RefStateAttribute);

STD_PAIR_REFLECTOR(osg::StateAttribute::TypeMemberPair)
STD_PAIR_REFLECTOR_WITH_TYPES(osg::StateSet::RefAttributePair, osg::StateAttribute *, osg::StateAttribute::Values)

