#include <osg/Notify>
#include <osg/Object>
#include <osg/Image>
#include <osg/Node>
#include <osg/Group>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

using namespace osg;
using namespace osgDB;

Object* osgDB::readObjectFile(const std::string& filename)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readObject(filename);
    if (rr.validObject()) return rr.takeObject();
    if (rr.error()) notify(WARN) << rr.message() << std::endl;
    return NULL;
}


Image* osgDB::readImageFile(const std::string& filename)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readImage(filename);
    if (rr.validImage()) return rr.takeImage();
    if (rr.error()) notify(WARN) << rr.message() << std::endl;
    return NULL;
}


Node* osgDB::readNodeFile(const std::string& filename)
{
    ReaderWriter::ReadResult rr = Registry::instance()->readNode(filename);
    if (rr.validNode()) return rr.takeNode();
    if (rr.error()) notify(WARN) << rr.message() << std::endl;
    return NULL;
}

Node* osgDB::readNodeFiles(std::vector<std::string>& commandLine)
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
            osg::Node *node = osgDB::readNodeFile( *itr );

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
        osg::Group* group = new osg::Group();
        for(NodeList::iterator itr=nodeList.begin();
            itr!=nodeList.end();
            ++itr)
        {
            group->addChild(*itr);
        }

        return group;
    }
    
}
