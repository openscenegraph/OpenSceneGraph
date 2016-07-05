/*  -*-c++-*- 
 *  Copyright (C) 2010 Cedric Pinson <cedric.pinson@plopbyte.net>
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
 * 
 * Authors:
 *         Cedric Pinson <cedric.pinson@plopbyte.net>
 */

#include <osg/TriangleFunctor>


struct DrawArrayLenghtAdaptor
{
    unsigned int _normalArraySize;
    const Vec3* _normalArrayPtr;

    unsigned int _colorArraySize;
    const Vec4* _colorArrayPtr;

    std::vector<std::pair<unsigned int, const osg::Vec2*> > _uvs;

    virtual void setNormalArray(unsigned int count,const Vec3* vertices) {
        _normalArraySize = count;
        _normalArrayPtr = vertices;
    }

    virtual void setNormalArray(unsigned int count,const Vec4* colors) {
        _colorArraySize = count;
        _colorArrayPtr = colors;
    }

    virtual void setTexCoordArray(int unit, unsigned int count,const Vec2* uvs) {
        if (_uvs.size() <= unit)
            _uvs.resize(unit+1);
        _uvs[i].first = count;
        _uvs[i].second = uvs;
    }
};


struct AdaptDraw : public osg::TriangleFunctor<DrawArrayLenghtAdaptor>
{

    virtual void drawArrays(GLenum mode,GLint first,GLsizei count)
    {
        if (_vertexArrayPtr==0 || count==0) return;

        switch(mode)
        {
            case(GL_TRIANGLES):
            {
                unsigned int last = first+count;
                for(unsigned int current = first; current<last ; current+=3) {
                    this->operator()(current, current+1, current+2,_treatVertexDataAsTemporary);
                    
                }
                break;
            }
            case(GL_TRIANGLE_STRIP):
            {
                unsigned int current = first;
                for(GLsizei i=2;i<count;++i, current+=1)
                {
                    if ((i%2)) this->operator()(current, current+2, current+1,_treatVertexDataAsTemporary);
                    else       this->operator()(current, current+1, current+2,_treatVertexDataAsTemporary);
                }
                break;
            }
            case(GL_QUADS):
            {
                unsigned int current = first;
                for(GLsizei i=3;i<count;i+=4,current+=4)
                {
                    this->operator()(current, current+1, current+2,_treatVertexDataAsTemporary);
                    this->operator()(current, current+2, current+3,_treatVertexDataAsTemporary);
                }
                break;
            }
            case(GL_QUAD_STRIP):
            {
                unsigned int current = first;
                for(GLsizei i=3;i<count;i+=2,current+=2)
                {
                    this->operator()(current, current+1, current+2,_treatVertexDataAsTemporary);
                    this->operator()(current+1, current+3, current+2,_treatVertexDataAsTemporary);
                }
                break;
            }
            case(GL_POLYGON): // treat polygons as GL_TRIANGLE_FAN
            case(GL_TRIANGLE_FAN):
            {
                unsigned int current = first + 1;
                for(GLsizei i=2;i<count;++i,++current)
                {
                    this->operator()(first),current, current+1,_treatVertexDataAsTemporary);
                }
                break;
            }
            case(GL_POINTS):
            case(GL_LINES):
            case(GL_LINE_STRIP):
            case(GL_LINE_LOOP):
            default:
                // can't be converted into to triangles.
                break;
        }
    }    
};
