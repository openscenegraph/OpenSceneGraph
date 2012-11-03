#include <osgGA/GUIEventAdapter>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( osgGA_GUIEventAdapter,
                         new osgGA::GUIEventAdapter,
                         osgGA::GUIEventAdapter,
                         "osg::Object osgGA::GUIEventAdapter" )
{
    ADD_DOUBLE_SERIALIZER( Time, 0.0 );

    BEGIN_ENUM_SERIALIZER( EventType, NONE );
        ADD_ENUM_VALUE( NONE );
        ADD_ENUM_VALUE( PUSH );
        ADD_ENUM_VALUE( RELEASE );
        ADD_ENUM_VALUE( DOUBLECLICK );
        ADD_ENUM_VALUE( DRAG );
        ADD_ENUM_VALUE( MOVE );
        ADD_ENUM_VALUE( KEYDOWN );
        ADD_ENUM_VALUE( KEYUP );
        ADD_ENUM_VALUE( FRAME );
        ADD_ENUM_VALUE( RESIZE );
        ADD_ENUM_VALUE( SCROLL );
        ADD_ENUM_VALUE( PEN_PRESSURE );
        ADD_ENUM_VALUE( PEN_ORIENTATION );
        ADD_ENUM_VALUE( PEN_PROXIMITY_ENTER );
        ADD_ENUM_VALUE( PEN_PROXIMITY_LEAVE );
        ADD_ENUM_VALUE( CLOSE_WINDOW );
        ADD_ENUM_VALUE( QUIT_APPLICATION );
        ADD_ENUM_VALUE( USER );
    END_ENUM_SERIALIZER();

    ADD_INT_SERIALIZER( Key, 0 );
    ADD_INT_SERIALIZER( UnmodifiedKey, 0 );
    ADD_INT_SERIALIZER( ModKeyMask, 0 ) ;

    BEGIN_ENUM_SERIALIZER( MouseYOrientation, Y_INCREASING_DOWNWARDS );
        ADD_ENUM_VALUE( Y_INCREASING_UPWARDS );
        ADD_ENUM_VALUE( Y_INCREASING_DOWNWARDS );
    END_ENUM_SERIALIZER();

    ADD_FLOAT_SERIALIZER( Xmin, -1.0f );
    ADD_FLOAT_SERIALIZER( Xmax, 1.0f );
    ADD_FLOAT_SERIALIZER( Ymin, -1.0f );
    ADD_FLOAT_SERIALIZER( Ymax, 1.0f );

    ADD_FLOAT_SERIALIZER( X, 0.0f );
    ADD_FLOAT_SERIALIZER( Y, 0.0f );
    ADD_INT_SERIALIZER( Button, 0 );
    ADD_INT_SERIALIZER( ButtonMask, 0 );

    BEGIN_ENUM_SERIALIZER( ScrollingMotion, SCROLL_NONE);
        ADD_ENUM_VALUE( SCROLL_NONE );
        ADD_ENUM_VALUE( SCROLL_LEFT );
        ADD_ENUM_VALUE( SCROLL_RIGHT );
        ADD_ENUM_VALUE( SCROLL_UP );
        ADD_ENUM_VALUE( SCROLL_DOWN );
        ADD_ENUM_VALUE( SCROLL_2D );
    END_ENUM_SERIALIZER();

    ADD_FLOAT_SERIALIZER( ScrollingDeltaX, 0.0f);
    ADD_FLOAT_SERIALIZER( ScrollingDeltaY, 0.0f);

}
