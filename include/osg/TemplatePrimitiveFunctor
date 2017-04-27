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

#ifndef OSG_TERMPLATEPRIMITIVEFUNCTOR
#define OSG_TERMPLATEPRIMITIVEFUNCTOR 1

#include <osg/PrimitiveSet>
#include <osg/Notify>

namespace osg {


/** Provides access to the primitives that compose an \c osg::Drawable.
    *  <p>Notice that \c TemplatePrimitiveFunctor is a class template, and that it inherits
    *  from its template parameter \c T. This template parameter must implement
    *  <tt>operator()(const osg::Vec3 v1, const osg::Vec3 v2, const osg::Vec3
    *  v3, bool treatVertexDataAsTemporary)</tt>,
    *  <tt>operator()(const osg::Vec3 v1, const osg::Vec3 v2, bool
    *  treatVertexDataAsTemporary)</tt>, <tt>operator()(const osg::Vec3 v1,
    *  const osg::Vec3 v2, const osg::Vec3 v3, bool treatVertexDataAsTemporary)</tt>,
    *  and <tt>operator()(const osg::Vec3 v1, const osg::Vec3 v2, const osg::Vec3 v3,
    *  const osg::Vec3 v4, bool treatVertexDataAsTemporary)</tt> which will be called
    *  for the matching primitive when the functor is applied to a \c Drawable.
    *  Parameters \c v1, \c v2, \c v3, and \c v4 are the vertices of the primitive.
    *  The last parameter, \c treatVertexDataAsTemporary, indicates whether these
    *  vertices are coming from a "real" vertex array, or from a temporary vertex array,
    *  created by the \c TemplatePrimitiveFunctor from some other geometry representation.
    *  @see \c PrimitiveFunctor for general usage hints.
    */
template<class T>
class TemplatePrimitiveFunctor : public PrimitiveFunctor, public T
{
public:

    TemplatePrimitiveFunctor()
    {
        _vertexArraySize=0;
        _vertexArrayPtr=0;
    }

    virtual ~TemplatePrimitiveFunctor() {}

    virtual void setVertexArray(unsigned int,const Vec2*)
    {
        notify(WARN)<<"Triangle Functor does not support Vec2* vertex arrays"<<std::endl;
    }

    virtual void setVertexArray(unsigned int count,const Vec3* vertices)
    {
        _vertexArraySize = count;
        _vertexArrayPtr = vertices;
    }

    virtual void setVertexArray(unsigned int,const Vec4* )
    {
        notify(WARN)<<"Triangle Functor does not support Vec4* vertex arrays"<<std::endl;
    }

    virtual void setVertexArray(unsigned int,const Vec2d*)
    {
        notify(WARN)<<"Triangle Functor does not support Vec2d* vertex arrays"<<std::endl;
    }

    virtual void setVertexArray(unsigned int,const Vec3d*)
    {
        notify(WARN)<<"Triangle Functor does not support Vec3d* vertex arrays"<<std::endl;
    }

    virtual void setVertexArray(unsigned int,const Vec4d* )
    {
        notify(WARN)<<"Triangle Functor does not support Vec4d* vertex arrays"<<std::endl;
    }


    virtual void drawArrays(GLenum mode,GLint first,GLsizei count)
    {
        if (_vertexArrayPtr==0 || count==0) return;

        switch(mode)
            {
            case(GL_TRIANGLES): {
                const Vec3* vlast = &_vertexArrayPtr[first+count];
                for(const Vec3* vptr=&_vertexArrayPtr[first];vptr<vlast;vptr+=3)
                    this->operator()(*(vptr),*(vptr+1),*(vptr+2),false);
                break;
            }
            case(GL_TRIANGLE_STRIP): {
                const Vec3* vptr = &_vertexArrayPtr[first];
                for(GLsizei i=2;i<count;++i,++vptr)
                {
                    if ((i%2)) this->operator()(*(vptr),*(vptr+2),*(vptr+1),false);
                    else       this->operator()(*(vptr),*(vptr+1),*(vptr+2),false);
                }
                break;
            }
            case(GL_QUADS): {
                const Vec3* vptr = &_vertexArrayPtr[first];
                for(GLsizei i=3;i<count;i+=4,vptr+=4)
                {
                    this->operator()(*(vptr),*(vptr+1),*(vptr+2),*(vptr+3),false);
                }
                break;
            }
            case(GL_QUAD_STRIP): {
                const Vec3* vptr = &_vertexArrayPtr[first];
                for(GLsizei i=3;i<count;i+=2,vptr+=2)
                {
                    this->operator()(*(vptr),*(vptr+1),*(vptr+3),*(vptr+2),false);
                }
                break;
            }
            case(GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
            case(GL_TRIANGLE_FAN): {
                const Vec3* vfirst = &_vertexArrayPtr[first];
                const Vec3* vptr = vfirst+1;
                for(GLsizei i=2;i<count;++i,++vptr)
                {
                    this->operator()(*(vfirst),*(vptr),*(vptr+1),false);
                }
                break;
            }
            case(GL_POINTS): {
                const Vec3* vlast = &_vertexArrayPtr[first+count];
                for(const Vec3* vptr=&_vertexArrayPtr[first];vptr<vlast;vptr+=1)
                    this->operator()(*(vptr),false);
                break;
            }
            case(GL_LINES): {
                const Vec3* vlast = &_vertexArrayPtr[first+count-1];
                for(const Vec3* vptr=&_vertexArrayPtr[first];vptr<vlast;vptr+=2)
                    this->operator()(*(vptr),*(vptr+1),false);
                break;
            }
            case(GL_LINE_STRIP): {
                const Vec3* vlast = &_vertexArrayPtr[first+count-1];
                for(const Vec3* vptr=&_vertexArrayPtr[first];vptr<vlast;vptr+=1)
                    this->operator()(*(vptr),*(vptr+1),false);
                break;
            }
            case(GL_LINE_STRIP_ADJACENCY): {
                const Vec3* vlast = &_vertexArrayPtr[first+count-2];
                for(const Vec3* vptr=&_vertexArrayPtr[first+1];vptr<vlast;vptr+=1)
                    this->operator()(*(vptr),*(vptr+1),false);
                break;
            }
            case(GL_LINE_LOOP): {
                const Vec3* vlast = &_vertexArrayPtr[first+count-1];
                for(const Vec3* vptr=&_vertexArrayPtr[first];vptr<vlast;vptr+=1)
                    this->operator()(*(vptr),*(vptr+1),false);
                this->operator()(*(vlast),_vertexArrayPtr[first],false);
                break;
            }
            default:
                break;
            }
    }

    template<class IndexType>
    void drawElementsTemplate(GLenum mode,GLsizei count,const IndexType* indices)
    {
        if (indices==0 || count==0) return;

        typedef const IndexType* IndexPointer;

        switch(mode)
            {
            case(GL_TRIANGLES): {
                IndexPointer ilast = &indices[count];
                for(IndexPointer  iptr=indices;iptr<ilast;iptr+=3)
                    this->operator()(_vertexArrayPtr[*iptr],_vertexArrayPtr[*(iptr+1)],_vertexArrayPtr[*(iptr+2)],false);
                break;
            }
            case(GL_TRIANGLE_STRIP): {
                IndexPointer iptr = indices;
                for(GLsizei i=2;i<count;++i,++iptr)
                {
                    if ((i%2)) this->operator()(_vertexArrayPtr[*(iptr)],_vertexArrayPtr[*(iptr+2)],
                                                _vertexArrayPtr[*(iptr+1)],false);
                    else       this->operator()(_vertexArrayPtr[*(iptr)],_vertexArrayPtr[*(iptr+1)],
                                                _vertexArrayPtr[*(iptr+2)],false);
                }
                break;
            }
            case(GL_QUADS): {
                IndexPointer iptr = indices;
                for(GLsizei i=3;i<count;i+=4,iptr+=4)
                {
                    this->operator()(_vertexArrayPtr[*(iptr)],_vertexArrayPtr[*(iptr+1)],
                                     _vertexArrayPtr[*(iptr+2)],_vertexArrayPtr[*(iptr+3)],
                                        false);
                }
                break;
            }
            case(GL_QUAD_STRIP): {
                IndexPointer iptr = indices;
                for(GLsizei i=3;i<count;i+=2,iptr+=2)
                {
                    this->operator()(_vertexArrayPtr[*(iptr)],_vertexArrayPtr[*(iptr+1)],
                                        _vertexArrayPtr[*(iptr+3)],_vertexArrayPtr[*(iptr+2)],
                                        false);
                }
                break;
            }
            case(GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
            case(GL_TRIANGLE_FAN): {
                IndexPointer iptr = indices;
                const Vec3& vfirst = _vertexArrayPtr[*iptr];
                ++iptr;
                for(GLsizei i=2;i<count;++i,++iptr)
                {
                    this->operator()(vfirst,_vertexArrayPtr[*(iptr)],_vertexArrayPtr[*(iptr+1)],
                                        false);
                }
                break;
            }
            case(GL_POINTS): {
                IndexPointer ilast = &indices[count];
                for(IndexPointer  iptr=indices;iptr<ilast;iptr+=1)
                    this->operator()(_vertexArrayPtr[*iptr],false);
                break;
            }
            case(GL_LINES): {
                IndexPointer ilast = &indices[count-1];
                for(IndexPointer  iptr=indices;iptr<ilast;iptr+=2)
                    this->operator()(_vertexArrayPtr[*iptr],_vertexArrayPtr[*(iptr+1)],
                                        false);
                break;
            }
            case(GL_LINE_STRIP): {
                IndexPointer ilast = &indices[count-1];
                for(IndexPointer  iptr=indices;iptr<ilast;iptr+=1)
                    this->operator()(_vertexArrayPtr[*iptr],_vertexArrayPtr[*(iptr+1)],
                                        false);
                break;
            }
            case(GL_LINE_STRIP_ADJACENCY): {
                IndexPointer ilast = &indices[count-2];
                for(IndexPointer  iptr=&indices[1];iptr<ilast;iptr+=1)
                    this->operator()(_vertexArrayPtr[*iptr],_vertexArrayPtr[*(iptr+1)],
                                        false);
                break;
            }
            case(GL_LINE_LOOP): {
                IndexPointer ilast = &indices[count-1];
                for(IndexPointer  iptr=indices;iptr<ilast;iptr+=1)
                    this->operator()(_vertexArrayPtr[*iptr],_vertexArrayPtr[*(iptr+1)],
                                        false);
                this->operator()(_vertexArrayPtr[*(ilast)],_vertexArrayPtr[indices[0]],
                                    false);
                break;
            }
            default:
                break;
            }
    }


    virtual void drawElements(GLenum mode,GLsizei count,const GLubyte* indices)
    {
        drawElementsTemplate(mode, count, indices);
    }

    virtual void drawElements(GLenum mode,GLsizei count,const GLushort* indices)
    {
        drawElementsTemplate(mode, count, indices);
    }

    virtual void drawElements(GLenum mode,GLsizei count,const GLuint* indices)
    {
        drawElementsTemplate(mode, count, indices);
    }

protected:

    unsigned int        _vertexArraySize;
    const Vec3*         _vertexArrayPtr;
};


}

#endif
