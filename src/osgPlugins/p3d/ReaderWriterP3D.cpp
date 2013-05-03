/* -*-c++-*- Present3D - Copyright (C) 1999-2006 Robert Osfield
 *
 * This software is open source and may be redistributed and/or modified under
 * the terms of the GNU General Public License (GPL) version 2.0.
 * The full license is in LICENSE.txt file included with this distribution,.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * include LICENSE.txt for more details.
*/

#include <osg/Notify>
#include <osg/io_utils>
#include <osg/PagedLOD>

#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include <osgWidget/PdfReader>

#include <osgPresentation/SlideShowConstructor>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <osgDB/XmlParser>

#include <sstream>
#include <iostream>


/**
 * OpenSceneGraph plugin wrapper/converter.
 */
class ReaderWriterP3DXML : public osgDB::ReaderWriter
{
public:
    ReaderWriterP3DXML()
    {
        _colorMap["WHITE"]  .set(1.0f,1.0f,1.0f,1.0f);
        _colorMap["BLACK"]  .set(0.0f,0.0f,0.0f,1.0f);
        _colorMap["PURPLE"] .set(1.0f,0.0f,1.0f,1.0f);
        _colorMap["BLUE"]   .set(0.0f,0.0f,1.0f,1.0f);
        _colorMap["RED"]    .set(1.0f,0.0f,0.0f,1.0f);
        _colorMap["CYAN"]   .set(0.0f,1.0f,1.0f,1.0f);
        _colorMap["YELLOW"] .set(1.0f,1.0f,0.0f,1.0f);
        _colorMap["GREEN"]  .set(0.0f,1.0f,0.0f,1.0f);
        _colorMap["SKY"]    .set(0.2f, 0.2f, 0.4f, 1.0f);

        _layoutMap["LEFT_TO_RIGHT"] = osgText::Text::LEFT_TO_RIGHT;
        _layoutMap["RIGHT_TO_LEFT"] = osgText::Text::RIGHT_TO_LEFT;
        _layoutMap["VERTICAL"] = osgText::Text::VERTICAL;

        _alignmentMap["LEFT_TOP"] = osgText::Text::LEFT_TOP;
        _alignmentMap["LEFT_CENTER"] = osgText::Text::LEFT_CENTER;
        _alignmentMap["LEFT_BOTTOM"] = osgText::Text::LEFT_BOTTOM;

        _alignmentMap["CENTER_TOP"] = osgText::Text::CENTER_TOP;
        _alignmentMap["CENTER_CENTER"] = osgText::Text::CENTER_CENTER;
        _alignmentMap["CENTER_BOTTOM"] = osgText::Text::CENTER_BOTTOM;

        _alignmentMap["RIGHT_TOP"] = osgText::Text::RIGHT_TOP;
        _alignmentMap["RIGHT_CENTER"] = osgText::Text::RIGHT_CENTER;
        _alignmentMap["RIGHT_BOTTOM"] = osgText::Text::RIGHT_BOTTOM;

        _alignmentMap["LEFT_BASE_LINE"] = osgText::Text::LEFT_BASE_LINE;
        _alignmentMap["CENTER_BASE_LINE"] = osgText::Text::CENTER_BASE_LINE;
        _alignmentMap["RIGHT_BASE_LINE"] = osgText::Text::RIGHT_BASE_LINE;
        _alignmentMap["BASE_LINE"] = osgText::Text::LEFT_BASE_LINE;

        _characterSizeModeMap["OBJECT_COORDS"] = osgText::Text::OBJECT_COORDS;
        _characterSizeModeMap["SCREEN_COORDS"] = osgText::Text::SCREEN_COORDS;
        _characterSizeModeMap["OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT"] = osgText::Text::OBJECT_COORDS_WITH_MAXIMUM_SCREEN_SIZE_CAPPED_BY_FONT_HEIGHT;

        _stringKeyMap["Home"]=' ';
        _stringKeyMap["Start"]= osgGA::GUIEventAdapter::KEY_Home;
        _stringKeyMap["Next"]= osgGA::GUIEventAdapter::KEY_Page_Down;
        _stringKeyMap["Previous"]=osgGA::GUIEventAdapter::KEY_Page_Up;
        _stringKeyMap["Up"]=osgGA::GUIEventAdapter::KEY_Up;
        _stringKeyMap["Down"]=osgGA::GUIEventAdapter::KEY_Down;
        _stringKeyMap["End"]=osgGA::GUIEventAdapter::KEY_End;
        _stringKeyMap["Page Down"]=osgGA::GUIEventAdapter::KEY_Page_Down;
        _stringKeyMap["Page Up"]=osgGA::GUIEventAdapter::KEY_Page_Up;
        _stringKeyMap["F1"]=osgGA::GUIEventAdapter::KEY_F1;
        _stringKeyMap["F2"]=osgGA::GUIEventAdapter::KEY_F2;
        _stringKeyMap["F3"]=osgGA::GUIEventAdapter::KEY_F3;
        _stringKeyMap["F4"]=osgGA::GUIEventAdapter::KEY_F4;
        _stringKeyMap["F5"]=osgGA::GUIEventAdapter::KEY_F5;
        _stringKeyMap["F6"]=osgGA::GUIEventAdapter::KEY_F6;
        _stringKeyMap["F7"]=osgGA::GUIEventAdapter::KEY_F7;
        _stringKeyMap["F8"]=osgGA::GUIEventAdapter::KEY_F8;
        _stringKeyMap["F9"]=osgGA::GUIEventAdapter::KEY_F9;
        _stringKeyMap["F10"]=osgGA::GUIEventAdapter::KEY_F10;
        _stringKeyMap["F11"]=osgGA::GUIEventAdapter::KEY_F11;
        _stringKeyMap["F12"]=osgGA::GUIEventAdapter::KEY_F12;


        _notifyLevel = osg::INFO;
    }

    virtual const char* className() const
    {
        return "present3D XML Reader/Writer";
    }

    virtual bool acceptsExtension(const std::string& extension) const
    {
        return osgDB::equalCaseInsensitive(extension,"p3d") ||
               osgDB::equalCaseInsensitive(extension,"xml") ;
    }

    virtual ReadResult readNode(const std::string& fileName,
                                const osgDB::ReaderWriter::Options* options) const;

    virtual ReadResult readNode(std::istream& fin, const Options* options) const;

    ReadResult readNode(osgDB::XmlNode::Input& input, osgDB::ReaderWriter::Options* options) const;

    osg::Node* parseXmlGraph(osgDB::XmlNode* root, bool readOnlyHoldingPage, osgDB::Options* options) const;

    bool parseProperties(osgDB::XmlNode* root, osg::UserDataContainer& udc) const;
    bool parsePropertyAnimation(osgDB::XmlNode* root, osgPresentation::PropertyAnimation& pa) const;

    void parseModel(osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode*cur) const;

    osg::TransferFunction1D* readTransferFunctionFile(const std::string& filename, float scale) const;
    void parseVolume(osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode*cur) const;

    void parseStereoPair(osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode*cur) const;

    void parseTimeout(osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode*cur) const;

    bool parseLayerChild(osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode* cur, float& totalIndent) const;

    void parseLayer(osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode*cur) const;

    void parseBullets(osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode*cur, bool inheritPreviousLayers, bool defineAsBaseLayer) const;
    void parseText(osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode*cur, bool inheritPreviousLayers, bool defineAsBaseLayer) const;

    void parsePage (osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode*cur) const;

    void parseSlide (osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode*cur, bool parseTitles=true, bool parseLayers=true) const;

    void parsePdfDocument (osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode*cur) const;

    osg::Vec4 mapStringToColor(const std::string& str) const
    {
        ColorMap::const_iterator itr=_colorMap.find(str);
        if (itr!=_colorMap.end()) return itr->second;
        osg::Vec4 color;
        if (read(str,color)) return color;
        else return osg::Vec4(0.0f,0.0f,0.0f,1.0f);
    }

    inline osg::Vec4 accumulateRotation(const osg::Vec4& lhs, const osg::Vec4& rhs) const
    {
        osg::Quat qlhs,qrhs;
        qlhs.makeRotate(osg::DegreesToRadians(lhs[0]),lhs[1],lhs[2],lhs[3]);
        qrhs.makeRotate(osg::DegreesToRadians(rhs[0]),rhs[1],rhs[2],rhs[3]);
        osg::Quat quat = qlhs*qrhs;
        osg::Vec4d result;
        quat.getRotate ( result[0], result[1], result[2], result[3]);
        result[0] = osg::RadiansToDegrees(result[0]);
        return result;
    }

    inline bool read(const char* str, int& value) const;
    inline bool read(const char* str, float& value) const;
    inline bool read(const char* str, double& value) const;
    inline bool read(const char* str, int numberValues, float* values) const;
    inline bool read(const char* str, osg::Vec2& value) const;
    inline bool read(const char* str, osg::Vec3& value) const;
    inline bool read(const char* str, osg::Vec4& value) const;

    inline bool read(const std::string& str, int& value) const;
    inline bool read(const std::string& str, float& value) const;
    inline bool read(const std::string& str, double& value) const;
    inline bool read(const std::string& str, int numberValues, float* values) const;
    inline bool read(const std::string& str, osg::Vec2& value) const;
    inline bool read(const std::string& str, osg::Vec3& value) const;
    inline bool read(const std::string& str, osg::Vec4& value) const;

    bool getProperty(osgDB::XmlNode*cur, const char* token) const;
    bool getKeyProperty(osgDB::XmlNode*cur, const char* token, int& value) const;
    bool getProperty(osgDB::XmlNode*cur, const char* token, int& value) const;
    bool getProperty(osgDB::XmlNode*cur, const char* token, float& value) const;
    bool getProperty(osgDB::XmlNode*cur, const char* token, double& value) const;
    bool getProperty(osgDB::XmlNode*cur, const char* token, int numberValues, float* values) const;
    bool getProperty(osgDB::XmlNode*cur, const char* token, osg::Vec2& value) const;
    bool getProperty(osgDB::XmlNode*cur, const char* token, osg::Vec3& value) const;
    bool getProperty(osgDB::XmlNode*cur, const char* token, osg::Vec4& value) const;
    bool getProperty(osgDB::XmlNode*cur, const char* token, std::string& value) const;
    bool getTrimmedProperty(osgDB::XmlNode*cur, const char* token, std::string& value) const;
    bool getProperty(osgDB::XmlNode*cur, const char* token, osgText::Text::Layout& value) const;
    bool getProperty(osgDB::XmlNode*cur, const char* token, osgText::Text::AlignmentType& value) const;
    bool getProperty(osgDB::XmlNode*cur, const char* token, osgText::Text::CharacterSizeMode& value) const;

    bool getProperties(osgDB::XmlNode*cur, osgPresentation::SlideShowConstructor::PositionData& value) const;
    bool getProperties(osgDB::XmlNode*cur, osgPresentation::SlideShowConstructor::FontData& value) const;
    bool getProperties(osgDB::XmlNode*cur, osgPresentation::SlideShowConstructor::ModelData& value) const;
    bool getProperties(osgDB::XmlNode*cur, osgPresentation::SlideShowConstructor::ImageData& value) const;

    bool getJumpProperties(osgDB::XmlNode*cur, osgPresentation::JumpData& jumpData) const;

    bool getKeyPositionInner(osgDB::XmlNode*cur, osgPresentation::KeyPosition& keyPosition) const;
    bool getKeyPosition(osgDB::XmlNode*cur, osgPresentation::KeyPosition& keyPosition) const;

    typedef std::map<std::string,osg::Vec4> ColorMap;
    typedef std::map<std::string,osgText::Text::Layout> LayoutMap;
    typedef std::map<std::string,osgText::Text::AlignmentType> AlignmentMap;
    typedef std::map<std::string,osgText::Text::CharacterSizeMode> CharacterSizeModeMap;
    typedef std::map<std::string, unsigned int> StringKeyMap;

    std::string expandEnvVarsInFileName(const std::string& filename) const;


    ColorMap                _colorMap;
    LayoutMap               _layoutMap;
    AlignmentMap            _alignmentMap;
    CharacterSizeModeMap    _characterSizeModeMap;
    StringKeyMap            _stringKeyMap;

    typedef std::map<std::string, osg::ref_ptr<osgDB::XmlNode> > TemplateMap;
    mutable TemplateMap _templateMap;

    osg::NotifySeverity _notifyLevel;

};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(p3d, ReaderWriterP3DXML)

std::string ReaderWriterP3DXML::expandEnvVarsInFileName(const std::string& filename) const
{
    std::string argument(filename);
    std::string::size_type start_pos = argument.find("${");

    while (start_pos != std::string::npos)
    {
        std::string::size_type end_pos = argument.find("}",start_pos);
        if (start_pos != std::string::npos)
        {
            std::string var = argument.substr(start_pos+2, end_pos-start_pos-2);
            const char* str = getenv(var.c_str());
            if (str)
            {
                argument.erase(start_pos, end_pos-start_pos+1);
                argument.insert(start_pos, str);
            }
            start_pos = argument.find("${",end_pos);
        }
        else
        {
            start_pos = std::string::npos;
        }
    }

    return argument;
}

bool ReaderWriterP3DXML::read(const char* str, int& value) const
{
    if (!str) return false;
    std::istringstream iss((const char*)str);
    iss >> value;
    return !iss.fail();
}

bool ReaderWriterP3DXML::read(const char* str, float& value) const
{
    if (!str) return false;
    std::istringstream iss((const char*)str);
    iss >> value;
    return !iss.fail();
}

bool ReaderWriterP3DXML::read(const char* str, double& value) const
{
    if (!str) return false;
    std::istringstream iss((const char*)str);
    iss >> value;
    return !iss.fail();
}

bool ReaderWriterP3DXML::read(const char* str, int numberValues, float* values) const
{
    if (!str) return false;
    std::istringstream iss((const char*)str);
    for(int i=0; i<numberValues && !iss.fail(); i++)
    {
        iss >> *values;
        ++values;
    }
    return !iss.fail();
}

bool ReaderWriterP3DXML::read(const char* str, osg::Vec2& value) const
{
    if (!str) return false;
    std::istringstream iss((const char*)str);
    iss >> value.x() >> value.y();
    return !iss.fail();
}

bool ReaderWriterP3DXML::read(const char* str, osg::Vec3& value) const
{
    if (!str) return false;
    std::istringstream iss((const char*)str);
    iss >> value.x() >> value.y() >> value.z();
    return !iss.fail();
}

bool ReaderWriterP3DXML::read(const char* str, osg::Vec4& value) const
{
    if (!str) return false;
    std::istringstream iss((const char*)str);
    iss >> value.x() >> value.y() >> value.z() >> value.w();
    return !iss.fail();
}

bool ReaderWriterP3DXML::read(const std::string& str, int& value) const
{
    std::istringstream iss(str);
    iss >> value;
    return !iss.fail();
}

bool ReaderWriterP3DXML::read(const std::string& str, float& value) const
{
    std::istringstream iss(str);
    iss >> value;
    return !iss.fail();
}

bool ReaderWriterP3DXML::read(const std::string& str, double& value) const
{
    std::istringstream iss(str);
    iss >> value;
    return !iss.fail();
}


bool ReaderWriterP3DXML::read(const std::string& str, int numberValues, float* values) const
{
    std::istringstream iss(str);
    for(int i=0; i<numberValues && !iss.fail(); i++)
    {
        iss >> *values;
        ++values;
    }
    return !iss.fail();
}

bool ReaderWriterP3DXML::read(const std::string& str, osg::Vec2& value) const
{
    std::istringstream iss(str);
    iss >> value.x() >> value.y();
    return !iss.fail();
}

bool ReaderWriterP3DXML::read(const std::string& str, osg::Vec3& value) const
{
    std::istringstream iss(str);
    iss >> value.x() >> value.y() >> value.z();
    return !iss.fail();
}

bool ReaderWriterP3DXML::read(const std::string& str, osg::Vec4& value) const
{
    std::istringstream iss(str);
    iss >> value.x() >> value.y() >> value.z() >> value.w();
    return !iss.fail();
}

bool ReaderWriterP3DXML::getProperty(osgDB::XmlNode* cur, const char* token) const
{
    return cur->properties.count(token)!=0;
}

bool ReaderWriterP3DXML::getProperty(osgDB::XmlNode*cur, const char* token, int& value) const
{
    osgDB::XmlNode::Properties::iterator itr = cur->properties.find(token);
    if (itr==cur->properties.end()) return false;
    return read(itr->second,value);
}

bool ReaderWriterP3DXML::getKeyProperty(osgDB::XmlNode*cur, const char* token, int& value) const
{
    osgDB::XmlNode::Properties::iterator itr = cur->properties.find(token);
    if (itr==cur->properties.end()) return false;

    OSG_NOTICE<<"getKeyProperty()="<<itr->second<<std::endl;

    if (itr->second.empty())
    {
        OSG_NOTICE<<"   empty()"<<std::endl;
        return false;
    }
    
    if (itr->second.find("0x",0,2)!=std::string::npos)
    {
        std::istringstream iss(itr->second);
        iss>>std::hex>>value;
        OSG_NOTICE<<"ReaderWriterP3DXML::getKeyProperty() hex result = "<<value<<std::endl;
        return true;
    }
    else if (itr->second.size()>1 && (itr->second[0]>='0' && itr->second[0]<='9'))
    {
        std::istringstream iss(itr->second);
        iss>>value;
        OSG_NOTICE<<"ReaderWriterP3DXML::getKeyProperty() numeric result = "<<value<<std::endl;
        return true;
    }
    else
    {
        value = itr->second[0];
        OSG_NOTICE<<"ReaderWriterP3DXML::getKeyProperty() alphanumeric result = "<<value<<std::endl;
        return true;
    }
}

bool ReaderWriterP3DXML::getProperty(osgDB::XmlNode*cur, const char* token, float& value) const
{
    osgDB::XmlNode::Properties::iterator itr = cur->properties.find(token);
    if (itr==cur->properties.end()) return false;
    return read(itr->second,value);
}

bool ReaderWriterP3DXML::getProperty(osgDB::XmlNode*cur, const char* token, double& value) const
{
    osgDB::XmlNode::Properties::iterator itr = cur->properties.find(token);
    if (itr==cur->properties.end()) return false;
    return read(itr->second,value);
}

bool ReaderWriterP3DXML::getProperty(osgDB::XmlNode*cur, const char* token, int numberValues, float* values) const
{
    osgDB::XmlNode::Properties::iterator itr = cur->properties.find(token);
    if (itr==cur->properties.end()) return false;
    return read(itr->second, numberValues, values);
}

bool ReaderWriterP3DXML::getProperty(osgDB::XmlNode*cur, const char* token, osg::Vec2& value) const
{
    osgDB::XmlNode::Properties::iterator itr = cur->properties.find(token);
    if (itr==cur->properties.end()) return false;
    return read(itr->second,value);
}

bool ReaderWriterP3DXML::getProperty(osgDB::XmlNode*cur, const char* token, osg::Vec3& value) const
{
    osgDB::XmlNode::Properties::iterator itr = cur->properties.find(token);
    if (itr==cur->properties.end()) return false;
    return read(itr->second,value);
}

bool ReaderWriterP3DXML::getProperty(osgDB::XmlNode*cur, const char* token, osg::Vec4& value) const
{
    osgDB::XmlNode::Properties::iterator itr = cur->properties.find(token);
    if (itr==cur->properties.end()) return false;
    return read(itr->second,value);
}

bool ReaderWriterP3DXML::getProperty(osgDB::XmlNode*cur, const char* token, std::string& value) const
{
    osgDB::XmlNode::Properties::iterator itr = cur->properties.find(token);
    if (itr==cur->properties.end()) return false;
    value = itr->second;
    return true;
}

bool ReaderWriterP3DXML::getTrimmedProperty(osgDB::XmlNode*cur, const char* token, std::string& value) const
{
    osgDB::XmlNode::Properties::iterator itr = cur->properties.find(token);
    if (itr==cur->properties.end()) return false;
    value = osgDB::trimEnclosingSpaces(itr->second);
    return true;
}

bool ReaderWriterP3DXML::getProperty(osgDB::XmlNode*cur, const char* token, osgText::Text::Layout& value) const
{
    osgDB::XmlNode::Properties::iterator pitr = cur->properties.find(token);
    if (pitr==cur->properties.end()) return false;

    const std::string& str = pitr->second;
    LayoutMap::const_iterator itr = _layoutMap.find(str);
    if (itr!=_layoutMap.end())
    {
        value = itr->second;
    }
    return true;
}

bool ReaderWriterP3DXML::getProperty(osgDB::XmlNode*cur, const char* token, osgText::Text::AlignmentType& value) const
{
    osgDB::XmlNode::Properties::iterator pitr = cur->properties.find(token);
    if (pitr==cur->properties.end()) return false;

    const std::string& str = pitr->second;
    AlignmentMap::const_iterator itr = _alignmentMap.find(str);
    if (itr!=_alignmentMap.end())
    {
        value = itr->second;
    }
    return true;
}

bool ReaderWriterP3DXML::getProperty(osgDB::XmlNode*cur, const char* token, osgText::Text::CharacterSizeMode& value) const
{
    osgDB::XmlNode::Properties::iterator pitr = cur->properties.find(token);
    if (pitr==cur->properties.end()) return false;

    const std::string& str = pitr->second;
    CharacterSizeModeMap::const_iterator itr = _characterSizeModeMap.find(str);
    if (itr!=_characterSizeModeMap.end())
    {
        value = itr->second;
    }
    return true;
}

bool ReaderWriterP3DXML::getProperties(osgDB::XmlNode*cur, osgPresentation::SlideShowConstructor::PositionData& value) const
{
    bool propertiesRead=false;

    osg::Vec3 position(0.0f,1.0f,0.0f);
    osg::Vec4 rotate(0.0f,0.0f,0.0f,1.0f);
    float scale = 1.0f;

    osg::Vec4 rotation(0.0f,0.0f,0.0f,1.0f);

    // temporary
    std::string str;

    if (getProperty(cur, "coordinate_frame", str))
    {
        propertiesRead = true;

        if (str=="model") value.frame = osgPresentation::SlideShowConstructor::MODEL;
        else if (str=="slide") value.frame = osgPresentation::SlideShowConstructor::SLIDE;
        else OSG_NOTIFY(_notifyLevel)<<"Parser error - coordinate_frame=\""<<str<<"\" unrecongonized value"<<std::endl;

        OSG_NOTIFY(_notifyLevel)<<"read coordinate_frame "<< ((value.frame==osgPresentation::SlideShowConstructor::MODEL) ? "osgPresentation::SlideShowConstructor::MODEL" : "osgPresentation::SlideShowConstructor::SLIDE")<<std::endl;
    }

    if (value.frame==osgPresentation::SlideShowConstructor::SLIDE)
    {

        if (getProperty(cur, "position", str))
        {
            value.position.set(0.5,0.5,0.0);

            propertiesRead = true;

            osg::Vec2 vec2;
            osg::Vec3 vec3;

            bool fail = false;
            if (str=="center") value.position.set(0.5f,.5f,0.0f);
            else if (str=="eye") value.position.set(0.0f,0.0f,1.0f);
            else if (read(str,vec3)) value.position = vec3;
            else if (read(str,vec2)) value.position.set(vec3.x(),vec3.y(),0.0f);
            else fail = true;

            if (fail) { OSG_NOTIFY(_notifyLevel)<<"Parser error - position=\""<<str<<"\" unrecongonized value"<<std::endl; }
            else { OSG_NOTIFY(_notifyLevel)<<"Read position="<<value.position<<std::endl; }
        }
    }
    else // value.frame==osgPresentation::SlideShowConstructor::MODEL
    {

        if (getProperty(cur, "position", str))
        {
            value.position.set(0.0,0.0,0.0);

            propertiesRead = true;

            bool fail = false;
            if (str=="center") value.position.set(0.0f,1.0f,0.0f);
            else if (str=="eye") value.position.set(0.0f,0.0f,0.0f);
            else if (!read(str,value.position)) fail = true;

            if (fail) { OSG_NOTIFY(_notifyLevel)<<"Parser error - position=\""<<str<<"\" unrecongonized value"<<std::endl; }
            else { OSG_NOTIFY(_notifyLevel)<<"Read position="<<value.position<<std::endl; }
        }
    }


    if (getProperty(cur, "scale", scale))
    {
        value.scale.set(scale,scale,scale);
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"scale read "<<scale<<std::endl;
    }

    if (getProperty(cur, "scale_x", scale) || getProperty(cur, "width", scale))
    {
        value.scale.x() = scale;
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"scale read_x "<<scale<<std::endl;
    }

    if (getProperty(cur, "scale_y", scale) || getProperty(cur, "height", scale))
    {
        value.scale.y() = scale;
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"scale read_y "<<scale<<std::endl;
    }

    if (getProperty(cur, "scale_z", scale))
    {
        value.scale.z() = scale;
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"scale read_z "<<scale<<std::endl;
    }

    if (getProperty(cur, "rotate", rotate))
    {
        value.rotate = rotate;
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"rotate read "<<rotate<<std::endl;
    }

    if (getProperty(cur, "rotate1", rotate))
    {
        // note may need to reverse once Quat * order is sorted out.
        value.rotate = accumulateRotation(rotate,value.rotate);
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"rotate1 read "<<rotate<<std::endl;
    }

    if (getProperty(cur, "rotate2", rotate))
    {
        // note may need to reverse once Quat * order is sorted out.
        value.rotate = accumulateRotation(rotate,value.rotate);
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"rotate1 read "<<rotate<<std::endl;
    }

    if (getProperty(cur, "rotate3", rotate))
    {
        // note may need to reverse once Quat * order is sorted out.
        value.rotate = accumulateRotation(rotate,value.rotate);
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"rotate1 read "<<rotate<<std::endl;
    }

    if (getProperty(cur, "rotation", rotation))
    {
        value.rotation = rotation;
        OSG_NOTIFY(_notifyLevel)<<"rotation read "<<rotation<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "rotation1", rotation))
    {
        value.rotation = accumulateRotation(rotation,value.rotation);
        OSG_NOTIFY(_notifyLevel)<<"rotation1 read "<<rotation<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "rotation2", rotation))
    {
        value.rotation = accumulateRotation(rotation,value.rotation);
        OSG_NOTIFY(_notifyLevel)<<"rotation2 read "<<rotation<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "rotation3", rotation))
    {
        value.rotation = accumulateRotation(rotation,value.rotation);
        OSG_NOTIFY(_notifyLevel)<<"rotation3 read "<<rotation<<std::endl;
        propertiesRead = true;
    }

    if (getTrimmedProperty(cur, "path", str))
    {

        value.absolute_path = false;
        value.inverse_path = false;
        value.path = expandEnvVarsInFileName(str);

        OSG_NOTIFY(_notifyLevel)<<"path read "<<str<<std::endl;
        propertiesRead = true;
    }

    if (getTrimmedProperty(cur, "camera_path", str))
    {
        value.absolute_path = true;
        value.inverse_path = true;
        value.path = expandEnvVarsInFileName(str);

        OSG_NOTIFY(_notifyLevel)<<"camera path read "<<str<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "path_time_offset", value.path_time_offset))
    {
        OSG_NOTIFY(_notifyLevel)<<"read path_time_offset"<<value.path_time_offset<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "path_time_multiplier", value.path_time_multiplier))
    {
        OSG_NOTIFY(_notifyLevel)<<"read path_time_multiplier"<<value.path_time_multiplier<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "animation_material_time_offset", value.animation_material_time_offset))
    {
        OSG_NOTIFY(_notifyLevel)<<"read animation_material_time_offset"<<value.animation_material_time_offset<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "animation_material_time_multiplier", value.animation_material_time_multiplier))
    {
        OSG_NOTIFY(_notifyLevel)<<"read animation_material_time_multiplier"<<value.animation_material_time_multiplier<<std::endl;
        propertiesRead = true;
    }

    if (getTrimmedProperty(cur, "animation_material", str))
    {
        value.animation_material_filename = str;

        OSG_NOTIFY(_notifyLevel)<<"animation_material read "<<str<<std::endl;
        propertiesRead = true;
    }

    if (getTrimmedProperty(cur, "animation_name", str))
    {
        value.animation_name = str;

        OSG_NOTIFY(_notifyLevel)<<"animation_name "<<str<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "fade", value.fade))
    {
        OSG_NOTIFY(_notifyLevel)<<"fade "<<value.fade<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "path_loop_mode", str))
    {
        OSG_NOTIFY(_notifyLevel)<<"path_loop_mode "<<str<<std::endl;
        if (str=="LOOP") value.path_loop_mode=osg::AnimationPath::LOOP;
        else if (str=="SWING") value.path_loop_mode=osg::AnimationPath::SWING;
        else if (str=="NO_LOOPING") value.path_loop_mode=osg::AnimationPath::NO_LOOPING;
        propertiesRead = true;
    }

    if (getProperty(cur, "animation_material_loop_mode", str))
    {
        OSG_NOTIFY(_notifyLevel)<<"animation_material_loop_mode "<<str<<std::endl;
        if (str=="LOOP") value.animation_material_loop_mode=osgPresentation::AnimationMaterial::LOOP;
        else if (str=="SWING") value.animation_material_loop_mode=osgPresentation::AnimationMaterial::SWING;
        else if (str=="NO_LOOPING") value.animation_material_loop_mode=osgPresentation::AnimationMaterial::NO_LOOPING;
        propertiesRead = true;
    }

    if (getProperty(cur, "billboard", str))
    {
        value.autoRotate = (str != "off" && str != "Off" && str != "OFF");
        OSG_NOTIFY(_notifyLevel)<<"billboard, str="<<str<<", autoRotate="<<value.autoRotate<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "scale_to_screen",str))
    {
        value.autoScale = (str != "off" && str != "Off" && str != "OFF");
        OSG_NOTIFY(_notifyLevel)<<"scale-to-screen, str="<<str<<", autoRotate="<<value.autoScale<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "hud", str))
    {
        value.hud = (str != "off" && str != "Off" && str != "OFF");
        OSG_NOTIFY(_notifyLevel)<<"hud, str="<<str<<", hud="<<value.hud<<std::endl;
        propertiesRead = true;
    }


    return propertiesRead;
}

bool ReaderWriterP3DXML::getProperties(osgDB::XmlNode*cur, osgPresentation::SlideShowConstructor::FontData& value) const
{
    bool propertiesRead=false;

    OSG_NOTIFY(_notifyLevel)<<"in getProperties(FontData)"<<std::endl;

    if (getProperty(cur, "font", value.font))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read font \""<<value.font<<"\""<<std::endl;
    }

    if (getProperty(cur, "character_size", value.characterSize))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read height \""<<value.characterSize<<"\""<<std::endl;
    }

    if (getProperty(cur, "character_size_mode", value.characterSizeMode))
    {
        propertiesRead = true;

        OSG_NOTIFY(_notifyLevel)<<"read character_size_mode \""<<value.characterSizeMode<<"\""<<std::endl;
    }

    if (getProperty(cur, "layout", value.layout))
    {
        propertiesRead = true;

        OSG_NOTIFY(_notifyLevel)<<"read layout \""<<value.layout<<"\""<<std::endl;
    }

    if (getProperty(cur, "alignment", value.alignment))
    {
        propertiesRead = true;

        OSG_NOTIFY(_notifyLevel)<<"read alignment \""<<value.alignment<<"\""<<std::endl;
    }

    return propertiesRead;
}

bool ReaderWriterP3DXML::getProperties(osgDB::XmlNode*cur, osgPresentation::SlideShowConstructor::ModelData& value) const
{
    bool propertiesRead=false;

    OSG_NOTIFY(_notifyLevel)<<"in getProperties(ModelData)"<<std::endl;

    if (getProperty(cur, "region", value.region))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read region \""<<value.region<<"\""<<std::endl;
    }

    if (getProperty(cur, "effect", value.effect))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read effect \""<<value.effect<<"\""<<std::endl;
    }

    if (getProperty(cur, "options", value.options))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read options \""<<value.options<<"\""<<std::endl;
    }

    return propertiesRead;
}

bool ReaderWriterP3DXML::getProperties(osgDB::XmlNode*cur, osgPresentation::SlideShowConstructor::ImageData& value) const
{
    bool propertiesRead=false;

    OSG_NOTIFY(_notifyLevel)<<"in getProperties(ImageData)"<<std::endl;

    if (getProperty(cur, "page", value.page))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read page \""<<value.page<<"\""<<std::endl;
    }

    if (getProperty(cur, "options", value.options))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read options \""<<value.options<<"\""<<std::endl;
    }

    osg::Vec4 bgColour;
    if (getProperty(cur, "background", value.backgroundColor))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read background colour \""<<value.backgroundColor<<"\""<<std::endl;
    }

    if (getProperty(cur, "width", value.width))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read width \""<<value.width<<"\""<<std::endl;
    }

    if (getProperty(cur, "height", value.height))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read height \""<<value.height<<"\""<<std::endl;
    }

    if (getProperty(cur, "region", value.region))
    {
        propertiesRead = true;
        value.region_in_pixel_coords = false;
        OSG_NOTIFY(_notifyLevel)<<"read region \""<<value.region<<"\""<<std::endl;
    }

    if (getProperty(cur, "pixel_region", value.region))
    {
        propertiesRead = true;
        value.region_in_pixel_coords = true;
        OSG_NOTIFY(_notifyLevel)<<"read pixel_region \""<<value.region<<"\""<<std::endl;
    }

    std::string str;
    if (getProperty(cur, "looping", str))
    {
        propertiesRead = true;
        if (str=="ON") value.loopingMode = osg::ImageStream::LOOPING;
        else value.loopingMode = osg::ImageStream::NO_LOOPING;
        OSG_NOTIFY(_notifyLevel)<<"looping \""<<str<<"\""<<std::endl;
    }

    if (getProperty(cur, "fps", value.fps))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read fps \""<<value.fps<<"\""<<std::endl;
    }

    if (getProperty(cur, "duration", value.duration))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read duration \""<<value.duration<<"\""<<std::endl;
    }

    if (getProperty(cur, "paging_mode", str))
    {
        propertiesRead = true;
        if (str=="PRE_LOAD_ALL_IMAGES") value.imageSequencePagingMode = osg::ImageSequence::PRE_LOAD_ALL_IMAGES;
        else if (str=="PAGE_AND_RETAIN_IMAGES") value.imageSequencePagingMode = osg::ImageSequence::PAGE_AND_RETAIN_IMAGES;
        else if (str=="PAGE_AND_DISCARD_USED_IMAGES") value.imageSequencePagingMode = osg::ImageSequence::PAGE_AND_DISCARD_USED_IMAGES;
        else if (str=="LOAD_AND_RETAIN_IN_UPDATE_TRAVERSAL") value.imageSequencePagingMode = osg::ImageSequence::LOAD_AND_RETAIN_IN_UPDATE_TRAVERSAL;
        else if (str=="LOAD_AND_DISCARD_IN_UPDATE_TRAVERSAL") value.imageSequencePagingMode = osg::ImageSequence::LOAD_AND_DISCARD_IN_UPDATE_TRAVERSAL;

        OSG_NOTIFY(_notifyLevel)<<"read imageSequencePagingMode \""<<value.imageSequencePagingMode<<"\""<<std::endl;
    }

    if (getProperty(cur, "interaction_mode", str))
    {
        propertiesRead = true;

        if (str=="PLAY_AUTOMATICALLY_LIKE_MOVIE") value.imageSequenceInteractionMode = osgPresentation::SlideShowConstructor::ImageData::PLAY_AUTOMATICALLY_LIKE_MOVIE;
        else if (str=="USE_MOUSE_X_POSITION") value.imageSequenceInteractionMode = osgPresentation::SlideShowConstructor::ImageData::USE_MOUSE_X_POSITION;
        else if (str=="USE_MOUSE_Y_POSITION") value.imageSequenceInteractionMode = osgPresentation::SlideShowConstructor::ImageData::USE_MOUSE_Y_POSITION;

        OSG_NOTIFY(_notifyLevel)<<"read imageSequencePagingMode \""<<value.imageSequenceInteractionMode<<"\""<<std::endl;
    }

    if (getProperty(cur, "blending", str))
    {
        propertiesRead = true;

        if (str=="ON" || str=="On" || str=="on" || str=="enable" ) value.blendingHint = osgPresentation::SlideShowConstructor::ImageData::ON;
        else if (str=="OFF" || str=="Off" || str=="off" || str=="disable" ) value.blendingHint = osgPresentation::SlideShowConstructor::ImageData::OFF;

        OSG_NOTIFY(_notifyLevel)<<"read blendingHint \""<<value.blendingHint<<"\""<<std::endl;
    }

    if (getProperty(cur, "delay", value.delayTime))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read delay \""<<value.delayTime<<"\""<<std::endl;
    }

    if (getProperty(cur, "start", value.startTime))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read start \""<<value.startTime<<"\""<<std::endl;
    }

    if (getProperty(cur, "stop", value.stopTime))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read stop \""<<value.stopTime<<"\""<<std::endl;
    }

    if (getProperty(cur, "volume", value.volume))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read volume \""<<value.volume<<"\""<<std::endl;
    }

    /*
    if (getProperty(cur, "texcoord_offset", value.texcoord_offset))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read offset \""<<value.texcoord_offset<<"\""<<std::endl;
    }

    if (getProperty(cur, "texcoord_scale", value.texcoord_scale))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read texcoord_scale \""<<value.texcoord_scale<<"\""<<std::endl;
    }

    if (getProperty(cur, "texcoord_rotate", value.texcoord_rotate))
    {
        propertiesRead = true;
        OSG_NOTIFY(_notifyLevel)<<"read texcoord_rotate \""<<value.texcoord_rotate<<"\""<<std::endl;
    }
*/
    return propertiesRead;
}

bool ReaderWriterP3DXML::parseProperties(osgDB::XmlNode* root, osg::UserDataContainer& udc) const
{
    bool readProperties = false;
    OSG_NOTICE<<"Doing parseProperties()"<<std::endl;
    for(osgDB::XmlNode::Children::iterator itr = root->children.begin();
        itr != root->children.end();
        ++itr)
    {
        osgDB::XmlNode* cur = itr->get();

        if (cur->name == "property")
        {
            std::string name;
            std::string type;

            getProperty(cur, "name", name);
            getProperty(cur, "type", type);

            if (type=="float")
            {
                float value;
                std::stringstream str(cur->contents);
                str>>value;
                
                udc.setUserValue(name, value);
                readProperties = true;

                OSG_NOTICE<<"Adding property float "<<value<<std::endl;
            }
            else if (type=="int")
            {
                int value;
                std::stringstream str(cur->contents);
                str>>value;

                udc.setUserValue(name, value);
                readProperties = true;

                OSG_NOTICE<<"Adding property int "<<value<<std::endl;
            }
            else
            {
                udc.setUserValue(name, cur->contents);
                readProperties = true;
                OSG_NOTICE<<"Adding property string "<<cur->contents<<std::endl;
            }
        }
        else
        {
            OSG_NOTICE<<"Unhandled tag["<<cur->name<<"] expecting <property>"<<std::endl;
        }
    }
    return readProperties;
}

bool ReaderWriterP3DXML::parsePropertyAnimation(osgDB::XmlNode* root, osgPresentation::PropertyAnimation& pa) const
{
    bool readKeyframes = false;
    OSG_NOTICE<<"Doing parsePropertyAnimation()"<<std::endl;
    for(osgDB::XmlNode::Children::iterator itr = root->children.begin();
        itr != root->children.end();
        ++itr)
    {
        osgDB::XmlNode* cur = itr->get();

        if (cur->name == "key_frame")
        {

            double time;
            if (getProperty(cur, "time", time))
            {
                osg::ref_ptr<osg::UserDataContainer> udc = new osg::DefaultUserDataContainer;
                if (parseProperties(cur, *udc))
                {
                    OSG_NOTICE<<"Adding keyframe"<<std::endl;
                    pa.addKeyFrame(time, udc.get());
                    readKeyframes = true;
                }
            }
            else
            {
                OSG_NOTICE<<"No time assigned to key_frame, ignoring <key_frame>"<<std::endl;
            }
        }
        else
        {
            OSG_NOTICE<<"Unhandled tag["<<cur->name<<"] expecting <key_frame>"<<std::endl;
        }
    }

    return readKeyframes;
}


bool ReaderWriterP3DXML::getJumpProperties(osgDB::XmlNode* cur, osgPresentation::JumpData& jumpData) const
{
    bool propertyRead = false;

    if (getProperty(cur, "slide_name", jumpData.slideName))
    {
        OSG_INFO<<"slide_name "<<jumpData.slideName<<std::endl;
        propertyRead = true;
    }

    if (getProperty(cur, "slide", jumpData.slideNum))
    {
        OSG_INFO<<"slide "<<jumpData.slideNum<<std::endl;
        propertyRead = true;
    }

    if (getProperty(cur, "layer", jumpData.layerNum))
    {
        OSG_INFO<<"layer "<<jumpData.layerNum<<std::endl;
        propertyRead = true;
    }

    if (getProperty(cur, "layer_name", jumpData.layerName))
    {
        OSG_INFO<<"layer_name "<<jumpData.layerName<<std::endl;
        propertyRead = true;
    }

    std::string jumpType;
    if (getProperty(cur, "jump", jumpType))
    {
        OSG_INFO<<"jump "<<jumpType<<std::endl;
        propertyRead = true;
        jumpData.relativeJump = (jumpType=="relative") || (jumpType=="Relative") || (jumpType=="RELATIVE") ;
    }

    return propertyRead;
}

void ReaderWriterP3DXML::parseModel(osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode*cur) const
{

    osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getModelPositionData();
   bool positionRead = getProperties(cur,positionData);

    osgPresentation::SlideShowConstructor::ModelData modelData;// = constructor.getModelData();
    getProperties(cur,modelData);

    std::string filename = cur->getTrimmedContents();

    if (!filename.empty())
    {
        constructor.addModel(filename,
                             positionRead ? positionData : constructor.getModelPositionData(),
                             modelData);
    }
}



osg::TransferFunction1D* ReaderWriterP3DXML::readTransferFunctionFile(const std::string& filename, float scale) const
{
    std::string foundFile = osgDB::findDataFile(filename);
    if (foundFile.empty())
    {
        OSG_NOTICE<<"Error: could not find transfer function file : "<<filename<<std::endl;
        return 0;
    }

    OSG_NOTICE<<"Reading transfer function "<<filename<<std::endl;

    osg::TransferFunction1D::ColorMap colorMap;
    osgDB::ifstream fin(foundFile.c_str());
    while(fin)
    {
        char readline[4096];
        *readline = 0;
        fin.getline(readline, sizeof(readline));

        if (*readline!=0)
        {
            std::stringstream str(readline);

            float value, red, green, blue, alpha;
            str >> value >> red >> green >> blue >> alpha;

            *readline = 0;
            str.getline(readline, sizeof(readline));

            char* comment = readline;
            while(*comment==' ' || *comment=='\t' ) ++comment;

            if (*comment!=0)
            {
                OSG_NOTICE<<"value = "<<value<<" ("<<red<<", "<<green<<", "<<blue<<", "<<alpha<<") comment = ["<<comment<<"]"<<std::endl;
            }
            else
            {
                OSG_NOTICE<<"value = "<<value<<" ("<<red<<", "<<green<<", "<<blue<<", "<<alpha<<")"<<std::endl;
            }
            colorMap[value] = osg::Vec4(red*scale,green*scale,blue*scale,alpha*scale);
        }
    }

    if (colorMap.empty())
    {
        OSG_NOTICE<<"Error: No values read from transfer function file: "<<filename<<std::endl;
        return 0;
    }

    osg::TransferFunction1D* tf = new osg::TransferFunction1D;
    tf->assign(colorMap);

    return tf;
}


void ReaderWriterP3DXML::parseVolume(osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode* cur) const
{

    osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getModelPositionData();
    bool positionRead = getProperties(cur,positionData);

    osgPresentation::SlideShowConstructor::VolumeData volumeData;

    // check the rendering technique/shading model to use
    std::string technique;
    if (getProperty(cur, "technique", technique))
    {
        if      (technique=="standard") volumeData.shadingModel =  osgPresentation::SlideShowConstructor::VolumeData::Standard;
        else if (technique=="mip") volumeData.shadingModel =  osgPresentation::SlideShowConstructor::VolumeData::MaximumIntensityProjection;
        else if (technique=="isosurface" || technique=="iso" ) volumeData.shadingModel =  osgPresentation::SlideShowConstructor::VolumeData::Isosurface;
        else if (technique=="light") volumeData.shadingModel =  osgPresentation::SlideShowConstructor::VolumeData::Light;
    }

    if (getProperty(cur, "alpha", volumeData.alphaValue)) {}
    if (getProperty(cur, "cutoff", volumeData.cutoffValue)) {}
    if (getProperty(cur, "region", volumeData.region)) {}
    if (getProperty(cur, "sampleDensity", volumeData.sampleDensityValue)) {}
    if (getProperty(cur, "sampleDensityWhenMoving", volumeData.sampleDensityWhenMovingValue)) {}


    if (getProperty(cur, "colourModulate", volumeData.colorModulate)) {}
    if (getProperty(cur, "colorModulate", volumeData.colorModulate)) {}

    std::string operation;
    if (getProperty(cur, "colorSpaceOperation", operation) || getProperty(cur, "colourSpaceOperation", operation))
    {
        if (operation=="NO_COLOR_SPACE_OPERATION") volumeData.colorSpaceOperation = osg::NO_COLOR_SPACE_OPERATION;
        else if (operation=="MODULATE_ALPHA_BY_LUMINANCE") volumeData.colorSpaceOperation = osg::MODULATE_ALPHA_BY_LUMINANCE;
        else if (operation=="MODULATE_ALPHA_BY_COLOR") volumeData.colorSpaceOperation = osg::MODULATE_ALPHA_BY_COLOR;
        else if (operation=="REPLACE_ALPHA_WITH_LUMINANCE") volumeData.colorSpaceOperation = osg::REPLACE_ALPHA_WITH_LUMINANCE;
        else if (operation=="REPLACE_RGB_WITH_LUMINANCE") volumeData.colorSpaceOperation = osg::REPLACE_RGB_WITH_LUMINANCE;
    }
    
    

    // check for any transfer function required
    std::string transferFunctionFile;
    if (getTrimmedProperty(cur, "tf", transferFunctionFile))
    {
        volumeData.transferFunction = readTransferFunctionFile(transferFunctionFile, 1.0);
    }

    if (getTrimmedProperty(cur, "tf-255", transferFunctionFile))
    {
        volumeData.transferFunction = readTransferFunctionFile(transferFunctionFile, 1.0/255.0);
    }

    if (getProperty(cur, "options", volumeData.options)) {}

    // check for draggers required
    std::string dragger;
    if (getProperty(cur, "dragger", dragger))
    {
        if (dragger=="trackball")
        {
            volumeData.useTabbedDragger = false;
            volumeData.useTrackballDragger = true;
        }
        if (dragger=="trackball-box")
        {
            volumeData.useTabbedDragger = true;
            volumeData.useTrackballDragger = true;
        }
        else
        {
            volumeData.useTabbedDragger = true;
            volumeData.useTrackballDragger = false;
        }
    }

    std::string filename = cur->getTrimmedContents();
    if (!filename.empty())
    {
        constructor.addVolume(filename,
                             positionRead ? positionData : constructor.getModelPositionData(),
                             volumeData);
    }
}

void ReaderWriterP3DXML::parseStereoPair(osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode*cur) const
{

    OSG_INFO<<"ReaderWriterP3DXML::parseStereoPair()"<<std::endl;


    std::string filenameLeft;
    std::string filenameRight;

    osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getImagePositionData();
    bool positionRead = getProperties(cur,positionData);

    osgPresentation::SlideShowConstructor::ImageData imageDataLeft;// = constructor.getImageData();
    osgPresentation::SlideShowConstructor::ImageData imageDataRight;// = constructor.getImageData();

    getProperties(cur,imageDataLeft);
    getProperties(cur,imageDataRight);

    for(osgDB::XmlNode::Children::iterator itr = cur->children.begin();
        itr != cur->children.end();
        ++itr)
    {
        osgDB::XmlNode* child = itr->get();

        if (child->name == "image_left")
        {
            getProperties(child,imageDataLeft);
            filenameLeft = child->getTrimmedContents();
        }
        else if (child->name == "imagesequence_left")
        {
            imageDataLeft.imageSequence = true;
            getProperties(child,imageDataLeft);
            filenameLeft = child->getTrimmedContents();
        }
        else if (child->name == "image_right")
        {
            getProperties(child,imageDataRight);
            filenameRight = child->getTrimmedContents();

            getProperties(cur,imageDataRight);
        }
        else if (child->name == "imagesequence_right")
        {
            imageDataRight.imageSequence = true;
            getProperties(child,imageDataRight);
            filenameRight = child->getTrimmedContents();
        }
    }

    OSG_INFO<<"    filenameLeft="<<filenameLeft<<std::endl;
    OSG_INFO<<"    filenameRight="<<filenameRight<<std::endl;


    if (!filenameLeft.empty() && !filenameRight.empty())
        constructor.addStereoImagePair(filenameLeft,imageDataLeft,
                                       filenameRight, imageDataRight,
                                       positionRead ? positionData : constructor.getImagePositionData());

}

bool ReaderWriterP3DXML::getKeyPosition(osgDB::XmlNode*cur, osgPresentation::KeyPosition& keyPosition) const
{
    if (cur->name == "key")
    {
        return getKeyPositionInner(cur, keyPosition);
    }
    if (cur->name == "escape" ||
        cur->name == "esc" ||
        cur->name == "exit")
    {
        keyPosition.set(osgGA::GUIEventAdapter::KEY_Escape, 0.0f, 0.0f);
        return true;
    }
    return false;
}

bool ReaderWriterP3DXML::getKeyPositionInner(osgDB::XmlNode*cur, osgPresentation::KeyPosition& keyPosition) const
{
    // x in range -1 to 1, from left to right
    float x = FLT_MAX;
    getProperty(cur, "x", x);

    // y in range -1 to 1, from bottom to top
    float y = FLT_MAX;
    getProperty(cur, "y", y);

    float h = FLT_MAX;
    if (getProperty(cur, "h", h))
    {
        // h in range 0.0 to 1, from left to right
        x = h*2.0f-1.0f;
    }

    float v = FLT_MAX;
    if (getProperty(cur, "v", v))
    {
        // v in range 0.0 to 1, from bottom to top
        y = v*2.0f-1.0f;
    }


    std::string key = cur->getTrimmedContents();
    unsigned int keyValue = 0;

    if (key.empty())
    {
        OSG_NOTICE<<"Warning: empty <key></key> is invalid, ignoring tag."<<std::endl;
        return false;
    }

    StringKeyMap::const_iterator itr=_stringKeyMap.find(key);
    if (itr != _stringKeyMap.end())
    {
        keyValue = itr->second;
    }
    else if (key.find("0x",0,2)!=std::string::npos)
    {
        std::istringstream iss(key);
        iss>>std::hex>>keyValue;
        OSG_INFO<<"ReaderWriterP3DXML::getKeyPositionInner() hex result = "<<keyValue<<std::endl;
    }
    else if (key.size()>1 && (key[0]>='0' && key[0]<='9'))
    {
        std::istringstream iss(key);
        iss>>keyValue;
        OSG_INFO<<"ReaderWriterP3DXML::getKeyPositionInner() numeric result = "<<keyValue<<std::endl;
    }
    else if (key.length()==1)
    {
        OSG_INFO<<"ReaderWriterP3DXML::getKeyPositionInner() alphanumeric result = "<<keyValue<<std::endl;
        keyValue = key[0];
    }
    else
    {
        OSG_NOTICE<<"Warning: invalid key used in <key>"<<key<<"</key>, ignoring tag. key=["<<key<<"]"<<std::endl;
        return false;
    }

    keyPosition.set(keyValue,x,y);
    return true;
}


void ReaderWriterP3DXML::parseTimeout(osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode* root) const
{
    // to allow the timeout to be nested with a Layer but still behave like a Layer itself we push the timeout as a Layer, saving the original Layer
    constructor.pushCurrentLayer();

    osg::ref_ptr<osgPresentation::Timeout> timeout = constructor.addTimeout();

    OSG_NOTICE<<"parseTimeout"<<std::endl;

    float totalIndent = 0.0f;

    for(osgDB::XmlNode::Children::iterator itr = root->children.begin();
        itr != root->children.end();
        ++itr)
    {
        osgDB::XmlNode* cur = itr->get();
        if (parseLayerChild(constructor, cur, totalIndent))
        {
            // no need to do anything
        }
        else if (cur->name == "timeout_jump")
        {
            osgPresentation::JumpData jumpData;
            if (getJumpProperties(cur, jumpData))
            {
                OSG_NOTICE<<"Timeout Jump "<<jumpData.relativeJump<<","<< jumpData.slideNum<<", "<<jumpData.layerNum<<std::endl;
                timeout->setActionJumpData(jumpData);
            }
        }
        else if (cur->name == "timeout_event")
        {
            osgPresentation::KeyPosition keyPosition;
            if (getKeyPositionInner( cur, keyPosition))
            {
                OSG_NOTICE<<"timeout event ["<<keyPosition._key<<"]"<<std::endl;
                timeout->setActionKeyPosition(keyPosition);
            }
        }
        else if (cur->name == "display_broadcast_event")
        {
            osgPresentation::KeyPosition keyPosition;
            if (getKeyPositionInner( cur, keyPosition))
            {
                OSG_NOTICE<<"display broadcast event ["<<keyPosition._key<<"]"<<std::endl;
                timeout->setDisplayBroadcastKeyPosition(keyPosition);
            }
        }
        else if (cur->name == "dismiss_broadcast_event")
        {
            osgPresentation::KeyPosition keyPosition;
            if (getKeyPositionInner( cur, keyPosition))
            {
                OSG_NOTICE<<"dismiss broadcast event ["<<keyPosition._key<<"]"<<std::endl;
                timeout->setDismissBroadcastKeyPosition(keyPosition);
            }
        }
        else if (cur->name == "timeout_broadcast_event")
        {
            osgPresentation::KeyPosition keyPosition;
            if (getKeyPositionInner( cur, keyPosition))
            {
                OSG_NOTICE<<"timeout broadcast event ["<<keyPosition._key<<"]"<<std::endl;
                timeout->setActionBroadcastKeyPosition(keyPosition);
            }
        }
        else if (cur->name == "idle_duration_before_timeout_display")
        {
            std::istringstream iss(cur->getTrimmedContents());
            double duration;
            iss>>duration;
            if (!iss.fail())
            {
                OSG_NOTICE<<"timeout->setIdleDurationBeforeTimeoutDisplay("<<duration<<")"<<std::endl;
                timeout->setIdleDurationBeforeTimeoutDisplay(duration);
            }
        }
        else if (cur->name == "idle_duration_before_timeout_action")
        {
            std::istringstream iss(cur->getTrimmedContents());
            double duration;
            iss>>duration;
            if (!iss.fail())
            {
                OSG_NOTICE<<"timeout->setIdleDurationBeforeTimeoutAction("<<duration<<")"<<std::endl;
                timeout->setIdleDurationBeforeTimeoutAction(duration);
            }
        }
        else if (cur->name == "key_starts_timeout_display")
        {
            osgPresentation::KeyPosition keyPosition;
            if (getKeyPositionInner( cur, keyPosition) && keyPosition._key!=0)
            {
                OSG_NOTICE<<"timeout->setKeyStartsTimoutDisplay("<<keyPosition._key<<")"<<std::endl;
                timeout->setKeyStartsTimoutDisplay(keyPosition._key);
            }
        }
        else if (cur->name == "key_dismiss_timeout_display")
        {
            osgPresentation::KeyPosition keyPosition;
            if (getKeyPositionInner( cur, keyPosition) && keyPosition._key!=0)
            {
                OSG_NOTICE<<"timeout->setKeyDismissTimoutDisplay("<<keyPosition._key<<")"<<std::endl;
                timeout->setKeyDismissTimoutDisplay(keyPosition._key);
            }
        }
        else if (cur->name == "key_run_action")
        {
            osgPresentation::KeyPosition keyPosition;
            if (getKeyPositionInner( cur, keyPosition) && keyPosition._key!=0)
            {
                OSG_NOTICE<<"timeout->setKeyRunTimoutAction("<<keyPosition._key<<")"<<std::endl;
                timeout->setKeyRunTimoutAction(keyPosition._key);
            }
        }
        
    }

    
    constructor.popCurrentLayer(); // return the
}

bool ReaderWriterP3DXML::parseLayerChild(osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode* cur, float& totalIndent) const
{
    if (cur->name == "newline")
    {
        constructor.translateTextCursor(osg::Vec3(0.0f,-0.05f,0.0f));
        return true;
    }
    else if (cur->name == "indent")
    {
        float localIndent = 0.05f;
        constructor.translateTextCursor(osg::Vec3(localIndent,0.0f,0.0f));
        totalIndent += localIndent;
        return true;
    }
    else if (cur->name == "unindent")
    {
        float localIndent = -0.05f;
        constructor.translateTextCursor(osg::Vec3(localIndent,0.0f,0.0f));
        totalIndent += localIndent;
        return true;
    }
    else if (cur->name == "bullet")
    {
        OSG_INFO<<"bullet ["<<cur->contents<<"]"<<std::endl;
        osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getTextPositionData();
        bool positionRead = getProperties(cur,positionData);

        osgPresentation::SlideShowConstructor::FontData fontData = constructor.getTextFontData();
        bool fontRead = getProperties(cur,fontData);

        constructor.addBullet(cur->contents,
                                positionRead ? positionData : constructor.getTextPositionData(),
                                fontRead ? fontData : constructor.getTextFontData());
        return true;
    }
    else if (cur->name == "paragraph")
    {
        osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getTextPositionData();
        bool positionRead = getProperties(cur,positionData);

        osgPresentation::SlideShowConstructor::FontData fontData = constructor.getTextFontData();
        bool fontRead = getProperties(cur,fontData);

        constructor.addParagraph(cur->contents,
                                    positionRead ? positionData : constructor.getTextPositionData(),
                                    fontRead ? fontData : constructor.getTextFontData());
        return true;
    }
    else if (cur->name == "image")
    {
        osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getImagePositionData();
        bool positionRead = getProperties(cur,positionData);

        osgPresentation::SlideShowConstructor::ImageData imageData;// = constructor.getImageData();
        getProperties(cur,imageData);

        constructor.addImage(cur->getTrimmedContents(),
                                positionRead ? positionData : constructor.getImagePositionData(),
                                imageData);
        return true;
    }
    else if (cur->name == "imagesequence")
    {
        osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getImagePositionData();
        bool positionRead = getProperties(cur,positionData);

        osgPresentation::SlideShowConstructor::ImageData imageData;// = constructor.getImageData();
        imageData.imageSequence = true;
        getProperties(cur,imageData);

        constructor.addImage(cur->getTrimmedContents(),
                                positionRead ? positionData : constructor.getImagePositionData(),
                                imageData);
        return true;
    }
    else if (cur->name == "graph")
    {
        osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getImagePositionData();
        bool positionRead = getProperties(cur,positionData);

        osgPresentation::SlideShowConstructor::ImageData imageData;// = constructor.getImageData();
        getProperties(cur,imageData);

        std::string options;
        getProperty(cur, "options", options);

        constructor.addGraph(cur->getTrimmedContents(),
                                positionRead ? positionData : constructor.getImagePositionData(),
                                imageData);
        return true;
    }
    else if (cur->name == "vnc")
    {
        osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getImagePositionData();
        bool positionRead = getProperties(cur,positionData);

        osgPresentation::SlideShowConstructor::ImageData imageData;// = constructor.getImageData();
        getProperties(cur,imageData);

        std::string password;
        getProperty(cur, "password", password);

        constructor.addVNC(cur->getTrimmedContents(),
                                positionRead ? positionData : constructor.getImagePositionData(),
                                imageData,
                                password
                            );
        return true;
    }
    else if (cur->name == "browser")
    {
        osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getImagePositionData();
        bool positionRead = getProperties(cur,positionData);

        osgPresentation::SlideShowConstructor::ImageData imageData;// = constructor.getImageData();
        getProperties(cur,imageData);

        constructor.addBrowser(cur->getTrimmedContents(),
                                positionRead ? positionData : constructor.getImagePositionData(),
                                imageData);
        return true;
    }
    else if (cur->name == "pdf")
    {
        osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getImagePositionData();
        bool positionRead = getProperties(cur,positionData);

        osgPresentation::SlideShowConstructor::ImageData imageData;// = constructor.getImageData();
        getProperties(cur,imageData);

        constructor.addPDF(cur->getTrimmedContents(),
                                positionRead ? positionData : constructor.getImagePositionData(),
                                imageData);
        return true;
    }
    else if (cur->name == "stereo_pair")
    {
        parseStereoPair(constructor, cur);
        return true;
    }
    else if (cur->name == "model")
    {
        parseModel(constructor, cur);
        return true;
    }
    else if (cur->name == "volume")
    {
        parseVolume(constructor, cur);
        return true;
    }
    else if (cur->name == "duration")
    {
        constructor.setLayerDuration(osg::asciiToDouble(cur->contents.c_str()));
        return true;
    }
    else if (cur->name == "property_animation")
    {
        osg::ref_ptr<osgPresentation::PropertyAnimation> pa = new osgPresentation::PropertyAnimation;
        if (parsePropertyAnimation(cur,*pa))
        {
            constructor.addPropertyAnimation(osgPresentation::SlideShowConstructor::CURRENT_LAYER, pa.get());
        }
        return true;
    }
    else if (cur->name == "properties")
    {
        if (!constructor.getCurrentLayer()) constructor.addLayer();
        if (constructor.getCurrentLayer())
        {
            osg::ref_ptr<osg::UserDataContainer> udc = constructor.getCurrentLayer()->getOrCreateUserDataContainer();
            if (parseProperties(cur, *udc))
            {
                OSG_NOTICE<<"Assigned properties to Layer"<<std::endl;
            }
        }
        return true;
    }

    return false;
}

void ReaderWriterP3DXML::parseLayer(osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode* root) const
{
    OSG_INFO<<std::endl<<"parseLayer"<<std::endl;

    float totalIndent = 0.0f;

    for(osgDB::XmlNode::Children::iterator itr = root->children.begin();
        itr != root->children.end();
        ++itr)
    {
        osgDB::XmlNode* cur = itr->get();
        if (parseLayerChild(constructor, cur, totalIndent))
        {
            // no need to do anything
        }
        else if (cur->name == "timeout")
        {
            parseTimeout(constructor, cur);
        }
        else if (cur->name == "run")
        {
            OSG_INFO<<"run ["<<cur->contents<<"]"<<std::endl;
            constructor.addLayerRunString(cur->contents);
        }
        else if (cur->name == "jump")
        {
            OSG_INFO<<"Parsed Jump "<<std::endl;

            osgPresentation::JumpData jumpData;
            if (getJumpProperties(cur, jumpData))
            {
                OSG_INFO<<"Layer Jump "<<jumpData.relativeJump<<","<< jumpData.slideNum<<", "<<jumpData.layerNum<<std::endl;

                constructor.setLayerJump(jumpData);
            }
        }
        else if (cur->name == "click_to_run")
        {
            osgPresentation::JumpData jumpData;
            getJumpProperties(cur, jumpData);

            OSG_INFO<<"click_to_run ["<<cur->contents<<"]"<<std::endl;
            constructor.layerClickToDoOperation(cur->contents,osgPresentation::RUN, jumpData);
        }
        else if (cur->name == "forward_mouse_event_to_device")
        {
            osgPresentation::JumpData jumpData;
            
            OSG_ALWAYS<<"forward_mouse_event_to_device ["<<cur->contents<<"]"<<std::endl;
            constructor.layerClickToDoOperation(cur->contents,osgPresentation::FORWARD_EVENT, jumpData);
        }
        else if (cur->name == "click_to_load")
        {
            osgPresentation::JumpData jumpData;
            getJumpProperties(cur, jumpData);

            OSG_INFO<<"click_to_load ["<<cur->contents<<"]"<<std::endl;
            constructor.layerClickToDoOperation(cur->contents,osgPresentation::LOAD, jumpData);
        }

        else if (cur->name == "click_to_event")
        {
            osgPresentation::JumpData jumpData;
            getJumpProperties(cur, jumpData);

            osgPresentation::KeyPosition keyPosition;
            if (getKeyPositionInner( cur, keyPosition))
            {
                OSG_INFO<<"click_to_event ["<<keyPosition._key<<"]"<<std::endl;
                constructor.layerClickEventOperation(keyPosition, jumpData);
            }
        }

        else if (cur->name == "click_to_jump")
        {
            osgPresentation::JumpData jumpData;
            getJumpProperties(cur, jumpData);

            constructor.layerClickEventOperation(osgPresentation::JUMP, jumpData);
        }

        else if (cur->name == "key_to_run")
        {
            int key;
            if (getKeyProperty(cur, "key", key))
            {
                osgPresentation::JumpData jumpData;
                getJumpProperties(cur, jumpData);

                OSG_NOTICE<<"key_to_run ["<<cur->contents<<"], key="<<key<<std::endl;
                constructor.keyToDoOperation(osgPresentation::SlideShowConstructor::CURRENT_LAYER, key, cur->contents,osgPresentation::RUN, jumpData);
            }
        }
        else if (cur->name == "key_to_load")
        {
            int key;
            if (getKeyProperty(cur, "key", key))
            {
                osgPresentation::JumpData jumpData;
                getJumpProperties(cur, jumpData);

                OSG_NOTICE<<"key_to_load ["<<cur->contents<<"]"<<std::endl;
                constructor.keyToDoOperation(osgPresentation::SlideShowConstructor::CURRENT_LAYER, key, cur->contents,osgPresentation::LOAD, jumpData);
            }
        }

        else if (cur->name == "key_to_event")
        {
            int key;
            if (getKeyProperty(cur, "key", key))
            {
                osgPresentation::JumpData jumpData;
                getJumpProperties(cur, jumpData);

                osgPresentation::KeyPosition keyPosition;
                if (getKeyPositionInner( cur, keyPosition))
                {
                    OSG_NOTICE<<"key_to_event ["<<keyPosition._key<<"]"<<std::endl;
                    constructor.keyEventOperation(osgPresentation::SlideShowConstructor::CURRENT_LAYER, key, keyPosition, jumpData);
                }
            }
        }

        else if (cur->name == "key_to_jump")
        {
            int key;
            if (getKeyProperty(cur, "key", key))
            {
                osgPresentation::JumpData jumpData;
                getJumpProperties(cur, jumpData);

                OSG_NOTICE<<"key_to_jump"<<std::endl;

                constructor.keyEventOperation(osgPresentation::SlideShowConstructor::CURRENT_LAYER, key, osgPresentation::JUMP, jumpData);
            }
            else
            {
                OSG_NOTICE<<"key_to_jump failed."<<std::endl;
            }
        }
        else 
        {
            osgPresentation::KeyPosition keyPosition;
            if (getKeyPosition(cur, keyPosition))
            {
                constructor.addLayerKey(keyPosition);
            }
        }
    }

    if (totalIndent != 0.0f)
    {
        constructor.translateTextCursor(osg::Vec3(-totalIndent,0.0f,0.0f));
    }

    std::string name;
    if (getProperty(root, "layer_name", name))
    {
        if (constructor.getCurrentLayer())
        {
            constructor.getCurrentLayer()->setUserValue("name",name);
            OSG_NOTICE<<"Setting current layers name "<<name<<std::endl;
        }
        else
        {
            OSG_NOTICE<<"getCurrentSlide() returns NULL, unable to set name "<<std::endl;
        }
    }
}

void ReaderWriterP3DXML::parseBullets(osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode*cur, bool inheritPreviousLayers, bool defineAsBaseLayer) const
{
    constructor.addLayer(inheritPreviousLayers, defineAsBaseLayer);

    OSG_INFO<<"bullets ["<<cur->contents<<"]"<<std::endl;
    osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getTextPositionData();
    bool positionRead = getProperties(cur,positionData);

    osgPresentation::SlideShowConstructor::FontData fontData = constructor.getTextFontData();
    bool fontRead = getProperties(cur,fontData);

    constructor.addBullet(cur->contents,
                            positionRead ? positionData : constructor.getTextPositionData(),
                            fontRead ? fontData : constructor.getTextFontData());
}


void ReaderWriterP3DXML::parseText(osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode*cur, bool inheritPreviousLayers, bool defineAsBaseLayer) const
{
    constructor.addLayer(inheritPreviousLayers, defineAsBaseLayer);

    OSG_INFO<<"text ["<<cur->contents<<"]"<<std::endl;
    osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getTextPositionData();
    bool positionRead = getProperties(cur,positionData);

    osgPresentation::SlideShowConstructor::FontData fontData = constructor.getTextFontData();
    bool fontRead = getProperties(cur,fontData);

    constructor.addParagraph(cur->contents,
                            positionRead ? positionData : constructor.getTextPositionData(),
                            fontRead ? fontData : constructor.getTextFontData());
}

void ReaderWriterP3DXML::parsePage(osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode*cur) const
{
    constructor.addSlide();

    std::string title;
    getProperty(cur, "title", title);

    std::string inherit;
    getProperty(cur, "inherit", inherit);

    if (!inherit.empty() && _templateMap.count(inherit)!=0)
    {
        parseSlide(constructor, _templateMap[inherit].get(), true, false);
    }

    if (!title.empty())
    {
        constructor.setSlideTitle(title,
                                    constructor.getTitlePositionData(),
                                    constructor.getTitleFontData());
    }

    if (!inherit.empty() && _templateMap.count(inherit)!=0)
    {
        parseSlide(constructor, _templateMap[inherit].get(), false, true);
    }

    constructor.addLayer(true,false);

    osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getTextPositionData();
    bool positionRead = getProperties(cur,positionData);

    osgPresentation::SlideShowConstructor::FontData fontData = constructor.getTextFontData();
    bool fontRead = getProperties(cur,fontData);

    constructor.addParagraph(cur->contents,
                            positionRead ? positionData : constructor.getTextPositionData(),
                            fontRead ? fontData : constructor.getTextFontData());
}

void ReaderWriterP3DXML::parsePdfDocument(osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode*cur) const
{
    std::string title;
    getProperty(cur, "title", title);

    std::string inherit;
    getProperty(cur, "inherit", inherit);

    constructor.addSlide();

    if (!inherit.empty() && _templateMap.count(inherit)!=0)
    {
        parseSlide(constructor, _templateMap[inherit].get(), true, false);
    }

    if (!title.empty())
    {
        constructor.setSlideTitle(title,
                                    constructor.getTitlePositionData(),
                                    constructor.getTitleFontData());
    }

    if (!inherit.empty() && _templateMap.count(inherit)!=0)
    {
        parseSlide(constructor, _templateMap[inherit].get(), false, true);
    }

    constructor.addLayer(true,false);

    osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getImagePositionData();
    getProperties(cur,positionData);

    osgPresentation::SlideShowConstructor::ImageData imageData;// = constructor.getImageData();
    imageData.page = 0;
    getProperties(cur,imageData);

    osg::Image* image = constructor.addInteractiveImage(cur->contents, positionData, imageData);
    osgWidget::PdfImage* pdfImage = dynamic_cast<osgWidget::PdfImage*>(image);
    if (pdfImage)
    {
        int numPages = pdfImage->getNumOfPages();
        OSG_INFO<<"NumOfPages = "<<numPages<<std::endl;

        if (numPages>1)
        {
            for(int pageNum=1; pageNum<numPages; ++pageNum)
            {
                imageData.page = pageNum;

                constructor.addSlide();

                if (!inherit.empty() && _templateMap.count(inherit)!=0)
                {
                    parseSlide(constructor, _templateMap[inherit].get(), true, false);
                }

                if (!title.empty())
                {
                    constructor.setSlideTitle(title,
                                                constructor.getTitlePositionData(),
                                                constructor.getTitleFontData());
                }

                if (!inherit.empty() && _templateMap.count(inherit)!=0)
                {
                    parseSlide(constructor, _templateMap[inherit].get(), false, true);
                }

                constructor.addLayer(true,false);

                constructor.addPDF(cur->getTrimmedContents(), positionData, imageData);

            }
        }
    }
}

void ReaderWriterP3DXML::parseSlide (osgPresentation::SlideShowConstructor& constructor, osgDB::XmlNode* root, bool parseTitles, bool parseLayers) const
{

    osg::Vec4 previous_bgcolor = constructor.getBackgroundColor();
    osg::Vec4 previous_textcolor = constructor.getTextColor();

    // create a keyPosition just in case we need it.
    osgPresentation::KeyPosition keyPosition;

    for(osgDB::XmlNode::Children::iterator itr = root->children.begin();
        itr != root->children.end();
        ++itr)
    {
        osgDB::XmlNode* cur = itr->get();

        if (parseTitles)
        {
            if (cur->name == "title")
            {
                osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getTitlePositionData();
                bool positionRead = getProperties(cur,positionData);

                osgPresentation::SlideShowConstructor::FontData fontData = constructor.getTitleFontData();
                bool fontRead = getProperties(cur,fontData);

                constructor.setSlideTitle(cur->contents,
                                          positionRead ? positionData : constructor.getTitlePositionData(),
                                          fontRead ? fontData : constructor.getTitleFontData());
            }
            else if (cur->name == "background")
            {
                constructor.setSlideBackground(cur->contents);

                std::string str;
                if (getProperty(cur, "hud", str))
                {
                    bool hud = (str != "off" && str != "Off" && str != "OFF");
                    OSG_NOTIFY(_notifyLevel)<<"background hud, str="<<str<<", hud="<<hud<<std::endl;
                    constructor.setSlideBackgrondHUD(hud);
                }
            }
            else if (cur->name == "bgcolor")
            {
                constructor.setBackgroundColor(mapStringToColor(cur->contents),true);
            }
            else if (cur->name == "textcolor")
            {
                constructor.setTextColor(mapStringToColor(cur->contents));
            }
        }
        if (parseLayers)
        {
            if (cur->name == "base")
            {
                constructor.addLayer(true, true);
                std::string inherit;
                if (getProperty(cur, "inherit", inherit) && !inherit.empty() && _templateMap.count(inherit)!=0)
                {
                    parseLayer(constructor, _templateMap[inherit].get());
                }
                parseLayer (constructor, cur);

            }
            else if (cur->name == "layer")
            {
                constructor.addLayer(true, false);
                std::string inherit;
                if (getProperty(cur, "inherit", inherit) && !inherit.empty() && _templateMap.count(inherit)!=0)
                {
                    parseLayer(constructor, _templateMap[inherit].get());
                }
                
                parseLayer (constructor, cur);
            }
            else if (cur->name == "clean_layer")
            {
                constructor.addLayer(false, false);
                std::string inherit;
                if (getProperty(cur, "inherit", inherit) && !inherit.empty() && _templateMap.count(inherit)!=0)
                {
                    parseLayer(constructor, _templateMap[inherit].get());
                }
                parseLayer (constructor, cur);
            }
            else if (cur->name == "modify_layer")
            {
                int layerNum;
                if (getProperty(cur, "layer", layerNum))
                {
                    constructor.selectLayer(layerNum);
                }
                else
                {
                    constructor.addLayer(true, false);
                }

                parseLayer (constructor, cur);
            }
            else if (cur->name == "bullets")
            {
                parseBullets (constructor, cur,true, false);
            }
            else if (cur->name == "duration")
            {
                constructor.setSlideDuration(osg::asciiToDouble(cur->contents.c_str()));
            }
            else if (cur->name == "property_animation")
            {
                osg::ref_ptr<osgPresentation::PropertyAnimation> pa = new osgPresentation::PropertyAnimation;
                if (parsePropertyAnimation(cur,*pa))
                {
                    constructor.addPropertyAnimation(osgPresentation::SlideShowConstructor::CURRENT_SLIDE, pa.get());
                }
            }
            else if (cur->name == "properties")
            {
                if (!constructor.getCurrentSlide()) constructor.addSlide();
                if (constructor.getCurrentSlide())
                {
                    osg::ref_ptr<osg::UserDataContainer> udc = constructor.getCurrentSlide()->getOrCreateUserDataContainer();
                    if (parseProperties(cur, *udc))
                    {
                        OSG_NOTICE<<"Assigned properties to Slide"<<std::endl;
                    }
                }
            }
            else if (getKeyPosition(cur, keyPosition))
            {
                constructor.addSlideKey(keyPosition);
            }
        }
    }

    std::string name;
    if (getProperty(root, "slide_name", name))
    {
        if (constructor.getCurrentSlide())
        {
            constructor.getCurrentSlide()->setUserValue("name",name);
            OSG_NOTICE<<"Setting current slide name "<<name<<std::endl;
        }
        else
        {
            OSG_NOTICE<<"getCurrentSlide() returns NULL, unable to set name "<<std::endl;
        }
    }
    
    constructor.setBackgroundColor(previous_bgcolor,false);
    constructor.setTextColor(previous_textcolor);

    return;
}

#include <iostream>


struct MyFindFileCallback : public osgDB::FindFileCallback
{
    virtual std::string findDataFile(const std::string& filename, const osgDB::Options* options, osgDB::CaseSensitivity caseSensitivity)
    {
        OSG_INFO<<std::endl<<std::endl<<"find file "<<filename<<std::endl;

        const osgDB::FilePathList& paths = options ? options->getDatabasePathList() : osgDB::getDataFilePathList();

        for(osgDB::FilePathList::const_iterator itr = paths.begin();
            itr != paths.end();
            ++itr)
        {
            const std::string& path = *itr;
            std::string newpath = osgDB::concatPaths(path, filename);
            if (osgDB::containsServerAddress(path))
            {
                osgDB::ReaderWriter* rw = osgDB::Registry::instance()->getReaderWriterForExtension("curl");
                OSG_INFO<<"  file on server "<<*itr<<", try path "<<newpath<<std::endl;
                OSG_INFO<<"  we have curl rw= "<<rw<<std::endl;
                if (rw && rw->fileExists(newpath, options))
                {
                    OSG_INFO<<"  FOUND on server "<<newpath<<std::endl;
                    return newpath;
                }
            }
            else
            {
                if(osgDB::fileExists(newpath))
                {
                    OSG_INFO<<" FOUND "<<newpath<<std::endl;
                    return newpath;
                }
            }
        }

        return osgDB::Registry::instance()->findDataFileImplementation(filename, options, caseSensitivity);
    }
};

class MyReadFileCallback : public virtual osgDB::ReadFileCallback
{
    public:

        osgDB::FilePathList _paths;

        typedef std::map< std::string, osg::ref_ptr<osg::Object> >  ObjectCache;

        enum ObjectType
        {
            OBJECT,
            IMAGE,
            HEIGHT_FIELD,
            NODE,
            SHADER
        };

        osgDB::ReaderWriter::ReadResult readLocal(ObjectType type, const std::string& filename, const osgDB::Options* options)
        {
            OSG_INFO<<"Trying local file "<<filename<<std::endl;

            switch(type)
            {
                case(OBJECT): return osgDB::Registry::instance()->readObjectImplementation(filename,options);
                case(IMAGE): return osgDB::Registry::instance()->readImageImplementation(filename,options);
                case(HEIGHT_FIELD): return osgDB::Registry::instance()->readHeightFieldImplementation(filename,options);
                case(NODE): return osgDB::Registry::instance()->readNodeImplementation(filename,options);
                case(SHADER): return osgDB::Registry::instance()->readShaderImplementation(filename,options);
            }
            return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;
        }


        osgDB::ReaderWriter::ReadResult readFileCache(ObjectType type, const std::string& filename, const osgDB::Options* options)
        {

            osgDB::FileCache* fileCache = options ? options->getFileCache() : 0;
            if (!fileCache) fileCache = osgDB::Registry::instance()->getFileCache();
            if (!fileCache) return osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND;

            OSG_INFO<<"Trying fileCache "<<filename<<std::endl;

            osgDB::ReaderWriter::ReadResult result;
            if (fileCache && fileCache->isFileAppropriateForFileCache(filename))
            {
                if (fileCache->existsInCache(filename))
                {
                    switch(type)
                    {
                        case(OBJECT):
                            result = fileCache->readObject(filename, 0);
                            break;
                        case(IMAGE):
                            result = fileCache->readImage(filename, 0);
                            break;
                        case(HEIGHT_FIELD):
                            result = fileCache->readHeightField(filename, 0);
                            break;
                        case(NODE):
                            result = fileCache->readNode(filename,0);
                            break;
                        case(SHADER):
                            result = fileCache->readShader(filename, 0);
                            break;
                    }

                    if (result.success())
                    {
                        OSG_INFO<<"   File read from FileCache."<<std::endl;
                        return result;
                    }

                    OSG_INFO<<"   File in FileCache, but not successfully read"<<std::endl;
                }
                else
                {
                    OSG_INFO<<"   File does not exist in FileCache: "<<fileCache->createCacheFileName(filename)<<std::endl;
                }
            }

            return osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND;

        }

        osgDB::ReaderWriter::ReadResult readServer(ObjectType type, const std::string& filename, const osgDB::Options* options)
        {
            OSG_INFO<<"Trying server file "<<filename<<std::endl;

            osgDB::ReaderWriter::ReadResult result;
            
            // get a specific readerwriter capable of handling the protocol and extension, will return a registered fallback readerwriter for extension '*'
            osgDB::ReaderWriter* rw = osgDB::Registry::instance()->getReaderWriterForProtocolAndExtension(
                osgDB::getServerProtocol(filename),
                osgDB::getFileExtension(filename));
                        
            if (!rw) return osgDB::ReaderWriter::ReadResult::FILE_NOT_HANDLED;

            switch(type)
            {
                case(OBJECT):
                    result = rw->readObject(filename,options);
                    break;
                case(IMAGE):
                    result = rw->readImage(filename,options);
                    break;
                case(HEIGHT_FIELD):
                    result = rw->readHeightField(filename,options);
                    break;
                case(NODE):
                    result = rw->readNode(filename,options);
                    break;
                case(SHADER):
                    result = rw->readShader(filename,options);
                    break;
            }

            if (result.success())
            {
                osgDB::FileCache* fileCache = options ? options->getFileCache() : 0;
                if (!fileCache) fileCache = osgDB::Registry::instance()->getFileCache();

                if (fileCache && fileCache->isFileAppropriateForFileCache(filename))
                {
                    switch(type)
                    {
                        case(OBJECT):
                            fileCache->writeObject(*result.getObject(),filename,options);
                            break;
                        case(IMAGE):
                            result.getImage()->setFileName(filename);
                            fileCache->writeImage(*result.getImage(),filename,options);
                            break;
                        case(HEIGHT_FIELD):
                            fileCache->writeHeightField(*result.getHeightField(),filename,options);
                            break;
                        case(NODE):
                            fileCache->writeNode(*result.getNode(),filename,options);
                            break;
                        case(SHADER):
                            fileCache->writeShader(*result.getShader(),filename,options);
                            break;
                    }
                }

                return result;
            }
            return osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND;
        }


        osgDB::ReaderWriter::ReadResult read(const osgDB::FilePathList& filePathList, ObjectType type, const std::string& filename, const osgDB::Options* options, bool checkLocalFiles)
        {
            // go look in http paths
            for(osgDB::FilePathList::const_iterator itr = filePathList.begin();
                itr != filePathList.end();
                ++itr)
            {
                const std::string& path = *itr;
                std::string newpath = osgDB::containsServerAddress(filename) ? filename : path.empty() ? filename : osgDB::concatPaths(path, filename);
                osgDB::ReaderWriter::ReadResult result;
                if (osgDB::containsServerAddress(newpath))
                {
                    if (checkLocalFiles) result = readFileCache(type, newpath, options);
                    else result = readServer(type, newpath, options);
                }
                else if (checkLocalFiles && osgDB::fileExists(newpath))
                {
                    result = readLocal(type, newpath, options);
                }

                if (result.success())
                {
                    OSG_INFO<<"   inserting object into file cache "<<filename<<", "<<result.getObject()<<std::endl;
                    _objectCache[filename] = result.getObject();

                    options->setPluginStringData("filename",newpath);
                    return result;
                }
            }
            return osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND;
        }

        osgDB::ReaderWriter::ReadResult read(ObjectType type, const std::string& filename, const osgDB::Options* options)
        {
            osgDB::FileCache* fileCache = options ? options->getFileCache() : 0;
            if (!fileCache) fileCache = osgDB::Registry::instance()->getFileCache();
            if (fileCache && !fileCache->isFileAppropriateForFileCache(filename)) fileCache = 0;

            OSG_INFO<<"MyReadFileCallback::reading file "<<filename<<std::endl;
            ObjectCache::iterator itr = _objectCache.find(filename);
            if (itr != _objectCache.end())
            {
                // object is in cache, just retrieve it.
                if (itr->second.valid())
                {
                    OSG_INFO<<"File retrieved from cache, filename="<<filename<<std::endl;
                    return itr->second.get();
                }
                else
                {
                    OSG_INFO<<"File failed to load previously, won't attempt a second time "<<filename<<std::endl;
                    return osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND;
                }
            }

            OSG_INFO<<"   MyReadFileCallback::reading file A"<<filename<<std::endl;

            {
                bool checkLocalFiles = true;
                osgDB::ReaderWriter::ReadResult result = read(_paths, type, filename, options, checkLocalFiles);
                if (result.success()) return result;

                if (options && !(options->getDatabasePathList().empty()))
                {
                    result = read(options->getDatabasePathList(), type, filename, options, checkLocalFiles);
                    if (result.success()) return result;
                }

                result = read(osgDB::Registry::instance()->getDataFilePathList(), type, filename, options, checkLocalFiles);
                if (result.success()) return result;
            }

            OSG_INFO<<"   MyReadFileCallback::reading file B"<<filename<<std::endl;

            {
                bool checkLocalFiles = false;
                osgDB::ReaderWriter::ReadResult result = read(_paths, type, filename, options, checkLocalFiles);
                if (result.success()) return result;

                if (options && !(options->getDatabasePathList().empty()))
                {
                    result = read(options->getDatabasePathList(), type, filename, options, checkLocalFiles);
                    if (result.success()) return result;
                }

                result = read(osgDB::Registry::instance()->getDataFilePathList(), type, filename, options, checkLocalFiles);
                if (result.success()) return result;
            }

            OSG_INFO<<"   MyReadFileCallback::reading file C"<<filename<<std::endl;

            _objectCache[filename] = 0;

            return osgDB::ReaderWriter::ReadResult::FILE_NOT_FOUND;
        }

        virtual osgDB::ReaderWriter::ReadResult readObject(const std::string& filename, const osgDB::Options* options)
        {
            return read(OBJECT, filename, options);
        }

        virtual osgDB::ReaderWriter::ReadResult readImage(const std::string& filename, const osgDB::Options* options)
        {
            return read(IMAGE, filename, options);
        }

        virtual osgDB::ReaderWriter::ReadResult readHeightField(const std::string& filename, const osgDB::Options* options)
        {
            return read(HEIGHT_FIELD, filename, options);
        }

        virtual osgDB::ReaderWriter::ReadResult readNode(const std::string& filename, const osgDB::Options* options)
        {
            return read(NODE, filename, options);
        }

        virtual osgDB::ReaderWriter::ReadResult readShader(const std::string& filename, const osgDB::Options* options)
        {
            return read(SHADER, filename, options);
        }

    protected:
        virtual ~MyReadFileCallback() {}

        ObjectCache _objectCache;
};

osgDB::ReaderWriter::ReadResult ReaderWriterP3DXML::readNode(const std::string& file,
                                                           const osgDB::ReaderWriter::Options* options) const
{
    OSG_INFO<<"readNode("<<file<<")"<<std::endl;


    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    std::string fileName = file;

    std::string fileNameSansExtension = osgDB::getNameLessExtension(fileName);
    std::string nestedExtension = osgDB::getFileExtension(fileNameSansExtension);
    std::string fileNameSansNestedExtension = osgDB::getNameLessExtension(fileNameSansExtension);

    if (nestedExtension=="preview" || nestedExtension=="main")
    {
        fileName = fileNameSansNestedExtension + "." + ext;
        OSG_INFO<<"Removed nested extension "<<nestedExtension<<" result = "<<fileName<<std::endl;
    }

    fileName = osgDB::findDataFile( fileName, options );
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    // code for setting up the database path so that internally referenced file are searched for on relative paths.
    osg::ref_ptr<osgDB::ReaderWriter::Options> local_opt = options ? static_cast<osgDB::ReaderWriter::Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
    local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));
    local_opt->setFindFileCallback(new MyFindFileCallback);
    local_opt->setPluginStringData("filename",file);

    osgDB::XmlNode::Input input;
    input.open(fileName);
    input.readAllDataIntoBuffer();

    return readNode(input, local_opt.get());
}

osgDB::ReaderWriter::ReadResult ReaderWriterP3DXML::readNode(std::istream& fin, const Options* options) const
{
    osgDB::XmlNode::Input input;
    input.attach(fin);
    input.readAllDataIntoBuffer();

    osg::ref_ptr<osgDB::ReaderWriter::Options> local_opt = options ? static_cast<osgDB::ReaderWriter::Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
    local_opt->setReadFileCallback(new MyReadFileCallback);
    local_opt->setFindFileCallback(new MyFindFileCallback);
        
    return readNode(input, local_opt.get());
}

osgDB::ReaderWriter::ReadResult ReaderWriterP3DXML::readNode(osgDB::XmlNode::Input& input, osgDB::ReaderWriter::Options* options) const
{
    std::string fileName = options ? options->getPluginStringData("filename") : std::string();
    std::string extension = osgDB::getFileExtension(fileName);
    std::string fileNameSansExtension = osgDB::getNameLessExtension(fileName);
    std::string nestedExtension = osgDB::getFileExtension(fileNameSansExtension);
    std::string fileNameSansNestedExtension = osgDB::getFileExtension(fileNameSansExtension);

    bool readOnlyHoldingPage = nestedExtension=="preview" || (options && options->getOptionString()=="preview");
    bool readOnlyMainPresentation = nestedExtension=="main" || (options && options->getOptionString()=="main");

    osg::ref_ptr<osgDB::XmlNode> doc = new osgDB::XmlNode;
    osgDB::XmlNode* root = 0;

    doc->read(input);

    OSG_INFO<<"P3D xml file read, now building presentation scene graph."<<std::endl;

#if 0
    std::ofstream fout("output.p3d");
    doc->write(fout);
#endif

    if (doc == NULL )
    {
            fprintf(stderr,"Document not parsed successfully. \n");
            return ReadResult::FILE_NOT_HANDLED;
    }

    for(osgDB::XmlNode::Children::iterator itr = doc->children.begin();
        itr != doc->children.end() && !root;
        ++itr)
    {
        if ((*itr)->name=="presentation") root = itr->get();
    }

    if (root == NULL)
    {
            fprintf(stderr,"empty document\n");
            return ReadResult::FILE_NOT_HANDLED;
    }

    if (root->name!="presentation")
    {
            fprintf(stderr,"document of the wrong type, root node != presentation");
            return ReadResult::FILE_NOT_HANDLED;
    }


    bool hasHoldingSlide = false;
    for(osgDB::XmlNode::Children::iterator itr = root->children.begin();
        itr != root->children.end() && !hasHoldingSlide;
        ++itr)
    {
        osgDB::XmlNode* cur = itr->get();
        if (cur->name == "holding_slide")
        {
            hasHoldingSlide = true;
        }
    }

    // if we are looking for a holding slide, but one isn't present return NULL.
    if (readOnlyHoldingPage && !hasHoldingSlide) return 0;

    OSG_INFO<<"hasHoldingSlide "<<hasHoldingSlide<<std::endl;
    OSG_INFO<<"readOnlyHoldingPage "<<readOnlyHoldingPage<<std::endl;
    OSG_INFO<<"readOnlyMainPresentation "<<readOnlyMainPresentation<<std::endl;

    osg::ref_ptr<osg::Node> presentation_node;


    if (!readOnlyHoldingPage && !readOnlyMainPresentation)
    {
        if (fileName.empty()) hasHoldingSlide = false;

        osg::ref_ptr<osg::Node> holdingslide_node = hasHoldingSlide ? parseXmlGraph(root, true, options) : 0;

        if (holdingslide_node.valid())
        {
            double farDistance = 10000.0f;

            osg::ref_ptr<osg::PagedLOD> pagedLOD = new osg::PagedLOD;

            pagedLOD->setDatabaseOptions(options);

            pagedLOD->addChild(holdingslide_node.get());
            pagedLOD->setRange(0,farDistance,2.0*farDistance);

            std::string fileNameToLoadMainPresentation = fileNameSansExtension + std::string(".main.")+extension;

            pagedLOD->setFileName(1,fileNameToLoadMainPresentation);
            pagedLOD->setRange(1,0.0,farDistance);

            presentation_node = pagedLOD.get();

        }
        else
        {
            presentation_node = parseXmlGraph(root, false, options);
        }
    }
    else
    {
        presentation_node = parseXmlGraph(root, readOnlyHoldingPage, options);
    }

    if (presentation_node.valid())
    {
        if (!options || options->getPluginStringData("P3D_EVENTHANDLER")!="none")
        {
            // if (!readOnlyHoldingPage && !readOnlyMainPresentation)
            {
                osgPresentation::SlideEventHandler* seh = new osgPresentation::SlideEventHandler;
                seh->set(presentation_node.get());
                presentation_node->addEventCallback(seh);
            }
        }
        return presentation_node.release();
    }
    else
    {
        return osgDB::ReaderWriter::ReadResult::ERROR_IN_READING_FILE;
    }
}

osg::Node* ReaderWriterP3DXML::parseXmlGraph(osgDB::XmlNode* root, bool readOnlyHoldingPage, osgDB::Options* options) const
{
    osgPresentation::SlideShowConstructor constructor(options);

    // create a keyPosition just in case we need it.
    osgPresentation::KeyPosition keyPosition;

    osgDB::FilePathList previousPaths = osgDB::getDataFilePathList();

    for(osgDB::XmlNode::Children::iterator itr = root->children.begin();
        itr != root->children.end();
        ++itr)
    {
        osgDB::XmlNode* cur = itr->get();

        if (cur->name=="env")
        {
            char* str = strdup(cur->contents.c_str());
            OSG_INFO<<"putenv("<<str<<")"<<std::endl;
            putenv(str);
        }
    }

    std::string pathToPresentation;
    MyReadFileCallback* readFileCallback = options ? dynamic_cast<MyReadFileCallback*>(options->getReadFileCallback()) : 0;

    if (options && !(options->getDatabasePathList().empty()))
    {
        pathToPresentation = options->getDatabasePathList().front();

       if (readFileCallback) readFileCallback->_paths.push_front(pathToPresentation);
    }


    for(osgDB::XmlNode::Children::iterator itr = root->children.begin();
        itr != root->children.end();
        ++itr)
    {
        osgDB::XmlNode* cur = itr->get();

        if (cur->name == "path")
        {
            std::string newpath = expandEnvVarsInFileName(cur->contents);

            // now check if an absolue or http path
            std::string::size_type colonPos = newpath.find_first_of(':');
            std::string::size_type backslashPos = newpath.find_first_of('/');
            std::string::size_type forwardslashPos = newpath.find_first_of('\\');
            bool relativePath = colonPos == std::string::npos &&
                                backslashPos != 0 &&
                                forwardslashPos != 0;

            if (relativePath && !pathToPresentation.empty())
            {
                newpath = osgDB::concatPaths(pathToPresentation, newpath);
                OSG_INFO<<"relative path = "<<cur->contents<<", newpath="<<newpath<<std::endl;
            }
            else
            {
                OSG_INFO<<"absolute path = "<<cur->contents<<", newpath="<<newpath<<std::endl;
            }

            if (readFileCallback) readFileCallback->_paths.push_back(newpath);
            else options->getDatabasePathList().push_back(newpath);
        }
    }


    for(osgDB::XmlNode::Children::iterator itr = root->children.begin();
        itr != root->children.end();
        ++itr)
    {
        osgDB::XmlNode* cur = itr->get();

        if (cur->name == "name")
        {
            constructor.setPresentationName(cur->contents);
        }
        else if (cur->name == "loop")
        {
            constructor.setLoopPresentation(true);
        }
        else if (cur->name == "auto")
        {
            constructor.setAutoSteppingActive(true);
        }
        else if (cur->name == "title-settings")
        {
            bool fontRead = getProperties(cur,constructor.getTitleFontDataDefault());
            if (fontRead)
            {
                OSG_INFO<<"Title font details read"<<std::endl;
            }
        }
        else if (cur->name == "text-settings")
        {
            bool fontRead = getProperties(cur,constructor.getTextFontDataDefault());
            if (fontRead)
            {
                OSG_INFO<<"Text font details read"<<std::endl;
            }
        }
        /*else if (cur->name == "ratio")
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) constructor.setPresentationAspectRatio(cur->contents);
            xmlFree(key);
        }*/
        else if (cur->name == "path")
        {
            OSG_INFO<<"Appending search path "<<cur->contents<<std::endl;
            osgDB::getDataFilePathList().push_front(expandEnvVarsInFileName(cur->contents));
        }
        else if (cur->name == "bgcolor")
        {
            constructor.setBackgroundColor(mapStringToColor(cur->contents),false);
        }
        else if (cur->name == "textcolor")
        {
            constructor.setTextColor(mapStringToColor(cur->contents));
        }
        else if (cur->name == "duration")
        {
            constructor.setPresentationDuration(osg::asciiToDouble(cur->contents.c_str()));
        }
        else if (getKeyPosition(cur, keyPosition))
        {
            constructor.addPresentationKey(keyPosition);
        }
        else if (readOnlyHoldingPage && cur->name == "holding_slide")
        {
            constructor.addSlide();
            parseSlide (constructor, cur);
        }
        else if (!readOnlyHoldingPage && cur->name == "slide")
        {
            constructor.addSlide();

            std::string inherit;
            if (getProperty(cur, "inherit", inherit) && !inherit.empty() && _templateMap.count(inherit)!=0)
            {
                parseSlide(constructor, _templateMap[inherit].get(), true, false);
                parseSlide (constructor, cur, true, false);
                parseSlide(constructor, _templateMap[inherit].get(), false, true);
                parseSlide (constructor, cur, false, true);
            }
            else
            {
                parseSlide (constructor, cur);
            }
        }
        else if (!readOnlyHoldingPage && cur->name == "modify_slide")
        {
            int slideNum;
            if (getProperty(cur, "slide", slideNum))
            {
                constructor.selectSlide(slideNum);
                parseSlide (constructor, cur);
            }
            else
            {
                constructor.addSlide();
            }
        }
        else if (!readOnlyHoldingPage && cur->name == "page")
        {
            parsePage (constructor, cur);
        }
        else if (!readOnlyHoldingPage && cur->name == "pdf_document")
        {
            parsePdfDocument(constructor, cur);
        }
        else if (!readOnlyHoldingPage && cur->name == "template_slide")
        {
            std::string name;
            if (getProperty(cur, "name", name))
            {
                _templateMap[name] = cur;
                std::cout<<"Defining template slide "<<name<<std::endl;
            }
        }
        else if (!readOnlyHoldingPage && cur->name == "template_layer")
        {
            std::string name;
            if (getProperty(cur, "name", name))
            {
                _templateMap[name] = cur;
                std::cout<<"Defining template layer "<<name<<std::endl;
            }
        }
        else if (!readOnlyHoldingPage && cur->name == "property_animation")
        {
            osg::ref_ptr<osgPresentation::PropertyAnimation> pa = new osgPresentation::PropertyAnimation;
            if (parsePropertyAnimation(cur,*pa))
            {
                constructor.addPropertyAnimation(osgPresentation::SlideShowConstructor::CURRENT_PRESENTATION, pa.get());
            }
        }
        else if (!readOnlyHoldingPage && cur->name == "properties")
        {
                if (!constructor.getPresentationSwitch()) constructor.createPresentation();
                if (constructor.getPresentationSwitch())
                {
                    osg::ref_ptr<osg::UserDataContainer> udc = constructor.getPresentationSwitch()->getOrCreateUserDataContainer();
                    if (parseProperties(cur, *udc))
                    {
                        OSG_NOTICE<<"Assigned properties to Presentation"<<std::endl;
                    }
                }
            }
    }

    osgDB::getDataFilePathList() = previousPaths;

    return constructor.takePresentation();
}
