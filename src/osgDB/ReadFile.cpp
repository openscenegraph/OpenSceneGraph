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
#include <osg/Notify>
#include <osg/Object>
#include <osg/Image>
#include <osg/Shader>
#include <osg/ImageStream>
#include <osg/Node>
#include <osg/Group>
#include <osg/Geode>
#include <osg/ShapeDrawable>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TextureRectangle>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

using namespace osg;
using namespace osgDB;

Object* osgDB::readObjectFile(const std::string& filename,const ReaderWriter::Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readObject(filename,options);
    if (rr.validObject()) return rr.takeObject();
    if (rr.error()) notify(WARN) << rr.message() << std::endl;
    return NULL;
}


Image* osgDB::readImageFile(const std::string& filename,const ReaderWriter::Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readImage(filename,options);
    if (rr.validImage()) return rr.takeImage();
    if (rr.error()) notify(WARN) << rr.message() << std::endl;
    return NULL;
}

Shader* osgDB::readShaderFile(const std::string& filename,const ReaderWriter::Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readShader(filename,options);
    if (rr.validShader()) return rr.takeShader();
    if (rr.error()) notify(WARN) << rr.message() << std::endl;
    return NULL;
}


HeightField* osgDB::readHeightFieldFile(const std::string& filename,const ReaderWriter::Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readHeightField(filename,options);
    if (rr.validHeightField()) return rr.takeHeightField();
    if (rr.error()) notify(WARN) << rr.message() << std::endl;
    return NULL;
}


Node* osgDB::readNodeFile(const std::string& filename,const ReaderWriter::Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readNode(filename,options);
    if (rr.validNode()) return rr.takeNode();
    if (rr.error()) notify(WARN) << rr.message() << std::endl;
    return NULL;
}

Node* osgDB::readNodeFiles(std::vector<std::string>& commandLine,const ReaderWriter::Options* options)
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
            osg::Node *node = osgDB::readNodeFile( *itr , options );

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

Node* osgDB::readNodeFiles(osg::ArgumentParser& arguments,const ReaderWriter::Options* options)
{

    typedef std::vector< osg::ref_ptr<osg::Node> > NodeList;
    NodeList nodeList;

    std::string filename;

    while (arguments.read("--file-cache",filename))
    {
        osgDB::Registry::instance()->setFileCache(new osgDB::FileCache(filename));
    }
    
    while (arguments.read("--image",filename))
    {
        osg::ref_ptr<osg::Image> image = readImageFile(filename.c_str(), options);
        if (image.valid())
        {
            osg::Geode* geode = osg::createGeodeForImage(image.get());
           
            if (image->isImageTranslucent())
            {
                osg::notify()<<"Image "<<image->getFileName()<<" is translucent; setting up blending."<<std::endl;
                geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
                geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            }

            nodeList.push_back(geode);
        }
    }

    while (arguments.read("--movie",filename))
    {
        osg::ref_ptr<osg::Image> image = readImageFile(filename.c_str(), options);
        osg::ref_ptr<osg::ImageStream> imageStream = dynamic_cast<osg::ImageStream*>(image.get());
        if (imageStream.valid())
        {
            // start the stream playing.
            imageStream->play();

            osg::ref_ptr<osg::Geometry> pictureQuad = 0;

            bool useTextureRectangle = true;
            if (useTextureRectangle)
            {
                pictureQuad = osg::createTexturedQuadGeometry(osg::Vec3(0.0f,0.0f,0.0f),
                                                   osg::Vec3(image->s(),0.0f,0.0f),
                                                   osg::Vec3(0.0f,0.0f,image->t()),
                                                   0.0f,image->t(), image->s(),0.0f);

                pictureQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                            new osg::TextureRectangle(image.get()),
                            osg::StateAttribute::ON);
            }
            else
            {
                pictureQuad = osg::createTexturedQuadGeometry(osg::Vec3(0.0f,0.0f,0.0f),
                                                   osg::Vec3(image->s(),0.0f,0.0f),
                                                   osg::Vec3(0.0f,0.0f,image->t()),
                                                   0.0f,0.0f, 1.0f,1.0f);

                pictureQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                            new osg::Texture2D(image.get()),
                            osg::StateAttribute::ON);
            }

            if (pictureQuad.valid())
            {
                osg::ref_ptr<osg::Geode> geode = new osg::Geode;
                geode->addDrawable(pictureQuad.get());
                nodeList.push_back(geode.get());

            }
        }
        else if (image.valid())
        {
            nodeList.push_back(osg::createGeodeForImage(image.get()));
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

    if (nodeList.size()==1)
    {
        return nodeList.front().release();
    }
    else  // size >1
    {
        osg::Group* group = new osg::Group;
        for(NodeList::iterator itr=nodeList.begin();
            itr!=nodeList.end();
            ++itr)
        {
            group->addChild((*itr).get());
        }

        return group;
    }
    
}

osg::ref_ptr<osg::Object> osgDB::readRefObjectFile(const std::string& filename,const ReaderWriter::Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readObject(filename,options);
    if (rr.validObject()) return osg::ref_ptr<osg::Object>(rr.getObject());
    if (rr.error()) notify(WARN) << rr.message() << std::endl;
    return NULL;
}

osg::ref_ptr<osg::Image> osgDB::readRefImageFile(const std::string& filename,const ReaderWriter::Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readImage(filename,options);
    if (rr.validImage()) return osg::ref_ptr<osg::Image>(rr.getImage());
    if (rr.error()) notify(WARN) << rr.message() << std::endl;
    return NULL;
}

osg::ref_ptr<osg::Shader> osgDB::readRefShaderFile(const std::string& filename,const ReaderWriter::Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readShader(filename,options);
    if (rr.validShader()) return osg::ref_ptr<osg::Shader>(rr.getShader());
    if (rr.error()) notify(WARN) << rr.message() << std::endl;
    return NULL;
}

osg::ref_ptr<osg::HeightField> osgDB::readRefHeightFieldFile(const std::string& filename,const ReaderWriter::Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readHeightField(filename,options);
    if (rr.validHeightField()) return osg::ref_ptr<osg::HeightField>(rr.getHeightField());
    if (rr.error()) notify(WARN) << rr.message() << std::endl;
    return NULL;
}

osg::ref_ptr<osg::Node> osgDB::readRefNodeFile(const std::string& filename,const ReaderWriter::Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readNode(filename,options);
    if (rr.validNode()) return osg::ref_ptr<osg::Node>(rr.getNode());
    if (rr.error()) notify(WARN) << rr.message() << std::endl;
    return NULL;
}
