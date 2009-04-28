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

#include "ReadShowFile.h"
#include "ShowEventHandler.h"

#include <osg/ImageStream>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/Switch>

#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <osgVolume/VolumeTile>

#include <libxml/xmlmemory.h>
#include <libxml/parser.h>

#include <string.h>

class AddVolumeEditingCallbackVisitor : public osg::NodeVisitor
{
public:
    AddVolumeEditingCallbackVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
        
    void apply(osg::Group& group)
    {
        osgVolume::VolumeTile* volumeTile = dynamic_cast<osgVolume::VolumeTile*>(&group);
        if (volumeTile)
        {
            volumeTile->addEventCallback(new osgVolume::PropertyAdjustmentCallback());
        }
        else
        {
            traverse(group);
        }
    }
    
};

bool p3d::getFileNames(osg::ArgumentParser& arguments, FileNameList& xmlFiles, FileNameList& normalFiles)
{
    // note currently doesn't delete the loaded file entries from the command line yet...
    for(int pos=1;pos<arguments.argc();++pos)
    {
        if (!arguments.isOption(pos))
        {
            std::string ext = osgDB::getFileExtension(arguments[pos]);
            if (osgDB::equalCaseInsensitive(ext,"xml") || osgDB::equalCaseInsensitive(ext,"p3d")) 
            {
                xmlFiles.push_back(arguments[pos]);
            }
            else
            {
                normalFiles.push_back(arguments[pos]);
            }
        }
    }
    return (!xmlFiles.empty() || !normalFiles.empty());
}   

bool p3d::readEnvVars(osg::ArgumentParser& arguments)
{
    bool readVars = false;

    for(int i=1; i<arguments.argc(); ++i)
    {
        if (!arguments.isOption(i))
        {
            std::string ext = osgDB::getLowerCaseFileExtension(arguments[i]);
            if (ext=="xml" || ext=="p3d")
            {
                std::string file = osgDB::findDataFile(arguments[i]);
                if (!file.empty())
                {
                    std::string path = osgDB::getFilePath(file);
                    if (!path.empty())
                    {
                        osgDB::getDataFilePathList().push_front(path);
                    }
                    
                    if (p3d::readEnvVars(file)) readVars = true;
                }
            }
        }
    }
    
    return readVars;
}

bool p3d::readEnvVars(const std::string& fileName)
{
    std::string ext = osgDB::getFileExtension(fileName);
    if (!osgDB::equalCaseInsensitive(ext,"xml") && 
        !osgDB::equalCaseInsensitive(ext,"p3d")) return false;
        
        
    xmlDocPtr doc;
    xmlNodePtr cur;

    doc = xmlParseFile(fileName.c_str());

    if (doc == NULL )
    {
            fprintf(stderr,"Document not parsed successfully. \n");
            return false;
    }

     cur = xmlDocGetRootElement(doc);

     if (cur == NULL)
     {
            fprintf(stderr,"empty document\n");
            xmlFreeDoc(doc);
            return false;
    }

     if (xmlStrcmp(cur->name, (const xmlChar *) "presentation"))
     {
            fprintf(stderr,"document of the wrong type, root node != presentation");
            xmlFreeDoc(doc);
            return false;
    }
    
    bool readVars = false;
    
    xmlChar *key;
    cur = cur->xmlChildrenNode;
    while (cur != NULL) {

        if ((!xmlStrcmp(cur->name, (const xmlChar *)"env")))
        {
            key = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
            if (key)
            {
                char* str = strdup((char*)key);
                osg::notify(osg::INFO)<<"putenv("<<str<<")"<<std::endl;
                putenv(str);
                readVars = true;
            }
            xmlFree(key);
        }
        cur = cur->next;
    }

 #ifndef __APPLE__
    
    xmlFreeDoc(doc);
    
 #endif
    
     return readVars;
}

osg::Node* p3d::readHoldingSlide(const std::string& filename)
{
    std::string ext = osgDB::getFileExtension(filename);
    if (!osgDB::equalCaseInsensitive(ext,"xml") && 
        !osgDB::equalCaseInsensitive(ext,"p3d")) return 0;

    osg::ref_ptr<osgDB::ReaderWriter::Options> options = new osgDB::ReaderWriter::Options;
    options->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_NONE);
    options->setOptionString("holding_slide");

    osgDB::ReaderWriter::ReadResult readResult = osgDB::Registry::instance()->readNode(filename, options.get());
    if (readResult.validNode()) return readResult.takeNode();
    else return 0;
}

osg::Node* p3d::readPresentation(const std::string& filename,const osgDB::ReaderWriter::Options* options)
{
    std::string ext = osgDB::getFileExtension(filename);
    if (!osgDB::equalCaseInsensitive(ext,"xml") &&
        !osgDB::equalCaseInsensitive(ext,"p3d")) return 0;
    return osgDB::readNodeFile(filename, options);
}

osg::Node* p3d::readShowFiles(osg::ArgumentParser& arguments,const osgDB::ReaderWriter::Options* options)
{

    typedef std::vector< osg::ref_ptr<osg::Node> > NodeList;
    NodeList nodeList;

    std::string filename;
    while (arguments.read("--image",filename))
    {
        osg::ref_ptr<osg::Image> image = readImageFile(filename.c_str(), options);
        if (image.valid()) nodeList.push_back(osg::createGeodeForImage(image.get()));
    }

    while (arguments.read("--movie",filename))
    {
        osg::ref_ptr<osg::Image> image = readImageFile(filename.c_str(), options);
        osg::ref_ptr<osg::ImageStream> imageStream = dynamic_cast<osg::ImageStream*>(image.get());
        if (image.valid())
        {
            imageStream->play();
            nodeList.push_back(osg::createGeodeForImage(imageStream.get()));
        }
    }

    while (arguments.read("--dem",filename))
    {
        osg::HeightField* hf = readHeightFieldFile(filename.c_str(), options);
        if (hf)
        {
            osg::Geode* geode = new osg::Geode;
            geode->addDrawable(new osg::ShapeDrawable(hf));
            nodeList.push_back(geode);
        }
    }

    // note currently doesn't delete the loaded file entries from the command line yet...
    for(int pos=1;pos<arguments.argc();++pos)
    {
        if (!arguments.isOption(pos))
        {
            // not an option so assume string is a filename.
            osg::Node *node = osgDB::readNodeFile( arguments[pos], options);

            if(node)
            {
                if (node->getName().empty()) node->setName( arguments[pos] );
                nodeList.push_back(node);
            }

        }
    }
    
    if (nodeList.empty())
    {
        return NULL;
    }
    
    osg::ref_ptr<osg::Node> root;

    if (nodeList.size()==1)
    {
        root = nodeList.front().get();
    }
    else  // size >1
    {
        
        osg::Switch* sw = new osg::Switch;
        for(NodeList::iterator itr=nodeList.begin();
            itr!=nodeList.end();
            ++itr)
        {
            sw->addChild((*itr).get());
        }
        sw->setSingleChildOn(0);
        
        sw->setEventCallback(new p3d::ShowEventHandler());

        root = sw;
    }

    if (root.valid())
    {
        osg::notify(osg::INFO)<<"Got node now adding callback"<<std::endl;
    
        AddVolumeEditingCallbackVisitor avecv;
        root->accept(avecv);
    }

    return root.release();    
}
