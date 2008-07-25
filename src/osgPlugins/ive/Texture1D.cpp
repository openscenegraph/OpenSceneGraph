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
    out->writeImage(getImage());
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
        osg::Image *image = in->readImage();
        if(image) {
            setImage(image);
        }
    }
    else{
        throw Exception("Texture1D::read(): Expected Texture1D identification.");
    }
}
