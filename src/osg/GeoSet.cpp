#if defined(_MSC_VER)
	#pragma warning( disable : 4786 )
#endif

#include <stdio.h>
#include <math.h>
#include <float.h>

#include <osg/GeoSet>
#include <osg/Notify>

//#include <osg/mem_ptr>

using namespace osg;

GeoSet::GeoSet()
{
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

    _normal_binding = BIND_OFF;
    _color_binding  = BIND_OFF;
    _texture_binding  = BIND_OFF;

    _fast_path = 1;

    //     cout << endl << "Above allocated"<<endl;
    //     newMemoryAdapter<Vec3>* ma = newMemoryAdapter<Vec3>::instance();
    //     mem_ptr<Vec3> coords(ma->allocate(100),ma);
    //     cout << "Registered"<<endl;mc

}


GeoSet::~GeoSet()
{
    // note all coordinates, colors, texture coordinates and normals
    // are not currently deleted, ahh!!!!  This issue needs to be
    // addressed.  However, since the data is simple passed in it
    // is unclear whether the data is shared, allocated with malloc,
    // or new [] so its difficult right now to now how to delete it
    // appropriatly.  Should data be copied rather than simply copying
    // pointers??  Using vector<> could simplify this issue. But then
    // would you want to share coords etc?
    // Robert Osfield, Decemeber 2000.
}


void GeoSet::setColorBinding( const BindingType binding )
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


void GeoSet::setNormalBinding( const BindingType binding )
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


void GeoSet::setTextureBinding( const BindingType binding )
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

void GeoSet::drawImmediateMode(State&)
{
    if( _coords == (Vec3 *)0 && _iaformat == IA_OFF ) return;

    // need to do this to get a valid _numcoords, _numindices & _primlength
    if( _numcoords == 0 )
        computeNumVerts();

    if( _fast_path )
        draw_fast_path();
    else
        draw_alternate_path();
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
            notify(WARN) << "Not supported primitive "<<(int)_primtype<<endl;
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


const bool GeoSet::computeBound() const
{
    if( _coords == (Vec3 *)0 ) return false;

    if( _numcoords == 0 )
    {
        computeNumVerts();
    }

    if( _numcoords == 0 )
        return false;

    Vec3 center(0.0f,0.0f,0.0f);

    int i;
    for( i = 0; i < _numcoords; i++ )
    {
        center += _coords[i];
    }

    center /= (float)_numcoords;

    _bbox.init();

    for( i = 0; i < _numcoords; i++ )
    {
        _bbox.expandBy(_coords[i]);
    }

    _bbox_computed=true;
    
    return true;
}

const bool GeoSet::check() const
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


void GeoSet::setPrimType( const PrimitiveType type )
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
    _bbox_computed = false;
    set_fast_path();
}


void GeoSet::setCoords( Vec3 *cp, ushort *ci )
{
    _coords = cp;
    // note the size of cindex defaults 0, but will be recalculated
    // automatically by computeNumVerts().
    _cindex.set(0,ci);
    _bbox_computed = false;
    set_fast_path();
}


void GeoSet::setCoords( Vec3 *cp, uint *ci )
{
    _coords = cp;
    // note the size of cindex defaults 0, but will be recalculated
    // automatically by computeNumVerts().
    _cindex.set(0,ci);
    _bbox_computed = false;
    set_fast_path();
}

void GeoSet::setCoords( Vec3 *cp, IndexPointer& ip )
{
    _coords = cp;
    _cindex = ip;
    _bbox_computed = false;
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


void GeoSet::setNormals( Vec3 *np, ushort *ni )
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

void GeoSet::setNormals( Vec3 *np, uint *ni )
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


void GeoSet::setColors( Vec4 *lp, ushort *coli )
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

void GeoSet::setColors( Vec4 *lp, uint *coli )
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


void GeoSet::setTextureCoords( Vec2 *tc, ushort *ti )
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

void GeoSet::setTextureCoords( Vec2 *tc, uint *ti )
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

void GeoSet::setInterleavedArray( const InterleaveArrayType format, float *pointer )
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


void GeoSet::setInterleavedArray( const InterleaveArrayType format, float *ia, ushort *iai )
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

void GeoSet::setInterleavedArray( const InterleaveArrayType format, float *ia, IndexPointer& iai )
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
 
Drawable::AttributeBitMask GeoSet::suppportsAttributeOperation() const
{
    // we do support coords,normals,texcoords and colors so return true.
    return COORDS | NORMALS | COLORS | TEXTURE_COORDS;
}

Drawable::AttributeBitMask GeoSet::applyAttributeOperation(AttributeFunctor& auf)
{
    if (_numcoords == 0) computeNumVerts();

    AttributeBitMask amb = auf.getAttributeBitMask();
    AttributeBitMask ramb = 0;
    
    if ((amb & COORDS) && _coords && _numcoords)
    {
//        cout << "number of coords = "<<_numcoords<<endl;
//        cout << "   _coords = "<<_coords<<"  _coords+_numcoords="<<_coords+_numcoords<<endl;
        if (auf.apply(COORDS,_coords,_coords+_numcoords)) ramb = COORDS;
    }
    
    if ((amb & NORMALS) && _normals && _numnormals)
    {
//        cout << "number of normals = "<<_numnormals<<endl;
//        cout << "   _normals = "<<_normals<<"  _normals+_numnormals="<<_normals+_numnormals<<endl;
        if (auf.apply(NORMALS,_normals,_normals+_numnormals)) ramb = NORMALS;
    }
    
    if ((amb & COLORS) && _colors && _numcolors)
    {
        if (auf.apply(COLORS,_colors,_colors+_numcolors)) ramb = COLORS;
    }
    
    if ((amb & TEXTURE_COORDS) && _tcoords && _numtcoords)
    {
        if (auf.apply(TEXTURE_COORDS,_tcoords,_tcoords+_numtcoords)) ramb = TEXTURE_COORDS;
    }
    
    return ramb;
}
