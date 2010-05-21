/**********************************************************************
 *
 *    FILE:           Texture2DArray.cpp
 *
 *    DESCRIPTION:    Read/Write osg::Texture2DArray in binary format to disk.
 *
 *    CREATED BY:     Wojtek Lewandowski based on TextureCubeMap ive support
 *
 *    HISTORY:        Created 12.02.2010
 *
 **********************************************************************/

#include "Exception.h"
#include "Texture2DArray.h"
#include "Texture.h"
#include "Image.h"

using namespace ive;

void Texture2DArray::write(DataOutputStream* out){
    // Write Texture2DArray's identification.
    out->writeInt(IVETEXTURE2DARRAY);
    // If the osg class is inherited by any other class we should also write this to file.
    osg::Texture*  tex = dynamic_cast<osg::Texture*>(this);
    if(tex){
        ((ive::Texture*)(tex))->write(out);
    }
    else
        out_THROW_EXCEPTION("Texture2DArray::write(): Could not cast this osg::Texture2DArray to an osg::Texture.");
    // Write Texture2DArray's properties.

    // Write texture size
    out->writeInt(getTextureWidth());
    out->writeInt(getTextureHeight());
    out->writeInt(getTextureDepth());

    // Write number of mipmap levels
    out->writeInt(getNumMipmapLevels());

    for( int i = 0; i < getTextureDepth(); i++ )
    {
        out->writeImage( getImage( i ) );
    }
}

void Texture2DArray::read(DataInputStream* in)
{
    // Peek on Texture2DArray's identification.
    int id = in->peekInt();
    if(id == IVETEXTURE2DARRAY){
        // Read Texture2DArray's identification.
        id = in->readInt();
        // If the osg class is inherited by any other class we should also read this from file.
        osg::Texture* tex = dynamic_cast<osg::Texture*>(this);
        if(tex){
            ((ive::Texture*)(tex))->read(in);
        }
        else
            in_THROW_EXCEPTION("Texture2DArray::read(): Could not cast this osg::Texture2DArray to an osg::Texture.");
        // Read Texture2DArray's properties

        // Read texture size
        int width = in->readInt();
        int height = in->readInt();
        int depth = in->readInt();
        setTextureSize(width, height, depth);

        // Read number of mipmap levels
        setNumMipmapLevels((unsigned int)in->readInt());

        for( int i = 0; i < depth; i++ )
        {
            setImage( i, in->readImage() );
        }
    }
    else{
        in_THROW_EXCEPTION("Texture2DArray::read(): Expected Texture2DArray identification.");
    }
}

