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
#include <osg/Geometry>
#include <osg/Notify>

using namespace osg;

#if 1
class DrawVertex
{
    public:
    
        DrawVertex(const Array* vertices,const IndexArray* indices):
            _vertices(vertices),
            _indices(indices)
        {
            _verticesType = _vertices->getType();
        }
    
        inline void operator () (unsigned int pos)
        {
            if (_indices) pos = _indices->index(pos);

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
            }
            
        }
        
        inline void apply(const Vec2& v)   { glVertex2fv(v.ptr()); }
        inline void apply(const Vec3& v)   { glVertex3fv(v.ptr()); }
        inline void apply(const Vec4& v)   { glVertex4fv(v.ptr()); }

        const Array*        _vertices;
        const IndexArray*   _indices;
        Array::Type         _verticesType;
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
    
        DrawNormal(const Vec3Array* normals,const IndexArray* indices):
            _normals(normals),
            _indices(indices) {}
    
        void operator () (unsigned int pos)
        {
            if (_indices) glNormal3fv((*_normals)[_indices->index(pos)].ptr());
            else glNormal3fv((*_normals)[pos].ptr());
        }
        
        const Vec3Array*   _normals;
        const IndexArray*  _indices;
};

#if 1
class DrawColor
{
    public:

        DrawColor(const Array* colors,const IndexArray* indices):
            _colors(colors),
            _indices(indices)
        {
            _colorsType = _colors->getType();
            _indicesType = _indices->getType();
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
            case(Array::UByte4ArrayType):
                apply((*static_cast<const UByte4Array*>(_colors))[pos]);
                break;
            case(Array::Vec3ArrayType):
                apply((*static_cast<const Vec3Array*>(_colors))[pos]);
                break;
            }
        }

        inline void apply(const UByte4& v) { glColor4ubv(v.ptr()); }
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

        virtual void apply(const UByte4& v) { glColor4ubv(v.ptr()); }
        virtual void apply(const Vec3& v)   { glColor3fv(v.ptr()); }
        virtual void apply(const Vec4& v)   { glColor4fv(v.ptr()); }
        
        const Array*        _colors;
        const IndexArray*   _indices;
};
#endif
class DrawVertexAttrib : public osg::Referenced, public osg::ConstValueVisitor
{
public:

    DrawVertexAttrib(const Drawable::Extensions * extensions,unsigned int index,GLboolean normalized,const Array* attribcoords,const IndexArray* indices):    
            _index(index),
            _normalized(normalized),
            _extensions(extensions),
            _attribcoords(attribcoords),
            _indices(indices) {;}

    inline void operator () (unsigned int pos)
    {
        if (_indices) _attribcoords->accept(_indices->index(pos),*this);
        else _attribcoords->accept(pos,*this);
    }

    virtual void apply(const GLshort& s) 
    {
        _extensions->glVertexAttrib1s( _index, s );
    }
    virtual void apply(const GLfloat& f) 
    {
        _extensions->glVertexAttrib1f( _index, f );
    }
    virtual void apply(const UByte4& v) 
    {
        if( _normalized )
        {
            _extensions->glVertexAttrib4Nubv( _index, v.ptr() );
        }
        else
        {
            _extensions->glVertexAttrib4ubv( _index, v.ptr() );
        }
    }
    virtual void apply(const Vec2& v) 
    {
        _extensions->glVertexAttrib2fv( _index, v.ptr() );
    }
    virtual void apply(const Vec3& v) 
    {
        _extensions->glVertexAttrib3fv( _index, v.ptr() );
    }
    virtual void apply(const Vec4& v) 
    {
        _extensions->glVertexAttrib4fv( _index, v.ptr() );
    }

    unsigned int                    _index;
    GLboolean                       _normalized;
    const Drawable::Extensions*     _extensions;
    const Array*                    _attribcoords;
    const IndexArray*               _indices;
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

        virtual void apply(const UByte4& v) { _extensions->glSecondaryColor3ubv(v.ptr()); }
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


Geometry::Geometry()
{
    _normalBinding = BIND_OFF;
    _colorBinding = BIND_OFF;
    _secondaryColorBinding = BIND_OFF;
    _fogCoordBinding = BIND_OFF;

    _fastPath = false;

    _fastPathHint = true;
}

Geometry::Geometry(const Geometry& geometry,const CopyOp& copyop):
    Drawable(geometry,copyop),
#ifdef COMPILE_POSSIBLE_NEW_ARRAY_METHODS
    _attributeList(geometry._attributeList),
#endif    
    _vertexArray((copyop(geometry._vertexArray.get()))),
    _vertexIndices(dynamic_cast<IndexArray*>(copyop(geometry._vertexIndices.get()))),
    _normalBinding(geometry._normalBinding),
    _normalArray(dynamic_cast<Vec3Array*>(copyop(geometry._normalArray.get()))),
    _normalIndices(dynamic_cast<IndexArray*>(copyop(geometry._normalIndices.get()))),
    _colorBinding(geometry._colorBinding),
    _colorArray(copyop(geometry._colorArray.get())),
    _colorIndices(dynamic_cast<IndexArray*>(copyop(geometry._colorIndices.get()))),
    _secondaryColorBinding(geometry._secondaryColorBinding),
    _secondaryColorArray(copyop(geometry._secondaryColorArray.get())),
    _secondaryColorIndices(dynamic_cast<IndexArray*>(copyop(geometry._secondaryColorIndices.get()))),
    _fogCoordBinding(geometry._fogCoordBinding),
    _fogCoordArray(dynamic_cast<FloatArray*>(copyop(geometry._fogCoordArray.get()))),
    _fogCoordIndices(dynamic_cast<IndexArray*>(copyop(geometry._fogCoordIndices.get()))),
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

    for(TexCoordArrayList::const_iterator titr=geometry._texCoordList.begin();
        titr!=geometry._texCoordList.end();
        ++titr)
    {
        _texCoordList.push_back(*titr);
    }

    for(VertexAttribArrayList::const_iterator vitr=geometry._vertexAttribList.begin();
        vitr!=geometry._vertexAttribList.end();
        ++vitr)
    {
        _vertexAttribList.push_back(*vitr);
    }
}

Geometry::~Geometry()
{
    // no need to delete, all automatically handled by ref_ptr :-)
}

void Geometry::setTexCoordArray(unsigned int unit,Array* array)
{
    if (_texCoordList.size()<=unit)
        _texCoordList.resize(unit+1);
        
   _texCoordList[unit].first = array;

    dirtyDisplayList();
}

Array* Geometry::getTexCoordArray(unsigned int unit)
{
    if (unit<_texCoordList.size()) return _texCoordList[unit].first.get();
    else return 0;
}

const Array* Geometry::getTexCoordArray(unsigned int unit) const
{
    if (unit<_texCoordList.size()) return _texCoordList[unit].first.get();
    else return 0;
}

void Geometry::setTexCoordIndices(unsigned int unit,IndexArray* array)
{
    if (_texCoordList.size()<=unit)
        _texCoordList.resize(unit+1);
        
   _texCoordList[unit].second = array;

    dirtyDisplayList();
}

IndexArray* Geometry::getTexCoordIndices(unsigned int unit)
{
    if (unit<_texCoordList.size()) return _texCoordList[unit].second.get();
    else return 0;
}

const IndexArray* Geometry::getTexCoordIndices(unsigned int unit) const
{
    if (unit<_texCoordList.size()) return _texCoordList[unit].second.get();
    else return 0;
}


#ifdef COMPILE_POSSIBLE_NEW_ARRAY_METHODS

void Geometry::setArray(AttributeType type,Array* array)
{
    if (_attributeList.size()<=type)
        _attributeList.resize(type+1);
        
   _attributeList[type]._array = array;

    dirtyDisplayList();
}

Array* Geometry::getArray(AttributeType type)
{
    if (type<_attributeList.size()) return _attributeList[type]._array.get();
    else return 0;
}

const Array* Geometry::getArray(AttributeType type) const
{
    if (type<_attributeList.size()) return _attributeList[type]._array.get();
    else return 0;
}


void Geometry::setIndices(AttributeType type,IndexArray* array)
{
    if (_attributeList.size()<=type)
        _attributeList.resize(type+1);
        
   _attributeList[type]._indices = array;

    dirtyDisplayList();
}

IndexArray* Geometry::getIndices(AttributeType type)
{
    if (type<_attributeList.size()) return _attributeList[type]._indices.get();
    else return 0;
}

const IndexArray* Geometry::getIndices(AttributeType type) const
{
    if (type<_attributeList.size()) return _attributeList[type]._indices.get();
    else return 0;
}


void Geometry::setNormalize(AttributeType type,GLboolean normalize)
{
    if (_attributeList.size()<=type)
        _attributeList.resize(type+1);
        
   _attributeList[type]._normalize = normalize;

    dirtyDisplayList();
}

GLboolean Geometry::getNormalize(AttributeType type) const
{
    if (type<_attributeList.size()) return _attributeList[type]._normalize;
    else return GL_FALSE;
}

void Geometry::setBinding(AttributeType type,AttributeBinding binding)
{
    if (_attributeList.size()<=type)
        _attributeList.resize(type+1);
        
   _attributeList[type]._binding = binding;

    dirtyDisplayList();
}

Geometry::AttributeBinding Geometry::getBinding(AttributeType type) const
{
    if (type<_attributeList.size()) return _attributeList[type]._binding;
    else return BIND_OFF;
}

#endif

void Geometry::setVertexAttribArray(unsigned int index,bool normalize,Array* array,AttributeBinding ab)
{
    if (_vertexAttribList.size()<=index)
    {
        _vertexAttribList.resize(index+1);
        _vertexAttribBindingList.resize(index+1);
    }

    _vertexAttribList[index].first = normalize;
    _vertexAttribList[index].second.first = array;

    if( index == 0 )
    {
        // Force bind per vertex
        _vertexAttribBindingList[index] = BIND_PER_VERTEX;
    }
    else
    {
        _vertexAttribBindingList[index] = ab;
    }

    computeFastPathsUsed();
    dirtyDisplayList();
}

Array *Geometry::getVertexAttribArray(unsigned int index)
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].second.first.get();
    else return 0;
}

const Array *Geometry::getVertexAttribArray(unsigned int index) const
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].second.first.get();
    else return 0;
}

bool Geometry::getVertexAttribBinding(unsigned int index, AttributeBinding& ab) const
{
    // AttributeBinding value goes in ab 
    // return true if index is valid, false otherwise

    if (index<_vertexAttribBindingList.size()) 
    {
        ab = _vertexAttribBindingList[index];
        return true;
    }
    else 
    {
        return false;
    }
}

bool Geometry::getVertexAttribNormalize(unsigned int index, GLboolean &ret) const
{
    // normalized value goes in ret 
    // return true if index is valid, false otherwise

    if (index<_vertexAttribList.size()) 
    {
        ret = _vertexAttribList[index].first;
        return true;
    }
    else 
    {
        return false;
    }
}

void Geometry::setVertexAttribIndices(unsigned int index,IndexArray* array)
{
    if (_vertexAttribList.size()<=index)
        _vertexAttribList.resize(index+1);

    _vertexAttribList[index].second.second = array;

    dirtyDisplayList();
}

IndexArray* Geometry::getVertexAttribIndices(unsigned int index)
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].second.second.get();
    else return 0;
}

const IndexArray* Geometry::getVertexAttribIndices(unsigned int index) const
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].second.second.get();
    else return 0;
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

/*
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // set up normals if required.
    //
    if (!_normalArray.valid() ||
        _normalArray->empty() ||
        (_normalIndices.valid() && _normalIndices->getNumElements()==0) )
    {
        // switch off if not supported or have a valid data.
        _normalBinding = BIND_OFF;
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // set up colours..
    //
    if (!_colorArray.valid() ||
        _colorArray->getNumElements()==0 ||
        (_colorIndices.valid() && _colorIndices->getNumElements()==0) )
    {
        // switch off if not supported or have a valid data.
        _colorBinding = BIND_OFF;
    }



    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up secondary color if required.
    //
    if (!_secondaryColorArray.valid() || 
        _secondaryColorArray->getNumElements()==0 ||
        (_secondaryColorIndices.valid() && _secondaryColorIndices->getNumElements()==0) )
    {
        // switch off if not supported or have a valid data.
        _secondaryColorBinding = BIND_OFF;
    }

    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up fog coord if required.
    //
    if (!_fogCoordArray.valid() || 
        _fogCoordArray->getNumElements()==0 ||
        (_fogCoordIndices.valid() && _fogCoordIndices->getNumElements()==0) )
    {
        // switch off if not supported or have a valid data.
        _fogCoordBinding = BIND_OFF;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up vertex attrib if required.
    //
    for( unsigned int va = 0; va < _vertexAttribList.size(); ++va )
    {
        const Array * array = _vertexAttribList[va].second.first.get();
        const IndexArray * idxArray = _vertexAttribList[va].second.second.get();

        if (!array || 
            array->getNumElements()==0 ||
            (idxArray && idxArray->getNumElements()==0) )
        {
            // switch off if not supported or have a valid data.
            _vertexAttribBindingList[va] = BIND_OFF;
        }
    }
*/
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // check to see if fast path can be used.
    //
    _fastPath = true;
    if (_vertexIndices.valid()) _fastPath = false;
    else if (_normalBinding==BIND_PER_PRIMITIVE || (_normalBinding==BIND_PER_VERTEX && _normalIndices.valid())) _fastPath = false;
    else if (_colorBinding==BIND_PER_PRIMITIVE || (_colorBinding==BIND_PER_VERTEX && _colorIndices.valid())) _fastPath = false;
    else if (_secondaryColorBinding==BIND_PER_PRIMITIVE || (_secondaryColorBinding==BIND_PER_VERTEX && _secondaryColorIndices.valid())) _fastPath = false;
    else if (_fogCoordBinding==BIND_PER_PRIMITIVE || (_fogCoordBinding==BIND_PER_VERTEX && _fogCoordIndices.valid())) _fastPath = false;
    else 
    {
        for( unsigned int va = 0; va < _vertexAttribBindingList.size(); ++va )
        {
            if (_vertexAttribBindingList[va]==BIND_PER_PRIMITIVE)
            {
                _fastPath = false;
                break;
            }
            else
            {
                const Array * array = _vertexAttribList[va].second.first.get();
                const IndexArray * idxArray = _vertexAttribList[va].second.second.get();

                if( _vertexAttribBindingList[va]==BIND_PER_VERTEX && 
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
    for(unsigned int unit=0;unit!=_texCoordList.size();++unit)
    {
        const ArrayPair& texcoordPair = _texCoordList[unit];
        if (texcoordPair.first.valid() && texcoordPair.first->getNumElements()>0)
        {
            if (texcoordPair.second.valid())
            {
                if (texcoordPair.second->getNumElements()>0)
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

void Geometry::drawImplementation(State& state) const
{
    if (_internalOptimizedGeometry.valid())
    {
        _internalOptimizedGeometry->drawImplementation(state);
        return;
    }

    const Extensions* extensions = getExtensions(state.getContextID(),true);

    if( !( ( _vertexArray.valid() && _vertexArray->getNumElements() != 0 ) ||
           ( _vertexAttribList.size() > 0 && 
             _vertexAttribList[0].second.first.valid() && 
             _vertexAttribList[0].second.first->getNumElements() != 0 ) ) )
    {
        return;
    }

    if( ( _vertexIndices.valid() && _vertexIndices->getNumElements() == 0 ) ||
          ( _vertexAttribList.size() > 0 && 
          _vertexAttribList[0].second.second.valid() && 
          _vertexAttribList[0].second.second->getNumElements() == 0 ) )
    {
        return;
    }

    DrawNormal         drawNormal(_normalArray.get(),_normalIndices.get());
    DrawColor          drawColor(_colorArray.get(),_colorIndices.get());
    DrawSecondaryColor drawSecondaryColor(_secondaryColorArray.get(),_secondaryColorIndices.get(),extensions);

    DrawFogCoord       drawFogCoord(_fogCoordArray.get(),_fogCoordIndices.get(),extensions);


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up secondary color if required.
    //
    AttributeBinding secondaryColorBinding = _secondaryColorBinding;
    if (secondaryColorBinding!=BIND_OFF && !extensions->isSecondaryColorSupported())
    {
        // switch off if not supported or have a valid data.
        secondaryColorBinding = BIND_OFF;
    }

    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up fog coord if required.
    //
    AttributeBinding fogCoordBinding = _fogCoordBinding;
    if (fogCoordBinding!=BIND_OFF && !extensions->isFogCoordSupported())
    {
        // switch off if not supported or have a valid data.
        fogCoordBinding = BIND_OFF;
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up vertex attrib if required.
    //
    if(!extensions->isVertexProgramSupported())
    {
        for( unsigned int va = 0; va < _vertexAttribBindingList.size(); ++va )
        {
            if (_vertexAttribBindingList[va]!=BIND_OFF)
            {
                _vertexAttribBindingList[va] = BIND_OFF;
            }
        }
    }

    unsigned int normalIndex = 0;
    unsigned int colorIndex = 0;
    unsigned int secondaryColorIndex = 0;
    unsigned int fogCoordIndex = 0;
    unsigned int vertexAttribIndex = 0;

#if USE_DEFAULT_NORMAL
    // if no values are defined for normal and color provide some defaults...
    if (_normalBinding==BIND_OFF) glNormal3f(0.0f,0.0f,1.0f);
#endif

#if USE_DEFAULT_COLOUR
    if (_colorBinding==BIND_OFF) glColor4f(1.0f,1.0f,1.0f,1.0f);
#endif

    typedef std::vector< ref_ptr<DrawVertexAttrib> > DrawVertexAttribList;
    typedef std::map< Geometry::AttributeBinding, DrawVertexAttribList> DrawVertexAttribMap;
    DrawVertexAttribMap drawVertexAttribMap;
    
    bool handleVertexAttributes = (!_vertexAttribList.empty() && extensions->isVertexProgramSupported());

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
                unsigned int totalSize = 0;
                
                _vertexOffset = 0;
                if (_vertexArray.valid()) totalSize += _vertexArray->getTotalDataSize();
                
                _normalOffset = totalSize; 
                if (_normalArray.valid()) totalSize += _normalArray->getTotalDataSize();
                                
                _colorOffset = totalSize;
                if (_colorArray.valid()) totalSize += _colorArray->getTotalDataSize();

                _secondaryColorOffset = totalSize;
                if (_secondaryColorArray.valid()) totalSize += _secondaryColorArray->getTotalDataSize();

                _fogCoordOffset = totalSize;
                if (_fogCoordArray.valid()) totalSize += _fogCoordArray->getTotalDataSize();


                unsigned int unit;
                for(unit=0;unit<_texCoordList.size();++unit)
                {
                    _texCoordList[unit].offset = totalSize;
                    const Array* array = _texCoordList[unit].first.get();
                    if (array)
                        totalSize += array->getTotalDataSize();
                        
                }

                if( handleVertexAttributes )
                {
                    unsigned int index;
                    for( index = 0; index < _vertexAttribList.size(); ++index )
                    {
                        _texCoordList[unit].offset = totalSize;
                        const Array* array = _vertexAttribList[index].second.first.get();
                        const AttributeBinding ab = _vertexAttribBindingList[index];
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
                if( _vertexArray.valid() )
                    extensions->glBufferSubData(GL_ARRAY_BUFFER_ARB, _vertexOffset, _vertexArray->getTotalDataSize(),_vertexArray->getDataPointer());

                if (_normalBinding==BIND_PER_VERTEX)
                    extensions->glBufferSubData(GL_ARRAY_BUFFER_ARB, _normalOffset, _normalArray->getTotalDataSize(),_normalArray->getDataPointer());

                if (_colorBinding==BIND_PER_VERTEX)
                    extensions->glBufferSubData(GL_ARRAY_BUFFER_ARB, _colorOffset, _colorArray->getTotalDataSize(),_colorArray->getDataPointer());

                if (secondaryColorBinding==BIND_PER_VERTEX)
                    extensions->glBufferSubData(GL_ARRAY_BUFFER_ARB, _secondaryColorOffset, _secondaryColorArray->getTotalDataSize(),_secondaryColorArray->getDataPointer());

                if (fogCoordBinding==BIND_PER_VERTEX)
                    extensions->glBufferSubData(GL_ARRAY_BUFFER_ARB, _fogCoordOffset, _fogCoordArray->getTotalDataSize(),_fogCoordArray->getDataPointer());

                for(unit=0;unit<_texCoordList.size();++unit)
                {
                    const Array* array = _texCoordList[unit].first.get();
                    if (array)
                        extensions->glBufferSubData(GL_ARRAY_BUFFER_ARB, _texCoordList[unit].offset, array->getTotalDataSize(), array->getDataPointer());
                }

                if( handleVertexAttributes )
                {
                    unsigned int index;
                    for( index = 0; index < _vertexAttribList.size(); ++index )
                    {
                        const Array* array = _vertexAttribList[index].second.first.get();
                        const AttributeBinding ab = _vertexAttribBindingList[index];

                        if( ab == BIND_PER_VERTEX && array )
                        {
                            extensions->glBufferSubData(GL_ARRAY_BUFFER_ARB, _vertexAttribList[index].second.offset, array->getTotalDataSize(), array->getDataPointer());
                        }
                    }
                }
                                     
                
            }
            
            //std::cout << "binding VertexBuffer "<<buffer<<std::endl;

            extensions->glBindBuffer(GL_ARRAY_BUFFER_ARB,buffer);
                      
            if( _vertexArray.valid() )
                state.setVertexPointer(_vertexArray->getDataSize(),_vertexArray->getDataType(),0,(const GLvoid*)_vertexOffset);
            else
                state.disableVertexPointer();

            if (_normalBinding==BIND_PER_VERTEX)
                state.setNormalPointer(GL_FLOAT,0,(const GLvoid*)_normalOffset);
            else
                state.disableNormalPointer();

            if (_colorBinding==BIND_PER_VERTEX)
                state.setColorPointer(_colorArray->getDataSize(),_colorArray->getDataType(),0,(const GLvoid*)_colorOffset);
            else
                state.disableColorPointer();

            if (secondaryColorBinding==BIND_PER_VERTEX)
                state.setSecondaryColorPointer(_secondaryColorArray->getDataSize(),_secondaryColorArray->getDataType(),0,(const GLvoid*)_secondaryColorOffset);
            else
                state.disableSecondaryColorPointer();

            if (fogCoordBinding==BIND_PER_VERTEX)
                state.setFogCoordPointer(GL_FLOAT,0,(const GLvoid*)_fogCoordOffset);
            else
                state.disableFogCoordPointer();

            unsigned int unit;
            for(unit=0;unit<_texCoordList.size();++unit)
            {
                const Array* array = _texCoordList[unit].first.get();
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
                    const Array* array = _vertexAttribList[index].second.first.get();
                    const AttributeBinding ab = _vertexAttribBindingList[index];

                    if( ab == BIND_PER_VERTEX && array )
                    {
                        state.setVertexAttribPointer( index, array->getDataSize(), array->getDataType(), 
                            _vertexAttribList[index].first, 0, (const GLvoid*)_vertexAttribList[index].second.offset );
                    }
                    else
                    {
                        if( array )
                        {
                            const IndexArray* indexArray = _vertexAttribList[index].second.second.get();

                            if( indexArray && indexArray->getNumElements() > 0 )
                            {
                                drawVertexAttribMap[ab].push_back( 
                                    new DrawVertexAttrib(extensions,index,_vertexAttribList[index].first,array,indexArray) );
                            }
                            else
                            {
                                drawVertexAttribMap[ab].push_back( 
                                    new DrawVertexAttrib(extensions,index,_vertexAttribList[index].first,array,0) );
                            }
                        }

                        state.disableVertexAttribPointer( index );
                    }
                }
                state.disableVertexAttribPointersAboveAndIncluding( index );
            }


        }
        else
        {
            //std::cout << "none VertexBuffer path"<<std::endl;

            //
            // None Vertex Buffer Object path for defining vertex arrays.
            //            
            if( _vertexArray.valid() )
                state.setVertexPointer(_vertexArray->getDataSize(),_vertexArray->getDataType(),0,_vertexArray->getDataPointer());
            else
                state.disableVertexPointer();

            if (_normalBinding==BIND_PER_VERTEX)
                state.setNormalPointer(GL_FLOAT,0,_normalArray->getDataPointer());
            else
                state.disableNormalPointer();

            if (_colorBinding==BIND_PER_VERTEX)
                state.setColorPointer(_colorArray->getDataSize(),_colorArray->getDataType(),0,_colorArray->getDataPointer());
            else
                state.disableColorPointer();

            if (secondaryColorBinding==BIND_PER_VERTEX)
                state.setSecondaryColorPointer(_secondaryColorArray->getDataSize(),_secondaryColorArray->getDataType(),0,_secondaryColorArray->getDataPointer());
            else
                state.disableSecondaryColorPointer();

            if (fogCoordBinding==BIND_PER_VERTEX)
                state.setFogCoordPointer(GL_FLOAT,0,_fogCoordArray->getDataPointer());
            else
                state.disableFogCoordPointer();

            unsigned int unit;
            for(unit=0;unit<_texCoordList.size();++unit)
            {
                const Array* array = _texCoordList[unit].first.get();
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
                    const Array* array = _vertexAttribList[index].second.first.get();
                    const AttributeBinding ab = _vertexAttribBindingList[index];

                    if( ab == BIND_PER_VERTEX && array )
                    {
                        state.setVertexAttribPointer( index, array->getDataSize(), array->getDataType(), 
                            _vertexAttribList[index].first, 0, array->getDataPointer() );
                    }
                    else
                    {
                        if( array )
                        {
                            const IndexArray* indexArray = _vertexAttribList[index].second.second.get();

                            if( indexArray && indexArray->getNumElements() > 0 )
                            {
                                drawVertexAttribMap[ab].push_back( 
                                    new DrawVertexAttrib(extensions,index,_vertexAttribList[index].first,array,indexArray) );
                            }
                            else
                            {
                                drawVertexAttribMap[ab].push_back( 
                                    new DrawVertexAttrib(extensions,index,_vertexAttribList[index].first,array,0) );
                            }
                        }

                        state.disableVertexAttribPointer( index );
                    }
                }
                state.disableVertexAttribPointersAboveAndIncluding( index );
            }
        }
        
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        // pass the overall binding values onto OpenGL.
        //
        if (_normalBinding==BIND_OVERALL)           drawNormal(normalIndex++);
        if (_colorBinding==BIND_OVERALL)            drawColor(colorIndex++);
        if (secondaryColorBinding==BIND_OVERALL)    drawSecondaryColor(secondaryColorIndex++);
        if (fogCoordBinding==BIND_OVERALL)          drawFogCoord(fogCoordIndex++);
        if (handleVertexAttributes)
        {
            DrawVertexAttribList &list = drawVertexAttribMap[BIND_OVERALL];

            for( unsigned int i = 0; i < list.size(); ++i )
            {
                ( *( list[i] ) )(vertexAttribIndex++);
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

            if (_normalBinding==BIND_PER_PRIMITIVE_SET)           drawNormal(normalIndex++);
            if (_colorBinding==BIND_PER_PRIMITIVE_SET)            drawColor(colorIndex++);
            if (secondaryColorBinding==BIND_PER_PRIMITIVE_SET)    drawSecondaryColor(secondaryColorIndex++);
            if (fogCoordBinding==BIND_PER_PRIMITIVE_SET)          drawFogCoord(fogCoordIndex++);
            if (handleVertexAttributes)
            {
                DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_PRIMITIVE_SET];

                for( unsigned int i = 0; i < list.size(); ++i )
                {
                    ( *( list[i] ) )(vertexAttribIndex++);
                }
            }

            (*itr)->draw();

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

        if (extensions->isMultiTexSupported())
        {
            // multitexture supported..
            for(unsigned int unit=0;unit!=_texCoordList.size();++unit)
            {
                const ArrayPair& texcoordPair = _texCoordList[unit];
                if (texcoordPair.first.valid() && texcoordPair.first->getNumElements()>0)
                {
                    if (texcoordPair.second.valid())
                    {
                        if (texcoordPair.second->getNumElements()>0)
                        {
                            drawTexCoordList.push_back(new DrawMultiTexCoord(GL_TEXTURE0+unit,texcoordPair.first.get(),texcoordPair.second.get(),
                                                                             extensions));

                        }
                    }
                    else
                    {
                        drawTexCoordList.push_back(new DrawMultiTexCoord(GL_TEXTURE0+unit,texcoordPair.first.get(),0,
                                                                          extensions));
                    }
                }
            }
        }
        else
        {
            if (!_texCoordList.empty())
            {
                const ArrayPair& texcoordPair = _texCoordList[0];
                if (texcoordPair.first.valid() && texcoordPair.first->getNumElements()>0)
                {
                    if (texcoordPair.second.valid())
                    {
                        if (texcoordPair.second->getNumElements()>0)
                        {
                            drawTextCoord = new DrawTexCoord(texcoordPair.first.get(),texcoordPair.second.get());
                        }
                    }
                    else
                    {
                        drawTextCoord = new DrawTexCoord(texcoordPair.first.get(),0);
                    }
                }

            }
        }

        if(handleVertexAttributes)
        {
            unsigned int index;
            for( index = 1; index < _vertexAttribList.size(); ++index )
            {
                const Array* array = _vertexAttribList[index].second.first.get();

                if( array && array->getNumElements() > 0 )
                {
                    const IndexArray* indexArray = _vertexAttribList[index].second.second.get();

                    if( indexArray && indexArray->getNumElements() > 0 )
                    {
                        drawVertexAttribMap[_vertexAttribBindingList[index]].push_back( 
                            new DrawVertexAttrib(extensions,index,_vertexAttribList[index].first,array,indexArray) );
                    }
                    else
                    {
                        drawVertexAttribMap[_vertexAttribBindingList[index]].push_back( 
                            new DrawVertexAttrib(extensions,index,_vertexAttribList[index].first,array,0) );
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
        if (_normalBinding==BIND_OVERALL)           drawNormal(normalIndex++);
        if (_colorBinding==BIND_OVERALL)            drawColor(colorIndex++);
        if (secondaryColorBinding==BIND_OVERALL)    drawSecondaryColor(secondaryColorIndex++);
        if (fogCoordBinding==BIND_OVERALL)          drawFogCoord(fogCoordIndex++);
        if (handleVertexAttributes)
        {
            DrawVertexAttribList &list = drawVertexAttribMap[BIND_OVERALL];

            for( unsigned int i = 0; i < list.size(); ++i )
            {
                ( *( list[i] ) )(vertexAttribIndex++);
            }
        }

        // set up vertex functor.
        DrawVertex drawVertex(_vertexArray.get(),_vertexIndices.get());

        bool useVertexAttrib =  _vertexAttribList.size() > 0 &&
                                _vertexAttribList[0].second.first.valid() && 
                                 _vertexAttribList[0].second.first->getNumElements();

        ref_ptr<DrawVertexAttrib> drawVertexAttribZero;
        if( useVertexAttrib )
        {
            drawVertexAttribZero = new DrawVertexAttrib(extensions,0,
                _vertexAttribList[0].first,_vertexAttribList[0].second.first.get(),
                _vertexAttribList[0].second.second.get()); 
        }

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        // draw the primitives themselves.
        //
        for(PrimitiveSetList::const_iterator itr=_primitives.begin();
            itr!=_primitives.end();
            ++itr)
        {
            if (_normalBinding==BIND_PER_PRIMITIVE_SET)           drawNormal(normalIndex++);
            if (_colorBinding==BIND_PER_PRIMITIVE_SET)            drawColor(colorIndex++);
            if (secondaryColorBinding==BIND_PER_PRIMITIVE_SET)    drawSecondaryColor(secondaryColorIndex++);
            if (fogCoordBinding==BIND_PER_PRIMITIVE_SET)          drawFogCoord(fogCoordIndex++);
            if (handleVertexAttributes)
            {
                DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_PRIMITIVE_SET];

                for( unsigned int i = 0; i < list.size(); ++i )
                {
                    ( *( list[i] ) )(vertexAttribIndex++);
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
                            if (_normalBinding==BIND_PER_PRIMITIVE)           drawNormal(normalIndex++);
                            if (_colorBinding==BIND_PER_PRIMITIVE)            drawColor(colorIndex++);
                            if (secondaryColorBinding==BIND_PER_PRIMITIVE)    drawSecondaryColor(secondaryColorIndex++);
                            if (fogCoordBinding==BIND_PER_PRIMITIVE)          drawFogCoord(fogCoordIndex++);
                            if (handleVertexAttributes)
                            {
                                DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_PRIMITIVE];

                                for( unsigned int i = 0; i < list.size(); ++i )
                                {
                                    ( *( list[i] ) )(vertexAttribIndex++);
                                }
                            }                        
                        }

                        if (_normalBinding==BIND_PER_VERTEX)           drawNormal(vindex);
                        if (_colorBinding==BIND_PER_VERTEX)            drawColor(vindex);
                        if (secondaryColorBinding==BIND_PER_VERTEX)    drawSecondaryColor(vindex);
                        if (fogCoordBinding==BIND_PER_VERTEX)          drawFogCoord(vindex);
                        if (handleVertexAttributes)
                        {
                            DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_VERTEX];

                            for( unsigned int i = 0; i < list.size(); ++i )
                            {
                                ( *( list[i] ) )(vertexAttribIndex++);
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
                                if (_normalBinding==BIND_PER_PRIMITIVE)           drawNormal(normalIndex++);
                                if (_colorBinding==BIND_PER_PRIMITIVE)            drawColor(colorIndex++);
                                if (secondaryColorBinding==BIND_PER_PRIMITIVE)    drawSecondaryColor(secondaryColorIndex++);
                                if (fogCoordBinding==BIND_PER_PRIMITIVE)          drawFogCoord(fogCoordIndex++);
                                if (handleVertexAttributes)
                                {
                                    DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_PRIMITIVE];

                                    for( unsigned int i = 0; i < list.size(); ++i )
                                    {
                                        ( *( list[i] ) )(vertexAttribIndex++);
                                    }
                                }  
                            }
                            
                            if (_normalBinding==BIND_PER_VERTEX)           drawNormal(vindex);
                            if (_colorBinding==BIND_PER_VERTEX)            drawColor(vindex);
                            if (secondaryColorBinding==BIND_PER_VERTEX)    drawSecondaryColor(vindex);
                            if (fogCoordBinding==BIND_PER_VERTEX)          drawFogCoord(vindex);
                            if (handleVertexAttributes)
                            {
                                DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_VERTEX];

                                for( unsigned int i = 0; i < list.size(); ++i )
                                {
                                    ( *( list[i] ) )(vertexAttribIndex++);
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
                            if (_normalBinding==BIND_PER_PRIMITIVE)           drawNormal(normalIndex++);
                            if (_colorBinding==BIND_PER_PRIMITIVE)            drawColor(colorIndex++);
                            if (secondaryColorBinding==BIND_PER_PRIMITIVE)    drawSecondaryColor(secondaryColorIndex++);
                            if (fogCoordBinding==BIND_PER_PRIMITIVE)          drawFogCoord(fogCoordIndex++);
                            if (handleVertexAttributes)
                            {
                                DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_PRIMITIVE];

                                for( unsigned int i = 0; i < list.size(); ++i )
                                {
                                    ( *( list[i] ) )(vertexAttribIndex++);
                                }
                            }  
                        }
                        
                        unsigned int vindex=*primItr;

                        if (_normalBinding==BIND_PER_VERTEX)           drawNormal(vindex);
                        if (_colorBinding==BIND_PER_VERTEX)            drawColor(vindex);
                        if (secondaryColorBinding==BIND_PER_VERTEX)    drawSecondaryColor(vindex);
                        if (fogCoordBinding==BIND_PER_VERTEX)          drawFogCoord(vindex);
                        if ( extensions->isVertexProgramSupported() )
                        {
                            DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_VERTEX];

                            for( unsigned int i = 0; i < list.size(); ++i )
                            {
                                ( *( list[i] ) )(vertexAttribIndex++);
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
                            if (_normalBinding==BIND_PER_PRIMITIVE)           drawNormal(normalIndex++);
                            if (_colorBinding==BIND_PER_PRIMITIVE)            drawColor(colorIndex++);
                            if (secondaryColorBinding==BIND_PER_PRIMITIVE)    drawSecondaryColor(secondaryColorIndex++);
                            if (fogCoordBinding==BIND_PER_PRIMITIVE)          drawFogCoord(fogCoordIndex++);
                            if (handleVertexAttributes)
                            {
                                DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_PRIMITIVE];

                                for( unsigned int i = 0; i < list.size(); ++i )
                                {
                                    ( *( list[i] ) )(vertexAttribIndex++);
                                }
                            }  
                        }
                        
                        unsigned int vindex=*primItr;

                        if (_normalBinding==BIND_PER_VERTEX)           drawNormal(vindex);
                        if (_colorBinding==BIND_PER_VERTEX)            drawColor(vindex);
                        if (secondaryColorBinding==BIND_PER_VERTEX)    drawSecondaryColor(vindex);
                        if (fogCoordBinding==BIND_PER_VERTEX)          drawFogCoord(vindex);
                        if (handleVertexAttributes)
                        {
                            DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_VERTEX];

                            for( unsigned int i = 0; i < list.size(); ++i )
                            {
                                ( *( list[i] ) )(vertexAttribIndex++);
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
                            if (_normalBinding==BIND_PER_PRIMITIVE)           drawNormal(normalIndex++);
                            if (_colorBinding==BIND_PER_PRIMITIVE)            drawColor(colorIndex++);
                            if (secondaryColorBinding==BIND_PER_PRIMITIVE)    drawSecondaryColor(secondaryColorIndex++);
                            if (fogCoordBinding==BIND_PER_PRIMITIVE)          drawFogCoord(fogCoordIndex++);
                            if ( extensions->isVertexProgramSupported() )
                            {
                                DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_PRIMITIVE];

                                for( unsigned int i = 0; i < list.size(); ++i )
                                {
                                    ( *( list[i] ) )(vertexAttribIndex++);
                                }
                            }  
                        }
                        
                        unsigned int vindex=*primItr;

                        if (_normalBinding==BIND_PER_VERTEX)           drawNormal(vindex);
                        if (_colorBinding==BIND_PER_VERTEX)            drawColor(vindex);
                        if (secondaryColorBinding==BIND_PER_VERTEX)    drawSecondaryColor(vindex);
                        if (fogCoordBinding==BIND_PER_VERTEX)          drawFogCoord(vindex);
                        if ( extensions->isVertexProgramSupported() )
                        {
                            DrawVertexAttribList &list = drawVertexAttribMap[BIND_PER_VERTEX];

                            for( unsigned int i = 0; i < list.size(); ++i )
                            {
                                ( *( list[i] ) )(vertexAttribIndex++);
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

class AttrbuteFunctorArrayVisitor : public ArrayVisitor
{
    public:
    
        AttrbuteFunctorArrayVisitor(Drawable::AttributeFunctor& af):
            _af(af) {}
    
        virtual ~AttrbuteFunctorArrayVisitor() {}

        virtual void apply(ByteArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(ShortArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(IntArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(UByteArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(UShortArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(UIntArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(UByte4Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
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
    AttrbuteFunctorArrayVisitor afav(af);
    
    afav.applyArray(VERTICES,_vertexArray.get());
    afav.applyArray(NORMALS,_normalArray.get());
    afav.applyArray(COLORS,_colorArray.get());
    
    for(unsigned unit=0;unit<_texCoordList.size();++unit)
    {
        afav.applyArray((AttributeType)(TEXTURE_COORDS_0+unit),_texCoordList[unit].first.get());
    }
}

class ConstAttrbuteFunctorArrayVisitor : public ConstArrayVisitor
{
    public:
    
        ConstAttrbuteFunctorArrayVisitor(Drawable::ConstAttributeFunctor& af):
            _af(af) {}
    
        virtual ~ConstAttrbuteFunctorArrayVisitor() {}

        virtual void apply(const ByteArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const ShortArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const IntArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const UByteArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const UShortArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const UIntArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const UByte4Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
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
    ConstAttrbuteFunctorArrayVisitor afav(af);
    
    afav.applyArray(VERTICES,_vertexArray.get());
    afav.applyArray(NORMALS,_normalArray.get());
    afav.applyArray(COLORS,_colorArray.get());
    
    for(unsigned unit=0;unit<_texCoordList.size();++unit)
    {
        afav.applyArray((AttributeType)(TEXTURE_COORDS_0+unit),_texCoordList[unit].first.get());
    }
}

void Geometry::accept(PrimitiveFunctor& functor) const
{
    if (!_vertexArray.valid() || _vertexArray->getNumElements()==0) return;

    if (!_vertexIndices.valid())
    {
        switch(_vertexArray->getType())
        {
        case(Array::Vec2ArrayType): 
            functor.setVertexArray(_vertexArray->getNumElements(),static_cast<const Vec2*>(_vertexArray->getDataPointer()));
            break;
        case(Array::Vec3ArrayType): 
            functor.setVertexArray(_vertexArray->getNumElements(),static_cast<const Vec3*>(_vertexArray->getDataPointer()));
            break;
        case(Array::Vec4ArrayType): 
            functor.setVertexArray(_vertexArray->getNumElements(),static_cast<const Vec4*>(_vertexArray->getDataPointer()));
            break;
        default:
            notify(WARN)<<"Warning: Geometry::accept(PrimtiveFunctor&) cannot handle Vertex Array type"<<_vertexArray->getType()<<std::endl;
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
        Array::Type type = _vertexArray->getType();
        switch(type)
        {
        case(Array::Vec2ArrayType): 
            vec2Array = static_cast<const Vec2Array*>(_vertexArray.get());
            break;
        case(Array::Vec3ArrayType): 
            vec3Array = static_cast<const Vec3Array*>(_vertexArray.get());
            break;
        case(Array::Vec4ArrayType): 
            vec4Array = static_cast<const Vec4Array*>(_vertexArray.get());
            break;
        default:
            notify(WARN)<<"Warning: Geometry::accept(PrimtiveFunctor&) cannot handle Vertex Array type"<<_vertexArray->getType()<<std::endl;
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
                            functor.vertex((*vec2Array)[_vertexIndices->index(vindex)]);
                            break;
                        case(Array::Vec3ArrayType): 
                            functor.vertex((*vec3Array)[_vertexIndices->index(vindex)]);
                            break;
                        case(Array::Vec4ArrayType): 
                            functor.vertex((*vec4Array)[_vertexIndices->index(vindex)]);
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
                                functor.vertex((*vec2Array)[_vertexIndices->index(vindex)]);
                                break;
                            case(Array::Vec3ArrayType): 
                                functor.vertex((*vec3Array)[_vertexIndices->index(vindex)]);
                                break;
                            case(Array::Vec4ArrayType): 
                                functor.vertex((*vec4Array)[_vertexIndices->index(vindex)]);
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
                            functor.vertex((*vec2Array)[_vertexIndices->index(vindex)]);
                            break;
                        case(Array::Vec3ArrayType): 
                            functor.vertex((*vec3Array)[_vertexIndices->index(vindex)]);
                            break;
                        case(Array::Vec4ArrayType): 
                            functor.vertex((*vec4Array)[_vertexIndices->index(vindex)]);
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
                            functor.vertex((*vec2Array)[_vertexIndices->index(vindex)]);
                            break;
                        case(Array::Vec3ArrayType): 
                            functor.vertex((*vec3Array)[_vertexIndices->index(vindex)]);
                            break;
                        case(Array::Vec4ArrayType): 
                            functor.vertex((*vec4Array)[_vertexIndices->index(vindex)]);
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
                            functor.vertex((*vec2Array)[_vertexIndices->index(vindex)]);
                            break;
                        case(Array::Vec3ArrayType): 
                            functor.vertex((*vec3Array)[_vertexIndices->index(vindex)]);
                            break;
                        case(Array::Vec4ArrayType): 
                            functor.vertex((*vec4Array)[_vertexIndices->index(vindex)]);
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

bool Geometry::verifyBindings() const
{
    switch(_normalBinding)
    {
        case(BIND_OFF):
            if (_normalArray.valid() && _normalArray->getNumElements()>0) return false;
            break;
        case(BIND_OVERALL):
            if (!_normalArray.valid()) return false;
            if (_normalArray->getNumElements()!=1) return false;
            break;
        case(BIND_PER_PRIMITIVE_SET):
            if (!_normalArray.valid()) return false;
            if (_normalArray->getNumElements()!=_primitives.size()) return false;
            break;
        case(BIND_PER_PRIMITIVE):
            if (!_normalArray.valid()) return false;
            break;
        case(BIND_PER_VERTEX):
            if (_vertexArray.valid())
            {
                if (!_normalArray.valid()) return false;
                if (_normalArray->getNumElements()!=_vertexArray->getNumElements()) return false;        
            }
            else if (_normalArray.valid() && _normalArray->getNumElements()==0) return false;
            break;
    } 
    
    switch(_colorBinding)
    {
        case(BIND_OFF):
            if (_colorArray.valid() && _colorArray->getNumElements()>0) return false;
            break;
        case(BIND_OVERALL):
            if (!_colorArray.valid()) return false;
            if (_colorArray->getNumElements()!=1) return false;
            break;
        case(BIND_PER_PRIMITIVE_SET):
            if (!_colorArray.valid()) return false;
            if (_colorArray->getNumElements()!=_primitives.size()) return false;
            break;
        case(BIND_PER_PRIMITIVE):
            if (!_colorArray.valid()) return false;
            break;
        case(BIND_PER_VERTEX):
            if (_vertexArray.valid())
            {
                if (!_colorArray.valid()) return false;
                if (_colorArray->getNumElements()!=_vertexArray->getNumElements()) return false;
            }
            else if (_colorArray.valid() && _colorArray->getNumElements()==0) return false;
            break;
    } 

    for(TexCoordArrayList::const_iterator itr=_texCoordList.begin();
        itr!=_texCoordList.end();
        ++itr)
    {
        const Array* array = itr->first.get();
        if (_vertexArray.valid())
        {
            if (array && array->getNumElements()!=_vertexArray->getNumElements()) return false;
        }
        else if (array && array->getNumElements()==0) return false;
    }

    return true;
}

void Geometry::computeCorrectBindingsAndArraySizes()
{
    if (verifyBindings()) return;

    if (!_vertexArray.valid() || _vertexArray->getNumElements()==0)
    {
        // no vertex array so switch everything off.
        
        _vertexArray = 0;
        
        _colorArray = 0;
        _colorBinding = BIND_OFF;

        _normalArray = 0;
        _normalBinding = BIND_OFF;
        
        _texCoordList.clear();
        
        notify(INFO)<<"Info: remove redundent attribute arrays from empty osg::Geometry"<<std::endl;
        
        return;
    }
    
    
    switch(_normalBinding)
    {
        case(BIND_OFF):
            if (_normalArray.valid()) _normalArray = 0;
            break;
        case(BIND_OVERALL):
            if (!_normalArray.valid())
            {
                _normalBinding = BIND_OFF;
            }
            else if (_normalArray->getNumElements()==0) 
            {
                _normalArray = 0;
                _normalBinding = BIND_OFF;
            }
            else if (_normalArray->getNumElements()>1) 
            {
                // trim the array down to 1 element long.
                _normalArray->erase(_normalArray->begin()+1,_normalArray->end());
            }
            break;
        case(BIND_PER_PRIMITIVE_SET):
            if (!_normalArray.valid())
            {
                _normalBinding = BIND_OFF;
            }
            else if (_normalArray->getNumElements()<_primitives.size()) 
            {
                _normalArray = 0;
                _normalBinding = BIND_OFF;
            }
            else if (_normalArray->getNumElements()>_primitives.size()) 
            {
                // trim the array down to size of the number of primitives.
                _normalArray->erase(_normalArray->begin()+_primitives.size(),_normalArray->end());
            }
            break;
        case(BIND_PER_PRIMITIVE):
            break;
        case(BIND_PER_VERTEX):
            if (!_normalArray.valid())
            {
                _normalBinding = BIND_OFF;
            }
            else if (_normalArray->getNumElements()<_vertexArray->getNumElements()) 
            {
                _normalArray = 0;
                _normalBinding = BIND_OFF;
            }
            else if (_normalArray->getNumElements()>_vertexArray->getNumElements()) 
            {
                // trim the array down to size of the number of primitives.
                _normalArray->erase(_normalArray->begin()+_vertexArray->getNumElements(),_normalArray->end());
            }
            break;
    } 
    
    // TODO colours and tex coords.
    
}

class ExpandIndexedArray : public osg::ConstArrayVisitor
{
    public:
        ExpandIndexedArray(const osg::IndexArray& indices,Array* targetArray):
            _indices(indices),
            _targetArray(targetArray) {}
        
        template <class T,class I>
        T* create(const T& array,const I& indices)
        {
            T* newArray = 0;

            // if source array type and target array type are equal
            if (_targetArray && _targetArray->getType()==array.getType())
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
        virtual void apply(const osg::UByte4Array& array) { _targetArray = create(array); }
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
    // copy over primitive sets.
    target.getPrimitiveSetList() = getPrimitiveSetList();

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
        target.setVertexArray(getVertexArray());
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
        target.setNormalArray(getNormalArray());
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
        target.setColorArray(getColorArray());
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
        target.setSecondaryColorArray(getSecondaryColorArray());
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
        target.setFogCoordArray(getFogCoordArray());
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
            target.setTexCoordArray(ti,getTexCoordArray(ti));
        }
    }
    
    for(unsigned int vi=0;vi<getNumVertexAttribArrays();++vi)
    {
        AttributeBinding vab;
        getVertexAttribBinding(vi,vab);
        if (getVertexAttribIndices(vi))
        {
            GLboolean normalize;
            ExpandIndexedArray eia(*(getVertexAttribIndices(vi)),target.getVertexAttribArray(vi));
            getVertexAttribArray(vi)->accept(eia);
            getVertexAttribNormalize(vi,normalize);
            target.setVertexAttribArray(vi,normalize,eia._targetArray,vab);
            target.setVertexAttribIndices(vi,0);
        }
        else if (getVertexAttribArray(vi))
        {
            GLboolean normalize;
            getVertexAttribNormalize(vi,normalize);
            target.setVertexAttribArray(vi,normalize,getVertexAttribArray(vi),vab);
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


Geometry* osg::createTexturedQuadGeometry(const osg::Vec3& corner,const osg::Vec3& widthVec,const osg::Vec3& heightVec)
{
    Geometry* geom = new Geometry;

    Vec3Array* coords = new Vec3Array(4);
    (*coords)[0] = corner+heightVec;
    (*coords)[1] = corner;
    (*coords)[2] = corner+widthVec;
    (*coords)[3] = corner+widthVec+heightVec;
    geom->setVertexArray(coords);

    Vec2Array* tcoords = new Vec2Array(4);
    (*tcoords)[0].set(0.0f,1.0f);
    (*tcoords)[1].set(0.0f,0.0f);
    (*tcoords)[2].set(1.0f,0.0f);
    (*tcoords)[3].set(1.0f,1.0f);
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
