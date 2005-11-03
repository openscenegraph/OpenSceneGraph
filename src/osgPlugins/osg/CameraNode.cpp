#include <osg/CameraNode>
#include <osg/io_utils>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include "Matrix.h"

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool CameraNode_readLocalData(Object& obj, Input& fr);
bool CameraNode_writeLocalData(const Object& obj, Output& fw);

// register the read and write functions with the osgDB::Registry.
RegisterDotOsgWrapperProxy g_CameraNodeProxy
(
    new osg::CameraNode,
    "CameraNode",
    "Object Node Transform CameraNode Group",
    &CameraNode_readLocalData,
    &CameraNode_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool CameraNode_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    CameraNode& camera = static_cast<CameraNode&>(obj);

    if (fr.matchSequence("clearColor %f %f %f %f"))
    {
        Vec4 color;
        fr[1].getFloat(color[0]);
        fr[2].getFloat(color[1]);
        fr[3].getFloat(color[2]);
        fr[4].getFloat(color[3]);
        camera.setClearColor(color);
        fr +=5 ;
        iteratorAdvanced = true;
    };
    
    if (fr.matchSequence("clearMask %i"))
    {
        unsigned int value;
        fr[1].getUInt(value);
        camera.setClearMask(value);
        fr += 2;
        iteratorAdvanced = true;
    }

    osg::ref_ptr<osg::StateAttribute> attribute;
    while((attribute=fr.readStateAttribute())!=NULL)
    {
        osg::Viewport* viewport = dynamic_cast<osg::Viewport*>(attribute.get());
        if (viewport) camera.setViewport(viewport);
        else
        {
            osg::ColorMask* colormask = dynamic_cast<osg::ColorMask*>(attribute.get());
            camera.setColorMask(colormask);
        }
    }

    if (fr.matchSequence("transformOrder %w"))
    {
        if      (fr[1].matchWord("PRE_MULTIPLE")) camera.setTransformOrder(osg::CameraNode::PRE_MULTIPLE);
        else if (fr[1].matchWord("POST_MULTIPLE")) camera.setTransformOrder(osg::CameraNode::POST_MULTIPLE);

        fr += 2;
        iteratorAdvanced = true;
    }

    Matrix matrix; 
    if (readMatrix(matrix,fr,"ProjectionMatrix"))
    {
        camera.setProjectionMatrix(matrix);
        iteratorAdvanced = true;
    }

    if (readMatrix(matrix,fr,"ViewMatrix"))
    {
        camera.setViewMatrix(matrix);
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("renderOrder %w"))
    {
        if      (fr[1].matchWord("PRE_RENDER")) camera.setRenderOrder(osg::CameraNode::PRE_RENDER);
        else if (fr[1].matchWord("NESTED_RENDER")) camera.setRenderOrder(osg::CameraNode::NESTED_RENDER);
        else if (fr[1].matchWord("POST_RENDER")) camera.setRenderOrder(osg::CameraNode::POST_RENDER);

        fr += 2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool CameraNode_writeLocalData(const Object& obj, Output& fw)
{
    const CameraNode& camera = static_cast<const CameraNode&>(obj);

    fw.indent()<<"clearColor "<<camera.getClearColor()<<std::endl;
    fw.indent()<<"clearMask 0x"<<std::hex<<camera.getClearMask()<<std::endl;

    if (camera.getColorMask())
    {
        fw.writeObject(*camera.getColorMask());
    }

    if (camera.getViewport())
    {
        fw.writeObject(*camera.getViewport());
    }

    fw.indent()<<"transformOrder ";
    switch(camera.getTransformOrder())
    {
        case(osg::CameraNode::PRE_MULTIPLE): fw <<"PRE_MULTIPLE"<<std::endl; break;
        case(osg::CameraNode::POST_MULTIPLE): fw <<"POST_MULTIPLE"<<std::endl; break;
    }

    writeMatrix(camera.getProjectionMatrix(),fw,"ProjectionMatrix");
    writeMatrix(camera.getViewMatrix(),fw,"ViewMatrix");

    fw.indent()<<"renderOrder ";
    switch(camera.getRenderOrder())
    {
        case(osg::CameraNode::PRE_RENDER): fw <<"PRE_RENDER"<<std::endl; break;
        case(osg::CameraNode::NESTED_RENDER): fw <<"NESTED_RENDER"<<std::endl; break;
        case(osg::CameraNode::POST_RENDER): fw <<"POST_RENDER"<<std::endl; break;
    }

    return true;
}
