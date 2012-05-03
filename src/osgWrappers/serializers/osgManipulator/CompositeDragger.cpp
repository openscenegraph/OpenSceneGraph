#include <osgManipulator/Dragger>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkDraggers( const osgManipulator::CompositeDragger& dragger )
{
    return dragger.getNumDraggers()>0;
}

static bool readDraggers( osgDB::InputStream& is, osgManipulator::CompositeDragger& dragger )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osgManipulator::Dragger* child = dynamic_cast<osgManipulator::Dragger*>( is.readObject() );
        if ( child ) dragger.addDragger( child );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeDraggers( osgDB::OutputStream& os, const osgManipulator::CompositeDragger& dragger )
{
    unsigned int size = dragger.getNumDraggers();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << dragger.getDragger(i);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgManipulator_CompositeDragger,
                         /*new osgManipulator::CompositeDragger*/NULL,
                         osgManipulator::CompositeDragger,
                         "osg::Object osg::Node osg::Transform osg::MatrixTransform osgManipulator::Dragger "
                         "osgManipulator::CompositeDragger" )
{
    ADD_USER_SERIALIZER( Draggers );  // _draggerList
}
