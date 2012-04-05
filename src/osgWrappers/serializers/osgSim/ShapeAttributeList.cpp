#include <osgSim/ShapeAttribute>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkAttributes( const osgSim::ShapeAttributeList& list )
{
    return list.size()>0;
}

static bool readAttributes( osgDB::InputStream& is, osgSim::ShapeAttributeList& list )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        std::string name; int type;
        is >> is.PROPERTY("ShapeAttribute") >> name;
        is >> is.PROPERTY("Type") >> type;
        switch ( type )
        {
        case osgSim::ShapeAttribute::INTEGER:
            {
                int value; is >> value;
                list.push_back( osgSim::ShapeAttribute(name.c_str(), value) );
            }
            break;
        case osgSim::ShapeAttribute::DOUBLE:
            {
                double value; is >> value;
                list.push_back( osgSim::ShapeAttribute(name.c_str(), value) );
            }
            break;
        default:
            {
                std::string value; is >> value;
                list.push_back( osgSim::ShapeAttribute(name.c_str(), value.c_str()) );
            }
            break;
        }
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeAttributes( osgDB::OutputStream& os, const osgSim::ShapeAttributeList& list )
{
    unsigned int size = list.size();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        const osgSim::ShapeAttribute& sa = list[i];
        os << os.PROPERTY("ShapeAttribute") << sa.getName();
        os << os.PROPERTY("Type") << (int)sa.getType();
        switch ( sa.getType() )
        {
        case osgSim::ShapeAttribute::INTEGER: os << sa.getInt() << std::endl; break;
        case osgSim::ShapeAttribute::DOUBLE: os << sa.getDouble() << std::endl; break;
        default: os << std::string(sa.getString()) << std::endl; break;
        }
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgSim_ShapeAttributeList,
                         new osgSim::ShapeAttributeList,
                         osgSim::ShapeAttributeList,
                         "osg::Object osgSim::ShapeAttributeList" )
{
    ADD_USER_SERIALIZER( Attributes );
}
