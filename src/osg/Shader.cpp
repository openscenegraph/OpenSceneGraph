/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield 
 * Copyright (C) 2003-2005 3Dlabs Inc. Ltd.
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

/* file:	src/osg/Program.cpp
 * author:	Mike Weiblen 2005-02-20
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

typedef std::list<GLuint> GlShaderHandleList;
typedef std::map<unsigned int, GlShaderHandleList> DeletedGlShaderCache;

static OpenThreads::Mutex    s_mutex_deletedGL2ShaderCache;
static DeletedGlShaderCache  s_deletedGlShaderCache;

///////////////////////////////////////////////////////////////////////////
// osg::Shader
///////////////////////////////////////////////////////////////////////////

Shader::Shader(Type type, const char* sourceText) :
    _type(type)
{
    setShaderSource( sourceText );
}

Shader::Shader(const Shader& rhs, const osg::CopyOp& /*copyop*/):
    Referenced( rhs ),
    _type(rhs._type)
{
    /*TODO*/
}

Shader::~Shader()
{
    /*TODO*/
}

int Shader::compare(const Shader& rhs) const
{
    if( this == &rhs ) return 0;

    if( getComment() < rhs.getComment() ) return -1;
    if( rhs.getComment() < getComment() ) return 1;

    if( getShaderSource() < rhs.getShaderSource() ) return -1;
    if( rhs.getShaderSource() < getShaderSource() ) return 1;
    return 0;
}

void Shader::dirtyShader()
{
    // mark each PCSO as needing a recompile
    for( unsigned int cxt=0; cxt < _pcsoList.size(); ++cxt )
    {
        if( ! _pcsoList[cxt] ) continue;

	PerContextShaderObj* pcso = _pcsoList[cxt].get();
	pcso->markAsDirty();
    }

    // also mark-as-dirty the Programs we're attach to
    for( unsigned int i=0; i < _programList.size(); ++i )
    {
	_programList[i]->dirtyProgram();
    }
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
    PerContextShaderObj* pcso = getPCSO( contextID );
    if( pcso->isDirty() ) pcso->compileShader();
}


Shader::PerContextShaderObj* Shader::getPCSO(unsigned int contextID) const
{
    if( ! _pcsoList[contextID].valid() )
    {
	_pcsoList[contextID] = new PerContextShaderObj( this, contextID );
    }
    return _pcsoList[contextID].get();
}


void Shader::attachShader(unsigned int contextID, GLuint program) const
{
    getPCSO( contextID )->attachShader( program );
}


void Shader::addProgObjRef( Program* program )
{
    _programList.push_back( program );
}


void Shader::deleteShader(unsigned int contextID, GLuint shader)
{
    if( shader )
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedGL2ShaderCache);

        // add shader to the cache for the appropriate context.
        s_deletedGlShaderCache[contextID].push_back(shader);
    }
}


///////////////////////////////////////////////////////////////////////////
// osg::Shader::PerContextShaderObj
// PCSO is the OSG abstraction of the per-context glShader
///////////////////////////////////////////////////////////////////////////

Shader::PerContextShaderObj::PerContextShaderObj(const Shader* shader, unsigned int contextID) :
	osg::Referenced(),
	_contextID( contextID )
{
    _shader = shader;
    _extensions = GL2Extensions::Get( _contextID, true );
    _glShaderHandle = _extensions->glCreateShader( shader->getType() );
    markAsDirty();
}

Shader::PerContextShaderObj::PerContextShaderObj(const PerContextShaderObj& rhs) :
	osg::Referenced(),
	_contextID( rhs._contextID )
{
    _shader = rhs._shader;
    _extensions = rhs._extensions;
    _glShaderHandle = rhs._glShaderHandle;
    _dirty = rhs._dirty;
}

Shader::PerContextShaderObj::~PerContextShaderObj()
{
}

void Shader::PerContextShaderObj::compileShader()
{
    GLint compiled;
    const char* sourceText = _shader->getShaderSource().c_str();

    _extensions->glShaderSource( _glShaderHandle, 1, &sourceText, NULL );
    _extensions->glCompileShader( _glShaderHandle );
    _extensions->glGetShaderiv( _glShaderHandle, GL_COMPILE_STATUS, &compiled );
    _dirty = (compiled == 0);

    if( _dirty )
    {
	// _still_ dirty, something went wrong
	std::string infoLog;
	_extensions->getShaderInfoLog( _glShaderHandle, infoLog );
	osg::notify(osg::WARN) << _shader->getTypename() <<
		" glCompileShader FAILED:\n" << infoLog << std::endl;
    }
}

void Shader::PerContextShaderObj::attachShader(GLuint program) const
{
    _extensions->glAttachShader( program, _glShaderHandle );
}

/*EOF*/
