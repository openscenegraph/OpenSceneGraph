#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#include "SlideShowConstructor.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/xmlmemory.h>
#include <libxml/parser.h>


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

    void parseLayer(SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur);

    void parseSlide (SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur);

    osg::Vec4 mapStringToColor(const std::string& str)
    {
        return _colorMap[str];
    }
    
    
    std::map<std::string,osg::Vec4> _colorMap;

};

// Register with Registry to instantiate the above reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterSS3D> g_readerWriter_SS3D_Proxy;


void ReaderWriterSS3D::parseModel(SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur)
{
    std::string filename;
    float scale = 1.0f;
    float rotation = 0.0f;
    float position = 0.5f;

    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"filename")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) filename = (const char*)key;
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"scale")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) scale = atoi((const char*)key);
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"rotation")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) rotation = atoi((const char*)key);
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"position")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key) position = atoi((const char*)key)/100.0f;
            xmlFree(key);
        }
        cur = cur->next;
    }
    
    if (!filename.empty()) constructor.addModel(filename,scale,rotation,position);
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
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"model")))
        {
            parseModel(constructor, doc,cur);
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
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"slide")))
        {
            parseSlide (constructor, doc, cur);
        }

        cur = cur->next;
    }

    xmlFreeDoc(doc);

    return constructor.takePresentation();
}

