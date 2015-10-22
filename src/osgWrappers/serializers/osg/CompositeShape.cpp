#include <osg/Shape>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkChildren( const osg::CompositeShape& shape )
{
    return shape.getNumChildren()>0;
}

static bool readChildren( osgDB::InputStream& is, osg::CompositeShape& shape )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osg::ref_ptr<osg::Shape> child = is.readObjectOfType<osg::Shape>();
        if ( child ) shape.addChild( child );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeChildren( osgDB::OutputStream& os, const osg::CompositeShape& shape )
{
    unsigned int size = shape.getNumChildren();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << shape.getChild(i);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( CompositeShape,
                         new osg::CompositeShape,
                         osg::CompositeShape,
                         "osg::Object osg::Shape osg::CompositeShape" )
{
    ADD_OBJECT_SERIALIZER( Shape, osg::Shape, NULL );  // _shape
    ADD_USER_SERIALIZER( Children );  //_children
}
