/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
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

/* file:	src/osg/Shader.cpp
 * author:	Mike Weiblen 2005-03-23
*/

// NOTICE: This code is CLOSED during construction and/or renovation!
// It is in active development, so DO NOT yet use in application code.
// This notice will be removed when the code is open for business.
// For development plan and status see:
// http://www.openscenegraph.org/index.php?page=Community.DevelopmentWork


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
typedef std::map<unsigned int, GlShaderHandleList> DeletedGlShaderCache;

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

        DeletedGlShaderCache::iterator citr = s_deletedGlShaderCache.find(contextID);
        if( citr != s_deletedGlShaderCache.end() )
        {
            GlShaderHandleList& pList = citr->second;
            for(GlShaderHandleList::iterator titr=pList.begin();
                titr!=pList.end() && elapsedTime<availableTime;
                )
            {
                extensions->glDeleteShader( *titr );
                titr = pList.erase( titr );
                elapsedTime = timer.delta_s(start_tick,timer.tick());
            }
        }
    }

    availableTime -= elapsedTime;
}


///////////////////////////////////////////////////////////////////////////
// osg::Shader
///////////////////////////////////////////////////////////////////////////

Shader::Shader(Type type, const char* sourceText) :
    _type(type)
{
    setShaderSource( sourceText );
}

Shader::Shader() :
    _type(VERTEX)
{
    // TODO this default constructor is inappropriate for the Shader class.
    // It exists for now because it is required by META_OBject
    osg::notify(osg::FATAL) << "how got here?" << std::endl;
}

Shader::Shader(const Shader& rhs, const osg::CopyOp& copyop):
    osg::Object( rhs, copyop ),
    _type(rhs._type),
    _name(rhs._name),
    _shaderSource(rhs._shaderSource)
{
}

Shader::~Shader()
{
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

void Shader::setShaderSource( const char* sourceText )
{
    _shaderSource = ( sourceText ? sourceText : "" );
    dirtyShader();
}


bool Shader::loadShaderSourceFromFile( const char* fileName )
{
    std::ifstream sourceFile;

    sourceFile.open(fileName, std::ios::binary);
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
	case VERTEX:	return "Vertex";
	case FRAGMENT:	return "Fragment";
	default:	return "UNDEFINED";
    }
}


void Shader::compileShader( unsigned int contextID ) const
{
    getPCS( contextID )->compileShader();
}


Shader::PerContextShader* Shader::getPCS(unsigned int contextID) const
{
    if( ! _pcsList[contextID].valid() )
    {
	_pcsList[contextID] = new PerContextShader( this, contextID );
    }
    return _pcsList[contextID].get();
}


void Shader::attachShader(unsigned int contextID, GLuint program) const
{
    getPCS( contextID )->attachShader( program );
}


void Shader::getGlShaderInfoLog(unsigned int contextID, std::string& log) const
{
    getPCS( contextID )->getInfoLog( log );
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

    GLint compiled;
    const char* sourceText = _shader->getShaderSource().c_str();
    _extensions->glShaderSource( _glShaderHandle, 1, &sourceText, NULL );
    _extensions->glCompileShader( _glShaderHandle );
    _extensions->glGetShaderiv( _glShaderHandle, GL_COMPILE_STATUS, &compiled );

    _isCompiled = (compiled == GL_TRUE);
    if( ! _isCompiled )
    {
	// compile failed
	std::string infoLog;
	getInfoLog( infoLog );
	osg::notify(osg::WARN) << _shader->getTypename() <<
		" glCompileShader FAILED:\n" << infoLog << std::endl;
    }
}

void Shader::PerContextShader::getInfoLog( std::string& infoLog ) const
{
    _extensions->getShaderInfoLog( _glShaderHandle, infoLog );
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
