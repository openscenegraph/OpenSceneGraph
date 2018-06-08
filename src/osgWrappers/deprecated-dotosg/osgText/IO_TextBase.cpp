#include <osgText/Text>
#include <osgText/Font>

#include <iostream>
#include <string>

#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/io_utils>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgDB/ParameterOutput>

bool TextBase_readLocalData(osg::Object &obj, osgDB::Input &fr);
bool TextBase_writeLocalData(const osg::Object &obj, osgDB::Output &fw);

REGISTER_DOTOSGWRAPPER(TextBase_Proxy)
(
    /*new osgText::Text*/NULL,
    "TextBase",
    "Object Drawable TextBase",
    TextBase_readLocalData,
    TextBase_writeLocalData
);

bool TextBase_readLocalData(osg::Object &obj, osgDB::Input &fr)
{
    osgText::Text &text = static_cast<osgText::Text &>(obj);
    bool itAdvanced = false;

    // color
    if (fr[0].matchWord("color"))
    {
        osg::Vec4 c;
        if (fr[1].getFloat(c.x()) && fr[2].getFloat(c.y()) && fr[3].getFloat(c.z()) && fr[4].getFloat(c.w()))
        {
            text.setColor(c);
            fr += 4;
            itAdvanced = true;
        }
    }

    if (fr.matchSequence("font %w"))
    {
        text.setFont(fr[1].getStr());
        fr += 2;
        itAdvanced = true;
    }

    if (fr[0].matchWord("fontResolution") || fr[0].matchWord("fontSize"))
    {
        unsigned int width;
        unsigned int height;
        if (fr[1].getUInt(width) && fr[2].getUInt(height))
        {
            text.setFontResolution(width,height);
            fr += 3;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("characterSize"))
    {
        float height;
        float aspectRatio;
        if (fr[1].getFloat(height) && fr[2].getFloat(aspectRatio))
        {
            text.setCharacterSize(height,aspectRatio);
            fr += 3;
            itAdvanced = true;
        }
    }

    if (fr.matchSequence("characterSizeMode %w"))
    {
        std::string str = fr[1].getStr();
        if      (str=="OBJECT_COORDS") text.setCharacterSizeMode(osgText::Text::OBJECT_COORDS);
        else if (str=="SCREEN_COORDS") text.setCharacterSizeMode(osgText::Text::SCREEN_COORDS);
        else if (str=="OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT") text.setCharacterSizeMode(osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT);
    }

    // maximum dimentsions of text box.
    if (fr[0].matchWord("maximumWidth"))
    {
        float width;
        if (fr[1].getFloat(width))
        {
            text.setMaximumWidth(width);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("maximumHeight"))
    {
        float height;
        if (fr[1].getFloat(height))
        {
            text.setMaximumHeight(height);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr[0].matchWord("lineSpacing"))
    {
        float height;
        if (fr[1].getFloat(height))
        {
            text.setLineSpacing(height);
            fr += 2;
            itAdvanced = true;
        }
    }

    if (fr.matchSequence("alignment %w"))
    {
        std::string str = fr[1].getStr();
        if      (str=="LEFT_TOP") text.setAlignment(osgText::Text::LEFT_TOP);
        else if (str=="LEFT_CENTER") text.setAlignment(osgText::Text::LEFT_CENTER);
        else if (str=="LEFT_BOTTOM") text.setAlignment(osgText::Text::LEFT_BOTTOM);
        else if (str=="CENTER_TOP") text.setAlignment(osgText::Text::CENTER_TOP);
        else if (str=="CENTER_CENTER") text.setAlignment(osgText::Text::CENTER_CENTER);
        else if (str=="CENTER_BOTTOM") text.setAlignment(osgText::Text::CENTER_BOTTOM);
        else if (str=="RIGHT_TOP") text.setAlignment(osgText::Text::RIGHT_TOP);
        else if (str=="RIGHT_CENTER") text.setAlignment(osgText::Text::RIGHT_CENTER);
        else if (str=="RIGHT_BOTTOM") text.setAlignment(osgText::Text::RIGHT_BOTTOM);
        else if (str=="LEFT_BASE_LINE") text.setAlignment(osgText::Text::LEFT_BASE_LINE);
        else if (str=="CENTER_BASE_LINE") text.setAlignment(osgText::Text::CENTER_BASE_LINE);
        else if (str=="RIGHT_BASE_LINE") text.setAlignment(osgText::Text::RIGHT_BASE_LINE);
        else if (str=="LEFT_BOTTOM_BASE_LINE") text.setAlignment(osgText::Text::LEFT_BOTTOM_BASE_LINE);
        else if (str=="CENTER_BOTTOM_BASE_LINE") text.setAlignment(osgText::Text::CENTER_BOTTOM_BASE_LINE);
        else if (str=="RIGHT_BOTTOM_BASE_LINE") text.setAlignment(osgText::Text::RIGHT_BOTTOM_BASE_LINE);
        else if (str=="BASE_LINE") text.setAlignment(osgText::Text::BASE_LINE);
        fr += 2;
        itAdvanced = true;
    }

    if (fr.matchSequence("axisAlignment %w"))
    {
        std::string str = fr[1].getStr();
        if      (str=="XY_PLANE") text.setAxisAlignment(osgText::Text::XY_PLANE);
        else if (str=="REVERSED_XY_PLANE") text.setAxisAlignment(osgText::Text::REVERSED_XY_PLANE);
        else if (str=="XZ_PLANE") text.setAxisAlignment(osgText::Text::XZ_PLANE);
        else if (str=="REVERSED_XZ_PLANE") text.setAxisAlignment(osgText::Text::REVERSED_XZ_PLANE);
        else if (str=="YZ_PLANE") text.setAxisAlignment(osgText::Text::YZ_PLANE);
        else if (str=="REVERSED_YZ_PLANE") text.setAxisAlignment(osgText::Text::REVERSED_YZ_PLANE);
        else if (str=="SCREEN") text.setAxisAlignment(osgText::Text::SCREEN);
        fr += 2;
        itAdvanced = true;
    }

    if (fr.matchSequence("rotation"))
    {
        osg::Vec4 rotation;
        if (fr[1].getFloat(rotation.x()) && fr[2].getFloat(rotation.y()) && fr[3].getFloat(rotation.z()) && fr[4].getFloat(rotation.w()))
        {
            text.setRotation(rotation);
            fr += 4;
            itAdvanced = true;
        }
    }

    if (fr.matchSequence("autoRotateToScreen TRUE"))
    {
        text.setAutoRotateToScreen(true);
        fr += 2;
        itAdvanced = true;
    }

    if (fr.matchSequence("autoScaleToLimitScreenSizeToFontResolution TRUE"))
    {
        text.setCharacterSizeMode(osgText::Text::SCREEN_COORDS);
        fr += 2;
        itAdvanced = true;
    }

    if (fr.matchSequence("layout %w") && fr[1].getStr())
    {
        std::string str = fr[1].getStr();
        if      (str=="LEFT_TO_RIGHT") text.setLayout(osgText::Text::LEFT_TO_RIGHT);
        else if (str=="RIGHT_TO_LEFT") text.setLayout(osgText::Text::RIGHT_TO_LEFT);
        else if (str=="VERTICAL") text.setLayout(osgText::Text::VERTICAL);
        fr += 2;
        itAdvanced = true;
    }


    // position
    if (fr[0].matchWord("position"))
    {
        osg::Vec3 p;
        if (fr[1].getFloat(p.x()) && fr[2].getFloat(p.y()) && fr[3].getFloat(p.z()))
        {
            text.setPosition(p);
            fr += 4;
            itAdvanced = true;
        }
    }

    // draw mode
    if (fr[0].matchWord("drawMode"))
    {
        int i;
        if (fr[1].getInt(i)) {
            text.setDrawMode(i);
            fr += 2;
            itAdvanced = true;
        }
    }

    // bounding box margin
    if (fr[0].matchWord("BoundingBoxMargin"))
    {
        float margin;
        if (fr[1].getFloat(margin)) {
            text.setBoundingBoxMargin(margin);
            fr += 2;
            itAdvanced = true;
        }
    }

    // bounding box color
    if (fr[0].matchWord("BoundingBoxColor"))
    {
        osg::Vec4 c;
        if (fr[1].getFloat(c.x()) && fr[2].getFloat(c.y()) && fr[3].getFloat(c.z()) && fr[4].getFloat(c.w()))
        {
            text.setBoundingBoxColor(c);
            fr += 5;
            itAdvanced = true;
        }
    }

    // text
    if (fr.matchSequence("text %s") && fr[1].getStr()) {
        text.setText(std::string(fr[1].getStr()));
        fr += 2;
        itAdvanced = true;
    }

    if (fr.matchSequence("text %i {"))
    {
        // pre 0.9.3 releases..
        int entry = fr[0].getNoNestedBrackets();

        int capacity;
        fr[1].getInt(capacity);

        osgText::String str;
        str.reserve(capacity);

        fr += 3;

        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            unsigned int c;
            if (fr[0].getUInt(c))
            {
                ++fr;
                str.push_back(c);
            }
            else
            {
                ++fr;
            }
        }

        text.setText(str);

        itAdvanced = true;
        ++fr;
    }

    return itAdvanced;
}

//osgText::Text::CharactereSizeMode convertCharactereSizeModeStringtoEnum(std::string & str)
//{
//    if      (str=="OBJECT_COORDS") return osgText::Text::OBJECT_COORDS;
//    else if (str=="SCREEN_COORDS") return osgText::Text::SCREEN_COORDS;
//    else if (str=="OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT") return osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT;
//    else return -1;
//}
//
//std::string convertCharactereSizeModeStringtoEnum(osgText::Text::CharactereSizeMode charactereSizeMode)
//{
//    switch(charactereSizeMode)
//    {
//      case osgText::Text::OBJECT_COORDS : return "OBJECT_COORDS";
//      case osgText::Text::SCREEN_COORDS : return "SCREEN_COORDS";
//      case osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT: return "OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT";
//      default : return "";
//    }
//}

bool TextBase_writeLocalData(const osg::Object &obj, osgDB::Output &fw)
{
    const osgText::Text &text = static_cast<const osgText::Text &>(obj);

    // color
    osg::Vec4 c = text.getColor();
    fw.indent() << "color " << c.x() << " " << c.y() << " " << c.z() << " " << c.w() << std::endl;

    if (text.getFont())
    {
        fw.indent() << "font " << text.getFont()->getFileName() << std::endl;
    }

    // font resolution
    fw.indent() << "fontResolution " << text.getFontWidth() << " " << text.getFontHeight() << std::endl;

    // character size.
    fw.indent() << "characterSize " << text.getCharacterHeight() << " " << text.getCharacterAspectRatio() << std::endl;

    fw.indent() << "characterSizeMode ";
    switch(text.getCharacterSizeMode())
    {
      case osgText::Text::OBJECT_COORDS : fw<<"OBJECT_COORDS"<<std::endl; break;
      case osgText::Text::SCREEN_COORDS : fw<<"SCREEN_COORDS"<<std::endl; break;
      case osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT: fw<<"OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT"<<std::endl; break;
    }

    // maximum dimension of text box.
    if (text.getMaximumWidth()>0.0f)
    {
        fw.indent() << "maximumWidth " << text.getMaximumWidth() << std::endl;
    }

    if (text.getMaximumHeight()>0.0f)
    {
        fw.indent() << "maximumHeight " << text.getMaximumHeight() << std::endl;
    }

    if (text.getLineSpacing()>0.0f)
    {
        fw.indent() << "lineSpacing " << text.getLineSpacing() << std::endl;
    }

    // alignment
    fw.indent() << "alignment ";
    switch(text.getAlignment())
    {
      case osgText::Text::LEFT_TOP:        fw << "LEFT_TOP" << std::endl; break;
      case osgText::Text::LEFT_CENTER :    fw << "LEFT_CENTER" << std::endl; break;
      case osgText::Text::LEFT_BOTTOM :    fw << "LEFT_BOTTOM" << std::endl; break;

      case osgText::Text::CENTER_TOP:      fw << "CENTER_TOP" << std::endl; break;
      case osgText::Text::CENTER_CENTER:   fw << "CENTER_CENTER" << std::endl; break;
      case osgText::Text::CENTER_BOTTOM:   fw << "CENTER_BOTTOM" << std::endl; break;

      case osgText::Text::RIGHT_TOP:       fw << "RIGHT_TOP" << std::endl; break;
      case osgText::Text::RIGHT_CENTER:    fw << "RIGHT_CENTER" << std::endl; break;
      case osgText::Text::RIGHT_BOTTOM:    fw << "RIGHT_BOTTOM" << std::endl; break;

      case osgText::Text::LEFT_BASE_LINE:  fw << "LEFT_BASE_LINE" << std::endl; break;
      case osgText::Text::CENTER_BASE_LINE:fw << "CENTER_BASE_LINE" << std::endl; break;
      case osgText::Text::RIGHT_BASE_LINE: fw << "RIGHT_BASE_LINE" << std::endl; break;

      case osgText::Text::LEFT_BOTTOM_BASE_LINE:  fw << "LEFT_BOTTOM_BASE_LINE" << std::endl; break;
      case osgText::Text::CENTER_BOTTOM_BASE_LINE:fw << "CENTER_BOTTOM_BASE_LINE" << std::endl; break;
      case osgText::Text::RIGHT_BOTTOM_BASE_LINE: fw << "RIGHT_BOTTOM_BASE_LINE" << std::endl; break;
    };


    if (!text.getRotation().zeroRotation())
    {
        fw.indent() << "rotation " << text.getRotation() << std::endl;
    }

    if (text.getAutoRotateToScreen())
    {
        fw.indent() << "autoRotateToScreen TRUE"<< std::endl;
    }


    // layout
    fw.indent() << "layout ";
    switch(text.getLayout())
    {
      case osgText::Text::LEFT_TO_RIGHT: fw << "LEFT_TO_RIGHT" << std::endl; break;
      case osgText::Text::RIGHT_TO_LEFT: fw << "RIGHT_TO_LEFT" << std::endl; break;
      case osgText::Text::VERTICAL: fw << "VERTICAL" << std::endl; break;
    };


    // position
    osg::Vec3 p = text.getPosition();
    fw.indent() << "position " << p.x() << " " << p.y() << " " << p.z() << std::endl;

    // color
//    osg::Vec4 c = text.getColor();
//    fw.indent() << "color " << c.x() << " " << c.y() << " " << c.z() << " " << c.w() << std::endl;

    // draw mode
    fw.indent() << "drawMode " << text.getDrawMode() << std::endl;

    // bounding box margin
    fw.indent() << "BoundingBoxMargin " << text.getBoundingBoxMargin() << std::endl;

    // bounding box color
    osg::Vec4 bbc = text.getBoundingBoxColor();
    fw.indent() << "BoundingBoxColor " << bbc.x() << " " << bbc.y() << " " << bbc.z() << " " << bbc.w() << std::endl;

    // text
    const osgText::String& textstring = text.getText();
    bool isACString = true;
    osgText::String::const_iterator itr;
    for(itr=textstring.begin();
        itr!=textstring.end() && isACString;
        ++itr)
    {
        if (*itr==0 || *itr>256) isACString=false;
    }
    if (isACString)
    {
        std::string str;

        for(itr=textstring.begin();
            itr!=textstring.end();
            ++itr)
        {
            str += (char)(*itr);
        }

        //std::copy(textstring.begin(),textstring.end(),std::back_inserter(str));

        fw.indent() << "text " << fw.wrapString(str) << std::endl;
    }
    else
    {
        // do it the hardway...output each character as an int
        fw.indent() << "text "<<textstring.size()<<std::endl;;
        osgDB::writeArray(fw,textstring.begin(),textstring.end());
    }

    return true;
}
