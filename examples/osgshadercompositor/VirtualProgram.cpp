////////////////////////////////////////////////////////////////////////////////
#include<osg/Shader>
#include<osg/Program>
#include<osg/State>
#include<osg/Notify>
#include<cassert>

////////////////////////////////////////////////////////////////////////////////
#include "VirtualProgram.h"

using namespace osg;

// If graphics board has program linking problems set MERGE_SHADERS to 1
// Merge shaders can be used to merge shaders strings into one shader. 
#define MERGE_SHADERS 0
#define NOTIFICATION_MESSAGES 0

namespace osgCandidate {
////////////////////////////////////////////////////////////////////////////////
VirtualProgram::VirtualProgram( unsigned int mask ) : _mask( mask ) 
{
}
////////////////////////////////////////////////////////////////////////////////
VirtualProgram::VirtualProgram
    ( const VirtualProgram& VirtualProgram, const osg::CopyOp& copyop ):
       osg::Program( VirtualProgram, copyop ),
       _shaderMap( VirtualProgram._shaderMap ),
       _mask( VirtualProgram._mask )
{
}
////////////////////////////////////////////////////////////////////////////////
VirtualProgram::~VirtualProgram( void )
{
}
////////////////////////////////////////////////////////////////////////////////
osg::Shader * VirtualProgram::getShader
    ( const std::string & shaderSemantic, osg::Shader::Type type )
{
    ShaderMap::key_type key( shaderSemantic, type );

    return _shaderMap[ key ].get();
}
////////////////////////////////////////////////////////////////////////////////
osg::Shader * VirtualProgram::setShader
( const std::string & shaderSemantic, osg::Shader * shader )
{
    if( shader->getType() == osg::Shader::UNDEFINED ) 
        return NULL;

    ShaderMap::key_type key( shaderSemantic, shader->getType() );

    ref_ptr< osg::Shader >   shaderNew     = shader;
    ref_ptr< osg::Shader > & shaderCurrent = _shaderMap[ key ];

#if 0 // Good for debugging of shader linking problems. 
      // Don't do it - User could use the name for its own purposes 
    shaderNew->setName( shaderSemantic );
#endif

    if( shaderCurrent != shaderNew ) {
#if 0
       if( shaderCurrent.valid() )
           Program::removeShader( shaderCurrent.get() );

       if( shaderNew.valid() )
           Program::addShader( shaderNew.get() );
#endif
       shaderCurrent = shaderNew;
    }

    return shaderCurrent.get();
}
////////////////////////////////////////////////////////////////////////////////
void VirtualProgram::apply( osg::State & state ) const
{
    if( _shaderMap.empty() ) // Virtual Program works as normal Program
        return Program::apply( state );

    State::AttributeVec *av = &state.getAttributeVec(this);

#if NOTIFICATION_MESSAGES
    std::ostream &os  = osg::notify( osg::NOTICE );
    os << "VirtualProgram cumulate Begin" << std::endl;
#endif

    ShaderMap shaderMap;
    for( State::AttributeVec::iterator i = av->begin(); i != av->end(); ++i )
    {
        const osg::StateAttribute * sa = i->first;
        const VirtualProgram * vp = dynamic_cast< const VirtualProgram *>( sa );
        if( vp && ( vp->_mask & _mask ) ) {

#if NOTIFICATION_MESSAGES
            if( vp->getName().empty() )
                os << "VirtualProgram cumulate [ Unnamed VP ] apply" << std::endl;
            else 
                os << "VirtualProgram cumulate ["<< vp->getName() << "] apply" << std::endl;
#endif

            for( ShaderMap::const_iterator i = vp->_shaderMap.begin();
                                           i != vp->_shaderMap.end(); ++i )
            {
                                                    shaderMap[ i->first ] = i->second;
            }

        } else {
#if NOTIFICATION_MESSAGES
            os << "VirtualProgram cumulate ( not VP or mask not match ) ignored" << std::endl;
#endif
            continue; // ignore osg::Programs
        }
    }

    for( ShaderMap::const_iterator i = this->_shaderMap.begin();
                                   i != this->_shaderMap.end(); ++i )
                                        shaderMap[ i->first ] = i->second;

#if NOTIFICATION_MESSAGES
    os << "VirtualProgram cumulate End" << std::endl;
#endif

    if( shaderMap.size() ) {

        ShaderList sl;
        for( ShaderMap::iterator i = shaderMap.begin(); i != shaderMap.end(); ++i )
            sl.push_back( i->second );

        osg::ref_ptr< osg::Program > & program = _programMap[ sl ];

        if( !program.valid() ) {
            program = new osg::Program;
#if !MERGE_SHADERS
            for( ShaderList::iterator i = sl.begin(); i != sl.end(); ++i )
                program->addShader( i->get() );
#else
            std::string strFragment;
            std::string strVertex;
            std::string strGeometry;
            
            for( ShaderList::iterator i = sl.begin(); i != sl.end(); ++i )
            {
                if( i->get()->getType() == osg::Shader::FRAGMENT )
                    strFragment += i->get()->getShaderSource();
                else if ( i->get()->getType() == osg::Shader::VERTEX )
                    strVertex += i->get()->getShaderSource();
                else if ( i->get()->getType() == osg::Shader::GEOMETRY )
                    strGeometry += i->get()->getShaderSource();
            }

            if( strFragment.length() > 0 ) {
                program->addShader( new osg::Shader( osg::Shader::FRAGMENT, strFragment ) );
#if NOTIFICATION_MESSAGES
                os << "====VirtualProgram merged Fragment Shader:"  << std::endl << strFragment << "====" << std::endl;
#endif
            }

            if( strVertex.length() > 0  ) {
                program->addShader( new osg::Shader( osg::Shader::VERTEX, strVertex ) );
#if NOTIFICATION_MESSAGES
                os << "VirtualProgram merged Vertex Shader:"  << std::endl << strVertex << "====" << std::endl;
#endif
            }

            if( strGeometry.length() > 0  ) {
                program->addShader( new osg::Shader( osg::Shader::GEOMETRY, strGeometry ) );
#if NOTIFICATION_MESSAGES
                os << "VirtualProgram merged Geometry Shader:"  << std::endl << strGeometry << "====" << std::endl;
#endif
            }
#endif
        }

        state.applyAttribute( program.get() );
    } else {
        Program::apply( state );
    }

#if NOTIFICATION_MESSAGES
    os << "VirtualProgram Apply" << std::endl;
#endif

}
////////////////////////////////////////////////////////////////////////////////
} // namespace osgExt
