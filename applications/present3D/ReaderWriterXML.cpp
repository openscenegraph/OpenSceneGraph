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

#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>

#include <osgWidget/PdfReader>

#include "SlideShowConstructor.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

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

    void parseModel(osgPresentation::SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur) const;

    void parseVolume(osgPresentation::SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur) const;

    void parseStereoPair(osgPresentation::SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur) const;

    void parseLayer(osgPresentation::SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur) const;
 
    void parseBullets(osgPresentation::SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur, bool inheritPreviousLayers, bool defineAsBaseLayer) const;
    void parseText(osgPresentation::SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur, bool inheritPreviousLayers, bool defineAsBaseLayer) const;

    void parsePage (osgPresentation::SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur) const;
    
    void parseSlide (osgPresentation::SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur, bool parseTitles=true, bool parseLayers=true) const;

    void parsePdfDocument (osgPresentation::SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur) const;

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
    inline bool read(const char* str, osg::Vec2& value) const;
    inline bool read(const char* str, osg::Vec3& value) const;
    inline bool read(const char* str, osg::Vec4& value) const;
    
    inline bool read(const std::string& str, int& value) const;
    inline bool read(const std::string& str, float& value) const;
    inline bool read(const std::string& str, double& value) const;
    inline bool read(const std::string& str, osg::Vec2& value) const;
    inline bool read(const std::string& str, osg::Vec3& value) const;
    inline bool read(const std::string& str, osg::Vec4& value) const;

    bool getProperty(xmlNodePtr cur, const char* token) const;
    bool getProperty(xmlNodePtr cur, const char* token, int& value) const;
    bool getProperty(xmlNodePtr cur, const char* token, float& value) const;
    bool getProperty(xmlNodePtr cur, const char* token, double& value) const;
    bool getProperty(xmlNodePtr cur, const char* token, osg::Vec2& value) const;
    bool getProperty(xmlNodePtr cur, const char* token, osg::Vec3& value) const;
    bool getProperty(xmlNodePtr cur, const char* token, osg::Vec4& value) const;
    bool getProperty(xmlNodePtr cur, const char* token, std::string& value) const;
    bool getProperty(xmlNodePtr cur, const char* token, osgText::Text::Layout& value) const;
    bool getProperty(xmlNodePtr cur, const char* token, osgText::Text::AlignmentType& value) const;
    
    bool getProperties(xmlNodePtr cur, osgPresentation::SlideShowConstructor::PositionData& value) const;
    bool getProperties(xmlNodePtr cur, osgPresentation::SlideShowConstructor::FontData& value) const;
    bool getProperties(xmlNodePtr cur, osgPresentation::SlideShowConstructor::ModelData& value) const;
    bool getProperties(xmlNodePtr cur, osgPresentation::SlideShowConstructor::ImageData& value) const;
    bool getJumpProperties(xmlNodePtr cur, bool& relativeJump, int& slideNum, int& layerNum) const;
 
    bool getKeyPositionInner(xmlDocPtr doc, xmlNodePtr cur, osgPresentation::KeyPosition& keyPosition) const;
    bool getKeyPosition(xmlDocPtr doc, xmlNodePtr cur, osgPresentation::KeyPosition& keyPosition) const;

    typedef std::map<std::string,osg::Vec4> ColorMap;
    typedef std::map<std::string,osgText::Text::Layout> LayoutMap;
    typedef std::map<std::string,osgText::Text::AlignmentType> AlignmentMap;
    typedef std::map<std::string, unsigned int> StringKeyMap;

    std::string expandEnvVarsInFileName(const std::string& filename) const; 


    ColorMap            _colorMap;
    LayoutMap           _layoutMap;
    AlignmentMap        _alignmentMap;
    StringKeyMap        _stringKeyMap;
    
    typedef std::pair<xmlDocPtr,xmlNodePtr> DocNodePair;
    typedef std::map<std::string, DocNodePair> TemplateMap;
    
    mutable TemplateMap _templateMap;

    osg::NotifySeverity _notifyLevel;

};

// Register with Registry to instantiate the above reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterP3DXML> g_readerWriter_P3DXML_Proxy;

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

bool ReaderWriterP3DXML::getProperty(xmlNodePtr cur, const char* token) const
{
    bool success = false;
    xmlChar *key;
    key = xmlGetProp (cur, (const xmlChar *)token);
    if (key) success=true;
    xmlFree(key);
    return success;
}

bool ReaderWriterP3DXML::getProperty(xmlNodePtr cur, const char* token, int& value) const
{
    xmlChar *key;
    key = xmlGetProp (cur, (const xmlChar *)token);
    bool success = read((const char*)key,value);
    xmlFree(key);
    return success;
}

bool ReaderWriterP3DXML::getProperty(xmlNodePtr cur, const char* token, float& value) const
{
    xmlChar *key;
    key = xmlGetProp (cur, (const xmlChar *)token);
    bool success = read((const char*)key,value);
    xmlFree(key);
    return success;
}

bool ReaderWriterP3DXML::getProperty(xmlNodePtr cur, const char* token, double& value) const
{
    xmlChar *key;
    key = xmlGetProp (cur, (const xmlChar *)token);
    bool success = read((const char*)key,value);
    xmlFree(key);
    return success;
}

bool ReaderWriterP3DXML::getProperty(xmlNodePtr cur, const char* token, osg::Vec2& value) const
{
    xmlChar *key;
    key = xmlGetProp (cur, (const xmlChar *)token);
    bool success = read((const char*)key,value);
    xmlFree(key);
    return success;
}

bool ReaderWriterP3DXML::getProperty(xmlNodePtr cur, const char* token, osg::Vec3& value) const
{
    xmlChar *key;
    key = xmlGetProp (cur, (const xmlChar *)token);
    bool success = read((const char*)key,value);
    xmlFree(key);
    return success;
}

bool ReaderWriterP3DXML::getProperty(xmlNodePtr cur, const char* token, osg::Vec4& value) const
{
    xmlChar *key;
    key = xmlGetProp (cur, (const xmlChar *)token);
    bool success = read((const char*)key,value);
    xmlFree(key);
    return success;
}

bool ReaderWriterP3DXML::getProperty(xmlNodePtr cur, const char* token, std::string& value) const
{
    bool success = false;
    xmlChar *key;
    key = xmlGetProp (cur, (const xmlChar *)token);
    if (key)
    {
        success = true;
        value = (const char*)key;
    }
    xmlFree(key);
    return success;
}

bool ReaderWriterP3DXML::getProperty(xmlNodePtr cur, const char* token, osgText::Text::Layout& value) const
{
    bool success = false;
    xmlChar *key;
    key = xmlGetProp (cur, (const xmlChar *)token);
    if (key)
    {
        success = true;
        std::string str = (const char*)key;
        LayoutMap::const_iterator itr = _layoutMap.find(str);
        if (itr!=_layoutMap.end())
        {
            value = itr->second;
        }
    }
    xmlFree(key);
    return success;
}

bool ReaderWriterP3DXML::getProperty(xmlNodePtr cur, const char* token, osgText::Text::AlignmentType& value) const
{
    bool success = false;
    xmlChar *key;
    key = xmlGetProp (cur, (const xmlChar *)token);
    if (key)
    {
        success = true;
        std::string str = (const char*)key;
        AlignmentMap::const_iterator itr = _alignmentMap.find(str);
        if (itr!=_alignmentMap.end())
        {
            value = itr->second;
        }
    }
    xmlFree(key);
    return success;
}

bool ReaderWriterP3DXML::getProperties(xmlNodePtr cur, osgPresentation::SlideShowConstructor::PositionData& value) const
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
        else osg::notify(_notifyLevel)<<"Parser error - coordinate_frame=\""<<str<<"\" unrecongonized value"<<std::endl;
        
        osg::notify(_notifyLevel)<<"read coordinate_frame "<< ((value.frame==osgPresentation::SlideShowConstructor::MODEL) ? "osgPresentation::SlideShowConstructor::MODEL" : "osgPresentation::SlideShowConstructor::SLIDE")<<std::endl;
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

            if (fail) osg::notify(_notifyLevel)<<"Parser error - position=\""<<str<<"\" unrecongonized value"<<std::endl;
            else osg::notify(_notifyLevel)<<"Read position="<<value.position<<std::endl;
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

            if (fail) osg::notify(_notifyLevel)<<"Parser error - position=\""<<str<<"\" unrecongonized value"<<std::endl;
            else osg::notify(_notifyLevel)<<"Read position="<<value.position<<std::endl;
        }
    }

    
    if (getProperty(cur, "scale", scale))
    {
        value.scale.set(scale,scale,scale);
        propertiesRead = true;
        osg::notify(_notifyLevel)<<"scale read "<<scale<<std::endl;
    }

    if (getProperty(cur, "scale_x", scale) || getProperty(cur, "width", scale))
    {
        value.scale.x() = scale;
        propertiesRead = true;
        osg::notify(_notifyLevel)<<"scale read_x "<<scale<<std::endl;
    }

    if (getProperty(cur, "scale_y", scale) || getProperty(cur, "height", scale))
    {
        value.scale.y() = scale;
        propertiesRead = true;
        osg::notify(_notifyLevel)<<"scale read_y "<<scale<<std::endl;
    }

    if (getProperty(cur, "scale_z", scale))
    {
        value.scale.z() = scale;
        propertiesRead = true;
        osg::notify(_notifyLevel)<<"scale read_z "<<scale<<std::endl;
    }

    if (getProperty(cur, "rotate", rotate))
    {
        value.rotate = rotate;
        propertiesRead = true;
        osg::notify(_notifyLevel)<<"rotate read "<<rotate<<std::endl;
    }

    if (getProperty(cur, "rotate1", rotate))
    {
        // note may need to reverse once Quat * order is sorted out.
        value.rotate = accumulateRotation(rotate,value.rotate);
        propertiesRead = true;
        osg::notify(_notifyLevel)<<"rotate1 read "<<rotate<<std::endl;
    }

    if (getProperty(cur, "rotate2", rotate))
    {
        // note may need to reverse once Quat * order is sorted out.
        value.rotate = accumulateRotation(rotate,value.rotate);
        propertiesRead = true;
        osg::notify(_notifyLevel)<<"rotate1 read "<<rotate<<std::endl;
    }

    if (getProperty(cur, "rotate3", rotate))
    {
        // note may need to reverse once Quat * order is sorted out.
        value.rotate = accumulateRotation(rotate,value.rotate);
        propertiesRead = true;
        osg::notify(_notifyLevel)<<"rotate1 read "<<rotate<<std::endl;
    }

    if (getProperty(cur, "rotation", rotation))
    {
        value.rotation = rotation;
        osg::notify(_notifyLevel)<<"rotation read "<<rotation<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "rotation1", rotation))
    {
        value.rotation = accumulateRotation(rotation,value.rotation);
        osg::notify(_notifyLevel)<<"rotation1 read "<<rotation<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "rotation2", rotation))
    {
        value.rotation = accumulateRotation(rotation,value.rotation);
        osg::notify(_notifyLevel)<<"rotation2 read "<<rotation<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "rotation3", rotation))
    {
        value.rotation = accumulateRotation(rotation,value.rotation);
        osg::notify(_notifyLevel)<<"rotation3 read "<<rotation<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "path", str))
    {
    
        value.absolute_path = false;
        value.inverse_path = false;
        value.path = expandEnvVarsInFileName(str);

        osg::notify(_notifyLevel)<<"path read "<<str<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "camera_path", str))
    {
        value.absolute_path = true;
        value.inverse_path = true;
        value.path = expandEnvVarsInFileName(str);

        osg::notify(_notifyLevel)<<"camera path read "<<str<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "path_time_offset", value.path_time_offset))
    {
        osg::notify(_notifyLevel)<<"read path_time_offset"<<value.path_time_offset<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "path_time_multiplier", value.path_time_multiplier))
    {
        osg::notify(_notifyLevel)<<"read path_time_multiplier"<<value.path_time_multiplier<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "animation_material_time_offset", value.animation_material_time_offset))
    {
        osg::notify(_notifyLevel)<<"read animation_material_time_offset"<<value.animation_material_time_offset<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "animation_material_time_multiplier", value.animation_material_time_multiplier))
    {
        osg::notify(_notifyLevel)<<"read animation_material_time_multiplier"<<value.animation_material_time_multiplier<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "animation_material", str))
    {
        value.animation_material_filename = str;

        osg::notify(_notifyLevel)<<"animation_material read "<<str<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "animation_name", str))
    {
        value.animation_name = str;

        osg::notify(_notifyLevel)<<"animation_name "<<str<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "fade", value.fade))
    {
        osg::notify(_notifyLevel)<<"fade "<<value.fade<<std::endl;
        propertiesRead = true;
    }

    if (getProperty(cur, "path_loop_mode", str))
    {
        osg::notify(_notifyLevel)<<"path_loop_mode "<<str<<std::endl;
        if (str=="LOOP") value.path_loop_mode=osg::AnimationPath::LOOP;
        else if (str=="SWING") value.path_loop_mode=osg::AnimationPath::SWING;
        else if (str=="NO_LOOPING") value.path_loop_mode=osg::AnimationPath::NO_LOOPING;
        propertiesRead = true;
    }

    if (getProperty(cur, "animation_material_loop_mode", str))
    {
        osg::notify(_notifyLevel)<<"animation_material_loop_mode "<<str<<std::endl;
        if (str=="LOOP") value.animation_material_loop_mode=ss3d::AnimationMaterial::LOOP;
        else if (str=="SWING") value.animation_material_loop_mode=ss3d::AnimationMaterial::SWING;
        else if (str=="NO_LOOPING") value.animation_material_loop_mode=ss3d::AnimationMaterial::NO_LOOPING;
        propertiesRead = true;
    }

    return propertiesRead;
}

bool ReaderWriterP3DXML::getProperties(xmlNodePtr cur, osgPresentation::SlideShowConstructor::FontData& value) const
{
    bool propertiesRead=false;    
    
    osg::notify(_notifyLevel)<<"in getProperties(FontData)"<<std::endl;

    if (getProperty(cur, "font", value.font))
    {
        propertiesRead = true;
        osg::notify(_notifyLevel)<<"read font \""<<value.font<<"\""<<std::endl;
    }

    if (getProperty(cur, "character_size", value.characterSize))
    {
        propertiesRead = true;
        osg::notify(_notifyLevel)<<"read height \""<<value.characterSize<<"\""<<std::endl;
    }

    if (getProperty(cur, "layout", value.layout))
    {
        propertiesRead = true;
        
        osg::notify(_notifyLevel)<<"read layout \""<<value.layout<<"\""<<std::endl;
    }

    if (getProperty(cur, "alignment", value.alignment))
    {
        propertiesRead = true;
        
        osg::notify(_notifyLevel)<<"read alignment \""<<value.alignment<<"\""<<std::endl;
    }

    return propertiesRead;
}

bool ReaderWriterP3DXML::getProperties(xmlNodePtr cur, osgPresentation::SlideShowConstructor::ModelData& value) const
{
    bool propertiesRead=false;    
    
    osg::notify(_notifyLevel)<<"in getProperties(ModelData)"<<std::endl;

    if (getProperty(cur, "effect", value.effect))
    {
        propertiesRead = true;
        osg::notify(_notifyLevel)<<"read effect \""<<value.effect<<"\""<<std::endl;
    }

    return propertiesRead;
}

bool ReaderWriterP3DXML::getProperties(xmlNodePtr cur, osgPresentation::SlideShowConstructor::ImageData& value) const
{
    bool propertiesRead=false;    
    
    osg::notify(_notifyLevel)<<"in getProperties(ImageData)"<<std::endl;

    if (getProperty(cur, "page", value.page))
    {
        propertiesRead = true;
        osg::notify(_notifyLevel)<<"read page \""<<value.page<<"\""<<std::endl;
    }

    osg::Vec4 bgColour;
    if (getProperty(cur, "background", value.backgroundColor))
    {
        propertiesRead = true;
        osg::notify(_notifyLevel)<<"read background colour \""<<value.backgroundColor<<"\""<<std::endl;
    }

    if (getProperty(cur, "width", value.width))
    {
        propertiesRead = true;
        osg::notify(_notifyLevel)<<"read width \""<<value.width<<"\""<<std::endl;
    }

    if (getProperty(cur, "height", value.height))
    {
        propertiesRead = true;
        osg::notify(_notifyLevel)<<"read height \""<<value.height<<"\""<<std::endl;
    }

    if (getProperty(cur, "region", value.region))
    {
        propertiesRead = true;
        value.region_in_pixel_coords = false;
        osg::notify(_notifyLevel)<<"read region \""<<value.region<<"\""<<std::endl;
    }

    if (getProperty(cur, "pixel_region", value.region))
    {
        propertiesRead = true;
        value.region_in_pixel_coords = true;
        osg::notify(_notifyLevel)<<"read pixel_region \""<<value.region<<"\""<<std::endl;
    }

    std::string str;
    if (getProperty(cur, "looping", str))
    {
        propertiesRead = true;
        if (str=="ON") value.loopingMode = osg::ImageStream::LOOPING;
        else value.loopingMode = osg::ImageStream::NO_LOOPING;
        osg::notify(_notifyLevel)<<"looping \""<<str<<"\""<<std::endl;
    }

/*
    if (getProperty(cur, "texcoord_offset", value.texcoord_offset))
    {
        propertiesRead = true;
        osg::notify(_notifyLevel)<<"read offset \""<<value.texcoord_offset<<"\""<<std::endl;
    }

    if (getProperty(cur, "texcoord_scale", value.texcoord_scale))
    {
        propertiesRead = true;
        osg::notify(_notifyLevel)<<"read texcoord_scale \""<<value.texcoord_scale<<"\""<<std::endl;
    }
    
    if (getProperty(cur, "texcoord_rotate", value.texcoord_rotate))
    {
        propertiesRead = true;
        osg::notify(_notifyLevel)<<"read texcoord_rotate \""<<value.texcoord_rotate<<"\""<<std::endl;
    }
*/
    return propertiesRead;
}

bool ReaderWriterP3DXML::getJumpProperties(xmlNodePtr cur, bool& relativeJump, int& slideNum, int& layerNum) const
{
    bool propertyRead = false;

    if (getProperty(cur, "slide", slideNum)) 
    {
        osg::notify(osg::NOTICE)<<"slide "<<slideNum<<std::endl;
        propertyRead = true;
    }

    if (getProperty(cur, "layer", layerNum))
    {
        osg::notify(osg::NOTICE)<<"layer "<<layerNum<<std::endl;
        propertyRead = true;
    }

    std::string jumpType;
    if (getProperty(cur, "jump", jumpType))
    {
        osg::notify(osg::NOTICE)<<"jump "<<jumpType<<std::endl;
        propertyRead = true;
        relativeJump = (jumpType=="relative") || (jumpType=="Relative") || (jumpType=="RELATIVE") ;
    }

    return propertyRead;
}

void ReaderWriterP3DXML::parseModel(osgPresentation::SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur) const
{

    osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getModelPositionData();
   bool positionRead = getProperties(cur,positionData);

    osgPresentation::SlideShowConstructor::ModelData modelData;// = constructor.getModelData();
    getProperties(cur,modelData);

    std::string filename;
    xmlChar *key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
    if (key) filename = (const char*)key;
    xmlFree(key);
    
    if (!filename.empty()) 
    {
        constructor.addModel(filename,
                             positionRead ? positionData : constructor.getModelPositionData(), 
                             modelData);
    }

}

void ReaderWriterP3DXML::parseVolume(osgPresentation::SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur) const
{

    osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getModelPositionData();
    bool positionRead = getProperties(cur,positionData);

    std::string filename;
    xmlChar *key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
    if (key) filename = (const char*)key;
    xmlFree(key);
    
    if (!filename.empty()) 
    {
        constructor.addVolume(filename,
                             positionRead ? positionData : constructor.getModelPositionData());
    }
}

void ReaderWriterP3DXML::parseStereoPair(osgPresentation::SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur) const
{
    std::string filenameLeft;
    std::string filenameRight;

    osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getImagePositionData();
    bool positionRead = getProperties(cur,positionData);

    osgPresentation::SlideShowConstructor::ImageData imageDataLeft;// = constructor.getImageData();
    osgPresentation::SlideShowConstructor::ImageData imageDataRight;// = constructor.getImageData();

    xmlChar *key;
    cur = cur->xmlChildrenNode;

    while (cur != NULL)
    {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"image_left")))
        {
            getProperties(cur,imageDataLeft);

            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) filenameLeft = (const char*)key;
            xmlFree(key);
            

        }
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"image_right")))
        {
            getProperties(cur,imageDataRight);

            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) filenameRight = (const char*)key;
            xmlFree(key);

        }
        cur = cur->next;
    }
    
    if (!filenameLeft.empty() && !filenameRight.empty()) 
        constructor.addStereoImagePair(filenameLeft,imageDataLeft,
				       filenameRight, imageDataRight,
                                       positionRead ? positionData : constructor.getImagePositionData());

}

bool ReaderWriterP3DXML::getKeyPosition(xmlDocPtr doc, xmlNodePtr cur, osgPresentation::KeyPosition& keyPosition) const
{
    if ((!xmlStrcmp(cur->name, (const xmlChar *)"key")))
    {
        return getKeyPositionInner(doc, cur, keyPosition);
    }
    if ((!xmlStrcmp(cur->name, (const xmlChar *)"escape")) || 
        (!xmlStrcmp(cur->name, (const xmlChar *)"esc")) ||
        (!xmlStrcmp(cur->name, (const xmlChar *)"exit")))
    {
        keyPosition.set(osgGA::GUIEventAdapter::KEY_Escape, 0.0f, 0.0f);
        return true;
    }
    return false;
}

bool ReaderWriterP3DXML::getKeyPositionInner(xmlDocPtr doc, xmlNodePtr cur, osgPresentation::KeyPosition& keyPosition) const
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

    xmlChar* key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
    if (key)
    {

        unsigned int keyValue = 0;

        StringKeyMap::const_iterator itr=_stringKeyMap.find((const char*)key);
        if (itr != _stringKeyMap.end())
        {
            keyValue = itr->second;
        }
        else if (strlen((const char*)key)==1)
        {
            keyValue = key[0];
        }
        else
        {
            osg::notify(osg::NOTICE)<<"Warning: unreconginized key sequence '"<<(const char*) key<<"'"<<std::endl;
        }

        keyPosition.set(keyValue,x,y);
        return true;
    }

    return false;
}




void ReaderWriterP3DXML::parseLayer(osgPresentation::SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur) const
{
    // create a keyPosition just in case we need it.
    osgPresentation::KeyPosition keyPosition;

    osg::notify(osg::INFO)<<std::endl<<"parseLayer"<<std::endl;

    float totalIndent = 0.0f;

    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"run")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) 
            {
                osg::notify(osg::INFO)<<"run ["<<(const char*)key<<"]"<<std::endl;
                constructor.addLayerRunString((const char*)key);
            }
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"jump")))
        {
            osg::notify(osg::NOTICE)<<"Parsed Jump "<<std::endl;

            bool relativeJump = true;
            int slideNum = 0;
            int layerNum = 0;
            if (getJumpProperties(cur, relativeJump, slideNum, layerNum))
            {
                osg::notify(osg::NOTICE)<<"Layer Jump "<<relativeJump<<","<< slideNum<<", "<<layerNum<<std::endl;
            
                constructor.setLayerJump(relativeJump, slideNum, layerNum);
            }
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"click_to_run")))
        {
            bool relativeJump = true;
            int slideNum = 0;
            int layerNum = 0;
            getJumpProperties(cur, relativeJump, slideNum, layerNum);

            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) 
            {
                osg::notify(osg::INFO)<<"click_to_run ["<<(const char*)key<<"]"<<std::endl;
                constructor.layerClickToDoOperation((const char*)key,osgPresentation::RUN, relativeJump, slideNum, layerNum);
            }
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"click_to_load")))
        {
            bool relativeJump = true;
            int slideNum = 0;
            int layerNum = 0;
            getJumpProperties(cur, relativeJump, slideNum, layerNum);

            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) 
            {
                osg::notify(osg::INFO)<<"click_to_load ["<<(const char*)key<<"]"<<std::endl;
                constructor.layerClickToDoOperation((const char*)key,osgPresentation::LOAD, relativeJump, slideNum, layerNum);
            }
            xmlFree(key);
        }

        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"click_to_event")))
        {
            bool relativeJump = true;
            int slideNum = 0;
            int layerNum = 0;
            getJumpProperties(cur, relativeJump, slideNum, layerNum);

            if (getKeyPositionInner(doc, cur, keyPosition))
            {
                osg::notify(osg::INFO)<<"click_to_event ["<<keyPosition._key<<"]"<<std::endl;
                constructor.layerClickEventOperation(keyPosition, relativeJump, slideNum, layerNum);
            }
        }

        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"click_to_jump")))
        {
            bool relativeJump = true;
            int slideNum = 0;
            int layerNum = 0;
            getJumpProperties(cur, relativeJump, slideNum, layerNum);

            constructor.layerClickEventOperation(osgPresentation::JUMP, relativeJump, slideNum, layerNum);
        }

        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"newline")))
        {
            constructor.translateTextCursor(osg::Vec3(0.0f,-0.05f,0.0f));
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"indent")))
        {
            float localIndent = 0.05f;
            constructor.translateTextCursor(osg::Vec3(localIndent,0.0f,0.0f));
            totalIndent += localIndent;
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"unindent")))
        {
            float localIndent = -0.05f;
            constructor.translateTextCursor(osg::Vec3(localIndent,0.0f,0.0f));
            totalIndent += localIndent;
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"bullet")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) 
            {
                 osg::notify(osg::INFO)<<"bullet ["<<(const char*)key<<"]"<<std::endl;
                osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getTextPositionData();
                bool positionRead = getProperties(cur,positionData);

                osgPresentation::SlideShowConstructor::FontData fontData = constructor.getTextFontData();
                bool fontRead = getProperties(cur,fontData);

                constructor.addBullet((const char*)key, 
                                      positionRead ? positionData : constructor.getTextPositionData(),
                                      fontRead ? fontData : constructor.getTextFontData());
            }
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"paragraph")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) 
            {
                osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getTextPositionData();
                bool positionRead = getProperties(cur,positionData);

                osgPresentation::SlideShowConstructor::FontData fontData = constructor.getTextFontData();
                bool fontRead = getProperties(cur,fontData);

                constructor.addParagraph((const char*)key, 
                                          positionRead ? positionData : constructor.getTextPositionData(),
                                          fontRead ? fontData : constructor.getTextFontData());
            }
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"image")))
        {
            std::string filename;
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) 
            {
                osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getImagePositionData();
                bool positionRead = getProperties(cur,positionData);

                osgPresentation::SlideShowConstructor::ImageData imageData;// = constructor.getImageData();
                getProperties(cur,imageData);

                constructor.addImage((const char*)key, 
                                     positionRead ? positionData : constructor.getImagePositionData(),
                                     imageData);
            }
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"vnc")))
        {
            std::string filename;
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) 
            {
                osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getImagePositionData();
                bool positionRead = getProperties(cur,positionData);

                osgPresentation::SlideShowConstructor::ImageData imageData;// = constructor.getImageData();
                getProperties(cur,imageData);

                constructor.addVNC((const char*)key, 
                                     positionRead ? positionData : constructor.getImagePositionData(),
                                     imageData);
            }
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"browser")))
        {
            std::string filename;
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) 
            {
                osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getImagePositionData();
                bool positionRead = getProperties(cur,positionData);

                osgPresentation::SlideShowConstructor::ImageData imageData;// = constructor.getImageData();
                getProperties(cur,imageData);

                constructor.addBrowser((const char*)key, 
                                     positionRead ? positionData : constructor.getImagePositionData(),
                                     imageData);
            }
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"pdf")))
        {
            std::string filename;
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) 
            {
                osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getImagePositionData();
                bool positionRead = getProperties(cur,positionData);

                osgPresentation::SlideShowConstructor::ImageData imageData;// = constructor.getImageData();
                getProperties(cur,imageData);

                constructor.addPDF((const char*)key, 
                                     positionRead ? positionData : constructor.getImagePositionData(),
                                     imageData);
            }
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"stereo_pair")))
        {
            parseStereoPair(constructor, doc,cur);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"model")))
        {
            parseModel(constructor, doc,cur);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"volume")))
        {
            parseVolume(constructor, doc,cur);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"duration")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) constructor.setLayerDuration(atof((const char*)key));
            xmlFree(key);
        }
        else if (getKeyPosition(doc, cur, keyPosition))
        {
            constructor.addLayerKey(keyPosition);
        }
        cur = cur->next;
    }

    if (totalIndent != 0.0f)
    {
        constructor.translateTextCursor(osg::Vec3(-totalIndent,0.0f,0.0f));
    }

}

void ReaderWriterP3DXML::parseBullets(osgPresentation::SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur, bool inheritPreviousLayers, bool defineAsBaseLayer) const
{
    xmlChar *key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
    if (key) 
    {
        constructor.addLayer(inheritPreviousLayers, defineAsBaseLayer);

        osg::notify(osg::INFO)<<"bullets ["<<(const char*)key<<"]"<<std::endl;
        osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getTextPositionData();
        bool positionRead = getProperties(cur,positionData);

        osgPresentation::SlideShowConstructor::FontData fontData = constructor.getTextFontData();
        bool fontRead = getProperties(cur,fontData);

        constructor.addBullet((const char*)key,
                               positionRead ? positionData : constructor.getTextPositionData(),
                               fontRead ? fontData : constructor.getTextFontData()); 
    }
    xmlFree(key);
}


void ReaderWriterP3DXML::parseText(osgPresentation::SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur, bool inheritPreviousLayers, bool defineAsBaseLayer) const
{
    xmlChar *key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
    if (key) 
    {
        constructor.addLayer(inheritPreviousLayers, defineAsBaseLayer);

        osg::notify(osg::INFO)<<"text ["<<(const char*)key<<"]"<<std::endl;
        osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getTextPositionData();
        bool positionRead = getProperties(cur,positionData);

        osgPresentation::SlideShowConstructor::FontData fontData = constructor.getTextFontData();
        bool fontRead = getProperties(cur,fontData);

        constructor.addParagraph((const char*)key,
                               positionRead ? positionData : constructor.getTextPositionData(),
                               fontRead ? fontData : constructor.getTextFontData()); 
    }
    xmlFree(key);
}

void ReaderWriterP3DXML::parsePage(osgPresentation::SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur) const
{
    xmlChar *key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
    if (key) 
    {
        constructor.addSlide();

        std::string title;
        getProperty(cur, "title", title);
        
        std::string inherit;
        getProperty(cur, "inherit", inherit);
        
        if (!inherit.empty() && _templateMap.count(inherit)!=0)
        {
            parseSlide(constructor, _templateMap[inherit].first, _templateMap[inherit].second, true, false);
        }

        if (!title.empty())
        {
            constructor.setSlideTitle(title,
                                      constructor.getTitlePositionData(),
                                      constructor.getTitleFontData());
        }
        
        if (!inherit.empty() && _templateMap.count(inherit)!=0)
        {
            parseSlide(constructor, _templateMap[inherit].first, _templateMap[inherit].second, false, true);
        }

        constructor.addLayer(true,false);

        osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getTextPositionData();
        bool positionRead = getProperties(cur,positionData);

        osgPresentation::SlideShowConstructor::FontData fontData = constructor.getTextFontData();
        bool fontRead = getProperties(cur,fontData);

        constructor.addParagraph((const char*)key,
                               positionRead ? positionData : constructor.getTextPositionData(),
                               fontRead ? fontData : constructor.getTextFontData()); 
    }
    xmlFree(key);
}

void ReaderWriterP3DXML::parsePdfDocument(osgPresentation::SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur) const
{
    xmlChar *key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
    if (key) 
    {

        std::string title;
        getProperty(cur, "title", title);

        std::string inherit;
        getProperty(cur, "inherit", inherit);
        
        constructor.addSlide();
        
        if (!inherit.empty() && _templateMap.count(inherit)!=0)
        {
            parseSlide(constructor, _templateMap[inherit].first, _templateMap[inherit].second, true, false);
        }

        if (!title.empty())
        {
            constructor.setSlideTitle(title,
                                      constructor.getTitlePositionData(),
                                      constructor.getTitleFontData());
        }
        
        if (!inherit.empty() && _templateMap.count(inherit)!=0)
        {
            parseSlide(constructor, _templateMap[inherit].first, _templateMap[inherit].second, false, true);
        }

        constructor.addLayer(true,false);

        osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getImagePositionData();
        getProperties(cur,positionData);

        osgPresentation::SlideShowConstructor::ImageData imageData;// = constructor.getImageData();
        imageData.page = 0;
        getProperties(cur,imageData);

        osg::Image* image = constructor.addInteractiveImage((const char*)key, positionData, imageData);
        osgWidget::PdfImage* pdfImage = dynamic_cast<osgWidget::PdfImage*>(image);
        if (pdfImage)
        {
            int numPages = pdfImage->getNumOfPages();
            osg::notify(osg::NOTICE)<<"NumOfPages = "<<numPages<<std::endl;
            
            if (numPages>1)
            {
                for(int pageNum=1; pageNum<numPages; ++pageNum)
                {
                    imageData.page = pageNum;
                
                    constructor.addSlide();

                    if (!inherit.empty() && _templateMap.count(inherit)!=0)
                    {
                        parseSlide(constructor, _templateMap[inherit].first, _templateMap[inherit].second, true, false);
                    }

                    if (!title.empty())
                    {
                        constructor.setSlideTitle(title,
                                                  constructor.getTitlePositionData(),
                                                  constructor.getTitleFontData());
                    }

                    if (!inherit.empty() && _templateMap.count(inherit)!=0)
                    {
                        parseSlide(constructor, _templateMap[inherit].first, _templateMap[inherit].second, false, true);
                    }

                    constructor.addLayer(true,false);

                    constructor.addPDF((const char*)key, positionData, imageData);

                }
            }
            
        }
    }
    xmlFree(key);
}

void ReaderWriterP3DXML::parseSlide (osgPresentation::SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur, bool parseTitles, bool parseLayers) const
{

    osg::Vec4 previous_bgcolor = constructor.getBackgroundColor();
    osg::Vec4 previous_textcolor = constructor.getTextColor();

    // create a keyPosition just in case we need it.
    osgPresentation::KeyPosition keyPosition;

    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        if (parseTitles)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"title")))
            {
                key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);

                if (key)
                {
                    osgPresentation::SlideShowConstructor::PositionData positionData = constructor.getTitlePositionData();
                    bool positionRead = getProperties(cur,positionData);

                    osgPresentation::SlideShowConstructor::FontData fontData = constructor.getTitleFontData();
                    bool fontRead = getProperties(cur,fontData);

                    constructor.setSlideTitle((const char*)key,
                                              positionRead ? positionData : constructor.getTitlePositionData(),
                                              fontRead ? fontData : constructor.getTitleFontData());

                    xmlFree(key);
                }
                else constructor.setSlideTitle("", constructor.getTitlePositionData(), constructor.getTitleFontData());
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"background")))
            {
                key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                if (key) constructor.setSlideBackground((const char*)key);
                else constructor.setSlideBackground("");
                xmlFree(key);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"bgcolor")))
            {
                key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                if (key) constructor.setBackgroundColor(mapStringToColor((const char*)key),true);
                xmlFree(key);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"textcolor")))
            {
                key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                if (key) constructor.setTextColor(mapStringToColor((const char*)key));
                xmlFree(key);
            }
        }
        if (parseLayers)
        {
            if ((!xmlStrcmp(cur->name, (const xmlChar *)"base")))
            {
                constructor.addLayer(true, true);
                parseLayer (constructor, doc, cur);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"layer")))
            {
                constructor.addLayer(true, false);
                parseLayer (constructor, doc, cur);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"clean_layer")))
            {
                constructor.addLayer(false, false);
                parseLayer (constructor, doc, cur);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"modify_layer")))
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

                parseLayer (constructor, doc, cur);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"bullets")))
            {
                parseBullets (constructor, doc, cur,true, false);
            }
            else if ((!xmlStrcmp(cur->name, (const xmlChar *)"duration")))
            {
                key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
                if (key) constructor.setSlideDuration(atof((const char*)key));
                xmlFree(key);
            }
            else if (getKeyPosition(doc, cur, keyPosition))
            {
                constructor.addSlideKey(keyPosition);
            }
        }
        cur = cur->next;
    }

    constructor.setBackgroundColor(previous_bgcolor,false);
    constructor.setTextColor(previous_textcolor);

    return;
}

osgDB::ReaderWriter::ReadResult ReaderWriterP3DXML::readNode(const std::string& file,
                                                           const osgDB::ReaderWriter::Options* options) const
{

    bool readOnlyHoldingPage = options ? options->getOptionString()=="holding_slide" : false;

    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    std::string fileName = osgDB::findDataFile( file );
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    // create a keyPosition just in case we need it.
    osgPresentation::KeyPosition keyPosition;

    xmlDocPtr doc;
    xmlNodePtr cur;

    doc = xmlParseFile(fileName.c_str());

    if (doc == NULL ) {
            fprintf(stderr,"Document not parsed successfully. \n");
            return ReadResult::FILE_NOT_HANDLED;
    }

     cur = xmlDocGetRootElement(doc);

     if (cur == NULL) {
            fprintf(stderr,"empty document\n");
            xmlFreeDoc(doc);
            return ReadResult::FILE_NOT_HANDLED;
    }

     if (xmlStrcmp(cur->name, (const xmlChar *) "presentation")) {
            fprintf(stderr,"document of the wrong type, root node != presentation");
            xmlFreeDoc(doc);
            return ReadResult::FILE_NOT_HANDLED;
    }

    osgPresentation::SlideShowConstructor constructor;
          
    osgDB::FilePathList previousPaths = osgDB::getDataFilePathList();
          
    bool readSlide = false;
    
    
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL) {


        if ((!xmlStrcmp(cur->name, (const xmlChar *)"name")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) constructor.setPresentationName((const char*)key);
            else constructor.setPresentationName("");
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"loop")))
        {
            constructor.setLoopPresentation(true);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"auto")))
        {
            constructor.setAutoSteppingActive(true);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"title-settings")))
        {
            bool fontRead = getProperties(cur,constructor.getTitleFontDataDefault());
            if (fontRead) 
            {
                osg::notify(osg::INFO)<<"Title font details read"<<std::endl;
            }
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"text-settings")))
        {
            bool fontRead = getProperties(cur,constructor.getTextFontDataDefault());
            if (fontRead) 
            {
                osg::notify(osg::INFO)<<"Text font details read"<<std::endl;
            }
        }
        /*else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ratio")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) constructor.setPresentationAspectRatio((const char*)key);
            xmlFree(key);
        }*/
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"path")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) 
            {
                osg::notify(osg::INFO)<<"Appending search path "<<(char*)key<<std::endl;
                osgDB::getDataFilePathList().push_front(expandEnvVarsInFileName(std::string((char*)key)));
            }
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"bgcolor")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) constructor.setBackgroundColor(mapStringToColor((const char*)key),false);
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"textcolor")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) constructor.setTextColor(mapStringToColor((const char*)key));
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"duration")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) constructor.setPresentationDuration(atof((const char*)key));
            xmlFree(key);
        }
        else if (getKeyPosition(doc, cur, keyPosition))
        {
            constructor.addPresentationKey(keyPosition);
        }
        else if (readOnlyHoldingPage && (!xmlStrcmp(cur->name, (const xmlChar *)"holding_slide")))
        {
            readSlide = true;
            constructor.addSlide();
            parseSlide (constructor, doc, cur);
        }
        else if (!readOnlyHoldingPage && (!xmlStrcmp(cur->name, (const xmlChar *)"slide")))
        {
            readSlide = true;
            constructor.addSlide();

            std::string inherit;
            if (getProperty(cur, "inherit", inherit) && !inherit.empty() && _templateMap.count(inherit)!=0)
            {
                parseSlide(constructor, _templateMap[inherit].first, _templateMap[inherit].second, true, false);
                parseSlide (constructor, doc, cur, true, false);
                parseSlide(constructor, _templateMap[inherit].first, _templateMap[inherit].second, false, true);
                parseSlide (constructor, doc, cur, false, true);
            }
            else
            {
                parseSlide (constructor, doc, cur);
            }
        }
        else if (!readOnlyHoldingPage && (!xmlStrcmp(cur->name, (const xmlChar *)"modify_slide")))
        {
            readSlide = true;
            int slideNum;
            if (getProperty(cur, "slide", slideNum))
            {            
                constructor.selectSlide(slideNum);
                parseSlide (constructor, doc, cur);
            }
            else
            {
                constructor.addSlide();
            }
        }
        else if (!readOnlyHoldingPage && (!xmlStrcmp(cur->name, (const xmlChar *)"page")))
        {
            readSlide = true;
            parsePage (constructor, doc, cur);
        }
        else if (!readOnlyHoldingPage && (!xmlStrcmp(cur->name, (const xmlChar *)"pdf_document")))
        {
            readSlide = true;
            parsePdfDocument(constructor, doc, cur);
        }
        else if (!readOnlyHoldingPage && (!xmlStrcmp(cur->name, (const xmlChar *)"template_slide")))
        {
            readSlide = true;
            std::string name;
            if (getProperty(cur, "name", name))
            {            
                _templateMap[name] = DocNodePair(doc,cur);
                std::cout<<"Defining template slide "<<name<<std::endl;
            }
        }

        cur = cur->next;
    }

    xmlFreeDoc(doc);
    
    osgDB::getDataFilePathList() = previousPaths;
    

    return constructor.takePresentation();
    /*
    std::cout<<"readSlide="<<readSlide<<std::endl;
    std::cout<<"node="<<node<<std::endl;
    return node;*/
}

