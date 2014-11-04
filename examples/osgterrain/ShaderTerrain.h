#ifndef OSGTERRAIN_SHADERTERRAIN
#define OSGTERRAIN_SHADERTERRAIN

#include <OpenThreads/Mutex>
#include <osg/MatrixTransform>
#include <osg/Program>
#include <osgTerrain/GeometryTechnique>

namespace osgTerrain
{

extern const osgTerrain::Locator* computeMasterLocator(const osgTerrain::TerrainTile* tile);

class GeometryPool : public osg::Referenced
{
    public:
        GeometryPool() {}

        struct GeometryKey
        {
            GeometryKey(): sx(0.0), sy(0.0), y(0.0), nx(0), ny(0) {}

            bool operator < (const GeometryKey& rhs) const
            {
                if (sx<rhs.sx) return true;
                if (sx>rhs.sx) return false;

                if (sx<rhs.sx) return true;
                if (sx>rhs.sx) return false;

                if (y<rhs.y) return true;
                if (y>rhs.y) return false;

                if (nx<rhs.nx) return true;
                if (nx>rhs.nx) return false;

                return (ny<rhs.ny);
            }

            double sx;
            double sy;
            double y;

            int nx;
            int ny;
        };

        typedef std::map< GeometryKey, osg::ref_ptr<osg::Geometry> >  GeometryMap;

        bool createKeyForTile(TerrainTile* tile, GeometryKey& key);

        enum LayerType
        {
            HEIGHTFIELD_LAYER,
            COLOR_LAYER,
            CONTOUR_LAYER
        };

        typedef std::vector<LayerType> LayerTypes;
        typedef std::map<LayerTypes, osg::ref_ptr<osg::Program> > ProgramMap;

        osg::ref_ptr<osg::Program> getOrCreateProgram(LayerTypes& layerTypes);

        osg::ref_ptr<osg::Geometry> getOrCreateGeometry(osgTerrain::TerrainTile* tile);

        osg::ref_ptr<osg::MatrixTransform> getTileSubgraph(osgTerrain::TerrainTile* tile);

        void applyLayers(osgTerrain::TerrainTile* tile, osg::StateSet* stateset);

    protected:
        virtual ~GeometryPool();

        OpenThreads::Mutex      _geometryMapMutex;
        GeometryMap             _geometryMap;

        OpenThreads::Mutex      _programMapMutex;
        ProgramMap              _programMap;
};

class HeightFieldDrawable : public osg::Drawable
{
    public:
        HeightFieldDrawable();

        HeightFieldDrawable(const HeightFieldDrawable&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Node(osgTerrain, HeightFieldDrawable);

        void setHeightField(osg::HeightField* hf) { _heightField = hf; }
        osg::HeightField* getHeightField() { return _heightField.get(); }
        const osg::HeightField* getHeightField() const { return _heightField.get(); }

        void setGeometry(osg::Geometry* geom) { _geometry = geom; }
        osg::Geometry* getGeometry() { return _geometry.get(); }
        const osg::Geometry* getGeometry() const { return _geometry.get(); }

        virtual void drawImplementation(osg::RenderInfo& renderInfo) const;
        virtual void compileGLObjects(osg::RenderInfo& renderInfo) const;
        virtual void resizeGLObjectBuffers(unsigned int maxSize);
        virtual void releaseGLObjects(osg::State* state=0) const;


        virtual bool supports(const osg::Drawable::AttributeFunctor&) const { return true; }
        virtual void accept(osg::Drawable::AttributeFunctor&);

        virtual bool supports(const osg::Drawable::ConstAttributeFunctor&) const { return true; }
        virtual void accept(osg::Drawable::ConstAttributeFunctor&) const;

        virtual bool supports(const osg::PrimitiveFunctor&) const { return true; }
        virtual void accept(osg::PrimitiveFunctor&) const;

        virtual bool supports(const osg::PrimitiveIndexFunctor&) const { return true; }
        virtual void accept(osg::PrimitiveIndexFunctor&) const;



protected:

        osg::ref_ptr<osg::HeightField>  _heightField;
        osg::ref_ptr<osg::Geometry>     _geometry;

};

class ShaderTerrain : public osgTerrain::TerrainTechnique
{
    public:

        ShaderTerrain();

        ShaderTerrain(const ShaderTerrain&,const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(osgTerrain, ShaderTerrain);

        virtual void init(int dirtyMask, bool assumeMultiThreaded);
        virtual void update(osgUtil::UpdateVisitor* uv);
        virtual void cull(osgUtil::CullVisitor* cv);
        virtual void traverse(osg::NodeVisitor& nv);
        virtual void cleanSceneGraph();
        virtual void releaseGLObjects(osg::State* state) const;

    protected:

        virtual ~ShaderTerrain();

        osg::ref_ptr<GeometryPool> _geometryPool;

        mutable OpenThreads::Mutex              _traversalMutex;

        mutable OpenThreads::Mutex              _transformMutex;
        osg::ref_ptr<osg::MatrixTransform>      _transform;

        OpenThreads::Atomic                     _currentTraversalCount;

};

}

#endif
