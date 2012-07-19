#include <osgText/Text3D>

#include <iostream>
#include <string>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool Text3D_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool Text3D_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(Text3D_Proxy)
(
    new osgText::Text3D,
    "Text3D",
    "Object Drawable TextBase Text3D",
    Text3D_readLocalData,
    Text3D_writeLocalData
);

osgText::Text3D::RenderMode convertRenderModeStringToEnum(const std::string str)
{
    if (str == "PER_GLYPH") return osgText::Text3D::PER_GLYPH;
    else if (str == "PER_FACE") return osgText::Text3D::PER_FACE;
    return static_cast<osgText::Text3D::RenderMode>(-1);
}
std::string convertRenderModeEnumToString(osgText::Text3D::RenderMode renderMode)
{
    switch (renderMode)
    {
    case osgText::Text3D::PER_GLYPH: return "PER_GLYPH";
    case osgText::Text3D::PER_FACE: return "PER_FACE";
    default: return "";
    }
}

bool Text3D_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgText::Text3D &text = static_cast<osgText::Text3D &>(obj);
    bool itAdvanced = false;


    // characterDepth
    if (fr[0].matchWord("characterDepth"))
    {
        float depth;
        if (fr[1].getFloat(depth))
        {
            text.setCharacterDepth(depth);
            fr += 2;
            itAdvanced = true;
        }
    }

    // RenderMode
    if (fr[0].matchWord("renderMode"))
    {
        osgText::Text3D::RenderMode renderMode = convertRenderModeStringToEnum(fr[1].getStr());
        if (renderMode != static_cast<osgText::Text3D::RenderMode>(-1))
        {
            text.setRenderMode(renderMode);
        }
        fr += 2;
        itAdvanced = true;
    }

    return itAdvanced;
}

bool Text3D_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgText::Text3D &text = static_cast<const osgText::Text3D &>(obj);

    fw.indent() << "characterDepth " << text.getCharacterDepth() << std::endl;

    fw.indent() << "renderMode " << convertRenderModeEnumToString(text.getRenderMode()) << std::endl;

    return true;
}
