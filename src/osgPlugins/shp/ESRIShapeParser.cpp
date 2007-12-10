#include <fcntl.h>
#include <osg/Geometry>
#include <osg/Notify>
#include <osgUtil/Tessellator>

#if defined(_MSC_VER)
    #include <io.h>
    #include <stdio.h>
#endif 

#include "ESRIShapeParser.h"

using namespace ESRIShape;

ESRIShapeParser::ESRIShapeParser( const std::string fileName, bool useDouble ):
    _valid(false),
    _useDouble(useDouble)
{
    int fd = 0;
    if( !fileName.empty() )
    {
#ifdef WIN32
        if( (fd = open( fileName.c_str(), O_RDONLY | O_BINARY )) <= 0 )
#else
        if( (fd = open( fileName.c_str(), O_RDONLY )) <= 0 )
#endif
        {
            perror( fileName.c_str() );
            return ;
        }
    }

    _valid = true;

    ESRIShape::ShapeHeader head;
    head.read(fd);

    //head.print();

    _geode = new osg::Geode;

    switch( head.shapeType )
    {
        case ESRIShape::ShapeTypeNullShape  :
            break;
    
        case ESRIShape::ShapeTypePoint      :
            {
                std::vector<ESRIShape::Point> pts;
                ESRIShape::PointRecord pointRecord;
                while( pointRecord.read(fd) )
                    pts.push_back( pointRecord.point );
                _process( pts );
            }
            break;

        case ESRIShape::ShapeTypeMultiPoint :
            {
                std::vector<ESRIShape::MultiPoint> mpts;
                ESRIShape::MultiPoint mpoint;

                while( mpoint.read(fd) )
                    mpts.push_back( mpoint );

                _process(  mpts );
            }
            break;

        case ESRIShape::ShapeTypePolyLine   : 
            {
                std::vector<ESRIShape::PolyLine> plines;
                ESRIShape::PolyLine pline;

                while( pline.read(fd) )
                    plines.push_back( pline );

                _process( plines );

            }
            break;

        case ESRIShape::ShapeTypePolygon    :
            {
                std::vector<ESRIShape::Polygon> polys;
                ESRIShape::Polygon poly;

                while( poly.read(fd) )
                    polys.push_back( poly );

                _process( polys );
            }
            break;

        case ESRIShape::ShapeTypePointM      :
            {
                std::vector<ESRIShape::PointM> ptms;
                ESRIShape::PointMRecord pointMRecord;
                while( pointMRecord.read(fd) )
                    ptms.push_back( pointMRecord.pointM );
                _process( ptms );
            }
            break;

        case ESRIShape::ShapeTypeMultiPointM :
            {
                std::vector<ESRIShape::MultiPointM> mptms;
                ESRIShape::MultiPointM mpointm;

                while( mpointm.read(fd) )
                    mptms.push_back( mpointm );

                _process(  mptms );
            }
            break;
    
        case ESRIShape::ShapeTypePolyLineM   :
            {
                std::vector<ESRIShape::PolyLineM> plinems;
                ESRIShape::PolyLineM plinem;

                while( plinem.read(fd) )
                    plinems.push_back( plinem );

                _process( plinems );
            }
            break;
    
        case ESRIShape::ShapeTypePolygonM    :
            {
                std::vector<ESRIShape::PolygonM> polyms;
                ESRIShape::PolygonM polym;

                while( polym.read(fd) )
                    polyms.push_back( polym );

                _process( polyms );
            }
            break;
    

        case ESRIShape::ShapeTypePointZ      :
            {
                std::vector<ESRIShape::PointZ> ptzs;
                ESRIShape::PointZ pointZ;
                while( pointZ.read( fd ) )
                    ptzs.push_back( pointZ );
                _process( ptzs );
            }
            break;
    
        case ESRIShape::ShapeTypeMultiPointZ :
            {
                std::vector<ESRIShape::MultiPointZ> mptzs;
                ESRIShape::MultiPointZ mpointz;

                while( mpointz.read(fd) )
                    mptzs.push_back( mpointz );

                _process(  mptzs );
            }
            break;
    
        case ESRIShape::ShapeTypePolyLineZ   :
            {
                std::vector<ESRIShape::PolyLineZ> plinezs;
                ESRIShape::PolyLineZ plinez;

                while( plinez.read(fd) )
                    plinezs.push_back( plinez );

                _process( plinezs );
            }
            break;
    
        case ESRIShape::ShapeTypePolygonZ    :
            {
                std::vector<ESRIShape::PolygonZ> polyzs;
                ESRIShape::PolygonZ polyz;

                while( polyz.read(fd) )
                    polyzs.push_back( polyz );

                _process( polyzs );
            }
            break;
    
    
        case ESRIShape::ShapeTypeMultiPatch  :
            {
                std::vector<ESRIShape::MultiPatch> mpatches;
                ESRIShape::MultiPatch mpatch;

                while( mpatch.read( fd ) )
                    mpatches.push_back( mpatch );

                _process(mpatches);
            }
            break;
    
        default:
            break;
    }

    if(fd)
    {
      close(fd);
      fd = 0;
    }
}

osg::Geode *ESRIShapeParser::getGeode() 
{ 
    return _geode.get(); 
}

void ESRIShapeParser::_combinePointToMultipoint()
{
    if( !_valid ) return;
    
    osg::notify(osg::NOTICE)<<"_combinePointToMultipoint()"<<std::endl;

    ArrayHelper coords(_useDouble);

    unsigned int numDrawables = _geode->getNumDrawables();

    for( unsigned int i = 0; i < numDrawables; i++ )
    {
        osg::Geometry *geom = dynamic_cast<osg::Geometry *>(_geode->getDrawable(i));
        if( geom != 0L )
        {
            coords.add( geom->getVertexArray(), 0 );
        }
    }

    _geode->removeDrawables( 0, numDrawables );

    osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
    geometry->setVertexArray(coords.get());
    geometry->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, coords.size()));
    _geode->addDrawable( geometry.get() );
}

void ESRIShapeParser::_process( const std::vector<ESRIShape::Point> &pts )
{
    if( !_valid ) return;

    std::vector<ESRIShape::Point>::const_iterator p;
    for( p = pts.begin(); p != pts.end(); p++ )
    {
        ArrayHelper coords(_useDouble);

        coords.add( p->x, p->y, 0.0 );
        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        geometry->setVertexArray(coords.get());
        geometry->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, 1));
        _geode->addDrawable( geometry.get() );
    }
    if( _geode->getNumDrawables() > 1 )
        _combinePointToMultipoint();
}


void ESRIShapeParser::_process( const std::vector<ESRIShape::MultiPoint> &mpts )
{
    if( !_valid ) return;

    std::vector<ESRIShape::MultiPoint>::const_iterator p;
    for( p = mpts.begin(); p != mpts.end(); p++ )
    {
        ArrayHelper coords(_useDouble);

        for( int i = 0; i < p->numPoints ; i++ )
            coords.add( p->points[i].x, p->points[i].y, 0.0 );

        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        geometry->setVertexArray(coords.get());
        geometry->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, coords.size()));

        _geode->addDrawable( geometry.get() );
    }
}

void ESRIShapeParser::_process(const std::vector<ESRIShape::PolyLine> &lines )
{
    if( !_valid ) return;

    std::vector<ESRIShape::PolyLine>::const_iterator p;
    for( p = lines.begin(); p != lines.end(); p++ )
    {
        ArrayHelper coords(_useDouble);

        int i;
        for( i = 0; i < p->numPoints; i++ )
            coords.add( p->points[i].x, p->points[i].y, 0.0 );

        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        geometry->setVertexArray(coords.get());

        for( i = 0; i < p->numParts; i++ )
        {
            int index = p->parts[i];
            int len = i < p->numParts - 1 ? 
                            p->parts[i+1] - p->parts[i] :
                            p->numPoints  - p->parts[i];

            geometry->addPrimitiveSet( 
                    new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, index, len));
        }
        _geode->addDrawable( geometry.get() );
    }
}

void ESRIShapeParser::_process( const std::vector<ESRIShape::Polygon> &polys )
{
    if( !_valid ) return;

    std::vector<ESRIShape::Polygon>::const_iterator p;
    for( p = polys.begin(); p != polys.end(); p++ )
    {
        ArrayHelper coords(_useDouble);
        int i;
        for( i = 0; i < p->numPoints; i++ )
            coords.add( p->points[i].x, p->points[i].y, 0.0 );

        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        geometry->setVertexArray(coords.get());

        for( i = 0; i < p->numParts; i++ )
        {
            int index = p->parts[i];
            int len = i < p->numParts - 1 ? 
                            p->parts[i+1] - p->parts[i] :
                            p->numPoints  - p->parts[i];

            geometry->addPrimitiveSet( 
                    new osg::DrawArrays(osg::PrimitiveSet::POLYGON, index, len));
        }

        // Use osgUtil::Tessellator to handle concave polygons
        osg::ref_ptr<osgUtil::Tessellator> tscx=new osgUtil::Tessellator;
        tscx->setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
        tscx->setBoundaryOnly(false);
        tscx->setWindingType( osgUtil::Tessellator::TESS_WINDING_ODD);

        tscx->retessellatePolygons(*(geometry.get()));

        _geode->addDrawable( geometry.get() );
    }
}

void ESRIShapeParser::_process( const std::vector<ESRIShape::PointM> &ptms )
{
    if( !_valid ) return;

    std::vector<ESRIShape::PointM>::const_iterator p;
    for( p = ptms.begin(); p != ptms.end(); p++ )
    {
        ArrayHelper coords(_useDouble);
        coords.add( p->x, p->y, 0.0 );
        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        geometry->setVertexArray(coords.get());
        geometry->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, 1));
        _geode->addDrawable( geometry.get() );
    }
    if( _geode->getNumDrawables() > 1 )
        _combinePointToMultipoint();
}

void ESRIShapeParser::_process( const std::vector<ESRIShape::MultiPointM> &mptms )
{
    if( !_valid ) return;

    std::vector<ESRIShape::MultiPointM>::const_iterator p;
    for( p = mptms.begin(); p != mptms.end(); p++ )
    {
        osg::ref_ptr<osg::Vec3Array> coords  = new osg::Vec3Array;

        // Here is where we would use the 'M' (?)
        for( int i = 0; i < p->numPoints ; i++ )
            coords->push_back( osg::Vec3( p->points[i].x, p->points[i].y, 0.0 ));

        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        geometry->setVertexArray(coords.get());
        geometry->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, coords->size()));

        _geode->addDrawable( geometry.get() );
    }
}

void ESRIShapeParser::_process(const std::vector<ESRIShape::PolyLineM> &linems )
{
    if( !_valid ) return;

    std::vector<ESRIShape::PolyLineM>::const_iterator p;
    for( p = linems.begin(); p != linems.end(); p++ )
    {
        osg::ref_ptr<osg::Vec3Array> coords  = new osg::Vec3Array;

        int i;
        for( i = 0; i < p->numPoints; i++ )
            coords->push_back( osg::Vec3( p->points[i].x, p->points[i].y, 0.0 ));

        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        geometry->setVertexArray(coords.get());

        for( i = 0; i < p->numParts; i++ )
        {
            int index = p->parts[i];
            int len = i < p->numParts - 1 ? 
                            p->parts[i+1] - p->parts[i] :
                            p->numPoints  - p->parts[i];

            geometry->addPrimitiveSet( 
                    new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, index, len));
        }
        _geode->addDrawable( geometry.get() );
    }
}

void ESRIShapeParser::_process( const std::vector<ESRIShape::PolygonM> &polyms )
{
    if( !_valid ) return;

    std::vector<ESRIShape::PolygonM>::const_iterator p;
    for( p = polyms.begin(); p != polyms.end(); p++ )
    {
        osg::ref_ptr<osg::Vec3Array> coords  = new osg::Vec3Array;
        int i;
        for( i = 0; i < p->numPoints; i++ )
            coords->push_back( osg::Vec3( p->points[i].x, p->points[i].y, 0.0 ));

        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        geometry->setVertexArray(coords.get());

        for( i = 0; i < p->numParts; i++ )
        {
            int index = p->parts[i];
            int len = i < p->numParts - 1 ? 
                            p->parts[i+1] - p->parts[i] :
                            p->numPoints  - p->parts[i];

            geometry->addPrimitiveSet( 
                    new osg::DrawArrays(osg::PrimitiveSet::POLYGON, index, len));
        }
        _geode->addDrawable( geometry.get() );
    }
}

void ESRIShapeParser::_process( const std::vector<ESRIShape::PointZ> &ptzs )
{
    if( !_valid ) return;

    std::vector<ESRIShape::PointZ>::const_iterator p;
    for( p = ptzs.begin(); p != ptzs.end(); p++ )
    {
        osg::ref_ptr<osg::Vec3Array> coords  = new osg::Vec3Array;
        coords->push_back( osg::Vec3( p->x, p->y, p->z ));
        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        geometry->setVertexArray(coords.get());
        geometry->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, 1));
        _geode->addDrawable( geometry.get() );
    }
    if( _geode->getNumDrawables() > 1 )
        _combinePointToMultipoint();
}

void ESRIShapeParser::_process( const std::vector<ESRIShape::MultiPointZ> &mptzs )
{
    if( !_valid ) return;

    std::vector<ESRIShape::MultiPointZ>::const_iterator p;
    for( p = mptzs.begin(); p != mptzs.end(); p++ )
    {
        osg::ref_ptr<osg::Vec3Array> coords  = new osg::Vec3Array;

        // Here is where we would use the 'M' (?)
        for( int i = 0; i < p->numPoints ; i++ )
            coords->push_back( osg::Vec3( p->points[i].x, p->points[i].y, p->zArray[i] ));

        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        geometry->setVertexArray(coords.get());
        geometry->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, coords->size()));

        _geode->addDrawable( geometry.get() );
    }
}

void ESRIShapeParser::_process(const std::vector<ESRIShape::PolyLineZ> &linezs )
{
    if( !_valid ) return;

    std::vector<ESRIShape::PolyLineZ>::const_iterator p;
    for( p = linezs.begin(); p != linezs.end(); p++ )
    {
        osg::ref_ptr<osg::Vec3Array> coords  = new osg::Vec3Array;

        int i;
        for( i = 0; i < p->numPoints; i++ )
            coords->push_back( osg::Vec3( p->points[i].x, p->points[i].y, p->zArray[i] ));

        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        geometry->setVertexArray(coords.get());

        for( i = 0; i < p->numParts; i++ )
        {
            int index = p->parts[i];
            int len = i < p->numParts - 1 ? 
                            p->parts[i+1] - p->parts[i] :
                            p->numPoints  - p->parts[i];

            geometry->addPrimitiveSet( 
                    new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP, index, len));
        }
        _geode->addDrawable( geometry.get() );
    }
}

void ESRIShapeParser::_process( const std::vector<ESRIShape::PolygonZ> &polyzs )
{
    if( !_valid ) return;

    std::vector<ESRIShape::PolygonZ>::const_iterator p;
    for( p = polyzs.begin(); p != polyzs.end(); p++ )
    {
        osg::ref_ptr<osg::Vec3Array> coords  = new osg::Vec3Array;
        
        int i;
        for( i = 0; i < p->numPoints; i++ )
            coords->push_back( osg::Vec3( p->points[i].x, p->points[i].y, p->zArray[i] ));

        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        geometry->setVertexArray(coords.get());

        for( i = 0; i < p->numParts; i++ )
        {
            int index = p->parts[i];
            int len = i < p->numParts - 1 ? 
                            p->parts[i+1] - p->parts[i] :
                            p->numPoints  - p->parts[i];

            geometry->addPrimitiveSet( 
                    new osg::DrawArrays(osg::PrimitiveSet::POLYGON, index, len));
        }
        _geode->addDrawable( geometry.get() );
    }
}

void ESRIShapeParser::_process( const std::vector<ESRIShape::MultiPatch> &mpatches )
{
    if( !_valid ) return;

    std::vector<ESRIShape::MultiPatch>::const_iterator p;
    for( p = mpatches.begin(); p != mpatches.end(); p++ )
    {
        osg::ref_ptr<osg::Vec3Array> coords  = new osg::Vec3Array;
        
        int i;
        for( i = 0; i < p->numPoints; i++ )
            coords->push_back( osg::Vec3( p->points[i].x, p->points[i].y, p->zArray[i] ));

        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        geometry->setVertexArray(coords.get());

        // Lets mark poorly supported primitives with red, otherwise white
        osg::ref_ptr<osg::Vec4Array> colors = new osg::Vec4Array;
        geometry->setColorArray(colors.get());
        geometry->setColorBinding(osg::Geometry::BIND_PER_VERTEX );

        for( i = 0; i < p->numParts; i++ )
        {
            int index = p->parts[i];
            int len = i < p->numParts - 1 ? 
                            p->parts[i+1] - p->parts[i] :
                            p->numPoints  - p->parts[i];

            int  mode = 
                p->partTypes[i] == TriangleStrip ? osg::PrimitiveSet::TRIANGLE_STRIP :
                p->partTypes[i] == TriangleFan   ? osg::PrimitiveSet::TRIANGLE_FAN :
                // HACK for now
                p->partTypes[i] == OuterRing     ? osg::PrimitiveSet::LINE_STRIP :
                p->partTypes[i] == InnerRing     ? osg::PrimitiveSet::LINE_STRIP :
                p->partTypes[i] == FirstRing     ? osg::PrimitiveSet::LINE_STRIP :
                p->partTypes[i] == Ring          ? osg::PrimitiveSet::LINE_STRIP : 
                                                   osg::PrimitiveSet::POINTS ; 

            if( p->partTypes[i] == OuterRing ||
                p->partTypes[i] == InnerRing || 
                p->partTypes[i] == FirstRing || p->partTypes[i] == Ring )
            {
                osg::notify(osg::WARN) << "ESRIShapeParser - MultiPatch type " << 
                    (p->partTypes[i] == TriangleStrip ? "TriangleStrip":
                     p->partTypes[i] == TriangleFan   ? "TriangleFan":
                     p->partTypes[i] == OuterRing     ? "OuterRing":
                     p->partTypes[i] == InnerRing     ? "InnerRing":
                     p->partTypes[i] == FirstRing     ? "FirstRing":
                     p->partTypes[i] == Ring          ? "Ring": "Dunno") << 
                    " poorly supported.  Will be represented by a red line strip" << std::endl;
            }
                                                   
        

            // Lets mark poorly supported primitives with red, otherwise white
            osg::Vec4 color = 
                p->partTypes[i] == TriangleStrip ? osg::Vec4(1.0,1.0,1.0,1.0) :
                p->partTypes[i] == TriangleFan   ? osg::Vec4(1.0,1.0,1.0,1.0) :
                // HACK for now
                p->partTypes[i] == OuterRing     ? osg::Vec4(1.0,0.0,0.0,1.0) :
                p->partTypes[i] == InnerRing     ? osg::Vec4(1.0,0.0,0.0,1.0) :
                p->partTypes[i] == FirstRing     ? osg::Vec4(1.0,0.0,0.0,1.0) :
                p->partTypes[i] == Ring          ? osg::Vec4(1.0,0.0,0.0,1.0) :
                                                   osg::Vec4(1.0,0.0,0.0,1.0) ;
            for( int j = 0; j < len; j++ )
                colors->push_back( color );

            geometry->addPrimitiveSet( new osg::DrawArrays(mode, index, len )); 
        }

        _geode->addDrawable( geometry.get() );
    }
}
