#include <stdio.h>
#include <string.h>
#include <osg/GeoSet>
#include "AreaGeoSetTriangulator.h"

namespace dx {

class AreaGeoSetTriangulator
  // Used to convert area primitive GeoSets into TRIANGLE Geosets.
  //   Efficiently collects the various data arrays, which GeoSet can't do.
{
protected:
  std::vector< osg::uint > _cindex;
  std::vector< osg::uint > _nindex;
  std::vector< osg::uint > _colindex;
  std::vector< osg::uint > _tindex;
  const osg::GeoSet       &_area_geoset;

public:
  AreaGeoSetTriangulator( const osg::GeoSet &area_geoset );

  void operator() ( osg::uint v1, osg::uint v2, osg::uint v3,
                    osg::uint prim_index, osg::uint prim_tri_index,
                    osg::uint v1_geoset, osg::uint v2_geoset, 
                    osg::uint v3_geoset );
    // Passed triangles as primitives are triangulated

  osg::GeoSet *BuildGeoSet();
};

//---------------------------------------------------------------------------
AreaGeoSetTriangulator::AreaGeoSetTriangulator( const osg::GeoSet &area_geoset)
    : _area_geoset(area_geoset)
{ 
  // Check type
  switch ( area_geoset.getPrimType() ) {
   case osg::GeoSet::TRIANGLE_STRIP:
   case osg::GeoSet::FLAT_TRIANGLE_STRIP:
   case osg::GeoSet::TRIANGLES:
   case osg::GeoSet::QUAD_STRIP:
   case osg::GeoSet::QUADS:
   case osg::GeoSet::TRIANGLE_FAN:
   case osg::GeoSet::FLAT_TRIANGLE_FAN:
   case osg::GeoSet::POLYGON:
     break;
   default:
     fprintf( stderr, 
              "Invalid primitive type passed to AreaGeoSetTriangulator\n" );
     exit(1);
  }

  // NOTE:  _coords, _normals, _colors, and _tcoords are maintained
  //        from the original geoset.  The new geoset will always have
  //        indexed coords/normals/colors/tcoords (to save space).
}

//---------------------------------------------------------------------------
void AreaGeoSetTriangulator::operator() ( 
                     osg::uint v1, osg::uint v2, osg::uint v3,
                     osg::uint prim_index, osg::uint prim_tri_index,
                     osg::uint v1_geoset, osg::uint v2_geoset, 
                     osg::uint v3_geoset )
  // Passed triangles as primitives are triangulated from the original
  //   GeoSet.  Note that the v? params are indicies into the Coord array,
  //   whereas the v?_geoset params are vertex numbers relative to the
  //   entire "area GeoSet" (which are different when the coords are 
  //   indexed).  The latter is needed to look up colors/normals/tcoords).
{
  osg::GeoSet::BindingType   binding;
  osg::GeoSet::PrimitiveType primtype = _area_geoset.getPrimType();
  int area_is_flatprim = ( primtype == osg::GeoSet::FLAT_TRIANGLE_STRIP ||
                           primtype == osg::GeoSet::FLAT_TRIANGLE_FAN );

  // Store the triangle coord indicies
  _cindex.push_back( v1 );
  _cindex.push_back( v2 );
  _cindex.push_back( v3 );
  
  int index;
  const osg::GeoSet::IndexPointer *ip;

  // Store normals (as needed)
  if ( _area_geoset.getNumNormals() ) {
    ip = &_area_geoset.getNormalIndices();

    // Grrr...  FLAT_ primitives lie about binding type... like Performer
    binding = _area_geoset.getNormalBinding();
    if ( area_is_flatprim && binding == osg::GeoSet::BIND_PERVERTEX )
      binding = osg::GeoSet::BIND_PERPRIM;

    switch ( binding ) {

      case osg::GeoSet::BIND_OVERALL   :                       // Only once
        if ( prim_index == 0 && prim_tri_index == 0 ) {
          index = ip->valid() ? (*ip)[0] : 0;
          _nindex.push_back( index );
          break;
        }

      case osg::GeoSet::BIND_PERPRIM   :                       // Once per tri
        index = ip->valid() ? (*ip)[prim_index] : prim_index;
        _nindex.push_back( index );
        break;

      case osg::GeoSet::BIND_PERVERTEX :                       // Each vertex
        index = ip->valid() ? (*ip)[v1_geoset] : v1_geoset;
        _nindex.push_back( index );
        index = ip->valid() ? (*ip)[v2_geoset] : v2_geoset;
        _nindex.push_back( index );
        index = ip->valid() ? (*ip)[v3_geoset] : v3_geoset;                  
        _nindex.push_back( index );
        break;

      case osg::GeoSet::BIND_OFF:
      case osg::GeoSet::BIND_DEFAULT:
        // Nonsensical cases
        break;
    }
  }

  // Store colors (as needed)
  if ( _area_geoset.getNumColors() ) {
    ip = &_area_geoset.getColorIndices();

    // Grrr...  FLAT_ primitives lie about binding type... like Performer
    binding = _area_geoset.getColorBinding();
    if ( area_is_flatprim && binding == osg::GeoSet::BIND_PERVERTEX )
      binding = osg::GeoSet::BIND_PERPRIM;

    switch ( binding ) {

      case osg::GeoSet::BIND_OVERALL   :                       // Only once
        if ( prim_index == 0 && prim_tri_index == 0 ) {
          index = ip->valid() ? (*ip)[0] : 0;
          _colindex.push_back( index );
          break;
        }

      case osg::GeoSet::BIND_PERPRIM   :                       // Once per tri
        index = ip->valid() ? (*ip)[prim_index] : prim_index;
        _colindex.push_back( index );
        break;

      case osg::GeoSet::BIND_PERVERTEX :                       // Each vertex
        index = ip->valid() ? (*ip)[v1_geoset] : v1_geoset;
        _colindex.push_back( index );
        index = ip->valid() ? (*ip)[v2_geoset] : v2_geoset;
        _colindex.push_back( index );
        index = ip->valid() ? (*ip)[v3_geoset] : v3_geoset;                  
        _colindex.push_back( index );
        break;

      case osg::GeoSet::BIND_OFF:
      case osg::GeoSet::BIND_DEFAULT:
        // Nonsensical cases
        break;
    }
  }

  // Store tcoords (as needed)
  if ( _area_geoset.getNumTextureCoords() ) {
    ip = &_area_geoset.getTextureIndices();

    switch ( _area_geoset.getTextureBinding() ) {

      case osg::GeoSet::BIND_OVERALL   :                       // Only once
        if ( prim_index == 0 && prim_tri_index == 0 ) {
          index = ip->valid() ? (*ip)[0] : 0;
          _tindex.push_back( index );
          break;
        }

      case osg::GeoSet::BIND_PERPRIM   :                       // Once per tri
        index = ip->valid() ? (*ip)[prim_index] : prim_index;
        _tindex.push_back( index );
        break;

      case osg::GeoSet::BIND_PERVERTEX :                       // Each vertex
        index = ip->valid() ? (*ip)[v1_geoset] : v1_geoset;
        _tindex.push_back( index );
        index = ip->valid() ? (*ip)[v2_geoset] : v2_geoset;
        _tindex.push_back( index );
        index = ip->valid() ? (*ip)[v3_geoset] : v3_geoset;                  
        _tindex.push_back( index );
        break;

      case osg::GeoSet::BIND_OFF:
      case osg::GeoSet::BIND_DEFAULT:
        // Nonsensical cases
        break;
    }
  }

}

//----------------------------------------------------------------------------
// Shamelessly lifted from the GeoSet header and adapted for our uses
//   This is the same except that: 1) it passes the coordinate indices 
//   to the op function insted of the Vec3s, and 2) it passes an original 
//   primitive index and "triangle-in-primitive" index to the op function,
//   3) it also passes the absolute vertex numbers in the primitive so they
//   can be used to look up normals/colors/tcoords, and 4) fixes some
//   problems with vertex ordering which caused FRONT/BACK confusion with
//   normals and lighting.
//   With this, we can construct a new TRIANGLE GeoSet from any area GeoSet.

/** Template function for iterating through a GeoSet operating on triangles
    with templated functor. Function automatically decomposes quads and polygons
    into sub triangles which are passed onto functor.*/

template<class T>
void for_each_triangle2(const osg::GeoSet& gset,T& op)
{
    switch(gset.getPrimType())
    {
    case(osg::GeoSet::TRIANGLE_STRIP):
    case(osg::GeoSet::FLAT_TRIANGLE_STRIP):
        {
            if (gset.getCoordIndices().valid())
            {
                if (gset.getCoordIndices()._is_ushort)
                {
                    osg::ushort* base = gset.getCoordIndices()._ptr._ushort;
                    osg::ushort* iptr = base;
                    const int numPrim = gset.getNumPrims();
                    for(int i=0; i<numPrim; ++i )
                    {
                        const int primLength = gset.getPrimLengths()[i];
                        osg::ushort* iend = iptr+primLength;
                        int tri=0;
                        for(int j = 2; j < primLength; j++ )
                        {
                          if( !(j%2) )
                              op(*(iptr),*(iptr+1),*(iptr+2),i,tri++,
                                 iptr-base,(iptr+1)-base,(iptr+2)-base);
                          else
                              op(*(iptr),*(iptr+2),*(iptr+1),i,tri++,
                                 iptr-base,(iptr+2)-base,(iptr+1)-base);
                          ++iptr;
                        }
                        iptr=iend;
                    }
                }
                else
                {
                    osg::uint* base = gset.getCoordIndices()._ptr._uint;
                    osg::uint* iptr = base;
                    const int numPrim = gset.getNumPrims();
                    for(int i=0; i<numPrim; ++i )
                    {
                        const int primLength = gset.getPrimLengths()[i];
                        osg::uint* iend = iptr+primLength;
                        int tri=0;
                        for(int j = 2; j < primLength; j++ )
                        {
                          if( !(j%2) )
                              op(*(iptr),*(iptr+1),*(iptr+2),i,tri++,
                                 iptr-base,(iptr+1)-base,(iptr+2)-base);
                          else
                              op(*(iptr),*(iptr+2),*(iptr+1),i,tri++,
                                 iptr-base,(iptr+2)-base,(iptr+1)-base);
                          ++iptr;
                        }
                        iptr=iend;
                    }
                }
            }
            else
            {
                osg::uint cindex = 0;
                const int numPrim = gset.getNumPrims();
                for(int i=0; i<numPrim; ++i )
                {
                    const int primLength  = gset.getPrimLengths()[i];
                    osg::uint cindex_end  = cindex+primLength;
                    int tri=0;
                    for(int j = 2; j < primLength; j++ )
                    {
                      if( !(j%2) )
                            op(cindex,cindex+1,cindex+2,i,tri++,
                               cindex,cindex+1,cindex+2);
                      else
                            op(cindex,cindex+2,cindex+1,i,tri++,
                               cindex,cindex+2,cindex+1);
                      ++cindex;
                    }
                    cindex = cindex_end;
                }
            }
        }
        break;
    case(osg::GeoSet::TRIANGLES):
        {

            if (gset.getCoordIndices().valid())
            {
                if (gset.getCoordIndices()._is_ushort)
                {
                    osg::ushort* base = gset.getCoordIndices()._ptr._ushort;
                    osg::ushort* iptr = base;
                    const int numPrim = gset.getNumPrims();
                    for(int i=0; i<numPrim; ++i )
                    {
                        op(*(iptr),*(iptr+1),*(iptr+2),i,0,
                           iptr-base,(iptr+1)-base,(iptr+2)-base);
                        iptr+=3;
                    }
                }
                else
                {
                    osg::uint* base = gset.getCoordIndices()._ptr._uint;
                    osg::uint* iptr = base;
                    const int numPrim = gset.getNumPrims();
                    for(int i=0; i<numPrim; ++i )
                    {
                        op(*(iptr),*(iptr+1),*(iptr+2),i,0,
                           iptr-base,(iptr+1)-base,(iptr+2)-base);
                        iptr+=3;
                    }
                }
            }
            else
            {
                osg::uint cindex = 0;
                const int numPrim = gset.getNumPrims();
                for(int i=0; i<numPrim; ++i )
                {
                    op(cindex,cindex+1,cindex+2,i,0,
                       cindex,cindex+1,cindex+2);
                    cindex+=3;
                }
            }
            
        }
        break;
    case(osg::GeoSet::QUAD_STRIP):
        {
            if (gset.getCoordIndices().valid())
            {
                if (gset.getCoordIndices()._is_ushort)
                {
                    osg::ushort* base = gset.getCoordIndices()._ptr._ushort;
                    osg::ushort* iptr = base;
                    const int numPrim = gset.getNumPrims();
                    for(int i=0; i<numPrim; ++i )
                    {
                        const int primLength = gset.getPrimLengths()[i];
                        osg::ushort* iend = iptr+primLength;
                        int tri=0;
                        for(int j = 3; j < primLength; j+=2 )
                        {
                            op(*(iptr),*(iptr+1),*(iptr+2),i,tri++,
                               iptr-base,(iptr+1)-base,(iptr+2)-base);
                            // rhh FIXME:  FIXED 1st vert offset
                            op(*(iptr+1),*(iptr+3),*(iptr+2),i,tri++,
                               (iptr+1)-base,(iptr+3)-base,(iptr+2)-base);
                            iptr+=2;
                        }
                        iptr=iend;
                    }
                }
                else
                {
                    osg::uint* base = gset.getCoordIndices()._ptr._uint;
                    osg::uint* iptr = base;
                    const int numPrim = gset.getNumPrims();
                    for(int i=0; i<numPrim; ++i )
                    {
                        const int primLength = gset.getPrimLengths()[i];
                        osg::uint* iend = iptr+primLength;
                        int tri=0;
                        for(int j = 3; j < primLength; j+=2 )
                        {
                            op(*(iptr),*(iptr+1),*(iptr+2),i,tri++,
                               iptr-base,(iptr+1)-base,(iptr+2)-base);
                            op(*(iptr+1),*(iptr+3),*(iptr+2),i,tri++,
                               (iptr+1)-base,(iptr+3)-base,(iptr+2)-base);
                            iptr+=2;
                        }
                        iptr=iend;
                    }
                }
            }
            else
            {
                osg::uint cindex = 0;
                const int numPrim = gset.getNumPrims();
                for(int i=0; i<numPrim; ++i )
                {
                    const int primLength = gset.getPrimLengths()[i];
                    osg::uint cindex_end = cindex+primLength;
                    int tri=0;
                    for(int j = 3; j < primLength; j+=2 )
                    {
                        op(cindex,cindex+1,cindex+2,i,tri++,
                           cindex,cindex+1,cindex+2);
                        op(cindex+1,cindex+3,cindex+2,i,tri++,
                           cindex+1,cindex+3,cindex+2);
                        cindex+=2;
                    }
                    cindex = cindex_end;
                }
            }
        }
        break;
    case(osg::GeoSet::QUADS):
        {
            if (gset.getCoordIndices().valid())
            {
                if (gset.getCoordIndices()._is_ushort)
                {
                    osg::ushort* base = gset.getCoordIndices()._ptr._ushort;
                    osg::ushort* iptr = base;
                    const int numPrim = gset.getNumPrims();
                    for(int i=0; i<numPrim; ++i )
                    {
                        op(*(iptr),*(iptr+1),*(iptr+2),i,0,
                           iptr-base,(iptr+1)-base,(iptr+2)-base);
                        // rhh FIXME:  FIXED 2nd & 3rd vert offset
                        op(*(iptr),*(iptr+2),*(iptr+3),i,1,
                           iptr-base,(iptr+2)-base,(iptr+3)-base);
                        iptr+=4;
                    }
                }
                else
                {
                    osg::uint* base = gset.getCoordIndices()._ptr._uint;
                    osg::uint* iptr = base;
                    const int numPrim = gset.getNumPrims();
                    for(int i=0; i<numPrim; ++i )
                    {
                        op(*(iptr),*(iptr+1),*(iptr+2),i,0,
                           iptr-base,(iptr+1)-base,(iptr+2)-base);
                        op(*(iptr),*(iptr+2),*(iptr+3),i,1,
                           iptr-base,(iptr+2)-base,(iptr+3)-base);
                        iptr+=4;
                    }
                }
            }
            else
            {
                osg::uint cindex = 0;
                const int numPrim = gset.getNumPrims();
                for(int i=0; i<numPrim; ++i )
                {
                    op(cindex,cindex+1,cindex+2,i,0,
                       cindex,cindex+1,cindex+2);
                    op(cindex,cindex+2,cindex+3,i,1,
                       cindex,cindex+2,cindex+3);
                    cindex+=4;
                }
            }
            
        }
        break;
    case(osg::GeoSet::TRIANGLE_FAN):
    case(osg::GeoSet::FLAT_TRIANGLE_FAN):  // FIXME:  rhh added
    case(osg::GeoSet::POLYGON):
        {
            if (gset.getCoordIndices().valid())
            {
                if (gset.getCoordIndices()._is_ushort)
                {
                    osg::ushort* base = gset.getCoordIndices()._ptr._ushort;
                    osg::ushort* iptr = base;
                    const int numPrim = gset.getNumPrims();
                    for(int i=0; i<numPrim; ++i )
                    {
                        const int primLength = gset.getPrimLengths()[i];
                        int tri=0;
                        if (primLength>0)
                        {
                            osg::ushort *start = iptr;
                            osg::ushort* iend = iptr+primLength;
                            ++iptr;
                            for(int j = 2; j < primLength; ++j )
                            {
                                op(*start,*(iptr),*(iptr+1),i,tri++,
                                   start-base, iptr-base,(iptr+1)-base);
                                ++iptr;
                            }
                            iptr=iend;
                        }
                    }
                }
                else
                {
                    osg::uint* base = gset.getCoordIndices()._ptr._uint;
                    osg::uint* iptr = base;
                    const int numPrim = gset.getNumPrims();
                    for(int i=0; i<numPrim; ++i )
                    {
                        const int primLength = gset.getPrimLengths()[i];
                        int tri=0;
                        if (primLength>0)
                        {
                            osg::uint *start = iptr;
                            osg::uint* iend = iptr+primLength;
                            ++iptr;
                            for(int j = 2; j < primLength; ++j )
                            {
                                op(*start,*(iptr),*(iptr+1),i,tri++,
                                   start-base, iptr-base,(iptr+1)-base);
                                ++iptr;
                            }
                            iptr=iend;
                        }
                    }
                }
            }
            else
            {
                osg::uint cindex = 0;
                const int numPrim = gset.getNumPrims();
                for(int i=0; i<numPrim; ++i )
                {
                    const int primLength = gset.getPrimLengths()[i];
                    int tri=0;
                    if (primLength>0)
                    {
                        osg::uint cindex_start = cindex;
                        osg::uint cindex_end   = cindex+primLength;
                        ++cindex;
                        for(int j = 2; j < primLength; ++j)
                        {
                            op(cindex_start,cindex,cindex+1,i,tri++,
                               cindex_start,cindex,cindex+1);
                            ++cindex;
                        }
                        cindex = cindex_end;
                    }
                }
            }
        }
        break;
    default:
        break;        
    }

};


//---------------------------------------------------------------------------
osg::GeoSet *AreaGeoSetTriangulator::BuildGeoSet()
  // After a triangulation pass, generate a valid TRIANGLE GeoSet containing 
  //   the resulting triangles (w/ colors, normals, and tcoords).
{
  int num_tris = _cindex.size() / 3;

  osg::GeoSet *res = new osg::GeoSet;
  res->setPrimType( osg::GeoSet::TRIANGLES );
  res->setNumPrims( num_tris );

  // First, dup all of the data arrays -- they haven't changed
  //   NOTE:  We generate fresh arrays for the new GeoSet to be compatible 
  //   with the delete [] behavior of the default GeoSet
  //   AttributeDeleteFunctor, which is invoked in GeoSet::~GeoSet.

  osg::Vec3 *new_coords  = 0;
  osg::Vec3 *new_normals = 0;
  osg::Vec4 *new_colors  = 0;
  osg::Vec2 *new_tcoords = 0;

  const osg::Vec3 *coords  = _area_geoset.getCoords();
  const osg::Vec3 *normals = _area_geoset.getNormals();
  const osg::Vec4 *colors  = _area_geoset.getColors();
  const osg::Vec2 *tcoords = _area_geoset.getTextureCoords();

  if ( coords )
    new_coords  = new osg::Vec3[ _area_geoset.getNumCoords()  ];
  if ( normals ) 
    new_normals = new osg::Vec3[ _area_geoset.getNumNormals() ];
  if ( colors )
    new_colors  = new osg::Vec4[ _area_geoset.getNumColors()  ];
  if ( tcoords )
    new_tcoords = new osg::Vec2[ _area_geoset.getNumTextureCoords() ];

  memcpy( new_coords, coords, 
          sizeof(osg::Vec3) * _area_geoset.getNumCoords() );
  memcpy( new_normals, normals, 
          sizeof(osg::Vec3) * _area_geoset.getNumNormals() );
  memcpy( new_colors, colors, 
          sizeof(osg::Vec4) * _area_geoset.getNumColors() );
  memcpy( new_tcoords, tcoords, 
          sizeof(osg::Vec2) * _area_geoset.getNumTextureCoords() );

  // Now generate the index arrays
  osg::uint *new_cindex   = 0;
  osg::uint *new_nindex   = 0;
  osg::uint *new_colindex = 0;
  osg::uint *new_tindex   = 0;

  if ( _cindex.size()   ) new_cindex   = new osg::uint[ _cindex.size()   ];
  if ( _nindex.size()   ) new_nindex   = new osg::uint[ _nindex.size()   ];
  if ( _colindex.size() ) new_colindex = new osg::uint[ _colindex.size() ];
  if ( _tindex.size()   ) new_tindex   = new osg::uint[ _tindex.size()   ];

  memcpy( new_cindex  , &_cindex  [0], sizeof(osg::uint) * _cindex.size()   );
  memcpy( new_nindex  , &_nindex  [0], sizeof(osg::uint) * _nindex.size()   );
  memcpy( new_colindex, &_colindex[0], sizeof(osg::uint) * _colindex.size() );
  memcpy( new_tindex  , &_tindex  [0], sizeof(osg::uint) * _tindex.size()   );

  res->setCoords ( new_coords , new_cindex   );
  res->setNormals( new_normals, new_nindex   );
  res->setColors ( new_colors , new_colindex );
  res->setTextureCoords( new_tcoords, new_tindex );

  // And finally, set the normal/color/tcoord binding 
  //   (Grrr...  FLAT_ primitives lie about binding type... like Performer)
  osg::GeoSet::BindingType  nbinding = _area_geoset.getNormalBinding();
  osg::GeoSet::BindingType  cbinding = _area_geoset.getColorBinding();
  if ( _area_geoset.getPrimType() == osg::GeoSet::FLAT_TRIANGLE_STRIP ||
       _area_geoset.getPrimType() == osg::GeoSet::FLAT_TRIANGLE_FAN ) {
    if ( nbinding == osg::GeoSet::BIND_PERVERTEX )
      nbinding = osg::GeoSet::BIND_PERPRIM;
    if ( cbinding == osg::GeoSet::BIND_PERVERTEX )
      cbinding = osg::GeoSet::BIND_PERPRIM;
  }
  
  res->setNormalBinding ( nbinding );
  res->setColorBinding  ( cbinding );
  res->setTextureBinding( _area_geoset.getTextureBinding()  );

  return res;
}

//---------------------------------------------------------------------------
osg::GeoSet *TriangulateAreaGeoSet( const osg::GeoSet &geoset )
  // The strategy here is to generate new coord/normal/color/tcoord index
  //   arrays as we bust up the primitives into triangles.  The data arrays
  //   don't change between the old and new geoset.
{
  geoset.computeNumVerts();  // Update # coords/# normals from index arrays

  AreaGeoSetTriangulator triangulator( geoset );
  for_each_triangle2( geoset, triangulator );
  return triangulator.BuildGeoSet();
}

}; // namespace dx
