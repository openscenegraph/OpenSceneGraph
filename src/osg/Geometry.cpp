/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
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
#include <osg/Geometry>
#include <osg/Notify>

using namespace osg;

const Geometry::ArrayData Geometry::s_InvalidArrayData;

#if 1
class DrawVertex
{
    public:
    
        DrawVertex(const Array* vertices,const IndexArray* indices):
            _vertices(vertices),
            _indices(indices)
        {
            _verticesType = _vertices?_vertices->getType():Array::ArrayType;
            _indicesType = _indices?_indices->getType():Array::ArrayType;
        }
    
        inline unsigned int index(unsigned int pos)
        {
            switch(_indicesType)
            {
            case(Array::ByteArrayType): return (*static_cast<const ByteArray*>(_indices))[pos];
            case(Array::ShortArrayType): return (*static_cast<const ShortArray*>(_indices))[pos];
            case(Array::IntArrayType): return (*static_cast<const IntArray*>(_indices))[pos];
            case(Array::UByteArrayType): return (*static_cast<const UByteArray*>(_indices))[pos];
            case(Array::UShortArrayType): return (*static_cast<const UShortArray*>(_indices))[pos];
            case(Array::UIntArrayType): return (*static_cast<const UIntArray*>(_indices))[pos];
            default: return 0;
            }
        }

        inline void operator () (unsigned int pos)
        {
            if (_indices) pos = index(pos);

            switch(_verticesType)
            {
            case(Array::Vec3ArrayType): 
                apply((*(static_cast<const Vec3Array*>(_vertices)))[pos]);
                break;
            case(Array::Vec2ArrayType): 
                apply((*(static_cast<const Vec2Array*>(_vertices)))[pos]);
                break;
            case(Array::Vec4ArrayType): 
                apply((*(static_cast<const Vec4Array*>(_vertices)))[pos]);
                break;
            default:
                break;
            }
            
        }
        
        inline void apply(const Vec2& v)   { glVertex2fv(v.ptr()); }
        inline void apply(const Vec3& v)   { glVertex3fv(v.ptr()); }
        inline void apply(const Vec4& v)   { glVertex4fv(v.ptr()); }

        const Array*        _vertices;
        const IndexArray*   _indices;
        Array::Type         _verticesType;
        Array::Type         _indicesType;
};
#else
class DrawVertex : public osg::ConstValueVisitor
{
    public:
    
        DrawVertex(const Array* vertices,const IndexArray* indices):
            _vertices(vertices),
            _indices(indices) {}
    
        inline void operator () (unsigned int pos)
        {
            if (_indices) _vertices->accept(_indices->index(pos),*this);
            else _vertices->accept(pos,*this);
        }
        
        virtual void apply(const Vec2& v)   { glVertex2fv(v.ptr()); }
        virtual void apply(const Vec3& v)   { glVertex3fv(v.ptr()); }
        virtual void apply(const Vec4& v)   { glVertex4fv(v.ptr()); }

        const Array*        _vertices;
        const IndexArray*   _indices;
};
#endif

class DrawNormal
{
    public:
    
        DrawNormal(const Array* normals,const IndexArray* indices):
            _normals(normals),
            _indices(indices) 
        {
            _normalsType = normals?normals->getType():Array::ArrayType;
        }
    
        void operator () (unsigned int pos)
        {
            switch(_normalsType)
            {
                case (Array::Vec3ArrayType):
                    {
                        const Vec3Array& normals = *static_cast<const Vec3Array*>(_normals);
                        if (_indices) glNormal3fv(normals[_indices->index(pos)].ptr());
                        else glNormal3fv(normals[pos].ptr());
                    }
                    break;
                case (Array::Vec3sArrayType):
                    {
                        const Vec3sArray& normals = *static_cast<const Vec3sArray*>(_normals);
                        if (_indices) glNormal3sv(normals[_indices->index(pos)].ptr());
                        else glNormal3sv(normals[pos].ptr());
                    }
                    break;
                case (Array::Vec4sArrayType):
                    {
                        const Vec4sArray& normals = *static_cast<const Vec4sArray*>(_normals);
                        if (_indices) glNormal3sv(normals[_indices->index(pos)].ptr());
                        else glNormal3sv(normals[pos].ptr());
                    }
                    break;
                case (Array::Vec3bArrayType):
                    {
                        const Vec3bArray& normals = *static_cast<const Vec3bArray*>(_normals);
                        if (_indices) glNormal3bv((const GLbyte*)normals[_indices->index(pos)].ptr());
                        else glNormal3bv((const GLbyte*)normals[pos].ptr());
                    }
                    break;
                case (Array::Vec4bArrayType):
                    {
                        const Vec4bArray& normals = *static_cast<const Vec4bArray*>(_normals);
                        if (_indices) glNormal3bv((const GLbyte*)normals[_indices->index(pos)].ptr());
                        else glNormal3bv((const GLbyte*)normals[pos].ptr());
                    }
                    break;
                default:
                    break;
                    
            }
        }
        
        const Array*       _normals;
        const IndexArray*  _indices;
        Array::Type        _normalsType;
};

#if 1
class DrawColor
{
    public:

        DrawColor(const Array* colors,const IndexArray* indices):
            _colors(colors),
            _indices(indices)
        {
            _colorsType = _colors?_colors->getType():Array::ArrayType;
            _indicesType = _indices?_indices->getType():Array::ArrayType;
        }

        inline unsigned int index(unsigned int pos)
        {
            switch(_indicesType)
            {
            case(Array::ByteArrayType): return (*static_cast<const ByteArray*>(_indices))[pos];
            case(Array::ShortArrayType): return (*static_cast<const ShortArray*>(_indices))[pos];
            case(Array::IntArrayType): return (*static_cast<const IntArray*>(_indices))[pos];
            case(Array::UByteArrayType): return (*static_cast<const UByteArray*>(_indices))[pos];
            case(Array::UShortArrayType): return (*static_cast<const UShortArray*>(_indices))[pos];
            case(Array::UIntArrayType): return (*static_cast<const UIntArray*>(_indices))[pos];
            default: return 0;
            }
        }

        inline void operator () (unsigned int pos)
        {
            if (_indices) pos = index(pos);

            switch(_colorsType)
            {
            case(Array::Vec4ArrayType):
                apply((*static_cast<const Vec4Array*>(_colors))[pos]);
                break;
            case(Array::Vec4ubArrayType):
                apply((*static_cast<const Vec4ubArray*>(_colors))[pos]);
                break;
            case(Array::Vec3ArrayType):
                apply((*static_cast<const Vec3Array*>(_colors))[pos]);
                break;
            default:
                break;
            }
        }

        inline void apply(const Vec4ub& v) { glColor4ubv(v.ptr()); }
        inline void apply(const Vec3& v)   { glColor3fv(v.ptr()); }
        inline void apply(const Vec4& v)   { glColor4fv(v.ptr()); }
        
        const Array*        _colors;
        const IndexArray*   _indices;
        Array::Type         _colorsType;
        Array::Type         _indicesType;
};
#else
class DrawColor : public osg::ConstValueVisitor
{
    public:

        DrawColor(const Array* colors,const IndexArray* indices):
            _colors(colors),
            _indices(indices) {}

        inline void operator () (unsigned int pos)
        {
            if (_indices) _colors->accept(_indices->index(pos),*this);
            else _colors->accept(pos,*this);
        }

        virtual void apply(const UVec4b& v) { glColor4ubv(v.ptr()); }
        virtual void apply(const Vec3& v)   { glColor3fv(v.ptr()); }
        virtual void apply(const Vec4& v)   { glColor4fv(v.ptr()); }
        
        const Array*        _colors;
        const IndexArray*   _indices;
};
#endif
class DrawVertexAttrib : public osg::Referenced, public osg::ConstValueVisitor
{
public:

    DrawVertexAttrib(const Drawable::Extensions * extensions,unsigned int vertAttribIndex,GLboolean normalized,const Array* attribcoords,const IndexArray* indices):    
            _vertAttribIndex(vertAttribIndex),
            _normalized(normalized),
            _extensions(extensions),
            _attribcoords(attribcoords),
            _indices(indices),
            _index(0) {;}
            
    inline void applyAndIncrement() { operator()(_index++); }

    inline void operator () (unsigned int pos)
    {
        if (_indices) _attribcoords->accept(_indices->index(pos),*this);
        else _attribcoords->accept(pos,*this);
    }

    virtual void apply(const GLshort& s) 
    {
        _extensions->glVertexAttrib1s( _vertAttribIndex, s );
    }
    virtual void apply(const GLfloat& f) 
    {
        _extensions->glVertexAttrib1f( _vertAttribIndex, f );
    }
    virtual void apply(const Vec4ub& v) 
    {
        if( _normalized )
        {
            _extensions->glVertexAttrib4Nubv( _vertAttribIndex, v.ptr() );
        }
        else
        {
            _extensions->glVertexAttrib4ubv( _vertAttribIndex, v.ptr() );
        }
    }
    virtual void apply(const Vec2& v) 
    {
        _extensions->glVertexAttrib2fv( _vertAttribIndex, v.ptr() );
    }
    virtual void apply(const Vec3& v) 
    {
        _extensions->glVertexAttrib3fv( _vertAttribIndex, v.ptr() );
    }
    virtual void apply(const Vec4& v) 
    {
        _extensions->glVertexAttrib4fv( _vertAttribIndex, v.ptr() );
    }

    unsigned int                    _vertAttribIndex;
    GLboolean                       _normalized;
    const Drawable::Extensions*     _extensions;
    const Array*                    _attribcoords;
    const IndexArray*               _indices;
    unsigned int                    _index;
};

class DrawTexCoord : public osg::Referenced, public osg::ConstValueVisitor
{
    public:

        DrawTexCoord(const Array* texcoords,const IndexArray* indices):
            _texcoords(texcoords),
            _indices(indices) {}

        inline void operator () (unsigned int pos)
        {
            if (_indices) _texcoords->accept(_indices->index(pos),*this);
            else _texcoords->accept(pos,*this);
        }

        virtual void apply(const GLfloat& v){ glTexCoord1f(v); }
        virtual void apply(const Vec2& v)   { glTexCoord2fv(v.ptr()); }
        virtual void apply(const Vec3& v)   { glTexCoord3fv(v.ptr()); }
        virtual void apply(const Vec4& v)   { glTexCoord4fv(v.ptr()); }

        const Array*        _texcoords;
        const IndexArray*   _indices;
};

class DrawMultiTexCoord : public osg::Referenced, public osg::ConstValueVisitor
{
    public:
    
        DrawMultiTexCoord(GLenum target,const Array* texcoords,const IndexArray* indices,
            const Drawable::Extensions * extensions):
            _target(target),
            _texcoords(texcoords),
            _indices(indices),
            _extensions(extensions) {}

        inline void operator () (unsigned int pos)
        {
            if (_indices) _texcoords->accept(_indices->index(pos),*this);
            else _texcoords->accept(pos,*this);
        }

        virtual void apply(const GLfloat& v){ _extensions->glMultiTexCoord1f(_target,v); }
        virtual void apply(const Vec2& v)   { _extensions->glMultiTexCoord2fv(_target,v.ptr()); }
        virtual void apply(const Vec3& v)   { _extensions->glMultiTexCoord3fv(_target,v.ptr()); }
        virtual void apply(const Vec4& v)   { _extensions->glMultiTexCoord4fv(_target,v.ptr()); }
        
        GLenum _target;
        const Array*        _texcoords;
        const IndexArray*   _indices;

        const Drawable::Extensions * _extensions;
};


class DrawSecondaryColor : public osg::ConstValueVisitor
{
    public:
    
        DrawSecondaryColor(const Array* colors,const IndexArray* indices,
                           const Drawable::Extensions * extensions):
            _colors(colors),
            _indices(indices),
            _extensions(extensions)
            {}
    
        inline void operator () (unsigned int pos)
        {
            if (_indices) _colors->accept(_indices->index(pos),*this);
            else _colors->accept(pos,*this);
        }

        virtual void apply(const Vec4ub& v) { _extensions->glSecondaryColor3ubv(v.ptr()); }
        virtual void apply(const Vec3& v)   { _extensions->glSecondaryColor3fv(v.ptr()); }
        virtual void apply(const Vec4& v)   { _extensions->glSecondaryColor3fv(v.ptr()); }

        const Array*        _colors;
        const IndexArray*   _indices;

        const Drawable::Extensions * _extensions;
};

class DrawFogCoord : public osg::ConstValueVisitor
{
    public:
    
        DrawFogCoord(const Array* fogcoords,const IndexArray* indices,const Drawable::Extensions * extensions):
            _fogcoords(fogcoords),
            _indices(indices),
            _extensions(extensions) {}
    
        inline void operator () (unsigned int pos)
        {
            if (_indices) _fogcoords->accept(_indices->index(pos),*this);
            else _fogcoords->accept(pos,*this);
        }

        virtual void apply(const GLfloat& v) { _extensions->glFogCoordfv(&v); }

        const Array*        _fogcoords;
        const IndexArray*   _indices;

        const Drawable::Extensions * _extensions;
};

Geometry::ArrayData::ArrayData(const ArrayData& data,const CopyOp& copyop):
    array(copyop(data.array.get())),
    indices(dynamic_cast<osg::IndexArray*>(copyop(data.indices.get()))),
    binding(data.binding),
    normalize(data.normalize),
    offset(data.offset)
{
}

Geometry::Vec3ArrayData::Vec3ArrayData(const Vec3ArrayData& data,const CopyOp& copyop):
    array(dynamic_cast<osg::Vec3Array*>(copyop(data.array.get()))),
    indices(dynamic_cast<osg::IndexArray*>(copyop(data.indices.get()))),
    binding(data.binding),
    normalize(data.normalize),
    offset(data.offset)
{
}

Geometry::Geometry()
{
    _fastPath = false;
    _fastPathHint = true;
}

Geometry::Geometry(const Geometry& geometry,const CopyOp& copyop):
    Drawable(geometry,copyop),
    _vertexData(geometry._vertexData,copyop),
    _normalData(geometry._normalData,copyop),
    _colorData(geometry._colorData,copyop),
    _secondaryColorData(geometry._secondaryColorData,copyop),
    _fogCoordData(geometry._fogCoordData,copyop),
    _fastPath(geometry._fastPath),
    _fastPathHint(geometry._fastPathHint)
{
    for(PrimitiveSetList::const_iterator pitr=geometry._primitives.begin();
        pitr!=geometry._primitives.end();
        ++pitr)
    {
        PrimitiveSet* primitive = copyop(pitr->get());
        if (primitive) _primitives.push_back(primitive);
    }

    for(ArrayList::const_iterator titr=geometry._texCoordList.begin();
        titr!=geometry._texCoordList.end();
        ++titr)
    {
        _texCoordList.push_back(*titr);
    }

    for(ArrayList::const_iterator vitr=geometry._vertexAttribList.begin();
        vitr!=geometry._vertexAttribList.end();
        ++vitr)
    {
        _vertexAttribList.push_back(*vitr);
    }
}

Geometry::~Geometry()
{
    // do dirty here to keep the getGLObjectSizeHint() estimate on the ball
    dirtyDisplayList();
    
    // no need to delete, all automatically handled by ref_ptr :-)
}

bool Geometry::empty() const
{
    if (!_primitives.empty()) return false;
    if (!_vertexData.empty()) return false;
    if (!_normalData.empty()) return false;
    if (!_colorData.empty()) return false;
    if (!_secondaryColorData.empty()) return false;
    if (!_fogCoordData.empty()) return false;
    if (!_texCoordList.empty()) return false;
    if (!_vertexAttribList.empty()) return false;
    return true;
}

void Geometry::setNormalBinding(AttributeBinding ab) 
{
    if (_normalData.binding == ab) return;
    
    _normalData.binding = ab;
    computeFastPathsUsed();
    dirtyDisplayList(); 
}

void Geometry::setColorBinding(AttributeBinding ab)
{
    if (_colorData.binding == ab) return;

    _colorData.binding = ab;
    computeFastPathsUsed();
    dirtyDisplayList(); 
}

void Geometry::setSecondaryColorBinding(AttributeBinding ab)
{
    if (_secondaryColorData.binding == ab) return;

    _secondaryColorData.binding = ab;
    computeFastPathsUsed();
    dirtyDisplayList(); 
}

void Geometry::setFogCoordBinding(AttributeBinding ab)
{
    if (_fogCoordData.binding == ab) return;

    _fogCoordData.binding = ab;
    computeFastPathsUsed();
    dirtyDisplayList(); 
}

void Geometry::setTexCoordData(unsigned int unit,const ArrayData& arrayData)
{
    if (_texCoordList.size()<=unit)
        _texCoordList.resize(unit+1);
    
    _texCoordList[unit] = arrayData;
}

Geometry::ArrayData& Geometry::getTexCoordData(unsigned int unit)
{
    if (_texCoordList.size()<=unit)
        _texCoordList.resize(unit+1);
        
    return _texCoordList[unit];
}

const Geometry::ArrayData& Geometry::getTexCoordData(unsigned int unit) const
{
    if (_texCoordList.size()<=unit)
        return s_InvalidArrayData;
        
    return _texCoordList[unit];
}

void Geometry::setTexCoordArray(unsigned int unit,Array* array)
{
    getTexCoordData(unit).binding = BIND_PER_VERTEX;
    getTexCoordData(unit).array = array;

    computeFastPathsUsed();
    dirtyDisplayList();
}

Array* Geometry::getTexCoordArray(unsigned int unit)
{
    if (unit<_texCoordList.size()) return _texCoordList[unit].array.get();
    else return 0;
}

const Array* Geometry::getTexCoordArray(unsigned int unit) const
{
    if (unit<_texCoordList.size()) return _texCoordList[unit].array.get();
    else return 0;
}

void Geometry::setTexCoordIndices(unsigned int unit,IndexArray* array)
{
    getTexCoordData(unit).indices = array;

    computeFastPathsUsed();
    dirtyDisplayList();
}

IndexArray* Geometry::getTexCoordIndices(unsigned int unit)
{
    if (unit<_texCoordList.size()) return _texCoordList[unit].indices.get();
    else return 0;
}

const IndexArray* Geometry::getTexCoordIndices(unsigned int unit) const
{
    if (unit<_texCoordList.size()) return _texCoordList[unit].indices.get();
    else return 0;
}


void Geometry::setVertexAttribData(unsigned int index, const Geometry::ArrayData& attrData)
{
    if (_vertexAttribList.size()<=index)
        _vertexAttribList.resize(index+1);
        
    _vertexAttribList[index] = attrData;

    computeFastPathsUsed();
    dirtyDisplayList();
}

Geometry::ArrayData& Geometry::getVertexAttribData(unsigned int index)
{
    if (_vertexAttribList.size()<=index)
        _vertexAttribList.resize(index+1);
        
    return _vertexAttribList[index];
}

const Geometry::ArrayData& Geometry::getVertexAttribData(unsigned int index) const
{
    if (_vertexAttribList.size()<=_vertexAttribList.size())
        return s_InvalidArrayData;
        
    return _vertexAttribList[index];
}

void Geometry::setVertexAttribArray(unsigned int index, Array* array)
{
    getVertexAttribData(index).array = array;

    computeFastPathsUsed();
    dirtyDisplayList();
}

Array *Geometry::getVertexAttribArray(unsigned int index)
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].array.get();
    else return 0;
}

const Array *Geometry::getVertexAttribArray(unsigned int index) const
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].array.get();
    else return 0;
}

void Geometry::setVertexAttribIndices(unsigned int index,IndexArray* array)
{
    getVertexAttribData(index).indices = array;

    computeFastPathsUsed();
    dirtyDisplayList();
}

IndexArray* Geometry::getVertexAttribIndices(unsigned int index)
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].indices.get();
    else return 0;
}

const IndexArray* Geometry::getVertexAttribIndices(unsigned int index) const
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].indices.get();
    else return 0;
}

void Geometry::setVertexAttribBinding(unsigned int index,AttributeBinding ab)
{
    if (getVertexAttribData(index).binding == ab)
       return;
    getVertexAttribData(index).binding = ab;
    computeFastPathsUsed();
    dirtyDisplayList();
}

Geometry::AttributeBinding Geometry::getVertexAttribBinding(unsigned int index) const
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].binding;
    else return BIND_OFF;
}

void Geometry::setVertexAttribNormalize(unsigned int index,GLboolean norm)
{
    getVertexAttribData(index).normalize = norm;

    dirtyDisplayList();
}

GLboolean Geometry::getVertexAttribNormalize(unsigned int index) const
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].normalize;
    else return GL_FALSE;
}

bool Geometry::addPrimitiveSet(PrimitiveSet* primitiveset)
{
    if (primitiveset)
    {
        _primitives.push_back(primitiveset);
        dirtyDisplayList();
        dirtyBound();
        return true;
    }
    notify(WARN)<<"Warning: invalid index i or primitiveset passed to osg::Geometry::addPrimitiveSet(i,primitiveset), ignoring call."<<std::endl;
    return false;
}

bool Geometry::setPrimitiveSet(unsigned int i,PrimitiveSet* primitiveset)
{
    if (i<_primitives.size() && primitiveset)
    {
        _primitives[i] = primitiveset;
        dirtyDisplayList();
        dirtyBound();
        return true;
    }
    notify(WARN)<<"Warning: invalid index i or primitiveset passed to osg::Geometry::setPrimitiveSet(i,primitiveset), ignoring call."<<std::endl;
    return false;
}

bool Geometry::insertPrimitiveSet(unsigned int i,PrimitiveSet* primitiveset)
{
    if (primitiveset)
    {
        if (i<_primitives.size())
        {
            _primitives.insert(_primitives.begin()+i,primitiveset);
            dirtyDisplayList();
            dirtyBound();
            return true;
        }
        else if (i==_primitives.size())
        {
            return addPrimitiveSet(primitiveset);
        }
    }
    notify(WARN)<<"Warning: invalid index i or primitiveset passed to osg::Geometry::insertPrimitiveSet(i,primitiveset), ignoring call."<<std::endl;
    return false;
}

bool Geometry::removePrimitiveSet(unsigned int i, unsigned int numElementsToRemove)
{
    if (i<_primitives.size() && numElementsToRemove>0)
    {
        if (i+numElementsToRemove<=_primitives.size())
        {
            _primitives.erase(_primitives.begin()+i,_primitives.begin()+i+numElementsToRemove);
        }
        else
        {
            // asking to delete too many elements, report a warning, and delete to
            // the end of the primitive list.
            notify(WARN)<<"Warning: osg::Geometry::removePrimitiveSet(i,numElementsToRemove) has been asked to remove more elements than are available,"<<std::endl;
            notify(WARN)<<"         removing on from i to the end of the list of primitive sets."<<std::endl;
            _primitives.erase(_primitives.begin()+i,_primitives.end());
        }
    
        dirtyDisplayList();
        dirtyBound();
        return true;
    }
    notify(WARN)<<"Warning: invalid index i passed to osg::Geometry::removePrimitiveSet(i,numElementsToRemove), ignoring call."<<std::endl;
    return false;
}

unsigned int Geometry::getPrimitiveSetIndex(const PrimitiveSet* primitiveset) const
{
    for (unsigned int primitiveSetIndex=0;primitiveSetIndex<_primitives.size();++primitiveSetIndex)
    {
        if (_primitives[primitiveSetIndex]==primitiveset) return primitiveSetIndex;
    }
    return _primitives.size(); // node not found.
}

bool Geometry::computeFastPathsUsed()
{
    static bool s_DisableFastPathInDisplayLists = getenv("OSG_DISABLE_FAST_PATH_IN_DISPLAY_LISTS")!=0;
    if (_useDisplayList && s_DisableFastPathInDisplayLists)
    {
        osg::notify(osg::DEBUG_INFO)<<"Geometry::computeFastPathsUsed() - Disabling fast paths in display lists"<<std::endl;
        _fastPath = false;
        return _fastPath;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // check to see if fast path can be used.
    //
    _fastPath = true;
    if (_vertexData.indices.valid()) _fastPath = false;
    else if (_normalData.binding==BIND_PER_PRIMITIVE || (_normalData.binding==BIND_PER_VERTEX && _normalData.indices.valid())) _fastPath = false;
    else if (_colorData.binding==BIND_PER_PRIMITIVE || (_colorData.binding==BIND_PER_VERTEX && _colorData.indices.valid())) _fastPath = false;
    else if (_secondaryColorData.binding==BIND_PER_PRIMITIVE || (_secondaryColorData.binding==BIND_PER_VERTEX && _secondaryColorData.indices.valid())) _fastPath = false;
    else if (_fogCoordData.binding==BIND_PER_PRIMITIVE || (_fogCoordData.binding==BIND_PER_VERTEX && _fogCoordData.indices.valid())) _fastPath = false;
    else 
    {
        for( unsigned int va = 0; va < _vertexAttribList.size(); ++va )
        {
            if (_vertexAttribList[va].binding==BIND_PER_PRIMITIVE)
            {
                _fastPath = false;
                break;
            }
            else
            {
                const Array * array = _vertexAttribList[va].array.get();
                const IndexArray * idxArray = _vertexAttribList[va].indices.get();

                if( _vertexAttribList[va].binding==BIND_PER_VERTEX && 
                    array && array->getNumElements()>0 &&
                    idxArray && idxArray->getNumElements()>0 ) 
                {
                    _fastPath = false;
                    break;
                }
            }
        }
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up tex coords if required.
    //
    for(unsigned int unit=0;unit<_texCoordList.size();++unit)
    {
        const ArrayData& texcoordData = _texCoordList[unit];
        if (texcoordData.array.valid() && texcoordData.array->getNumElements()>0)
        {
            if (texcoordData.indices.valid())
            {
                if (texcoordData.indices->getNumElements()>0)
                {
                    _fastPath = false;         
                    break;
                }
            }
        }
    }
    
    _supportsVertexBufferObjects = _fastPath;
    
    //_supportsVertexBufferObjects = false;
    //_useVertexBufferObjects = false;

    return _fastPath;
}

unsigned int Geometry::getGLObjectSizeHint() const
{
    unsigned int totalSize = 0;
    if (_vertexData.array.valid()) totalSize += _vertexData.array->getTotalDataSize();

    if (_normalData.array.valid()) totalSize += _normalData.array->getTotalDataSize();

    if (_colorData.array.valid()) totalSize += _colorData.array->getTotalDataSize();

    if (_secondaryColorData.array.valid()) totalSize += _secondaryColorData.array->getTotalDataSize();

    if (_fogCoordData.array.valid()) totalSize += _fogCoordData.array->getTotalDataSize();


    unsigned int unit;
    for(unit=0;unit<_texCoordList.size();++unit)
    {
        const Array* array = _texCoordList[unit].array.get();
        if (array) totalSize += array->getTotalDataSize();

    }

    bool handleVertexAttributes = true;
    if (handleVertexAttributes)
    {
        unsigned int index;
        for( index = 0; index < _vertexAttribList.size(); ++index )
        {
            const Array* array = _vertexAttribList[index].array.get();
            if (array) totalSize += array->getTotalDataSize();           
        }
    }

        for(PrimitiveSetList::const_iterator itr=_primitives.begin();
            itr!=_primitives.end();
            ++itr)
        {

            totalSize += 4*(*itr)->getNumIndices();

        }


    // do a very simply mapping of display list size proportional to vertex datasize.
    return totalSize;
}

void Geometry::drawImplementation(State& state) const
{

    // osg::notify(osg::NOTICE)<<"Geometry::drawImplementation"<<std::endl;

    if (_internalOptimizedGeometry.valid())
    {
        _internalOptimizedGeometry->drawImplementation(state);
        return;
    }

    const Extensions* extensions = getExtensions(state.getContextID(),true);

    if( !( ( _vertexData.array.valid() && _vertexData.array->getNumElements() != 0 ) ||
           ( _vertexAttribList.size() > 0 && 
             _vertexAttribList[0].array.valid() && 
             _vertexAttribList[0].array->getNumElements() != 0 ) ) )
    {
        return;
    }

    if( ( _vertexData.indices.valid() && _vertexData.indices->getNumElements() == 0 ) ||
          ( _vertexAttribList.size() > 0 && 
          _vertexAttribList[0].indices.valid() && 
          _vertexAttribList[0].indices->getNumElements() == 0 ) )
    {
        return;
    }

    DrawNormal         drawNormal(_normalData.array.get(),_normalData.indices.get());
    DrawColor          drawColor(_colorData.array.get(),_colorData.indices.get());
    DrawSecondaryColor drawSecondaryColor(_secondaryColorData.array.get(),_secondaryColorData.indices.get(),extensions);

    DrawFogCoord       drawFogCoord(_fogCoordData.array.get(),_fogCoordData.indices.get(),extensions);


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up secondary color if required.
    //
    AttributeBinding secondaryColorBinding = _secondaryColorData.binding;
    if (secondaryColorBinding!=BIND_OFF && !extensions->isSecondaryColorSupported())
    {
        // switch off if not supported or have a valid data.
        secondaryColorBinding = BIND_OFF;
    }

    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up fog coord if required.
    //
    AttributeBinding fogCoordBinding = _fogCoordData.binding;
    if (fogCoordBinding!=BIND_OFF && !extensions->isFogCoordSupported())
    {
        // switch off if not supported or have a valid data.
        fogCoordBinding = BIND_OFF;
    }

    unsigned int normalIndex = 0;
    unsigned int colorIndex = 0;
    unsigned int secondaryColorIndex = 0;
    unsigned int fogCoordIndex = 0;

#if USE_DEFAULT_NORMAL
    // if no values are defined for normal and color provide some defaults...
    if (_normalData.binding==BIND_OFF) glNormal3f(0.0f,0.0f,1.0f);
#endif

#if USE_DEFAULT_COLOUR
    if (_colorData.binding==BIND_OFF) glColor4f(1.0f,1.0f,1.0f,1.0f);
#endif

    typedef std::vector< ref_ptr<DrawVertexAttrib> > DrawVertexAttribList;
    typedef std::map< Geometry::AttributeBinding, DrawVertexAttribList> DrawVertexAttribMap;
    DrawVertexAttribMap drawVertexAttribMap;
    
    bool vertexVertexAttributesSupported = extensions->isVertexProgramSupported();
    bool handleVertexAttributes = (!_vertexAttribList.empty() && vertexVertexAttributesSupported);

    bool usingVertexBufferObjects = _useVertexBufferObjects && state.isVertexBufferObjectSupported();
    
    if (areFastPathsUsed())
    {
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        // fast path.        
        //
        if (usingVertexBufferObjects)
        {
            //
            // Vertex Buffer Object path for defining vertex arrays.
            // 
            
            GLuint& buffer = _vboList[state.getContextID()];
            if (!buffer)
            {

                //std::cout << "creating VertexBuffer "<<buffer<<std::endl;

                extensions->glGenBuffers(1, &buffer);
                extensions->glBindBuffer(GL_ARRAY_BUFFER_ARB,buffer);
                
                //std::cout << "  gen VertexBuffer "<<buffer<<std::endl;

                // compute total size and offsets required.
                unsigned long totalSize = 0;
                
                _vertexData.offset = 0;
                if (_vertexData.array.valid()) totalSize += _vertexData.array->getTotalDataSize();
                
                _normalData.offset = totalSize; 
                if (_normalData.array.valid()) totalSize += _normalData.array->getTotalDataSize();
                                
                _colorData.offset = totalSize;
                if (_colorData.array.valid()) totalSize += _colorData.array->getTotalDataSize();

                _secondaryColorData.offset = totalSize;
                if (_secondaryColorData.array.valid()) totalSize += _secondaryColorData.array->getTotalDataSize();

                _fogCoordData.offset = totalSize;
                if (_fogCoordData.array.valid()) totalSize += _fogCoordData.array->getTotalDataSize();


                unsigned int unit;
                for(unit=0;unit<_texCoordList.size();++unit)
                {
                    _texCoordList[unit].offset = totalSize;
                    const Array* array = _texCoordList[unit].array.get();
                    if (array)
                        totalSize += array->getTotalDataSize();
                        
                }

                if( handleVertexAttributes )
                {
                    unsigned int index;
                    for( index = 0; index < _vertexAttribList.size(); ++index )
                    {
                        _vertexAttribList[index].offset = totalSize;
                        const Array* array = _vertexAttribList[index].array.get();
                        const AttributeBinding ab = _vertexAttribList[index].binding;
                        if( ab == BIND_PER_VERTEX && array )
                        {
                            totalSize += array->getTotalDataSize();           
                        }
                    }
                }

                // allocated the buffer space, but leave the copy to be done per vertex array below
                extensions->glBufferData(GL_ARRAY_BUFFER_ARB,totalSize, 0, GL_STATIC_DRAW_ARB);
                
                //std::cout << "  Created VertexBuffer "<<buffer<<" size="<<totalSize<<std::endl;

                //
                // copy the data
                //
                if( _vertexData.array.valid() )
                    extensions->glBufferSubData(GL_ARRAY_BUFFER_ARB, _vertexData.offset, _vertexData.array->getTotalDataSize(),_vertexData.array->getDataPointer());

                if (_normalData.binding==BIND_PER_VERTEX)
                    extensions->glBufferSubData(GL_ARRAY_BUFFER_ARB, _normalData.offset, _normalData.array->getTotalDataSize(),_normalData.array->getDataPointer());

                if (_colorData.binding==BIND_PER_VERTEX)
                    extensions->glBufferSubData(GL_ARRAY_BUFFER_ARB, _colorData.offset, _colorData.array->getTotalDataSize(),_colorData.array->getDataPointer());

                if (secondaryColorBinding==BIND_PER_VERTEX)
                    extensions->glBufferSubData(GL_ARRAY_BUFFER_ARB, _secondaryColorData.offset, _secondaryColorData.array->getTotalDataSize(),_secondaryColorData.array->getDataPointer());

                if (fogCoordBinding==BIND_PER_VERTEX)
                    extensions->glBufferSubData(GL_ARRAY_BUFFER_ARB, _fogCoordData.offset, _fogCoordData.array->getTotalDataSize(),_fogCoordData.array->getDataPointer());

                for(unit=0;unit<_texCoordList.size();++unit)
                {
                    const Array* array = _texCoordList[unit].array.get();
                    if (array)
                        extensions->glBufferSubData(GL_ARRAY_BUFFER_ARB, _texCoordList[unit].offset, array->getTotalDataSize(), array->getDataPointer());
                }

                if( handleVertexAttributes )
                {
                    unsigned int index;
                    for( index = 0; index < _vertexAttribList.size(); ++index )
                    {
                        const Array* array = _vertexAttribList[index].array.get();
                        const AttributeBinding ab = _vertexAttribList[index].binding;

                        if( ab == BIND_PER_VERTEX && array )
                        {
                            extensions->glBufferSubData(GL_ARRAY_BUFFER_ARB, _vertexAttribList[index].offset, array->getTotalDataSize(), array->getDataPointer());
                        }
                    }
                }
                                     
                
            }
            
            //std::cout << "binding VertexBuffer "<<buffer<<std::endl;

            extensions->glBindBuffer(GL_ARRAY_BUFFER_ARB,buffer);
                      
            if( _vertexData.array.valid() )
                state.setVertexPointer(_vertexData.array->getDataSize(),_vertexData.array->getDataType(),0,(const GLvoid*)_vertexData.offset);
            else
                state.disableVertexPointer();

            if (_normalData.binding==BIND_PER_VERTEX && _normalData.array.valid())
                state.setNormalPointer(GL_FLOAT,0,(const GLvoid*)_normalData.offset);
            else
                state.disableNormalPointer();

            if (_colorData.binding==BIND_PER_VERTEX && _colorData.array.valid())
                state.setColorPointer(_colorData.array->getDataSize(),_colorData.array->getDataType(),0,(const GLvoid*)_colorData.offset);
            else
                state.disableColorPointer();

            if (secondaryColorBinding==BIND_PER_VERTEX && _secondaryColorData.array.valid())
                state.setSecondaryColorPointer(_secondaryColorData.array->getDataSize(),_secondaryColorData.array->getDataType(),0,(const GLvoid*)_secondaryColorData.offset);
            else
                state.disableSecondaryColorPointer();

            if (fogCoordBinding==BIND_PER_VERTEX && _fogCoordData.array.valid())
                state.setFogCoordPointer(GL_FLOAT,0,(const GLvoid*)_fogCoordData.offset);
            else
                state.disableFogCoordPointer();

            unsigned int unit;
            for(unit=0;unit<_texCoordList.size();++unit)
            {
                const Array* array = _texCoordList[unit].array.get();
                if (array)
                    state.setTexCoordPointer(unit,array->getDataSize(),array->getDataType(),0,(const GLvoid*)_texCoordList[unit].offset);
                else
                    state.disableTexCoordPointer(unit);
            }
            state.disableTexCoordPointersAboveAndIncluding(unit);

            if( handleVertexAttributes )
            {
                unsigned int index;
                for( index = 0; index < _vertexAttribList.size(); ++index )
                {
                    const Array* array = _vertexAttribList[index].array.get();
                    const AttributeBinding ab = _vertexAttribList[index].binding;

                    if( ab == BIND_PER_VERTEX && array )
                    {
                        state.setVertexAttribPointer( index, array->getDataSize(), array->getDataType(), 
                            _vertexAttribList[index].normalize, 0, (const GLvoid*)_vertexAttribList[index].offset );
                    }
                    else
                    {
                        if( array )
                        {
                            const IndexArray* indexArray = _vertexAttribList[index].indices.get();

                            if( indexArray && indexArray->getNumElements() > 0 )
                            {
                                drawVertexAttribMap[ab].push_back( 
                                    new DrawVertexAttrib(extensions,index,_vertexAttribList[index].normalize,array,indexArray) );
                            }
                            else
                            {
                                drawVertexAttribMap[ab].push_back( 
                                    new DrawVertexAttrib(extensions,index,_vertexAttribList[index].normalize,array,0) );
                            }
                        }

                        state.disableVertexAttribPointer( index );
                    }
                }
                state.disableVertexAttribPointersAboveAndIncluding( index );
                
            }
            else if (vertexVertexAttributesSupported)
            {
                state.disableVertexAttribPointersAboveAndIncluding( 0 );
            }


        }
        else
        {
            //std::cout << "none VertexBuffer path"<<std::endl;

            //
            // None Vertex Buffer Object path for defining vertex arrays.
            //            
            if( _vertexData.array.valid() )
                state.setVertexPointer(_vertexData.array->getDataSize(),_vertexData.array->getDataType(),0,_vertexData.array->getDataPointer());
            else
                state.disableVertexPointer();

            if (_normalData.binding==BIND_PER_VERTEX && _normalData.array.valid())
                state.setNormalPointer(_normalData.array->getDataType(),0,_normalData.array->getDataPointer());
            else
                state.disableNormalPointer();

            if (_colorData.binding==BIND_PER_VERTEX && _colorData.array.valid())
                state.setColorPointer(_colorData.array->getDataSize(),_colorData.array->getDataType(),0,_colorData.array->getDataPointer());
            else
                state.disableColorPointer();

            if (secondaryColorBinding==BIND_PER_VERTEX && _secondaryColorData.array.valid())
                state.setSecondaryColorPointer(_secondaryColorData.array->getDataSize(),_secondaryColorData.array->getDataType(),0,_secondaryColorData.array->getDataPointer());
            else
                state.disableSecondaryColorPointer();

            if (fogCoordBinding==BIND_PER_VERTEX && _fogCoordData.array.valid())
                state.setFogCoordPointer(GL_FLOAT,0,_fogCoordData.array->getDataPointer());
            else
                state.disableFogCoordPointer();

            unsigned int unit;
            for(unit=0;unit<_texCoordList.size();++unit)
            {
                const Array* array = _texCoordList[unit].array.get();
                if (array)
                    state.setTexCoordPointer(unit,array->getDataSize(),array->getDataType(),0,array->getDataPointer());
                else
                    state.disableTexCoordPointer(unit);
            }
            state.disableTexCoordPointersAboveAndIncluding(unit);

            if( handleVertexAttributes )
            {
                unsigned int index;
                for( index = 0; index < _vertexAttribList.size(); ++index )
                {
                    const Array* array = _vertexAttribList[index].array.get();
                    const AttributeBinding ab = _vertexAttribList[index].binding;

                    if( ab == BIND_PER_VERTEX && array )
                    {
                        state.setVertexAttribPointer( index, array->getDataSize(), array->getDataType(), 
                            _vertexAttribList[index].normalize, 0, array->getDataPointer() );
                    }
                    else
                    {
                        if( array )
                        {
                            const IndexArray* indexArray = _vertexAttribList[index].indices.get();

                            if( indexArray && indexArray->getNumElements() > 0 )
                            {
                                drawVertexAttribMap[ab].push_back( 
                                    new DrawVertexAttrib(extensions,index,_vertexAttribList[index].normalize,array,indexArray) );
                            }
                            else
                            {
                                drawVertexAttribMap[ab].push_back( 
                                    new DrawVertexAttrib(extensions,index,_vertexAttribList[index].normalize,array,0) );
                            }
                        }

                        state.disableVertexAttribPointer( index );
                    }
                }
                state.disableVertexAttribPointersAboveAndIncluding( index );
                
            }
            else if (vertexVertexAttributesSupported)
            {
                state.disableVertexAttribPointersAboveAndIncluding( 0 );
            }
        }
        
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        // pass the overall binding values onto OpenGL.
        //
        if (_normalData.binding==BIND_OVERALL)      drawNormal(normalIndex++);
        if (_colorData.binding==BIND_OVERALL)       drawColor(colorIndex++);
        if (secondaryColorBinding==BIND_OVERALL)    drawSecondaryColor(secondaryColorIndex++);
        if (fogCoordBinding==BIND_OVERALL)          drawFogCoord(fogCoordIndex++);
        if (handleVertexAttributes)
        {
            DrawVertexAttribList &list = drawVertexAttribMap[BIND_OVERALL];

            for( unsigned int i = 0; i < list.size(); ++i )
            {
                list[i]->applyAndIncrement();
            }
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        // draw the primitives themselves.
        //
        for(PrimitiveSetList::const_iterator itr=_primitives.begin();
            itr!=_primitives.end();
            ++itr)
        {

            if (_normalData.binding==BIND_PER_PRIMITIVE_SET)      drawNormal(normalIndex++);
            if (_colorData.binding==BIND_PER_PRIMITIVE_SET)       drawColor(colorIndex++);
            if (secondaryColorBinding==BIND_PER_PRIMITIVE_SET)    drawSecondaryColor(secondaryColorIndex++);
            if (fogCoordBinding==BIND_PER_PRIMITIVE_SET)          drawFogCoord(fogCoordIndex++);
            if (handleVertexAttributes)
            {
                DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_PRIMITIVE_SET];

                for( unsigned int i = 0; i < list.size(); ++i )
                {
                    list[i]->applyAndIncrement();
                }
            }

            (*itr)->draw(state, usingVertexBufferObjects);

        }

        if (usingVertexBufferObjects)
        {
            extensions->glBindBuffer(GL_ARRAY_BUFFER_ARB,0);
        }

    }
    else
    {   

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        // slow path.        
        //
        

        typedef std::vector< ref_ptr<DrawMultiTexCoord> > DrawTexCoordList;
        DrawTexCoordList drawTexCoordList;
        drawTexCoordList.reserve(_texCoordList.size());

        // fallback if multitexturing not supported.
        ref_ptr<DrawTexCoord> drawTextCoord;

        if (extensions->isMultiTexSupported() && _texCoordList.size()>1)
        {
            // multitexture supported..
            for(unsigned int unit=0;unit!=_texCoordList.size();++unit)
            {
                const ArrayData& texcoordData = _texCoordList[unit];
                if (texcoordData.array.valid() && texcoordData.array->getNumElements()>0)
                {
                    if (texcoordData.indices.valid() && texcoordData.indices->getNumElements()>0)
                    {
                        drawTexCoordList.push_back(new DrawMultiTexCoord(GL_TEXTURE0+unit,texcoordData.array.get(),texcoordData.indices.get(),
                                                                         extensions));
                    }
                    else
                    {
                        drawTexCoordList.push_back(new DrawMultiTexCoord(GL_TEXTURE0+unit,texcoordData.array.get(),0,
                                                                          extensions));
                    }
                }
            }
        }
        else
        {
            if (!_texCoordList.empty())
            {
                const ArrayData& texcoordData = _texCoordList[0];
                if (texcoordData.array.valid() && texcoordData.array->getNumElements()>0)
                {
                    if (texcoordData.indices.valid())
                    {
                        if (texcoordData.indices->getNumElements()>0)
                        {
                            drawTextCoord = new DrawTexCoord(texcoordData.array.get(),texcoordData.indices.get());
                        }
                    }
                    else
                    {
                        drawTextCoord = new DrawTexCoord(texcoordData.array.get(),0);
                    }
                }
            }
        }

        if(handleVertexAttributes)
        {
            unsigned int index;
            for( index = 1; index < _vertexAttribList.size(); ++index )
            {
                const ArrayData& vertAttribData = _vertexAttribList[index];
            
                if( vertAttribData.array.valid() && vertAttribData.array->getNumElements() > 0 )
                {
                    if( vertAttribData.indices.valid() && vertAttribData.indices->getNumElements() > 0 )
                    {
                        drawVertexAttribMap[vertAttribData.binding].push_back( 
                            new DrawVertexAttrib(extensions,index,vertAttribData.normalize,vertAttribData.array.get(),vertAttribData.indices.get() ));
                    }
                    else
                    {
                        drawVertexAttribMap[vertAttribData.binding].push_back( 
                            new DrawVertexAttrib(extensions,index,vertAttribData.normalize,vertAttribData.array.get(),0) );
                    }            
                }
            }
        }

        // disable all the vertex arrays in the slow path as we are
        // sending everything using glVertex etc.
        state.disableAllVertexArrays();


        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        // pass the overall binding values onto OpenGL.
        //
        if (_normalData.binding==BIND_OVERALL)      drawNormal(normalIndex++);
        if (_colorData.binding==BIND_OVERALL)       drawColor(colorIndex++);
        if (secondaryColorBinding==BIND_OVERALL)    drawSecondaryColor(secondaryColorIndex++);
        if (fogCoordBinding==BIND_OVERALL)          drawFogCoord(fogCoordIndex++);
        if (handleVertexAttributes)
        {
            DrawVertexAttribList &list = drawVertexAttribMap[BIND_OVERALL];

            for( unsigned int i = 0; i < list.size(); ++i )
            {
                list[i]->applyAndIncrement();
            }
        }

        // set up vertex functor.
        DrawVertex drawVertex(_vertexData.array.get(),_vertexData.indices.get());

        bool useVertexAttrib =  _vertexAttribList.size() > 0 &&
                                _vertexAttribList[0].array.valid() && 
                                 _vertexAttribList[0].indices->getNumElements();

        ref_ptr<DrawVertexAttrib> drawVertexAttribZero;
        if( useVertexAttrib )
        {
            drawVertexAttribZero = new DrawVertexAttrib(extensions,0,
                _vertexAttribList[0].normalize,_vertexAttribList[0].array.get(),
                _vertexAttribList[0].indices.get()); 
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        // draw the primitives themselves.
        //
        for(PrimitiveSetList::const_iterator itr=_primitives.begin();
            itr!=_primitives.end();
            ++itr)
        {
            if (_normalData.binding==BIND_PER_PRIMITIVE_SET)           drawNormal(normalIndex++);
            if (_colorData.binding==BIND_PER_PRIMITIVE_SET)            drawColor(colorIndex++);
            if (secondaryColorBinding==BIND_PER_PRIMITIVE_SET)    drawSecondaryColor(secondaryColorIndex++);
            if (fogCoordBinding==BIND_PER_PRIMITIVE_SET)          drawFogCoord(fogCoordIndex++);
            if (handleVertexAttributes)
            {
                DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_PRIMITIVE_SET];

                for( unsigned int i = 0; i < list.size(); ++i )
                {
                    list[i]->applyAndIncrement();
                }
            }

            const PrimitiveSet* primitiveset = itr->get();
            GLenum mode=primitiveset->getMode();

            unsigned int primLength;
            switch(mode)
            {
                case(GL_POINTS):    primLength=1; break;
                case(GL_LINES):     primLength=2; break;
                case(GL_TRIANGLES): primLength=3; break;
                case(GL_QUADS):     primLength=4; break;
                default:            primLength=0; break; // compute later when =0.
            }


            // draw primtives by the more flexible "slow" path,
            // sending OpenGL glBegin/glVertex.../glEnd().
            switch(primitiveset->getType())
            {
                case(PrimitiveSet::DrawArraysPrimitiveType):
                {
                    if (primLength==0) primLength=primitiveset->getNumIndices();

                    const DrawArrays* drawArray = static_cast<const DrawArrays*>(primitiveset);
                    glBegin(mode);

                    unsigned int primCount=0;
                    unsigned int indexEnd = drawArray->getFirst()+drawArray->getCount();
                    for(unsigned int vindex=drawArray->getFirst();
                        vindex<indexEnd;
                        ++vindex,++primCount)
                    {

                        if ((primCount%primLength)==0)
                        {
                            if (_normalData.binding==BIND_PER_PRIMITIVE)           drawNormal(normalIndex++);
                            if (_colorData.binding==BIND_PER_PRIMITIVE)            drawColor(colorIndex++);
                            if (secondaryColorBinding==BIND_PER_PRIMITIVE)    drawSecondaryColor(secondaryColorIndex++);
                            if (fogCoordBinding==BIND_PER_PRIMITIVE)          drawFogCoord(fogCoordIndex++);
                            if (handleVertexAttributes)
                            {
                                DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_PRIMITIVE];

                                for( unsigned int i = 0; i < list.size(); ++i )
                                {
                                    list[i]->applyAndIncrement();
                                }
                            }                        
                        }

                        if (_normalData.binding==BIND_PER_VERTEX)           drawNormal(vindex);
                        if (_colorData.binding==BIND_PER_VERTEX)            drawColor(vindex);
                        if (secondaryColorBinding==BIND_PER_VERTEX)    drawSecondaryColor(vindex);
                        if (fogCoordBinding==BIND_PER_VERTEX)          drawFogCoord(vindex);
                        if (handleVertexAttributes)
                        {
                            DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_VERTEX];

                            for( unsigned int i = 0; i < list.size(); ++i )
                            {
                                list[i]->applyAndIncrement();
                            }
                        }  

                        for(DrawTexCoordList::iterator texItr=drawTexCoordList.begin();
                            texItr!=drawTexCoordList.end();
                            ++texItr)
                        {
                            (*(*texItr))(vindex);
                        }
                        if (drawTextCoord.valid()) (*drawTextCoord)(vindex);

                        if( useVertexAttrib )
                        {
                            (*drawVertexAttribZero)(vindex);
                        }
                        else
                        {
                            drawVertex(vindex);
                        }
                    }
                    
                    glEnd();
                    break;
                }
                case(PrimitiveSet::DrawArrayLengthsPrimitiveType):
                {

                    const DrawArrayLengths* drawArrayLengths = static_cast<const DrawArrayLengths*>(primitiveset);
                    unsigned int vindex=drawArrayLengths->getFirst();
                    for(DrawArrayLengths::const_iterator primItr=drawArrayLengths->begin();
                        primItr!=drawArrayLengths->end();
                        ++primItr)
                    {
                        unsigned int localPrimLength;
                        if (primLength==0) localPrimLength=*primItr;
                        else localPrimLength=primLength;

                        glBegin(mode);

                        for(GLsizei primCount=0;primCount<*primItr;++primCount)
                        {
                            if ((primCount%localPrimLength)==0)
                            {
                                if (_normalData.binding==BIND_PER_PRIMITIVE)           drawNormal(normalIndex++);
                                if (_colorData.binding==BIND_PER_PRIMITIVE)            drawColor(colorIndex++);
                                if (secondaryColorBinding==BIND_PER_PRIMITIVE)    drawSecondaryColor(secondaryColorIndex++);
                                if (fogCoordBinding==BIND_PER_PRIMITIVE)          drawFogCoord(fogCoordIndex++);
                                if (handleVertexAttributes)
                                {
                                    DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_PRIMITIVE];

                                    for( unsigned int i = 0; i < list.size(); ++i )
                                    {
                                        list[i]->applyAndIncrement();
                                    }
                                }  
                            }
                            
                            if (_normalData.binding==BIND_PER_VERTEX)           drawNormal(vindex);
                            if (_colorData.binding==BIND_PER_VERTEX)            drawColor(vindex);
                            if (secondaryColorBinding==BIND_PER_VERTEX)    drawSecondaryColor(vindex);
                            if (fogCoordBinding==BIND_PER_VERTEX)          drawFogCoord(vindex);
                            if (handleVertexAttributes)
                            {
                                DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_VERTEX];

                                for( unsigned int i = 0; i < list.size(); ++i )
                                {
                                    list[i]->applyAndIncrement();
                                }
                            }  
                            for(DrawTexCoordList::iterator texItr=drawTexCoordList.begin();
                                texItr!=drawTexCoordList.end();
                                ++texItr)
                            {
                                (*(*texItr))(vindex);
                            }
                            if (drawTextCoord.valid()) (*drawTextCoord)(vindex);

                            if( useVertexAttrib )
                            {
                                (*drawVertexAttribZero)(vindex);
                            }
                            else
                            {
                                drawVertex(vindex);
                            }

                            ++vindex;
                        }
                        
                        glEnd();

                    }
                    break;
                }
                case(PrimitiveSet::DrawElementsUBytePrimitiveType):
                {
                    if (primLength==0) primLength=primitiveset->getNumIndices();

                    const DrawElementsUByte* drawElements = static_cast<const DrawElementsUByte*>(primitiveset);
                    glBegin(mode);

                    unsigned int primCount=0;
                    for(DrawElementsUByte::const_iterator primItr=drawElements->begin();
                        primItr!=drawElements->end();
                        ++primCount,++primItr)
                    {

                        if ((primCount%primLength)==0)
                        {
                            if (_normalData.binding==BIND_PER_PRIMITIVE)           drawNormal(normalIndex++);
                            if (_colorData.binding==BIND_PER_PRIMITIVE)            drawColor(colorIndex++);
                            if (secondaryColorBinding==BIND_PER_PRIMITIVE)    drawSecondaryColor(secondaryColorIndex++);
                            if (fogCoordBinding==BIND_PER_PRIMITIVE)          drawFogCoord(fogCoordIndex++);
                            if (handleVertexAttributes)
                            {
                                DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_PRIMITIVE];

                                for( unsigned int i = 0; i < list.size(); ++i )
                                {
                                    list[i]->applyAndIncrement();
                                }
                            }  
                        }
                        
                        unsigned int vindex=*primItr;

                        if (_normalData.binding==BIND_PER_VERTEX)           drawNormal(vindex);
                        if (_colorData.binding==BIND_PER_VERTEX)            drawColor(vindex);
                        if (secondaryColorBinding==BIND_PER_VERTEX)    drawSecondaryColor(vindex);
                        if (fogCoordBinding==BIND_PER_VERTEX)          drawFogCoord(vindex);
                        if ( extensions->isVertexProgramSupported() )
                        {
                            DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_VERTEX];

                            for( unsigned int i = 0; i < list.size(); ++i )
                            {
                                list[i]->applyAndIncrement();
                            }
                        }  

                        for(DrawTexCoordList::iterator texItr=drawTexCoordList.begin();
                            texItr!=drawTexCoordList.end();
                            ++texItr)
                        {
                            (*(*texItr))(vindex);
                        }
                        if (drawTextCoord.valid()) (*drawTextCoord)(vindex);

                        if( useVertexAttrib )
                        {
                            (*drawVertexAttribZero)(vindex);
                        }
                        else
                        {
                            drawVertex(vindex);
                        }
                    }

                    glEnd();
                    break;
                }
                case(PrimitiveSet::DrawElementsUShortPrimitiveType):
                {
                    if (primLength==0) primLength=primitiveset->getNumIndices();

                    const DrawElementsUShort* drawElements = static_cast<const DrawElementsUShort*>(primitiveset);
                    glBegin(mode);

                    unsigned int primCount=0;
                    for(DrawElementsUShort::const_iterator primItr=drawElements->begin();
                        primItr!=drawElements->end();
                        ++primCount,++primItr)
                    {

                        if ((primCount%primLength)==0)
                        {
                            if (_normalData.binding==BIND_PER_PRIMITIVE)           drawNormal(normalIndex++);
                            if (_colorData.binding==BIND_PER_PRIMITIVE)            drawColor(colorIndex++);
                            if (secondaryColorBinding==BIND_PER_PRIMITIVE)    drawSecondaryColor(secondaryColorIndex++);
                            if (fogCoordBinding==BIND_PER_PRIMITIVE)          drawFogCoord(fogCoordIndex++);
                            if (handleVertexAttributes)
                            {
                                DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_PRIMITIVE];

                                for( unsigned int i = 0; i < list.size(); ++i )
                                {
                                    list[i]->applyAndIncrement();
                                }
                            }  
                        }
                        
                        unsigned int vindex=*primItr;

                        if (_normalData.binding==BIND_PER_VERTEX)           drawNormal(vindex);
                        if (_colorData.binding==BIND_PER_VERTEX)            drawColor(vindex);
                        if (secondaryColorBinding==BIND_PER_VERTEX)    drawSecondaryColor(vindex);
                        if (fogCoordBinding==BIND_PER_VERTEX)          drawFogCoord(vindex);
                        if (handleVertexAttributes)
                        {
                            DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_VERTEX];

                            for( unsigned int i = 0; i < list.size(); ++i )
                            {
                                list[i]->applyAndIncrement();
                            }
                        }  

                        for(DrawTexCoordList::iterator texItr=drawTexCoordList.begin();
                            texItr!=drawTexCoordList.end();
                            ++texItr)
                        {
                            (*(*texItr))(vindex);
                        }
                        if (drawTextCoord.valid()) (*drawTextCoord)(vindex);

                        if( useVertexAttrib )
                        {
                            (*drawVertexAttribZero)(vindex);
                        }
                        else
                        {
                            drawVertex(vindex);
                        }
                    }

                    glEnd();
                    break;
                }
                case(PrimitiveSet::DrawElementsUIntPrimitiveType):
                {
                    if (primLength==0) primLength=primitiveset->getNumIndices();

                    const DrawElementsUInt* drawElements = static_cast<const DrawElementsUInt*>(primitiveset);
                    glBegin(mode);

                    unsigned int primCount=0;
                    for(DrawElementsUInt::const_iterator primItr=drawElements->begin();
                        primItr!=drawElements->end();
                        ++primCount,++primItr)
                    {

                        if ((primCount%primLength)==0)
                        {
                            if (_normalData.binding==BIND_PER_PRIMITIVE)           drawNormal(normalIndex++);
                            if (_colorData.binding==BIND_PER_PRIMITIVE)            drawColor(colorIndex++);
                            if (secondaryColorBinding==BIND_PER_PRIMITIVE)    drawSecondaryColor(secondaryColorIndex++);
                            if (fogCoordBinding==BIND_PER_PRIMITIVE)          drawFogCoord(fogCoordIndex++);
                            if ( extensions->isVertexProgramSupported() )
                            {
                                DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_PRIMITIVE];

                                for( unsigned int i = 0; i < list.size(); ++i )
                                {
                                    list[i]->applyAndIncrement();
                                }
                            }  
                        }
                        
                        unsigned int vindex=*primItr;

                        if (_normalData.binding==BIND_PER_VERTEX)           drawNormal(vindex);
                        if (_colorData.binding==BIND_PER_VERTEX)            drawColor(vindex);
                        if (secondaryColorBinding==BIND_PER_VERTEX)    drawSecondaryColor(vindex);
                        if (fogCoordBinding==BIND_PER_VERTEX)          drawFogCoord(vindex);
                        if ( extensions->isVertexProgramSupported() )
                        {
                            DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_VERTEX];

                            for( unsigned int i = 0; i < list.size(); ++i )
                            {
                                list[i]->applyAndIncrement();
                            }
                        }  

                        for(DrawTexCoordList::iterator texItr=drawTexCoordList.begin();
                            texItr!=drawTexCoordList.end();
                            ++texItr)
                        {
                            (*(*texItr))(vindex);
                        }
                        if (drawTextCoord.valid()) (*drawTextCoord)(vindex);

                        if( useVertexAttrib )
                        {
                            (*drawVertexAttribZero)(vindex);
                        }
                        else
                        {
                            drawVertex(vindex);
                        }
                    }

                    glEnd();
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }

}

class AttributeFunctorArrayVisitor : public ArrayVisitor
{
    public:
    
        AttributeFunctorArrayVisitor(Drawable::AttributeFunctor& af):
            _af(af) {}
    
        virtual ~AttributeFunctorArrayVisitor() {}

        virtual void apply(ByteArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(ShortArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(IntArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(UByteArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(UShortArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(UIntArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(Vec4ubArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(FloatArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(Vec2Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(Vec3Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(Vec4Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
    
    
        inline void applyArray(Drawable::AttributeType type,Array* array)
        {
            if (array)
            {
                _type = type;
                array->accept(*this);
            }
        }
    
        Drawable::AttributeFunctor&   _af;
        Drawable::AttributeType       _type;
};

void Geometry::accept(AttributeFunctor& af)
{
    AttributeFunctorArrayVisitor afav(af);

    afav.applyArray(VERTICES,_vertexData.array.get());
    afav.applyArray(NORMALS,_normalData.array.get());
    afav.applyArray(COLORS,_colorData.array.get());
    afav.applyArray(SECONDARY_COLORS,_secondaryColorData.array.get());
    afav.applyArray(FOG_COORDS,_fogCoordData.array.get());
    
    for(unsigned unit=0;unit<_texCoordList.size();++unit)
    {
        afav.applyArray((AttributeType)(TEXTURE_COORDS_0+unit),_texCoordList[unit].array.get());
    }

    for(unsigned int index=0; index<_vertexAttribList.size(); ++index)
    {
        afav.applyArray(index,_vertexAttribList[index].array.get());
    }
}

class ConstAttributeFunctorArrayVisitor : public ConstArrayVisitor
{
    public:
    
        ConstAttributeFunctorArrayVisitor(Drawable::ConstAttributeFunctor& af):
            _af(af) {}
    
        virtual ~ConstAttributeFunctorArrayVisitor() {}

        virtual void apply(const ByteArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const ShortArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const IntArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const UByteArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const UShortArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const UIntArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const Vec4ubArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const FloatArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const Vec2Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const Vec3Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const Vec4Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
    
    
        inline void applyArray(Drawable::AttributeType type,const Array* array)
        {
            if (array)
            {
                _type = type;
                array->accept(*this);
            }
        }
    
        Drawable::ConstAttributeFunctor&    _af;
        Drawable::AttributeType             _type;
};

void Geometry::accept(ConstAttributeFunctor& af) const
{
    ConstAttributeFunctorArrayVisitor afav(af);
    
    afav.applyArray(VERTICES,_vertexData.array.get());
    afav.applyArray(NORMALS,_normalData.array.get());
    afav.applyArray(COLORS,_colorData.array.get());
    afav.applyArray(SECONDARY_COLORS,_secondaryColorData.array.get());
    afav.applyArray(FOG_COORDS,_fogCoordData.array.get());

    for(unsigned unit=0;unit<_texCoordList.size();++unit)
    {
        afav.applyArray((AttributeType)(TEXTURE_COORDS_0+unit),_texCoordList[unit].array.get());
    }

    for(unsigned int index=0; index<_vertexAttribList.size(); ++index)
    {
        afav.applyArray(index,_vertexAttribList[index].array.get());
    }
}

void Geometry::accept(PrimitiveFunctor& functor) const
{
    if (!_vertexData.array.valid() || _vertexData.array->getNumElements()==0) return;

    if (!_vertexData.indices.valid())
    {
        switch(_vertexData.array->getType())
        {
        case(Array::Vec2ArrayType): 
            functor.setVertexArray(_vertexData.array->getNumElements(),static_cast<const Vec2*>(_vertexData.array->getDataPointer()));
            break;
        case(Array::Vec3ArrayType): 
            functor.setVertexArray(_vertexData.array->getNumElements(),static_cast<const Vec3*>(_vertexData.array->getDataPointer()));
            break;
        case(Array::Vec4ArrayType): 
            functor.setVertexArray(_vertexData.array->getNumElements(),static_cast<const Vec4*>(_vertexData.array->getDataPointer()));
            break;
        default:
            notify(WARN)<<"Warning: Geometry::accept(PrimtiveFunctor&) cannot handle Vertex Array type"<<_vertexData.array->getType()<<std::endl;
            return;
        }
        
        for(PrimitiveSetList::const_iterator itr=_primitives.begin();
            itr!=_primitives.end();
            ++itr)
        {
            (*itr)->accept(functor);
        }
    }
    else
    {
        const Vec2Array* vec2Array = 0;
        const Vec3Array* vec3Array = 0;
        const Vec4Array* vec4Array = 0;
        Array::Type type = _vertexData.array->getType();
        switch(type)
        {
        case(Array::Vec2ArrayType): 
            vec2Array = static_cast<const Vec2Array*>(_vertexData.array.get());
            break;
        case(Array::Vec3ArrayType): 
            vec3Array = static_cast<const Vec3Array*>(_vertexData.array.get());
            break;
        case(Array::Vec4ArrayType): 
            vec4Array = static_cast<const Vec4Array*>(_vertexData.array.get());
            break;
        default:
            notify(WARN)<<"Warning: Geometry::accept(PrimtiveFunctor&) cannot handle Vertex Array type"<<_vertexData.array->getType()<<std::endl;
            return;
        }

    
        for(PrimitiveSetList::const_iterator itr=_primitives.begin();
            itr!=_primitives.end();
            ++itr)
        {
            const PrimitiveSet* primitiveset = itr->get();
            GLenum mode=primitiveset->getMode();
            switch(primitiveset->getType())
            {
                case(PrimitiveSet::DrawArraysPrimitiveType):
                {
                    const DrawArrays* drawArray = static_cast<const DrawArrays*>(primitiveset);
                    functor.begin(mode);

                    unsigned int indexEnd = drawArray->getFirst()+drawArray->getCount();
                    for(unsigned int vindex=drawArray->getFirst();
                        vindex<indexEnd;
                        ++vindex)
                    {
                        switch(type)
                        {
                        case(Array::Vec2ArrayType): 
                            functor.vertex((*vec2Array)[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec3ArrayType): 
                            functor.vertex((*vec3Array)[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec4ArrayType): 
                            functor.vertex((*vec4Array)[_vertexData.indices->index(vindex)]);
                            break;
                        default:
                            break;
                        }

                    }
                    
                    functor.end();
                    break;
                }
                case(PrimitiveSet::DrawArrayLengthsPrimitiveType):
                {

                    const DrawArrayLengths* drawArrayLengths = static_cast<const DrawArrayLengths*>(primitiveset);
                    unsigned int vindex=drawArrayLengths->getFirst();
                    for(DrawArrayLengths::const_iterator primItr=drawArrayLengths->begin();
                        primItr!=drawArrayLengths->end();
                        ++primItr)
                    {

                        functor.begin(mode);

                        for(GLsizei primCount=0;primCount<*primItr;++primCount)
                        {
                            switch(type)
                            {
                            case(Array::Vec2ArrayType): 
                                functor.vertex((*vec2Array)[_vertexData.indices->index(vindex)]);
                                break;
                            case(Array::Vec3ArrayType): 
                                functor.vertex((*vec3Array)[_vertexData.indices->index(vindex)]);
                                break;
                            case(Array::Vec4ArrayType): 
                                functor.vertex((*vec4Array)[_vertexData.indices->index(vindex)]);
                                break;
                            default:
                                break;
                            }
                            ++vindex;
                        }
                        
                        functor.end();

                    }
                    break;
                }
                case(PrimitiveSet::DrawElementsUBytePrimitiveType):
                {
                    const DrawElementsUByte* drawElements = static_cast<const DrawElementsUByte*>(primitiveset);
                    functor.begin(mode);

                    unsigned int primCount=0;
                    for(DrawElementsUByte::const_iterator primItr=drawElements->begin();
                        primItr!=drawElements->end();
                        ++primCount,++primItr)
                    {
                        unsigned int vindex=*primItr;
                        switch(type)
                        {
                        case(Array::Vec2ArrayType): 
                            functor.vertex((*vec2Array)[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec3ArrayType): 
                            functor.vertex((*vec3Array)[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec4ArrayType): 
                            functor.vertex((*vec4Array)[_vertexData.indices->index(vindex)]);
                            break;
                        default:
                            break;
                        }
                    }

                    functor.end();
                    break;
                }
                case(PrimitiveSet::DrawElementsUShortPrimitiveType):
                {
                    const DrawElementsUShort* drawElements = static_cast<const DrawElementsUShort*>(primitiveset);
                    functor.begin(mode);

                    for(DrawElementsUShort::const_iterator primItr=drawElements->begin();
                        primItr!=drawElements->end();
                        ++primItr)
                    {
                        unsigned int vindex=*primItr;
                        switch(type)
                        {
                        case(Array::Vec2ArrayType): 
                            functor.vertex((*vec2Array)[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec3ArrayType): 
                            functor.vertex((*vec3Array)[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec4ArrayType): 
                            functor.vertex((*vec4Array)[_vertexData.indices->index(vindex)]);
                            break;
                        default:
                            break;
                        }
                    }

                    functor.end();
                    break;
                }
                case(PrimitiveSet::DrawElementsUIntPrimitiveType):
                {
                    const DrawElementsUInt* drawElements = static_cast<const DrawElementsUInt*>(primitiveset);
                    functor.begin(mode);

                    for(DrawElementsUInt::const_iterator primItr=drawElements->begin();
                        primItr!=drawElements->end();
                        ++primItr)
                    {
                        unsigned int vindex=*primItr;
                        switch(type)
                        {
                        case(Array::Vec2ArrayType): 
                            functor.vertex((*vec2Array)[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec3ArrayType): 
                            functor.vertex((*vec3Array)[_vertexData.indices->index(vindex)]);
                            break;
                        case(Array::Vec4ArrayType): 
                            functor.vertex((*vec4Array)[_vertexData.indices->index(vindex)]);
                            break;
                        default:
                            break;
                        }
                    }

                    functor.end();
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }
    return;
}

void Geometry::accept(PrimitiveIndexFunctor& functor) const
{
    if (!_vertexData.array.valid() || _vertexData.array->getNumElements()==0) return;

    switch(_vertexData.array->getType())
    {
    case(Array::Vec2ArrayType): 
        functor.setVertexArray(_vertexData.array->getNumElements(),static_cast<const Vec2*>(_vertexData.array->getDataPointer()));
        break;
    case(Array::Vec3ArrayType): 
        functor.setVertexArray(_vertexData.array->getNumElements(),static_cast<const Vec3*>(_vertexData.array->getDataPointer()));
        break;
    case(Array::Vec4ArrayType): 
        functor.setVertexArray(_vertexData.array->getNumElements(),static_cast<const Vec4*>(_vertexData.array->getDataPointer()));
        break;
    default:
        notify(WARN)<<"Warning: Geometry::accept(PrimtiveIndexFunctor&) cannot handle Vertex Array type"<<_vertexData.array->getType()<<std::endl;
        return;
    }

    if (!_vertexData.indices.valid())
    {
        for(PrimitiveSetList::const_iterator itr=_primitives.begin();
            itr!=_primitives.end();
            ++itr)
        {
            (*itr)->accept(functor);
        }
    }
    else
    {
        for(PrimitiveSetList::const_iterator itr=_primitives.begin();
            itr!=_primitives.end();
            ++itr)
        {
            const PrimitiveSet* primitiveset = itr->get();
            GLenum mode=primitiveset->getMode();
            switch(primitiveset->getType())
            {
                case(PrimitiveSet::DrawArraysPrimitiveType):
                {
                    const DrawArrays* drawArray = static_cast<const DrawArrays*>(primitiveset);
                    functor.begin(mode);

                    unsigned int indexEnd = drawArray->getFirst()+drawArray->getCount();
                    for(unsigned int vindex=drawArray->getFirst();
                        vindex<indexEnd;
                        ++vindex)
                    {
                        functor.vertex(_vertexData.indices->index(vindex));
                    }
                    
                    functor.end();
                    break;
                }
                case(PrimitiveSet::DrawArrayLengthsPrimitiveType):
                {

                    const DrawArrayLengths* drawArrayLengths = static_cast<const DrawArrayLengths*>(primitiveset);
                    unsigned int vindex=drawArrayLengths->getFirst();
                    for(DrawArrayLengths::const_iterator primItr=drawArrayLengths->begin();
                        primItr!=drawArrayLengths->end();
                        ++primItr)
                    {

                        functor.begin(mode);

                        for(GLsizei primCount=0;primCount<*primItr;++primCount)
                        {
                            functor.vertex(_vertexData.indices->index(vindex));
                            ++vindex;
                        }
                        
                        functor.end();

                    }
                    break;
                }
                case(PrimitiveSet::DrawElementsUBytePrimitiveType):
                {
                    const DrawElementsUByte* drawElements = static_cast<const DrawElementsUByte*>(primitiveset);
                    functor.begin(mode);

                    unsigned int primCount=0;
                    for(DrawElementsUByte::const_iterator primItr=drawElements->begin();
                        primItr!=drawElements->end();
                        ++primCount,++primItr)
                    {
                        unsigned int vindex=*primItr;
                        functor.vertex(_vertexData.indices->index(vindex));
                    }

                    functor.end();
                    break;
                }
                case(PrimitiveSet::DrawElementsUShortPrimitiveType):
                {
                    const DrawElementsUShort* drawElements = static_cast<const DrawElementsUShort*>(primitiveset);
                    functor.begin(mode);

                    for(DrawElementsUShort::const_iterator primItr=drawElements->begin();
                        primItr!=drawElements->end();
                        ++primItr)
                    {
                        unsigned int vindex=*primItr;
                        functor.vertex(_vertexData.indices->index(vindex));
                    }

                    functor.end();
                    break;
                }
                case(PrimitiveSet::DrawElementsUIntPrimitiveType):
                {
                    const DrawElementsUInt* drawElements = static_cast<const DrawElementsUInt*>(primitiveset);
                    functor.begin(mode);

                    for(DrawElementsUInt::const_iterator primItr=drawElements->begin();
                        primItr!=drawElements->end();
                        ++primItr)
                    {
                        unsigned int vindex=*primItr;
                        functor.vertex(_vertexData.indices->index(vindex));
                    }

                    functor.end();
                    break;
                }
                default:
                {
                    break;
                }
            }
        }
    }
    return;
}

unsigned int _computeNumberOfPrimtives(const osg::Geometry& geom)
{

    unsigned int totalNumberOfPrimitives = 0;
    
    for(Geometry::PrimitiveSetList::const_iterator itr=geom.getPrimitiveSetList().begin();
        itr!=geom.getPrimitiveSetList().end();
        ++itr)
    {
        const PrimitiveSet* primitiveset = itr->get();
        GLenum mode=primitiveset->getMode();

        unsigned int primLength;
        switch(mode)
        {
            case(GL_POINTS):    primLength=1; osg::notify(osg::INFO)<<"prim=GL_POINTS"<<std::endl; break;
            case(GL_LINES):     primLength=2; osg::notify(osg::INFO)<<"prim=GL_LINES"<<std::endl; break;
            case(GL_TRIANGLES): primLength=3; osg::notify(osg::INFO)<<"prim=GL_TRIANGLES"<<std::endl; break;
            case(GL_QUADS):     primLength=4; osg::notify(osg::INFO)<<"prim=GL_QUADS"<<std::endl; break;
            default:            primLength=0; osg::notify(osg::INFO)<<"prim="<<std::hex<<mode<<std::dec<<std::endl; break; // compute later when =0.
        }

        // draw primtives by the more flexible "slow" path,
        // sending OpenGL glBegin/glVertex.../glEnd().
        switch(primitiveset->getType())
        {
            case(PrimitiveSet::DrawArrayLengthsPrimitiveType):
            {

                const DrawArrayLengths* drawArrayLengths = static_cast<const DrawArrayLengths*>(primitiveset);
                for(DrawArrayLengths::const_iterator primItr=drawArrayLengths->begin();
                    primItr!=drawArrayLengths->end();
                    ++primItr)
                {
                    if (primLength==0) totalNumberOfPrimitives += 1;
                    else totalNumberOfPrimitives += *primItr/primLength;
                }
                break;
            }
            default:
            {
                if (primLength==0) { totalNumberOfPrimitives += 1; osg::notify(osg::INFO)<<"   totalNumberOfPrimitives="<<totalNumberOfPrimitives<<std::endl;}
                else { totalNumberOfPrimitives += primitiveset->getNumIndices()/primLength; osg::notify(osg::INFO)<<"   primitiveset->getNumIndices()="<<primitiveset->getNumIndices()<<" totalNumberOfPrimitives="<<totalNumberOfPrimitives<<std::endl; }

            }
        }
    }

    return totalNumberOfPrimitives;
}

template<class A>
bool _verifyBindings(const osg::Geometry& geom, const A& arrayData)
{
    unsigned int numElements = arrayData.indices.valid()?arrayData.indices->getNumElements():
                               arrayData.array.valid()?arrayData.array->getNumElements():0;

    switch(arrayData.binding)
    {
        case(osg::Geometry::BIND_OFF):
            if (numElements>0) return false;
            break;
        case(osg::Geometry::BIND_OVERALL):
            if (numElements!=1) return false;
            break;
        case(osg::Geometry::BIND_PER_PRIMITIVE_SET):
            if (numElements!=geom.getPrimitiveSetList().size()) return false;
            break;
        case(osg::Geometry::BIND_PER_PRIMITIVE):
            if (numElements!=_computeNumberOfPrimtives(geom)) return false;
            break;
        case(osg::Geometry::BIND_PER_VERTEX):
        {
            unsigned int numVertices = geom.getVertexIndices()?geom.getVertexIndices()->getNumElements():
                                       geom.getVertexArray()?geom.getVertexArray()->getNumElements():0;
            if (numElements!=numVertices) return false;        
            break;
        }
    } 
    return true;
}

template<class A>
void _computeCorrectBindingsAndArraySizes(std::ostream& out, const osg::Geometry& geom, A& arrayData, const char* arrayName)
{
    unsigned int numElements = arrayData.indices.valid()?arrayData.indices->getNumElements():
                               arrayData.array.valid()?arrayData.array->getNumElements():0;

    // check to see if binding matches 0 elements required.
    if (numElements==0)
    {
        // correct binding if not correct.
        if (arrayData.binding!=osg::Geometry::BIND_OFF)
        { 
            out<<"Warning: in osg::Geometry::computeCorrectBindingsAndArraySizes() "<<std::endl
                   <<"         "<<arrayName<<" binding has been reset to BIND_OFF"<<std::endl;
            arrayData.binding=osg::Geometry::BIND_OFF;
        }
        return;
    }

    // check to see if binding matches 1 elements required.
    if (numElements==1)
    {
        // correct binding if not correct.
        if (arrayData.binding!=osg::Geometry::BIND_OVERALL)
        { 
            out<<"Warning: in osg::Geometry::computeCorrectBindingsAndArraySizes() "<<std::endl
                   <<"         "<<arrayName<<" binding has been reset to BIND_OVERALL"<<std::endl;
            arrayData.binding=osg::Geometry::BIND_OVERALL;
        }
        return;
    }


    unsigned int numVertices = geom.getVertexIndices()?geom.getVertexIndices()->getNumElements():
                               geom.getVertexArray()?geom.getVertexArray()->getNumElements():0;
    
    if ( numVertices==0 )
    {
        if (arrayData.binding!=osg::Geometry::BIND_OFF)
        {
            arrayData.array = 0;
            arrayData.indices = 0;
            arrayData.binding = osg::Geometry::BIND_OFF;
            out<<"Warning: in osg::Geometry::computeCorrectBindingsAndArraySizes() vertex array is empty but "<<std::endl
                <<"         vertex array is empty but"<<arrayName<<" is set"<<std::endl
                <<"         reseting "<<arrayName<< " binding to BIND_OFF and array & indices to 0."<<std::endl;
        }
    }
    
    if (numElements==numVertices)
    {
        // correct the binding to per vertex.
        if (arrayData.binding!=osg::Geometry::BIND_PER_VERTEX)
        {
            out<<"Warning: in osg::Geometry::computeCorrectBindingsAndArraySizes() "<<std::endl
                   <<"         "<<arrayName<<" binding has been reset to BIND_PER_VERTEX"<<std::endl;
            arrayData.binding = osg::Geometry::BIND_PER_VERTEX;
        }
        return;
    }



    // check to see if binding might be per primitive set    
    unsigned int numPrimitiveSets = geom.getPrimitiveSetList().size();
    
    if (numElements==numPrimitiveSets)
    {
        if (arrayData.binding != osg::Geometry::BIND_PER_PRIMITIVE_SET)
        {
            out<<"Warning: in osg::Geometry::computeCorrectBindingsAndArraySizes() "<<std::endl
                   <<"         "<<arrayName<<" binding has been reset to BIND_PER_PRIMITIVE_SET"<<std::endl;
            arrayData.binding = osg::Geometry::BIND_PER_PRIMITIVE_SET;
        }
        return;
    }
    
    // check to see if binding might be per primitive   
    unsigned int numPrimitives = _computeNumberOfPrimtives(geom);
    if (numElements==numPrimitives)
    {
        if (arrayData.binding != osg::Geometry::BIND_PER_PRIMITIVE)
        {
            out<<"Warning: in osg::Geometry::computeCorrectBindingsAndArraySizes() "<<std::endl
                   <<"         "<<arrayName<<" binding has been reset to BIND_PER_PRIMITIVE"<<std::endl;
            arrayData.binding = osg::Geometry::BIND_PER_PRIMITIVE;
        }
        return;
    }

    if (numElements>numVertices)
    {
        arrayData.binding = osg::Geometry::BIND_PER_VERTEX;
        return;
    }
    if (numElements>numPrimitives)
    {
        arrayData.binding = osg::Geometry::BIND_PER_PRIMITIVE;
        return;
    }
    if (numElements>numPrimitiveSets)
    {
        arrayData.binding = osg::Geometry::BIND_PER_PRIMITIVE_SET;
        return;
    }
    if (numElements>=1)
    {
        arrayData.binding = osg::Geometry::BIND_OVERALL;
        return;
    }
    arrayData.binding = osg::Geometry::BIND_OFF;
 
}        

bool Geometry::verifyBindings(const ArrayData& arrayData) const
{
    return _verifyBindings(*this,arrayData);
}

bool Geometry::verifyBindings(const Vec3ArrayData& arrayData) const
{
    return _verifyBindings(*this,arrayData);
}

void Geometry::computeCorrectBindingsAndArraySizes(ArrayData& arrayData, const char* arrayName)
{
    _computeCorrectBindingsAndArraySizes(osg::notify(osg::INFO),*this,arrayData,arrayName);
}

void Geometry::computeCorrectBindingsAndArraySizes(Vec3ArrayData& arrayData, const char* arrayName)
{
    _computeCorrectBindingsAndArraySizes(osg::notify(osg::INFO),*this,arrayData,arrayName);
}

bool Geometry::verifyBindings() const
{
    if (!verifyBindings(_normalData)) return false;
    if (!verifyBindings(_colorData)) return false;
    if (!verifyBindings(_secondaryColorData)) return false;
    if (!verifyBindings(_fogCoordData)) return false;

    for(ArrayList::const_iterator titr=_texCoordList.begin();
        titr!=_texCoordList.end();
        ++titr)
    {
        if (!verifyBindings(*titr)) return false;
    }

    for(ArrayList::const_iterator vitr=_vertexAttribList.begin();
        vitr!=_vertexAttribList.end();
        ++vitr)
    {
        if (!verifyBindings(*vitr)) return false;
    }

    return true;
}

void Geometry::computeCorrectBindingsAndArraySizes()
{
    // if (verifyBindings()) return;
    
    computeCorrectBindingsAndArraySizes(_normalData,"_normalData");
    computeCorrectBindingsAndArraySizes(_colorData,"_colorData");
    computeCorrectBindingsAndArraySizes(_secondaryColorData,"_secondaryColorData");
    computeCorrectBindingsAndArraySizes(_fogCoordData,"_fogCoordData");

    for(ArrayList::iterator titr=_texCoordList.begin();
        titr!=_texCoordList.end();
        ++titr)
    {
        computeCorrectBindingsAndArraySizes(*titr,"_texCoordList[]");
    }

    for(ArrayList::iterator vitr=_vertexAttribList.begin();
        vitr!=_vertexAttribList.end();
        ++vitr)
    {
        computeCorrectBindingsAndArraySizes(*vitr,"_vertAttribList[]");
    }
}

class ExpandIndexedArray : public osg::ConstArrayVisitor
{
    public:
        ExpandIndexedArray(const osg::IndexArray& indices,Array* targetArray):
            _indices(indices),
            _targetArray(targetArray) {}
        
        virtual ~ExpandIndexedArray() {}

        template <class T,class I>
        T* create(const T& array,const I& indices)
        {
            T* newArray = 0;

            // if source array type and target array type are equal but arrays arn't equal
            if (_targetArray && _targetArray->getType()==array.getType() && _targetArray!=(osg::Array*)(&array))
            {
                // reuse exisiting target array 
                newArray = static_cast<T*>(_targetArray);
                if (newArray->size()!=indices.size())
                {
                    // make sure its the right size
                    newArray->resize(indices.size());
                }
            }
            else
            {
                //  else create a new array.
                newArray = new T(indices.size());
            }

            for(unsigned int i=0;i<indices.size();++i)
            {
                (*newArray)[i]= array[indices[i]];
            }

            return newArray;
        }

        template <class T>
        T* create(const T& array)
        {
            switch(_indices.getType())
            {
            case(osg::Array::ByteArrayType): return create(array,static_cast<const osg::ByteArray&>(_indices));
            case(osg::Array::ShortArrayType): return create(array,static_cast<const osg::ShortArray&>(_indices));
            case(osg::Array::IntArrayType): return create(array,static_cast<const osg::IntArray&>(_indices));
            case(osg::Array::UByteArrayType): return create(array,static_cast<const osg::UByteArray&>(_indices));
            case(osg::Array::UShortArrayType): return create(array,static_cast<const osg::UShortArray&>(_indices));
            case(osg::Array::UIntArrayType): return create(array,static_cast<const osg::UIntArray&>(_indices));
            default: return 0;
            }
            
        }

        virtual void apply(const osg::ByteArray& array) { _targetArray = create(array); }
        virtual void apply(const osg::ShortArray& array) { _targetArray = create(array); }
        virtual void apply(const osg::IntArray& array) { _targetArray = create(array); }
        virtual void apply(const osg::UByteArray& array) { _targetArray = create(array); }
        virtual void apply(const osg::UShortArray& array) { _targetArray = create(array); }
        virtual void apply(const osg::UIntArray& array) { _targetArray = create(array); }
        virtual void apply(const osg::Vec4ubArray& array) { _targetArray = create(array); }
        virtual void apply(const osg::FloatArray& array) { _targetArray = create(array); }
        virtual void apply(const osg::Vec2Array& array) { _targetArray = create(array); }
        virtual void apply(const osg::Vec3Array& array) { _targetArray = create(array); }
        virtual void apply(const osg::Vec4Array& array) { _targetArray = create(array); }

        const osg::IndexArray& _indices;
        osg::Array* _targetArray;
};

bool Geometry::suitableForOptimization() const
{
    bool hasIndices = false;

    if (getVertexIndices()) hasIndices = true;

    if (getNormalIndices()) hasIndices = true;

    if (getColorIndices()) hasIndices = true;

    if (getSecondaryColorIndices()) hasIndices = true;

    if (getFogCoordIndices()) hasIndices = true;

    for(unsigned int ti=0;ti<getNumTexCoordArrays();++ti)
    {
        if (getTexCoordIndices(ti)) hasIndices = true;
    }
    
    for(unsigned int vi=0;vi<getNumVertexAttribArrays();++vi)
    {
        if (getVertexAttribIndices(vi)) hasIndices = true;
    }

    return hasIndices;
}

void Geometry::copyToAndOptimize(Geometry& target)
{
    bool copyToSelf = (this==&target);

    // copy over primitive sets.
    if (!copyToSelf) target.getPrimitiveSetList() = getPrimitiveSetList();

    // copy over attribute arrays.
    if (getVertexIndices())
    {
        ExpandIndexedArray eia(*(getVertexIndices()),target.getVertexArray());
        getVertexArray()->accept(eia);

        target.setVertexArray(eia._targetArray);
        target.setVertexIndices(0);
    }
    else if (getVertexArray())
    {
        if (!copyToSelf) target.setVertexArray(getVertexArray());
    }

    target.setNormalBinding(getNormalBinding());
    if (getNormalIndices())
    {
        ExpandIndexedArray eia(*(getNormalIndices()),target.getNormalArray());
        getNormalArray()->accept(eia);

        target.setNormalArray(dynamic_cast<osg::Vec3Array*>(eia._targetArray));
        target.setNormalIndices(0);
    }
    else if (getNormalArray())
    {
        if (!copyToSelf) target.setNormalArray(getNormalArray());
    }

    target.setColorBinding(getColorBinding());
    if (getColorIndices())
    {
        ExpandIndexedArray eia(*(getColorIndices()),target.getColorArray());
        getColorArray()->accept(eia);

        target.setColorArray(eia._targetArray);
        target.setColorIndices(0);
    }
    else if (getColorArray())
    {
        if (!copyToSelf) target.setColorArray(getColorArray());
    }

    target.setSecondaryColorBinding(getSecondaryColorBinding());
    if (getSecondaryColorIndices())
    {
        ExpandIndexedArray eia(*(getSecondaryColorIndices()),target.getSecondaryColorArray());
        getSecondaryColorArray()->accept(eia);

        target.setSecondaryColorArray(eia._targetArray);
        target.setSecondaryColorIndices(0);
    }
    else if (getSecondaryColorArray())
    {
        if (!copyToSelf) target.setSecondaryColorArray(getSecondaryColorArray());
    }

    target.setFogCoordBinding(getFogCoordBinding());
    if (getFogCoordIndices())
    {
        ExpandIndexedArray eia(*(getFogCoordIndices()),target.getFogCoordArray());
        getFogCoordArray()->accept(eia);

        target.setFogCoordArray(eia._targetArray);
        target.setFogCoordIndices(0);
    }
    else if (getFogCoordArray())
    {
        if (!copyToSelf) target.setFogCoordArray(getFogCoordArray());
    }

    for(unsigned int ti=0;ti<getNumTexCoordArrays();++ti)
    {
        if (getTexCoordIndices(ti))
        {
            ExpandIndexedArray eia(*(getTexCoordIndices(ti)),target.getTexCoordArray(ti));
            getTexCoordArray(ti)->accept(eia);

            target.setTexCoordArray(ti,eia._targetArray);
            target.setTexCoordIndices(ti,0);
        }
        else if (getTexCoordArray(ti)) 
        {
            if (!copyToSelf) target.setTexCoordArray(ti,getTexCoordArray(ti));
        }
    }
    
    for(unsigned int vi=0;vi<_vertexAttribList.size();++vi)
    {
        ArrayData& arrayData = _vertexAttribList[vi];
        if (arrayData.indices.valid())
        {
            ExpandIndexedArray eia(*arrayData.indices,target.getVertexAttribArray(vi));
            arrayData.array->accept(eia);
            target.setVertexAttribData(vi,ArrayData(eia._targetArray, 0, arrayData.binding, arrayData.normalize));
        }
        else if (arrayData.array.valid())
        {
            if (!copyToSelf) target.setVertexAttribData(vi,arrayData);
        }
    }
}

void Geometry::computeInternalOptimizedGeometry()
{
    if (suitableForOptimization())
    {
        if (!_internalOptimizedGeometry) _internalOptimizedGeometry = new Geometry;

        copyToAndOptimize(*_internalOptimizedGeometry);
    }
}


Geometry* osg::createTexturedQuadGeometry(const Vec3& corner,const Vec3& widthVec,const Vec3& heightVec, float l, float b, float r, float t)
{
    Geometry* geom = new Geometry;

    Vec3Array* coords = new Vec3Array(4);
    (*coords)[0] = corner+heightVec;
    (*coords)[1] = corner;
    (*coords)[2] = corner+widthVec;
    (*coords)[3] = corner+widthVec+heightVec;
    geom->setVertexArray(coords);

    Vec2Array* tcoords = new Vec2Array(4);
    (*tcoords)[0].set(l,t);
    (*tcoords)[1].set(l,b);
    (*tcoords)[2].set(r,b);
    (*tcoords)[3].set(r,t);
    geom->setTexCoordArray(0,tcoords);

    osg::Vec4Array* colours = new osg::Vec4Array(1);
    (*colours)[0].set(1.0f,1.0f,1.0,1.0f);
    geom->setColorArray(colours);
    geom->setColorBinding(Geometry::BIND_OVERALL);

    osg::Vec3Array* normals = new osg::Vec3Array(1);
    (*normals)[0] = widthVec^heightVec;
    (*normals)[0].normalize();
    geom->setNormalArray(normals);
    geom->setNormalBinding(Geometry::BIND_OVERALL);

    geom->addPrimitiveSet(new DrawArrays(PrimitiveSet::QUADS,0,4));
    
    return geom;
}




///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////


// experimental templated rendering code, please ignore...
// will move to osg::Geometry once complete.
// Robert Osfield, August 2003.
#if 0

    struct DrawAttributeArrays 
    {
        virtual bool valid() const = 0;
        virtual void set(osg::Geometry* geometry) = 0;
        virtual unsigned int draw(unsigned int index, unsigned int count) const = 0;
    };

    struct V3
    {
        V3():_array(0) {}

        bool valid() const { return _array!=0; }

        void set(osg::Geometry* geometry)
        {
            _array = 0;
            osg::Array* array = geometry->getVertexArray();
            if (array && array->getType()==osg::Array::Vec3ArrayType)
            {
                osg::Vec3Array* vec3array = static_cast<osg::Vec3Array*>(array);
                if (!vec3array->empty()) _array = &(vec3array->front());
            }
        }

        inline void draw(unsigned int index) const
        {
            glVertex3fv(_array[index].ptr());
        }

        osg::Vec3* _array;
    };

    struct V3USI
    {
        V3USI():_array(0),_indices(0) {}

        bool valid() const { return _array!=0 && _indices!=0; }

        void set(osg::Geometry* geometry)
        {
            _array = 0;
            osg::Array* array = geometry->getVertexArray();
            if (array && array->getType()==osg::Array::Vec3ArrayType)
            {
                osg::Vec3Array* vec3array = static_cast<osg::Vec3Array*>(array);
                if (!vec3array->empty()) _array = &(vec3array->front());
            }

            _indices = 0;
            osg::IndexArray* indices = geometry->getVertexIndices();
            if (indices && indices->getType()==osg::Array::UShortArrayType)
            {
                osg::UShortArray* ushort3array = static_cast<osg::UShortArray*>(array);
                if (!ushort3array->empty()) _indices = &(ushort3array->front());
            }
        }

        inline void draw(unsigned int index) const
        {
            glVertex3fv(_array[_indices[index]].ptr());
        }

        osg::Vec3*      _array;
        unsigned short* _indices;
    };

    //////////////////////////////

    struct N3
    {
        N3():_array(0) {}

        bool valid() const { return _array!=0; }

        void set(osg::Geometry* geometry)
        {
            _array = 0;
            osg::Array* array = geometry->getVertexArray();
            if (array && array->getType()==osg::Array::Vec3ArrayType)
            {
                osg::Vec3Array* vec3array = static_cast<osg::Vec3Array*>(array);
                if (!vec3array->empty()) _array = &(vec3array->front());
            }
        }

        inline void draw(unsigned int index) const
        {
            glNormal3fv(_array[index].ptr());
        }

        osg::Vec3* _array;
    };

    struct N3USI
    {
        N3USI():_array(0),_indices(0) {}

        bool valid() const { return _array!=0 && _indices!=0; }

        void set(osg::Geometry* geometry)
        {
            _array = 0;
            osg::Array* array = geometry->getVertexArray();
            if (array && array->getType()==osg::Array::Vec3ArrayType)
            {
                osg::Vec3Array* vec3array = static_cast<osg::Vec3Array*>(array);
                if (!vec3array->empty()) _array = &(vec3array->front());
            }

            _indices = 0;
            osg::IndexArray* indices = geometry->getVertexIndices();
            if (indices && indices->getType()==osg::Array::UShortArrayType)
            {
                osg::UShortArray* ushortarray = static_cast<osg::UShortArray*>(array);
                if (!ushortarray->empty()) _indices = &(ushortarray->front());
            }
        }

        inline void draw(unsigned int index) const
        {
            glNormal3fv(_array[_indices[index]].ptr());
        }

        osg::Vec3*      _array;
        unsigned short* _indices;
    };

    //////////////////////////////

    struct C4
    {
        C4():_array(0) {}

        bool valid() const { return _array!=0; }

        void set(osg::Geometry* geometry)
        {
            _array = 0;
            osg::Array* array = geometry->getColorArray();
            if (array && array->getType()==osg::Array::Vec4ArrayType)
            {
                osg::Vec4Array* vec4array = static_cast<osg::Vec4Array*>(array);
                if (!vec4array->empty()) _array = &(vec4array->front());
            }
        }

        inline void draw(unsigned int index) const
        {
            glVertex3fv(_array[index].ptr());
        }

        osg::Vec4* _array;
    };

    struct C4USI
    {
        C4USI():_array(0),_indices(0) {}

        bool valid() const { return _array!=0 && _indices!=0; }

        void set(osg::Geometry* geometry)
        {
            _array = 0;
            osg::Array* array = geometry->getColorArray();
            if (array && array->getType()==osg::Array::Vec4ArrayType)
            {
                osg::Vec4Array* vec4array = static_cast<osg::Vec4Array*>(array);
                if (!vec4array->empty()) _array = &(vec4array->front());
            }

            _indices = 0;
            osg::IndexArray* indices = geometry->getColorIndices();
            if (indices && indices->getType()==osg::Array::UShortArrayType)
            {
                osg::UShortArray* ushortarray = static_cast<osg::UShortArray*>(array);
                if (!ushortarray->empty()) _indices = &(ushortarray->front());
            }
        }

        inline void draw(unsigned int index) const
        {
            glColor4fv(_array[_indices[index]].ptr());
        }

        osg::Vec4*      _array;
        unsigned short* _indices;
    };

    //////////////////////////////

    struct T2
    {
        T2():_array(0) {}

        bool valid() const { return _array!=0; }

        void set(osg::Geometry* geometry)
        {
            _array = 0;
            osg::Array* array = geometry->getTexCoordArray(0);
            if (array && array->getType()==osg::Array::Vec2ArrayType)
            {
                osg::Vec2Array* vec2array = static_cast<osg::Vec2Array*>(array);
                if (!vec2array->empty()) _array = &(vec2array->front());
            }
        }

        inline void draw(unsigned int index) const
        {
            glTexCoord2fv(_array[index].ptr());
        }

        osg::Vec2* _array;
    };

    struct T2USI
    {
        T2USI():_array(0),_indices(0) {}

        bool valid() const { return _array!=0 && _indices!=0; }

        void set(osg::Geometry* geometry)
        {
            _array = 0;
            osg::Array* array = geometry->getTexCoordArray(0);
            if (array && array->getType()==osg::Array::Vec2ArrayType)
            {
                osg::Vec2Array* vec2array = static_cast<osg::Vec2Array*>(array);
                if (!vec2array->empty()) _array = &(vec2array->front());
            }

            _indices = 0;
            osg::IndexArray* indices = geometry->getTexCoordIndices(0);
            if (indices && indices->getType()==osg::Array::UShortArrayType)
            {
                osg::UShortArray* ushortarray = static_cast<osg::UShortArray*>(array);
                if (!ushortarray->empty()) _indices = &(ushortarray->front());
            }
        }

        inline void draw(unsigned int index) const
        {
            glTexCoord2fv(_array[_indices[index]].ptr());
        }

        osg::Vec2*      _array;
        unsigned short* _indices;
    };


    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

    template < class T1 >
    struct DrawAttributeArrays_T : public DrawAttributeArrays
    {
        DrawAttributeArrays_T(osg::Geometry* geometry)
        {

        }

        virtual bool valid() const { return _t1.valid(); }

        virtual void set(osg::Geometry* geometry)
        {
            _t1.set(geometry);
        }

        virtual unsigned int draw(unsigned int index, unsigned int count) const
        {
            for(unsigned int i=0;i<count;++i,++index)
            {
                _t1.draw(index);
            }
            return index;
        }

        T1 _t1;
    };

    template < class T1, class T2 >
    struct DrawAttributeArrays_TT : public DrawAttributeArrays
    {
        DrawAttributeArrays_TT()
        {
        }

        virtual bool valid() const { return _t1.valid() && _t2.valid(); }

        virtual void set(osg::Geometry* geometry)
        {
            _t1.set(geometry);
            _t2.set(geometry);
        }

        virtual unsigned int draw(unsigned int index, unsigned int count) const
        {
            for(unsigned int i=0;i<count;++i,++index)
            {
                _t1.draw(index);
                _t2.draw(index);
            }
            return index;
        }

        T1 _t1;
        T1 _t2;
    };

    template < class T1, class T2, class T3 >
    struct DrawAttributeArrays_TTT : public DrawAttributeArrays
    {
        DrawAttributeArrays_TTT()
        {
        }

        virtual bool valid() const { return _t1.valid() && _t2.valid() && _t3.valid(); }

        virtual void set(osg::Geometry* geometry)
        {
            _t1.set(geometry);
            _t2.set(geometry);
            _t3.set(geometry);
        }

        virtual unsigned int draw(unsigned int index, unsigned int count) const
        {
            for(unsigned int i=0;i<count;++i,++index)
            {
                _t1.draw(index);
                _t2.draw(index);
                _t3.draw(index);
            }
            return index;
        }

        T1 _t1;
        T2 _t2;
        T3 _t3;
    };

    template < class T1, class T2, class T3, class T4 >
    struct DrawAttributeArrays_TTTT : public DrawAttributeArrays
    {
        DrawAttributeArrays_TTTT()
        {
        }

        virtual bool valid() const { return _t1.valid() && _t2.valid() && _t3.valid() && _t4.valid(); }

        virtual void set(osg::Geometry* geometry)
        {
            _t1.set(geometry);
            _t2.set(geometry);
            _t3.set(geometry);
            _t4.set(geometry);
        }

        virtual unsigned int draw(unsigned int index, unsigned int count) const
        {
            for(unsigned int i=0;i<count;++i,++index)
            {
                _t1.draw(index);
                _t2.draw(index);
                _t3.draw(index);
                _t4.draw(index);
            }
            return index;
        }

        T1 _t1;
        T2 _t2;
        T3 _t3;
        T4 _t4;
    };

    template < class T1, class T2 >
    struct DrawAttributeArrays_TT_USI : public DrawAttributeArrays
    {
        DrawAttributeArrays_TT_USI()
        {
        }

        virtual bool valid() const { return _t1.valid() && _t2.valid() && _indices!=0; }

        virtual void set(osg::Geometry* geometry)
        {
            _t1.set(geometry);
            _t2.set(geometry);

            _indices = 0;
            osg::IndexArray* indices = geometry->getVertexIndices();
            if (indices && indices->getType()==osg::Array::UShortArrayType)
            {
                osg::UShortArray* ushort3array = static_cast<osg::UShortArray*>(array);
                if (!ushort3array->empty()) _indices = &(ushort3array->front());
            }
        }

        virtual unsigned int draw(unsigned int index, unsigned int count) const
        {
            for(unsigned int i=0;i<count;++i,++index)
            {
                unsigned int ivalue = _indices[index];
                _t1.draw(ivalue);
                _t2.draw(ivalue);
            }
            return index;
        }

        T1 _t1;
        T2 _t2;
    };

    template < class T1, class T2, class T3 >
    struct DrawAttributeArrays_TTT_USI : public DrawAttributeArrays
    {
        DrawAttributeArrays_TTT_USI()
        {
        }

        virtual bool valid() const { return _t1.valid() && _t2.valid() && _t3.valid() && _indices!=0; }

        virtual void set(osg::Geometry* geometry)
        {
            _t1.set(geometry);
            _t2.set(geometry);
            _t3.set(geometry);

            _indices = 0;
            osg::IndexArray* indices = geometry->getVertexIndices();
            if (indices && indices->getType()==osg::Array::UShortArrayType)
            {
                osg::UShortArray* ushort3array = static_cast<osg::UShortArray*>(array);
                if (!ushort3array->empty()) _indices = &(ushort3array->front());
            }
        }

        virtual unsigned int draw(unsigned int index, unsigned int count) const
        {
            for(unsigned int i=0;i<count;++i,++index)
            {
                unsigned int ivalue = _indices[index];
                _t1.draw(ivalue);
                _t2.draw(ivalue);
                _t3.draw(ivalue);
            }
            return index;
        }

        T1 _t1;
        T2 _t2;
        T3 _t3;
    };

    template < class T1, class T2, class T3, class T4 >
    struct DrawAttributeArrays_TTTT_USI : public DrawAttributeArrays
    {
        DrawAttributeArrays_TTTT_USI()
        {
        }

        virtual bool valid() const { return _t1.valid() && _t2.valid() && _t3.valid() && _t4.valid() && _indices!=0; }

        virtual void set(osg::Geometry* geometry)
        {
            _t1.set(geometry);
            _t2.set(geometry);
            _t3.set(geometry);
            _t4.set(geometry);

            _indices = 0;
            osg::IndexArray* indices = geometry->getVertexIndices();
            if (indices && indices->getType()==osg::Array::UShortArrayType)
            {
                osg::UShortArray* ushort3array = static_cast<osg::UShortArray*>(array);
                if (!ushort3array->empty()) _indices = &(ushort3array->front());
            }
        }

        virtual unsigned int draw(unsigned int index, unsigned int count) const
        {
            for(unsigned int i=0;i<count;++i,++index)
            {
                unsigned int ivalue = _indices[index];
                _t1.draw(ivalue);
                _t2.draw(ivalue);
                _t3.draw(ivalue);
                _t4.draw(ivalue);
            }
            return index;
        }

        T1 _t1;
        T2 _t2;
        T3 _t3;
        T4 _t4;
    };

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


    // One attribute x 2

    typedef DrawAttributeArrays_T<V3>                    DrawAttributeArrays_V3;
    typedef DrawAttributeArrays_T<V3USI>                 DrawAttributeArrays_V3i;

    // Two attributes x 15

    typedef DrawAttributeArrays_TT<N3,V3>                DrawAttributeArrays_N3V3;
    typedef DrawAttributeArrays_TT<N3USI,V3>             DrawAttributeArrays_N3iV3;
    typedef DrawAttributeArrays_TT<N3,V3USI>             DrawAttributeArrays_N3V3i;
    typedef DrawAttributeArrays_TT<N3USI,V3USI>          DrawAttributeArrays_N3iV3i;

    typedef DrawAttributeArrays_TT_USI<N3,V3>            DrawAttributeArrays_N3V3_i;

    typedef DrawAttributeArrays_TT<C4,V3>                DrawAttributeArrays_C4V3;
    typedef DrawAttributeArrays_TT<C4USI,V3>             DrawAttributeArrays_C4iV3;
    typedef DrawAttributeArrays_TT<C4,V3USI>             DrawAttributeArrays_C4V3i;
    typedef DrawAttributeArrays_TT<C4USI,V3USI>          DrawAttributeArrays_C4iV3i;

    typedef DrawAttributeArrays_TT_USI<C4,V3>            DrawAttributeArrays_C4V3_i;

    typedef DrawAttributeArrays_TT<T2,V3>                DrawAttributeArrays_T2V3;
    typedef DrawAttributeArrays_TT<T2USI,V3>             DrawAttributeArrays_T2iV3;
    typedef DrawAttributeArrays_TT<T2,V3USI>             DrawAttributeArrays_T2V3i;
    typedef DrawAttributeArrays_TT<T2USI,V3USI>          DrawAttributeArrays_T2iV3i;

    typedef DrawAttributeArrays_TT_USI<T2,V3>            DrawAttributeArrays_T2V3_i;

    // Three attributes x 27

    typedef DrawAttributeArrays_TTT<C4,N3,V3>            DrawAttributeArrays_C4N3V3;
    typedef DrawAttributeArrays_TTT<C4USI,N3,V3>         DrawAttributeArrays_C4iN3V3;
    typedef DrawAttributeArrays_TTT<C4,N3USI,V3>         DrawAttributeArrays_C4N3iV3;
    typedef DrawAttributeArrays_TTT<C4USI,N3USI,V3>      DrawAttributeArrays_C4iN3iV3;

    typedef DrawAttributeArrays_TTT<C4,N3,V3USI>         DrawAttributeArrays_C4N3V3i;
    typedef DrawAttributeArrays_TTT<C4USI,N3,V3USI>      DrawAttributeArrays_C4iN3V3i;
    typedef DrawAttributeArrays_TTT<C4,N3USI,V3USI>      DrawAttributeArrays_C4N3iV3i;
    typedef DrawAttributeArrays_TTT<C4USI,N3USI,V3USI>   DrawAttributeArrays_C4iN3iV3i;

    typedef DrawAttributeArrays_TTT_USI<C4,N3,V3>        DrawAttributeArrays_C4N3V3_i;


    typedef DrawAttributeArrays_TTT<T2,N3,V3>            DrawAttributeArrays_T2N3V3;
    typedef DrawAttributeArrays_TTT<T2USI,N3,V3>         DrawAttributeArrays_T2iN3V3;
    typedef DrawAttributeArrays_TTT<T2,N3USI,V3>         DrawAttributeArrays_T2iN3iV3;
    typedef DrawAttributeArrays_TTT<T2USI,N3USI,V3>      DrawAttributeArrays_T2N3iV3;

    typedef DrawAttributeArrays_TTT<T2,N3,V3USI>         DrawAttributeArrays_T2N3V3i;
    typedef DrawAttributeArrays_TTT<T2USI,N3,V3USI>      DrawAttributeArrays_T2iN3V3i;
    typedef DrawAttributeArrays_TTT<T2,N3USI,V3USI>      DrawAttributeArrays_T2iN3iV3i;
    typedef DrawAttributeArrays_TTT<T2USI,N3USI,V3USI>   DrawAttributeArrays_T2N3iV3i;

    typedef DrawAttributeArrays_TTT_USI<T2,N3,V3>        DrawAttributeArrays_T2N3V3_i;


    typedef DrawAttributeArrays_TTT<T2,C4,V3>            DrawAttributeArrays_T2C4V3;
    typedef DrawAttributeArrays_TTT<T2USI,C4,V3>         DrawAttributeArrays_T2iC4V3;
    typedef DrawAttributeArrays_TTT<T2,C4USI,V3>         DrawAttributeArrays_T2C4iV3;
    typedef DrawAttributeArrays_TTT<T2USI,C4USI,V3>      DrawAttributeArrays_T2iC4iV3;

    typedef DrawAttributeArrays_TTT<T2,C4,V3USI>         DrawAttributeArrays_T2C4V3i;
    typedef DrawAttributeArrays_TTT<T2USI,C4,V3USI>      DrawAttributeArrays_T2iC4V3i;
    typedef DrawAttributeArrays_TTT<T2,C4USI,V3USI>      DrawAttributeArrays_T2C4iV3i;
    typedef DrawAttributeArrays_TTT<T2USI,C4USI,V3USI>   DrawAttributeArrays_T2iC4iV3i;

    typedef DrawAttributeArrays_TTT_USI<T2,C4,V3>        DrawAttributeArrays_T2C4V3_t;


    // Four attributes x 17

    typedef DrawAttributeArrays_TTTT<T2,C4,N3,V3>                DrawAttributeArrays_T2C4N3V3;
    typedef DrawAttributeArrays_TTTT<T2USI,C4,N3,V3>             DrawAttributeArrays_T2iC4N3V3;
    typedef DrawAttributeArrays_TTTT<T2,C4USI,N3,V3>             DrawAttributeArrays_T2C4iN3V3;
    typedef DrawAttributeArrays_TTTT<T2USI,C4USI,N3,V3>          DrawAttributeArrays_T2iC4iN3V3;

    typedef DrawAttributeArrays_TTTT<T2,C4,N3USI,V3>             DrawAttributeArrays_T2C4N3iV3;
    typedef DrawAttributeArrays_TTTT<T2USI,C4,N3USI,V3>          DrawAttributeArrays_T2iC4N3iV3;
    typedef DrawAttributeArrays_TTTT<T2,C4USI,N3USI,V3>           DrawAttributeArrays_T2C4iN3iV3;
    typedef DrawAttributeArrays_TTTT<T2USI,C4USI,N3USI,V3>       DrawAttributeArrays_T2iC4iN3iV3;

    typedef DrawAttributeArrays_TTTT<T2,C4,N3,V3USI>             DrawAttributeArrays_T2C4N3V3i;
    typedef DrawAttributeArrays_TTTT<T2USI,C4,N3,V3USI>          DrawAttributeArrays_T2iC4N3V3i;
    typedef DrawAttributeArrays_TTTT<T2,C4USI,N3,V3USI>          DrawAttributeArrays_T2C4iN3V3i;
    typedef DrawAttributeArrays_TTTT<T2USI,C4USI,N3,V3USI>       DrawAttributeArrays_T2iC4iN3V3i;

    typedef DrawAttributeArrays_TTTT<T2,C4,N3USI,V3USI>          DrawAttributeArrays_T2C4N3iV3i;
    typedef DrawAttributeArrays_TTTT<T2USI,C4,N3USI,V3USI>       DrawAttributeArrays_T2iC4N3iV3i;
    typedef DrawAttributeArrays_TTTT<T2,C4USI,N3USI,V3USI>       DrawAttributeArrays_T2C4iN3iV3i;
    typedef DrawAttributeArrays_TTTT<T2USI,C4USI,N3USI,V3USI>    DrawAttributeArrays_T2iC4iN3iV3i;

    typedef DrawAttributeArrays_TTTT_USI<T2,C4,N3,V3>            DrawAttributeArrays_T2C4N3V3_i;

    /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


#endif
