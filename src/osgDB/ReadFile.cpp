/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
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
#include <osg/Notify>
#include <osg/Object>
#include <osg/Image>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/ShapeDrawable>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

using namespace osg;
using namespace osgDB;

Object* osgDB::readObjectFile(const std::string& filename,bool useObjectCache)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readObject(filename,useObjectCache);
    if (rr.validObject()) return rr.takeObject();
    if (rr.error()) notify(WARN) << rr.message() << std::endl;
    return NULL;
}


Image* osgDB::readImageFile(const std::string& filename,bool useObjectCache)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readImage(filename,useObjectCache);
    if (rr.validImage()) return rr.takeImage();
    if (rr.error()) notify(WARN) << rr.message() << std::endl;
    return NULL;
}


HeightField* osgDB::readHeightFieldFile(const std::string& filename,bool useObjectCache)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readHeightField(filename,useObjectCache);
    if (rr.validHeightField()) return rr.takeHeightField();
    if (rr.error()) notify(WARN) << rr.message() << std::endl;
    return NULL;
}


Node* osgDB::readNodeFile(const std::string& filename,bool useObjectCache)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readNode(filename,useObjectCache);
    if (rr.validNode()) return rr.takeNode();
    if (rr.error()) notify(WARN) << rr.message() << std::endl;
    return NULL;
}

Node* osgDB::readNodeFiles(std::vector<std::string>& commandLine,bool useObjectCache)
{
    typedef std::vector<osg::Node*> NodeList;
    NodeList nodeList;

    // note currently doesn't delete the loaded file entries from the command line yet...

    for(std::vector<std::string>::iterator itr=commandLine.begin();
        itr!=commandLine.end();
        ++itr)
    {
        if ((*itr)[0]!='-')
        {
            // not an option so assume string is a filename.
            osg::Node *node = osgDB::readNodeFile( *itr ,useObjectCache );

            if( node != (osg::Node *)0L )
            {
                if (node->getName().empty()) node->setName( *itr );
                nodeList.push_back(node);
            }

        }
    }
    
    if (nodeList.empty())
    {
        return NULL;
    }

    if (nodeList.size()==1)
    {
        return nodeList.front();
    }
    else  // size >1
    {
        osg::Group* group = new osg::Group;
        for(NodeList::iterator itr=nodeList.begin();
            itr!=nodeList.end();
            ++itr)
        {
            group->addChild(*itr);
        }

        return group;
    }
    
}

Node* osgDB::readNodeFiles(osg::ArgumentParser& arguments,bool useObjectCache)
{

    typedef std::vector<osg::Node*> NodeList;
    NodeList nodeList;

    std::string filename;
    while (arguments.read("--image",filename))
    {
        osg::Image* image = readImageFile(filename.c_str(), useObjectCache);
        if (image) nodeList.push_back(osg::createGeodeForImage(image));
    }

    while (arguments.read("--dem",filename))
    {
        osg::HeightField* hf = readHeightFieldFile(filename.c_str(), useObjectCache);
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
            osg::Node *node = osgDB::readNodeFile( arguments[pos], useObjectCache);

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

    if (nodeList.size()==1)
    {
        return nodeList.front();
    }
    else  // size >1
    {
        osg::Group* group = new osg::Group;
        for(NodeList::iterator itr=nodeList.begin();
            itr!=nodeList.end();
            ++itr)
        {
            group->addChild(*itr);
        }

        return group;
    }
    
}
