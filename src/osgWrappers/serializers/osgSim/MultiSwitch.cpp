#include <osgSim/MultiSwitch>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkValues( const osgSim::MultiSwitch& node )
{
    return node.getSwitchSetList().size()>0;
}

static bool readValues( osgDB::InputStream& is, osgSim::MultiSwitch& node )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        is >> is.PROPERTY("SwitchSet");
        unsigned int valueSize = is.readSize(); is >> is.BEGIN_BRACKET;

        osgSim::MultiSwitch::ValueList values;
        for ( unsigned int j=0; j<valueSize; ++j )
        {
            bool value; is >> value;
            values.push_back( value );
        }
        node.setValueList( i, values );
        is >> is.END_BRACKET;
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeValues( osgDB::OutputStream& os, const osgSim::MultiSwitch& node )
{
    const osgSim::MultiSwitch::SwitchSetList& switches = node.getSwitchSetList();
    os.writeSize( switches.size() ); os << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<switches.size(); ++i )
    {
        const osgSim::MultiSwitch::ValueList& values = node.getValueList(i);
        os << os.PROPERTY("SwitchSet"); os.writeSize( values.size() );
        os << os.BEGIN_BRACKET << std::endl;
        for ( osgSim::MultiSwitch::ValueList::const_iterator itr=values.begin();
              itr!=values.end(); ++itr )
        {
            os << *itr << std::endl;
        }
        os << os.END_BRACKET << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgSim_MultiSwitch,
                         new osgSim::MultiSwitch,
                         osgSim::MultiSwitch,
                         "osg::Object osg::Node osg::Group osgSim::MultiSwitch" )
{
    ADD_BOOL_SERIALIZER( NewChildDefaultValue, true );  // _newChildDefaultValue
    ADD_UINT_SERIALIZER( ActiveSwitchSet, 0 );  // _activeSwitchSet
    ADD_USER_SERIALIZER( Values );  // _values
}
