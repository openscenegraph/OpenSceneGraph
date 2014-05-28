#include <osg/TransferFunction>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#if 0

static bool checkColorMap( const osg::TransferFunction1D& func )
{
    return func.getColorMap().size()>0;
}

static bool readColorMap( osgDB::InputStream& is, osg::TransferFunction1D& func )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        float key = 0.0f;
        osg::Vec4d value;
        is >> key >> value;
        func.setColor( key, value );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeColorMap( osgDB::OutputStream& os, const osg::TransferFunction1D& func )
{
    const osg::TransferFunction1D::ColorMap& map = func.getColorMap();
    os.writeSize(map.size()); os << os.BEGIN_BRACKET << std::endl;
    for ( osg::TransferFunction1D::ColorMap::const_iterator itr=map.begin();
          itr!=map.end(); ++itr )
    {
        os << itr->first << itr->second << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}
#endif


REGISTER_OBJECT_WRAPPER( TransferFunction1D,
                         new osg::TransferFunction1D,
                         osg::TransferFunction1D,
                         "osg::Object osg::TransferFunction osg::TransferFunction1D" )
{
#if 0
    ADD_USER_SERIALIZER( ColorMap );  // _colorMap
#else
    ADD_MAP_SERIALIZER(ColorMap, osg::TransferFunction1D::ColorMap, osgDB::BaseSerializer::RW_FLOAT, osgDB::BaseSerializer::RW_VEC4F);
#endif
}
