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
#include <osg/GLExtensions>
#include <osg/Geometry>
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
    
        DrawFogCoord(const Array* fogcoords,const IndexArray* indices,FogCoordProc fogCoordProc):
            _fogcoords(fogcoords),
            _indices(indices),
            _glFogCoord1fv(fogCoordProc) {}
    
        void operator () (unsigned int pos)
        {
            if (_indices) _fogcoords->accept(_indices->index(pos),*this);
            else _fogcoords->accept(pos,*this);
        }

        virtual void apply(const GLfloat& v) { _glFogCoord1fv(&v); }

        const Array*        _fogcoords;
        const IndexArray*   _indices;

        FogCoordProc        _glFogCoord1fv;
};


Geometry::Geometry()
{
    _normalBinding = BIND_OFF;
    _colorBinding = BIND_OFF;
    _secondaryColorBinding = BIND_OFF;
    _fogCoordBinding = BIND_OFF;

    _fastPathComputed = false;
    _fastPath = false;
}

Geometry::Geometry(const Geometry& geometry,const CopyOp& copyop):
    Drawable(geometry,copyop),
    _vertexArray(dynamic_cast<Vec3Array*>(copyop(geometry._vertexArray.get()))),
    _normalBinding(geometry._normalBinding),
    _normalArray(dynamic_cast<Vec3Array*>(copyop(geometry._normalArray.get()))),
    _colorBinding(geometry._colorBinding),
    _colorArray(copyop(geometry._colorArray.get())),
    _secondaryColorBinding(geometry._secondaryColorBinding),
    _secondaryColorArray(copyop(geometry._secondaryColorArray.get())),
    _fogCoordBinding(geometry._fogCoordBinding),
    _fogCoordArray(dynamic_cast<FloatArray*>(copyop(geometry._fogCoordArray.get()))),
    _fastPathComputed(geometry._fastPathComputed),
    _fastPath(geometry._fastPath)
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
        if (i+numElementsToRemove<_primitives.size())
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

bool Geometry::areFastPathsUsed() const
{
    if (_fastPathComputed) return _fastPath;

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
    // check to see if fast path can be used.
    //
    _fastPath = true;
    if (_normalBinding==BIND_PER_PRIMITIVE || (_normalBinding==BIND_PER_VERTEX && _normalIndices.valid())) _fastPath = false;
    else if (_colorBinding==BIND_PER_PRIMITIVE || (_colorBinding==BIND_PER_VERTEX && _colorIndices.valid())) _fastPath = false;
    else if (_secondaryColorBinding==BIND_PER_PRIMITIVE || (_secondaryColorBinding==BIND_PER_VERTEX && _secondaryColorIndices.valid())) _fastPath = false;
    else if (_fogCoordBinding==BIND_PER_PRIMITIVE || (_fogCoordBinding==BIND_PER_VERTEX && _fogCoordIndices.valid())) _fastPath = false;

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up tex coords if required.
    //
    for(unsigned int unit=0;unit!=_texCoordList.size();++unit)
    {
        const TexCoordArrayPair& texcoordPair = _texCoordList[unit];
        if (texcoordPair.first.valid() && texcoordPair.first->getNumElements()>0)
        {
            if (texcoordPair.second.valid())
            {
                if (texcoordPair.second->getNumElements()>0)
                {
                    _fastPath = false;           
                }
            }
        }
    }

    _fastPathComputed = true;

    return _fastPath;
}

void Geometry::drawImplementation(State& state) const
{
    if (!_vertexArray.valid() || _vertexArray->getNumElements()==0) return;
    if (_vertexIndices.valid() && _vertexIndices->getNumElements()==0) return;
    
    // set up extensions.
    static SecondaryColor3ubvProc s_glSecondaryColor3ubv =
            (SecondaryColor3ubvProc) osg::getGLExtensionFuncPtr("glSecondaryColor3ubv","glSecondaryColor3ubvEXT");
    static SecondaryColor3fvProc s_glSecondaryColor3fv =
            (SecondaryColor3fvProc) osg::getGLExtensionFuncPtr("glSecondaryColor3fv","glSecondaryColor3fvEXT");

    static FogCoordProc s_glFogCoordfv =
            (FogCoordProc) osg::getGLExtensionFuncPtr("glFogCoordfv","glFogCoordfvEXT");

    DrawNormal         drawNormal(_normalArray.get(),_normalIndices.get());
    DrawColor          drawColor(_colorArray.get(),_colorIndices.get());
    DrawSecondaryColor drawSecondaryColor(_secondaryColorArray.get(),_secondaryColorIndices.get(),
                                s_glSecondaryColor3ubv,s_glSecondaryColor3fv);
    DrawFogCoord       drawFogCoord(_fogCoordArray.get(),_fogCoordIndices.get(),
                                s_glFogCoordfv);


    

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up secondary color if required.
    //
    AttributeBinding secondaryColorBinding = _secondaryColorBinding;
    if (secondaryColorBinding!=BIND_OFF && (!s_glSecondaryColor3ubv || s_glSecondaryColor3fv))
    {
        // switch off if not supported or have a valid data.
        secondaryColorBinding = BIND_OFF;
    }

    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up fog coord if required.
    //
    AttributeBinding fogCoordBinding = _fogCoordBinding;
    if (fogCoordBinding!=BIND_OFF && !s_glFogCoordfv)
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
    if (_normalBinding==BIND_OFF) glNormal3f(0.0f,0.0f,1.0f);
#endif

#if USE_DEFAULT_COLOUR
    if (_colorBinding==BIND_OFF) glColor4f(1.0f,1.0f,1.0f,1.0f);
#endif

    if (areFastPathsUsed())
    {
    
        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        // fast path.        
        //

        state.setVertexPointer(3,GL_FLOAT,0,_vertexArray->getDataPointer());
    
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


        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        // pass the overall binding values onto OpenGL.
        //
        if (_normalBinding==BIND_OVERALL)           drawNormal(normalIndex++);
        if (_colorBinding==BIND_OVERALL)            drawColor(colorIndex++);
        if (secondaryColorBinding==BIND_OVERALL)    drawSecondaryColor(secondaryColorIndex++);
        if (fogCoordBinding==BIND_OVERALL)          drawFogCoord(fogCoordIndex++);


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

            (*itr)->draw();

        }

    }
    else
    {   

        ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
        //
        // slow path.        
        //
        

        // Set up tex coords if required.
        static MultiTexCoord1fProc s_glMultiTexCoord1f =
                (MultiTexCoord1fProc) osg::getGLExtensionFuncPtr("glMultiTexCoord1f","glMultiTexCoord1fARB");
        static MultiTexCoordfvProc s_glMultiTexCoord2fv =
                (MultiTexCoordfvProc) osg::getGLExtensionFuncPtr("glMultiTexCoord2fv","glMultiTexCoord2fvARB");
        static MultiTexCoordfvProc s_glMultiTexCoord3fv =
                (MultiTexCoordfvProc) osg::getGLExtensionFuncPtr("glMultiTexCoord3fv","glMultiTexCoord3fvARB");
        static MultiTexCoordfvProc s_glMultiTexCoord4fv =
                (MultiTexCoordfvProc) osg::getGLExtensionFuncPtr("glMultiTexCoord4fv","glMultiTexCoord4fvARB");


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
                const TexCoordArrayPair& texcoordPair = _texCoordList[unit];
                if (texcoordPair.first.valid() && texcoordPair.first->getNumElements()>0)
                {
                    if (texcoordPair.second.valid())
                    {
                        if (texcoordPair.second->getNumElements()>0)
                        {
                            drawTexCoordList.push_back(new DrawMultiTexCoord(GL_TEXTURE0+unit,texcoordPair.first.get(),texcoordPair.second.get(),
                                                                             s_glMultiTexCoord1f,
                                                                             s_glMultiTexCoord2fv,
                                                                             s_glMultiTexCoord3fv,
                                                                             s_glMultiTexCoord4fv));

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
                const TexCoordArrayPair& texcoordPair = _texCoordList[0];
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


        // set up vertex functor.
        DrawVertex drawVertex(_vertexArray.get(),_vertexIndices.get());

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
                        }

                        if (_normalBinding==BIND_PER_VERTEX)           drawNormal(vindex);
                        if (_colorBinding==BIND_PER_VERTEX)            drawColor(vindex);
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
                            }
                            
                            if (_normalBinding==BIND_PER_VERTEX)           drawNormal(vindex);
                            if (_colorBinding==BIND_PER_VERTEX)            drawColor(vindex);
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
                        }
                        
                        unsigned int vindex=*primItr;

                        if (_normalBinding==BIND_PER_VERTEX)           drawNormal(vindex);
                        if (_colorBinding==BIND_PER_VERTEX)            drawColor(vindex);
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
                        }
                        
                        unsigned int vindex=*primItr;

                        if (_normalBinding==BIND_PER_VERTEX)           drawNormal(vindex);
                        if (_colorBinding==BIND_PER_VERTEX)            drawColor(vindex);
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
                        }
                        
                        unsigned int vindex=*primItr;

                        if (_normalBinding==BIND_PER_VERTEX)           drawNormal(vindex);
                        if (_colorBinding==BIND_PER_VERTEX)            drawColor(vindex);
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
    if (!_vertexArray.valid() || _vertexArray->empty()) return;
    
    
    if (!_vertexIndices.valid())
    {
        functor.setVertexArray(_vertexArray->size(),&(_vertexArray->front()));

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
                        functor.vertex((*_vertexArray)[_vertexIndices->index(vindex)]);
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
                            functor.vertex((*_vertexArray)[_vertexIndices->index(vindex)]);
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
                        functor.vertex((*_vertexArray)[_vertexIndices->index(vindex)]);
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
                        functor.vertex((*_vertexArray)[_vertexIndices->index(vindex)]);
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
                        functor.vertex((*_vertexArray)[_vertexIndices->index(vindex)]);
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

    if (!_vertexArray.valid() || _vertexArray->empty())
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
