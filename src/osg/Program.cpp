/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 * Copyright (C) 2003-2005 3Dlabs Inc. Ltd.
 * Copyright (C) 2004-2005 Nathan Cournia
 * Copyright (C) 2008 Zebra Imaging
 * Copyright (C) 2010 VIRES Simulationstechnologie GmbH
 * Copyright (C) 2012 David Callu
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commercial and non commercial
 * applications, as long as this copyright notice is maintained.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
*/

/* file:        src/osg/Program.cpp
 * author:      Mike Weiblen 2008-01-19
 *              Holger Helmich 2010-10-21
*/

#include <list>
#include <fstream>

#include <osg/Notify>
#include <osg/State>
#include <osg/Timer>
#include <osg/buffered_value>
#include <osg/ref_ptr>
#include <osg/Program>
#include <osg/Shader>
#include <osg/GLExtensions>
#include <osg/ContextData>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/Mutex>

#include <string.h>

using namespace osg;

class GLProgramManager : public GLObjectManager
{
public:
    GLProgramManager(unsigned int contextID) : GLObjectManager("GLProgramManager", contextID) {}

    virtual void deleteGLObject(GLuint globj)
    {
        const GLExtensions* extensions = GLExtensions::Get(_contextID,true);
        if (extensions->isGlslSupported) extensions->glDeleteProgram( globj );
    }
};


///////////////////////////////////////////////////////////////////////////
// osg::Program::ProgramBinary
///////////////////////////////////////////////////////////////////////////

Program::ProgramBinary::ProgramBinary() : _format(0)
{
}

Program::ProgramBinary::ProgramBinary(const ProgramBinary& rhs, const osg::CopyOp& copyop) :
    osg::Object(rhs, copyop),
    _data(rhs._data), _format(rhs._format)
{
}

void Program::ProgramBinary::allocate(unsigned int size)
{
    _data.clear();
    _data.resize(size);
}

void Program::ProgramBinary::assign(unsigned int size, const unsigned char* data)
{
    allocate(size);
    if (data)
    {
        for(unsigned int i=0; i<size; ++i)
        {
            _data[i] = data[i];
        }
    }
}


///////////////////////////////////////////////////////////////////////////
// osg::Program
///////////////////////////////////////////////////////////////////////////

Program::Program() :
    _geometryVerticesOut(1), _geometryInputType(GL_TRIANGLES),
    _geometryOutputType(GL_TRIANGLE_STRIP), _feedbackmode(GL_SEPARATE_ATTRIBS)
{
}


Program::Program(const Program& rhs, const osg::CopyOp& copyop):
    osg::StateAttribute(rhs, copyop)
{

    if ((copyop.getCopyFlags()&osg::CopyOp::DEEP_COPY_STATEATTRIBUTES)!=0)
    {
        for( unsigned int shaderIndex=0; shaderIndex < rhs.getNumShaders(); ++shaderIndex )
        {
            addShader( new osg::Shader( *rhs.getShader( shaderIndex ), copyop ) );
        }
    }
    else
    {
        for( unsigned int shaderIndex=0; shaderIndex < rhs.getNumShaders(); ++shaderIndex )
        {
            addShader( const_cast<osg::Shader*>(rhs.getShader( shaderIndex )) );
        }
    }

    const osg::Program::AttribBindingList &abl = rhs.getAttribBindingList();
    for( osg::Program::AttribBindingList::const_iterator attribute = abl.begin(); attribute != abl.end(); ++attribute )
    {
        addBindAttribLocation( attribute->first, attribute->second );
    }

    const osg::Program::FragDataBindingList &fdl = rhs.getFragDataBindingList();
    for( osg::Program::FragDataBindingList::const_iterator fragdata = fdl.begin(); fragdata != fdl.end(); ++fragdata )
    {
        addBindFragDataLocation( fragdata->first, fragdata->second );
    }

    _geometryVerticesOut = rhs._geometryVerticesOut;
    _geometryInputType = rhs._geometryInputType;
    _geometryOutputType = rhs._geometryOutputType;

    _feedbackmode=rhs._feedbackmode;
    _feedbackout=rhs._feedbackout;
}


Program::~Program()
{
    // inform any attached Shaders that we're going away
    for( unsigned int i=0; i < _shaderList.size(); ++i )
    {
        _shaderList[i]->removeProgramRef( this );
    }
}


int Program::compare(const osg::StateAttribute& sa) const
{
    // check the types are equal and then create the rhs variable
    // used by the COMPARE_StateAttribute_Parameter macros below.
    COMPARE_StateAttribute_Types(Program,sa)

    if( _shaderList.size() < rhs._shaderList.size() ) return -1;
    if( rhs._shaderList.size() < _shaderList.size() ) return 1;

    if( getName() < rhs.getName() ) return -1;
    if( rhs.getName() < getName() ) return 1;

    if( _geometryVerticesOut < rhs._geometryVerticesOut ) return -1;
    if( rhs._geometryVerticesOut < _geometryVerticesOut ) return 1;

    if( _geometryInputType < rhs._geometryInputType ) return -1;
    if( rhs._geometryInputType < _geometryInputType ) return 1;

    if( _geometryOutputType < rhs._geometryOutputType ) return -1;
    if( rhs._geometryOutputType < _geometryOutputType ) return 1;

    if(_feedbackout<rhs._feedbackout) return -1;
    if(_feedbackmode<rhs._feedbackmode) return -1;

    ShaderList::const_iterator litr=_shaderList.begin();
    ShaderList::const_iterator ritr=rhs._shaderList.begin();
    for(;
        litr!=_shaderList.end();
        ++litr,++ritr)
    {
        int result = (*litr)->compare(*(*ritr));
        if (result!=0) return result;
    }

    return 0; // passed all the above comparison macros, must be equal.
}


void Program::compileGLObjects( osg::State& state ) const
{
    if( _shaderList.empty() ) return;

    for( unsigned int i=0; i < _shaderList.size(); ++i )
    {
        _shaderList[i]->compileShader( state );
    }

    if(!_feedbackout.empty())
    {
        const PerContextProgram* pcp = getPCP(state);
        const GLExtensions* extensions = state.get<GLExtensions>();

        unsigned int numfeedback = _feedbackout.size();
        const char**varyings = new const char*[numfeedback];
        const char **varyingsptr = varyings;
        for(std::vector<std::string>::const_iterator it=_feedbackout.begin();
            it!=_feedbackout.end();
            it++)
        {
            *varyingsptr++=(*it).c_str();
        }

        extensions->glTransformFeedbackVaryings( pcp->getHandle(), numfeedback, varyings, _feedbackmode);
        delete [] varyings;
    }
    getPCP( state )->linkProgram(state);
}

void Program::setThreadSafeRefUnref(bool threadSafe)
{
    StateAttribute::setThreadSafeRefUnref(threadSafe);

    for( unsigned int i=0; i < _shaderList.size(); ++i )
    {
        if (_shaderList[i].valid()) _shaderList[i]->setThreadSafeRefUnref(threadSafe);
    }
}

void Program::dirtyProgram()
{
    // mark our PCPs as needing relink
    for( unsigned int cxt=0; cxt < _pcpList.size(); ++cxt )
    {
        if( _pcpList[cxt].valid() ) _pcpList[cxt]->requestLink();
    }

    // update list of defines required.
    _shaderDefines.clear();
    for(ShaderList::iterator itr = _shaderList.begin();
        itr != _shaderList.end();
        ++itr)
    {
        Shader* shader = itr->get();
        ShaderDefines& sd = shader->getShaderDefines();
        _shaderDefines.insert(sd.begin(), sd.end());

        ShaderDefines& sr = shader->getShaderRequirements();
        _shaderDefines.insert(sr.begin(), sr.end());
    }
}


void Program::resizeGLObjectBuffers(unsigned int maxSize)
{
    for( unsigned int i=0; i < _shaderList.size(); ++i )
    {
        if (_shaderList[i].valid()) _shaderList[i]->resizeGLObjectBuffers(maxSize);
    }

    _pcpList.resize(maxSize);
}

void Program::releaseGLObjects(osg::State* state) const
{
    for( unsigned int i=0; i < _shaderList.size(); ++i )
    {
        if (_shaderList[i].valid()) _shaderList[i]->releaseGLObjects(state);
    }

    if (!state) _pcpList.setAllElementsTo(0);
    else
    {
        unsigned int contextID = state->getContextID();
        _pcpList[contextID] = 0;
    }
}

bool Program::addShader( Shader* shader )
{
    if( !shader ) return false;

    // Shader can only be added once to a Program
    for( unsigned int i=0; i < _shaderList.size(); ++i )
    {
        if( shader == _shaderList[i].get() ) return false;
    }

    // Add shader to PCPs
    for( unsigned int cxt=0; cxt < _pcpList.size(); ++cxt )
    {
        if( _pcpList[cxt].valid() ) _pcpList[cxt]->addShaderToAttach( shader );
    }

    shader->addProgramRef( this );
    _shaderList.push_back( shader );
    dirtyProgram();
    return true;
}


bool Program::removeShader( Shader* shader )
{
    if( !shader ) return false;

    // Shader must exist to be removed.
    for( ShaderList::iterator itr = _shaderList.begin();
         itr != _shaderList.end();
         ++itr)
    {
        if( shader == itr->get() )
        {
            // Remove shader from PCPs
            for( unsigned int cxt=0; cxt < _pcpList.size(); ++cxt )
            {
                if( _pcpList[cxt].valid() ) _pcpList[cxt]->addShaderToDetach( shader );
            }

            shader->removeProgramRef( this );
            _shaderList.erase(itr);

            dirtyProgram();
            return true;
        }
    }

    return false;
}


void Program::setParameter( GLenum pname, GLint value )
{
    switch( pname )
    {
        case GL_GEOMETRY_VERTICES_OUT:
        case GL_GEOMETRY_VERTICES_OUT_EXT:
            _geometryVerticesOut = value;
            dirtyProgram();
            break;
        case GL_GEOMETRY_INPUT_TYPE:
        case GL_GEOMETRY_INPUT_TYPE_EXT:
            _geometryInputType = value;
            dirtyProgram();    // needed?
            break;
        case GL_GEOMETRY_OUTPUT_TYPE:
        case GL_GEOMETRY_OUTPUT_TYPE_EXT:
            _geometryOutputType = value;
            //dirtyProgram();    // needed?
            break;
        case GL_PATCH_VERTICES:
            OSG_WARN << "Program::setParameter invalid param " << GL_PATCH_VERTICES << ", use osg::PatchParameter when setting GL_PATCH_VERTICES."<<std::endl;
            break;
        default:
            OSG_WARN << "Program::setParameter invalid param " << pname << std::endl;
            break;
    }
}

GLint Program::getParameter( GLenum pname ) const
{
    switch( pname )
    {
        case GL_GEOMETRY_VERTICES_OUT:
        case GL_GEOMETRY_VERTICES_OUT_EXT:
            return _geometryVerticesOut;
        case GL_GEOMETRY_INPUT_TYPE:
        case GL_GEOMETRY_INPUT_TYPE_EXT:
            return _geometryInputType;
        case GL_GEOMETRY_OUTPUT_TYPE:
        case GL_GEOMETRY_OUTPUT_TYPE_EXT:
            return _geometryOutputType;
    }
    OSG_WARN << "getParameter invalid param " << pname << std::endl;
    return 0;
}

void Program::addBindAttribLocation( const std::string& name, GLuint index )
{
    _attribBindingList[name] = index;
    dirtyProgram();
}

void Program::removeBindAttribLocation( const std::string& name )
{
    _attribBindingList.erase(name);
    dirtyProgram();
}

void Program::addBindFragDataLocation( const std::string& name, GLuint index )
{
    _fragDataBindingList[name] = index;
    dirtyProgram();
}

void Program::removeBindFragDataLocation( const std::string& name )
{
    _fragDataBindingList.erase(name);
    dirtyProgram();
}

void Program::addBindUniformBlock(const std::string& name, GLuint index)
{
    _uniformBlockBindingList[name] = index;
    dirtyProgram(); // XXX
}

void Program::removeBindUniformBlock(const std::string& name)
{
    _uniformBlockBindingList.erase(name);
    dirtyProgram(); // XXX
}



#include <iostream>
void Program::apply( osg::State& state ) const
{
    const GLExtensions* extensions = state.get<GLExtensions>();
    if( ! extensions->isGlslSupported ) return;

    if( _shaderList.empty() )
    {
        extensions->glUseProgram( 0 );
        state.setLastAppliedProgramObject(0);
        return;
    }

#if 0
    State::DefineMap& defMap = state.getDefineMap();

    OSG_NOTICE<<"Program::apply() defMap.changed="<<defMap.changed<<std::endl;
    for(State::DefineMap::DefineStackMap::const_iterator itr = defMap.map.begin();
        itr != defMap.map.end();
        ++itr)
    {
        const State::DefineStack& ds = itr->second;
        OSG_NOTICE<<"  define ["<<itr->first<<"] ds.changed="<<ds.changed<<" ";
        if (ds.defineVec.empty())
        {
            OSG_NOTICE<<" DefineStack empty "<<std::endl;
        }
        else
        {
            const StateSet::DefinePair& dp = ds.defineVec.back();
            OSG_NOTICE<<"  value = ["<<dp.first<<"], overridevalue = ["<<dp.second<<"]"<< std::endl;
        }
    }

    if (defMap.changed) defMap.updateCurrentDefines();

    std::string shaderDefineStr = state.getDefineString(getShaderDefines());
    OSG_NOTICE<<"TailoredShaderDefineStr={"<<std::endl;
    OSG_NOTICE<<shaderDefineStr;
    OSG_NOTICE<<"}"<<std::endl;


    shaderDefineStr.clear();
    const StateSet::DefineList& currentDefines = defMap.currentDefines;
    for(StateSet::DefineList::const_iterator itr = currentDefines.begin();
        itr != currentDefines.end();
        ++itr)
    {
        const StateSet::DefinePair& dp = itr->second;
        shaderDefineStr += "#define ";
        shaderDefineStr += itr->first;
        if (itr->second.first.empty())
        {
            shaderDefineStr += "\n";
        }
        else
        {
            shaderDefineStr += " ";
            shaderDefineStr += itr->second.first;
            shaderDefineStr += "\n";
        }
        OSG_NOTICE<<"  active-define = ["<<itr->first<<"], value="<<itr->second.first<<", overridevalue = ["<<itr->second.second<<"]"<< std::endl;
    }

    OSG_NOTICE<<"FullShaderDefineStr={"<<std::endl;
    OSG_NOTICE<<shaderDefineStr;
    OSG_NOTICE<<"}"<<std::endl;


#endif


    PerContextProgram* pcp = getPCP( state );
    if( pcp->needsLink() ) compileGLObjects( state );
    if( pcp->isLinked() )
    {
        // for shader debugging: to minimize performance impact,
        // optionally validate based on notify level.
#ifndef __APPLE__
        if( osg::isNotifyEnabled(osg::INFO) )
            pcp->validateProgram();
#endif
        pcp->useProgram();
        state.setLastAppliedProgramObject(pcp);
    }
    else
    {
        // program not usable, fallback to fixed function.
        extensions->glUseProgram( 0 );
        state.setLastAppliedProgramObject(0);
    }
}


Program::ProgramObjects::ProgramObjects(const osg::Program* program, unsigned int contextID):
    _contextID(contextID),
    _program(program)
{
}


Program::PerContextProgram* Program::ProgramObjects::getPCP(const std::string& defineStr) const
{
    for(PerContextPrograms::const_iterator itr = _perContextPrograms.begin();
        itr != _perContextPrograms.end();
        ++itr)
    {
        if ((*itr)->getDefineString()==defineStr)
        {
            // OSG_NOTICE<<"Returning PCP "<<itr->get()<<" DefineString = "<<(*itr)->getDefineString()<<std::endl;
            return itr->get();
        }
    }
    return 0;
}

Program::PerContextProgram* Program::ProgramObjects::createPerContextProgram(const std::string& defineStr)
{
    Program::PerContextProgram* pcp = new PerContextProgram( _program, _contextID );
    _perContextPrograms.push_back( pcp );
    pcp->setDefineString(defineStr);
    // OSG_NOTICE<<"Creating PCP "<<pcp<<" PCP DefineString = ["<<pcp->getDefineString()<<"]"<<std::endl;
    return pcp;
}

void Program::ProgramObjects::requestLink()
{
    for(PerContextPrograms::iterator itr = _perContextPrograms.begin();
        itr != _perContextPrograms.end();
        ++itr)
    {
        (*itr)->requestLink();
    }
}

void Program::ProgramObjects::addShaderToAttach(Shader* shader)
{
    for(PerContextPrograms::iterator itr = _perContextPrograms.begin();
        itr != _perContextPrograms.end();
        ++itr)
    {
        (*itr)->addShaderToAttach(shader);
    }
}

void Program::ProgramObjects::addShaderToDetach(Shader* shader)
{
    for(PerContextPrograms::iterator itr = _perContextPrograms.begin();
        itr != _perContextPrograms.end();
        ++itr)
    {
        (*itr)->addShaderToDetach(shader);
    }
}


bool Program::ProgramObjects::getGlProgramInfoLog(std::string& log) const
{
    bool result = false;
    for(PerContextPrograms::const_iterator itr = _perContextPrograms.begin();
        itr != _perContextPrograms.end();
        ++itr)
    {
        result = (*itr)->getInfoLog( log ) | result;
    }
    return result;
}

Program::PerContextProgram* Program::getPCP(State& state) const
{
    unsigned int contextID = state.getContextID();
    const std::string defineStr = state.getDefineString(getShaderDefines());

    if( ! _pcpList[contextID].valid() )
    {
        _pcpList[contextID] = new ProgramObjects( this, contextID );
    }

    Program::PerContextProgram* pcp = _pcpList[contextID]->getPCP(defineStr);
    if (pcp) return pcp;

    pcp = _pcpList[contextID]->createPerContextProgram(defineStr);

    // attach all PCSs to this new PCP
    for( unsigned int i=0; i < _shaderList.size(); ++i )
    {
        pcp->addShaderToAttach( _shaderList[i].get() );
    }

    return pcp;
}


bool Program::isFixedFunction() const
{
#ifdef OSG_GL_FIXED_FUNCTION_AVAILABLE
    // A Program object having no attached Shaders is a special case:
    // it indicates that programmable shading is to be disabled,
    // and thus use GL 1.x "fixed functionality" rendering.
    return _shaderList.empty();
#else
    return false;
#endif
}


bool Program::getGlProgramInfoLog(unsigned int contextID, std::string& log) const
{
    if (contextID<_pcpList.size()) return (_pcpList[ contextID ])->getGlProgramInfoLog( log );
    else return false;
}

#if 0
const Program::ActiveUniformMap& Program::getActiveUniforms(unsigned int contextID) const
{
    return getPCP( contextID )->getActiveUniforms();
}

const Program::ActiveVarInfoMap& Program::getActiveAttribs(unsigned int contextID) const
{
    return getPCP( contextID )->getActiveAttribs();
}

const Program::UniformBlockMap& Program::getUniformBlocks(unsigned contextID) const
{
    return getPCP( contextID )->getUniformBlocks();
}
#endif

///////////////////////////////////////////////////////////////////////////
// osg::Program::PerContextProgram
// PCP is an OSG abstraction of the per-context glProgram
///////////////////////////////////////////////////////////////////////////

Program::PerContextProgram::PerContextProgram(const Program* program, unsigned int contextID, GLuint programHandle ) :
        osg::Referenced(),
        _glProgramHandle(programHandle),
        _loadedBinary(false),
        _contextID( contextID ),
        _ownsProgramHandle(false)
{
    _program = program;
    if (_glProgramHandle == 0)
    {
        _extensions = GLExtensions::Get( _contextID, true );
        _glProgramHandle = _extensions->glCreateProgram();

        if (_glProgramHandle)
        {
            _ownsProgramHandle = true;
        }
        else
        {
            OSG_WARN << "Unable to create osg::Program \"" << _program->getName() << "\"" << " contextID=" << _contextID <<  std::endl;
        }
    }
    requestLink();
}

Program::PerContextProgram::~PerContextProgram()
{
    if (_ownsProgramHandle)
    {
        osg::get<GLProgramManager>(_contextID)->scheduleGLObjectForDeletion(_glProgramHandle);
    }
}


void Program::PerContextProgram::requestLink()
{
    _needsLink = true;
    _isLinked = false;
}


void Program::PerContextProgram::linkProgram(osg::State& state)
{
    if( ! _needsLink ) return;
    _needsLink = false;

    if (!_glProgramHandle) return;

    OSG_INFO << "Linking osg::Program \"" << _program->getName() << "\""
             << " id=" << _glProgramHandle
             << " contextID=" << _contextID
             <<  std::endl;

    const ProgramBinary* programBinary = _program->getProgramBinary();

    _loadedBinary = false;
    if (programBinary && programBinary->getSize())
    {
        GLint linked = GL_FALSE;
        _extensions->glProgramBinary( _glProgramHandle, programBinary->getFormat(),
            reinterpret_cast<const GLvoid*>(programBinary->getData()), programBinary->getSize() );
        _extensions->glGetProgramiv( _glProgramHandle, GL_LINK_STATUS, &linked );
        _loadedBinary = _isLinked = (linked == GL_TRUE);
    }

    if (!_loadedBinary && _extensions->isGeometryShader4Supported)
    {
        _extensions->glProgramParameteri( _glProgramHandle, GL_GEOMETRY_VERTICES_OUT_EXT, _program->_geometryVerticesOut );
        _extensions->glProgramParameteri( _glProgramHandle, GL_GEOMETRY_INPUT_TYPE_EXT, _program->_geometryInputType );
        _extensions->glProgramParameteri( _glProgramHandle, GL_GEOMETRY_OUTPUT_TYPE_EXT, _program->_geometryOutputType );
    }

    if (!_loadedBinary)
    {
        const GLsizei shaderMaxCount = 20;
        GLsizei shadersCount = 0;
        GLuint shaderObjectHandle[shaderMaxCount];
        _extensions->glGetAttachedShaders(_glProgramHandle, shaderMaxCount, &shadersCount, shaderObjectHandle);

        typedef std::map<GLuint, int> ShaderSet;
        ShaderSet shadersRequired;

        for(GLsizei i=0; i<shadersCount; ++i)
        {
            shadersRequired[shaderObjectHandle[i]]--;
        }

        for(unsigned int i=0; i < getProgram()->getNumShaders(); ++i)
        {
            const Shader* shader = getProgram()->getShader( i );
            Shader::PerContextShader* pcs = shader->getPCS(state);
            if (pcs) shadersRequired[ pcs->getHandle() ]++;
        }

        for(ShaderSet::iterator itr = shadersRequired.begin();
            itr != shadersRequired.end();
            ++itr)
        {
            if (itr->second>0)
            {
                _extensions->glAttachShader( _glProgramHandle, itr->first );
            }
            else if (itr->second<0)
            {
                _extensions->glDetachShader( _glProgramHandle, itr->first );
            }
        }

    }
    _shadersToDetach.clear();
    _shadersToAttach.clear();

    _uniformInfoMap.clear();
    _attribInfoMap.clear();
    _lastAppliedUniformList.clear();

    if (!_loadedBinary)
    {
        // set any explicit vertex attribute bindings
        const AttribBindingList& programBindlist = _program->getAttribBindingList();
        for( AttribBindingList::const_iterator itr = programBindlist.begin();
            itr != programBindlist.end(); ++itr )
        {
            OSG_INFO<<"Program's vertex attrib binding "<<itr->second<<", "<<itr->first<<std::endl;
            _extensions->glBindAttribLocation( _glProgramHandle, itr->second, reinterpret_cast<const GLchar*>(itr->first.c_str()) );
        }

        // set any explicit vertex attribute bindings that are set up via osg::State, such as the vertex arrays
        //  that have been aliase to vertex attrib arrays
        if (state.getUseVertexAttributeAliasing())
        {
            const AttribBindingList& stateBindlist = state.getAttributeBindingList();
            for( AttribBindingList::const_iterator itr = stateBindlist.begin();
                itr != stateBindlist.end(); ++itr )
            {
                OSG_INFO<<"State's vertex attrib binding "<<itr->second<<", "<<itr->first<<std::endl;
                _extensions->glBindAttribLocation( _glProgramHandle, itr->second, reinterpret_cast<const GLchar*>(itr->first.c_str()) );
            }
        }

        // set any explicit frag data bindings
        const FragDataBindingList& fdbindlist = _program->getFragDataBindingList();
        for( FragDataBindingList::const_iterator itr = fdbindlist.begin();
            itr != fdbindlist.end(); ++itr )
        {
            _extensions->glBindFragDataLocation( _glProgramHandle, itr->second, reinterpret_cast<const GLchar*>(itr->first.c_str()) );
        }

        // if any program binary has been set then assume we want to retrieve a binary later.
        if (programBinary)
        {
            _extensions->glProgramParameteri( _glProgramHandle, GL_PROGRAM_BINARY_RETRIEVABLE_HINT, GL_TRUE );
        }

        // link the glProgram
        GLint linked = GL_FALSE;
        _extensions->glLinkProgram( _glProgramHandle );
        _extensions->glGetProgramiv( _glProgramHandle, GL_LINK_STATUS, &linked );
        _isLinked = (linked == GL_TRUE);
    }

    if( ! _isLinked )
    {
        OSG_NOTICE << "glLinkProgram "<<this<<"\""<< _program->getName() << "\" FAILED" << std::endl;

        std::string infoLog;
        if( getInfoLog(infoLog) )
        {
            OSG_NOTICE << "Program \""<< _program->getName() << "\" "
                                      "infolog:\n" << infoLog << std::endl;
        }

        return;
    }
    else
    {
        std::string infoLog;
        if( getInfoLog(infoLog) )
        {
            OSG_INFO << "Program \""<< _program->getName() << "\" "<<
                                      "link succeeded, infolog:\n" << infoLog << std::endl;
        }

        _extensions->debugObjectLabel(GL_PROGRAM, _glProgramHandle, _program->getName());
    }

    if (_extensions->isUniformBufferObjectSupported)
    {
        GLuint activeUniformBlocks = 0;
        GLsizei maxBlockNameLen = 0;
        _extensions->glGetProgramiv(_glProgramHandle, GL_ACTIVE_UNIFORM_BLOCKS,
                                    reinterpret_cast<GLint*>(&activeUniformBlocks));
        _extensions->glGetProgramiv(_glProgramHandle,
                                    GL_ACTIVE_UNIFORM_BLOCK_MAX_NAME_LENGTH,
                                    &maxBlockNameLen);
        if (maxBlockNameLen > 0)
        {
            std::vector<GLchar> blockName(maxBlockNameLen);
            for (GLuint i = 0; i < activeUniformBlocks; ++i)
            {
                GLsizei len = 0;
                GLint blockSize = 0;
                _extensions->glGetActiveUniformBlockName(_glProgramHandle, i,
                                                         maxBlockNameLen, &len,
                                                         &blockName[0]);
                _extensions->glGetActiveUniformBlockiv(_glProgramHandle, i,
                                                       GL_UNIFORM_BLOCK_DATA_SIZE,
                                                       &blockSize);
                _uniformBlockMap
                    .insert(UniformBlockMap::value_type(&blockName[0],
                                                        UniformBlockInfo(i, blockSize)));
            }
        }
        // Bind any uniform blocks
        const UniformBlockBindingList& bindingList = _program->getUniformBlockBindingList();
        for (UniformBlockMap::iterator itr = _uniformBlockMap.begin(),
                 end = _uniformBlockMap.end();
             itr != end;
            ++itr)
        {
            const std::string& blockName = itr->first;
            UniformBlockBindingList::const_iterator bitr = bindingList.find(blockName);
            if (bitr != bindingList.end())
            {
                _extensions->glUniformBlockBinding(_glProgramHandle, itr->second._index,
                                                   bitr->second);
                OSG_INFO << "uniform block " << blockName << ": " << itr->second._index
                         << " binding: " << bitr->second << "\n";
            }
            else
            {
                OSG_WARN << "uniform block " << blockName << " has no binding.\n";
            }
        }
    }

    typedef std::map<GLuint, std::string> AtomicCounterMap;
    AtomicCounterMap atomicCounterMap;

    // build _uniformInfoMap
    GLint numUniforms = 0;
    GLsizei maxLen = 0;
    _extensions->glGetProgramiv( _glProgramHandle, GL_ACTIVE_UNIFORMS, &numUniforms );
    _extensions->glGetProgramiv( _glProgramHandle, GL_ACTIVE_UNIFORM_MAX_LENGTH, &maxLen );
    if( (numUniforms > 0) && (maxLen > 1) )
    {
        GLint size = 0;
        GLenum type = 0;
        GLchar* name = new GLchar[maxLen];

        for( GLint i = 0; i < numUniforms; ++i )
        {
            _extensions->glGetActiveUniform( _glProgramHandle,
                    i, maxLen, 0, &size, &type, name );

            int pos = strlen(name);
            if (pos>0 && name[pos-1]==']')
            {
                // need to trim [..] from end of name as some drivers append this causing problems with look up.
                --pos;
                while(pos>0 && name[pos]!='[') { --pos; }
                name[pos] = 0;
            }

            if (type == GL_UNSIGNED_INT_ATOMIC_COUNTER)
            {
                atomicCounterMap[i] = name;
            }

            GLint loc = _extensions->glGetUniformLocation( _glProgramHandle, name );

            if( loc != -1 )
            {
                _uniformInfoMap[Uniform::getNameID(reinterpret_cast<const char*>(name))] = ActiveVarInfo(loc,type,size);

                OSG_INFO << "\tUniform \"" << name << "\""
                    << " loc="<< loc
                    << " size="<< size
                    << " type=" << Uniform::getTypename((Uniform::Type)type)
                    << std::endl;
            }
        }
        delete [] name;
    }

    // print atomic counter

    if (_extensions->isShaderAtomicCountersSupported && !atomicCounterMap.empty())
    {
        std::vector<GLint> bufferIndex( atomicCounterMap.size(), 0 );
        std::vector<GLuint> uniformIndex;
        for (AtomicCounterMap::iterator it = atomicCounterMap.begin(), end = atomicCounterMap.end();
             it != end; ++it)
        {
            uniformIndex.push_back(it->first);
        }

        _extensions->glGetActiveUniformsiv( _glProgramHandle, uniformIndex.size(),
                                            &(uniformIndex[0]), GL_UNIFORM_ATOMIC_COUNTER_BUFFER_INDEX,
                                            &(bufferIndex[0]) );

        for (unsigned int j = 0; j < uniformIndex.size(); ++j)
        {
            OSG_INFO << "\tUniform atomic counter \""<<atomicCounterMap[ uniformIndex[j] ] <<"\""
                     <<" buffer bind= " << bufferIndex[j] << ".\n";
        }

        std::map<int, std::vector<int> > bufferIndexToUniformIndices;
        for (unsigned int i=0; i<bufferIndex.size(); ++i)
        {
            bufferIndexToUniformIndices[ bufferIndex[i] ].push_back( uniformIndex[i] );
        }

        GLuint activeAtomicCounterBuffers = 0;
        _extensions->glGetProgramiv(_glProgramHandle, GL_ACTIVE_ATOMIC_COUNTER_BUFFERS,
                                    reinterpret_cast<GLint*>(&activeAtomicCounterBuffers));
        if (activeAtomicCounterBuffers > 0)
        {
            for (GLuint i = 0; i < activeAtomicCounterBuffers; ++i)
            {
                GLint bindID = 0;
                _extensions->glGetActiveAtomicCounterBufferiv(_glProgramHandle, i,
                                                              GL_ATOMIC_COUNTER_BUFFER_BINDING,
                                                              &bindID);

                GLsizei num = 0;
                _extensions->glGetActiveAtomicCounterBufferiv(_glProgramHandle, i,
                                                              GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTERS,
                                                              &num);
                GLsizei minSize = 0;
                _extensions->glGetActiveAtomicCounterBufferiv(_glProgramHandle, i,
                                                              GL_ATOMIC_COUNTER_BUFFER_DATA_SIZE,
                                                              &minSize);


                OSG_INFO << "\tUniform atomic counter buffer bind \"" << bindID << "\""
                         << " num active atomic counter= "<< num
                         << " min size= " << minSize << "\n";

                if (num)
                {
                    std::vector<GLint> indices(num);
                    _extensions->glGetActiveAtomicCounterBufferiv(_glProgramHandle, i,
                                                                  GL_ATOMIC_COUNTER_BUFFER_ACTIVE_ATOMIC_COUNTER_INDICES,
                                                                  &(indices[0]));
                    OSG_INFO << "\t\tindices used= ";
                    for (GLint j = 0; j < num; ++j)
                    {
                        OSG_INFO << indices[j];
                        if (j < (num-1))
                        {
                            OSG_INFO <<  ", ";
                        }
                        else
                        {
                            OSG_INFO <<  ".\n";
                        }
                    }
                }
            }
        }
    }

    // build _attribInfoMap
    GLint numAttrib = 0;
    _extensions->glGetProgramiv( _glProgramHandle, GL_ACTIVE_ATTRIBUTES, &numAttrib );
    _extensions->glGetProgramiv( _glProgramHandle, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &maxLen );
    if( (numAttrib > 0) && (maxLen > 1) )
    {
        GLint size = 0;
        GLenum type = 0;
        GLchar* name = new GLchar[maxLen];

        for( GLint i = 0; i < numAttrib; ++i )
        {
            _extensions->glGetActiveAttrib( _glProgramHandle,
                    i, maxLen, 0, &size, &type, name );

            GLint loc = _extensions->glGetAttribLocation( _glProgramHandle, name );

            if( loc != -1 )
            {
                _attribInfoMap[reinterpret_cast<char*>(name)] = ActiveVarInfo(loc,type,size);

                OSG_INFO << "\tAttrib \"" << name << "\""
                         << " loc=" << loc
                         << " size=" << size
                         << std::endl;
            }
        }
        delete [] name;
    }
    OSG_INFO << std::endl;


    //state.checkGLErrors("After Program::PerContextProgram::linkProgram.");

}

bool Program::PerContextProgram::validateProgram()
{
    if (!_glProgramHandle) return false;

    GLint validated = GL_FALSE;
    _extensions->glValidateProgram( _glProgramHandle );
    _extensions->glGetProgramiv( _glProgramHandle, GL_VALIDATE_STATUS, &validated );
    if( validated == GL_TRUE)
        return true;

    OSG_WARN << "glValidateProgram FAILED \"" << _program->getName() << "\""
             << " id=" << _glProgramHandle
             << " contextID=" << _contextID
             <<  std::endl;

    std::string infoLog;
    if( getInfoLog(infoLog) )
        OSG_WARN << "infolog:\n" << infoLog << std::endl;

    OSG_WARN << std::endl;

    return false;
}

bool Program::PerContextProgram::getInfoLog( std::string& infoLog ) const
{
    if (!_glProgramHandle) return false;

    return _extensions->getProgramInfoLog( _glProgramHandle, infoLog );
}

Program::ProgramBinary* Program::PerContextProgram::compileProgramBinary(osg::State& state)
{
    if (!_glProgramHandle) return 0;

    linkProgram(state);
    GLint binaryLength = 0;
    _extensions->glGetProgramiv( _glProgramHandle, GL_PROGRAM_BINARY_LENGTH, &binaryLength );
    if (binaryLength)
    {
        ProgramBinary* programBinary = new ProgramBinary;
        programBinary->allocate(binaryLength);
        GLenum binaryFormat = 0;
        _extensions->glGetProgramBinary( _glProgramHandle, binaryLength, 0, &binaryFormat, reinterpret_cast<GLvoid*>(programBinary->getData()) );
        programBinary->setFormat(binaryFormat);
        return programBinary;
    }
    return 0;
}

void Program::PerContextProgram::useProgram() const
{
    if (!_glProgramHandle) return;

    _extensions->glUseProgram( _glProgramHandle  );
}
