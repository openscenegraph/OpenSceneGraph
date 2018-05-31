/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include "ReadShowFile.h"
#include "ShowEventHandler.h"

#include <osgPresentation/SlideEventHandler>

#include <osg/ImageStream>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osg/Switch>

#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <osgVolume/VolumeTile>

#include <osgDB/XmlParser>

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
            if (dynamic_cast<osgVolume::PropertyAdjustmentCallback*>(volumeTile->getEventCallback())==0)
            {
                volumeTile->addEventCallback(new osgVolume::PropertyAdjustmentCallback());
            }
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


    osg::ref_ptr<osgDB::XmlNode> doc = new osgDB::XmlNode;
    osgDB::XmlNode* root = 0;

    osgDB::XmlNode::Input input;
    input.open(fileName);
    input.readAllDataIntoBuffer();

    doc->read(input);

    if (doc == NULL )
    {
            fprintf(stderr,"Document not parsed successfully. \n");
            return false;
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
        return false;
    }

    if (root->name!="presentation")
    {
        fprintf(stderr,"document of the wrong type, root node != presentation");
        return false;
    }

    bool readVars = false;

    for(osgDB::XmlNode::Children::iterator itr = root->children.begin();
        itr != root->children.end();
        ++itr)
    {
        osgDB::XmlNode* cur = itr->get();

        if (cur->name=="env")
        {
            char* str = strdup(cur->contents.c_str());
            osg::notify(osg::INFO)<<"putenv("<<str<<")"<<std::endl;
            putenv(str);
            readVars = true;
        }
    }

    return readVars;
}

osgDB::Options* createOptions(const osgDB::ReaderWriter::Options* options)
{
    osg::ref_ptr<osgDB::Options> local_options = options ? options->cloneOptions() : 0;
    if (!local_options)
    {
        local_options = osgDB::Registry::instance()->getOptions() ?
                osgDB::Registry::instance()->getOptions()->cloneOptions() :
                new osgDB::Options;
    }

    local_options->setPluginStringData("P3D_EVENTHANDLER","none");
    return local_options.release();
}

osg::ref_ptr<osg::Node> p3d::readHoldingSlide(const std::string& filename)
{
    std::string ext = osgDB::getFileExtension(filename);
    if (!osgDB::equalCaseInsensitive(ext,"xml") &&
        !osgDB::equalCaseInsensitive(ext,"p3d")) return 0;

    osg::ref_ptr<osgDB::ReaderWriter::Options> options = createOptions(0);
    options->setObjectCacheHint(osgDB::ReaderWriter::Options::CACHE_NONE);
    options->setOptionString("preview");

    return osgDB::readRefNodeFile(filename, options.get());
}

osg::ref_ptr<osg::Node> p3d::readPresentation(const std::string& filename,const osgDB::ReaderWriter::Options* options)
{
    std::string ext = osgDB::getFileExtension(filename);
    if (!osgDB::equalCaseInsensitive(ext,"xml") &&
        !osgDB::equalCaseInsensitive(ext,"p3d")) return 0;

    osg::ref_ptr<osgDB::Options> local_options = createOptions(options);
    local_options->setOptionString("main");

    return osgDB::readRefNodeFile(filename, local_options.get());
}

osg::ref_ptr<osg::Node> p3d::readShowFiles(osg::ArgumentParser& arguments,const osgDB::ReaderWriter::Options* options)
{
    osg::ref_ptr<osgDB::Options> local_options = createOptions(options);
    local_options->setOptionString("main");

    typedef std::vector< osg::ref_ptr<osg::Node> > NodeList;
    NodeList nodeList;

    std::string filename;
    while (arguments.read("--image",filename))
    {
        osg::ref_ptr<osg::Image> image = readRefImageFile(filename.c_str(), local_options.get());
        if (image.valid()) nodeList.push_back(osg::createGeodeForImage(image));
    }

    while (arguments.read("--movie",filename))
    {
        osg::ref_ptr<osg::Image> image = readRefImageFile(filename.c_str(), local_options.get());
        osg::ImageStream* imageStream = dynamic_cast<osg::ImageStream*>(image.get());
        if (imageStream)
        {
            imageStream->play();
            nodeList.push_back(osg::createGeodeForImage(imageStream));
        }
    }

    while (arguments.read("--dem",filename))
    {
        osg::ref_ptr<osg::HeightField> hf = readRefHeightFieldFile(filename.c_str(), local_options.get());
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
            osg::ref_ptr<osg::Node> node = osgDB::readRefNodeFile( arguments[pos], local_options.get());

            if(node)
            {

                if (node->getName().empty()) node->setName( arguments[pos] );
                nodeList.push_back(node);

                // make sure that this presentation isn't cached
                osgDB::Registry::instance()->removeFromObjectCache( arguments[pos], local_options.get());
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

    return root;
}
