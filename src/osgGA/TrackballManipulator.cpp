#include <osgGA/TrackballManipulator>

using namespace osg;
using namespace osgGA;



/// Constructor.
TrackballManipulator::TrackballManipulator( int flags )
   : inherited( flags )
{
    setVerticalAxisFixed( false );
}


/// Constructor.
TrackballManipulator::TrackballManipulator( const TrackballManipulator& tm, const CopyOp& copyOp )
    : inherited( tm, copyOp )
{
}
