#include <osg/Shape>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

// _columns, _rows
static bool checkArea( const osg::HeightField& shape )
{
    return true;
}

static bool readArea( osgDB::InputStream& is, osg::HeightField& shape )
{
    unsigned int numCols, numRows;
    is >> numCols >> numRows;
    shape.allocate( numCols, numRows );
    return true;
}

static bool writeArea( osgDB::OutputStream& os, const osg::HeightField& shape )
{
    os << shape.getNumColumns() << shape.getNumRows() << std::endl;
    return true;
}

// _heights
static bool checkHeights( const osg::HeightField& shape )
{
    return shape.getFloatArray()!=NULL;
}

static bool readHeights( osgDB::InputStream& is, osg::HeightField& shape )
{
    osg::FloatArray* array = dynamic_cast<osg::FloatArray*>( is.readArray() );
    if ( array )
    {
        unsigned int numCols = shape.getNumColumns(), numRows = shape.getNumRows();
        if ( array->size()<numRows*numCols ) return false;

        unsigned int index = 0;
        for ( unsigned int r=0; r<numRows; ++r )
        {
            for ( unsigned int c=0; c<numCols; ++c )
                shape.setHeight( c, r, (*array)[index++] );
        }
    }
    return true;
}

static bool writeHeights( osgDB::OutputStream& os, const osg::HeightField& shape )
{
    os.writeArray( shape.getFloatArray() );
    return true;
}

REGISTER_OBJECT_WRAPPER( HeightField,
                         new osg::HeightField,
                         osg::HeightField,
                         "osg::Object osg::Shape osg::HeightField" )
{
    ADD_USER_SERIALIZER( Area );  // _columns, _rows
    ADD_VEC3_SERIALIZER( Origin, osg::Vec3() );  // _origin
    ADD_FLOAT_SERIALIZER( XInterval, 0.0f );  // _dx
    ADD_FLOAT_SERIALIZER( YInterval, 0.0f );  // _dy
    ADD_FLOAT_SERIALIZER( SkirtHeight, 0.0f );  // _skirtHeight
    ADD_UINT_SERIALIZER( BorderWidth, 0 );  // _borderWidth
    ADD_QUAT_SERIALIZER( Rotation, osg::Quat() );  // _rotation
    ADD_USER_SERIALIZER( Heights );  // _heights
}
