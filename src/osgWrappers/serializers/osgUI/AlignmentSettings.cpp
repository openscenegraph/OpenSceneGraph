#include <osgUI/AlignmentSettings>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>



REGISTER_OBJECT_WRAPPER( AlignmentSettings,
                         new osgUI::AlignmentSettings,
                         osgUI::AlignmentSettings,
                         "osg::Object osgUI::AlignmentSettings" )
{
    BEGIN_ENUM_SERIALIZER2( Alignment, osgUI::AlignmentSettings::Alignment, LEFT_BOTTOM );
        ADD_ENUM_VALUE( LEFT_TOP );
        ADD_ENUM_VALUE( LEFT_CENTER );
        ADD_ENUM_VALUE( LEFT_BOTTOM );
        ADD_ENUM_VALUE( CENTER_TOP );
        ADD_ENUM_VALUE( CENTER_CENTER );
        ADD_ENUM_VALUE( CENTER_BOTTOM );
        ADD_ENUM_VALUE( RIGHT_TOP );
        ADD_ENUM_VALUE( RIGHT_CENTER );
        ADD_ENUM_VALUE( RIGHT_BOTTOM );
        ADD_ENUM_VALUE( LEFT_BASE_LINE );
        ADD_ENUM_VALUE( CENTER_BASE_LINE );
        ADD_ENUM_VALUE( RIGHT_BASE_LINE );
        ADD_ENUM_VALUE( LEFT_BOTTOM_BASE_LINE );
        ADD_ENUM_VALUE( CENTER_BOTTOM_BASE_LINE );
        ADD_ENUM_VALUE( RIGHT_BOTTOM_BASE_LINE );
    END_ENUM_SERIALIZER();  // _alignment
}
