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

class DrawTexCoord : public osg::Referenced, public osg::ConstValueVisitor
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

typedef void (APIENTRY * MultiTexCoord1fProc) (GLenum target,GLfloat coord);
typedef void (APIENTRY * MultiTexCoordfvProc) (GLenum target,const GLfloat* coord);
class DrawMultiTexCoord : public osg::Referenced, public osg::ConstValueVisitor
{
    public:
    
        DrawMultiTexCoord(GLenum target,const Array* texcoords,const IndexArray* indices,
                          MultiTexCoord1fProc glMultiTexCoord1f,
                          MultiTexCoordfvProc glMultiTexCoord2fv,
                          MultiTexCoordfvProc glMultiTexCoord3fv,
                          MultiTexCoordfvProc glMultiTexCoord4fv):
            _target(target),
            _texcoords(texcoords),
            _indices(indices),
            _glMultiTexCoord1f(glMultiTexCoord1f),
            _glMultiTexCoord2fv(glMultiTexCoord2fv),
            _glMultiTexCoord3fv(glMultiTexCoord3fv),
            _glMultiTexCoord4fv(glMultiTexCoord4fv) {}

        void operator () (unsigned int pos)
        {
            if (_indices) _texcoords->accept(_indices->index(pos),*this);
            else _texcoords->accept(pos,*this);
        }

        virtual void apply(const GLfloat& v){ _glMultiTexCoord1f(_target,v); }
        virtual void apply(const Vec2& v)   { _glMultiTexCoord2fv(_target,v.ptr()); }
        virtual void apply(const Vec3& v)   { _glMultiTexCoord3fv(_target,v.ptr()); }
        virtual void apply(const Vec4& v)   { _glMultiTexCoord4fv(_target,v.ptr()); }
        
        GLenum _target;
        const Array*        _texcoords;
        const IndexArray*   _indices;
        MultiTexCoord1fProc _glMultiTexCoord1f;
        MultiTexCoordfvProc _glMultiTexCoord2fv;
        MultiTexCoordfvProc _glMultiTexCoord3fv;
        MultiTexCoordfvProc _glMultiTexCoord4fv;
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
    if (!_vertexArray.valid() || _vertexArray->getNumElements()>0) return;
    if (_vertexIndices.valid() && _vertexIndices->getNumElements()>0) return;
    
    // set up extensions.
    static SecondaryColor3ubvProc s_glSecondaryColor3ubv =
            (SecondaryColor3ubvProc) osg::getGLExtensionFuncPtr("glSecondaryColor3ubv","glSecondaryColor3ubvEXT");
    static SecondaryColor3fvProc s_glSecondaryColor3fv =
            (SecondaryColor3fvProc) osg::getGLExtensionFuncPtr("glSecondaryColor3fv","glSecondaryColor3fvEXT");

    static FogCoordProc s_glFogCoordfv =
            (FogCoordProc) osg::getGLExtensionFuncPtr("glFogCoordfv","glFogCoordfvEXT");

    static MultiTexCoord1fProc s_glMultiTexCoord1f =
            (MultiTexCoord1fProc) osg::getGLExtensionFuncPtr("glMultiTexCoord1f","glMultiTexCoord1fARB");
    static MultiTexCoordfvProc s_glMultiTexCoord2fv =
            (MultiTexCoordfvProc) osg::getGLExtensionFuncPtr("glMultiTexCoord2fv","glMultiTexCoord2fvARB");
    static MultiTexCoordfvProc s_glMultiTexCoord3fv =
            (MultiTexCoordfvProc) osg::getGLExtensionFuncPtr("glMultiTexCoord3fv","glMultiTexCoord3fvARB");
    static MultiTexCoordfvProc s_glMultiTexCoord4fv =
            (MultiTexCoordfvProc) osg::getGLExtensionFuncPtr("glMultiTexCoord4fv","glMultiTexCoord4fvARB");

    DrawVertex         drawVertex(_vertexArray.get(),_vertexIndices.get());
    DrawNormal         drawNormal(_normalArray.get(),_normalIndices.get());
    DrawColor          drawColor(_colorArray.get(),_colorIndices.get());
    DrawSecondaryColor drawSecondaryColor(_secondaryColorArray.get(),_secondaryColorIndices.get(),
                                s_glSecondaryColor3ubv,s_glSecondaryColor3fv);
    DrawFogCoord       drawFogCoord(_fogCoordArray.get(),_fogCoordIndices.get(),
                                s_glFogCoordfv);


    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // set up normals if required.
    //
    unsigned int normalIndex = 0;
    AttributeBinding normalBinding = _normalBinding;
    if (!_normalArray.valid() ||
        _normalArray->empty() ||
        (_normalIndices.valid() && _normalIndices->getNumElements()>0) )
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
        _colorArray->getNumElements()>0 ||
        (_colorIndices.valid() && _colorIndices->getNumElements()>0) )
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
        _secondaryColorArray->getNumElements()>0 ||
        !s_glSecondaryColor3ubv ||
        !s_glSecondaryColor3fv ||
        (_secondaryColorIndices.valid() && _secondaryColorIndices->getNumElements()>0) )
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
        _fogCoordArray->getNumElements()>0 ||
        !s_glFogCoordfv ||
        (_fogCoordIndices.valid() && _fogCoordIndices->getNumElements()>0) )
    {
        // switch off if not supported or have a valid data.
        fogCoordBinding = BIND_OFF;
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // check to see if fast path can be used.
    //
    bool fastPath = true;
    if (normalBinding==BIND_PER_PRIMITIVE || (normalBinding!=BIND_OFF && _normalIndices.valid())) fastPath = false;
    else if (colorBinding==BIND_PER_PRIMITIVE || (colorBinding!=BIND_OFF && _colorIndices.valid())) fastPath = false;
    else if (secondaryColorBinding==BIND_PER_PRIMITIVE || (secondaryColorBinding!=BIND_OFF && _secondaryColorIndices.valid())) fastPath = false;
    else if (fogCoordBinding==BIND_PER_PRIMITIVE || (fogCoordBinding!=BIND_OFF && _fogCoordIndices.valid())) fastPath = false;



    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up tex coords if required.
    //
    typedef std::vector< ref_ptr<DrawMultiTexCoord> > DrawTexCoordList;
    DrawTexCoordList drawTexCoordList;
    drawTexCoordList.reserve(_texCoordList.size());
    
    // fallback if multitexturing not supported.
    ref_ptr<DrawTexCoord> drawTextCoord;

    if (s_glMultiTexCoord2fv)
    {
        // multitexture supported..
        for(unsigned int unit=0;unit!=_texCoordList.size();++unit)
        {
            TexCoordArrayPair& texcoordPair = _texCoordList[unit];
            if (texcoordPair.first.valid() && texcoordPair.first->getNumElements()>0)
            {
                if (texcoordPair.second.valid())
                {
                    if (!texcoordPair.second->getNumElements()>0)
                    {
                        drawTexCoordList.push_back(new DrawMultiTexCoord(GL_TEXTURE0+unit,texcoordPair.first.get(),texcoordPair.second.get(),
                                                                         s_glMultiTexCoord1f,
                                                                         s_glMultiTexCoord2fv,
                                                                         s_glMultiTexCoord3fv,
                                                                         s_glMultiTexCoord4fv));

                        fastPath = false;           
                    }
                }
                else
                {
                    drawTexCoordList.push_back(new DrawMultiTexCoord(GL_TEXTURE0+unit,texcoordPair.first.get(),0,
                                                                     s_glMultiTexCoord1f,
                                                                     s_glMultiTexCoord2fv,
                                                                     s_glMultiTexCoord3fv,
                                                                     s_glMultiTexCoord4fv));
                }
            }
        }
    }
    else
    {
        if (!_texCoordList.empty())
        {
            TexCoordArrayPair& texcoordPair = _texCoordList[0];
            if (texcoordPair.first.valid() && !texcoordPair.first->getNumElements()>0)
            {
                if (texcoordPair.second.valid())
                {
                    if (!texcoordPair.second->getNumElements()>0)
                    {
                        drawTextCoord = new DrawTexCoord(texcoordPair.first.get(),texcoordPair.second.get());

                        fastPath = false;           
                    }
                }
                else
                {
                    drawTextCoord = new DrawTexCoord(texcoordPair.first.get(),0);
                }
            }
            
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // set up vertex arrays if appropriate.
    //
    if (fastPath)
    {
        state.setVertexPointer(3,GL_FLOAT,0,_vertexArray->getDataPointer());
    
        if (normalBinding==BIND_PER_VERTEX)
            state.setNormalPointer(GL_FLOAT,0,_normalArray->getDataPointer());
        else
            state.disableNormalPointer();
            
        if (colorBinding==BIND_PER_VERTEX)
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
            Array* array = _texCoordList[unit].first.get();
            if (array)
                state.setTexCoordPointer(unit,array->getDataSize(),array->getDataType(),0,array->getDataPointer());
            else
                state.disableTexCoordPointer(unit);
        }
        state.disableTexCoordPointersAboveAndIncluding(unit);
    }
    else
    {   
        // disable all the vertex arrays in the slow path as we are
        // sending everything using glVertex etc.
        state.disableAllVertexArrays();
    }
    


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // pass the overall binding values onto OpenGL.
    //
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
    
        if (fastPath)
        {
            (*itr)->draw();
        }
        else
        {
            // draw primtives by the more flexible "slow" path,
            // sending OpenGL glBegin/glVertex.../glEnd().
            PrimitiveSet* primitiveset = itr->get();
            switch(primitiveset->getType())
            {
                case(PrimitiveSet::DrawArraysPrimitiveType):
                {
                    const DrawArrays* drawArray = static_cast<const DrawArrays*>(primitiveset);
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
                        if (drawTextCoord.valid()) (*drawTextCoord)(vindex);

                        drawVertex(vindex);
                    }

                    glEnd();
                    break;
                }
                case(PrimitiveSet::DrawArrayLengthsPrimitiveType):
                {
                    const DrawArrayLengths* drawArrayLengths = static_cast<const DrawArrayLengths*>(primitiveset);
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
                            if (drawTextCoord.valid()) (*drawTextCoord)(vindex);

                            drawVertex(vindex);
                        }

                        glEnd();

                    }
                    break;
                }
                case(PrimitiveSet::DrawElementsUBytePrimitiveType):
                {
                    const DrawElementsUByte* drawElements = static_cast<const DrawElementsUByte*>(primitiveset);
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
                        if (drawTextCoord.valid()) (*drawTextCoord)(vindex);

                        drawVertex(vindex);
                    }

                    glEnd();
                    break;
                }
                case(PrimitiveSet::DrawElementsUShortPrimitiveType):
                {
                    const DrawElementsUShort* drawElements = static_cast<const DrawElementsUShort*>(primitiveset);
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
                        if (drawTextCoord.valid()) (*drawTextCoord)(vindex);

                        drawVertex(vindex);
                    }

                    glEnd();
                    break;
                }
                case(PrimitiveSet::DrawElementsUIntPrimitiveType):
                {
                    const DrawElementsUInt* drawElements = static_cast<const DrawElementsUInt*>(primitiveset);
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
                        if (drawTextCoord.valid()) (*drawTextCoord)(vindex);

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
        case(BIND_PER_PRIMITIVE):
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
        case(BIND_PER_PRIMITIVE):
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
        case(BIND_PER_PRIMITIVE_SET):
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
