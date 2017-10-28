/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2010 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/
// Written by Wang Rui, (C) 2010

#include <osg/Version>
#include <osg/Notify>
#include <osg/BlendFunc>
#include <osg/ClampColor>
#include <osg/Fog>
#include <osg/FragmentProgram>
#include <osg/GLExtensions>
#include <osg/PointSprite>
#include <osg/StateSet>
#include <osg/StencilTwoSided>
#include <osg/TexEnvCombine>
#include <osg/Texture>
#include <osg/TextureCubeMap>
#include <osg/TextureRectangle>
#include <osg/VertexProgram>

#include <osgDB/Options>
#include <osgDB/DataTypes>
#include <osgDB/ObjectWrapper>
#include <osgDB/Registry>

// pull in OSG headers to just introduce their GL defines for GL3/GLES compatibility
#include <osg/AlphaFunc>
#include <osg/Material>
#include <osg/LineStipple>
#include <osg/PolygonStipple>
#include <osg/Point>
#include <osg/TexGen>
#include <osg/ClipPlane>
#include <osg/Fog>
#include <osg/PolygonMode>
#include <osg/PolygonOffset>
#include <osg/Texture1D>
#include <osg/LogicOp>

#include <sstream>

#ifndef GL_PERSPECTIVE_CORRECTION_HINT
    #define GL_PERSPECTIVE_CORRECTION_HINT      0x0C50
#endif

#if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE) || defined(OSG_GLES3_AVAILABLE)
    #define GL_POLYGON_SMOOTH_HINT              0x0C53
    #define GL_LINE_SMOOTH_HINT                 0x0C52
    #define GL_FRAGMENT_SHADER_DERIVATIVE_HINT  0x8B8B
#endif

using namespace osgDB;

void osgDB::split( const std::string& src, StringList& list, char separator )
{
    std::string::size_type start = src.find_first_not_of(separator);
    while ( start!=std::string::npos )
    {
        std::string::size_type end = src.find_first_of(separator, start);
        if ( end!=std::string::npos )
        {
            list.push_back( std::string(src, start, end-start) );
            start = src.find_first_not_of(separator, end);
        }
        else
        {
            list.push_back( std::string(src, start, src.size()-start) );
            start = end;
        }
    }
}

void ObjectWrapper::splitAssociates( const std::string& src, ObjectWrapper::RevisionAssociateList& list, char separator )
{
    std::string::size_type start = src.find_first_not_of(separator);
    while ( start!=std::string::npos )
    {
        std::string::size_type end = src.find_first_of(separator, start);
        if ( end!=std::string::npos )
        {
            list.push_back( ObjectWrapperAssociate(std::string(src, start, end-start)) );
            start = src.find_first_not_of(separator, end);
        }
        else
        {
            list.push_back( ObjectWrapperAssociate(std::string(src, start, src.size()-start)) );
            start = end;
        }
    }
}
////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ObjectWrapper
//
ObjectWrapper::ObjectWrapper( CreateInstanceFunc* createInstanceFunc, const std::string& name,
                              const std::string& associates )
:   osg::Referenced(),
    _createInstanceFunc(createInstanceFunc), _name(name), _version(0),_isAssociatesRevisionsInheritanceDone(false)
{
    splitAssociates( associates, _associates );
}

ObjectWrapper::ObjectWrapper( CreateInstanceFunc* createInstanceFunc, const std::string& domain, const std::string& name,
                              const std::string& associates )
:   osg::Referenced(),
    _createInstanceFunc(createInstanceFunc), _domain(domain), _name(name), _version(0),_isAssociatesRevisionsInheritanceDone(false)
{
    splitAssociates( associates, _associates );
}

void ObjectWrapper::setupAssociatesRevisionsInheritanceIfRequired()
{
    if(!_isAssociatesRevisionsInheritanceDone)
    {
        ///for each associate wrapper
        for ( ObjectWrapper::RevisionAssociateList::const_iterator itr=_associates.begin(); itr!=_associates.end(); ++itr )
        {
            ObjectWrapper* assocWrapper = Registry::instance()->getObjectWrapperManager()->findWrapper(itr->_name);
            if ( assocWrapper && assocWrapper != this )
            {
                ///crawl association revisions in associates
                for ( ObjectWrapper::RevisionAssociateList::const_iterator itr2=assocWrapper->getAssociates().begin(); itr2!=assocWrapper->getAssociates().end(); ++itr2 )
                {
                    for ( ObjectWrapper::RevisionAssociateList::iterator itr3=_associates.begin(); itr3!=_associates.end(); ++itr3 )
                    {
                        ///they share associates
                        if(itr3->_name==itr2->_name)
                        {
                            itr3->_firstVersion=itr3->_firstVersion>itr2->_firstVersion? itr3->_firstVersion:itr2->_firstVersion;
                            itr3->_lastVersion=itr3->_lastVersion<itr2->_lastVersion? itr3->_lastVersion:itr2->_lastVersion;
                        }
                    }
                }
            }
        }
        _isAssociatesRevisionsInheritanceDone=true;
    }

}
void ObjectWrapper::markAssociateAsAdded(const std::string& name)
{
    for ( ObjectWrapper::RevisionAssociateList:: iterator itr=_associates.begin(); itr!=_associates.end(); ++itr )
    {
        if(itr->_name==name)
        {
            itr->_firstVersion=_version;
            return;
        }
    }
    OSG_NOTIFY(osg::WARN)<<"ObjectWrapper::associateAddedAtVersion: Associate class "<<name<<" not defined for wrapper "<<_name<<std::endl;
}
void ObjectWrapper::markAssociateAsRemoved(const std::string& name)
{
    for ( ObjectWrapper::RevisionAssociateList:: iterator itr=_associates.begin(); itr!=_associates.end(); ++itr )
    {
        if(itr->_name==name)
        {
            itr->_lastVersion = _version-1;
            return;
        }
    }
    OSG_NOTIFY(osg::WARN)<<"ObjectWrapper::associateRemovedAtVersion: Associate class "<<name<<" not defined for wrapper "<<_name<<std::endl;
}
void ObjectWrapper::addSerializer( BaseSerializer* s, BaseSerializer::Type t )
{
    s->_firstVersion = _version;
    _serializers.push_back(s);
    _typeList.push_back(t);
}

void ObjectWrapper::markSerializerAsRemoved( const std::string& name )
{
    for ( SerializerList::iterator itr=_serializers.begin(); itr!=_serializers.end(); ++itr )
    {
        // When a serializer is marked as 'removed', it means that this serializer won't be used any more
        // from specified OSG version (by macro UPDATE_TO_VERSION). The read() functions of higher versions
        // will thus ignore it according to the sign and value of the _version variable.
        if ( (*itr)->getName()==name )
            (*itr)->_lastVersion = _version-1;
    }
}

BaseSerializer* ObjectWrapper::getSerializer( const std::string& name )
{
    for ( SerializerList::iterator itr=_serializers.begin(); itr!=_serializers.end(); ++itr )
    {
        if ( (*itr)->getName()==name )
            return itr->get();
    }

    for ( RevisionAssociateList::const_iterator itr=_associates.begin(); itr!=_associates.end(); ++itr )
    {
        const std::string& assocName = itr->_name;
        ObjectWrapper* assocWrapper = Registry::instance()->getObjectWrapperManager()->findWrapper(assocName);
        if ( !assocWrapper )
        {
            osg::notify(osg::WARN) << "ObjectWrapper::getSerializer(): Unsupported associated class "
                                   << assocName << std::endl;
            continue;
        }

        for ( SerializerList::iterator aitr=assocWrapper->_serializers.begin();
              aitr!=assocWrapper->_serializers.end(); ++aitr )
        {
            if ( (*aitr)->getName()==name )
                return aitr->get();
        }
    }
    return NULL;
}

BaseSerializer* ObjectWrapper::getSerializer( const std::string& name, BaseSerializer::Type& type)
{

    unsigned int i = 0;
    for (SerializerList::iterator itr=_serializers.begin();
         itr!=_serializers.end();
         ++itr, ++i )
    {
        if ( (*itr)->getName()==name )
        {
            type = _typeList[i];
            return itr->get();
        }
    }

    for ( RevisionAssociateList::const_iterator itr=_associates.begin(); itr!=_associates.end(); ++itr )
    {
        const std::string& assocName = itr->_name;
        ObjectWrapper* assocWrapper = Registry::instance()->getObjectWrapperManager()->findWrapper(assocName);
        if ( !assocWrapper )
        {
            osg::notify(osg::WARN) << "ObjectWrapper::getSerializer(): Unsupported associated class "
                                   << assocName << std::endl;
            continue;
        }

        i = 0;
        for ( SerializerList::iterator aitr=assocWrapper->_serializers.begin();
              aitr!=assocWrapper->_serializers.end();
              ++aitr, ++i )
        {
            if ( (*aitr)->getName()==name )
            {
                type = assocWrapper->_typeList[i];
                return aitr->get();
            }
        }
    }
    type = BaseSerializer::RW_UNDEFINED;
    return NULL;
}

bool ObjectWrapper::read( InputStream& is, osg::Object& obj )
{
    bool readOK = true;
    int inputVersion = is.getFileVersion(_domain);
    for ( SerializerList::iterator itr=_serializers.begin();
          itr!=_serializers.end(); ++itr )
    {
        BaseSerializer* serializer = itr->get();
        if ( serializer->_firstVersion <= inputVersion &&
             inputVersion <= serializer->_lastVersion &&
             serializer->supportsReadWrite())
        {
            if ( !serializer->read(is, obj) )
            {
                OSG_WARN << "ObjectWrapper::read(): Error reading property "
                                    << _name << "::" << (*itr)->getName() << std::endl;
                readOK = false;
            }
        }
        else
        {
            // OSG_NOTICE<<"Ignoring serializer due to version mismatch"<<std::endl;
        }
    }

    for ( FinishedObjectReadCallbackList::iterator itr=_finishedObjectReadCallbacks.begin();
          itr!=_finishedObjectReadCallbacks.end();
          ++itr )
     {
         (*itr)->objectRead(is, obj);
     }

    return readOK;
}

bool ObjectWrapper::write( OutputStream& os, const osg::Object& obj )
{
    bool writeOK = true;
    int outputVersion = os.getFileVersion(_domain);
    for ( SerializerList::iterator itr=_serializers.begin();
          itr!=_serializers.end(); ++itr )
    {
        BaseSerializer* serializer = itr->get();
        if ( serializer->_firstVersion <= outputVersion &&
             outputVersion <= serializer->_lastVersion  &&
             serializer->supportsReadWrite())
        {
            if ( !serializer->write(os, obj) )
            {
                OSG_WARN << "ObjectWrapper::write(): Error writing property "
                                    << _name << "::" << (*itr)->getName() << std::endl;
                writeOK = false;
            }
        }
        else
        {
            // OSG_NOTICE<<"Ignoring serializer due to version mismatch"<<std::endl;
        }
    }
    return writeOK;
}

bool ObjectWrapper::readSchema( const StringList& properties, const TypeList& )
{
    // FIXME: At present, I didn't do anything to determine serializers from their types...
    if ( !_backupSerializers.size() )
        _backupSerializers = _serializers;
    _serializers.clear();

    unsigned int size = properties.size();
    unsigned int serializersSize = _backupSerializers.size();
    for ( unsigned int i=0; i<size; ++i )
    {
        if ( serializersSize<i )
        {
            OSG_WARN << "ObjectWrapper::readSchema(): Wrapper " << _name
                                   << ": Incompatible serializers size" << std::endl;
            break;
        }

        const std::string& prop = properties[i];
        if ( prop==_backupSerializers[i]->getName() )
        {
            _serializers.push_back( _backupSerializers[i] );
        }
        else
        {
            bool hasSerializer = false;
            for ( SerializerList::iterator itr=_backupSerializers.begin();
                  itr!=_backupSerializers.end(); ++itr )
            {
                if ( prop!=(*itr)->getName() ) continue;
                _serializers.push_back( *itr );
                hasSerializer = true;
            }
            if ( !hasSerializer )
            {
                OSG_WARN << "ObjectWrapper::readSchema(): Wrapper " << _name
                                       << ": Unknown property " << prop << std::endl;
            }
        }
    }
    return size==_serializers.size();
}

void ObjectWrapper::writeSchema( StringList& properties, TypeList& types )
{
    SerializerList::iterator sitr = _serializers.begin();
    TypeList::iterator titr = _typeList.begin();
    while(sitr!=_serializers.end() && titr!=_typeList.end())
    {
        if ((*sitr)->supportsReadWrite())
        {
            properties.push_back( (*sitr)->getName() );
            types.push_back( (*titr) );
        }
        ++sitr;
        ++titr;
    }
}

void ObjectWrapper::addMethodObject(const std::string& methodName, MethodObject* mo)
{
    _methodObjectMap.insert(MethodObjectMap::value_type(methodName, mo));
}


////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// RegisterWrapperProxy
//
RegisterWrapperProxy::RegisterWrapperProxy( ObjectWrapper::CreateInstanceFunc *createInstanceFunc, const std::string& name,
                        const std::string& associates, AddPropFunc func )
{
    _wrapper = new ObjectWrapper( createInstanceFunc, name, associates );
    if ( func ) (*func)( _wrapper.get() );

    if (Registry::instance())
    {
        Registry::instance()->getObjectWrapperManager()->addWrapper( _wrapper.get() );
    }
}

RegisterWrapperProxy::~RegisterWrapperProxy()
{
    if (Registry::instance())
    {
        Registry::instance()->getObjectWrapperManager()->removeWrapper( _wrapper.get() );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// RegisterCustomWrapperProxy
//
RegisterCustomWrapperProxy::RegisterCustomWrapperProxy(
        ObjectWrapper::CreateInstanceFunc *createInstanceFunc, const std::string& domain, const std::string& name,
        const std::string& associates, AddPropFunc func )
{
    _wrapper = new ObjectWrapper( createInstanceFunc, domain, name, associates );
    if ( func ) (*func)( domain.c_str(), _wrapper.get() );

    if (Registry::instance())
    {
        Registry::instance()->getObjectWrapperManager()->addWrapper( _wrapper.get() );
    }
}

RegisterCustomWrapperProxy::~RegisterCustomWrapperProxy()
{
    if (Registry::instance())
    {
        Registry::instance()->getObjectWrapperManager()->removeWrapper( _wrapper.get() );
    }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ObjectWrapperManager
//
////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// GlobalLookupTable
//
ObjectWrapperManager::ObjectWrapperManager()
{
    IntLookup& glTable = _globalMap["GL"];

    // Modes
    glTable.add( "GL_ALPHA_TEST", GL_ALPHA_TEST );
    glTable.add( "GL_BLEND", GL_BLEND );
    glTable.add( "GL_COLOR_LOGIC_OP", GL_COLOR_LOGIC_OP );
    glTable.add( "GL_COLOR_MATERIAL", GL_COLOR_MATERIAL );
    glTable.add( "GL_CULL_FACE", GL_CULL_FACE );
    glTable.add( "GL_DEPTH_TEST", GL_DEPTH_TEST );
    glTable.add( "GL_FOG", GL_FOG );
    glTable.add( "GL_FRAGMENT_PROGRAM_ARB", GL_FRAGMENT_PROGRAM_ARB );
    glTable.add( "GL_LINE_STIPPLE", GL_LINE_STIPPLE );
    glTable.add( "GL_POINT_SMOOTH", GL_POINT_SMOOTH );
    glTable.add( "GL_POINT_SPRITE_ARB", GL_POINT_SPRITE_ARB );
    glTable.add( "GL_POLYGON_OFFSET_FILL", GL_POLYGON_OFFSET_FILL );
    glTable.add( "GL_POLYGON_OFFSET_LINE", GL_POLYGON_OFFSET_LINE );
    glTable.add( "GL_POLYGON_OFFSET_POINT", GL_POLYGON_OFFSET_POINT );
    glTable.add( "GL_POLYGON_STIPPLE", GL_POLYGON_STIPPLE );
    glTable.add( "GL_SCISSOR_TEST", GL_SCISSOR_TEST);
    glTable.add( "GL_STENCIL_TEST", GL_STENCIL_TEST );
    glTable.add( "GL_STENCIL_TEST_TWO_SIDE", GL_STENCIL_TEST_TWO_SIDE );
    glTable.add( "GL_VERTEX_PROGRAM_ARB", GL_VERTEX_PROGRAM_ARB );

    glTable.add( "GL_COLOR_SUM", GL_COLOR_SUM );
    glTable.add( "GL_LIGHTING", GL_LIGHTING );
    glTable.add( "GL_NORMALIZE", GL_NORMALIZE );
    glTable.add( "GL_RESCALE_NORMAL", GL_RESCALE_NORMAL );

    glTable.add( "GL_TEXTURE_1D", GL_TEXTURE_1D );
    glTable.add( "GL_TEXTURE_2D", GL_TEXTURE_2D );
    glTable.add( "GL_TEXTURE_3D", GL_TEXTURE_3D );
    glTable.add( "GL_TEXTURE_CUBE_MAP", GL_TEXTURE_CUBE_MAP );
    glTable.add( "GL_TEXTURE_RECTANGLE", GL_TEXTURE_RECTANGLE );
    glTable.add( "GL_TEXTURE_GEN_Q", GL_TEXTURE_GEN_Q );
    glTable.add( "GL_TEXTURE_GEN_R", GL_TEXTURE_GEN_R );
    glTable.add( "GL_TEXTURE_GEN_S", GL_TEXTURE_GEN_S );
    glTable.add( "GL_TEXTURE_GEN_T", GL_TEXTURE_GEN_T );

    glTable.add( "GL_CLIP_PLANE0", GL_CLIP_PLANE0 );
    glTable.add( "GL_CLIP_PLANE1", GL_CLIP_PLANE1 );
    glTable.add( "GL_CLIP_PLANE2", GL_CLIP_PLANE2 );
    glTable.add( "GL_CLIP_PLANE3", GL_CLIP_PLANE3 );
    glTable.add( "GL_CLIP_PLANE4", GL_CLIP_PLANE4 );
    glTable.add( "GL_CLIP_PLANE5", GL_CLIP_PLANE5 );

    glTable.add( "GL_LIGHT0", GL_LIGHT0 );
    glTable.add( "GL_LIGHT1", GL_LIGHT1 );
    glTable.add( "GL_LIGHT2", GL_LIGHT2 );
    glTable.add( "GL_LIGHT3", GL_LIGHT3 );
    glTable.add( "GL_LIGHT4", GL_LIGHT4 );
    glTable.add( "GL_LIGHT5", GL_LIGHT5 );
    glTable.add( "GL_LIGHT6", GL_LIGHT6 );
    glTable.add( "GL_LIGHT7", GL_LIGHT7 );

    glTable.add("GL_VERTEX_PROGRAM_POINT_SIZE", GL_VERTEX_PROGRAM_POINT_SIZE);
    glTable.add("GL_VERTEX_PROGRAM_TWO_SIDE", GL_VERTEX_PROGRAM_TWO_SIDE);

    // Functions
    glTable.add( "NEVER", GL_NEVER );
    glTable.add( "LESS", GL_LESS );
    glTable.add( "EQUAL", GL_EQUAL );
    glTable.add( "LEQUAL", GL_LEQUAL );
    glTable.add( "GREATER", GL_GREATER );
    glTable.add( "NOTEQUAL", GL_NOTEQUAL );
    glTable.add( "GEQUAL", GL_GEQUAL );
    glTable.add( "ALWAYS", GL_ALWAYS );

    // Texture environment states
    glTable.add( "REPLACE", GL_REPLACE );
    glTable.add( "MODULATE", GL_MODULATE );
    glTable.add( "ADD", GL_ADD );
    glTable.add( "ADD_SIGNED", GL_ADD_SIGNED_ARB );
    glTable.add( "INTERPOLATE", GL_INTERPOLATE_ARB );
    glTable.add( "SUBTRACT", GL_SUBTRACT_ARB );
    glTable.add( "DOT3_RGB", GL_DOT3_RGB_ARB );
    glTable.add( "DOT3_RGBA", GL_DOT3_RGBA_ARB );

    glTable.add( "CONSTANT", GL_CONSTANT_ARB );
    glTable.add( "PRIMARY_COLOR", GL_PRIMARY_COLOR_ARB );
    glTable.add( "PREVIOUS", GL_PREVIOUS_ARB );
    glTable.add( "TEXTURE", GL_TEXTURE );
    glTable.add( "TEXTURE0", GL_TEXTURE0 );
    glTable.add( "TEXTURE1", GL_TEXTURE0+1 );
    glTable.add( "TEXTURE2", GL_TEXTURE0+2 );
    glTable.add( "TEXTURE3", GL_TEXTURE0+3 );
    glTable.add( "TEXTURE4", GL_TEXTURE0+4 );
    glTable.add( "TEXTURE5", GL_TEXTURE0+5 );
    glTable.add( "TEXTURE6", GL_TEXTURE0+6 );
    glTable.add( "TEXTURE7", GL_TEXTURE0+7 );

    // Texture clamp modes
    glTable.add( "CLAMP", GL_CLAMP );
    glTable.add( "CLAMP_TO_EDGE", GL_CLAMP_TO_EDGE );
    glTable.add( "CLAMP_TO_BORDER", GL_CLAMP_TO_BORDER_ARB );
    glTable.add( "REPEAT", GL_REPEAT );
    glTable.add( "MIRROR", GL_MIRRORED_REPEAT_IBM );

    // Texture filter modes
    glTable.add( "LINEAR", GL_LINEAR );
    glTable.add( "LINEAR_MIPMAP_LINEAR", GL_LINEAR_MIPMAP_LINEAR );
    glTable.add( "LINEAR_MIPMAP_NEAREST", GL_LINEAR_MIPMAP_NEAREST );
    glTable.add( "NEAREST", GL_NEAREST );
    glTable.add( "NEAREST_MIPMAP_LINEAR", GL_NEAREST_MIPMAP_LINEAR );
    glTable.add( "NEAREST_MIPMAP_NEAREST", GL_NEAREST_MIPMAP_NEAREST );

    // Texture formats
    glTable.add( "GL_INTENSITY", GL_INTENSITY );
    glTable.add( "GL_LUMINANCE", GL_LUMINANCE );
    glTable.add( "GL_ALPHA", GL_ALPHA );
    glTable.add( "GL_LUMINANCE_ALPHA", GL_LUMINANCE_ALPHA );
    glTable.add( "GL_RGB", GL_RGB );
    glTable.add( "GL_RGBA", GL_RGBA );
    glTable.add( "GL_COMPRESSED_ALPHA_ARB", GL_COMPRESSED_ALPHA_ARB );
    glTable.add( "GL_COMPRESSED_LUMINANCE_ARB", GL_COMPRESSED_LUMINANCE_ARB );
    glTable.add( "GL_COMPRESSED_INTENSITY_ARB", GL_COMPRESSED_INTENSITY_ARB );
    glTable.add( "GL_COMPRESSED_LUMINANCE_ALPHA_ARB", GL_COMPRESSED_LUMINANCE_ALPHA_ARB );
    glTable.add( "GL_COMPRESSED_RGB_ARB", GL_COMPRESSED_RGB_ARB );
    glTable.add( "GL_COMPRESSED_RGBA_ARB", GL_COMPRESSED_RGBA_ARB );
    glTable.add( "GL_COMPRESSED_RGB_S3TC_DXT1_EXT", GL_COMPRESSED_RGB_S3TC_DXT1_EXT );
    glTable.add( "GL_COMPRESSED_RGBA_S3TC_DXT1_EXT", GL_COMPRESSED_RGBA_S3TC_DXT1_EXT );
    glTable.add( "GL_COMPRESSED_RGBA_S3TC_DXT3_EXT", GL_COMPRESSED_RGBA_S3TC_DXT3_EXT );
    glTable.add( "GL_COMPRESSED_RGBA_S3TC_DXT5_EXT", GL_COMPRESSED_RGBA_S3TC_DXT5_EXT );
    glTable.add( "GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG",GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG );
    glTable.add( "GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG",GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG );
    glTable.add( "GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG",GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG );
    glTable.add( "GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG",GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG );
    glTable.add( "GL_ETC1_RGB8_OES",GL_ETC1_RGB8_OES );
    glTable.add( "GL_COMPRESSED_RGB8_ETC2",GL_COMPRESSED_RGB8_ETC2 );
    glTable.add( "GL_COMPRESSED_SRGB8_ETC2",GL_COMPRESSED_SRGB8_ETC2 );
    glTable.add( "GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2",GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2 );
    glTable.add( "GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2",GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2 );
    glTable.add( "GL_COMPRESSED_RGBA8_ETC2_EAC",GL_COMPRESSED_RGBA8_ETC2_EAC );
    glTable.add( "GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC",GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC );
    glTable.add( "GL_COMPRESSED_R11_EAC",GL_COMPRESSED_R11_EAC );
    glTable.add( "GL_COMPRESSED_SIGNED_R11_EAC",GL_COMPRESSED_SIGNED_R11_EAC );
    glTable.add( "GL_COMPRESSED_RG11_EAC",GL_COMPRESSED_RG11_EAC );
    glTable.add( "GL_COMPRESSED_SIGNED_RG11_EAC",GL_COMPRESSED_SIGNED_RG11_EAC );

    // Texture source types
    glTable.add( "GL_BYTE", GL_BYTE );
    glTable.add( "GL_SHORT", GL_SHORT );
    glTable.add( "GL_INT", GL_INT );
    glTable.add( "GL_FLOAT", GL_FLOAT );
    glTable.add( "GL_DOUBLE", GL_DOUBLE );
    glTable.add( "GL_UNSIGNED_BYTE", GL_UNSIGNED_BYTE );
    glTable.add( "GL_UNSIGNED_SHORT", GL_UNSIGNED_SHORT );
    glTable.add( "GL_UNSIGNED_INT", GL_UNSIGNED_INT );

    // Blend values
    glTable.add( "DST_ALPHA", GL_DST_ALPHA );
    glTable.add( "DST_COLOR", GL_DST_COLOR );
    glTable.add( "ONE", GL_ONE );
    glTable.add( "ONE_MINUS_DST_ALPHA", GL_ONE_MINUS_DST_ALPHA );
    glTable.add( "ONE_MINUS_DST_COLOR", GL_ONE_MINUS_DST_COLOR );
    glTable.add( "ONE_MINUS_SRC_ALPHA", GL_ONE_MINUS_SRC_ALPHA );
    glTable.add( "ONE_MINUS_SRC_COLOR", GL_ONE_MINUS_SRC_COLOR );
    glTable.add( "SRC_ALPHA", GL_SRC_ALPHA );
    glTable.add( "SRC_ALPHA_SATURATE", GL_SRC_ALPHA_SATURATE );
    glTable.add( "SRC_COLOR", GL_SRC_COLOR );
    glTable.add( "CONSTANT_COLOR", GL_CONSTANT_COLOR );
    glTable.add( "ONE_MINUS_CONSTANT_COLOR", GL_ONE_MINUS_CONSTANT_COLOR );
    glTable.add( "CONSTANT_ALPHA", GL_CONSTANT_ALPHA );
    glTable.add( "ONE_MINUS_CONSTANT_ALPHA", GL_ONE_MINUS_CONSTANT_ALPHA );
    glTable.add( "ZERO", GL_ZERO );

    // Fog coordinate sources
    glTable.add( "COORDINATE", GL_FOG_COORDINATE );
    glTable.add( "DEPTH", GL_FRAGMENT_DEPTH );

    // Hint targets
    glTable.add( "FOG_HINT", GL_FOG_HINT );
    glTable.add( "GENERATE_MIPMAP_HINT", GL_GENERATE_MIPMAP_HINT_SGIS );
    glTable.add( "LINE_SMOOTH_HINT", GL_LINE_SMOOTH_HINT );
    glTable.add( "PERSPECTIVE_CORRECTION_HINT", GL_PERSPECTIVE_CORRECTION_HINT );
    glTable.add( "POINT_SMOOTH_HINT", GL_POINT_SMOOTH_HINT );
    glTable.add( "POLYGON_SMOOTH_HINT", GL_POLYGON_SMOOTH_HINT );
    glTable.add( "TEXTURE_COMPRESSION_HINT", GL_TEXTURE_COMPRESSION_HINT_ARB );
    glTable.add( "FRAGMENT_SHADER_DERIVATIVE_HINT", GL_FRAGMENT_SHADER_DERIVATIVE_HINT );

    // Polygon modes
    glTable.add( "POINT", GL_POINT );
    glTable.add( "LINE", GL_LINE );
    glTable.add( "FILL", GL_FILL );

    // Misc
    glTable.add( "BACK", GL_BACK );
    glTable.add( "FRONT", GL_FRONT );
    glTable.add( "FRONT_AND_BACK", GL_FRONT_AND_BACK );
    glTable.add( "FIXED_ONLY", GL_FIXED_ONLY );
    glTable.add( "FASTEST", GL_FASTEST );
    glTable.add( "NICEST", GL_NICEST );
    glTable.add( "DONT_CARE", GL_DONT_CARE );

    IntLookup& arrayTable = _globalMap["ArrayType"];

    arrayTable.add( "ByteArray", ID_BYTE_ARRAY );
    arrayTable.add( "UByteArray", ID_UBYTE_ARRAY );
    arrayTable.add( "ShortArray", ID_SHORT_ARRAY );
    arrayTable.add( "UShortArray", ID_USHORT_ARRAY );
    arrayTable.add( "IntArray", ID_INT_ARRAY );
    arrayTable.add( "UIntArray", ID_UINT_ARRAY );
    arrayTable.add( "FloatArray", ID_FLOAT_ARRAY );
    arrayTable.add( "DoubleArray", ID_DOUBLE_ARRAY );

    arrayTable.add( "Vec2bArray", ID_VEC2B_ARRAY );
    arrayTable.add( "Vec3bArray", ID_VEC3B_ARRAY );
    arrayTable.add( "Vec4bArray", ID_VEC4B_ARRAY );
    arrayTable.add( "Vec2ubArray", ID_VEC2UB_ARRAY );
    arrayTable.add( "Vec3ubArray", ID_VEC3UB_ARRAY );
    arrayTable.add( "Vec4ubArray", ID_VEC4UB_ARRAY );
    arrayTable.add( "Vec2sArray", ID_VEC2S_ARRAY );
    arrayTable.add( "Vec3sArray", ID_VEC3S_ARRAY );
    arrayTable.add( "Vec4sArray", ID_VEC4S_ARRAY );
    arrayTable.add( "Vec2usArray", ID_VEC2US_ARRAY );
    arrayTable.add( "Vec3usArray", ID_VEC3US_ARRAY );
    arrayTable.add( "Vec4usArray", ID_VEC4US_ARRAY );
    arrayTable.add( "Vec2fArray", ID_VEC2_ARRAY );
    arrayTable.add( "Vec3fArray", ID_VEC3_ARRAY );
    arrayTable.add( "Vec4fArray", ID_VEC4_ARRAY );
    arrayTable.add( "Vec2dArray", ID_VEC2D_ARRAY );
    arrayTable.add( "Vec3dArray", ID_VEC3D_ARRAY );
    arrayTable.add( "Vec4dArray", ID_VEC4D_ARRAY );

    arrayTable.add( "Vec2iArray", ID_VEC2I_ARRAY );
    arrayTable.add( "Vec3iArray", ID_VEC3I_ARRAY );
    arrayTable.add( "Vec4iArray", ID_VEC4I_ARRAY );
    arrayTable.add( "Vec2uiArray", ID_VEC2UI_ARRAY );
    arrayTable.add( "Vec3uiArray", ID_VEC3UI_ARRAY );
    arrayTable.add( "Vec4uiArray", ID_VEC4UI_ARRAY );

    IntLookup& primitiveTable = _globalMap["PrimitiveType"];

    primitiveTable.add( "DrawArrays", ID_DRAWARRAYS );
    primitiveTable.add( "DrawArraysLength", ID_DRAWARRAY_LENGTH );
    primitiveTable.add( "DrawElementsUByte", ID_DRAWELEMENTS_UBYTE );
    primitiveTable.add( "DrawElementsUShort", ID_DRAWELEMENTS_USHORT );
    primitiveTable.add( "DrawElementsUInt", ID_DRAWELEMENTS_UINT );

    primitiveTable.add( "GL_POINTS", GL_POINTS );
    primitiveTable.add( "GL_LINES", GL_LINES );
    primitiveTable.add( "GL_LINE_STRIP", GL_LINE_STRIP );
    primitiveTable.add( "GL_LINE_LOOP", GL_LINE_LOOP );
    primitiveTable.add( "GL_TRIANGLES", GL_TRIANGLES );
    primitiveTable.add( "GL_TRIANGLE_STRIP", GL_TRIANGLE_STRIP );
    primitiveTable.add( "GL_TRIANGLE_FAN", GL_TRIANGLE_FAN );
    primitiveTable.add( "GL_QUADS", GL_QUADS );
    primitiveTable.add( "GL_QUAD_STRIP", GL_QUAD_STRIP );
    primitiveTable.add( "GL_POLYGON", GL_POLYGON );

    primitiveTable.add2("GL_LINES_ADJACENCY_EXT", "GL_LINES_ADJACENCY", GL_LINES_ADJACENCY );
    primitiveTable.add2("GL_LINE_STRIP_ADJACENCY_EXT", "GL_LINE_STRIP_ADJACENCY", GL_LINE_STRIP_ADJACENCY );
    primitiveTable.add2("GL_TRIANGLES_ADJACENCY_EXT", "GL_TRIANGLES_ADJACENCY", GL_TRIANGLES_ADJACENCY );
    primitiveTable.add2("GL_TRIANGLE_STRIP_ADJACENCY_EXT", "GL_TRIANGLE_STRIP_ADJACENCY", GL_TRIANGLE_STRIP_ADJACENCY );

    primitiveTable.add( "GL_PATCHES", GL_PATCHES );
}

ObjectWrapperManager::~ObjectWrapperManager()
{
}


void ObjectWrapperManager::addWrapper( ObjectWrapper* wrapper )
{
    if ( !wrapper ) return;

    OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_wrapperMutex);

    WrapperMap::iterator itr = _wrappers.find( wrapper->getName() );
    if ( itr!=_wrappers.end() )
    {
        OSG_WARN << "ObjectWrapperManager::addWrapper(): '" << wrapper->getName()
                               << "' already exists." << std::endl;
    }
    _wrappers[wrapper->getName()] = wrapper;
}

void ObjectWrapperManager::removeWrapper( ObjectWrapper* wrapper )
{
    if ( !wrapper ) return;

    OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_wrapperMutex);

    WrapperMap::iterator itr = _wrappers.find( wrapper->getName() );
    if ( itr!=_wrappers.end() ) _wrappers.erase( itr );
}

ObjectWrapper* ObjectWrapperManager::findWrapper( const std::string& name )
{
    OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_wrapperMutex);

    WrapperMap::iterator itr = _wrappers.find( name );
    if ( itr!=_wrappers.end() ) return itr->second.get();

    // Load external libraries
    std::string::size_type posDoubleColon = name.rfind("::");
    if ( posDoubleColon!=std::string::npos )
    {
        std::string libName = std::string( name, 0, posDoubleColon );

        ObjectWrapper* found=0;
        std::string nodeKitLib = osgDB::Registry::instance()->createLibraryNameForNodeKit(libName);
        if ( osgDB::Registry::instance()->loadLibrary(nodeKitLib)==osgDB::Registry::LOADED )
            found= findWrapper(name);

        std::string pluginLib = osgDB::Registry::instance()->createLibraryNameForExtension(std::string("serializers_")+libName);
        if ( osgDB::Registry::instance()->loadLibrary(pluginLib)==osgDB::Registry::LOADED )
            found= findWrapper(name);

        pluginLib = osgDB::Registry::instance()->createLibraryNameForExtension(libName);
        if ( osgDB::Registry::instance()->loadLibrary(pluginLib)==osgDB::Registry::LOADED )
            found= findWrapper(name);

        if (found) found->setupAssociatesRevisionsInheritanceIfRequired();

        return found;
    }
    return NULL;
}

void ObjectWrapperManager::addCompressor( BaseCompressor* compressor )
{
    if ( !compressor ) return;

    OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_wrapperMutex);

    CompressorMap::iterator itr = _compressors.find( compressor->getName() );
    if ( itr!=_compressors.end() )
    {
        OSG_WARN << "ObjectWrapperManager::addCompressor(): '" << compressor->getName()
                               << "' already exists." << std::endl;
    }
    _compressors[compressor->getName()] = compressor;
}

void ObjectWrapperManager::removeCompressor( BaseCompressor* compressor )
{
    if ( !compressor ) return;

    OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_wrapperMutex);

    CompressorMap::iterator itr = _compressors.find( compressor->getName() );
    if ( itr!=_compressors.end() ) _compressors.erase( itr );
}

BaseCompressor* ObjectWrapperManager::findCompressor( const std::string& name )
{
    OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_wrapperMutex);

    CompressorMap::iterator itr = _compressors.find( name );
    if ( itr!=_compressors.end() ) return itr->second.get();

    // Load external libraries
    std::string nodeKitLib = osgDB::Registry::instance()->createLibraryNameForNodeKit(name);
    if ( osgDB::Registry::instance()->loadLibrary(nodeKitLib)==osgDB::Registry::LOADED )
        return findCompressor(name);

    std::string pluginLib = osgDB::Registry::instance()->createLibraryNameForExtension(std::string("compressor_")+name);
    if ( osgDB::Registry::instance()->loadLibrary(pluginLib)==osgDB::Registry::LOADED )
        return findCompressor(name);

    pluginLib = osgDB::Registry::instance()->createLibraryNameForExtension(name);
    if ( osgDB::Registry::instance()->loadLibrary(pluginLib)==osgDB::Registry::LOADED )
        return findCompressor(name);
    return NULL;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// RegisytrCompressorProxy
//
RegisterCompressorProxy::RegisterCompressorProxy( const std::string& name, BaseCompressor* compressor ):
    _compressor(compressor)
{
    _compressor->setName( name );
    if (Registry::instance())
    {
        Registry::instance()->getObjectWrapperManager()->addCompressor( _compressor.get() );
    }
}

RegisterCompressorProxy::~RegisterCompressorProxy()
{
    if (Registry::instance())
    {
        Registry::instance()->getObjectWrapperManager()->removeCompressor( _compressor.get() );
    }
}
