#include <osgGA/KeySwitchMatrixManipulator>

#define OBJECT_CAST dynamic_cast

#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkKeyManipMap( const osgGA::KeySwitchMatrixManipulator& kwmm )
{
    return !kwmm.getKeyManipMap().empty();
}

static bool readKeyManipMap( osgDB::InputStream& is, osgGA::KeySwitchMatrixManipulator& kwmm )
{
    int activeCameraManipulatorIndex = -1; is>>activeCameraManipulatorIndex;
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        int key; is >> key;
        std::string name; is >> name;
        
        osg::ref_ptr<osg::Object> obj = is.readObject();
        osgGA::CameraManipulator* child = dynamic_cast<osgGA::CameraManipulator*>( obj.get() );
        if ( child ) kwmm.addMatrixManipulator( key, name, child );
    }

    if (activeCameraManipulatorIndex>=0) kwmm.selectMatrixManipulator(activeCameraManipulatorIndex);
    
    return true;
}

static bool writeKeyManipMap( osgDB::OutputStream& os, const osgGA::KeySwitchMatrixManipulator& kwmm )
{
    const osgGA::KeySwitchMatrixManipulator::KeyManipMap& kmm = kwmm.getKeyManipMap();    
    unsigned int size = kmm.size();

    // find out what num the acive camera manipulator is
    const osgGA::CameraManipulator* cm = kwmm.getCurrentMatrixManipulator();
    int index = 0;
    int activeCameraManipulatorIndex = -1;
    for ( osgGA::KeySwitchMatrixManipulator::KeyManipMap::const_iterator itr = kmm.begin();
          itr != kmm.end();
          ++itr, ++index)
    {
        if (itr->second.second==cm)
        {
            activeCameraManipulatorIndex = index;
            break;
        }
    }

    os << activeCameraManipulatorIndex;
    os << size << os.BEGIN_BRACKET << std::endl;

    for ( osgGA::KeySwitchMatrixManipulator::KeyManipMap::const_iterator itr = kmm.begin();
          itr != kmm.end();
          ++itr)
    {
        os << itr->first;
        os << itr->second.first;
        os.writeObject(itr->second.second.get());
    }
    os << os.END_BRACKET << std::endl;

    return true;
}

REGISTER_OBJECT_WRAPPER( osgGA_KeySwitchMatrixManipulator,
                         new osgGA::KeySwitchMatrixManipulator,
                         osgGA::KeySwitchMatrixManipulator,
                         "osg::Object osgGA::KeySwitchMatrixManipulator" )
{
    ADD_USER_SERIALIZER( KeyManipMap );  // _children
}
