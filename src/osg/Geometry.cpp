#include <osg/Geometry>
#include <osg/Notify>

using namespace osg;

Geometry::Geometry()
{
    _normalBinding = BIND_OFF;
    _colorBinding = BIND_OFF;
    _secondaryColorBinding = BIND_OFF;
    _fogCoordBinding = BIND_OFF;
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
    _fogCoordArray(dynamic_cast<FloatArray*>(copyop(geometry._fogCoordArray.get())))
{
    for(PrimitiveList::const_iterator pitr=geometry._primitives.begin();
        pitr!=geometry._primitives.end();
        ++pitr)
    {
        Primitive* primitive = copyop(pitr->get());
        if (primitive) _primitives.push_back(primitive);
    }

    for(TexCoordArrayList::const_iterator titr=geometry._texCoordList.begin();
        titr!=geometry._texCoordList.end();
        ++titr)
    {
        _texCoordList.push_back(copyop(titr->get()));
    }
}

Geometry::~Geometry()
{
    // no need to delete, all automatically handled by ref_ptr :-)
}

void Geometry::setTexCoordArray(unsigned int unit,Array* array)
{
    if (_texCoordList.size()<=unit)
        _texCoordList.resize(unit+1,0);
        
   _texCoordList[unit] = array;

    dirtyDisplayList();
}

Array* Geometry::getTexCoordArray(unsigned int unit)
{
    if (unit<_texCoordList.size()) return _texCoordList[unit].get();
    else return 0;
}

const Array* Geometry::getTexCoordArray(unsigned int unit) const
{
    if (unit<_texCoordList.size()) return _texCoordList[unit].get();
    else return 0;
}

typedef void (APIENTRY * FogCoordProc) (const GLfloat* coord);
typedef void (APIENTRY * SecondaryColor3ubvProc) (const GLubyte* coord);
typedef void (APIENTRY * SecondaryColor3fvProc) (const GLfloat* coord);

void Geometry::drawImmediateMode(State& state)
{
    if (!_vertexArray.valid()) return;
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // set up the vertex arrays.
    //
    state.setVertexPointer(3,GL_FLOAT,0,_vertexArray->getDataPointer());
    

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // set up texture coordinates.
    //
    unsigned int i;
    for(i=0;i<_texCoordList.size();++i)
    {
        Array* array = _texCoordList[i].get();
        if (array)
            state.setTexCoordPointer(i,array->getDataSize(),array->getDataType(),0,array->getDataPointer());
        else
            state.disableTexCoordPointer(i);
    }
    state.disableTexCoordPointersAboveAndIncluding(i);
    
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
;    // set up normals if required.
    //
    Vec3* normalPointer = 0;
    if (_normalArray.valid() && !_normalArray->empty()) normalPointer = &(_normalArray->front());

    AttributeBinding normalBinding = _normalBinding;
    if (normalBinding!=BIND_OFF && !normalPointer)
    {
        // switch off if not supported or have a valid data.
        normalBinding = BIND_OFF;
    }

    switch (normalBinding)
    {
        case(BIND_OFF):
            state.disableNormalPointer();
            break;
        case(BIND_OVERALL):
            state.disableNormalPointer();
            glNormal3fv(reinterpret_cast<const GLfloat*>(normalPointer));
            break;
        case(BIND_PER_PRIMITIVE):
            state.disableNormalPointer();
            break;
        case(BIND_PER_VERTEX):
            state.setNormalPointer(GL_FLOAT,0,normalPointer);
            break;
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up color if required.
    //
    // set up colors, complicated by the fact that the color array
    // might be bound in 4 different ways, and be represented as 3 different data types -
    // Vec3, Vec4 or UByte4 Arrays.
    //
    const unsigned char* colorPointer = 0;
    unsigned int colorStride = 0;
    Array::Type colorType = Array::ArrayType;
    if (_colorArray.valid())
    {
        colorType = _colorArray->getType();
        switch(colorType)
        {
            case(Array::UByte4ArrayType):
            {
                colorPointer = reinterpret_cast<const unsigned char*>(_colorArray->getDataPointer());
                colorStride = 4;
                break;
            }
            case(Array::Vec3ArrayType):
            {
                colorPointer = reinterpret_cast<const unsigned char*>(_colorArray->getDataPointer());
                colorStride = 12;
                break;
            }
            case(Array::Vec4ArrayType):
            {
                colorPointer = reinterpret_cast<const unsigned char*>(_colorArray->getDataPointer());
                colorStride = 16;
                break;
            }
            default:
                break;
        }
    }
    
    
    AttributeBinding colorBinding = _colorBinding;
    if (colorBinding!=BIND_OFF && !colorPointer)
    {
        // switch off if not supported or have a valid data.
        colorBinding = BIND_OFF;
    }

    switch (colorBinding)
    {
        case(BIND_OFF):
            state.disableColorPointer();
            break;
        case(BIND_OVERALL):
            state.disableColorPointer();
            if (colorPointer)
            {
                switch(colorType)
                {
                    case(Array::UByte4ArrayType):
                        glColor4ubv(reinterpret_cast<const GLubyte*>(colorPointer));
                        break;
                    case(Array::Vec3ArrayType):
                        glColor3fv(reinterpret_cast<const GLfloat*>(colorPointer));
                        break;
                    case(Array::Vec4ArrayType):
                        glColor4fv(reinterpret_cast<const GLfloat*>(colorPointer));
                        break;
                    default:
                        break;
                }
            }
            break;
        case(BIND_PER_PRIMITIVE):
            state.disableColorPointer();
            break;
        case(BIND_PER_VERTEX):
            state.setColorPointer(_colorArray->getDataSize(),_colorArray->getDataType(),0,colorPointer);
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up secondary color if required.
    //
    // set up colors, complicated by the fact that the color array
    // might be bound in 4 different ways, and be represented as 3 different data types -
    // Vec3, Vec4 or UByte4 Arrays.
    const unsigned char* secondaryColorPointer = 0;
    unsigned int secondaryColorStride = 0;
    Array::Type secondaryColorType = Array::ArrayType;
    if (_secondaryColorArray.valid())
    {
        secondaryColorType = _secondaryColorArray->getType();
        switch(secondaryColorType)
        {
            case(Array::UByte4ArrayType):
            {
                secondaryColorPointer = reinterpret_cast<const unsigned char*>(_secondaryColorArray->getDataPointer());
                secondaryColorStride = 4;
                break;
            }
            case(Array::Vec3ArrayType):
            {
                secondaryColorPointer = reinterpret_cast<const unsigned char*>(_secondaryColorArray->getDataPointer());
                secondaryColorStride = 12;
                break;
            }
            default:
                break;
        }
    }
    
    static SecondaryColor3ubvProc s_glSecondaryColor3ubv =
            (SecondaryColor3ubvProc) osg::getGLExtensionFuncPtr("glSecondaryColor3ubv","glSecondaryColor3ubvEXT");
    static SecondaryColor3fvProc s_glSecondaryColor3fv =
            (SecondaryColor3fvProc) osg::getGLExtensionFuncPtr("glSecondaryColor3fv","glSecondaryColor3fvEXT");

    AttributeBinding secondaryColorBinding = _secondaryColorBinding;
    if (secondaryColorBinding!=BIND_OFF && (!secondaryColorPointer || !s_glSecondaryColor3ubv || !s_glSecondaryColor3fv))
    {
        // switch off if not supported or have a valid data.
        secondaryColorBinding = BIND_OFF;
    }


    switch (secondaryColorBinding)
    {
        case(BIND_OFF):
            state.disableSecondaryColorPointer();
            break;
        case(BIND_OVERALL):
            state.disableSecondaryColorPointer();
            if (secondaryColorPointer)
            {
                switch(secondaryColorType)
                {
                    case(Array::UByte4ArrayType):
                        s_glSecondaryColor3ubv(reinterpret_cast<const GLubyte*>(secondaryColorPointer));
                        break;
                    case(Array::Vec3ArrayType):
                        s_glSecondaryColor3fv(reinterpret_cast<const GLfloat*>(secondaryColorPointer));
                        break;
                    default:
                        break;
                }
            }
            break;
        case(BIND_PER_PRIMITIVE):
            state.disableSecondaryColorPointer();
            break;
        case(BIND_PER_VERTEX):
            state.setSecondaryColorPointer(_secondaryColorArray->getDataSize(),_secondaryColorArray->getDataType(),0,secondaryColorPointer);
    }


    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // Set up fog coord if required.
    //

    GLfloat* fogCoordPointer = 0;
    if (_fogCoordArray.valid() && !_fogCoordArray->empty()) fogCoordPointer = &(_fogCoordArray->front());

    static FogCoordProc s_glFogCoordfv =
            (FogCoordProc) osg::getGLExtensionFuncPtr("glFogCoordfv","glFogCoordfvEXT");


    AttributeBinding fogCoordBinding = _fogCoordBinding;
    if (fogCoordBinding!=BIND_OFF && (!fogCoordPointer || !s_glFogCoordfv))
    {
        // swithc off if not supported or have a valid data.
        fogCoordBinding = BIND_OFF;
    }
    
    switch (fogCoordBinding)
    {
        case(BIND_OFF):
            state.disableFogCoordPointer();
            break;
        case(BIND_OVERALL):
            state.disableFogCoordPointer();
            s_glFogCoordfv(fogCoordPointer);
            break;
        case(BIND_PER_PRIMITIVE):
            state.disableFogCoordPointer();
            break;
        case(BIND_PER_VERTEX):
            state.setFogCoordPointer(GL_FLOAT,0,fogCoordPointer);
            break;
    }
    
    
    
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // draw the primitives themselves.
    //
    
    for(PrimitiveList::iterator itr=_primitives.begin();
        itr!=_primitives.end();
        ++itr)
    {
        if (normalBinding==BIND_PER_PRIMITIVE)
        {
            glNormal3fv((const GLfloat *)normalPointer++);
        }
    
        if (colorBinding==BIND_PER_PRIMITIVE)
        {
            switch(colorType)
            {
                case(Array::UByte4ArrayType):
                    glColor4ubv(reinterpret_cast<const GLubyte*>(colorPointer));
                    break;
                case(Array::Vec3ArrayType):
                    glColor3fv(reinterpret_cast<const GLfloat*>(colorPointer));
                    break;
                case(Array::Vec4ArrayType):
                    glColor4fv(reinterpret_cast<const GLfloat*>(colorPointer));
                    break;
                default:
                    break;
            }
            colorPointer += colorStride;
        }

        if (secondaryColorBinding==BIND_PER_PRIMITIVE)
        {
            switch(secondaryColorType)
            {
                case(Array::UByte4ArrayType):
                    s_glSecondaryColor3ubv(reinterpret_cast<const GLubyte*>(colorPointer));
                    break;
                case(Array::Vec3ArrayType):
                    s_glSecondaryColor3fv(reinterpret_cast<const GLfloat*>(colorPointer));
                    break;
                default:
                    break;
            }
            colorPointer += colorStride;
        }

        if (fogCoordBinding==BIND_PER_PRIMITIVE)
        {
            s_glFogCoordfv(fogCoordPointer++);
        }
    
        (*itr)->draw();
    }

}


void Geometry::accept(AttributeFunctor& af)
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

void Geometry::accept(PrimitiveFunctor& functor)
{
    if (!_vertexArray.valid() || _vertexArray->empty()) return;
    
    functor.setVertexArray(_vertexArray->size(),&(_vertexArray->front()));
    
    for(PrimitiveList::iterator itr=_primitives.begin();
        itr!=_primitives.end();
        ++itr)
    {
        (*itr)->accept(functor);
    }
    
}

// just use the base Drawable's PrimitiveFunctor based implementation.
// const bool Geometry::computeBound() const
// {
//     _bbox.init();
//     
//     const Vec3Array* coords = dynamic_cast<const Vec3Array*>(_vertexArray.get());
//     if (coords)
//     {
//         for(Vec3Array::const_iterator itr=coords->begin();
//             itr!=coords->end();
//             ++itr)
//         {
//             _bbox.expandBy(*itr);
//         }
//     }
//     _bbox_computed = true;
//     
//     return _bbox.valid();
// }

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
        const Array* array = itr->get();
        if (_vertexArray.valid())
        {
            if (array && array->getNumElements()!=_vertexArray->getNumElements()) return false;
        }
        else if (array && array->getNumElements()>0) return false;
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
