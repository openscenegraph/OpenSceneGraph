#ifndef ESRI_SHAPE_PARSER_H
#define ESRI_SHAPE_PARSER_H

#include <string>
#include <osg/Geode>

#include "ESRIShape.h"

namespace ESRIShape {

class ESRIShapeParser
{
    public:
        ESRIShapeParser( const std::string fileName="" );

        osg::Geode *getGeode();

    private:
        bool _valid;
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
