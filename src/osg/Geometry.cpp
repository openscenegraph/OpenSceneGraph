#include <osg/Geometry>

using namespace osg;

Geometry::Geometry()
{
    _normalBinding = BIND_OFF;
    _colorBinding = BIND_OFF;
}

Geometry::Geometry(const Geometry& geometry,const CopyOp& copyop):
    Drawable(geometry,copyop),
    _vertexArray(geometry._vertexArray),
    _normalBinding(geometry._normalBinding),
    _normalArray(geometry._normalArray),
    _colorBinding(geometry._colorBinding),
    _colorArray(geometry._colorArray),
    _texCoordList(geometry._texCoordList)
{
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

void Geometry::drawImmediateMode(State& /*state*/)
{
    if (!_vertexArray.valid()) return;
    
    // set up the vertex arrays.
    glEnableClientState( GL_VERTEX_ARRAY );
    glVertexPointer(3,GL_FLOAT,0,_vertexArray->dataPointer());
    
    
    // set up texture coordinates.
    for(unsigned int i=0;i<_texCoordList.size();++i)
    {
        Array* array = _texCoordList[i].get();
        //glClientActiveTextureARB(GL_TEXTURE0_ARB+i);
        if (array)
        {
            glEnableClientState( GL_TEXTURE_COORD_ARRAY );
            glTexCoordPointer(array->dataSize(),array->dataType(),0,array->dataPointer());
        }
        else
        {
            glDisableClientState( GL_TEXTURE_COORD_ARRAY );
        }
    }
    
    
    // set up normals.
    Vec3* normalPointer = 0;
    if (_normalArray.valid() && !_normalArray->empty()) normalPointer = &(_normalArray->front());

    switch (_normalBinding)
    {
        case(BIND_OFF):
            glDisableClientState( GL_NORMAL_ARRAY );
            break;
        case(BIND_OVERALL):
            glDisableClientState( GL_NORMAL_ARRAY );
            if (normalPointer) glNormal3fv(reinterpret_cast<const GLfloat*>(normalPointer));
            break;
        case(BIND_PER_PRIMITIVE):
            glDisableClientState( GL_NORMAL_ARRAY );
            break;
        case(BIND_PER_VERTEX):
            glEnableClientState( GL_NORMAL_ARRAY );
            if (normalPointer) glNormalPointer(GL_FLOAT,0,normalPointer);
            break;
    }


    // set up colors, complicated by the fact that the color array
    // might be bound in 4 different ways, and be represented as 3 different data types -
    // Vec3, Vec4 or UByte4 Arrays.
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
                colorPointer = reinterpret_cast<const unsigned char*>(_colorArray->dataPointer());
                colorStride = 4;
                break;
            }
            case(Array::Vec3ArrayType):
            {
                colorPointer = reinterpret_cast<const unsigned char*>(_colorArray->dataPointer());
                colorStride = 12;
                break;
            }
            case(Array::Vec4ArrayType):
            {
                colorPointer = reinterpret_cast<const unsigned char*>(_colorArray->dataPointer());
                colorStride = 16;
                break;
            }
        }
    }

    switch (_colorBinding)
    {
        case(BIND_OFF):
            glDisableClientState( GL_COLOR_ARRAY );
            break;
        case(BIND_OVERALL):
            glDisableClientState( GL_COLOR_ARRAY );
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
                }
            }
            break;
        case(BIND_PER_PRIMITIVE):
            glDisableClientState( GL_COLOR_ARRAY );
            break;
        case(BIND_PER_VERTEX):
            glEnableClientState( GL_COLOR_ARRAY );
            if (colorPointer) glColorPointer(_colorArray->dataSize(),_colorArray->dataType(),0,colorPointer);
    }


    // draw the primitives themselves.
    for(PrimitiveList::iterator itr=_primitives.begin();
        itr!=_primitives.end();
        ++itr)
    {
        if (_normalBinding==BIND_PER_PRIMITIVE)
        {
            glNormal3fv((const GLfloat *)normalPointer++);
        }
    
        if (_colorBinding==BIND_PER_PRIMITIVE)
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
            }
            colorPointer += colorStride;
        }

        (*itr)->draw();
    }

}

/** Statistics collection for each drawable- 26.09.01
 */
bool Geometry::getStats(Statistics &)
{
    return false;
}

Drawable::AttributeBitMask Geometry::suppportsAttributeOperation() const
{
    // we do support coords,normals,texcoords and colors so return true.
    return COORDS | NORMALS | COLORS | TEXTURE_COORDS;
}


/** return the attributes successully applied in applyAttributeUpdate.*/
Drawable::AttributeBitMask Geometry::applyAttributeOperation(AttributeFunctor& auf)
{
    AttributeBitMask amb = auf.getAttributeBitMask();
    AttributeBitMask ramb = 0;
    
    if ((amb & COORDS) && _vertexArray.valid() && !_vertexArray->empty())
    {
        if (auf.apply(COORDS,&(*(_vertexArray->begin())),&(*(_vertexArray->end())))) ramb = COORDS;
    }
    
    if ((amb & NORMALS) && _normalArray.valid() && !_normalArray->empty())
    {
        if (auf.apply(NORMALS,&(*(_normalArray->begin())),&(*(_normalArray->end())))) ramb = NORMALS;
    }
    
    // colors and texture coords to implement...
    
    return ramb;
}

void Geometry::applyPrimitiveOperation(PrimitiveFunctor& functor)
{
    if (!_vertexArray.valid() || _vertexArray->empty()) return;
    
    functor.setVertexArray(_vertexArray->size(),&(_vertexArray->front()));
    
    for(PrimitiveList::iterator itr=_primitives.begin();
        itr!=_primitives.end();
        ++itr)
    {
        (*itr)->applyPrimitiveOperation(functor);
    }
    
}


const bool Geometry::computeBound() const
{
    _bbox.init();
    
    const Vec3Array* coords = dynamic_cast<const Vec3Array*>(_vertexArray.get());
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

