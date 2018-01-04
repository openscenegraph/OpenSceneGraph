#include <osg/Image>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( Image,
                         new osg::Image,
                         osg::Image,
                         "osg::Object osg::BufferData osg::Image" )
{
    {
         UPDATE_TO_VERSION_SCOPED( 154 )
         ADDED_ASSOCIATE("osg::BufferData")
    }

    {
        UPDATE_TO_VERSION_SCOPED( 112 )

        ADD_STRING_SERIALIZER(FileName, "");

        BEGIN_ENUM_SERIALIZER( WriteHint, NO_PREFERENCE );
            ADD_ENUM_VALUE( NO_PREFERENCE ) ;
            ADD_ENUM_VALUE( STORE_INLINE ) ;
            ADD_ENUM_VALUE( EXTERNAL_FILE );
        END_ENUM_SERIALIZER();

        BEGIN_ENUM_SERIALIZER( AllocationMode, USE_NEW_DELETE );
            ADD_ENUM_VALUE( NO_DELETE ) ;
            ADD_ENUM_VALUE( USE_NEW_DELETE );
            ADD_ENUM_VALUE( USE_MALLOC_FREE );
        END_ENUM_SERIALIZER();

        // Everything is done in OutputStream and InputStream classes
        ADD_GLENUM_SERIALIZER( InternalTextureFormat, GLint, GL_NONE );
        ADD_GLENUM_SERIALIZER( DataType, GLenum, GL_NONE );
        ADD_GLENUM_SERIALIZER( PixelFormat, GLenum, GL_NONE );
        ADD_INT_SERIALIZER( RowLength, 0 );
        ADD_UINT_SERIALIZER( Packing, 0 );

        BEGIN_ENUM_SERIALIZER( Origin, BOTTOM_LEFT );
            ADD_ENUM_VALUE( BOTTOM_LEFT ) ;
            ADD_ENUM_VALUE( TOP_LEFT );
        END_ENUM_SERIALIZER();
    }
}
