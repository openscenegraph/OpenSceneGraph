/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2009 Robert Osfield
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

#ifndef OSGVOLUME_MULTIPASSTECHNIQUE
#define OSGVOLUME_MULTIPASSTECHNIQUE 1

#include <osgVolume/VolumeTechnique>
#include <osg/MatrixTransform>
#include <osg/Texture2D>

namespace osgVolume {



class OSGVOLUME_EXPORT MultipassTechnique : public VolumeTechnique
{
    public:

        MultipassTechnique();

        MultipassTechnique(const MultipassTechnique&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(osgVolume, MultipassTechnique);

        virtual void init();

        virtual void update(osgUtil::UpdateVisitor* nv);

        virtual void backfaceSubgraphCullTraversal(osgUtil::CullVisitor* cv);

        virtual void cull(osgUtil::CullVisitor* cv);

        /** Clean scene graph from any terrain technique specific nodes.*/
        virtual void cleanSceneGraph();

        /** Traverse the terrain subgraph.*/
        virtual void traverse(osg::NodeVisitor& nv);

        enum RenderingMode
        {
            CUBE,
            HULL,
            CUBE_AND_HULL
        };

        RenderingMode computeRenderingMode();

        /** Container for render to texture objects used when doing multi-pass volume rendering techniques.*/
        struct OSGVOLUME_EXPORT MultipassTileData : public TileData
        {
            MultipassTileData(osgUtil::CullVisitor* cv, MultipassTechnique* mpt);

            virtual void update(osgUtil::CullVisitor* cv);

            void setUp(osg::ref_ptr<osg::Camera>& camera, osg::ref_ptr<osg::Texture2D>& texture2D, int width, int height);


            osg::observer_ptr<MultipassTechnique>       multipassTechnique;
            RenderingMode                               currentRenderingMode;

            osg::ref_ptr<osg::Texture2D>                frontFaceDepthTexture;
            osg::ref_ptr<osg::Camera>                   frontFaceRttCamera;

            osg::ref_ptr<osg::Texture2D>                backFaceDepthTexture;
            osg::ref_ptr<osg::Camera>                   backFaceRttCamera;

            osg::ref_ptr<osg::Uniform>                  eyeToTileUniform;
            osg::ref_ptr<osg::Uniform>                  tileToImageUniform;
        };

        /** Called from VolumeScene to create the TileData container when a multi-pass technique is being used.
         *  The TileData container caches any render to texture objects that are required. */
        virtual TileData* createTileData(osgUtil::CullVisitor* cv) { return new MultipassTileData(cv, this); }

    protected:

        virtual ~MultipassTechnique();

        osg::ref_ptr<osg::MatrixTransform> _transform;

        typedef std::map<osgUtil::CullVisitor::Identifier*, osg::Matrix> ModelViewMatrixMap;

        OpenThreads::Mutex _mutex;
        ModelViewMatrixMap _modelViewMatrixMap;

        osg::ref_ptr<osg::StateSet> _whenMovingStateSet;
        osg::ref_ptr<osg::StateSet> _volumeRenderStateSet;

        osg::StateSet* createStateSet(osg::StateSet* statesetPrototype, osg::Program* programPrototype, osg::Shader* shaderToAdd1=0, osg::Shader* shaderToAdd2=0);

        enum ShaderMask
        {
            CUBE_SHADERS = 1,
            HULL_SHADERS = 2,
            CUBE_AND_HULL_SHADERS = 4,
            STANDARD_SHADERS = 8,
            LIT_SHADERS = 16,
            ISO_SHADERS = 32,
            MIP_SHADERS = 64,
            TF_SHADERS = 128
        };

        typedef std::map<int, osg::ref_ptr<osg::StateSet> > StateSetMap;
        StateSetMap _stateSetMap;

        osg::ref_ptr<osg::StateSet> _frontFaceStateSet;
};

}

#endif
