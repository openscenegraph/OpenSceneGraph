#include <osgIntrospection/ReflectionMacros>
#include <osgIntrospection/TypedMethodInfo>
#include <osgIntrospection/Attributes>

#include <osg/StateAttribute>

using namespace osgIntrospection;

BEGIN_ABSTRACT_OBJECT_REFLECTOR(osg::StateAttribute)
	BaseType(osg::Object);
END_REFLECTOR

BEGIN_ENUM_REFLECTOR(osg::StateAttribute::Values)
	EnumLabel(osg::StateAttribute::OFF);
	EnumLabel(osg::StateAttribute::ON);
	EnumLabel(osg::StateAttribute::OVERRIDE);
	EnumLabel(osg::StateAttribute::PROTECTED);
	EnumLabel(osg::StateAttribute::INHERIT);
END_REFLECTOR


BEGIN_ENUM_REFLECTOR(osg::StateAttribute::Type)
    EnumLabel(osg::StateAttribute::TEXTURE);
    EnumLabel(osg::StateAttribute::POLYGONMODE);
    EnumLabel(osg::StateAttribute::POLYGONOFFSET);
    EnumLabel(osg::StateAttribute::MATERIAL);
    EnumLabel(osg::StateAttribute::ALPHAFUNC);
    EnumLabel(osg::StateAttribute::ANTIALIAS);
    EnumLabel(osg::StateAttribute::COLORTABLE);
    EnumLabel(osg::StateAttribute::CULLFACE);
    EnumLabel(osg::StateAttribute::FOG);
    EnumLabel(osg::StateAttribute::FRONTFACE);
    EnumLabel(osg::StateAttribute::LIGHT);
    EnumLabel(osg::StateAttribute::POINT);
    EnumLabel(osg::StateAttribute::LINEWIDTH);
    EnumLabel(osg::StateAttribute::LINESTIPPLE);
    EnumLabel(osg::StateAttribute::POLYGONSTIPPLE);
    EnumLabel(osg::StateAttribute::SHADEMODEL);
    EnumLabel(osg::StateAttribute::TEXENV);
    EnumLabel(osg::StateAttribute::TEXENVFILTER);
    EnumLabel(osg::StateAttribute::TEXGEN);
    EnumLabel(osg::StateAttribute::TEXMAT);
    EnumLabel(osg::StateAttribute::LIGHTMODEL);
    EnumLabel(osg::StateAttribute::BLENDFUNC);
    EnumLabel(osg::StateAttribute::STENCIL);
    EnumLabel(osg::StateAttribute::COLORMASK);
    EnumLabel(osg::StateAttribute::DEPTH);
    EnumLabel(osg::StateAttribute::VIEWPORT);
    EnumLabel(osg::StateAttribute::BLENDCOLOR);
    EnumLabel(osg::StateAttribute::MULTISAMPLE);
    EnumLabel(osg::StateAttribute::CLIPPLANE);
    EnumLabel(osg::StateAttribute::COLORMATRIX);
    EnumLabel(osg::StateAttribute::VERTEXPROGRAM);
    EnumLabel(osg::StateAttribute::FRAGMENTPROGRAM);
    EnumLabel(osg::StateAttribute::POINTSPRITE);
    EnumLabel(osg::StateAttribute::PROGRAMOBJECT);
    EnumLabel(osg::StateAttribute::VALIDATOR);
    EnumLabel(osg::StateAttribute::VIEWMATRIXEXTRACTOR);

    EnumLabel(osg::StateAttribute::OSGNV_PARAMETER_BLOCK);
    EnumLabel(osg::StateAttribute::OSGNVEXT_TEXTURE_SHADER);
    EnumLabel(osg::StateAttribute::OSGNVEXT_VERTEX_PROGRAM);
    EnumLabel(osg::StateAttribute::OSGNVEXT_REGISTER_COMBINERS);
    EnumLabel(osg::StateAttribute::OSGNVCG_PROGRAM);
    EnumLabel(osg::StateAttribute::OSGNVSLANG_PROGRAM);
    EnumLabel(osg::StateAttribute::OSGNVPARSE_PROGRAM_PARSER);

END_REFLECTOR
