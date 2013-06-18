#include <osg/Geode>
#include <osg/Geometry>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkDrawables( const osg::Geode& node )
{
    return node.getNumDrawables()>0;
}

static bool readDrawables( osgDB::InputStream& is, osg::Geode& node )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osg::Drawable* drawable = dynamic_cast<osg::Drawable*>( is.readObject() );
        if ( drawable ) 
        {
            node.addDrawable( drawable );
        }
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeDrawables( osgDB::OutputStream& os, const osg::Geode& node )
{
    unsigned int size = node.getNumDrawables();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << node.getDrawable(i);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( Geode,
                         new osg::Geode,
                         osg::Geode,
                         "osg::Object osg::Node osg::Geode" )
{
    ADD_USER_SERIALIZER( Drawables );  // _drawables
}
