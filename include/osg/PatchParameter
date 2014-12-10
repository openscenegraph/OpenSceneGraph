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

#ifndef OSG_PATCHPARAMETER
#define OSG_PATCHPARAMETER 1

#include <osg/Vec2>
#include <osg/Vec4>
#include <osg/StateAttribute>

namespace osg {

/** Class which encapsulates glPatchParameter(..).
*/
class OSG_EXPORT PatchParameter : public StateAttribute
{
    public :

        PatchParameter(GLint vertices=3);

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        PatchParameter(const PatchParameter& rhs,const CopyOp& copyop=CopyOp::SHALLOW_COPY):
            StateAttribute(rhs,copyop),
            _vertices(rhs._vertices),
            _patchDefaultInnerLevel(rhs._patchDefaultInnerLevel),
            _patchDefaultOuterLevel(rhs._patchDefaultOuterLevel) {}


        META_StateAttribute(osg, PatchParameter, PATCH_PARAMETER);

        /** return -1 if *this < *rhs, 0 if *this==*rhs, 1 if *this>*rhs.*/
        virtual int compare(const StateAttribute& sa) const
        {
            // check the types are equal and then create the rhs variable
            // used by the COMPARE_StateAttribute_Parameter macros below.
            COMPARE_StateAttribute_Types(PatchParameter,sa)

            // compare each parameter in turn against the rhs.
            COMPARE_StateAttribute_Parameter(_vertices)
            COMPARE_StateAttribute_Parameter(_patchDefaultInnerLevel)
            COMPARE_StateAttribute_Parameter(_patchDefaultOuterLevel)

            return 0; // passed all the above comparison macros, must be equal.
        }

        /** Set GL_PATCH_VERTICES parameter.*/
        void setVertices(GLint vertices) { _vertices = vertices; }

        /** Get GL_PATCH_VERTICES parameter.*/
        GLint getVertices() const { return _vertices; }

        /** Set GL_PATCH_DEFAULT_INNER_LEVEL parameter.*/
        void setPatchDefaultInnerLevel(const osg::Vec2& level) { _patchDefaultInnerLevel = level; }

        /** Get GL_PATCH_DEFAULT_INNER_LEVEL parameter.*/
        const osg::Vec2& getPatchDefaultInnerLevel() const { return _patchDefaultInnerLevel; }

        /** Set GL_PATCH_DEFAULT_OUTER_LEVEL parameter.*/
        void setPatchDefaultOuterLevel(const osg::Vec4& level) { _patchDefaultOuterLevel = level; }

        /** Get GL_PATCH_DEFAULT_INNER_LEVEL parameter.*/
        const osg::Vec4& getPatchDefaultOuterLevel() const { return _patchDefaultOuterLevel; }

        virtual void apply(State& state) const;

    protected:

        virtual ~PatchParameter();

        GLint           _vertices;
        osg::Vec2       _patchDefaultInnerLevel;
        osg::Vec4       _patchDefaultOuterLevel;
};

}

#endif
