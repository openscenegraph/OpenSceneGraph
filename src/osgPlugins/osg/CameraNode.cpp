#include <osg/CameraNode>
#include <osg/io_utils>

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

    writeMatrix(camera.getProjectionMatrix(),fw,"ProjectionMatrix");
    writeMatrix(camera.getViewMatrix(),fw,"ViewMatrix");

    return true;
}
