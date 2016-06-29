
#if defined(_MSC_VER) || defined(__MINGW32__)
    #include <stdio.h>
    #include <io.h>

    namespace esri
    {
        int read(int fd, void * buf, size_t nbytes) { return _read(fd, buf, static_cast<unsigned int>(nbytes)); }
    }

#else
    #include <unistd.h>

    namespace esri
    {
        int read(int fd, void * buf, size_t nbytes) { return ::read(fd, buf, nbytes); }
    }

#endif

#include "ESRIShape.h"

#include <float.h>

using namespace ESRIShape ;

#define SAFE_DELETE_ARRAY( ptr ) delete[] ptr; ptr = 0L;

template <class T>
inline void swapBytes(  T &s )
{
    if( sizeof( T ) == 1 ) return;

    T d = s;
    BytePtr sptr = (BytePtr)&s;
    BytePtr dptr = &(((BytePtr)&d)[sizeof(T)-1]);

    for( unsigned int i = 0; i < sizeof(T); i++ )
        *(sptr++) = *(dptr--);
}

inline ByteOrder getByteOrder()
{
    int one = 1;
    unsigned char *ptr = (unsigned char *)&one;
    if( ptr[0] == 1 )
        return LittleEndian;
    else
        return BigEndian;
}

template <class T>
inline bool readVal( int fd, T &val, ByteOrder bo = LittleEndian )
{
    int nbytes = 0;
    if( (nbytes = esri::read( fd, &val, sizeof(T))) <= 0 )
        return false;

    if( getByteOrder() != bo )
        swapBytes<T>(val);

    return true;
}


template <class T>
inline bool readPositiveVal( int fd, T &val, ByteOrder bo = LittleEndian )
{
    int nbytes = 0;
    if( (nbytes = esri::read( fd, &val, sizeof(T))) <= 0 )
        return false;

    if( getByteOrder() != bo )
        swapBytes<T>(val);

    if (val<0)
    {
         val = 0;
         return false;
    }

    return true;
}

inline void printType( ShapeType type )
{
    printf( "%s",
        type == ShapeTypeNullShape   ? "NullShape" :
        type == ShapeTypePoint       ? "Point" :
        type == ShapeTypePolyLine    ? "PolyLine" :
        type == ShapeTypePolygon     ? "Polygon" :
        type == ShapeTypeMultiPoint  ? "MultiPoint" :
        type == ShapeTypePointZ      ? "PointZ" :
        type == ShapeTypePolyLineZ   ? "PolyLineZ" :
        type == ShapeTypePolygonZ    ? "PolygonZ" :
        type == ShapeTypeMultiPointZ ? "MultiPointZ" :
        type == ShapeTypePointM      ? "PointM" :
        type == ShapeTypePolyLineM   ? "PolyLineM" :
        type == ShapeTypePolygonM    ? "PolygonM" :
        type == ShapeTypeMultiPointM ? "MultiPointM" :
        type == ShapeTypeMultiPatch  ? "MultiPatch" : "Unknown" );
}


bool BoundingBox::read( int fd )
{
    if( readVal<Double>(fd, Xmin, LittleEndian ) == false ) return false;
    if( readVal<Double>(fd, Ymin, LittleEndian ) == false ) return false;
    if( readVal<Double>(fd, Xmax, LittleEndian ) == false ) return false;
    if( readVal<Double>(fd, Ymax, LittleEndian ) == false ) return false;
    if( readVal<Double>(fd, Zmin, LittleEndian ) == false ) return false;
    if( readVal<Double>(fd, Zmax, LittleEndian ) == false ) return false;
    if( readVal<Double>(fd, Mmin, LittleEndian ) == false ) return false;
    if( readVal<Double>(fd, Mmax, LittleEndian ) == false ) return false;

    return true;
}

void BoundingBox::print()
{
    printf( "    Xmin: %G\n", Xmin );
    printf( "    Ymin: %G\n", Ymin );
    printf( "    Xmax: %G\n", Xmax );
    printf( "    Ymax: %G\n", Ymax );
    printf( "    Zmin: %G\n", Zmin );
    printf( "    Zmax: %G\n", Zmax );
    printf( "    Mmin: %G\n", Mmin );
    printf( "    Mmax: %G\n", Mmax );
}


bool ShapeHeader::read(int fd)
{
    if( readVal<Integer>( fd, fileCode, BigEndian ) == false ) return false;
    if( esri::read( fd, _unused_0, sizeof(_unused_0)) <= 0 ) return false;
    if( readVal<Integer>( fd, fileLength, BigEndian ) == false ) return false;
    if( readVal<Integer>( fd, version, LittleEndian ) == false ) return false;
    if( readVal<Integer>( fd, shapeType, LittleEndian ) == false ) return false;
    bbox.read(fd);
    return true;
}

void ShapeHeader::print()
{
    printf( "File Code: %d\n", fileCode );
    printf( "File Length: %d\n", fileLength );
    printf( "Version: %d\n", version );
    printf( "Shape Type: "); printType( ShapeType(shapeType) ); printf( "\n" );
    printf( "Bounding Box:\n" );
    bbox.print();
}

RecordHeader::RecordHeader():
    recordNumber(-1),
    contentLength(0)
{
}

bool RecordHeader::read( int fd )
{
    if( readVal<Integer>( fd, recordNumber, BigEndian ) == false ) return false;
    if( readVal<Integer>( fd, contentLength, BigEndian ) == false ) return false;
    return true;
}

void RecordHeader::print()
{
    printf( " Record Number: %d\n", recordNumber );
    printf( "Content Length: %d\n", contentLength );
}



NullRecord::NullRecord():
    shapeType(ShapeTypeNullShape)
{}

bool NullRecord::read( int fd )
{
    if( readVal<Integer>( fd, shapeType, LittleEndian ) == false ) return false;
    return true;
}


Box::Box():
    Xmin(DBL_MAX),
    Ymin(DBL_MAX),
    Xmax(-DBL_MAX),
    Ymax(-DBL_MAX)
    {}

Box::Box(const Box &b ):
    Xmin(b.Xmin),
    Ymin(b.Ymin),
    Xmax(b.Xmax),
    Ymax(b.Ymax)
    {}

bool Box::read( int fd )
{
    if( readVal<Double>(fd, Xmin, LittleEndian) == false ) return false;
    if( readVal<Double>(fd, Ymin, LittleEndian) == false ) return false;
    if( readVal<Double>(fd, Xmax, LittleEndian) == false ) return false;
    if( readVal<Double>(fd, Ymax, LittleEndian) == false ) return false;
    return true;
}

Range::Range():min(DBL_MAX), max(-DBL_MAX) {}
Range::Range( const Range &r ): min(r.min), max(r.max) {}

bool Range::read( int fd )
{
    if( readVal<Double>(fd, min, LittleEndian ) == false ) return false;
    if( readVal<Double>(fd, max, LittleEndian ) == false ) return false;
    return true;
}

ShapeObject::ShapeObject(ShapeType s):
    shapeType(s)
{}

ShapeObject::~ShapeObject()
{ }



Point::Point():
    ShapeObject(ShapeTypePoint),
    x(0.0),
    y(0.0)
{}

Point::Point(const Point &p):
    ShapeObject(ShapeTypePoint),
    x(p.x),
    y(p.y)
{}

Point::~Point() {}

bool Point::read( int fd )
{

    if( readVal<Double>( fd, x, LittleEndian ) == false ) return false;
    if( readVal<Double>( fd, y, LittleEndian ) == false ) return false;

    return true;
}

bool PointRecord::read( int fd )
{
    RecordHeader rh;
    if( rh.read(fd) == false )
        return false;

    Integer shapeType;
    if( readVal<Integer>(fd, shapeType, LittleEndian ) == false )
        return false;

    if( shapeType != ShapeTypePoint )
        return false;

    return point.read(fd);
}

void Point::print()
{
    printf( "    %G %G\n", x, y );
}

MultiPoint::MultiPoint():
    ShapeObject(ShapeTypeMultiPoint),
    numPoints(0),
    points(0L)
{}

MultiPoint::MultiPoint( const struct MultiPoint &mpoint ): ShapeObject(ShapeTypeMultiPoint),
    bbox(mpoint.bbox),
    numPoints(mpoint.numPoints)
{
    points = new Point[numPoints];
    for( int i = 0; i < numPoints; i++ )
        points[i] = mpoint.points[i];
}

MultiPoint::~MultiPoint()
{
    delete[] points;
}

bool MultiPoint::read( int fd )
{
    RecordHeader rh;
    if( rh.read(fd) == false )
        return false;

    SAFE_DELETE_ARRAY( points );

    Integer st;
    if( readVal<Integer>(fd, st, LittleEndian ) == false )
        return false;

    if( st != ShapeTypeMultiPoint )
        return false;

    if( bbox.read(fd) == false )
        return false;

    if( readPositiveVal<Integer>(fd, numPoints, LittleEndian ) == false )
        return false;

    points = new struct Point[numPoints];
    for( Integer i = 0; i < numPoints; i++ )
    {
        if( points[i].read(fd) == false )
            return false;
    }
    return true;
}

void MultiPoint::print()
{
    printf( "Point - numPoints: %d\n", numPoints );
    for( int i= 0; i < numPoints; i++ )
        points[i].print();
}



PolyLine::PolyLine():
    ShapeObject(ShapeTypePolyLine),
    numParts(0),
    numPoints(0),
    parts(0L),
    points(0L) {}

PolyLine::PolyLine( const PolyLine &p ):
    ShapeObject(ShapeTypePolyLine),
    numParts(p.numParts),
    numPoints(p.numPoints)
{
    parts = new Integer[numParts];
    Integer i;
    for(i = 0; i < numParts; i++ )
        parts[i] = p.parts[i];

    points = new Point[numPoints];
    for(i = 0; i < numPoints; i++ )
        points[i] = p.points[i];
}

PolyLine::~PolyLine()
{
    delete [] parts;
    delete [] points;
}

bool PolyLine::read( int fd )
{
    RecordHeader rh;
    if( rh.read(fd) == false )
        return false;

    SAFE_DELETE_ARRAY( parts );
    SAFE_DELETE_ARRAY( points );

    Integer st;
    if( readVal<Integer>(fd, st, LittleEndian ) == false )
        return false;

    if( st != ShapeTypePolyLine )
        return false;

    if( bbox.read(fd) == false )
        return false;

    if( readPositiveVal<Integer>(fd, numParts, LittleEndian ) == false )
        return false;

    if( readPositiveVal<Integer>(fd, numPoints, LittleEndian ) == false )
        return false;

    parts  = new Integer[numParts];
    int i;
    for( i = 0; i < numParts; i++ )
    {
        if( readVal<Integer>(fd, parts[i], LittleEndian ) == false )
            return false;

    }
    points = new struct Point[numPoints];
    for( i = 0; i < numPoints; i++ )
    {
        if( points[i].read(fd ) == false )
            return false;
    }
    return true;
}


Polygon::Polygon():
    ShapeObject(ShapeTypePolygon),
    numParts(0),
    numPoints(0),
    parts(0L),
    points(0L) {}

    Polygon::Polygon( const Polygon &p ):
    ShapeObject(ShapeTypePolygon),
    numParts(p.numParts),
    numPoints(p.numPoints)
{
    parts = new Integer[numParts];
    Integer i;
    for( i = 0; i < numParts; i++ )
        parts[i] = p.parts[i];

    points = new Point[numPoints];
    for( i = 0; i < numPoints; i++ )
        points[i] = p.points[i];
}

Polygon::~Polygon()
{
    delete [] parts;
    delete [] points;
}


bool Polygon::read( int fd )
{
    RecordHeader rh;
    if( rh.read(fd) == false )
        return false;

    SAFE_DELETE_ARRAY( parts );
    SAFE_DELETE_ARRAY( points );

    Integer st;
    if( readVal<Integer>(fd, st, LittleEndian ) == false )
        return false;

    if( st != ShapeTypePolygon )
        return false;

    if( bbox.read(fd) == false )
        return false;

    if( readPositiveVal<Integer>(fd, numParts, LittleEndian ) == false )
        return false;

    if( readPositiveVal<Integer>(fd, numPoints, LittleEndian ) == false )
        return false;

    parts  = new Integer[numParts];
    int i;
    for( i = 0; i < numParts; i++ )
    {
        if( readVal<Integer>(fd, parts[i], LittleEndian ) == false )
            return false;
    }
    points = new struct Point[numPoints];
    for( i = 0; i < numPoints; i++ )
    {
        if( points[i].read(fd ) == false )
            return false;
    }
    return true;
}

//////////////////////////////////////////////////////////////////////

PointM::PointM():
    ShapeObject(ShapeTypePointM),
    x(0.0),
    y(0.0),
    m(0.0)
{}

PointM::PointM(const PointM &p):
    ShapeObject(ShapeTypePointM),
    x(p.x),
    y(p.y),
    m(p.m)
{}

PointM::~PointM() {}

bool PointM::read( int fd )
{
    if( readVal<Double>( fd, x, LittleEndian ) == false ) return false;
    if( readVal<Double>( fd, y, LittleEndian ) == false ) return false;
    if( readVal<Double>( fd, m, LittleEndian ) == false ) return false;

    return true;
}

void PointM::print()
{
    printf( "    %G %G (%G)\n", x, y, m );
}

bool PointMRecord::read( int fd )
{
    RecordHeader rh;
    if( rh.read(fd) == false )
        return false;

    Integer shapeType;
    if( readVal<Integer>(fd, shapeType, LittleEndian ) == false )
        return false;

    if( shapeType != ShapeTypePointM )
        return false;

    return pointM.read(fd);
}


MultiPointM::MultiPointM():
    ShapeObject(ShapeTypeMultiPointM),
    numPoints(0),
    points(0L),
    mArray(0L)
{}

    MultiPointM::MultiPointM( const struct MultiPointM &mpointm ):
    ShapeObject(ShapeTypeMultiPointM),
    bbox(mpointm.bbox),
    numPoints(mpointm.numPoints),
    mRange(mpointm.mRange)
{
    points = new Point[numPoints];
    mArray = new Double[numPoints];
    for( int i = 0; i < numPoints; i++ )
    {
        points[i] = mpointm.points[i];
        mArray[i] = mpointm.mArray[i];
    }
}

MultiPointM::~MultiPointM()
{
    delete [] points;
    delete [] mArray;
}

bool MultiPointM::read( int fd )
{
    RecordHeader rh;
    if( rh.read(fd) == false )
        return false;

    SAFE_DELETE_ARRAY( points );
    SAFE_DELETE_ARRAY( mArray );

    Integer st;
    if( readVal<Integer>(fd, st, LittleEndian ) == false )
        return false;

    if( st != ShapeTypeMultiPointM )
        return false;

    if( bbox.read(fd) == false )
        return false;

    if( readPositiveVal<Integer>(fd, numPoints, LittleEndian ) == false )
        return false;

    points = new struct Point[numPoints];
    Integer i;
    for( i = 0; i < numPoints; i++ )
    {
        if( points[i].read(fd) == false )
            return false;
    }

    int X = 40 + (16 * numPoints);
    if( rh.contentLength*2 > X )
    {
        if( mRange.read(fd) == false )
            return false;

        mArray = new Double[numPoints];
        for( i = 0; i < numPoints; i++ )
        {
            if( readVal<Double>(fd, mArray[i], LittleEndian ) == false )
                return false;
        }
    }

    return true;
}

void MultiPointM::print()
{
    printf( "Point - numPoints: %d\n", numPoints );
    for( int i= 0; i < numPoints; i++ )
        points[i].print();
}

PolyLineM::PolyLineM():
    ShapeObject(ShapeTypePolyLineM),
    numParts(0),
    numPoints(0),
    parts(0L),
    points(0L),
    mArray(0L)
{}

PolyLineM::PolyLineM(const PolyLineM &p):
    ShapeObject(ShapeTypePolyLineM),
    numParts(p.numParts),
    numPoints(p.numPoints),
    parts(0L),
    points(0L),
    mArray(0L)
{
    parts = new Integer[numParts];
    Integer i;
    for( i = 0; i < numParts; i++ )
        parts[i] = p.parts[i];

    points = new Point[numPoints];
    mArray = new Double[numPoints];
    for( i = 0; i < numPoints; i++ )
    {
        points[i] = p.points[i];
        mArray[i] = p.mArray[i];
    }
}

PolyLineM::~PolyLineM()
{
    delete [] parts;
    delete [] points;
    delete [] mArray;
}

bool PolyLineM::read( int fd )
{
    RecordHeader rh;
    if( rh.read(fd) == false )
        return false;

    SAFE_DELETE_ARRAY( parts );
    SAFE_DELETE_ARRAY( points );
    SAFE_DELETE_ARRAY( mArray );

    Integer st;
    if( readVal<Integer>(fd, st, LittleEndian ) == false )
        return false;

    if( st != ShapeTypePolyLineM )
        return false;

    if( bbox.read(fd) == false )
        return false;

    if( readPositiveVal<Integer>(fd, numParts, LittleEndian ) == false )
        return false;

    if( readPositiveVal<Integer>(fd, numPoints, LittleEndian ) == false )
        return false;

    parts  = new Integer[numParts];
    int i;
    for( i = 0; i < numParts; i++ )
    {
        if( readVal<Integer>(fd, parts[i], LittleEndian ) == false )
            return false;

    }
    points = new struct Point[numPoints];
    for( i = 0; i < numPoints; i++ )
    {
        if( points[i].read(fd ) == false )
            return false;
    }

    int X = 44 + (4 * numParts);
    int Y = X + (16 * numPoints);

    if( rh.contentLength*2 > Y )
    {
        mRange.read(fd);
        mArray = new Double[numPoints];
        for( i = 0; i < numPoints; i++  )
        {
            if( readVal<Double>(fd, mArray[i], LittleEndian ) == false )
                return false;
        }
    }

    return true;
}


PolygonM::PolygonM():
    ShapeObject(ShapeTypePolygonM),
    numParts(0),
    numPoints(0),
    parts(0L),
    points(0L) ,
    mArray(0L)
{}

PolygonM::PolygonM(const PolygonM &p):
    ShapeObject(ShapeTypePolygonM),
    numParts(p.numParts),
    numPoints(p.numPoints),
    parts(0L),
    points(0L) ,
    mArray(0L)
{
    parts = new Integer[numParts];
    Integer i;
    for( i = 0; i < numParts; i++ )
        parts[i] = p.parts[i];

    points = new Point[numPoints];
    mArray = new Double[numPoints];
    for( i = 0; i < numPoints; i++ )
    {
        points[i] = p.points[i];
        mArray[i] = p.mArray[i];
    }
}

PolygonM::~PolygonM()
{
    delete[] parts;
    delete[] points;
    delete[] mArray;
}


bool PolygonM::read( int fd )
{
    RecordHeader rh;
    if( rh.read(fd) == false )
        return false;

    SAFE_DELETE_ARRAY( parts );
    SAFE_DELETE_ARRAY( points );
    SAFE_DELETE_ARRAY( mArray );

    Integer st;
    if( readVal<Integer>(fd, st, LittleEndian ) == false )
        return false;

    if( st != ShapeTypePolygonM )
        return false;

    if( bbox.read(fd) == false )
        return false;

    if( readPositiveVal<Integer>(fd, numParts, LittleEndian ) == false )
        return false;

    if( readPositiveVal<Integer>(fd, numPoints, LittleEndian ) == false )
        return false;

    parts  = new Integer[numParts];
    int i;
    for( i = 0; i < numParts; i++ )
    {
        if( readVal<Integer>(fd, parts[i], LittleEndian ) == false )
            return false;
    }
    points = new struct Point[numPoints];
    for( i = 0; i < numPoints; i++ )
    {
        if( points[i].read(fd ) == false )
            return false;
    }

    int X = 44 + (4 * numParts);
    int Y = X + (16 * numPoints);

    if( rh.contentLength*2 > Y )
    {
        if( mRange.read(fd) == false )
            return false;

        mArray = new Double[numPoints];
        for( i = 0; i < numPoints; i++ )
        {
            if( readVal<Double>(fd, mArray[i], LittleEndian ) == false )
                return false;
        }
    }

    return true;
}


//////////////////////////////////////////////////////////////////////

PointZ::PointZ():
    ShapeObject(ShapeTypePointZ),
    x(0.0),
    y(0.0),
    z(0.0),
    m(0.0)
{}

PointZ::PointZ(const PointZ &p):
    ShapeObject(ShapeTypePointZ),
    x(p.x),
    y(p.y),
    z(p.z),
    m(p.m)
{}

PointZ::~PointZ() {}

bool PointZ::read( int fd )
{
    RecordHeader rh;
    if( rh.read(fd) == false )
        return false;

    Integer st;
    if( readVal<Integer>(fd, st, LittleEndian ) == false )
        return false;

    if( st != ShapeTypePointZ )
        return false;

    if( readVal<Double>( fd, x, LittleEndian ) == false )
        return false;

    if( readVal<Double>( fd, y, LittleEndian ) == false )
        return false;

    if( readVal<Double>( fd, z, LittleEndian ) == false )
        return false;

    // Sometimes, M field is not supplied
    if( rh.contentLength >= 18 )
        if( readVal<Double>( fd, m, LittleEndian ) == false )
            return false;

    return true;
}

void PointZ::print()
{
    printf( "    %G %G %G (%G)\n", x, y, z, m );
}

MultiPointZ::MultiPointZ():
    ShapeObject(ShapeTypeMultiPointZ),
    numPoints(0),
    points(0L),
    zArray(0L),
    mArray(0L)
    {}

MultiPointZ::MultiPointZ( const struct MultiPointZ &mpointm ):
    ShapeObject(ShapeTypeMultiPointZ),
    bbox(mpointm.bbox),
    numPoints(mpointm.numPoints),
    zRange(mpointm.zRange),
    mRange(mpointm.mRange)
{
    points = new Point[numPoints];
    zArray = new Double[numPoints];
    mArray = new Double[numPoints];
    for( int i = 0; i < numPoints; i++ )
    {
        points[i] = mpointm.points[i];
        zArray[i] = mpointm.zArray[i];
        mArray[i] = mpointm.mArray[i];
    }
}

MultiPointZ::~MultiPointZ()
{
    delete [] points;
    delete [] zArray;
    delete [] mArray;
}

bool MultiPointZ::read( int fd )
{
    RecordHeader rh;
    if( rh.read(fd) == false )
        return false;

    SAFE_DELETE_ARRAY( points );
    SAFE_DELETE_ARRAY( zArray );
    SAFE_DELETE_ARRAY( mArray );

    Integer st;
    if( readVal<Integer>(fd, st, LittleEndian ) == false )
        return false;

    if( st != ShapeTypeMultiPointZ )
        return false;

    if( bbox.read(fd) == false )
        return false;

    if( readPositiveVal<Integer>(fd, numPoints, LittleEndian ) == false )
        return false;

    points = new struct Point[numPoints];
    Integer i;
    for( i = 0; i < numPoints; i++ )
    {
        if( points[i].read(fd) == false )
            return false;
    }

    if( zRange.read(fd) == false )
        return false;

    zArray = new Double[numPoints];
    for( i = 0; i < numPoints; i++ )
    {
        if( readVal<Double>(fd, zArray[i], LittleEndian) == false )
            return false;
    }

    int X = 40 + (16*numPoints);
    int Y = X + 16 + (8*numPoints);
    if( rh.contentLength*2 > Y )
    {
        if( mRange.read(fd) == false )
            return false;

        mArray = new Double[numPoints];
        for( i = 0; i < numPoints; i++ )
        {
            if( readVal<Double>(fd, mArray[i], LittleEndian ) == false )
                return false;
        }
    }

    return true;
}

void MultiPointZ::print()
{
    printf( "Point - numPoints: %d\n", numPoints );
    for( int i= 0; i < numPoints; i++ )
        points[i].print();
}


PolyLineZ::PolyLineZ():
    ShapeObject(ShapeTypePolyLineZ),
    numParts(0),
    numPoints(0),
    parts(0L),
    points(0L),
    zArray(0L) ,
    mArray(0L)
{}

PolyLineZ::PolyLineZ(const PolyLineZ &p):
    ShapeObject(ShapeTypePolyLineZ),
    numParts(p.numParts),
    numPoints(p.numPoints),
    parts(0L),
    points(0L),
    zArray(0L) ,
    mArray(0L)
{
    parts = new Integer[numParts];
    Integer i;
    for( i = 0; i < numParts; i++ )
        parts[i] = p.parts[i];

    points = new Point[numPoints];
    zArray = new Double[numPoints];
    for( i = 0; i < numPoints; i++ )
    {
        points[i] = p.points[i];
        zArray[i] = p.zArray[i];
    }

    // Sometimes, M Array is not present on the file
    if( p.mArray != NULL )
    {
        mArray = new Double[numPoints];
        for( i = 0; i < numPoints; i++ )
            mArray[i] = p.mArray[i];
    }
}

PolyLineZ::~PolyLineZ()
{
    delete [] parts;
    delete [] points;
    delete [] zArray;
    delete [] mArray;
}

bool PolyLineZ::read( int fd )
{
    RecordHeader rh;
    if( rh.read(fd) == false )
        return false;

    SAFE_DELETE_ARRAY( parts );
    SAFE_DELETE_ARRAY( points );
    SAFE_DELETE_ARRAY( zArray );
    SAFE_DELETE_ARRAY( mArray );

    Integer st;
    if( readVal<Integer>(fd, st, LittleEndian ) == false )
        return false;

    if( st != ShapeTypePolyLineZ )
        return false;

    if( bbox.read(fd) == false )
        return false;

    if( readPositiveVal<Integer>(fd, numParts, LittleEndian ) == false )
        return false;

    if( readPositiveVal<Integer>(fd, numPoints, LittleEndian ) == false )
        return false;

    parts  = new Integer[numParts];
    int i;
    for( i = 0; i < numParts; i++ )
    {
        if( readVal<Integer>(fd, parts[i], LittleEndian ) == false )
            return false;

    }
    points = new struct Point[numPoints];
    for( i = 0; i < numPoints; i++ )
    {
        if( points[i].read(fd ) == false )
            return false;
    }

    zRange.read(fd);
    zArray = new Double[numPoints];
    for( i = 0; i < numPoints; i++ )
    {
        if( readVal<Double>(fd, zArray[i], LittleEndian ) == false )
            return false;
    }

    int X = 44 + (4 * numParts);
    int Y = X + (15 * numPoints);
    int Z = Y + 16 + (8 * numPoints);

    if( rh.contentLength*2 != Z )
    {
        mRange.read(fd);
        mArray = new Double[numPoints];
        for( i = 0; i < numPoints; i++ )
        {
            if( readVal<Double>(fd, mArray[i], LittleEndian ) == false )
                return false;
        }
    }

    return true;
}


PolygonZ::PolygonZ():
    ShapeObject(ShapeTypePolygonZ),
    numParts(0),
    numPoints(0),
    parts(0L),
    points(0L),
    zArray(0L),
    mArray(0L)
{}

PolygonZ::PolygonZ(const PolygonZ &p):
    ShapeObject(ShapeTypePolygonZ),
    numParts(p.numParts),
    numPoints(p.numPoints),
    parts(0L),
    points(0L) ,
    mArray(0L)
{
    parts = new Integer[numParts];
    Integer i;
    for( i = 0; i < numParts; i++ )
        parts[i] = p.parts[i];

    points = new Point[numPoints];
    zArray = new Double[numPoints]; // jcm
    mArray = new Double[numPoints];
    for( i = 0; i < numPoints; i++ )
    {
        points[i] = p.points[i];
        zArray[i] = p.zArray[i]; // jcm
    // M-Array seems to be missing sometimes
    if(p.mArray)
        mArray[i] = p.mArray[i];
    }
}

PolygonZ::~PolygonZ()
{
    delete [] parts;
    delete [] points;
    delete [] zArray;
    delete [] mArray;
}

bool PolygonZ::read( int fd )
{
    RecordHeader rh;
    if( rh.read(fd) == false )
        return false;

    SAFE_DELETE_ARRAY( parts );
    SAFE_DELETE_ARRAY( points );
    SAFE_DELETE_ARRAY( zArray );
    SAFE_DELETE_ARRAY( mArray );

    Integer st;
    if( readVal<Integer>(fd, st, LittleEndian ) == false )
        return false;

    if( st != ShapeTypePolygonZ )
        return false;

    if( bbox.read(fd) == false )
        return false;

    if( readVal<Integer>(fd, numParts, LittleEndian ) == false )
        return false;

    if( readVal<Integer>(fd, numPoints, LittleEndian ) == false )
        return false;

    parts  = new Integer[numParts];
    int i;
    for( i = 0; i < numParts; i++ )
    {
        if( readVal<Integer>(fd, parts[i], LittleEndian ) == false )
            return false;
    }
    points = new struct Point[numPoints];
    for( i = 0; i < numPoints; i++ )
    {
        if( points[i].read(fd ) == false )
            return false;
    }

    if( zRange.read(fd) == false )
        return false;

    zArray = new Double[numPoints];
    for( i = 0; i < numPoints; i++ )
    {
        if( readVal<Double>(fd, zArray[i], LittleEndian ) == false )
            return false;
    }

    int  X = 44 + (4*numParts);
    int  Y = X + (16*numPoints);
    int  Z = Y + 16 + (8*numPoints);
    if( rh.contentLength*2 != Z )
    {
        if( mRange.read(fd) == false )
            return false;

        mArray = new Double[numPoints];
        for( i = 0; i < numPoints; i++ )
        {
            if( readVal<Double>(fd, mArray[i], LittleEndian ) == false )
                return false;
        }
    }

    return true;
}


//////////////////////////////////////////////////////////////////////

/*
struct MultiPatch
{
    Box             bbox;
    Integer         numParts;
    Integer         numPoints;
    Integer         *parts;
    Integer         *partTypes;
    struct Point    *points;
    Range           zRange;
    Double          *zArray;
    Range           mRange;
    Double          *mArray;

    MultiPatch();
    MultiPatch( const MultiPatch &);
    virtual ~MultiPatch();
    bool read( fd );
};
*/

MultiPatch::MultiPatch():
    numParts(0),
    numPoints(0),
    parts(0L),
    partTypes(0L),
    points(0L),
    zArray(0L),
    mArray(0L)
{ }

MultiPatch::MultiPatch( const MultiPatch &mp):
    bbox(mp.bbox),
    numParts(mp.numParts),
    numPoints(mp.numPoints),
    zRange(mp.zRange),
    mRange(mp.mRange)
{
    parts = new Integer[numParts];
    partTypes = new Integer[numParts];
    Integer i;
    for( i = 0; i < numParts; i++ )
    {
        parts[i] = mp.parts[i];
        partTypes[i] = mp.partTypes[i];
    }
    points = new Point[numPoints];
    zArray = new Double[numPoints];
    mArray = new Double[numPoints];
    for( i = 0; i < numPoints; i++ )
    {
        points[i] = mp.points[i];
        zArray[i] = mp.zArray[i];
        if( mp.mArray != 0L )
            mArray[i] = mp.mArray[i];
    }
}

MultiPatch::~MultiPatch()
{
    delete [] parts;
    delete [] partTypes;
    delete [] points;
    delete [] zArray;
    delete [] mArray;
}

bool MultiPatch::read( int fd )
{
    RecordHeader rh;
    if( rh.read(fd) == false )
        return false;

    SAFE_DELETE_ARRAY( parts );
    SAFE_DELETE_ARRAY( partTypes );
    SAFE_DELETE_ARRAY( points );
    SAFE_DELETE_ARRAY( zArray );
    SAFE_DELETE_ARRAY( mArray );

    Integer shapeType;
    if( readVal<Integer>(fd, shapeType, LittleEndian ) == false )
        return false;

    if( shapeType != ShapeTypeMultiPatch )
        return false;

    if( bbox.read(fd) == false )
        return false;

    if( readVal<Integer>(fd, numParts, LittleEndian ) == false )
        return false;

    if( readVal<Integer>(fd, numPoints, LittleEndian ) == false )
        return false;

    parts  = new Integer[numParts];
    int i;
    for( i = 0; i < numParts; i++ )
    {
        if( readVal<Integer>(fd, parts[i], LittleEndian ) == false )
            return false;
    }

    partTypes = new Integer[numParts];
    for( i = 0; i < numParts; i++ )
    {
        if( readVal<Integer>(fd, partTypes[i], LittleEndian ) == false )
            return false;
    }

    points = new struct Point[numPoints];
    for( i = 0; i < numPoints; i++ )
    {
        if( points[i].read(fd ) == false )
            return false;
    }

    if( zRange.read(fd) == false )
        return false;

    zArray = new Double[numPoints];
    for( i = 0; i < numPoints; i++ )
    {
        if( readVal<Double>(fd, zArray[i], LittleEndian ) == false )
            return false;
    }

    int  W = 44 + (4*numParts);
    int  X = W + (4 * numParts);
    int  Y = X + (16 *numPoints);
    int  Z = Y + 16 + (8 *numPoints);
    if( rh.contentLength*2 > Z )
    {
        if( mRange.read(fd) == false )
            return false;

        mArray = new Double[numPoints];
        for( i = 0; i < numPoints; i++ )
        {
            if( readVal<Double>(fd, mArray[i], LittleEndian ) == false )
                return false;
        }
    }

    return true;
}


