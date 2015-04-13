#include <osg/Camera>
#include <osg/io_utils>
#include <osg/Notify>

#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

#include "Matrix.h"

#include <string.h>

using namespace osg;
using namespace osgDB;

// forward declare functions to use later.
bool Camera_readLocalData(Object& obj, Input& fr);
bool Camera_writeLocalData(const Object& obj, Output& fw);

bool Camera_matchBufferComponentStr(const char* str,Camera::BufferComponent& buffer);
const char* Camera_getBufferComponentStr(Camera::BufferComponent buffer);


// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(Camera)
(
    new osg::Camera,
    "Camera",
    "Object Node Transform Camera Group",
    &Camera_readLocalData,
    &Camera_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

// register the read and write functions with the osgDB::Registry.
REGISTER_DOTOSGWRAPPER(CameraNode)
(
    new osg::Camera,
    "CameraNode",
    "Object Node Transform CameraNode Group",
    &Camera_readLocalData,
    &Camera_writeLocalData,
    DotOsgWrapper::READ_AND_WRITE
);

bool Camera_readLocalData(Object& obj, Input& fr)
{
    bool iteratorAdvanced = false;

    Camera& camera = static_cast<Camera&>(obj);

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
        if      (fr[1].matchWord("PRE_MULTIPLY")) camera.setTransformOrder(osg::Camera::PRE_MULTIPLY);
        else if (fr[1].matchWord("POST_MULTIPLY")) camera.setTransformOrder(osg::Camera::POST_MULTIPLY);
        // the following are for backwards compatibility.
        else if (fr[1].matchWord("PRE_MULTIPLE")) camera.setTransformOrder(osg::Camera::PRE_MULTIPLY);
        else if (fr[1].matchWord("POST_MULTIPLE")) camera.setTransformOrder(osg::Camera::POST_MULTIPLY);

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
        if      (fr[1].matchWord("PRE_RENDER")) camera.setRenderOrder(osg::Camera::PRE_RENDER);
        else if (fr[1].matchWord("NESTED_RENDER")) camera.setRenderOrder(osg::Camera::NESTED_RENDER);
        else if (fr[1].matchWord("POST_RENDER")) camera.setRenderOrder(osg::Camera::POST_RENDER);

        fr += 2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("renderTargetImplementation %w"))
    {
        osg::Camera::RenderTargetImplementation implementation = osg::Camera::FRAME_BUFFER;

        if      (fr[1].matchWord("FRAME_BUFFER_OBJECT")) implementation = osg::Camera::FRAME_BUFFER_OBJECT;
        else if (fr[1].matchWord("PIXEL_BUFFER_RTT")) implementation = osg::Camera::PIXEL_BUFFER_RTT;
        else if (fr[1].matchWord("PIXEL_BUFFER")) implementation = osg::Camera::PIXEL_BUFFER;
        else if (fr[1].matchWord("FRAME_BUFFER")) implementation = osg::Camera::FRAME_BUFFER;
        else if (fr[1].matchWord("SEPARATE_WINDOW")) implementation = osg::Camera::SEPARATE_WINDOW;

        camera.setRenderTargetImplementation(implementation);

        fr += 2;
        iteratorAdvanced = true;
    }

    if (fr.matchSequence("renderTargetImplementation %w"))
    {
        osg::Camera::RenderTargetImplementation fallback = camera.getRenderTargetFallback();

        if      (fr[1].matchWord("FRAME_BUFFER_OBJECT")) fallback = osg::Camera::FRAME_BUFFER_OBJECT;
        else if (fr[1].matchWord("PIXEL_BUFFER_RTT")) fallback = osg::Camera::PIXEL_BUFFER_RTT;
        else if (fr[1].matchWord("PIXEL_BUFFER")) fallback = osg::Camera::PIXEL_BUFFER;
        else if (fr[1].matchWord("FRAME_BUFFER")) fallback = osg::Camera::FRAME_BUFFER;
        else if (fr[1].matchWord("SEPARATE_WINDOW")) fallback = osg::Camera::SEPARATE_WINDOW;

        camera.setRenderTargetImplementation(camera.getRenderTargetImplementation(), fallback);

        fr += 2;
        iteratorAdvanced = true;
    }


    if (fr.matchSequence("bufferComponent %w {"))
    {
        int entry = fr[1].getNoNestedBrackets();

        Camera::BufferComponent buffer;
        Camera_matchBufferComponentStr(fr[1].getStr(),buffer);

        fr += 3;

        Camera::Attachment& attachment = camera.getBufferAttachmentMap()[buffer];

        // read attachment data.
        while (!fr.eof() && fr[0].getNoNestedBrackets()>entry)
        {
            bool localAdvance = false;

            if (fr.matchSequence("internalFormat %i"))
            {
                // In their infinite wisdom, the Apple engineers changed the type
                // of GLenum from 'unsigned int' to 'unsigned long', thus breaking
                // the call by reference of getUInt.
                unsigned int format;
                fr[1].getUInt(format);
                attachment._internalFormat = format;
                fr += 2;
                localAdvance = true;
            }

            osg::ref_ptr<osg::Object> attribute;
            while((attribute=fr.readObject())!=NULL)
            {
                localAdvance = true;

                osg::Texture* texture = dynamic_cast<osg::Texture*>(attribute.get());
                if (texture) attachment._texture = texture;
                else
                {
                    osg::Image* image = dynamic_cast<osg::Image*>(attribute.get());
                    attachment._image = image;
                }

            }

            if (fr.matchSequence("level %i"))
            {
                fr[1].getUInt(attachment._level);
                fr += 2;
                localAdvance = true;
            }

            if (fr.matchSequence("face %i"))
            {
                fr[1].getUInt(attachment._face);
                fr += 2;
                localAdvance = true;
            }

            if (fr.matchSequence("mipMapGeneration TRUE"))
            {
                attachment._mipMapGeneration = true;
                fr += 2;
                localAdvance = true;
            }

            if (fr.matchSequence("mipMapGeneration FALSE"))
            {
                attachment._mipMapGeneration = false;
                fr += 2;
                localAdvance = true;
            }

            if (!localAdvance) ++fr;
        }

        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool Camera_writeLocalData(const Object& obj, Output& fw)
{
    const Camera& camera = static_cast<const Camera&>(obj);

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
        case(osg::Camera::PRE_MULTIPLY): fw <<"PRE_MULTIPLY"<<std::endl; break;
        case(osg::Camera::POST_MULTIPLY): fw <<"POST_MULTIPLY"<<std::endl; break;
    }

    writeMatrix(camera.getProjectionMatrix(),fw,"ProjectionMatrix");
    writeMatrix(camera.getViewMatrix(),fw,"ViewMatrix");

    fw.indent()<<"renderOrder ";
    switch(camera.getRenderOrder())
    {
        case(osg::Camera::PRE_RENDER): fw <<"PRE_RENDER"<<std::endl; break;
        case(osg::Camera::NESTED_RENDER): fw <<"NESTED_RENDER"<<std::endl; break;
        case(osg::Camera::POST_RENDER): fw <<"POST_RENDER"<<std::endl; break;
    }

    fw.indent()<<"renderTargetImplementation ";
    switch(camera.getRenderTargetImplementation())
    {
        case(osg::Camera::FRAME_BUFFER_OBJECT): fw <<"FRAME_BUFFER_OBJECT"<<std::endl; break;
        case(osg::Camera::PIXEL_BUFFER_RTT): fw <<"PIXEL_BUFFER_RTT"<<std::endl; break;
        case(osg::Camera::PIXEL_BUFFER): fw <<"PIXEL_BUFFER"<<std::endl; break;
        case(osg::Camera::FRAME_BUFFER): fw <<"FRAME_BUFFER"<<std::endl; break;
        case(osg::Camera::SEPARATE_WINDOW): fw <<"SEPARATE_WINDOW"<<std::endl; break;
    }

    fw.indent()<<"renderTargetFallback ";
    switch(camera.getRenderTargetFallback())
    {
        case(osg::Camera::FRAME_BUFFER_OBJECT): fw <<"FRAME_BUFFER_OBJECT"<<std::endl; break;
        case(osg::Camera::PIXEL_BUFFER_RTT): fw <<"PIXEL_BUFFER_RTT"<<std::endl; break;
        case(osg::Camera::PIXEL_BUFFER): fw <<"PIXEL_BUFFER"<<std::endl; break;
        case(osg::Camera::FRAME_BUFFER): fw <<"FRAME_BUFFER"<<std::endl; break;
        case(osg::Camera::SEPARATE_WINDOW): fw <<"SEPARATE_WINDOW"<<std::endl; break;
    }

    fw.indent()<<"drawBuffer "<<std::hex<<camera.getDrawBuffer()<<std::endl;
    fw.indent()<<"readBuffer "<<std::hex<<camera.getReadBuffer()<<std::endl;

    const osg::Camera::BufferAttachmentMap& bam = camera.getBufferAttachmentMap();
    if (!bam.empty())
    {
        for(osg::Camera::BufferAttachmentMap::const_iterator itr=bam.begin();
            itr!=bam.end();
            ++itr)
        {
            const osg::Camera::Attachment& attachment = itr->second;
            fw.indent()<<"bufferComponent "<<Camera_getBufferComponentStr(itr->first)<<" {"<<std::endl;
            fw.moveIn();

            fw.indent()<<"internalFormat "<<attachment._internalFormat<<std::endl;
            if (attachment._texture.valid())
            {
                fw.writeObject(*attachment._texture.get());
            }
            fw.indent()<<"level "<<attachment._level<<std::endl;
            fw.indent()<<"face "<<attachment._face<<std::endl;
            fw.indent()<<"mipMapGeneration "<<(attachment._mipMapGeneration ? "TRUE" : "FALSE")<<std::endl;

            fw.moveOut();
            fw.indent()<<"}"<<std::endl;
        }
    }

    return true;
}

bool Camera_matchBufferComponentStr(const char* str,Camera::BufferComponent& buffer)
{
    if      (strcmp(str,"DEPTH_BUFFER")==0)                 buffer = osg::Camera::DEPTH_BUFFER;
    else if (strcmp(str,"STENCIL_BUFFER")==0)               buffer = osg::Camera::STENCIL_BUFFER;
    else if (strcmp(str,"PACKED_DEPTH_STENCIL_BUFFER")==0)  buffer = osg::Camera::PACKED_DEPTH_STENCIL_BUFFER;
    else if (strcmp(str,"COLOR_BUFFER")==0)                 buffer = osg::Camera::COLOR_BUFFER;
    else if (strcmp(str,"COLOR_BUFFER0")==0)                buffer = osg::Camera::COLOR_BUFFER0;
    else if (strcmp(str,"COLOR_BUFFER1")==0)                buffer = osg::Camera::COLOR_BUFFER1;
    else if (strcmp(str,"COLOR_BUFFER2")==0)                buffer = osg::Camera::COLOR_BUFFER2;
    else if (strcmp(str,"COLOR_BUFFER3")==0)                buffer = osg::Camera::COLOR_BUFFER3;
    else if (strcmp(str,"COLOR_BUFFER4")==0)                buffer = osg::Camera::COLOR_BUFFER4;
    else if (strcmp(str,"COLOR_BUFFER5")==0)                buffer = osg::Camera::COLOR_BUFFER5;
    else if (strcmp(str,"COLOR_BUFFER6")==0)                buffer = osg::Camera::COLOR_BUFFER6;
    else if (strcmp(str,"COLOR_BUFFER7")==0)                buffer = osg::Camera::COLOR_BUFFER7;
    else if (strcmp(str,"COLOR_BUFFER8")==0)                buffer = osg::Camera::COLOR_BUFFER8;
    else if (strcmp(str,"COLOR_BUFFER9")==0)                buffer = osg::Camera::COLOR_BUFFER9;
    else if (strcmp(str,"COLOR_BUFFER10")==0)               buffer = osg::Camera::COLOR_BUFFER10;
    else if (strcmp(str,"COLOR_BUFFER11")==0)               buffer = osg::Camera::COLOR_BUFFER11;
    else if (strcmp(str,"COLOR_BUFFER12")==0)               buffer = osg::Camera::COLOR_BUFFER12;
    else if (strcmp(str,"COLOR_BUFFER13")==0)               buffer = osg::Camera::COLOR_BUFFER13;
    else if (strcmp(str,"COLOR_BUFFER14")==0)               buffer = osg::Camera::COLOR_BUFFER14;
    else if (strcmp(str,"COLOR_BUFFER15")==0)               buffer = osg::Camera::COLOR_BUFFER15;
    else return false;
    return true;
}
const char* Camera_getBufferComponentStr(Camera::BufferComponent buffer)
{
    switch(buffer)
    {
        case (osg::Camera::DEPTH_BUFFER)                : return "DEPTH_BUFFER";
        case (osg::Camera::STENCIL_BUFFER)              : return "STENCIL_BUFFER";
        case (osg::Camera::PACKED_DEPTH_STENCIL_BUFFER) : return "PACKED_DEPTH_STENCIL_BUFFER";
        case (osg::Camera::COLOR_BUFFER)                : return "COLOR_BUFFER";
        case (osg::Camera::COLOR_BUFFER0)               : return "COLOR_BUFFER0";
        case (osg::Camera::COLOR_BUFFER1)               : return "COLOR_BUFFER1";
        case (osg::Camera::COLOR_BUFFER2)               : return "COLOR_BUFFER2";
        case (osg::Camera::COLOR_BUFFER3)               : return "COLOR_BUFFER3";
        case (osg::Camera::COLOR_BUFFER4)               : return "COLOR_BUFFER4";
        case (osg::Camera::COLOR_BUFFER5)               : return "COLOR_BUFFER5";
        case (osg::Camera::COLOR_BUFFER6)               : return "COLOR_BUFFER6";
        case (osg::Camera::COLOR_BUFFER7)               : return "COLOR_BUFFER7";
        case (osg::Camera::COLOR_BUFFER8)               : return "COLOR_BUFFER8";
        case (osg::Camera::COLOR_BUFFER9)               : return "COLOR_BUFFER9";
        case (osg::Camera::COLOR_BUFFER10)              : return "COLOR_BUFFER10";
        case (osg::Camera::COLOR_BUFFER11)              : return "COLOR_BUFFER11";
        case (osg::Camera::COLOR_BUFFER12)              : return "COLOR_BUFFER12";
        case (osg::Camera::COLOR_BUFFER13)              : return "COLOR_BUFFER13";
        case (osg::Camera::COLOR_BUFFER14)              : return "COLOR_BUFFER14";
        case (osg::Camera::COLOR_BUFFER15)              : return "COLOR_BUFFER15";
        default                                         : return "UnknownBufferComponent";
    }
}

