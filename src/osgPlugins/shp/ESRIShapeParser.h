#ifndef ESRI_SHAPE_PARSER_H
#define ESRI_SHAPE_PARSER_H

#include <string>
#include <osg/Geode>

#include "ESRIShape.h"

namespace ESRIShape {

class ArrayHelper
{
    public:
        ArrayHelper(bool useDouble)
        {
            if (useDouble) _vec3darray = new osg::Vec3dArray;
            else _vec3farray = new osg::Vec3Array;
        }

        osg::Array* get() { return _vec3farray.valid() ? 
                static_cast<osg::Array*>(_vec3farray.get()) : 
                static_cast<osg::Array*>(_vec3darray.get()); }

        unsigned int size() { return _vec3farray.valid() ? 
                _vec3farray->size() : 
                _vec3darray->size(); }

        void add(double x, double y, double z)
        {
            if (_vec3farray.valid()) _vec3farray->push_back(osg::Vec3(x,y,z));
            else _vec3darray->push_back(osg::Vec3d(x,y,z));
        }

        void add(const osg::Vec3& v)
        {
            if (_vec3farray.valid()) _vec3farray->push_back(v);
            else _vec3darray->push_back(osg::Vec3d(v.x(),v.y(),v.z()));
        }

        void add(const osg::Vec3d& v)
        {
            if (_vec3farray.valid()) _vec3farray->push_back(osg::Vec3(v.x(),v.y(),v.z()));
            else _vec3darray->push_back(v);
        }

        void add(osg::Array* array, unsigned int index)
        {
            osg::Vec3Array* vec3Array = dynamic_cast<osg::Vec3Array*>(array);
            if (vec3Array && index<vec3Array->size()) add((*vec3Array)[index]);
            
            osg::Vec3dArray* vec3dArray = dynamic_cast<osg::Vec3dArray*>(array);
            if (vec3dArray && index<vec3dArray->size()) add((*vec3dArray)[index]);
        }

        osg::ref_ptr<osg::Vec3Array> _vec3farray;
        osg::ref_ptr<osg::Vec3dArray> _vec3darray;
};


class ESRIShapeParser
{
    public:
    
        ESRIShapeParser( const std::string fileName, bool useDouble);

        osg::Geode *getGeode();

#if 0
#if 1
        typedef osg::Vec3d ShapeVec3;
        typedef osg::Vec3dArray ShapeVec3Array;
#else
        typedef osg::Vec3 ShapeVec3;
        typedef osg::Vec3Array ShapeVec3Array;
#endif
#endif

    private:
    
    
    
        bool _valid;
        bool _useDouble;
        
        osg::ref_ptr<osg::Geode> _geode;

        void _combinePointToMultipoint();
        void _process( const std::vector<ESRIShape::Point> &);
        void _process( const std::vector<ESRIShape::MultiPoint> &);
        void _process( const std::vector<ESRIShape::PolyLine> &);
        void _process( const std::vector<ESRIShape::Polygon> &);

        void _process( const std::vector<ESRIShape::PointM> &);
        void _process( const std::vector<ESRIShape::MultiPointM> &);
        void _process( const std::vector<ESRIShape::PolyLineM> &);
        void _process( const std::vector<ESRIShape::PolygonM> &);

        void _process( const std::vector<ESRIShape::PointZ> &);
        void _process( const std::vector<ESRIShape::MultiPointZ> &);
        void _process( const std::vector<ESRIShape::PolyLineZ> &);
        void _process( const std::vector<ESRIShape::PolygonZ> &);
        void _process( const std::vector<ESRIShape::MultiPatch> &);

};

}

#endif
