#include <osg/GLExtensions>
#include <osg/IndexedGeometry>
#include <osg/Notify>

using namespace osg;

class DrawVertex
{
    public:
    
        DrawVertex(const Vec3Array* vertices,const IndexArray* indices):
            _vertices(vertices),
            _indices(indices) {}
    
        void operator () (unsigned int pos)
        {
            if (_indices) glVertex3fv((*_vertices)[_indices->index(pos)].ptr());
            else glVertex3fv((*_vertices)[pos].ptr());
        }
        
        const Vec3Array*   _vertices;
        const IndexArray*  _indices;
};

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

class DrawColor : public osg::ConstValueVisitor
{
    public:

        DrawColor(const Array* colors,const IndexArray* indices):
            _colors(colors),
            _indices(indices) {}

        void operator () (unsigned int pos)
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

class DrawTexCoord : public osg::ConstValueVisitor
{
    public:

        DrawTexCoord(const Array* texcoords,const IndexArray* indices):
            _texcoords(texcoords),
            _indices(indices) {}

        void operator () (unsigned int pos)
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

class DrawMultiTexCoord : public osg::ConstValueVisitor
{
    public:
    
        DrawMultiTexCoord(GLenum target,const Array* texcoords,const IndexArray* indices):
            _target(target),
            _texcoords(texcoords),
            _indices(indices) {}

        void operator () (unsigned int pos)
        {
            if (_indices) _texcoords->accept(_indices->index(pos),*this);
            else _texcoords->accept(pos,*this);
        }

        virtual void apply(const GLfloat& v){ glMultiTexCoord1f(_target,v); }
        virtual void apply(const Vec2& v)   { glMultiTexCoord2fv(_target,v.ptr()); }
        virtual void apply(const Vec3& v)   { glMultiTexCoord3fv(_target,v.ptr()); }
        virtual void apply(const Vec4& v)   { glMultiTexCoord4fv(_target,v.ptr()); }
        
        GLenum _target;
        const Array*        _texcoords;
        const IndexArray*   _indices;
};


typedef void (APIENTRY * SecondaryColor3ubvProc) (const GLubyte* coord);
typedef void (APIENTRY * SecondaryColor3fvProc) (const GLfloat* coord);
class DrawSecondaryColor : public osg::ConstValueVisitor
{
    public:
    
        DrawSecondaryColor(const Array* colors,const IndexArray* indices,
                          SecondaryColor3ubvProc sc3ubv,SecondaryColor3fvProc sc3fv):
            _colors(colors),
            _indices(indices),
            _glSecondaryColor3ubv(sc3ubv),
            _glSecondaryColor3fv(sc3fv) {}
    
        void operator () (unsigned int pos)
        {
            if (_indices) _colors->accept(_indices->index(pos),*this);
            else _colors->accept(pos,*this);
        }

        virtual void apply(const UByte4& v) { _glSecondaryColor3ubv(v.ptr()); }
        virtual void apply(const Vec3& v)   { _glSecondaryColor3fv(v.ptr()); }
        virtual void apply(const Vec4& v)   { _glSecondaryColor3fv(v.ptr()); }

        const Array*        _colors;
        const IndexArray*   _indices;

        SecondaryColor3ubvProc  _glSecondaryColor3ubv;
        SecondaryColor3fvProc   _glSecondaryColor3fv;
};

typedef void (APIENTRY * FogCoordProc) (const GLfloat* coord);
class DrawFogCoord : public osg::ConstValueVisitor
{
    public:
    
        DrawFogCoord(const FloatArray* fogcoords,const IndexArray* indices,
                        FogCoordProc fogCoordProc):
            _fogcoords(fogcoords),
            _indices(indices),
            _glFogCoord1fv(fogCoordProc) {}
    
        void operator () (unsigned int pos)
        {
            if (_indices) _glFogCoord1fv(&(*_fogcoords)[_indices->index(pos)]);
            else _glFogCoord1fv(&(*_fogcoords)[pos]);
        }

        const FloatArray*   _fogcoords;
        const IndexArray*   _indices;

        FogCoordProc _glFogCoord1fv;
};


IndexedGeometry::IndexedGeometry()
{
    _normalBinding = BIND_OFF;
    _colorBinding = BIND_OFF;
    _secondaryColorBinding = BIND_OFF;
    _fogCoordBinding = BIND_OFF;
}

IndexedGeometry::IndexedGeometry(const IndexedGeometry& geometry,const CopyOp& copyop):
    Drawable(geometry,copyop),
    _vertexArray(dynamic_cast<Vec3Array*>(copyop(geometry._vertexArray.get()))),
    _normalBinding(geometry._normalBinding),
    _normalArray(dynamic_cast<Vec3Array*>(copyop(geometry._normalArray.get()))),
    _colorBinding(geometry._colorBinding),
    _colorArray(copyop(geometry._colorArray.get())),
    _secondaryColorBinding(geometry._secondaryColorBinding),
    _secondaryColorArray(copyop(geometry._secondaryColorArray.get())),
    _fogCoordBinding(geometry._fogCoordBinding),
    _fogCoordArray(dynamic_cast<FloatArray*>(copyop(geometry._fogCoordArray.get())))
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
}

IndexedGeometry::~IndexedGeometry()
{
    // no need to delete, all automatically handled by ref_ptr :-)
}

void IndexedGeometry::setTexCoordArray(unsigned int unit,Array* array)
{
    if (_texCoordList.size()<=unit)
        _texCoordList.resize(unit+1);
        
   _texCoordList[unit].first = array;

    dirtyDisplayList();
}

Array* IndexedGeometry::getTexCoordArray(unsigned int unit)
{
    if (unit<_texCoordList.size()) return _texCoordList[unit].first.get();
    else return 0;
}

const Array* IndexedGeometry::getTexCoordArray(unsigned int unit) const
{
    if (unit<_texCoordList.size()) return _texCoordList[unit].first.get();
    else return 0;
}

void IndexedGeometry::drawImmediateMode(State& state)
{
    if (!_vertexArray.valid() || _vertexArray->empty()) return;
    if (_vertexIndices.valid() && _vertexIndices->empty()) return;
    
    // set up extensions.
    static SecondaryColor3ubvProc s_glSecondaryColor3ubv =
            (SecondaryColor3ubvProc) osg::getGLExtensionFuncPtr("glSecondaryColor3ubv","glSecondaryColor3ubvEXT");
    static SecondaryColor3fvProc s_glSecondaryColor3fv =
            (SecondaryColor3fvProc) osg::getGLExtensionFuncPtr("glSecondaryColor3fv","glSecondaryColor3fvEXT");

    static FogCoordProc s_glFogCoordfv =
            (FogCoordProc) osg::getGLExtensionFuncPtr("glFogCoordfv","glFogCoordfvEXT");

    DrawVertex         drawVertex(_vertexArray.get(),_vertexIndices.get());
    DrawNormal         drawNormal(_normalArray.get(),_normalIndices.get());
    DrawColor          drawColor(_colorArray.get(),_colorIndices.get());
    DrawSecondaryColor drawSecondaryColor(_secondaryColorArray.get(),_secondaryColorIndices.get(),
                                s_glSecondaryColor3ubv,s_glSecondaryColor3fv);
    DrawFogCoord       drawFogCoord(_fogCoordArray.get(),_fogCoordIndices.get(),
                                s_glFogCoordfv);


    typedef std::vector<DrawMultiTexCoord*> DrawTexCoordList;
    DrawTexCoordList drawTexCoordList;
    for(unsigned int unit=0;unit!=_texCoordList.size();++unit)
    {
        TexCoordArrayPair& texcoordPair = _texCoordList[unit];
        if (texcoordPair.first.valid() && !texcoordPair.first->empty())
        {
            if (texcoordPair.second.valid())
            {
                if (!texcoordPair.second->empty()) drawTexCoordList.push_back(new DrawMultiTexCoord(GL_TEXTURE0_ARB+unit,texcoordPair.first.get(),texcoordPair.second.get()));
            }
            else
            {
                drawTexCoordList.push_back(new DrawMultiTexCoord(GL_TEXTURE0_ARB+unit,texcoordPair.first.get(),0));
            }
        }
    }


    state.disableAllVertexArrays();
    
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // set up normals if required.
    //

    unsigned int normalIndex = 0;
    AttributeBinding normalBinding = _normalBinding;
    if (!_normalArray.valid() ||
        _normalArray->empty() ||
        (_normalIndices.valid() && _normalIndices->empty()) )
    {
        // switch off if not supported or have a valid data.
        normalBinding = BIND_OFF;
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // set up colours..
    //
    unsigned int colorIndex = 0;
    AttributeBinding colorBinding = _colorBinding;
    if (!_colorArray.valid() ||
        _colorArray->empty() ||
        (_colorIndices.valid() && _colorIndices->empty()) )
    {
        // switch off if not supported or have a valid data.
        colorBinding = BIND_OFF;
    }



    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up secondary color if required.
    //
    unsigned int secondaryColorIndex = 0;
    AttributeBinding secondaryColorBinding = _secondaryColorBinding;
    if (!_secondaryColorArray.valid() || 
        _secondaryColorArray->empty() ||
        !s_glSecondaryColor3ubv ||
        !s_glSecondaryColor3fv ||
        (_secondaryColorIndices.valid() && _secondaryColorIndices->empty()) )
    {
        // switch off if not supported or have a valid data.
        secondaryColorBinding = BIND_OFF;
    }

    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up fog coord if required.
    //
    unsigned int fogCoordIndex = 0;
    AttributeBinding fogCoordBinding = _fogCoordBinding;
    if (!_fogCoordArray.valid() || 
        _fogCoordArray->empty() ||
        !s_glFogCoordfv ||
        (_fogCoordIndices.valid() && _fogCoordIndices->empty()) )
    {
        // switch off if not supported or have a valid data.
        fogCoordBinding = BIND_OFF;
    }


    // pass the values onto OpenGL.
    if (normalBinding==BIND_OVERALL)            drawNormal(normalIndex++);
    if (colorBinding==BIND_OVERALL)             drawColor(colorIndex++);
    if (secondaryColorBinding==BIND_OVERALL)    drawSecondaryColor(secondaryColorIndex++);
    if (fogCoordBinding==BIND_OVERALL)          drawFogCoord(fogCoordIndex++);

    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // draw the primitives themselves.
    //
    
    for(PrimitiveSetList::iterator itr=_primitives.begin();
        itr!=_primitives.end();
        ++itr)
    {
        if (normalBinding==BIND_PER_PRIMITIVE_SET)            drawNormal(normalIndex++);
        if (colorBinding==BIND_PER_PRIMITIVE_SET)             drawColor(colorIndex++);
        if (secondaryColorBinding==BIND_PER_PRIMITIVE_SET)    drawSecondaryColor(secondaryColorIndex++);
        if (fogCoordBinding==BIND_PER_PRIMITIVE_SET)          drawFogCoord(fogCoordIndex++);
    
        //(*itr)->draw();
        
        PrimitiveSet* primitiveset = itr->get();
        switch(primitiveset->getType())
        {
            case(PrimitiveSet::DrawArraysPrimitiveType):
            {
                DrawArrays* drawArray = static_cast<DrawArrays*>(primitiveset);
                glBegin(primitiveset->getMode());
                
                unsigned int indexEnd = drawArray->getFirst()+drawArray->getCount();
                for(unsigned int vindex=drawArray->getFirst();
                    vindex!=indexEnd;
                    ++vindex)
                {
                    if (normalBinding==BIND_PER_VERTEX)            drawNormal(vindex);
                    if (colorBinding==BIND_PER_VERTEX)             drawColor(vindex);
                    if (secondaryColorBinding==BIND_PER_VERTEX)    drawSecondaryColor(vindex);
                    if (fogCoordBinding==BIND_PER_VERTEX)          drawFogCoord(vindex);

                    for(DrawTexCoordList::iterator texItr=drawTexCoordList.begin();
                        texItr!=drawTexCoordList.end();
                        ++texItr)
                    {
                        (*(*texItr))(vindex);
                    }

                    drawVertex(vindex);
                }
    
                glEnd();
                break;
            }
            case(PrimitiveSet::DrawArrayLengthsPrimitiveType):
            {
                DrawArrayLengths* drawArrayLengths = static_cast<DrawArrayLengths*>(primitiveset);
                GLenum mode = primitiveset->getMode();
                
                unsigned int vindex=drawArrayLengths->getFirst();
                for(DrawArrayLengths::const_iterator primItr=drawArrayLengths->begin();
                    primItr!=drawArrayLengths->end();
                    ++primItr)
                {
                    glBegin(mode);
                
                    for(GLsizei count=0;
                        count<*primItr;
                        ++count,++vindex)
                    {
                        if (normalBinding==BIND_PER_VERTEX)            drawNormal(vindex);
                        if (colorBinding==BIND_PER_VERTEX)             drawColor(vindex);
                        if (secondaryColorBinding==BIND_PER_VERTEX)    drawSecondaryColor(vindex);
                        if (fogCoordBinding==BIND_PER_VERTEX)          drawFogCoord(vindex);

                        for(DrawTexCoordList::iterator texItr=drawTexCoordList.begin();
                            texItr!=drawTexCoordList.end();
                            ++texItr)
                        {
                            (*(*texItr))(vindex);
                        }

                        drawVertex(vindex);
                    }

                    glEnd();
                    
                }
                break;
            }
            case(PrimitiveSet::DrawElementsUBytePrimitiveType):
            {
                DrawElementsUByte* drawElements = static_cast<DrawElementsUByte*>(primitiveset);
                glBegin(primitiveset->getMode());
                
                for(DrawElementsUByte::const_iterator primItr=drawElements->begin();
                    primItr!=drawElements->end();
                    ++primItr)
                {

                    unsigned int vindex=*primItr;

                    if (normalBinding==BIND_PER_VERTEX)            drawNormal(vindex);
                    if (colorBinding==BIND_PER_VERTEX)             drawColor(vindex);
                    if (secondaryColorBinding==BIND_PER_VERTEX)    drawSecondaryColor(vindex);
                    if (fogCoordBinding==BIND_PER_VERTEX)          drawFogCoord(vindex);

                    for(DrawTexCoordList::iterator texItr=drawTexCoordList.begin();
                        texItr!=drawTexCoordList.end();
                        ++texItr)
                    {
                        (*(*texItr))(vindex);
                    }

                    drawVertex(vindex);
                }
    
                glEnd();
                break;
            }
            case(PrimitiveSet::DrawElementsUShortPrimitiveType):
            {
                DrawElementsUShort* drawElements = static_cast<DrawElementsUShort*>(primitiveset);
                glBegin(primitiveset->getMode());
                
                for(DrawElementsUShort::const_iterator primItr=drawElements->begin();
                    primItr!=drawElements->end();
                    ++primItr)
                {

                    unsigned int vindex=*primItr;

                    if (normalBinding==BIND_PER_VERTEX)            drawNormal(vindex);
                    if (colorBinding==BIND_PER_VERTEX)             drawColor(vindex);
                    if (secondaryColorBinding==BIND_PER_VERTEX)    drawSecondaryColor(vindex);
                    if (fogCoordBinding==BIND_PER_VERTEX)          drawFogCoord(vindex);

                    for(DrawTexCoordList::iterator texItr=drawTexCoordList.begin();
                        texItr!=drawTexCoordList.end();
                        ++texItr)
                    {
                        (*(*texItr))(vindex);
                    }

                    drawVertex(vindex);
                }
    
                glEnd();
                break;
            }
            case(PrimitiveSet::DrawElementsUIntPrimitiveType):
            {
                DrawElementsUInt* drawElements = static_cast<DrawElementsUInt*>(primitiveset);
                glBegin(primitiveset->getMode());
                
                for(DrawElementsUInt::const_iterator primItr=drawElements->begin();
                    primItr!=drawElements->end();
                    ++primItr)
                {

                    unsigned int vindex=*primItr;

                    if (normalBinding==BIND_PER_VERTEX)            drawNormal(vindex);
                    if (colorBinding==BIND_PER_VERTEX)             drawColor(vindex);
                    if (secondaryColorBinding==BIND_PER_VERTEX)    drawSecondaryColor(vindex);
                    if (fogCoordBinding==BIND_PER_VERTEX)          drawFogCoord(vindex);

                    for(DrawTexCoordList::iterator texItr=drawTexCoordList.begin();
                        texItr!=drawTexCoordList.end();
                        ++texItr)
                    {
                        (*(*texItr))(vindex);
                    }

                    drawVertex(vindex);
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


void IndexedGeometry::accept(AttributeFunctor& af)
{
    if (_vertexArray.valid() && !_vertexArray->empty())
    {
        af.apply(VERTICES,_vertexArray->size(),&(_vertexArray->front()));
    }
    
    if (_normalArray.valid() && !_normalArray->empty())
    {
        af.apply(NORMALS,_normalArray->size(),&(_normalArray->front()));
    }
    // need to add other attriubtes
}

void IndexedGeometry::accept(PrimitiveFunctor& functor)
{
    if (!_vertexArray.valid() || _vertexArray->empty()) return;
    
    functor.setVertexArray(_vertexArray->size(),&(_vertexArray->front()));
    
    for(PrimitiveSetList::iterator itr=_primitives.begin();
        itr!=_primitives.end();
        ++itr)
    {
        (*itr)->accept(functor);
    }
    
}

bool IndexedGeometry::verifyBindings() const
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
        case(BIND_PER_VERTEX):
            if (_vertexArray.valid())
            {
                if (!_normalArray.valid()) return false;
                if (_normalArray->getNumElements()!=_vertexArray->getNumElements()) return false;        
            }
            else if (_normalArray.valid() && _normalArray->getNumElements()>0) return false;
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
        case(BIND_PER_VERTEX):
            if (_vertexArray.valid())
            {
                if (!_colorArray.valid()) return false;
                if (_colorArray->getNumElements()!=_vertexArray->getNumElements()) return false;
            }
            else if (_colorArray.valid() && _colorArray->getNumElements()>0) return false;
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
        else if (array && array->getNumElements()>0) return false;
    }

    return true;
}

void IndexedGeometry::computeCorrectBindingsAndArraySizes()
{
    if (verifyBindings()) return;

    if (!_vertexArray.valid() || _vertexArray->empty())
    {
        // no vertex array so switch everything off.
        
        _vertexArray = 0;
        
        _colorArray = 0;
        _colorBinding = BIND_OFF;

        _normalArray = 0;
        _normalBinding = BIND_OFF;
        
        _texCoordList.clear();
        
        notify(INFO)<<"Info: remove redundent attribute arrays from empty osg::IndexedGeometry"<<std::endl;
        
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
        case(BIND_PER_PRIMITIVE):
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
