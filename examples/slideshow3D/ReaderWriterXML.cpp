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
    ReaderWriterSS3D() { }

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
        if (str=="BLACK") return osg::Vec4(0.0f,0.0f,0.0f,1.0f);
        else return osg::Vec4(1.0f,1.0f,1.0f,1.0f);
    }

};

// Register with Registry to instantiate the above reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterSS3D> g_readerWriter_SS3D_Proxy;


void ReaderWriterSS3D::parseModel(SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur)
{
    std::string filename;
    float scale = 1.0f;
    float rotation = 0.0f;
    float position = 0.5f;

    printf("        new model\n");
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"filename")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            printf("            filename: %s\n", key);
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"scale")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            printf("            scale: %s\n", key);
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"rotation")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            printf("            rotation: %s\n", key);
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"position")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            printf("            position: %s\n", key);
            xmlFree(key);
        }
        cur = cur->next;
    }
    
    if (!filename.empty()) constructor.addModel(filename,scale,rotation,position);
}

void ReaderWriterSS3D::parseLayer(SlideShowConstructor& constructor, xmlDocPtr doc, xmlNodePtr cur)
{
    constructor.addLayer();

    printf("    new layer\n");
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"bullet")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            constructor.addBullet((const char*)key);
            printf("        bullet: %s\n", key);
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"paragraph")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            printf("        paragraph: %s\n", key);
            constructor.addParagraph((const char*)key);
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"image")))
        {
            std::string filename;
            float height = 1.0f;

            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            printf("        image: %s\n", key);
            filename = (const char*)key;
            xmlFree(key);
            
            key = xmlGetProp (cur, (const xmlChar *)"height");
            if (key)
            {
                printf("            height: %s\n", key);
                height = atoi((const char*)key);
                xmlFree(key);
            }

            constructor.addImage(filename,height);

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

    printf("new slide\n");

    constructor.addSlide();
    
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL)
    {
        if ((!xmlStrcmp(cur->name, (const xmlChar *)"title")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            constructor.setSlideTitle((const char*)key);
            printf("    title: %s\n", key);
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"background")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            constructor.setSlideBackground((const char*)key);
            printf("    background: %s\n", key);
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
                                                           const osgDB::ReaderWriter::Options* options)
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
            constructor.setPresentationName((const char*)key);
            printf("name: %s\n", key);
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"bgcolor")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            constructor.setBackgroundColor(mapStringToColor((const char*)key));
            printf("bgcolor: %s\n", key);
            xmlFree(key);
        }
        else if ((!xmlStrcmp(cur->name, (const xmlChar *)"textcolor")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            constructor.setTextColor(mapStringToColor((const char*)key));
            printf("textcolor: %s\n", key);
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

