#include "pfb.h"

// Open Scene Graph includes.
#include <osg/OSG>
#include <osg/Node>
#include <osg/Geode>
#include <osg/Group>

#include <osg/Output>
#include <osg/Input>
#include <osg/Registry>

// Performer includes.
#include <Performer/pfdu.h>

#include "ConvertFromPerformer.h"
#include "ConvertToPerformer.h"

using namespace osg;

// now register with Registry to instantiate the pfb reader/writer.
RegisterReaderWriterProxy<PerformerReaderWriter> g_performerReaderWriterProxy;

PerformerReaderWriter::PerformerReaderWriter()
{
    _perfomerInitialised = false;
}

PerformerReaderWriter::~PerformerReaderWriter()
{
    if (_perfomerInitialised)
    {
        pfExit(); 
    }
}

void PerformerReaderWriter::initialisePerformer(const std::string& fileName)
{
    if (_perfomerInitialised) return;

    pfInit();
    pfdInitConverter(fileName.c_str());
    pfConfig();

    _perfomerInitialised = true;
}

osg::Object* PerformerReaderWriter::readObject(const std::string& fileName)
{
    return readNode(fileName);
}

osg::Node* PerformerReaderWriter::readNode(const std::string& fileName)
{
    initialisePerformer(fileName.c_str());

    osg::Node* osg_root = NULL;
    pfNode* pf_root = pfdLoadFile(fileName.c_str());
    if (pf_root)
    {
        ConvertFromPerformer converter;
        osg_root = converter.convert(pf_root);
    }
    return osg_root;
}

bool PerformerReaderWriter::writeObject(osg::Object& obj,const std::string& fileName)
{
    osg::Node* node = dynamic_cast<osg::Node*>(&obj);
    if (node) return writeNode(*node,fileName);
    return false;
}

bool PerformerReaderWriter::writeNode(osg::Node& node,const std::string& fileName)
{
    initialisePerformer(fileName.c_str());

    ConvertToPerformer converter;
    pfNode* pf_root = converter.convert(&node);

    if (pf_root) {
        pfdStoreFile(pf_root,fileName.c_str());
    }

    return false;
}
