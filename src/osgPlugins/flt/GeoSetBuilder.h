#ifndef __FLT_GEOSETBUILDER_H
#define __FLT_GEOSETBUILDER_H

#include <osg/ref_ptr>
#include <osg/Referenced>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/GeoSet>
#include <osg/Material>
#include <osg/StateSet>

#include <map>
#include <vector>


namespace flt {

class Record;
class TmpGeoSet;


////////////////////////////////////////////////////////////////////
//
//                       TmpGeoSet
//
////////////////////////////////////////////////////////////////////


/** TmpGeoSet - Temporary GeoSet with dynamic vertex array.
  * Problem: osg::GeoSet use C arrays (static size) for coordinates,
  * normals, colors and texture coordinates.
  */ 
class TmpGeoSet : public osg::Referenced
{
    public:

        TmpGeoSet(FltFile* pFltFile);
        virtual ~TmpGeoSet() {};

        void addVertex(Record* vertexRec) { _vertexRecList.push_back(vertexRec); }
        void addPrimLen(int len) { _primLenList.push_back(len); }

        // Append vertices from other TmpGeoSet
        void addVertices(TmpGeoSet* source)
        {
            _vertexRecList.insert(_vertexRecList.end(),
                source->_vertexRecList.begin(), source->_vertexRecList.end());
            _primLenList.insert(_primLenList.end(),
                source->_primLenList.begin(), source->_primLenList.end());
        }

        inline const int numberOfVertices() const { return _vertexRecList.size(); }
        inline osg::GeoSet* getGeoSet() { return _geoSet.get(); }
        inline const osg::GeoSet* getGeoSet() const { return _geoSet.get(); }

        // Create complete osg::GeoSet.
        osg::GeoSet* createOsgGeoSet();

    private:

        typedef std::vector<osg::ref_ptr<Record> >  VertexRecList;
        typedef std::vector<int>                    PrimLenList;

        void setVertex(osg::GeoSet* gset, int index, Record* vertex);

        osg::ref_ptr<osg::GeoSet>   _geoSet;
        PrimLenList                 _primLenList;
        VertexRecList               _vertexRecList;

        osg::ref_ptr<ColorPool>     _colorPool;
        osg::ref_ptr<TexturePool>   _texturePool;
        osg::ref_ptr<MaterialPool>  _materialPool;

};




////////////////////////////////////////////////////////////////////
//
//                       GeoSetBuilder
//
////////////////////////////////////////////////////////////////////

/** GeoSetBuilder - Contains a list of TmpGeoSets to be converted to osg::Geode.
  * 
  */

class GeoSetBuilder
{
    public:

        GeoSetBuilder(FltFile* pFltFile);
        virtual ~GeoSetBuilder() {}

        void addVertex(Record* vertex) { _tmpGeoSet.get()->addVertex(vertex); }
        void addPrimLen(int len) { _tmpGeoSet.get()->addPrimLen(len); }

        bool addPrimitive();
        osg::Geode* createOsgGeoSets(osg::Geode* geode=NULL);

        inline osg::GeoSet* getGeoSet() { return _tmpGeoSet.get()->getGeoSet(); }
        inline const osg::GeoSet* getGeoSet() const { return _tmpGeoSet.get()->getGeoSet(); }
        const int numberOfVertices() const { return _tmpGeoSet.get()->numberOfVertices(); }

    protected:

        void initPrimData();
        TmpGeoSet* findMatchingGeoSet();
        osg::GeoSet::PrimitiveType findPrimType(const int nVertices);

    private:

        osg::ref_ptr<FltFile>       _pFltFile;
        osg::ref_ptr<TmpGeoSet>     _tmpGeoSet;

        typedef std::vector<osg::ref_ptr<TmpGeoSet> > TmpGeoSetList;
        TmpGeoSetList               _tmpGeoSetList;
};


}; // end of namespace flt


#endif

