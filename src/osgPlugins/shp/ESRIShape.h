#ifndef OSG_SHAPE_H
#define OSG_SHAPE_H

#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif
#include <osg/Referenced>

namespace ESRIShape {

typedef int             Integer;
typedef short           Short;
typedef unsigned char   Byte;
typedef double          Double;
typedef unsigned char * BytePtr;

enum ByteOrder {
    LittleEndian,
    BigEndian
};

enum PartType{
    TriangleStrip   = 0,
    TriangleFan     = 1,
    OuterRing       = 2,
    InnerRing       = 3,
    FirstRing       = 4,
    Ring            = 5
};

enum ShapeType {
    ShapeTypeNullShape   = 0,
    ShapeTypePoint       = 1,
    ShapeTypePolyLine    = 3,
    ShapeTypePolygon     = 5,
    ShapeTypeMultiPoint  = 8,
    ShapeTypePointZ      = 11,
    ShapeTypePolyLineZ   = 13,
    ShapeTypePolygonZ    = 15,
    ShapeTypeMultiPointZ = 18,
    ShapeTypePointM      = 21,
    ShapeTypePolyLineM   = 23,
    ShapeTypePolygonM    = 25,
    ShapeTypeMultiPointM = 28,
    ShapeTypeMultiPatch  = 31
};


struct BoundingBox 
{
    Double Xmin;
    Double Ymin;
    Double Xmax;
    Double Ymax;
    Double Zmin;
    Double Zmax;
    Double Mmin;
    Double Mmax;

    bool read( int fd );

    void print();
};

///////////////

struct ShapeHeader 
{
    Integer fileCode;
    Byte _unused_0[20];
    Integer fileLength;
    Integer version;
    Integer shapeType;
    BoundingBox bbox;

    bool read(int fd);

    void print();
};

struct RecordHeader 
{
    Integer recordNumber;
    Integer contentLength;

    RecordHeader();

    bool read( int fd );

    void print();
};


struct NullRecord
{
    Integer shapeType;
    NullRecord();

    bool read( int fd );
};

//////////////////////////////////////////////////////////////////////

struct Box
{
    Double Xmin, Ymin, Xmax, Ymax;

    Box();
    Box(const Box &b );
    bool read( int fd );
};

struct Range {
    Double min, max;
    Range();
    Range( const Range &r );

    bool read( int fd );
};

struct ShapeObject : public osg::Referenced
{
    ShapeType shapeType;
    ShapeObject(ShapeType s);
    virtual ~ShapeObject();
};


struct Point : public ShapeObject
{
    Double x, y;

    Point();
    Point(const Point &p);
    virtual ~Point();

    bool read( int fd );
    void print();
};

struct PointRecord
{
    Point point;
    bool read( int fd );
};

struct MultiPoint: public ShapeObject
{
    Box     bbox;
    Integer numPoints;
    struct Point   *points;

    MultiPoint();

    MultiPoint( const struct MultiPoint &mpoint );

    virtual ~MultiPoint();

    bool read( int fd );

    void print();
};

struct PolyLine: public ShapeObject
{
    Box             bbox;
    Integer         numParts;
    Integer         numPoints;
    Integer         *parts;
    struct Point    *points;

    PolyLine();

    PolyLine( const PolyLine &p ); 

    virtual ~PolyLine();

    bool read( int fd );
};



struct Polygon : public ShapeObject
{
    Box             bbox;
    Integer         numParts;
    Integer         numPoints;
    Integer         *parts;
    Point           *points;

    Polygon();

    Polygon( const Polygon &p );

    virtual ~Polygon();


    bool read( int fd );
};

//////////////////////////////////////////////////////////////////////
struct PointM : public ShapeObject
{
    Double x, y, m;

    PointM();

    PointM(const PointM &p);

    virtual ~PointM();

    bool read( int fd );

    void print();
};

struct PointMRecord
{
    PointM pointM;

    bool read( int fd );
};


struct MultiPointM: public ShapeObject
{
    Box             bbox;
    Integer         numPoints;
    struct Point    *points;
    struct Range    mRange; 
    Double          *mArray;

    MultiPointM();

    MultiPointM( const struct MultiPointM &mpointm );

    virtual ~MultiPointM();

    bool read( int fd );

    void print();
};


struct PolyLineM: public ShapeObject
{
    Box             bbox;
    Integer         numParts;
    Integer         numPoints;
    Integer         *parts;
    struct Point    *points;
    struct Range    mRange; 
    Double          *mArray;

    PolyLineM();

    PolyLineM(const PolyLineM &p);

    virtual ~PolyLineM();

    bool read( int fd );
};


struct PolygonM : public ShapeObject
{
    Box             bbox;
    Integer         numParts;
    Integer         numPoints;
    Integer         *parts;
    Point           *points;
    struct Range    mRange; 
    Double          *mArray;

    PolygonM();

    PolygonM(const PolygonM &p);

    bool read( int fd );
};




//////////////////////////////////////////////////////////////////////




struct PointZ : public ShapeObject
{
    Double x, y, z, m;

    PointZ();
    PointZ(const PointZ &p);
    virtual ~PointZ();

    bool read( int fd );

    void print();
};

struct PointZRecord
{
    PointZ pointZ;
    bool read( int fd );
};

struct MultiPointZ: public ShapeObject
{
    Box             bbox;
    Integer         numPoints;
    struct Point    *points;
    struct Range    zRange; 
    Double          *zArray;
    struct Range    mRange; 
    Double          *mArray;

    MultiPointZ();

    MultiPointZ( const struct MultiPointZ &);

    virtual ~MultiPointZ();

    bool read( int fd );

    void print();
};



struct PolyLineZ: public ShapeObject
{
    Box             bbox;
    Integer         numParts;
    Integer         numPoints;
    Integer         *parts;
    struct Point    *points;
    struct Range    zRange; 
    Double          *zArray;
    struct Range    mRange; 
    Double          *mArray;

    PolyLineZ();

    PolyLineZ( const PolyLineZ &p ); 

    virtual ~PolyLineZ();

    bool read( int fd );
};


struct PolygonZ : public ShapeObject
{
    Box             bbox;
    Integer         numParts;
    Integer         numPoints;
    Integer         *parts;
    Point           *points;
    struct Range    zRange; 
    Double          *zArray;
    struct Range    mRange; 
    Double          *mArray;

    PolygonZ();

    PolygonZ( const PolygonZ &p );

    virtual ~PolygonZ();


    bool read( int fd );
};


//////////////////////////////////////////////////////////////////////


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
    bool read( int );
};

}

#endif
