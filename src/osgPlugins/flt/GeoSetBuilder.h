#ifndef __FLT_GEOSETBUILDER_H
#define __FLT_GEOSETBUILDER_H

#include <osg/ref_ptr>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/GeoSet>
#include <osg/Geode>
#include <osg/Material>
#include <osg/StateSet>

#include <map>
#include <vector>


namespace flt {

class Record;
class TmpGeoSet;



////////////////////////////////////////////////////////////////////
//
//                       DynGeoSet
//
////////////////////////////////////////////////////////////////////
#if 0
#  define COMPARE_DynGeoSet_Parameter(parameter) \
        if (parameter<rhs.parameter) return -1; \
        if (rhs.parameter<parameter) return 1;
#else
#  define COMPARE_DynGeoSet_Parameter(parameter) \
        if (parameter != rhs.parameter) return -1;
#endif

/** DynGeoSet - Dynamic GeoSet.
  * Problem: osg::GeoSet use C arrays (static size) for coordinates,
  * normals, colors and texture coordinates.
  */ 
class DynGeoSet : public osg::GeoSet
{
    public:

        DynGeoSet();
        
        virtual osg::Object* clone() const { return new DynGeoSet(); }
        virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const DynGeoSet*>(obj)!=NULL; }
        virtual const char* className() const { return "GeoSet"; }

        int compare(const DynGeoSet& rhs) const
        {
            COMPARE_DynGeoSet_Parameter(_primtype)
            COMPARE_DynGeoSet_Parameter(_color_binding)
            COMPARE_DynGeoSet_Parameter(_normal_binding)
            COMPARE_DynGeoSet_Parameter(_texture_binding)

            if ((_color_binding == osg::GeoSet::BIND_OVERALL)
            &&  (_colorList.size() >= 1) &&  (rhs._colorList.size() >= 1)
            &&  (_colorList[0] != rhs._colorList[0]))
                return -1;

            return getStateSet()->compare(*rhs.getStateSet(), true);
        }
        
        bool operator <  (const DynGeoSet& rhs) const { return compare(rhs)<0; }
        bool operator == (const DynGeoSet& rhs) const { return compare(rhs)==0; }
        bool operator != (const DynGeoSet& rhs) const { return compare(rhs)!=0; }

        inline void addPrimLen(const int len)           { _primLenList.push_back(len); }
        inline void addCoord(const osg::Vec3& coord)    { _coordList.push_back(coord); }
        inline void addNormal(const osg::Vec3& normal)  { _normalList.push_back(normal); }
        inline void addColor(const osg::Vec4& color)    { _colorList.push_back(color); }
        inline void addTCoord(const osg::Vec2& tcoord)  { _tcoordList.push_back(tcoord); }

        void append(DynGeoSet* source);
        void setBinding();
        bool setLists();

        inline const int primLenListSize() const { return _primLenList.size(); }
        inline const int coordListSize() const { return _coordList.size(); }
        inline const int normalListSize() const { return _normalList.size(); }
        inline const int colorListSize() const { return _colorList.size(); }
        inline const int tcoordListSize() const { return _tcoordList.size(); }

    private:

        typedef std::vector<int>        PrimLenList;
        typedef std::vector<osg::Vec3>  CoordList;
        typedef std::vector<osg::Vec3>  NormalList;
        typedef std::vector<osg::Vec4>  ColorList;
        typedef std::vector<osg::Vec2>  TcoordList;

        PrimLenList     _primLenList;
        CoordList       _coordList;
        NormalList      _normalList;
        ColorList       _colorList;
        TcoordList      _tcoordList;
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
        GeoSetBuilder(osg::Geode* geode = NULL);
        virtual ~GeoSetBuilder() {}

        bool addPrimitive( bool dontMerge = false);
        osg::Geode* createOsgGeoSets(osg::Geode* geode = NULL);

        inline DynGeoSet* getDynGeoSet() { return _dynGeoSet.get(); }
        inline const DynGeoSet* getDynGeoSet() const { return _dynGeoSet.get(); }
        inline bool empty()    { return _dynGeoSetList.empty(); } ;

    protected:

        void initPrimData();
        DynGeoSet* findMatchingGeoSet();
        osg::GeoSet::PrimitiveType findPrimType(const int nVertices);

    private:

        osg::ref_ptr<osg::Geode>    _geode;
        osg::ref_ptr<DynGeoSet>     _dynGeoSet;

        typedef std::vector<osg::ref_ptr<DynGeoSet> > DynGeoSetList;
        DynGeoSetList               _dynGeoSetList;
};


}; // end of namespace flt


#endif

