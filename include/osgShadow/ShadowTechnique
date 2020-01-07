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

#ifndef OSGSHADOW_SHADOWEDTECHNIQUE
#define OSGSHADOW_SHADOWEDTECHNIQUE 1

#include <osg/buffered_value>
#include <osg/Camera>
#include <osg/Texture2D>
#include <osg/TexGenNode>
#include <osgUtil/CullVisitor>

#include <osgShadow/Export>

namespace osgShadow {

// forward declare ShadowedScene
class ShadowedScene;

/** ShadowTechnique is the base class for different shadow implementations.*/
class OSGSHADOW_EXPORT ShadowTechnique : public osg::Object
{
    public :
        ShadowTechnique();

        ShadowTechnique(const ShadowTechnique& es, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const ShadowTechnique*>(obj)!=NULL; } \
        virtual const char* libraryName() const { return "osgShadow"; }\
        virtual const char* className() const { return "ShadowTechnique"; }

        virtual void setShadowedScene(ShadowedScene* ss);

        ShadowedScene* getShadowedScene() { return _shadowedScene; }

        const ShadowedScene* getShadowedScene() const { return _shadowedScene; }

        /** initialize the ShadowedScene and local cached data structures.*/
        virtual void init();

        /** run the update traversal of the ShadowedScene and update any local cached data structures.*/
        virtual void update(osg::NodeVisitor& nv);

        /** run the cull traversal of the ShadowedScene and set up the rendering for this ShadowTechnique.*/
        virtual void cull(osgUtil::CullVisitor& cv);

        /** Clean scene graph from any shadow technique specific nodes, state and drawables.*/
        virtual void cleanSceneGraph();

        virtual void traverse(osg::NodeVisitor& nv);

        /** Dirty so that cached data structures are updated.*/
        virtual void dirty() { _dirty = true; }

        /** Resize any per context GLObject buffers to specified size. */
        virtual void resizeGLObjectBuffers(unsigned int maxSize) = 0;

        /** If State is non-zero, this function releases any associated OpenGL objects for
           * the specified graphics context. Otherwise, releases OpenGL objects
           * for all graphics contexts. */
        virtual void releaseGLObjects(osg::State* = 0) const = 0;

protected :

        class OSGSHADOW_EXPORT CameraCullCallback : public osg::NodeCallback
        {
            public:

                CameraCullCallback(ShadowTechnique* st);

                virtual void operator()(osg::Node*, osg::NodeVisitor* nv);

            protected:

                ShadowTechnique* _shadowTechnique;
        };

        osg::Vec3 computeOrthogonalVector(const osg::Vec3& direction) const;

        virtual ~ShadowTechnique();

        friend class ShadowedScene;

        ShadowedScene*  _shadowedScene;
        bool            _dirty;

};

}

#endif
