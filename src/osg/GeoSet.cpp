/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <math.h>
#include <float.h>

#include <osg/GeoSet>
#include <osg/Notify>

#include <osg/Geometry>
#include <osg/ShadeModel>

#include <set>

using namespace osg;

GeoSet::GeoSet()
{
    // we will use the a default delete functor which
    // assumes that users have allocated arrays with new only
    // and that now sharing of attributes exists between GeoSet's.
    _adf = new AttributeDeleteFunctor;

    _coords         = (Vec3 *)0;

    _normals = (Vec3 *)0;

    _colors     = (Vec4 *)0;

    _tcoords = (Vec2 *)0;

    _iarray = (float *)0L;
    _iaformat = IA_OFF;
    _ogliaformat = 0;


    _numprims = 0;
    _primtype = NO_TYPE;
    _oglprimtype = 0xFFFF;
    _needprimlen = 0;
    _primLengths = (int *)0;

    _numcoords = 0;
    _numnormals = 0;
    _numcolors = 0;
    _numtcoords = 0;

    _normal_binding = BIND_OFF;
    _color_binding  = BIND_OFF;
    _texture_binding  = BIND_OFF;

    _fast_path = 1;

    _primlength = 0;
    _flat_shaded_skip = 0;

}


GeoSet::GeoSet(const GeoSet& geoset,const CopyOp& copyop):
    Drawable(geoset,copyop)
{
    // ensure that the num of vertices etc have been set up before we copy.
    geoset.computeNumVerts();

    _adf = geoset._adf;

    _numprims = geoset._numprims;
    _primtype = geoset._primtype;
    _needprimlen = geoset._needprimlen;
    _oglprimtype = geoset._oglprimtype;
    _primlength = geoset._primlength;
    _flat_shaded_skip = geoset._flat_shaded_skip;
    if (geoset._primLengths)
    {
        _primLengths = new int [_numprims];
        memcpy(_primLengths,geoset._primLengths,_numprims*sizeof(int));
    }
    else
    {
        _primLengths = 0L;
    }

    _numcoords = geoset._numcoords;
    _cindex = geoset._cindex;
    if (geoset._coords)
    {
        _coords = new Vec3 [_numcoords];
        memcpy(_coords,geoset._coords,_numcoords*sizeof(Vec3));
    }
    else
    {
        _coords = 0L;
    }
    
    _normal_binding = geoset._normal_binding;
    _numnormals = geoset._numnormals;
    _nindex = geoset._nindex;
    if (geoset._normals)
    {
        _normals = new Vec3 [_numnormals];
        memcpy(_normals,geoset._normals,_numnormals*sizeof(Vec3));
    }
    else
    {
        _normals = 0L;
    }
    
    _color_binding = geoset._color_binding;
    _numcolors = geoset._numcolors;
    _colindex = geoset._colindex;
    if (geoset._colors)
    {
        _colors = new Vec4 [_numcolors];
        memcpy(_colors,geoset._colors,_numcolors*sizeof(Vec4));
    }
    else
    {
        _colors = 0L;
    }

    _texture_binding = geoset._texture_binding;
    _numtcoords = geoset._numtcoords;
    _tindex = geoset._tindex;
    if (geoset._tcoords)
    {
        _tcoords = new Vec2 [_numtcoords];
        memcpy(_tcoords,geoset._tcoords,_numtcoords*sizeof(Vec2));
    }
    else
    {
        _tcoords = 0L;
    }

    _iaindex = geoset._iaindex;
    _iaformat = geoset._iaformat;
    _ogliaformat = geoset._ogliaformat;
    _fast_path = geoset._fast_path;
    if (geoset._iarray)
    {
        _iarray = 0L;
        osg::notify(osg::WARN)<<"Warning :: GeoSet copy constructor error, copying of interleaved arrays unsupported."<<std::endl; 
    }
    else
    {
        _iarray = 0L;
    }
}

#define INDEX_ARRAY_DELETE(A) if (A._is_ushort) ushortList.insert(A._ptr._ushort); else uintList.insert(A._ptr._uint);

void GeoSet::AttributeDeleteFunctor::operator() (GeoSet* gset)
{
    // note, delete checks for NULL so want delete NULL pointers.
    delete [] gset->getPrimLengths();
    delete [] gset->getCoords();
    delete [] gset->getNormals();
    delete [] gset->getColors();
    delete [] gset->getTextureCoords();
    // can't delete a void* right now... interleaved arrays needs to be reimplemented with a proper pointer..
    //    delete [] gset->getInterleavedArray();


    // coord indicies may be shared so we have to go through the long winded
    // step of creating unique pointer sets which we then delete.  This
    // ensures that arrays aren't delete twice. Robert. 
    std::set<GLushort*> ushortList;
    std::set<GLuint*>   uintList;
    
    INDEX_ARRAY_DELETE(gset->getCoordIndices())
    INDEX_ARRAY_DELETE(gset->getNormalIndices());
    INDEX_ARRAY_DELETE(gset->getColorIndices());
    INDEX_ARRAY_DELETE(gset->getTextureIndices())
    INDEX_ARRAY_DELETE(gset->getInterleavedIndices());
    
    for(std::set<GLushort*>::iterator sitr=ushortList.begin();
        sitr!=ushortList.end();
        ++sitr)
    {
        delete [] *sitr;
    }
    
    for(std::set<GLuint*>::iterator iitr=uintList.begin();
        iitr!=uintList.end();
        ++iitr)
    {
        delete [] *iitr;
    }
}

#undef INDEX_ARRAY_DELETE

GeoSet::~GeoSet()
{
    // if attached call the adf do delete the memory.
    if (_adf.valid()) (*_adf)(this);
}


void GeoSet::setColorBinding( BindingType binding )
{
    if( binding != BIND_DEFAULT &&
        binding != BIND_OFF &&
        binding != BIND_OVERALL &&
        binding != BIND_PERPRIM  &&
        binding != BIND_PERVERTEX )
        _color_binding = BIND_OFF;
    else
        _color_binding = binding;

    if( _color_binding == BIND_DEFAULT )
        _color_binding = BIND_PERVERTEX;

    set_fast_path();
}


void GeoSet::setNormalBinding( BindingType binding )
{
    if( binding != BIND_DEFAULT &&
        binding != BIND_OFF &&
        binding != BIND_OVERALL &&
        binding != BIND_PERPRIM  &&
        binding != BIND_PERVERTEX )
        _normal_binding = BIND_OFF;
    else
        _normal_binding = binding;

    if( _normal_binding == BIND_DEFAULT )
        _normal_binding = BIND_PERVERTEX;

    set_fast_path();
}


void GeoSet::setTextureBinding( BindingType binding )
{
    if( binding != BIND_DEFAULT &&
        binding != BIND_OFF &&
        binding != BIND_PERVERTEX )
        _texture_binding = BIND_OFF;
    else
        _texture_binding = binding;

    if( _texture_binding == BIND_DEFAULT )
        _texture_binding = BIND_PERVERTEX;

    set_fast_path();
}

void GeoSet::drawImplementation(State& state) const
{
    if( _coords == (Vec3 *)0 && _iaformat == IA_OFF ) return;

    // need to do this to get a valid _numcoords, _numindices & _primlength
    if( _numcoords == 0 )
        computeNumVerts();

    if( _fast_path )
        draw_fast_path(state);
    else
        draw_alternate_path(state);
}

void GeoSet::computeNumVerts() const
{
    int i;
    int numverts=0;

    int flat_shaded_offset=0;
    if      (_primtype == FLAT_LINE_STRIP)     flat_shaded_offset=_numprims;
    else if (_primtype == FLAT_TRIANGLE_STRIP) flat_shaded_offset=2*_numprims;
    else if (_primtype == FLAT_TRIANGLE_FAN)   flat_shaded_offset=2*_numprims;

    switch( _primtype )
    {
        case POINTS :
            _primlength = 1;
            numverts = _numprims * _primlength;
            break;

        case LINES :
            _primlength = 2;
            numverts = _numprims * _primlength;
            break;

        case TRIANGLES :
            _primlength = 3;
            numverts =  _numprims * _primlength;
            break;

        case QUADS :
            _primlength = 4;
            numverts =  _numprims * _primlength;
            break;

        case QUAD_STRIP :
        case FLAT_TRIANGLE_FAN :
        case TRIANGLE_FAN :
        case LINE_LOOP :
        case LINE_STRIP :
        case FLAT_LINE_STRIP :
        case TRIANGLE_STRIP :
        case FLAT_TRIANGLE_STRIP :
        case POLYGON :
            _primlength = 0;
            numverts = 0;
            for( i = 0; i < _numprims; i++ )
                numverts += _primLengths[i];
            break;
        default:
            notify(WARN) << "Not supported primitive "<<(int)_primtype<<std::endl;
            break;
    }

    if( _cindex._ptr._ushort)
    {
        _cindex._size = numverts;
        _numcoords = _cindex.maxIndex() + 1;
    }
    else
    {
        _cindex._size = 0;
        _numcoords = numverts;
    }

    if (_normals)
    {

        int nn;
        switch(_normal_binding)
        {
            case (BIND_OFF)        : nn = 0; break;
            case (BIND_OVERALL)    : nn = 1; break;
            case (BIND_PERPRIM)    : nn = getNumPrims(); break;
            case (BIND_PERVERTEX)  : nn = numverts-flat_shaded_offset; break;
            default                     : nn = 0; break;
        }

        // calc the maximum num of normals from the index list.
        if( _nindex._ptr._ushort)
        {
            _nindex._size = nn;
            _numnormals = _nindex.maxIndex() + 1;
        }
        else
        {
            _nindex._size = 0;
            _numnormals = nn;
        }
    }
    else
    {
        _nindex._size = 0;
        _numnormals = 0;
    }

    if (_colors)
    {

        int nn;
        switch(_color_binding)
        {
            case (BIND_OFF)        : nn = 0; break;
            case (BIND_OVERALL)    : nn = 1; break;
            case (BIND_PERPRIM)    : nn = getNumPrims(); break;
            case (BIND_PERVERTEX)  : nn = numverts-flat_shaded_offset; break;
            default                     : nn = 0; break;
        }

        // calc the maximum num of colors from the index list.
        if( _colindex._ptr._ushort)
        {
            _colindex._size = nn;
            _numcolors = _colindex.maxIndex() + 1;
        }
        else
        {
            _colindex._size = 0;
            _numcolors = nn;
        }

    }
    else
    {
        _colindex._size = 0;
        _numcolors = 0;
    }

    if (_tcoords)
    {

        int nn;
        switch(_texture_binding)
        {
            case (BIND_OFF)        : nn = 0; break;
            case (BIND_OVERALL)    : nn = 1; break;
            case (BIND_PERPRIM)    : nn = getNumPrims(); break;
            case (BIND_PERVERTEX)  : nn = numverts; break;
            default                     : nn = 0; break;
        }

        // calc the maximum num of vertex from the index list.
        if( _tindex._ptr._ushort)
        {
            _tindex._size = nn;
            _numtcoords = _tindex.maxIndex() + 1;
        }
        else
        {
            _tindex._size = 0;
            _numtcoords = nn;
        }
    }
    else
    {
        _tindex._size = 0;
        _numtcoords = 0;
    }

}


// just use the base Drawable's PrimitiveFunctor based implementation.
bool GeoSet::computeBound() const
{
    _bbox.init();

    if( _iaformat == IA_OFF && _coords == (Vec3 *)0 ) return false;

    if( _numcoords == 0 )
    {
        computeNumVerts();
    }

    if( _numcoords == 0 )
        return false;

    Vec3 center(0.0f,0.0f,0.0f);

    int i;
    if( _iaformat == IA_OFF )
    {
        if( _cindex.valid() )
	{
            for( i = 0; i < int(_cindex._size); i++ )
            {
                center += _coords[_cindex[i]];
                _bbox.expandBy(_coords[_cindex[i]]);
            }
	}
	else
	{
            for( i = 0; i < _numcoords; i++ )
            {
                center += _coords[i];
                _bbox.expandBy(_coords[i]);
            }
        }
    }
    else
    {
        float *fptr = (float*)_iarray;
	int stride = 
	    _iaformat == IA_V2F             ?  2 :
            _iaformat == IA_V3F             ?  3 :
            _iaformat == IA_C4UB_V2F        ?  6 :
            _iaformat == IA_C4UB_V3F        ?  7 :
            _iaformat == IA_C3F_V3F         ?  6 :
            _iaformat == IA_N3F_V3F         ?  6 :
            _iaformat == IA_C4F_N3F_V3F     ? 10 :
            _iaformat == IA_T2F_V3F         ?  5 : 
            _iaformat == IA_T4F_V4F         ?  8 :
            _iaformat == IA_T2F_C4UB_V3F    ?  9 :
            _iaformat == IA_T2F_C3F_V3F     ?  8 :
            _iaformat == IA_T2F_N3F_V3F     ?  8 :
            _iaformat == IA_T2F_C4F_N3F_V3F ? 12 :
            _iaformat == IA_T4F_C4F_N3F_V4F ? 15 : -1;

	if( stride == -1 ) // INTERNAL ERROR!!
	     return false; 

	int offset = 
	    _iaformat == IA_V2F             ?  0 :
            _iaformat == IA_V3F             ?  0 :
            _iaformat == IA_C4UB_V2F        ?  4 :
            _iaformat == IA_C4UB_V3F        ?  4 :
            _iaformat == IA_C3F_V3F         ?  3 :
            _iaformat == IA_N3F_V3F         ?  3 :
            _iaformat == IA_C4F_N3F_V3F     ?  7 :
            _iaformat == IA_T2F_V3F         ?  2 : 
            _iaformat == IA_T4F_V4F         ?  3 :
            _iaformat == IA_T2F_C4UB_V3F    ?  6 :
            _iaformat == IA_T2F_C3F_V3F     ?  5 :
            _iaformat == IA_T2F_N3F_V3F     ?  5 :
            _iaformat == IA_T2F_C4F_N3F_V3F ?  9 :
            _iaformat == IA_T4F_C4F_N3F_V4F ? 11 : 0;

	fptr += offset;

	int ncomp = 
	    _iaformat == IA_V2F             ?  2 :
            _iaformat == IA_V3F             ?  3 :
            _iaformat == IA_C4UB_V2F        ?  2 :
            _iaformat == IA_C4UB_V3F        ?  3 :
            _iaformat == IA_C3F_V3F         ?  3 :
            _iaformat == IA_N3F_V3F         ?  3 :
            _iaformat == IA_C4F_N3F_V3F     ?  3 :
            _iaformat == IA_T2F_V3F         ?  3 : 
            _iaformat == IA_T4F_V4F         ?  4 :
            _iaformat == IA_T2F_C4UB_V3F    ?  3 :
            _iaformat == IA_T2F_C3F_V3F     ?  3 :
            _iaformat == IA_T2F_N3F_V3F     ?  3 :
            _iaformat == IA_T2F_C4F_N3F_V3F ?  3 :
            _iaformat == IA_T4F_C4F_N3F_V4F ?  4 : 0;
        
	
	for( i = 0; i < _numcoords; i++ )
	{
	    float x = fptr[0];
	    float y = fptr[1];
	    float z = ncomp >= 3 ? fptr[2] : 0.0f;
	    Vec3  vv(x,y,z);
	    center += vv;
	    _bbox.expandBy(vv);

	    fptr += stride;
	}
    }

    center /= (float)_numcoords;

    _bbox_computed=true;
    
    return true;
}

bool GeoSet::check() const
{
    if( _coords == (Vec3 *)0 ) return false;

    if( _cindex.valid() ||
        _nindex.valid() ||
        _colindex.valid() ||
        _tindex.valid()  )
    {

        if( (_coords  && _cindex.null()) ||
            (_normals && _nindex.null()) ||
            (_colors  && _colindex.null()) ||
            (_tcoords && _tindex.null())  )
        {

            notify(WARN) <<  "GeoSet::check() : "
                "Cannot mix indexed and non-indexed attributes.\n";
            return false;
        }
    }
    return true;
}


void GeoSet::setPrimType( PrimitiveType type )
{
    switch( type )
    {
        case NO_TYPE: break;

        case POINTS:              _oglprimtype = GL_POINTS;         _needprimlen = 0; break;
        case LINES:               _oglprimtype = GL_LINES;          _needprimlen = 0; break;
        case FLAT_LINE_STRIP:     _oglprimtype = GL_LINE_STRIP;     _needprimlen=1; break;
        case LINE_STRIP:          _oglprimtype = GL_LINE_STRIP;     _needprimlen=1; break;
        case LINE_LOOP:           _oglprimtype = GL_LINE_LOOP;      _needprimlen=1; break;
        case TRIANGLES:           _oglprimtype = GL_TRIANGLES;      _needprimlen=0; break;
        case FLAT_TRIANGLE_STRIP: _oglprimtype = GL_TRIANGLE_STRIP; _needprimlen=1; break;
        case TRIANGLE_STRIP:      _oglprimtype = GL_TRIANGLE_STRIP; _needprimlen=1; break;
        case TRIANGLE_FAN:        _oglprimtype = GL_TRIANGLE_FAN;   _needprimlen=1; break;
        case FLAT_TRIANGLE_FAN:   _oglprimtype = GL_TRIANGLE_FAN;   _needprimlen=1; break;
        case QUADS:               _oglprimtype = GL_QUADS;          _needprimlen=0; break;
        case QUAD_STRIP:          _oglprimtype = GL_QUAD_STRIP;     _needprimlen=1; break;
        case POLYGON :            _oglprimtype = GL_POLYGON;        _needprimlen=1; break;
    }

    _primtype = type;

    if( _primtype == FLAT_LINE_STRIP ) _flat_shaded_skip = 1;
    else if( _primtype == FLAT_TRIANGLE_STRIP ) _flat_shaded_skip = 2;
    else if( _primtype == FLAT_TRIANGLE_FAN )   _flat_shaded_skip = 2;
    else _flat_shaded_skip = 0;
}


void GeoSet::setCoords( Vec3 *cp )
{
    _coords = cp;
    _cindex.setToNull();
    dirtyBound();
    set_fast_path();
}


void GeoSet::setCoords( Vec3 *cp, GLushort *ci )
{
    _coords = cp;
    // note the size of cindex defaults 0, but will be recalculated
    // automatically by computeNumVerts().
    _cindex.set(0,ci);
    dirtyBound();
    set_fast_path();
}


void GeoSet::setCoords( Vec3 *cp, GLuint *ci )
{
    _coords = cp;
    // note the size of cindex defaults 0, but will be recalculated
    // automatically by computeNumVerts().
    _cindex.set(0,ci);
    dirtyBound();
    set_fast_path();
}

void GeoSet::setCoords( Vec3 *cp, IndexPointer& ip )
{
    _coords = cp;
    _cindex = ip;
    dirtyBound();
    set_fast_path();
}

void GeoSet::setNormals( Vec3 *np )
{
    _normals = np;
    _nindex.setToNull();
    if( _normal_binding == BIND_OFF )
        setNormalBinding( BIND_DEFAULT );
    else
        set_fast_path();
}


void GeoSet::setNormals( Vec3 *np, GLushort *ni )
{
    _normals = np;
    // note the size of nindex defaults 0, but will be recalculated
    // automatically by computeNumVerts().
    _nindex.set(0,ni);
    if( _normal_binding == BIND_OFF )
        setNormalBinding( BIND_DEFAULT );
    else
        set_fast_path();
}

void GeoSet::setNormals( Vec3 *np, GLuint *ni )
{
    _normals = np;
    // note the size of nindex defaults 0, but will be recalculated
    // automatically by computeNumVerts().
    _nindex.set(0,ni);
    if( _normal_binding == BIND_OFF )
        setNormalBinding( BIND_DEFAULT );
    else
        set_fast_path();
}

void GeoSet::setNormals( Vec3 *cp, IndexPointer& ip )
{
    _normals = cp;
    _nindex = ip;
    _bbox_computed = false;
    if( _normal_binding == BIND_OFF )
        setNormalBinding( BIND_DEFAULT );
    else
        set_fast_path();
}


void GeoSet::setColors( Vec4 *lp )
{
    _colors = lp;
    _colindex.setToNull();
    if( _color_binding == BIND_OFF )
        setColorBinding( BIND_DEFAULT );
    else
        set_fast_path();
}


void GeoSet::setColors( Vec4 *lp, GLushort *coli )
{
    _colors = lp;
    // note the size of colindex defaults 0, but will be recalculated
    // automatically by computeNumVerts().
    _colindex.set(0,coli);
    if( _color_binding == BIND_OFF )
        setColorBinding( BIND_DEFAULT );
    else
        set_fast_path();
}

void GeoSet::setColors( Vec4 *lp, GLuint *coli )
{
    _colors = lp;
    // note the size of colindex defaults 0, but will be recalculated
    // automatically by computeNumVerts().
    _colindex.set(0,coli);
    if( _color_binding == BIND_OFF )
        setColorBinding( BIND_DEFAULT );
    else
        set_fast_path();
}

void GeoSet::setColors( Vec4 *cp, IndexPointer& ip )
{
    _colors = cp;
    _colindex = ip;
    _bbox_computed = false;
    if( _color_binding == BIND_OFF )
        setColorBinding( BIND_DEFAULT );
    else
        set_fast_path();
}


void GeoSet::setTextureCoords( Vec2 *tc )
{
    _tcoords = tc;
    _tindex.setToNull();
    if( _texture_binding == BIND_OFF )
        setTextureBinding( BIND_DEFAULT );
    else
        set_fast_path();
}


void GeoSet::setTextureCoords( Vec2 *tc, GLushort *ti )
{
    _tcoords = tc;
    // note the size of tindex defaults 0, but will be recalculated
    // automatically by computeNumVerts().
    _tindex.set(0,ti);
    if( _texture_binding == BIND_OFF )
        setTextureBinding( BIND_DEFAULT );
    else
        set_fast_path();

}

void GeoSet::setTextureCoords( Vec2 *tc, GLuint *ti )
{
    _tcoords = tc;
    // note the size of tindex defaults 0, but will be recalculated
    // automatically by computeNumVerts().
    _tindex.set(0,ti);
    if( _texture_binding == BIND_OFF )
        setTextureBinding( BIND_DEFAULT );
    else
        set_fast_path();

}

void GeoSet::setTextureCoords( Vec2 *cp, IndexPointer& ip )
{
    _tcoords = cp;
    _tindex = ip;
    _bbox_computed = false;
    if( _texture_binding == BIND_OFF )
        setTextureBinding( BIND_DEFAULT );
    else
        set_fast_path();
}

void GeoSet::setInterleavedArray( InterleaveArrayType format, float *pointer )
{
    _iaformat = format;

    _ogliaformat =
        (_iaformat == IA_OFF )            ? 0 :
    (_iaformat == IA_V2F )            ?  GL_V2F:
    (_iaformat == IA_V3F )            ?  GL_V3F:
    (_iaformat == IA_C4UB_V2F)        ?  GL_C4UB_V2F:
    (_iaformat == IA_C4UB_V3F)        ?  GL_C4UB_V3F:
    (_iaformat == IA_C3F_V3F)         ?  GL_C3F_V3F:
    (_iaformat == IA_N3F_V3F)         ?  GL_N3F_V3F:
    (_iaformat == IA_C4F_N3F_V3F)     ?  GL_C4F_N3F_V3F:
    (_iaformat == IA_T2F_V3F)         ?  GL_T2F_V3F:
    (_iaformat == IA_T4F_V4F)         ?  GL_T4F_V4F:
    (_iaformat == IA_T2F_C4UB_V3F)    ?  GL_T2F_C4UB_V3F:
    (_iaformat == IA_T2F_C3F_V3F)     ?  GL_T2F_C3F_V3F:
    (_iaformat == IA_T2F_N3F_V3F)     ?  GL_T2F_N3F_V3F:
    (_iaformat == IA_T2F_C4F_N3F_V3F) ?  GL_T2F_C4F_N3F_V3F:
    (_iaformat == IA_T4F_C4F_N3F_V4F) ?  GL_T4F_C4F_N3F_V4F: 0;

    _iarray = pointer;
    _iaindex.setToNull();

    set_fast_path();
}


void GeoSet::setInterleavedArray( InterleaveArrayType format, float *ia, GLushort *iai )
{
    _iaformat = format;

    _ogliaformat =
        (_iaformat == IA_OFF )            ? 0 :
    (_iaformat == IA_V2F )            ?  GL_V2F:
    (_iaformat == IA_V3F )            ?  GL_V3F:
    (_iaformat == IA_C4UB_V2F)        ?  GL_C4UB_V2F:
    (_iaformat == IA_C4UB_V3F)        ?  GL_C4UB_V3F:
    (_iaformat == IA_C3F_V3F)         ?  GL_C3F_V3F:
    (_iaformat == IA_N3F_V3F)         ?  GL_N3F_V3F:
    (_iaformat == IA_C4F_N3F_V3F)     ?  GL_C4F_N3F_V3F:
    (_iaformat == IA_T2F_V3F)         ?  GL_T2F_V3F:
    (_iaformat == IA_T4F_V4F)         ?  GL_T4F_V4F:
    (_iaformat == IA_T2F_C4UB_V3F)    ?  GL_T2F_C4UB_V3F:
    (_iaformat == IA_T2F_C3F_V3F)     ?  GL_T2F_C3F_V3F:
    (_iaformat == IA_T2F_N3F_V3F)     ?  GL_T2F_N3F_V3F:
    (_iaformat == IA_T2F_C4F_N3F_V3F) ?  GL_T2F_C4F_N3F_V3F:
    (_iaformat == IA_T4F_C4F_N3F_V4F) ?  GL_T4F_C4F_N3F_V4F: 0;

    _iarray = ia;
    // note the size of _iaindex defaults 0, but will be recalculated
    // automatically by computeNumVerts().
    _iaindex.set(0,iai);

    set_fast_path();
}

void GeoSet::setInterleavedArray( InterleaveArrayType format, float *ia, GLuint *iai )
{
    _iaformat = format;

    _ogliaformat =
        (_iaformat == IA_OFF )            ? 0 :
    (_iaformat == IA_V2F )            ?  GL_V2F:
    (_iaformat == IA_V3F )            ?  GL_V3F:
    (_iaformat == IA_C4UB_V2F)        ?  GL_C4UB_V2F:
    (_iaformat == IA_C4UB_V3F)        ?  GL_C4UB_V3F:
    (_iaformat == IA_C3F_V3F)         ?  GL_C3F_V3F:
    (_iaformat == IA_N3F_V3F)         ?  GL_N3F_V3F:
    (_iaformat == IA_C4F_N3F_V3F)     ?  GL_C4F_N3F_V3F:
    (_iaformat == IA_T2F_V3F)         ?  GL_T2F_V3F:
    (_iaformat == IA_T4F_V4F)         ?  GL_T4F_V4F:
    (_iaformat == IA_T2F_C4UB_V3F)    ?  GL_T2F_C4UB_V3F:
    (_iaformat == IA_T2F_C3F_V3F)     ?  GL_T2F_C3F_V3F:
    (_iaformat == IA_T2F_N3F_V3F)     ?  GL_T2F_N3F_V3F:
    (_iaformat == IA_T2F_C4F_N3F_V3F) ?  GL_T2F_C4F_N3F_V3F:
    (_iaformat == IA_T4F_C4F_N3F_V4F) ?  GL_T4F_C4F_N3F_V4F: 0;

    _iarray = ia;
    // note the size of _iaindex defaults 0, but will be recalculated
    // automatically by computeNumVerts().
    _iaindex.set(0,iai);

    set_fast_path();
}

void GeoSet::setInterleavedArray( InterleaveArrayType format, float *ia, IndexPointer& iai )
{
    _iaformat = format;

    _ogliaformat =
        (_iaformat == IA_OFF )            ? 0 :
    (_iaformat == IA_V2F )            ?  GL_V2F:
    (_iaformat == IA_V3F )            ?  GL_V3F:
    (_iaformat == IA_C4UB_V2F)        ?  GL_C4UB_V2F:
    (_iaformat == IA_C4UB_V3F)        ?  GL_C4UB_V3F:
    (_iaformat == IA_C3F_V3F)         ?  GL_C3F_V3F:
    (_iaformat == IA_N3F_V3F)         ?  GL_N3F_V3F:
    (_iaformat == IA_C4F_N3F_V3F)     ?  GL_C4F_N3F_V3F:
    (_iaformat == IA_T2F_V3F)         ?  GL_T2F_V3F:
    (_iaformat == IA_T4F_V4F)         ?  GL_T4F_V4F:
    (_iaformat == IA_T2F_C4UB_V3F)    ?  GL_T2F_C4UB_V3F:
    (_iaformat == IA_T2F_C3F_V3F)     ?  GL_T2F_C3F_V3F:
    (_iaformat == IA_T2F_N3F_V3F)     ?  GL_T2F_N3F_V3F:
    (_iaformat == IA_T2F_C4F_N3F_V3F) ?  GL_T2F_C4F_N3F_V3F:
    (_iaformat == IA_T4F_C4F_N3F_V4F) ?  GL_T4F_C4F_N3F_V4F: 0;

    _iarray = ia;
    // note the size of _iaindex defaults 0, but will be recalculated
    // automatically by computeNumVerts().
    _iaindex  = iai;

    set_fast_path();
}

void GeoSet::accept(AttributeFunctor& auf)
{
    if (_numcoords == 0) computeNumVerts();

    if (_coords && _numcoords)
    {
        auf.apply(VERTICES,_numcoords,_coords);
    }
    
    if (_normals && _numnormals)
    {
        auf.apply(NORMALS,_numnormals,_normals);
    }
    
    if (_colors && _numcolors)
    {
        auf.apply(COLORS,_numcolors,_colors);
    }
    
    if (_tcoords && _numtcoords)
    {
        auf.apply(TEXTURE_COORDS_0,_numtcoords,_tcoords);
    }
}

void GeoSet::accept(ConstAttributeFunctor& auf) const
{
    if (_numcoords == 0) computeNumVerts();

    if (_coords && _numcoords)
    {
        auf.apply(VERTICES,_numcoords,_coords);
    }
    
    if (_normals && _numnormals)
    {
        auf.apply(NORMALS,_numnormals,_normals);
    }
    
    if (_colors && _numcolors)
    {
        auf.apply(COLORS,_numcolors,_colors);
    }
    
    if (_tcoords && _numtcoords)
    {
        auf.apply(TEXTURE_COORDS_0,_numtcoords,_tcoords);
    }
}

void GeoSet::accept(PrimitiveFunctor& functor) const
{
    // will easily convert into a Geometry.

    if (!_coords || !_numcoords) return;

    functor.setVertexArray(_numcoords,_coords);

    if( _needprimlen )
    {
        // LINE_STRIP, LINE_LOOP, TRIANGLE_STRIP,
        // TRIANGLE_FAN, QUAD_STRIP, POLYGONS
        int index = 0;
        if( _primLengths == (int *)0 )
        {
            return;
        }

        for( int i = 0; i < _numprims; i++ )
        {
            if( _cindex.valid() )
            {

                if (_cindex._is_ushort)
                    functor.drawElements( (GLenum)_oglprimtype, _primLengths[i],&_cindex._ptr._ushort[index] );
                else
                    functor.drawElements( (GLenum)_oglprimtype, _primLengths[i],&_cindex._ptr._uint[index] );
            }
            else
                functor.drawArrays( (GLenum)_oglprimtype, index, _primLengths[i] );

            index += _primLengths[i];
        }
    }
    else                         // POINTS, LINES, TRIANGLES, QUADS
    {
        if( _cindex.valid())
        {
            if (_cindex._is_ushort)
                functor.drawElements( (GLenum)_oglprimtype, _cindex._size, _cindex._ptr._ushort );
            else
                functor.drawElements( (GLenum)_oglprimtype, _cindex._size, _cindex._ptr._uint );
        }
        else
            functor.drawArrays( (GLenum)_oglprimtype, 0, _numcoords );
    }
        
}

Geometry* GeoSet::convertToGeometry()
{
    set_fast_path();
    computeNumVerts();
    
    ref_ptr<Geometry> geom = new Geometry;
    geom->setStateSet(getStateSet());
    
    if (_flat_shaded_skip)
    {
        // will need to add flat shading to primitive.

        StateSet* stateset = geom->getOrCreateStateSet();
        ShadeModel* shademodel = dynamic_cast<ShadeModel*>(stateset->getAttribute(StateAttribute::SHADEMODEL));
        if (!shademodel)
        {
            shademodel = new osg::ShadeModel;
            stateset->setAttribute(shademodel);
        }
        shademodel->setMode( ShadeModel::FLAT );
    }

    switch(_normal_binding)
    {
        case(BIND_OFF):
            geom->setNormalBinding(Geometry::BIND_OFF);
            break;
        case(BIND_OVERALL):
            geom->setNormalBinding(Geometry::BIND_OVERALL);
            break;
        case(BIND_PERPRIM):
            geom->setNormalBinding(Geometry::BIND_PER_PRIMITIVE);
            break;
        case(BIND_PERVERTEX):
            geom->setNormalBinding(Geometry::BIND_PER_VERTEX);
            break;
        default:
            geom->setNormalBinding(Geometry::BIND_OFF);
            break;
    }

    switch(_color_binding)
    {
        case(BIND_OFF):
            geom->setColorBinding(Geometry::BIND_OFF);
            break;
        case(BIND_OVERALL):
            geom->setColorBinding(Geometry::BIND_OVERALL);
            break;
        case(BIND_PERPRIM):
            geom->setColorBinding(Geometry::BIND_PER_PRIMITIVE);
            break;
        case(BIND_PERVERTEX):
            geom->setColorBinding(Geometry::BIND_PER_VERTEX);
            break;
        default:
            geom->setColorBinding(Geometry::BIND_OFF);
            break;
    }

    if (_coords)
    {
        geom->setVertexArray(new Vec3Array(_numcoords,_coords));
        if (_cindex.valid())
        {
            if (_cindex._is_ushort)         geom->setVertexIndices(new UShortArray(_cindex._size,_cindex._ptr._ushort));
            else /* _nindex._is_uint*/      geom->setVertexIndices(new UIntArray(_cindex._size,_cindex._ptr._uint));
        }
    }
        
    if (_normals)
    {
        if (_flat_shaded_skip && _needprimlen && _normal_binding==BIND_PERVERTEX) 
        {
            if (_nindex.valid())
            {
                geom->setNormalArray(new Vec3Array(_numnormals,_normals));
                if (_nindex._is_ushort)
                {
                    UShortArray* indices = new UShortArray;
                    int index=0;
                    for(int primNo = 0; primNo<_numprims; ++primNo)
                    {
                        for (int i=0;i<_primLengths[primNo];++i)
                        {
                            indices->push_back(_nindex._ptr._ushort[index]);
                            if (i>=_flat_shaded_skip) ++index;
                        }                        
                    }
                    geom->setNormalIndices(indices);
                }
                else
                {
                    UIntArray* indices = new UIntArray;
                    int index=0;
                    for(int primNo = 0; primNo<_numprims; ++primNo)
                    {
                        for (int i=0;i<_primLengths[primNo];++i)
                        {
                            indices->push_back(_nindex._ptr._uint[index]);
                            if (i>=_flat_shaded_skip) ++index;
                        }                        
                    }
                    geom->setNormalIndices(indices);
                }
            }
            else
            {
                Vec3Array* normals = new Vec3Array;
                int index=0;
                for(int primNo = 0; primNo<_numprims; ++primNo)
                {
                    for (int i=0;i<_primLengths[primNo];++i)
                    {
                        normals->push_back(_normals[index]);
                        if (i>=_flat_shaded_skip) ++index;
                    }                        
                }
                geom->setNormalArray(normals);
            }
        }
        else
        {
            // usual path.
            geom->setNormalArray(new Vec3Array(_numnormals,_normals));
            if (_nindex.valid())
            {
                if (_nindex==_cindex)           geom->setNormalIndices(geom->getVertexIndices());
                else if (_nindex._is_ushort)    geom->setNormalIndices(new UShortArray(_nindex._size,_nindex._ptr._ushort));
                else /* _nindex._is_uint*/      geom->setNormalIndices(new UIntArray(_nindex._size,_nindex._ptr._uint));
            }
        }
    }
    
    if (_colors) 
    {
        if (_flat_shaded_skip && _needprimlen && _color_binding==BIND_PERVERTEX) 
        {
            if (_colindex.valid())
            {
                geom->setColorArray(new Vec4Array(_numcolors,_colors));
                if (_colindex==_nindex && _normal_binding==BIND_PERVERTEX)
                {
                    geom->setColorIndices(geom->getNormalIndices());
                }
                else if (_colindex._is_ushort)
                {
                    UShortArray* indices = new UShortArray;
                    int index=0;
                    for(int primNo = 0; primNo<_numprims; ++primNo)
                    {
                        for (int i=0;i<_primLengths[primNo];++i)
                        {
                            indices->push_back(_colindex._ptr._ushort[index]);
                            if (i>=_flat_shaded_skip) ++index;
                        }                        
                    }
                    geom->setColorIndices(indices);
                }
                else
                {
                    UIntArray* indices = new UIntArray;
                    int index=0;
                    for(int primNo = 0; primNo<_numprims; ++primNo)
                    {
                        for (int i=0;i<_primLengths[primNo];++i)
                        {
                            indices->push_back(_colindex._ptr._uint[index]);
                            if (i>=_flat_shaded_skip) ++index;
                        }                        
                    }
                    geom->setColorIndices(indices);
                }
            }
            else
            {
                Vec4Array* colors = new Vec4Array;
                int index=0;
                for(int primNo = 0; primNo<_numprims; ++primNo)
                {
                    for (int i=0;i<_primLengths[primNo];++i)
                    {
                        colors->push_back(_colors[index]);
                        if (i>=_flat_shaded_skip) ++index;
                    }                        
                }
                geom->setColorArray(colors);
            }

        }
        else
        {
            // usual path.
            geom->setColorArray(new Vec4Array(_numcolors,_colors));
            if (_colindex.valid())
            {
                if (_colindex==_cindex)         geom->setColorIndices(geom->getVertexIndices());
                else if (_colindex==_nindex)    geom->setColorIndices(geom->getNormalIndices());
                else if (_colindex._is_ushort)  geom->setColorIndices(new UShortArray(_colindex._size,_colindex._ptr._ushort));
                else /* _colindex._is_uint*/    geom->setColorIndices(new UIntArray(_colindex._size,_colindex._ptr._uint));
            }
        }
    }
    
    if (_tcoords)
    {
        geom->setTexCoordArray(0,new Vec2Array(_numtcoords,_tcoords));
        if (_tindex.valid())
        {
            if (_tindex==_cindex)           geom->setTexCoordIndices(0,geom->getVertexIndices());
            else if (_tindex==_nindex)      geom->setTexCoordIndices(0,geom->getNormalIndices());
            else if (_tindex==_colindex)    geom->setTexCoordIndices(0,geom->getColorIndices());
            else if (_tindex._is_ushort)    geom->setTexCoordIndices(0,new UShortArray(_tindex._size,_tindex._ptr._ushort));
            else /* _tindex._is_uint*/      geom->setTexCoordIndices(0,new UIntArray(_tindex._size,_tindex._ptr._uint));
        }
    }
    

    if (_needprimlen)
    {
        DrawArrayLengths* primitives = new DrawArrayLengths((GLenum)_oglprimtype);
        primitives->insert(primitives->end(), _primLengths, _primLengths+_numprims );
        geom->addPrimitiveSet(primitives);
    }
    else
    {
        geom->addPrimitiveSet(new DrawArrays((GLenum)_oglprimtype,0, _numcoords));
    }

    return geom.take();
    
        
}
