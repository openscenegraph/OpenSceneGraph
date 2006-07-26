/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 * Copyright (C) 2003-2005 3Dlabs Inc. Ltd.
 * Copyright (C) 2004-2005 Nathan Cournia
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial
 * applications, as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
*/

/* file:   src/osg/Shader.cpp
 * author: Mike Weiblen 2005-06-15
*/

#include <fstream>
#include <list>

#include <osg/Notify>
#include <osg/State>
#include <osg/Timer>
#include <osg/FrameStamp>
#include <osg/buffered_value>
#include <osg/ref_ptr>
#include <osg/Shader>
#include <osg/GLExtensions>

#include <OpenThreads/ScopedLock>
#include <OpenThreads/Mutex>

using namespace osg;

///////////////////////////////////////////////////////////////////////////
// static cache of glShaders flagged for deletion, which will actually
// be deleted in the correct GL context.

typedef std::list<GLuint> GlShaderHandleList;
typedef osg::buffered_object<GlShaderHandleList> DeletedGlShaderCache;

static OpenThreads::Mutex    s_mutex_deletedGlShaderCache;
static DeletedGlShaderCache  s_deletedGlShaderCache;

void Shader::deleteGlShader(unsigned int contextID, GLuint shader)
{
    if( shader )
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedGlShaderCache);

        // add glShader to the cache for the appropriate context.
        s_deletedGlShaderCache[contextID].push_back(shader);
    }
}

void Shader::flushDeletedGlShaders(unsigned int contextID,double /*currentTime*/, double& availableTime)
{
    // if no time available don't try to flush objects.
    if (availableTime<=0.0) return;

    const GL2Extensions* extensions = GL2Extensions::Get(contextID,true);
    if( ! extensions->isGlslSupported() ) return;

    const osg::Timer& timer = *osg::Timer::instance();
    osg::Timer_t start_tick = timer.tick();
    double elapsedTime = 0.0;

    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedGlShaderCache);

        GlShaderHandleList& pList = s_deletedGlShaderCache[contextID];
        for(GlShaderHandleList::iterator titr=pList.begin();
            titr!=pList.end() && elapsedTime<availableTime;
            )
        {
            extensions->glDeleteShader( *titr );
            titr = pList.erase( titr );
            elapsedTime = timer.delta_s(start_tick,timer.tick());
        }
    }

    availableTime -= elapsedTime;
}


///////////////////////////////////////////////////////////////////////////
// osg::Shader
///////////////////////////////////////////////////////////////////////////

Shader::Shader(Type type) :
    _type(type)
{
}

Shader::Shader(Type type, const std::string& source) :
    _type(type)
{
    setShaderSource( source);
}

Shader::Shader(const Shader& rhs, const osg::CopyOp& copyop):
    osg::Object( rhs, copyop ),
    _type(rhs._type),
    _shaderSource(rhs._shaderSource)
{
}

Shader::~Shader()
{
}

bool Shader::setType( Type t )
{
    if( _type != UNDEFINED )
    {
        osg::notify(osg::WARN) << "cannot change type of Shader" << std::endl;
        return false;
    }

    _type = t;
    return true;
}

int Shader::compare(const Shader& rhs) const
{
    if( this == &rhs ) return 0;

    if( getType() < rhs.getType() ) return -1;
    if( rhs.getType() < getType() ) return 1;

    if( getName() < rhs.getName() ) return -1;
    if( rhs.getName() < getName() ) return 1;

    if( getShaderSource() < rhs.getShaderSource() ) return -1;
    if( rhs.getShaderSource() < getShaderSource() ) return 1;
    return 0;
}

void Shader::setShaderSource( const std::string& sourceText )
{
    _shaderSource = sourceText;
    dirtyShader();
}


Shader* Shader::readShaderFile( Type type, const std::string& fileName )
{
    ref_ptr<Shader> shader = new Shader(type);
    if (shader->loadShaderSourceFromFile(fileName)) return shader.release();
    return 0;
}

bool Shader::loadShaderSourceFromFile( const std::string& fileName )
{
    std::ifstream sourceFile;

    sourceFile.open(fileName.c_str(), std::ios::binary);
    if(!sourceFile)
    {
        osg::notify(osg::WARN)<<"Error: can't open file \""<<fileName<<"\""<<std::endl;
        return false;
    }

    osg::notify(osg::INFO)<<"Loading shader source file \""<<fileName<<"\""<<std::endl;

    sourceFile.seekg(0, std::ios::end);
    int length = sourceFile.tellg();
    char *text = new char[length + 1];
    sourceFile.seekg(0, std::ios::beg);
    sourceFile.read(text, length);
    sourceFile.close();
    text[length] = '\0';

    setShaderSource( text );
    delete [] text;
    return true;
}


const char* Shader::getTypename() const
{
    switch( getType() )
    {
        case VERTEX:    return "VERTEX";
        case FRAGMENT:  return "FRAGMENT";
        default:        return "UNDEFINED";
    }
}


Shader::Type Shader::getTypeId( const std::string& tname )
{
    if( tname == "VERTEX" )     return VERTEX;
    if( tname == "FRAGMENT" )   return FRAGMENT;
    return UNDEFINED;
}


void Shader::releaseGLObjects(osg::State* state) const
{
    if (!state) _pcsList.setAllElementsTo(0);
    else
    {
        unsigned int contextID = state->getContextID();
        _pcsList[contextID] = 0;
    }
}

void Shader::compileShader( unsigned int contextID ) const
{
    PerContextShader* pcs = getPCS( contextID );
    if( pcs ) pcs->compileShader();
}


Shader::PerContextShader* Shader::getPCS(unsigned int contextID) const
{
    if( getType() == UNDEFINED )
    {
        osg::notify(osg::WARN) << "Shader type is UNDEFINED" << std::endl;
        return 0;
    }

    if( ! _pcsList[contextID].valid() )
    {
        _pcsList[contextID] = new PerContextShader( this, contextID );
    }
    return _pcsList[contextID].get();
}


void Shader::attachShader(unsigned int contextID, GLuint program) const
{
    PerContextShader* pcs = getPCS( contextID );
    if( pcs ) pcs->attachShader( program );
}


bool Shader::getGlShaderInfoLog(unsigned int contextID, std::string& log) const
{
    PerContextShader* pcs = getPCS( contextID );
    return (pcs) ? pcs->getInfoLog( log ) : false;
}


/////////////////////////////////////////////////////////////////////////
// A Shader stores pointers to the osg::Programs to which it is attached,
// so that if the Shader is marked for recompilation with
// Shader::dirtyShader(), the upstream Program can be marked for relinking.
// _programSet does not use ref_ptrs, as that would cause a cyclical
// dependency, and neither the Program nor the Shader would be deleted.

bool Shader::addProgramRef( Program* program )
{
    ProgramSet::iterator itr = _programSet.find(program);
    if( itr != _programSet.end() ) return false;

    _programSet.insert( program );
    return true;
}

bool Shader::removeProgramRef( Program* program )
{
    ProgramSet::iterator itr = _programSet.find(program);
    if( itr == _programSet.end() ) return false;

    _programSet.erase( itr );
    return true;
}

void Shader::dirtyShader()
{
    // Mark our PCSs as needing recompilation.
    for( unsigned int cxt=0; cxt < _pcsList.size(); ++cxt )
    {
        if( _pcsList[cxt].valid() ) _pcsList[cxt]->requestCompile();
    }

    // Also mark Programs that depend on us as needing relink.
    for( ProgramSet::iterator itr = _programSet.begin();
        itr != _programSet.end(); ++itr )
    {
        (*itr)->dirtyProgram();
    }
}


/////////////////////////////////////////////////////////////////////////
// osg::Shader::PerContextShader
// PCS is the OSG abstraction of the per-context glShader
///////////////////////////////////////////////////////////////////////////

Shader::PerContextShader::PerContextShader(const Shader* shader, unsigned int contextID) :
        osg::Referenced(),
        _contextID( contextID )
{
    _shader = shader;
    _extensions = GL2Extensions::Get( _contextID, true );
    _glShaderHandle = _extensions->glCreateShader( shader->getType() );
    requestCompile();
}


Shader::PerContextShader::~PerContextShader()
{
    Shader::deleteGlShader( _contextID, _glShaderHandle );
}


void Shader::PerContextShader::requestCompile()
{
    _needsCompile = true;
    _isCompiled = false;
}


void Shader::PerContextShader::compileShader()
{
    if( ! _needsCompile ) return;
    _needsCompile = false;

    osg::notify(osg::INFO)
        << "\nCompiling " << _shader->getTypename()
        << " source:\n" << _shader->getShaderSource() << std::endl;

    GLint compiled = GL_FALSE;
    const char* sourceText = _shader->getShaderSource().c_str();
    _extensions->glShaderSource( _glShaderHandle, 1, &sourceText, NULL );
    _extensions->glCompileShader( _glShaderHandle );
    _extensions->glGetShaderiv( _glShaderHandle, GL_COMPILE_STATUS, &compiled );

    _isCompiled = (compiled == GL_TRUE);
    if( ! _isCompiled )
    {
        osg::notify(osg::WARN) << _shader->getTypename() << " glCompileShader \""
            << _shader->getName() << "\" FAILED" << std::endl;

        std::string infoLog;
        if( getInfoLog(infoLog) )
        {
            osg::notify(osg::WARN) << _shader->getTypename() << " Shader \""
                << _shader->getName() << "\" infolog:\n" << infoLog << std::endl;
        }
    }
    else
    {
        std::string infoLog;
        if( getInfoLog(infoLog) )
        {
            osg::notify(osg::INFO) << _shader->getTypename() << " Shader \""
                << _shader->getName() << "\" infolog:\n" << infoLog << std::endl;
        }
    }

}

bool Shader::PerContextShader::getInfoLog( std::string& infoLog ) const
{
    return _extensions->getShaderInfoLog( _glShaderHandle, infoLog );
}

void Shader::PerContextShader::attachShader(GLuint program) const
{
    _extensions->glAttachShader( program, _glShaderHandle );
}

void Shader::PerContextShader::detachShader(GLuint program) const
{
    _extensions->glDetachShader( program, _glShaderHandle );
}

/*EOF*/
