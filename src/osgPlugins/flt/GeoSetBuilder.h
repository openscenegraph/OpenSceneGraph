#ifndef __FLT_GEOSETBUILDER_H
#define __FLT_GEOSETBUILDER_H

// Added DynGeoSet::setDetailTextureAttrData that is used to store texture 
// Attributes
// Julian Ortiz, June 18th 2003.
// ---
// Added support for multiple layers of texture coordinates.  Changed the
// detail texture support to only store the M & N scalar values instead of
// the whole AttrData structure.
// Jason Daly, Sept 25, 2004

#include <osg/ref_ptr>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Geometry>
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

            for (unsigned int i = 0; i < _texture_bindings.size(); i++)
            {
               if (getTextureBinding(i)<rhs.getTextureBinding(i))
                  return -1;
               if (getTextureBinding(i)>rhs.getTextureBinding(i))
                  return 1;
            }

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

            for (unsigned int i = 0; i < _texture_bindings.size(); i++)
            {
               if (getTextureBinding(i)<rhs.getTextureBinding(i))
                  return -1;
               if (getTextureBinding(i)>rhs.getTextureBinding(i))
                  return 1;
            }

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
        void setTextureBinding(osg::Geometry::AttributeBinding bind) 
        { 
           setTextureBinding(0, bind);
        }
        void setTextureBinding(unsigned int index, 
                               osg::Geometry::AttributeBinding bind) 
        { 
           if (_texture_bindings.size() <= index)
              _texture_bindings.resize(index+1);

           _texture_bindings[index] = bind;
        }

        osg::Geometry::AttributeBinding getColorBinding() const { return _color_binding; }
        osg::Geometry::AttributeBinding getNormalBinding() const { return _normal_binding; }
        osg::Geometry::AttributeBinding getTextureBinding() const { return getTextureBinding(0); }
        osg::Geometry::AttributeBinding getTextureBinding(unsigned int index) const
        {
           if (_texture_bindings.size() > index)
              return _texture_bindings[index];
           else
              return osg::Geometry::BIND_OFF;
        }

        void setPrimType(osg::PrimitiveSet::Mode type) { _primtype=type; }
        osg::PrimitiveSet::Mode getPrimType() const { return _primtype; }

        inline void addPrimLen(int len)           { _primLenList.push_back(len); }
        inline void addCoord(const osg::Vec3& coord)    { _coordList.push_back(coord); }
        inline void addNormal(const osg::Vec3& normal)  { _normalList.push_back(normal); }
        inline void addColor(const osg::Vec4& color)    { _colorList.push_back(color); }
        inline void addTCoord(const osg::Vec2& tcoord)  { addTCoord(0, tcoord); }
        inline void addTCoord(unsigned int index, const osg::Vec2& tcoord)  
        { 
           if (_tcoordLists.size() <= index)
               _tcoordLists.resize(index+1);

           _tcoordLists[index].push_back(tcoord); 
        }

        typedef std::vector<osg::Vec3>   CoordList;
        typedef std::vector<osg::Vec3>   NormalList;
        typedef std::vector<osg::Vec4>   ColorList;
        typedef std::vector<osg::Vec2>   TcoordList;
        typedef std::vector<TcoordList>  TcoordLists;
        typedef std::vector<osg::Geometry::AttributeBinding>  TextureBindings;

	const CoordList&  getCoordList()    { return _coordList; }
	const NormalList& getNormalList()   { return _normalList; }
	const ColorList&  getColorList()    { return _colorList; }
	const TcoordList& getTcoordList()   { return getTcoordList(0); }
	const TcoordList& getTcoordList(unsigned int index)
        { 
           if (_tcoordLists.size() <= index)
               _tcoordLists.resize(index+1);

           return _tcoordLists[index]; 
        }

        void append(DynGeoSet* source);
        void setBinding();
        
        void addToGeometry(osg::Geometry* geom);

        inline int primLenListSize() const { return _primLenList.size(); }
        inline int coordListSize() const { return _coordList.size(); }
        inline int normalListSize() const { return _normalList.size(); }
        inline int colorListSize() const { return _colorList.size(); }
        inline int tcoordListSize() const { return tcoordListSize(0); }
        inline int tcoordListSize(unsigned int index) const 
        { 
           if (_tcoordLists.size() <= index) 
              return _tcoordLists[index].size() ;
           else 
              return 0; 
        }

        inline void enableDetailTexture() { _detailTextureEnabled=true; }
        inline void disableDetailTexture() { _detailTextureEnabled=false; }
        inline void setDetailTexCoords(int32 m, int32 n)
        {
            // If somebody knows what the other TX_DETAIL parameters do,
            // please add them.  I looked through the OF specs as well as
            // the SGIS_detail_texture extension, and I didn't find any 
            // clear explanation.  The only reason this is here at all is
            // because of Julian Ortiz' previous work.
            _detailTexCoord_m = m;
            _detailTexCoord_n = n;
        }

    osg::Geometry* getGeometry() {
            CERR  << "_geom.get(): " << _geom.get()
                << "; referenceCount: " << _geom.get()->referenceCount()<<"\n";
            return _geom.get();
        };

    private:

        typedef std::vector<int>        PrimLenList;

        osg::ref_ptr<osg::Geometry>  _geom;

        osg::ref_ptr<osg::StateSet> _stateset;


        osg::PrimitiveSet::Mode     _primtype;
        PrimLenList                 _primLenList;

        CoordList                   _coordList;

        osg::Geometry::AttributeBinding _normal_binding;
        NormalList                  _normalList;

        osg::Geometry::AttributeBinding _color_binding;
        ColorList                   _colorList;

        TextureBindings             _texture_bindings;
        TcoordLists                 _tcoordLists;

        int32                       _detailTexCoord_m;
        int32                       _detailTexCoord_n;
        bool                        _detailTextureEnabled;
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

