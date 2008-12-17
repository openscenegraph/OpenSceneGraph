/* 
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or (at
 * your option) any later version. The full license is in the LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * OpenSceneGraph Public License for more details.
*/

//
// Copyright(c) 2008 Skew Matrix Software LLC.
//

#include "TexturePaletteManager.h"
#include "FltExportVisitor.h"
#include "ExportOptions.h"
#include "DataOutputStream.h"
#include "Opcodes.h"
#include <osg/Notify>
#include <osg/Texture2D>
#include <osgDB/FileNameUtils>
#include <map>


namespace flt
{


TexturePaletteManager::TexturePaletteManager( const FltExportVisitor& nv, const ExportOptions& fltOpt )
  : _currIndex( 0 ),
    _nv( nv ),
    _fltOpt( fltOpt )
{
}

int
TexturePaletteManager::add( int unit, const osg::Texture2D* texture )
{
    if( (!texture) ||
        (!texture->getImage()) )
        return -1;

    int index( -1 );
    TextureIndexMap::const_iterator it = _indexMap.find( texture );
    if (it != _indexMap.end())
        index = it->second;
    else
    {
        index = _currIndex++;
        _indexMap[ texture ] = index;

        // If there is no .attr file, write one
        _nv.writeATTRFile( unit, texture );
    }

    return index;
}

void
TexturePaletteManager::write( DataOutputStream& dos ) const
{
    int x( 0 ), y( 0 ), height( 0 );
    TextureIndexMap::const_iterator it = _indexMap.begin();
    while (it != _indexMap.end())
    {
        const osg::Texture2D* texture = it->first;
        int index = it->second;

        std::string fileName;
        if ( _fltOpt.getStripTextureFilePath() )
            fileName = osgDB::getSimpleFileName( texture->getImage()->getFileName() );
        else
            fileName = texture->getImage()->getFileName();

        dos.writeInt16( (int16) TEXTURE_PALETTE_OP );
        dos.writeUInt16( 216 );
        dos.writeString( fileName, 200 );
        dos.writeInt32( index );
        dos.writeInt32( x );
        dos.writeInt32( y );
        it++;

        x += texture->getImage()->s();
        if (texture->getImage()->t() > height)
            height = texture->getImage()->t();
        if (x > 1024)
        {
            x = 0;
            y += height;
            height = 0;
        }
    }
}


}
