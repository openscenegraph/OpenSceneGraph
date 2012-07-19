#include <osgText/Text>

#include <iostream>
#include <string>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool Text_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool Text_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(Text_Proxy)
(
    new osgText::Text,
    "Text",
    "Object Drawable TextBase Text",
    Text_readLocalData,
    Text_writeLocalData
);


osgText::Text::BackdropType convertBackdropTypeStringToEnum(std::string & str)
{
    if      (str=="DROP_SHADOW_BOTTOM_RIGHT") return osgText::Text::DROP_SHADOW_BOTTOM_RIGHT;
    else if (str=="DROP_SHADOW_CENTER_RIGHT") return osgText::Text::DROP_SHADOW_CENTER_RIGHT;
    else if (str=="DROP_SHADOW_TOP_RIGHT") return osgText::Text::DROP_SHADOW_TOP_RIGHT;
    else if (str=="DROP_SHADOW_BOTTOM_CENTER") return osgText::Text::DROP_SHADOW_BOTTOM_CENTER;
    else if (str=="DROP_SHADOW_TOP_CENTER") return osgText::Text::DROP_SHADOW_TOP_CENTER;
    else if (str=="DROP_SHADOW_BOTTOM_LEFT") return osgText::Text::DROP_SHADOW_BOTTOM_LEFT;
    else if (str=="DROP_SHADOW_CENTER_LEFT") return osgText::Text::DROP_SHADOW_CENTER_LEFT;
    else if (str=="DROP_SHADOW_TOP_LEFT") return osgText::Text::DROP_SHADOW_TOP_LEFT;
    else if (str=="OUTLINE") return osgText::Text::OUTLINE;
    else if (str=="NONE") return osgText::Text::NONE;
    else return static_cast<osgText::Text::BackdropType>(-1);
}
std::string convertBackdropTypeEnumToString(osgText::Text::BackdropType backdropType)
{
    switch (backdropType)
    {
    case osgText::Text::DROP_SHADOW_BOTTOM_RIGHT: return "DROP_SHADOW_BOTTOM_RIGHT";
    case osgText::Text::DROP_SHADOW_CENTER_RIGHT: return "DROP_SHADOW_CENTER_RIGHT";
    case osgText::Text::DROP_SHADOW_TOP_RIGHT: return "DROP_SHADOW_TOP_RIGHT";
    case osgText::Text::DROP_SHADOW_BOTTOM_CENTER: return "DROP_SHADOW_BOTTOM_CENTER";
    case osgText::Text::DROP_SHADOW_TOP_CENTER: return "DROP_SHADOW_TOP_CENTER";
    case osgText::Text::DROP_SHADOW_BOTTOM_LEFT: return "DROP_SHADOW_BOTTOM_LEFT";
    case osgText::Text::DROP_SHADOW_CENTER_LEFT: return "DROP_SHADOW_CENTER_LEFT";
    case osgText::Text::DROP_SHADOW_TOP_LEFT: return "DROP_SHADOW_TOP_LEFT";
    case osgText::Text::OUTLINE: return "OUTLINE";
    case osgText::Text::NONE: return "NONE";
    default : return "";
    }
}


osgText::Text::BackdropImplementation convertBackdropImplementationStringToEnum(std::string & str)
{
    if      (str=="POLYGON_OFFSET") return osgText::Text::POLYGON_OFFSET;
    else if (str=="NO_DEPTH_BUFFER") return osgText::Text::NO_DEPTH_BUFFER;
    else if (str=="DEPTH_RANGE") return osgText::Text::DEPTH_RANGE;
    else if (str=="STENCIL_BUFFER") return osgText::Text::STENCIL_BUFFER;
    else return static_cast<osgText::Text::BackdropImplementation>(-1);
}
std::string convertBackdropImplementationEnumToString(osgText::Text::BackdropImplementation backdropImplementation)
{
    switch (backdropImplementation)
    {
    case osgText::Text::POLYGON_OFFSET: return "POLYGON_OFFSET";
    case osgText::Text::NO_DEPTH_BUFFER: return "NO_DEPTH_BUFFER";
    case osgText::Text::DEPTH_RANGE: return "DEPTH_RANGE";
    case osgText::Text::STENCIL_BUFFER: return "STENCIL_BUFFER";
    default : return "";
    }
}

osgText::Text::ColorGradientMode convertColorGradientModeStringToEnum(std::string & str)
{
    if      (str=="SOLID") return osgText::Text::SOLID;
    else if (str=="PER_CHARACTER") return osgText::Text::PER_CHARACTER;
    else if (str=="OVERALL") return osgText::Text::OVERALL;
    else return static_cast<osgText::Text::ColorGradientMode>(-1);
}
std::string convertColorGradientModeEnumToString(osgText::Text::ColorGradientMode colorGradientMode)
{
    switch (colorGradientMode)
    {
    case osgText::Text::SOLID: return "SOLID";
    case osgText::Text::PER_CHARACTER: return "PER_CHARACTER";
    case osgText::Text::OVERALL: return "OVERALL";
    default : return "";
    }
}


bool Text_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgText::Text &text = static_cast<osgText::Text &>(obj);
    bool itAdvanced = false;


    // backdropType
    if (fr[0].matchWord("backdropType"))
    {
        std::string str = fr[1].getStr();
        osgText::Text::BackdropType backdropType = convertBackdropTypeStringToEnum(str);

        if (backdropType != static_cast<osgText::Text::BackdropType>(-1))
            text.setBackdropType(backdropType);

        fr += 2;
        itAdvanced = true;
    }

    float backdropHorizontalOffset = text.getBackdropHorizontalOffset();
    float backdropVerticalOffset = text.getBackdropVerticalOffset();

    // backdropHorizontalOffset
    if (fr[0].matchWord("backdropHorizontalOffset"))
    {
        if (fr[1].getFloat(backdropHorizontalOffset))
        {
            fr += 2;
            itAdvanced = true;
        }
    }

    // backdropVerticalOffset
    if (fr[0].matchWord("backdropVerticalOffset"))
    {
        if (fr[1].getFloat(backdropVerticalOffset))
        {
            fr += 2;
            itAdvanced = true;
        }
    }
    text.setBackdropOffset(backdropHorizontalOffset, backdropVerticalOffset);

    // backdropColor
    if (fr[0].matchWord("backdropColor"))
    {
        osg::Vec4 c;
        if (fr[1].getFloat(c.x()) && fr[2].getFloat(c.y()) && fr[3].getFloat(c.z()) && fr[4].getFloat(c.w()))
        {
            text.setBackdropColor(c);
            fr += 4;
            itAdvanced = true;
        }
    }

    // backdropImplementation
    if (fr[0].matchWord("backdropImplementation"))
    {
        std::string str = fr[1].getStr();
        osgText::Text::BackdropImplementation backdropImplementation = convertBackdropImplementationStringToEnum(str);

        if (backdropImplementation != static_cast<osgText::Text::BackdropImplementation>(-1))
            text.setBackdropImplementation(backdropImplementation);

        fr += 2;
        itAdvanced = true;
    }

    // ColorGradientMode
    if (fr[0].matchWord("colorGradientMode"))
    {
        std::string str = fr[1].getStr();
        osgText::Text::ColorGradientMode colorGradientMode = convertColorGradientModeStringToEnum(str);

        if (colorGradientMode != static_cast<osgText::Text::ColorGradientMode>(-1))
            text.setColorGradientMode(colorGradientMode);

        fr += 2;
        itAdvanced = true;
    }

    // ** get default value;
    osg::Vec4 colorGradientTopLeft = text.getColorGradientTopLeft();
    osg::Vec4 colorGradientBottomLeft = text.getColorGradientBottomLeft();
    osg::Vec4 colorGradientBottomRight = text.getColorGradientBottomRight();
    osg::Vec4 colorGradientTopRight = text.getColorGradientTopRight();

    // colorGradientTopLeft
    if (fr[0].matchWord("colorGradientTopLeft"))
    {
        osg::Vec4 c;
        if (fr[1].getFloat(c.x()) && fr[2].getFloat(c.y()) && fr[3].getFloat(c.z()) && fr[4].getFloat(c.w()))
        {
            colorGradientTopLeft = c;
            fr += 4;
            itAdvanced = true;
        }
    }

    // colorGradientBottomLeft
    if (fr[0].matchWord("colorGradientBottomLeft"))
    {
        osg::Vec4 c;
        if (fr[1].getFloat(c.x()) && fr[2].getFloat(c.y()) && fr[3].getFloat(c.z()) && fr[4].getFloat(c.w()))
        {
            colorGradientBottomLeft = c;
            fr += 4;
            itAdvanced = true;
        }
    }

    // colorGradientBottomRight
    if (fr[0].matchWord("colorGradientBottomRight"))
    {
        osg::Vec4 c;
        if (fr[1].getFloat(c.x()) && fr[2].getFloat(c.y()) && fr[3].getFloat(c.z()) && fr[4].getFloat(c.w()))
        {
            colorGradientBottomRight = c;
            fr += 4;
            itAdvanced = true;
        }
    }

    // colorGradientTopRight
    if (fr[0].matchWord("colorGradientTopRight"))
    {
        osg::Vec4 c;
        if (fr[1].getFloat(c.x()) && fr[2].getFloat(c.y()) && fr[3].getFloat(c.z()) && fr[4].getFloat(c.w()))
        {
            colorGradientTopRight = c;
            fr += 4;
            itAdvanced = true;
        }
    }

    text.setColorGradientCorners(colorGradientTopLeft, colorGradientBottomLeft, colorGradientBottomRight, colorGradientTopRight);

    return itAdvanced;
}

bool Text_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgText::Text &text = static_cast<const osgText::Text &>(obj);

    // backdropType
    fw.indent() << "backdropType " << convertBackdropTypeEnumToString(text.getBackdropType()) << std::endl;

    // backdropHorizontalOffset
    fw.indent() << "backdropHorizontalOffset " << text.getBackdropHorizontalOffset() << std::endl;

    // backdropVerticalOffset
    fw.indent() << "backdropVerticalOffset " << text.getBackdropVerticalOffset() << std::endl;

    // backdropColor
    osg::Vec4 c = text.getBackdropColor();
    fw.indent() << "backdropColor " << c.x() << " " << c.y() << " " << c.z() << " " << c.w() << std::endl;

    // backdropImplementation
    fw.indent() << "backdropImplementation " << convertBackdropImplementationEnumToString(text.getBackdropImplementation()) << std::endl;

    // colorGradientMode
    fw.indent() << "colorGradientMode " << convertColorGradientModeEnumToString(text.getColorGradientMode()) << std::endl;

    // colorGradientTopLeft
    c = text.getColorGradientTopLeft();
    fw.indent() << "colorGradientTopLeft " << c.x() << " " << c.y() << " " << c.z() << " " << c.w() << std::endl;

    // colorGradientBottomLeft
    c = text.getColorGradientBottomLeft();
    fw.indent() << "colorGradientBottomLeft " << c.x() << " " << c.y() << " " << c.z() << " " << c.w() << std::endl;

    // colorGradientBottomRight
    c = text.getColorGradientBottomRight();
    fw.indent() << "colorGradientBottomRight " << c.x() << " " << c.y() << " " << c.z() << " " << c.w() << std::endl;

    // colorGradientTopRight
    c = text.getColorGradientTopRight();
    fw.indent() << "colorGradientTopRight " << c.x() << " " << c.y() << " " << c.z() << " " << c.w() << std::endl;

    return true;
}
