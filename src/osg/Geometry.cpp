#include <osg/Geometry>

using namespace osg;

Geometry::Geometry()
{
}

Geometry::Geometry(const Geometry& geometry,const CopyOp& copyop):
    Drawable(geometry,copyop),
    _coords(geometry._coords),
    _coordIndices(geometry._coordIndices),
    _normals(geometry._normals),
    _normalIndices(geometry._normalIndices),
    _colors(geometry._colors),
    _colorIndices(geometry._colorIndices),
    _texCoordList(geometry._texCoordList),
    _texCoordIndicesList(geometry._texCoordIndicesList)
{
}

Geometry::~Geometry()
{
    // no need to delete, all automatically handled by ref_ptr :-)
}

void Geometry::setTexCoordArray(unsigned int pos,AttributeArray* array)
{
    if (_texCoordList.size()<=pos)
        _texCoordList.resize(pos+1,0);
        
   _texCoordList[pos] = array;
}

AttributeArray* Geometry::getTexCoordArray(unsigned int pos)
{
    if (pos<_texCoordList.size()) return _texCoordList[pos].get();
    else return 0;
}

void Geometry::setTexCoordIndicesArray(unsigned int pos,AttributeArray* array)
{
    if (_texCoordList.size()<=pos)
        _texCoordList.resize(pos+1,0);
        
   _texCoordIndicesList[pos] = array;
}

AttributeArray* Geometry::getTexCoordIndicesArray(unsigned int pos)
{
    if (pos<_texCoordIndicesList.size()) return _texCoordIndicesList[pos].get();
    else return 0;
}

void Geometry::setAttribute(AttributeType type,AttributeArray* array)
{
    switch(type)
    {
    case(PRIMITIVES):
        _primitives = array;
        break;
    case(COORDS):
        _coords = array;
        break;
    case(NORMALS):
        _normals = array;
        break;
    case(COLORS):
        _colors = array;
        break;
    default:
        if (type>=TEX_COORDS_0)
        {
            setTexCoordArray(type-TEX_COORDS_0,array);
        }
        break;
    }
}


void Geometry::setAttribute(AttributeType type,AttributeArray* array,AttributeArray* indices)
{
    switch(type)
    {
    case(PRIMITIVES):
        _primitives = array;
        // indices not appropriate!
        break;
    case(COORDS):
        _coords = array;
        _coordIndices = indices;
        break;
    case(NORMALS):
        _normals = array;
        _normalIndices = indices;
        break;
    case(COLORS):
        _colors = array;
        _colorIndices = indices;
        break;
    default:
        if (type>=TEX_COORDS_0)
        {
            setTexCoordArray(type-TEX_COORDS_0,array);
            setTexCoordIndicesArray(type-TEX_COORDS_0,indices);
        }
        break;
    }
}


AttributeArray* Geometry::getAttribute(AttributeType type)
{
    switch(type)
    {
    case(PRIMITIVES):
        return _primitives.get();
        break;
    case(COORDS):
        return _coords.get();
        break;
    case(NORMALS):
        return _normals.get();
        break;
    case(COLORS):
        return _colors.get();
        break;
    default:
        if (type>=TEX_COORDS_0)
        {
            return getTexCoordArray(type-TEX_COORDS_0);
        }
        break;
    }
    return 0;
}


void Geometry::setIndices(AttributeType type,AttributeArray* indices)
{
    switch(type)
    {
    case(PRIMITIVES):
        // indices not appropriate!
        break;
    case(COORDS):
        _coordIndices = indices;
        break;
    case(NORMALS):
        _normalIndices = indices;
        break;
    case(COLORS):
        _colorIndices = indices;
        break;
    default:
        if (type>=TEX_COORDS_0)
        {
            setTexCoordIndicesArray(type-TEX_COORDS_0,indices);
        }
        break;
    }
}


AttributeArray* Geometry::getIndices(AttributeType type)
{
    switch(type)
    {
    case(COORDS):
        return _coordIndices.get();
        break;
    case(NORMALS):
        return _normalIndices.get();
        break;
    case(COLORS):
        return _colorIndices.get();
        break;
    default:
        if (type>=TEX_COORDS_0)
        {
            return getTexCoordIndicesArray(type-TEX_COORDS_0);
        }
        break;
    }
    return 0;
}


void Geometry::drawImmediateMode(State& /*state*/)
{
}

/** Statistics collection for each drawable- 26.09.01
 */
bool Geometry::getStats(Statistics &)
{
    return false;
}

Drawable::AttributeBitMask Geometry::suppportsAttributeOperation() const
{
    return 0;
}


/** return the attributes successully applied in applyAttributeUpdate.*/
Drawable::AttributeBitMask Geometry::applyAttributeOperation(AttributeFunctor& )
{
    return 0;
}

const bool Geometry::computeBound() const
{
    _bbox.init();
    
    const Vec3Array* coords = dynamic_cast<const Vec3Array*>(_coords.get());
    if (coords)
    {
        for(Vec3Array::const_iterator itr=coords->begin();
            itr!=coords->end();
            ++itr)
        {
            _bbox.expandBy(*itr);
        }
    }
    _bbox_computed = true;
    
    return _bbox.valid();
}

