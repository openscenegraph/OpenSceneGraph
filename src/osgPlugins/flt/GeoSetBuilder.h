#ifndef __FLT_GEOSETBUILDER_H
#define __FLT_GEOSETBUILDER_H

// Added DynGeoSet::setDetailTextureAttrData that is used to store texture Attributes
// Julian Ortiz, June 18th 2003.

#include <osg/ref_ptr>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Material>
#include <osg/StateSet>

#include "AttrData.h"

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
#if 1
#  define COMPARE_DynGeoSet_Parameter(parameter) \
        if (parameter<rhs.parameter) return -1; \
        if (rhs.parameter<parameter) return 1;
#else
#  define COMPARE_DynGeoSet_Parameter(parameter) \
        if (parameter != rhs.parameter) return -1;
#endif

/** DynGeoSet - Dynamic GeoSet.
  */ 
class DynGeoSet : public osg::Referenced
{
    public:

        DynGeoSet();

        int compare(const DynGeoSet& rhs) const
        {
            COMPARE_DynGeoSet_Parameter(_color_binding)
            COMPARE_DynGeoSet_Parameter(_normal_binding)
            COMPARE_DynGeoSet_Parameter(_texture_binding)

            if (_color_binding == osg::Geometry::BIND_OVERALL)
            {
                if ((_colorList.size() >= 1) &&  (rhs._colorList.size() >= 1))
                {
                    if (_colorList[0]<rhs._colorList[0]) return -1;
                    if (rhs._colorList[0]<_colorList[0]) return 1;
                }
            }
            
            int result=getStateSet()->compare(*rhs.getStateSet(), true);
            if (result!=0) return result;
            
            COMPARE_DynGeoSet_Parameter(_primtype);
            return 0;
        }
        
        int compatible(const DynGeoSet& rhs) const
        {
        
            COMPARE_DynGeoSet_Parameter(_color_binding)
            COMPARE_DynGeoSet_Parameter(_texture_binding)

            int result=getStateSet()->compare(*rhs.getStateSet(), true);
            if (result!=0) return result;

            COMPARE_DynGeoSet_Parameter(_normal_binding)

            return 0;


            if (_color_binding == osg::Geometry::BIND_OVERALL)
            {
                if ((_colorList.size() >= 1) &&  (rhs._colorList.size() >= 1))
                {
                    if (_colorList[0]<rhs._colorList[0]) return -1;
                    if (rhs._colorList[0]<_colorList[0]) return 1;
                }
            }

            return 0;

        }
        
        bool operator <  (const DynGeoSet& rhs) const { return compare(rhs)<0; }
        bool operator == (const DynGeoSet& rhs) const { return compare(rhs)==0; }
        bool operator != (const DynGeoSet& rhs) const { return compare(rhs)!=0; }

        void setStateSet(osg::StateSet* stateset) {
        _stateset = stateset;
        _geom->setStateSet( stateset );
    }
        osg::StateSet* getStateSet() { return _stateset.get(); }
        const osg::StateSet* getStateSet() const { return _stateset.get(); }
        
        void setColorBinding(osg::Geometry::AttributeBinding bind) { _color_binding = bind; }
        void setNormalBinding(osg::Geometry::AttributeBinding bind) { _normal_binding = bind; }
        void setTextureBinding(osg::Geometry::AttributeBinding bind) { _texture_binding = bind; }

        osg::Geometry::AttributeBinding getColorBinding() const { return _color_binding; }
        osg::Geometry::AttributeBinding getNormalBinding() const { return _normal_binding; }
        osg::Geometry::AttributeBinding getTextureBinding() const { return _texture_binding; }

        void setPrimType(osg::PrimitiveSet::Mode type) { _primtype=type; }
        osg::PrimitiveSet::Mode getPrimType() const { return _primtype; }

        inline void addPrimLen(int len)           { _primLenList.push_back(len); }
        inline void addCoord(const osg::Vec3& coord)    { _coordList.push_back(coord); }
        inline void addNormal(const osg::Vec3& normal)  { _normalList.push_back(normal); }
        inline void addColor(const osg::Vec4& color)    { _colorList.push_back(color); }
        inline void addTCoord(const osg::Vec2& tcoord)  { _tcoordList.push_back(tcoord); }

        void append(DynGeoSet* source);
        void setBinding();
        
        void addToGeometry(osg::Geometry* geom);

        inline int primLenListSize() const { return _primLenList.size(); }
        inline int coordListSize() const { return _coordList.size(); }
        inline int normalListSize() const { return _normalList.size(); }
        inline int colorListSize() const { return _colorList.size(); }
        inline int tcoordListSize() const { return _tcoordList.size(); }
    inline void setDetailTextureAttrData(AttrData *attrdata) {_attrdata=attrdata; }

    osg::Geometry* getGeometry() {
            CERR  << "_geom.get(): " << _geom.get()
                << "; referenceCount: " << _geom.get()->referenceCount()<<"\n";
            return _geom.get();
        };

    private:

        typedef std::vector<int>        PrimLenList;
        typedef std::vector<osg::Vec3>  CoordList;
        typedef std::vector<osg::Vec3>  NormalList;
        typedef std::vector<osg::Vec4>  ColorList;
        typedef std::vector<osg::Vec2>  TcoordList;

        osg::ref_ptr<osg::Geometry>  _geom;

        osg::ref_ptr<osg::StateSet> _stateset;


        osg::PrimitiveSet::Mode     _primtype;
        PrimLenList                 _primLenList;

        CoordList                   _coordList;

        osg::Geometry::AttributeBinding _normal_binding;
        NormalList                  _normalList;

        osg::Geometry::AttributeBinding _color_binding;
        ColorList                   _colorList;

        osg::Geometry::AttributeBinding _texture_binding;
        TcoordList                  _tcoordList;

        AttrData                    *_attrdata;
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
        osg::PrimitiveSet::Mode findPrimType(const int nVertices);

    private:

        osg::ref_ptr<osg::Geode>    _geode;
        osg::ref_ptr<DynGeoSet>     _dynGeoSet;

        typedef std::vector<osg::ref_ptr<DynGeoSet> > DynGeoSetList;
        DynGeoSetList               _dynGeoSetList;
};


}; // end of namespace flt


#endif

