#include <osgUI/FrameSettings>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>



REGISTER_OBJECT_WRAPPER( FrameSettings,
                         new osgUI::FrameSettings,
                         osgUI::FrameSettings,
                         "osg::Object osgUI::FrameSettings" )
{
    BEGIN_ENUM_SERIALIZER2( Shape, osgUI::FrameSettings::Shape, NO_FRAME );
        ADD_ENUM_VALUE( NO_FRAME );
        ADD_ENUM_VALUE( BOX );
        ADD_ENUM_VALUE( PANEL );
    END_ENUM_SERIALIZER();

    BEGIN_ENUM_SERIALIZER2( Shadow, osgUI::FrameSettings::Shadow, PLAIN );
        ADD_ENUM_VALUE( PLAIN );
        ADD_ENUM_VALUE( SUNKEN );
        ADD_ENUM_VALUE( RAISED );
    END_ENUM_SERIALIZER();

    ADD_FLOAT_SERIALIZER( LineWidth, 0.01);
}
