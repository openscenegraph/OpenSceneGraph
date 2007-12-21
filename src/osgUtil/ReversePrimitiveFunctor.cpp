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

#include <osgUtil/ReversePrimitiveFunctor>


template <typename Type>
osg::PrimitiveSet * drawElementsTemplate(GLenum mode,GLsizei count, const typename Type::value_type* indices)
{
     if (indices==0 || count==0) return NULL;
     
     Type * dePtr = new Type(mode);
     Type & de = *dePtr;
     de.reserve(count);
     
     typedef const typename Type::value_type* IndexPointer;

     switch(mode)
     {
         case(GL_TRIANGLES):
         {
             IndexPointer ilast = &indices[count];
             
             for (IndexPointer iptr=indices; iptr<ilast; iptr+=3)
             {
                 de.push_back(*(iptr));
                 de.push_back(*(iptr+2));
                 de.push_back(*(iptr+1));
             }
             break;
         }
         case (GL_QUADS):
         {
             
             IndexPointer ilast = &indices[count - 3];
             for (IndexPointer iptr = indices; iptr<ilast; iptr+=4)
             {
                 de.push_back(*(iptr));
                 de.push_back(*(iptr+3));
                 de.push_back(*(iptr+2));
                 de.push_back(*(iptr+1));
             }
             break;
         }
         case (GL_TRIANGLE_STRIP):
         case (GL_QUAD_STRIP):
         {
             IndexPointer ilast = &indices[count];
             for (IndexPointer iptr = indices; iptr<ilast; iptr+=2)
             {
                 de.push_back(*(iptr+1));
                 de.push_back(*(iptr));
             }
             break;
         }
         case (GL_TRIANGLE_FAN):
         {
             de.push_back(*indices);
          
             IndexPointer iptr = indices + 1;
             IndexPointer ilast = &indices[count];
             de.resize(count);
             std::reverse_copy(iptr, ilast, de.begin() + 1);
             
             break;
         }
         case (GL_POLYGON):
         case (GL_POINTS):
         case (GL_LINES):
         case (GL_LINE_STRIP):
         case (GL_LINE_LOOP):
         {
             IndexPointer iptr = indices;
             IndexPointer ilast = &indices[count];
             de.resize(count);
             std::reverse_copy(iptr, ilast, de.begin());
             
             break;
         }
         default:
             break;
     }
     
     return &de;
}

namespace osgUtil {

void ReversePrimitiveFunctor::drawElements(GLenum mode,GLsizei count,const GLubyte* indices)
{
    _reversedPrimitiveSet = drawElementsTemplate<osg::DrawElementsUByte>(mode, count, indices);
}
void ReversePrimitiveFunctor::drawElements(GLenum mode,GLsizei count,const GLushort* indices)
{
    _reversedPrimitiveSet = drawElementsTemplate<osg::DrawElementsUShort>(mode, count, indices);
}
void ReversePrimitiveFunctor::drawElements(GLenum mode,GLsizei count,const GLuint* indices)
{
    _reversedPrimitiveSet = drawElementsTemplate<osg::DrawElementsUInt>(mode, count, indices);
}

} // end osgUtil namespace
