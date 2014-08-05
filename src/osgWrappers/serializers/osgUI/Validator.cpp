#include <osgUI/Validator>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>


REGISTER_OBJECT_WRAPPER( Validator,
                         new osgUI::Validator,
                         osgUI::Validator,
                         "osg::Object osgUI::Validator" )
{
}
