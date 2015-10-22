#include <osg/Camera>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

#ifndef GL_ACCUM_BUFFER_BIT
    #define GL_ACCUM_BUFFER_BIT 0x00000200
#endif

BEGIN_USER_TABLE( RenderOrder, osg::Camera );
    ADD_USER_VALUE( PRE_RENDER );
    ADD_USER_VALUE( NESTED_RENDER );
    ADD_USER_VALUE( POST_RENDER );
END_USER_TABLE()

USER_READ_FUNC( RenderOrder, readOrderValue )
USER_WRITE_FUNC( RenderOrder, writeOrderValue )

BEGIN_USER_TABLE( BufferComponent, osg::Camera );
    ADD_USER_VALUE( DEPTH_BUFFER );
    ADD_USER_VALUE( STENCIL_BUFFER );
    ADD_USER_VALUE( PACKED_DEPTH_STENCIL_BUFFER );
    ADD_USER_VALUE( COLOR_BUFFER );
    ADD_USER_VALUE( COLOR_BUFFER0 );
    ADD_USER_VALUE( COLOR_BUFFER1 );
    ADD_USER_VALUE( COLOR_BUFFER2 );
    ADD_USER_VALUE( COLOR_BUFFER3 );
    ADD_USER_VALUE( COLOR_BUFFER4 );
    ADD_USER_VALUE( COLOR_BUFFER5 );
    ADD_USER_VALUE( COLOR_BUFFER6 );
    ADD_USER_VALUE( COLOR_BUFFER7 );
    ADD_USER_VALUE( COLOR_BUFFER8 );
    ADD_USER_VALUE( COLOR_BUFFER9 );
    ADD_USER_VALUE( COLOR_BUFFER10 );
    ADD_USER_VALUE( COLOR_BUFFER11 );
    ADD_USER_VALUE( COLOR_BUFFER12 );
    ADD_USER_VALUE( COLOR_BUFFER13 );
    ADD_USER_VALUE( COLOR_BUFFER14 );
    ADD_USER_VALUE( COLOR_BUFFER15 );
END_USER_TABLE()

USER_READ_FUNC( BufferComponent, readBufferComponent )
USER_WRITE_FUNC( BufferComponent, writeBufferComponent )

static osg::Camera::Attachment readBufferAttachment( osgDB::InputStream& is )
{
    osg::Camera::Attachment attachment;
    char type = -1; is >> is.PROPERTY("Type") >> type;
    if ( type==0 )
    {
        is >> is.PROPERTY("InternalFormat") >> attachment._internalFormat;
        return attachment;
    }
    else if ( type==1 )
    {
        is >> is.PROPERTY("Image");
        attachment._image = is.readObjectOfType<osg::Image>();
    }
    else if ( type==2 )
    {
        is >> is.PROPERTY("Texture");
        attachment._texture = is.readObjectOfType<osg::Texture>();
        is >> is.PROPERTY("Level") >> attachment._level;
        is >> is.PROPERTY("Face") >> attachment._face;
        is >> is.PROPERTY("MipMapGeneration") >> attachment._mipMapGeneration;
    }
    else
        return attachment;

    is >> is.PROPERTY("MultisampleSamples") >> attachment._multisampleSamples;
    is >> is.PROPERTY("MultisampleColorSamples") >> attachment._multisampleColorSamples;
    return attachment;
}

static void writeBufferAttachment( osgDB::OutputStream& os, const osg::Camera::Attachment& attachment )
{
    os << os.PROPERTY("Type");
    if ( attachment._internalFormat!=GL_NONE )
    {
        os << (char)0 << std::endl;
        os << os.PROPERTY("InternalFormat") << GLENUM(attachment._internalFormat) << std::endl;
        return;
    }
    else if ( attachment._image.valid() )
    {
        os << (char)1 << std::endl;
        os << os.PROPERTY("Image") << attachment._image.get();
    }
    else if ( attachment._texture.valid() )
    {
        os << (char)2 << std::endl;
        os << os.PROPERTY("Texture") << attachment._texture.get();
        os << os.PROPERTY("Level") << attachment._level << std::endl;
        os << os.PROPERTY("Face") << attachment._face << std::endl;
        os << os.PROPERTY("MipMapGeneration") << attachment._mipMapGeneration << std::endl;
    }
    else
    {
        os << (char)-1 << std::endl;
        return;
    }

    os << os.PROPERTY("MultisampleSamples") << attachment._multisampleSamples << std::endl;
    os << os.PROPERTY("MultisampleColorSamples") << attachment._multisampleColorSamples << std::endl;
}

// _renderOrder & _renderOrderNum
static bool checkRenderOrder( const osg::Camera& node )
{
    return true;
}

static bool readRenderOrder( osgDB::InputStream& is, osg::Camera& node )
{
    int order = readOrderValue(is);
    int orderNumber = 0; is >> orderNumber;
    node.setRenderOrder( static_cast<osg::Camera::RenderOrder>(order), orderNumber );
    return true;
}

static bool writeRenderOrder( osgDB::OutputStream& os, const osg::Camera& node )
{
    writeOrderValue( os, (int)node.getRenderOrder() );
    os << node.getRenderOrderNum() << std::endl;
    return true;
}

// _bufferAttachmentMap
static bool checkBufferAttachmentMap( const osg::Camera& node )
{
    return node.getBufferAttachmentMap().size()>0;
}

static bool readBufferAttachmentMap( osgDB::InputStream& is, osg::Camera& node )
{
    unsigned int size = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        is >> is.PROPERTY("Attachment");
        osg::Camera::BufferComponent bufferComponent =
            static_cast<osg::Camera::BufferComponent>( readBufferComponent(is) );
        is >> is.BEGIN_BRACKET;
        osg::Camera::Attachment attachment = readBufferAttachment(is);
        is >> is.END_BRACKET;

        if ( attachment._internalFormat!=GL_NONE )
        {
            node.attach( bufferComponent, attachment._internalFormat );
        }
        else if ( attachment._image.valid() )
        {
            node.attach( bufferComponent, attachment._image.get(),
                         attachment._multisampleSamples, attachment._multisampleColorSamples );
        }
        else if ( attachment._texture.valid() )
        {
            node.attach( bufferComponent, attachment._texture.get(),
                         attachment._level, attachment._face, attachment._mipMapGeneration,
                         attachment._multisampleSamples, attachment._multisampleColorSamples );
        }
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeBufferAttachmentMap( osgDB::OutputStream& os, const osg::Camera& node )
{
    const osg::Camera::BufferAttachmentMap& map = node.getBufferAttachmentMap();
    os.writeSize(map.size()); os<< os.BEGIN_BRACKET << std::endl;
    for ( osg::Camera::BufferAttachmentMap::const_iterator itr=map.begin();
          itr!=map.end(); ++itr )
    {
        os << os.PROPERTY("Attachment"); writeBufferComponent( os, itr->first );
        os << os.BEGIN_BRACKET << std::endl;
        writeBufferAttachment( os, itr->second );
        os << os.END_BRACKET << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( Camera,
                         new osg::Camera,
                         osg::Camera,
                         "osg::Object osg::Node osg::Group osg::Transform osg::Camera" )
{
    ADD_BOOL_SERIALIZER( AllowEventFocus, true );  // _allowEventFocus
    BEGIN_BITFLAGS_SERIALIZER(ClearMask,GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
        ADD_BITFLAG_VALUE(COLOR, GL_COLOR_BUFFER_BIT);
        ADD_BITFLAG_VALUE(DEPTH, GL_DEPTH_BUFFER_BIT);
        ADD_BITFLAG_VALUE(ACCUM, GL_ACCUM_BUFFER_BIT);
        ADD_BITFLAG_VALUE(STENCIL, GL_STENCIL_BUFFER_BIT);
    END_BITFLAGS_SERIALIZER();
    ADD_VEC4_SERIALIZER( ClearColor, osg::Vec4() );  // _clearColor
    ADD_VEC4_SERIALIZER( ClearAccum, osg::Vec4() );  // _clearAccum
    ADD_DOUBLE_SERIALIZER( ClearDepth, 1.0 );  // _clearDepth
    ADD_INT_SERIALIZER( ClearStencil, 0 );  // _clearStencil
    ADD_OBJECT_SERIALIZER( ColorMask, osg::ColorMask, NULL );  // _colorMask
    ADD_OBJECT_SERIALIZER( Viewport, osg::Viewport, NULL );  // _viewport

    BEGIN_ENUM_SERIALIZER( TransformOrder, PRE_MULTIPLY );
        ADD_ENUM_VALUE( PRE_MULTIPLY );
        ADD_ENUM_VALUE( POST_MULTIPLY );
    END_ENUM_SERIALIZER();  // _transformOrder

    BEGIN_ENUM_SERIALIZER( ProjectionResizePolicy, HORIZONTAL );
        ADD_ENUM_VALUE( FIXED );
        ADD_ENUM_VALUE( HORIZONTAL );
        ADD_ENUM_VALUE( VERTICAL );
    END_ENUM_SERIALIZER();  // _projectionResizePolicy

    ADD_MATRIXD_SERIALIZER( ProjectionMatrix, osg::Matrixd() );  // _projectionMatrix
    ADD_MATRIXD_SERIALIZER( ViewMatrix, osg::Matrixd() );  // _viewMatrix
    ADD_USER_SERIALIZER( RenderOrder );  // _renderOrder & _renderOrderNum
    ADD_GLENUM_SERIALIZER( DrawBuffer, GLenum, GL_NONE );  // _drawBuffer
    ADD_GLENUM_SERIALIZER( ReadBuffer, GLenum, GL_NONE );  // _readBuffer

    BEGIN_ENUM_SERIALIZER( RenderTargetImplementation, FRAME_BUFFER );
        ADD_ENUM_VALUE( FRAME_BUFFER_OBJECT );
        ADD_ENUM_VALUE( PIXEL_BUFFER_RTT );
        ADD_ENUM_VALUE( PIXEL_BUFFER );
        ADD_ENUM_VALUE( FRAME_BUFFER );
        ADD_ENUM_VALUE( SEPARATE_WINDOW );
    END_ENUM_SERIALIZER();  // _renderTargetImplementation

    ADD_USER_SERIALIZER( BufferAttachmentMap );  // _bufferAttachmentMap
    ADD_OBJECT_SERIALIZER( InitialDrawCallback, osg::Camera::DrawCallback, NULL );  // _initialDrawCallback
    ADD_OBJECT_SERIALIZER( PreDrawCallback, osg::Camera::DrawCallback, NULL );  // _preDrawCallback
    ADD_OBJECT_SERIALIZER( PostDrawCallback, osg::Camera::DrawCallback, NULL );  // _postDrawCallback
    ADD_OBJECT_SERIALIZER( FinalDrawCallback, osg::Camera::DrawCallback, NULL );  // _finalDrawCallback

    {
        UPDATE_TO_VERSION_SCOPED( 123 )
        BEGIN_ENUM_SERIALIZER( InheritanceMaskActionOnAttributeSetting, DISABLE_ASSOCIATED_INHERITANCE_MASK_BIT );
            ADD_ENUM_VALUE( DISABLE_ASSOCIATED_INHERITANCE_MASK_BIT );
            ADD_ENUM_VALUE( DO_NOT_MODIFY_INHERITANCE_MASK );
        END_ENUM_SERIALIZER();

        BEGIN_INT_BITFLAGS_SERIALIZER(InheritanceMask, osg::Camera::ALL_VARIABLES);
            ADD_BITFLAG_VALUE(COMPUTE_NEAR_FAR_MODE, osg::Camera::COMPUTE_NEAR_FAR_MODE);
            ADD_BITFLAG_VALUE(CULLING_MODE, osg::Camera::CULLING_MODE);
            ADD_BITFLAG_VALUE(LOD_SCALE, osg::Camera::LOD_SCALE);
            ADD_BITFLAG_VALUE(SMALL_FEATURE_CULLING_PIXEL_SIZE, osg::Camera::SMALL_FEATURE_CULLING_PIXEL_SIZE);
            ADD_BITFLAG_VALUE(CLAMP_PROJECTION_MATRIX_CALLBACK, osg::Camera::CLAMP_PROJECTION_MATRIX_CALLBACK);
            ADD_BITFLAG_VALUE(NEAR_FAR_RATIO, osg::Camera::NEAR_FAR_RATIO);
            ADD_BITFLAG_VALUE(IMPOSTOR_ACTIVE, osg::Camera::IMPOSTOR_ACTIVE);
            ADD_BITFLAG_VALUE(DEPTH_SORT_IMPOSTOR_SPRITES, osg::Camera::DEPTH_SORT_IMPOSTOR_SPRITES);
            ADD_BITFLAG_VALUE(IMPOSTOR_PIXEL_ERROR_THRESHOLD, osg::Camera::IMPOSTOR_PIXEL_ERROR_THRESHOLD);
            ADD_BITFLAG_VALUE(NUM_FRAMES_TO_KEEP_IMPOSTORS_SPRITES, osg::Camera::NUM_FRAMES_TO_KEEP_IMPOSTORS_SPRITES);
            ADD_BITFLAG_VALUE(CULL_MASK, osg::Camera::CULL_MASK);
            ADD_BITFLAG_VALUE(CULL_MASK_LEFT, osg::Camera::CULL_MASK_LEFT);
            ADD_BITFLAG_VALUE(CULL_MASK_RIGHT, osg::Camera::CULL_MASK_RIGHT);
            ADD_BITFLAG_VALUE(CLEAR_COLOR, osg::Camera::CLEAR_COLOR);
            ADD_BITFLAG_VALUE(CLEAR_MASK, osg::Camera::CLEAR_MASK);
            ADD_BITFLAG_VALUE(LIGHTING_MODE, osg::Camera::LIGHTING_MODE);
            ADD_BITFLAG_VALUE(LIGHT, osg::Camera::LIGHT);
            ADD_BITFLAG_VALUE(DRAW_BUFFER, osg::Camera::DRAW_BUFFER);
            ADD_BITFLAG_VALUE(READ_BUFFER, osg::Camera::READ_BUFFER);
            ADD_BITFLAG_VALUE(NO_VARIABLES, osg::Camera::NO_VARIABLES);
            /** ADD_BITFLAG_VALUE(ALL_VARIABLES, osg::Camera::ALL_VARIABLES);*/
        END_BITFLAGS_SERIALIZER();
    }

    {
        UPDATE_TO_VERSION_SCOPED( 140 )

        BEGIN_INT_BITFLAGS_SERIALIZER( ImplicitBufferAttachmentRenderMask, osg::Camera::USE_DISPLAY_SETTINGS_MASK );
            ADD_BITFLAG_VALUE( IMPLICIT_DEPTH_BUFFER_ATTACHMENT, osg::Camera::IMPLICIT_DEPTH_BUFFER_ATTACHMENT );
            ADD_BITFLAG_VALUE( IMPLICIT_STENCIL_BUFFER_ATTACHMENT, osg::Camera::IMPLICIT_STENCIL_BUFFER_ATTACHMENT );
            ADD_BITFLAG_VALUE( IMPLICIT_COLOR_BUFFER_ATTACHMENT, osg::Camera::IMPLICIT_COLOR_BUFFER_ATTACHMENT );
            ADD_BITFLAG_VALUE( USE_DISPLAY_SETTINGS_MASK, osg::Camera::USE_DISPLAY_SETTINGS_MASK );
        END_BITFLAGS_SERIALIZER();

        BEGIN_INT_BITFLAGS_SERIALIZER( ImplicitBufferAttachmentResolveMask, osg::Camera::USE_DISPLAY_SETTINGS_MASK );
            ADD_BITFLAG_VALUE( IMPLICIT_DEPTH_BUFFER_ATTACHMENT, osg::Camera::IMPLICIT_DEPTH_BUFFER_ATTACHMENT );
            ADD_BITFLAG_VALUE( IMPLICIT_STENCIL_BUFFER_ATTACHMENT, osg::Camera::IMPLICIT_STENCIL_BUFFER_ATTACHMENT );
            ADD_BITFLAG_VALUE( IMPLICIT_COLOR_BUFFER_ATTACHMENT, osg::Camera::IMPLICIT_COLOR_BUFFER_ATTACHMENT );
            ADD_BITFLAG_VALUE( USE_DISPLAY_SETTINGS_MASK, osg::Camera::USE_DISPLAY_SETTINGS_MASK );
        END_BITFLAGS_SERIALIZER();
    }
}
