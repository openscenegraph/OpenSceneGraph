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

#ifdef OSG_PROVIDE_READFILE
Object* osgDB::readObjectFile(const std::string& filename,const Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readObject(filename,options);
    if (rr.validObject()) return rr.takeObject();
    if (!rr.success()) OSG_WARN << "Error reading file " << filename << ": " << rr.statusMessage() << std::endl;
    return NULL;
}


Image* osgDB::readImageFile(const std::string& filename,const Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readImage(filename,options);
    if (rr.validImage()) return rr.takeImage();
    if (!rr.success()) OSG_WARN << "Error reading file " << filename << ": " << rr.statusMessage() << std::endl;
    return NULL;
}

Shader* osgDB::readShaderFile(const std::string& filename,const Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readShader(filename,options);
    if (rr.validShader()) return rr.takeShader();
    if (!rr.success()) OSG_WARN << "Error reading file " << filename << ": " << rr.statusMessage() << std::endl;
    return NULL;
}


HeightField* osgDB::readHeightFieldFile(const std::string& filename,const Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readHeightField(filename,options);
    if (rr.validHeightField()) return rr.takeHeightField();
    if (!rr.success()) OSG_WARN << "Error reading file " << filename << ": " << rr.statusMessage() << std::endl;
    return NULL;
}


Node* osgDB::readNodeFile(const std::string& filename,const Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readNode(filename,options);
    if (rr.validNode()) return rr.takeNode();
    if (!rr.success()) OSG_WARN << "Error reading file " << filename << ": " << rr.statusMessage() << std::endl;
    return NULL;
}

Node* osgDB::readNodeFiles(std::vector<std::string>& fileList,const Options* options)
{
    return readRefNodeFiles(fileList, options).release();
}

Node* osgDB::readNodeFiles(osg::ArgumentParser& arguments,const Options* options)
{
    return readRefNodeFiles(arguments, options).release();
}


Script* osgDB::readScriptFile(const std::string& filename,const Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readScript(filename,options);
    if (rr.validScript()) return rr.takeScript();
    if (!rr.success()) OSG_WARN << "Error reading file " << filename << ": " << rr.statusMessage() << std::endl;
    return NULL;
}
#endif

osg::ref_ptr<osg::Object> osgDB::readRefObjectFile(const std::string& filename,const Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readObject(filename,options);
    if (rr.validObject()) return osg::ref_ptr<osg::Object>(rr.getObject());
    if (!rr.success()) OSG_WARN << "Error reading file " << filename << ": " << rr.statusMessage() << std::endl;
    return NULL;
}

osg::ref_ptr<osg::Image> osgDB::readRefImageFile(const std::string& filename,const Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readImage(filename,options);
    if (rr.validImage()) return osg::ref_ptr<osg::Image>(rr.getImage());
    if (!rr.success()) OSG_WARN << "Error reading file " << filename << ": " << rr.statusMessage() << std::endl;
    return NULL;
}

osg::ref_ptr<osg::Shader> osgDB::readRefShaderFile(const std::string& filename,const Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readShader(filename,options);
    if (rr.validShader()) return osg::ref_ptr<osg::Shader>(rr.getShader());
    if (!rr.success()) OSG_WARN << "Error reading file " << filename << ": " << rr.statusMessage() << std::endl;
    return NULL;
}

osg::ref_ptr<osg::HeightField> osgDB::readRefHeightFieldFile(const std::string& filename,const Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readHeightField(filename,options);
    if (rr.validHeightField()) return osg::ref_ptr<osg::HeightField>(rr.getHeightField());
    if (!rr.success()) OSG_WARN << "Error reading file " << filename << ": " << rr.statusMessage() << std::endl;
    return NULL;
}

osg::ref_ptr<osg::Node> osgDB::readRefNodeFile(const std::string& filename,const Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readNode(filename,options);
    if (rr.validNode()) return osg::ref_ptr<osg::Node>(rr.getNode());
    if (!rr.success()) OSG_WARN << "Error reading file " << filename << ": " << rr.statusMessage() << std::endl;
    return NULL;
}

osg::ref_ptr<osg::Script> osgDB::readRefScriptFile(const std::string& filename,const Options* options)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readScript(filename,options);
    if (rr.validScript()) return osg::ref_ptr<osg::Script>(rr.getScript());
    if (!rr.success()) OSG_WARN << "Error reading file " << filename << ": " << rr.statusMessage() << std::endl;
    return NULL;
}

osg::ref_ptr<Node> osgDB::readRefNodeFiles(std::vector<std::string>& fileList,const Options* options)
{
    typedef std::vector< osg::ref_ptr<osg::Node> > NodeList;
    NodeList nodeList;

    for(std::vector<std::string>::iterator itr=fileList.begin();
        itr!=fileList.end();
        ++itr)
    {
        osg::ref_ptr<osg::Node> node = osgDB::readRefNodeFile( *itr , options );

        if( node != (osg::Node *)0L )
        {
            if (node->getName().empty()) node->setName( *itr );
            nodeList.push_back(node);
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
        osg::ref_ptr<osg::Group> group = new osg::Group;
        for(NodeList::iterator itr=nodeList.begin();
            itr!=nodeList.end();
            ++itr)
        {
            group->addChild(*itr);
        }

        return group;
    }

}

osg::ref_ptr<Node> osgDB::readRefNodeFiles(osg::ArgumentParser& arguments,const Options* options)
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
        osg::ref_ptr<osg::Image> image = readRefImageFile(filename.c_str(), options);
        if (image.valid())
        {
            osg::Geode* geode = osg::createGeodeForImage(image.get());

            if (image->isImageTranslucent())
            {
                OSG_INFO<<"Image "<<image->getFileName()<<" is translucent; setting up blending."<<std::endl;
                geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);
                geode->getOrCreateStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            }

            nodeList.push_back(geode);
        }
    }

    while (arguments.read("--movie",filename))
    {
        osg::ref_ptr<osg::Image> image = readRefImageFile(filename.c_str(), options);
        osg::ref_ptr<osg::ImageStream> imageStream = dynamic_cast<osg::ImageStream*>(image.get());
        if (imageStream.valid())
        {
            bool flip = image->getOrigin()==osg::Image::TOP_LEFT;

            // start the stream playing.
            imageStream->play();

            osg::ref_ptr<osg::Geometry> pictureQuad = osg::createTexturedQuadGeometry(osg::Vec3(0.0f,0.0f,0.0f),
                                                                                      osg::Vec3(image->s(),0.0f,0.0f),
                                                                                      osg::Vec3(0.0f,0.0f,image->t()),
                                                                                      0.0f, flip ? 1.0f : 0.0f , 1.0f, flip ? 0.0f : 1.0f);

            pictureQuad->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                        new osg::Texture2D(image.get()),
                        osg::StateAttribute::ON);


            osg::ref_ptr<osg::Geode> geode = new osg::Geode;
            geode->addDrawable(pictureQuad.get());
            nodeList.push_back(geode.get());
        }
        else if (image.valid())
        {
            nodeList.push_back(osg::createGeodeForImage(image.get()));
        }
    }

    while (arguments.read("--dem",filename))
    {
        osg::ref_ptr<osg::HeightField> hf = readRefHeightFieldFile(filename.c_str(), options);
        if (hf)
        {
            osg::ref_ptr<osg::Geode> geode = new osg::Geode;
            geode->addDrawable(new osg::ShapeDrawable(hf.get()));
            nodeList.push_back(geode);
        }
    }

    // note currently doesn't delete the loaded file entries from the command line yet...
    for(int pos=1;pos<arguments.argc();++pos)
    {
        if (!arguments.isOption(pos))
        {
            // not an option so assume string is a filename.
            osg::ref_ptr<osg::Node> node = osgDB::readRefNodeFile( arguments[pos], options);

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
        osg::ref_ptr<osg::Group> group = new osg::Group;
        for(NodeList::iterator itr=nodeList.begin();
            itr!=nodeList.end();
            ++itr)
        {
            group->addChild(*itr);
        }

        return group;
    }

}

osg::ref_ptr<osg::Shader> osgDB::readRefShaderFileWithFallback(osg::Shader::Type type, const std::string& filename, const Options* options, const char* fallback)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readShader(filename,options);
    osg::ref_ptr<osg::Shader> shader = rr.getShader();
    if (!rr.success())
    {
        OSG_INFO << "Error reading file " << filename << ": " << rr.statusMessage() << std::endl;
    }

    if (shader.valid() && type != osg::Shader::UNDEFINED) shader->setType(type);
    if (!shader) shader = new osg::Shader(type, fallback);
    return shader;
}
