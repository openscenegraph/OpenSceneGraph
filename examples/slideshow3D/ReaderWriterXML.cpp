#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#include "SlideShowConstructor.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <sstream>


/**
 * OpenSceneGraph plugin wrapper/converter.
 */
class ReaderWriterSS3D : public osgDB::ReaderWriter
{
public:
    ReaderWriterSS3D()
    {
        _colorMap["WHITE"]  .set(1.0f,1.0f,1.0f,1.0f);
        _colorMap["BLACK"]  .set(0.0f,0.0f,0.0f,1.0f);
        _colorMap["PURPLE"] .set(1.0f,0.0f,1.0f,1.0f);
        _colorMap["BLUE"]   .set(0.0f,0.0f,1.0f,1.0f);
        _colorMap["RED"]    .set(1.0f,0.0f,0.0f,1.0f);
        _colorMap["CYAN"]   .set(0.0f,1.0f,1.0f,1.0f);
        _colorMap["YELLOW"] .set(1.0f,1.0f,0.0f,1.0f);
        _colorMap["GREEN"]  .set(0.0f,1.0f,0.0f,1.0f);
    }
    
    virtual const char* className()
    {
        return "slideshow3D XML Reader/Writer";
    }

    virtual bool acceptsExtension(const std::string& extension)
    { 
        return osgDB::equalCaseInsensitive(extension,"ss3d") ||
               osgDB::equalCaseInsensitive(extension,"xml") ;
    }

    virtual ReadResult readNode(const std::string& fileName,
                                const osgDB::ReaderWriter::Options* options);


    void parseModel(SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur);

    void parseStereoPair(SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur);

    void parseLayer(SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur);

    void parseSlide (SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur);

    osg::Vec4 mapStringToColor(const std::string& str)
    {
        return _colorMap[str];
    }
    
    inline bool read(const char* str, float& value) const;
    inline bool read(const char* str, osg::Vec2& value) const;
    inline bool read(const char* str, osg::Vec3& value) const;
    inline bool read(const char* str, osg::Vec4& value) const;
    
    inline bool read(const std::string& str, float& value) const;
    inline bool read(const std::string& str, osg::Vec2& value) const;
    inline bool read(const std::string& str, osg::Vec3& value) const;
    inline bool read(const std::string& str, osg::Vec4& value) const;

    bool getProperty(xmlNodePtr cur, const char* token) const;
    bool getProperty(xmlNodePtr cur, const char* token, float& value) const;
    bool getProperty(xmlNodePtr cur, const char* token, osg::Vec2& value) const;
    bool getProperty(xmlNodePtr cur, const char* token, osg::Vec3& value) const;
    bool getProperty(xmlNodePtr cur, const char* token, osg::Vec4& value) const;
    bool getProperty(xmlNodePtr cur, const char* token, std::string& value) const;
 
    std::map<std::string,osg::Vec4> _colorMap;

};

// Register with Registry to instantiate the above reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterSS3D> g_readerWriter_SS3D_Proxy;

bool ReaderWriterSS3D::read(const char* str, float& value) const
{
    if (!str) return false;
    std::istringstream iss((const char*)str);
    iss >> value;
    return !iss.fail();
}

bool ReaderWriterSS3D::read(const char* str, osg::Vec2& value) const
{
    if (!str) return false;
    std::istringstream iss((const char*)str);
    iss >> value.x() >> value.y();
    return !iss.fail();
}

bool ReaderWriterSS3D::read(const char* str, osg::Vec3& value) const
{
    if (!str) return false;
    std::istringstream iss((const char*)str);
    iss >> value.x() >> value.y() >> value.z();
    return !iss.fail();
}

bool ReaderWriterSS3D::read(const char* str, osg::Vec4& value) const
{
    if (!str) return false;
    std::istringstream iss((const char*)str);
    iss >> value.x() >> value.y() >> value.z() >> value.w();
    return !iss.fail();
}


bool ReaderWriterSS3D::read(const std::string& str, float& value) const
{
    std::istringstream iss(str);
    iss >> value;
    return !iss.fail();
}

bool ReaderWriterSS3D::read(const std::string& str, osg::Vec2& value) const
{
    std::istringstream iss(str);
    iss >> value.x() >> value.y();
    return !iss.fail();
}

bool ReaderWriterSS3D::read(const std::string& str, osg::Vec3& value) const
{
    std::istringstream iss(str);
    iss >> value.x() >> value.y() >> value.z();
    return !iss.fail();
}

bool ReaderWriterSS3D::read(const std::string& str, osg::Vec4& value) const
{
    std::istringstream iss(str);
    iss >> value.x() >> value.y() >> value.z() >> value.w();
    return !iss.fail();
}

bool ReaderWriterSS3D::getProperty(xmlNodePtr cur, const char* token) const
{
    bool success = false;
    xmlChar *key;
    key = xmlGetProp (cur, (const xmlChar *)token);
    if (key) success=true;
    xmlFree(key);
    return success;
}

bool ReaderWriterSS3D::getProperty(xmlNodePtr cur, const char* token, float& value) const
{
    xmlChar *key;
    key = xmlGetProp (cur, (const xmlChar *)token);
    bool success = read((const char*)key,value);
    xmlFree(key);
    return success;
}

bool ReaderWriterSS3D::getProperty(xmlNodePtr cur, const char* token, osg::Vec2& value) const
{
    xmlChar *key;
    key = xmlGetProp (cur, (const xmlChar *)token);
    bool success = read((const char*)key,value);
    xmlFree(key);
    return success;
}

bool ReaderWriterSS3D::getProperty(xmlNodePtr cur, const char* token, osg::Vec3& value) const
{
    xmlChar *key;
    key = xmlGetProp (cur, (const xmlChar *)token);
    bool success = read((const char*)key,value);
    xmlFree(key);
    return success;
}

bool ReaderWriterSS3D::getProperty(xmlNodePtr cur, const char* token, osg::Vec4& value) const
{
    xmlChar *key;
    key = xmlGetProp (cur, (const xmlChar *)token);
    bool success = read((const char*)key,value);
    xmlFree(key);
    return success;
}

bool ReaderWriterSS3D::getProperty(xmlNodePtr cur, const char* token, std::string& value) const
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


void ReaderWriterSS3D::parseModel(SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur)
{
    std::string filename;

    SlideShowConstructor::CoordinateFrame coordinate_frame = SlideShowConstructor::SLIDE;
    osg::Vec3 position(0.0f,1.0f,0.0f);
    osg::Vec4 rotate(0.0f,0.0f,0.0f,1.0f);
    float scale = 1.0f;

    osg::Vec4 rotation(0.0f,0.0f,0.0f,1.0f);
    std::string animation_path;
    std::string camera_path;

    // temporary
    std::string str;

    if (getProperty(cur, "coordinate_frame", str))
    {
        if (str=="model") coordinate_frame = SlideShowConstructor::MODEL;
        else if (str=="slide") coordinate_frame = SlideShowConstructor::SLIDE;
        else std::cout<<"Parser error - coordinate_frame=\""<<str<<"\" unrecongonized value"<<std::endl;
    }

    if (getProperty(cur, "position", str))
    {
        bool fail = false;
        if (str=="center") position.set(0.0f,1.0f,0.0f);
        else if (str=="eye") position.set(0.0f,0.0f,0.0f);
        else if (!read(str,position)) fail = true;
        
        if (fail) std::cout<<"Parser error - position=\""<<str<<"\" unrecongonized value"<<std::endl;
        else std::cout<<"Read position="<<position<<std::endl;
    }
    
    if (getProperty(cur, "scale", scale))
    {
        std::cout<<"scale read "<<scale<<std::endl;
    }

    if (getProperty(cur, "rotate", rotate))
    {
        std::cout<<"rotate read "<<rotate<<std::endl;
    }

    if (getProperty(cur, "rotation", rotation))
    {
        std::cout<<"rotation read "<<rotation<<std::endl;
    }

    if (getProperty(cur, "path", animation_path))
    {
        std::cout<<"path read "<<animation_path<<std::endl;
    }

    if (getProperty(cur, "camera_path", camera_path))
    {
        std::cout<<"camera path read "<<camera_path<<std::endl;
    }

    xmlChar *key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
    if (key) filename = (const char*)key;
    xmlFree(key);
    
    if (!filename.empty()) 
    {
        if (!camera_path.empty())
        {
            constructor.addModelWithCameraPath(filename,coordinate_frame,position,scale,rotate,camera_path);
        }
        else if (!animation_path.empty())
        {
            constructor.addModelWithPath(filename,coordinate_frame,position,scale,rotate,animation_path);
        }
        else 
        {
            constructor.addModel(filename,coordinate_frame,position,scale,rotate,rotation);
        }
    }
}

void ReaderWriterSS3D::parseStereoPair(SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur)
{
    std::string filenameLeft;
    std::string filenameRight;

    float height = 1.0f;
    xmlChar *key;
    key = xmlGetProp (cur, (const xmlChar *)"height");
    if (key) height = atof((const char*)key);
    xmlFree(key);

    cur = cur->xmlChildrenNode;

    while (cur != NULL)
    {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"image_left")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) filenameLeft = (const char*)key;
            xmlFree(key);
            

        }
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"image_right")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) filenameRight = (const char*)key;
            xmlFree(key);

        }
        cur = cur->next;
    }
    
    if (!filenameLeft.empty() && !filenameRight.empty()) 
        constructor.addStereoImagePair(filenameLeft,filenameRight,height);

}

void ReaderWriterSS3D::parseLayer(SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur)
{
    constructor.addLayer();

    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"bullet")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) constructor.addBullet((const char*)key);
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"paragraph")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) constructor.addParagraph((const char*)key);
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"image")))
        {
            std::string filename;
            float height = 1.0f;

            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) filename = (const char*)key;
            xmlFree(key);
            
            key = xmlGetProp (cur, (const xmlChar *)"height");
            if (key) height = atoi((const char*)key);
            xmlFree(key);

            if (!filename.empty()) constructor.addImage(filename,height);

        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"stereo_pair")))
        {
            parseStereoPair(constructor, doc,cur);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"model")))
        {
            parseModel(constructor, doc,cur);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"duration")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) constructor.setLayerDuration(atof((const char*)key));
            xmlFree(key);
        }
        cur = cur->next;
    }
}


void ReaderWriterSS3D::parseSlide (SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur)
{

    constructor.addSlide();
    
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"title")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) constructor.setSlideTitle((const char*)key);
            else constructor.setSlideTitle("");
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"background")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) constructor.setSlideBackground((const char*)key);
            else constructor.setSlideBackground("");
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"layer")))
        {
            parseLayer (constructor, doc, cur);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"duration")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) constructor.setSlideDuration(atof((const char*)key));
            xmlFree(key);
        }
        cur = cur->next;
    }
    return;
}

osgDB::ReaderWriter::ReadResult ReaderWriterSS3D::readNode(const std::string& fileName,
                                                           const osgDB::ReaderWriter::Options*)
{
    std::string ext = osgDB::getLowerCaseFileExtension(fileName);
    if (!acceptsExtension(ext))
        return ReadResult::FILE_NOT_HANDLED;



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
            fprintf(stderr,"document of the wrong type, root node != story");
            xmlFreeDoc(doc);
            return ReadResult::FILE_NOT_HANDLED;
    }

    SlideShowConstructor constructor;
    
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
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"ratio")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) constructor.setPresentationAspectRatio((const char*)key);
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"bgcolor")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) constructor.setBackgroundColor(mapStringToColor((const char*)key));
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
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"slide")))
        {
            parseSlide (constructor, doc, cur);
        }

        cur = cur->next;
    }

    xmlFreeDoc(doc);

    return constructor.takePresentation();
}

