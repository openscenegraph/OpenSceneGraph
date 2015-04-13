#include <osgManipulator/Constraint>
#include <osgManipulator/Dragger>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

// TransformUpdating
static bool checkTransformUpdating( const osgManipulator::Dragger& dragger )
{
    return dragger.getDraggerCallbacks().size()>0;
}

static bool readTransformUpdating( osgDB::InputStream& is, osgManipulator::Dragger& dragger )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        std::string name; is >> name >> is.BEGIN_BRACKET;
        if ( name=="DraggerTransformCallback" )
        {
            osg::MatrixTransform* transform = dynamic_cast<osg::MatrixTransform*>( is.readObject() );
            if ( transform ) dragger.addTransformUpdating( transform );
        }
        is >> is.END_BRACKET;
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeTransformUpdating( osgDB::OutputStream& os, const osgManipulator::Dragger& dragger )
{
    const osgManipulator::Dragger::DraggerCallbacks& callbacks = dragger.getDraggerCallbacks();
    os.writeSize( callbacks.size() ); os << os.BEGIN_BRACKET << std::endl;
    for ( osgManipulator::Dragger::DraggerCallbacks::const_iterator itr=callbacks.begin();
          itr!=callbacks.end(); ++itr )
    {
        osgManipulator::DraggerTransformCallback* dtcb =
            dynamic_cast<osgManipulator::DraggerTransformCallback*>( itr->get() );
        if ( dtcb )
        {
            os << std::string("DraggerTransformCallback") << os.BEGIN_BRACKET << std::endl;
            os << dtcb->getTransform();
        }
        else
        {
            os << std::string("DraggerCallback") << os.BEGIN_BRACKET << std::endl;
        }
        os << os.END_BRACKET << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

// DefaultGeometry: FIXME - add a setUseDefaultGeometry(bool) here?
static bool checkDefaultGeometry( const osgManipulator::Dragger& dragger )
{ return true; }

static bool readDefaultGeometry( osgDB::InputStream& is, osgManipulator::Dragger& dragger )
{
    bool useDefGeometry = false; is >> useDefGeometry;
    dragger.setupDefaultGeometry();
    return true;
}

static bool writeDefaultGeometry( osgDB::OutputStream& os, const osgManipulator::Dragger& dragger )
{
    os << true << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgManipulator_Dragger,
                         /*new osgManipulator::Dragger*/NULL,
                         osgManipulator::Dragger,
                         "osg::Object osg::Node osg::Transform osg::MatrixTransform osgManipulator::Dragger" )
{
    // Dragger should not record children separately, so ignore the osg::Group class wrapper

    ADD_BOOL_SERIALIZER( HandleEvents, false );  // _handleEvents
    ADD_BOOL_SERIALIZER( DraggerActive, false );  // _draggerActive
    ADD_UINT_SERIALIZER( ActivationModKeyMask, 0 );  // _activationModKeyMask
    ADD_INT_SERIALIZER( ActivationKeyEvent, 0 );  // _activationKeyEvent
    ADD_USER_SERIALIZER( TransformUpdating );  // _draggerCallbacks
    ADD_USER_SERIALIZER( DefaultGeometry );
}
