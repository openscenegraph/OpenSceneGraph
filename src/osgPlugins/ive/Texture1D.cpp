/**********************************************************************
 *
 *    FILE:            Texture2D.cpp
 *
 *    DESCRIPTION:    Read/Write osg::Texture2D in binary format to disk.
 *
 *    CREATED BY:     Copied from Texture 2D.cpp and edited for Texture 1D
 *                     by Don Burns
 *                    
 *    HISTORY:        Created 27.1.2004
 *
 *    Copyright 2003 VR-C, OSGPL
 **********************************************************************/

#include "Exception.h"
#include "Texture1D.h"
#include "Texture.h"
#include "Image.h"

using namespace ive;

void Texture1D::write(DataOutputStream* out){
    // Write Texture1D's identification.
    out->writeInt(IVETEXTURE1D);
    // If the osg class is inherited by any other class we should also write this to file.
    osg::Texture*  tex = dynamic_cast<osg::Texture*>(this);
    if(tex){
        ((ive::Texture*)(tex))->write(out);
    }
    else
        throw Exception("Texture1D::write(): Could not cast this osg::Texture1D to an osg::Texture.");
    // Write Texture1D's properties.
    // Write image.

    // Should we include images date in stream
    bool includeImg = out->getIncludeImageData();
    out->writeBool(includeImg);

    // Include image data in stream
    if(includeImg){
        out->writeBool(getImage()!=0);
        if(getImage())
            ((ive::Image*)getImage())->write(out);
    }
    // Only include image name in stream
    else{
        if (getImage() && !(getImage()->getFileName().empty())){
            out->writeString(getImage()->getFileName());
        }
        else{ 
            out->writeString("");
        }    
    }
}

void Texture1D::read(DataInputStream* in){
    // Read Texture1D's identification.
    int id = in->peekInt();
    if(id == IVETEXTURE1D){
        // Code to read Texture1D's properties.
        id = in->readInt();
        // If the osg class is inherited by any other class we should also read this from file.
        osg::Texture*  tex = dynamic_cast<osg::Texture*>(this);
        if(tex){
            ((ive::Texture*)(tex))->read(in);
        }
        else
            throw Exception("Texture1D::read(): Could not cast this osg::Texture1D to an osg::Texture.");
        // Read image.
        
        // Should we read image data from stream
        bool includeImg = in->readBool();

        // Read image data from stream
        if(includeImg)
        {
            if(in->readBool())
            {
                osg::Image* image = new osg::Image();
                ((ive::Image*)image)->read(in);
                setImage(image);
            }
        }
        // Only read image name from stream.
        else{
            std::string filename = in->readString();
            if(filename.compare("")!=0){
                osg::Image* image = in->readImage(filename);
                if (image){
                    setImage(image);
                }
            }
        }
    }
    else{
        throw Exception("Texture1D::read(): Expected Texture1D identification.");
    }
}
