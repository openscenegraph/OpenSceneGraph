#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <math.h>
#include <float.h>

#include <osg/GeoSet>
#include <osg/Notify>
#include <osg/Statistics>

//#include <osg/mem_ptr>

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

    _normal_binding = BIND_OFF;
    _color_binding  = BIND_OFF;
    _texture_binding  = BIND_OFF;

    _fast_path = 1;

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
    std::set<osg::ushort*> ushortList;
    std::set<osg::uint*>   uintList;
    
    INDEX_ARRAY_DELETE(gset->getCoordIndices())
    INDEX_ARRAY_DELETE(gset->getNormalIndices());
    INDEX_ARRAY_DELETE(gset->getColorIndices());
    INDEX_ARRAY_DELETE(gset->getTextureIndices())
    INDEX_ARRAY_DELETE(gset->getInterleavedIndices());
    
    for(std::set<osg::ushort*>::iterator sitr=ushortList.begin();
        sitr!=ushortList.end();
        ++sitr)
    {
        delete [] *sitr;
    }
    
    for(std::set<osg::uint*>::iterator iitr=uintList.begin();
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


const bool GeoSet::computeBound() const
{
    if( _iaformat == IA_OFF && _coords == (Vec3 *)0 ) return false;

    if( _numcoords == 0 )
    {
        computeNumVerts();
    }

    if( _numcoords == 0 )
        return false;

    Vec3 center(0.0f,0.0f,0.0f);
    _bbox.init();

    int i;
    if( _iaformat == IA_OFF )
    {
        for( i = 0; i < _numcoords; i++ )
        {
            center += _coords[i];
            _bbox.expandBy(_coords[i]);
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
        if (auf.apply(COORDS,_coords,_coords+_numcoords)) ramb = COORDS;
    }
    
    if ((amb & NORMALS) && _normals && _numnormals)
    {
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

bool GeoSet::getStats(Statistics &stat)
{ // analyse the drawable GeoSet
    const int np=getNumPrims(); // number of primitives in this geoset
    stat.addNumPrims(np);
    const int type=getPrimType();
    int nprimlens=0, numprimtypes=0, primverts=0; 
    switch (type) {
    case    osg::GeoSet::POINTS:
    case    osg::GeoSet::LINES:
    case    osg::GeoSet::LINE_STRIP:
    case    osg::GeoSet::FLAT_LINE_STRIP:
    case    osg::GeoSet::LINE_LOOP:
    case    osg::GeoSet::TRIANGLE_STRIP:
    case    osg::GeoSet::FLAT_TRIANGLE_STRIP:
    case    osg::GeoSet::TRIANGLE_FAN:
    case    osg::GeoSet::FLAT_TRIANGLE_FAN:
    case    osg::GeoSet::QUAD_STRIP:
    case    osg::GeoSet::POLYGON:
        break;
    case    osg::GeoSet::TRIANGLES: // should not have any lengths for tris & quads
        nprimlens=np;
        numprimtypes=np;
        primverts=3*np;
        break;
    case    osg::GeoSet::QUADS:
        nprimlens=np*2;
        primverts=4*np;
        numprimtypes=np;
        break;
    case    osg::GeoSet::NO_TYPE:
    default:
        break;
    }
    // now count the lengths, ie efficiency of triangulation
    const int *lens=getPrimLengths(); // primitive lengths
    if (lens) { // then count for each length
        for (int i=0; i<np; i++) {
            switch (type) {
            case    osg::GeoSet::POINTS:
            case    osg::GeoSet::LINES:
            case    osg::GeoSet::LINE_STRIP:
            case    osg::GeoSet::FLAT_LINE_STRIP:
            case    osg::GeoSet::LINE_LOOP:
            case    osg::GeoSet::TRIANGLES: // should not have any lengths for tris & quads
            case    osg::GeoSet::QUADS:
            case    osg::GeoSet::QUAD_STRIP:
                nprimlens+=lens[i];
                break;
            case    osg::GeoSet::POLYGON: // moved Nov 2001 to count triangles cf lengths
            case    osg::GeoSet::TRIANGLE_STRIP:
            case    osg::GeoSet::FLAT_TRIANGLE_STRIP:
            case    osg::GeoSet::TRIANGLE_FAN:
            case    osg::GeoSet::FLAT_TRIANGLE_FAN:
                nprimlens+=lens[i]-2; // tri strips & fans create lens[i]-2 triangles
                break;
            case    osg::GeoSet::NO_TYPE:
            default:
                break;
            }
            switch (type) {
            case    osg::GeoSet::POINTS:
            case    osg::GeoSet::LINES:
            case    osg::GeoSet::LINE_STRIP:
            case    osg::GeoSet::FLAT_LINE_STRIP:
            case    osg::GeoSet::LINE_LOOP:
            case    osg::GeoSet::TRIANGLES:
            case    osg::GeoSet::QUADS:
            case    osg::GeoSet::TRIANGLE_STRIP:
            case    osg::GeoSet::FLAT_TRIANGLE_STRIP:
            case    osg::GeoSet::TRIANGLE_FAN:
            case    osg::GeoSet::FLAT_TRIANGLE_FAN:
            case    osg::GeoSet::QUAD_STRIP:
            case    osg::GeoSet::POLYGON:
                numprimtypes++;
                primverts+=lens[i];
                break;
            case    osg::GeoSet::NO_TYPE:
            default:
                break;
            }
        }
    } else { // no lengths - num prims is the length of a point set
        switch (type) {
        case    osg::GeoSet::POINTS:
            numprimtypes++;
            primverts+=np;
            break;
        }
    }
    stat.addNumPrims(type, nprimlens, numprimtypes, primverts);
    return true;
}
