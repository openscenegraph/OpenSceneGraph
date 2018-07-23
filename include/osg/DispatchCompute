/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
 * Copyright (C) 2017 Julien Valentin
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

#ifndef OSG_DispatchCompute
#define OSG_DispatchCompute 1

#include <osg/Export>

#include <osg/Geometry>

namespace osg
{

/** Wrapper around glDispatchCompute.*/
class OSG_EXPORT DispatchCompute : public osg::Drawable
{
    public:
        DispatchCompute(GLint numGroupsX=0, GLint numGroupsY=0, GLint numGroupsZ=0):
            Drawable(),
            _numGroupsX(numGroupsX),
            _numGroupsY(numGroupsY),
            _numGroupsZ(numGroupsZ)
        {}

        DispatchCompute(const DispatchCompute&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Node(osg, DispatchCompute);

        virtual void compileGLObjects(RenderInfo&) const {}

        virtual VertexArrayState* createVertexArrayStateImplememtation(RenderInfo&) const { return 0; }

        virtual void drawImplementation(RenderInfo& renderInfo) const;

        /** Set compute shader work groups */
        void setComputeGroups( GLint numGroupsX, GLint numGroupsY, GLint numGroupsZ ) { _numGroupsX=numGroupsX; _numGroupsY=numGroupsY; _numGroupsZ=numGroupsZ; }

        /** Get compute shader work groups */
        void getComputeGroups( GLint& numGroupsX, GLint& numGroupsY, GLint& numGroupsZ ) const{ numGroupsX=_numGroupsX; numGroupsY=_numGroupsY; numGroupsZ=_numGroupsZ; }

    protected:
        GLint _numGroupsX, _numGroupsY, _numGroupsZ;

};

}
#endif

