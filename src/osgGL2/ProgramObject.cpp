/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 * Copyright (C) 2003 3Dlabs Inc. Ltd.
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

/* file:	src/osgGL2/ProgramObject.cpp
 * author:	Mike Weiblen 2003-09-18
 *
 * See http://www.3dlabs.com/opengl2/ for more information regarding
 * the OpenGL Shading Language.
*/

#include <fstream>

#include <osg/Notify>
#include <osg/State>
#include <osg/Timer>
#include <osg/FrameStamp>

#include <osgGL2/ProgramObject>
#include <osgGL2/UniformValue>
#include <osgGL2/Extensions>

#include <list>

using namespace osgGL2;


///////////////////////////////////////////////////////////////////////////
// static cache of deleted GL2 objects which may only 
// by actually deleted in the correct GL context.

typedef std::list<GLhandleARB> GL2ObjectList;
typedef std::map<unsigned int, GL2ObjectList> DeletedGL2ObjectCache;
static DeletedGL2ObjectCache s_deletedGL2ObjectCache;

void ProgramObject::deleteObject(unsigned int contextID, GLhandleARB handle)
{
    if (handle!=0)
    {
        // add handle to the cache for the appropriate context.
        s_deletedGL2ObjectCache[contextID].push_back(handle);
    }
}

void ProgramObject::flushDeletedGL2Objects(unsigned int contextID,double /*currentTime*/, double& availableTime)
{
    // if no time available don't try to flush objects.
    if (availableTime<=0.0) return;

    const osg::Timer& timer = *osg::Timer::instance();
    osg::Timer_t start_tick = timer.tick();
    double elapsedTime = 0.0;

    DeletedGL2ObjectCache::iterator citr = s_deletedGL2ObjectCache.find(contextID);
    if( citr != s_deletedGL2ObjectCache.end() )
    {
        const Extensions* extensions = Extensions::Get(contextID,true);

	if (!extensions->isShaderObjectsSupported())
	{
	    // can we really get here?
	    osg::notify(osg::WARN) << "flushDeletedGL2Objects not supported by OpenGL driver" << std::endl;
	    return;
	}

        GL2ObjectList& vpObjectList = citr->second;

        for(GL2ObjectList::iterator titr=vpObjectList.begin();
            titr!=vpObjectList.end() && elapsedTime<availableTime;
            )
        {
            extensions->glDeleteObject( *titr );
            titr = vpObjectList.erase( titr );
            elapsedTime = timer.delta_s(start_tick,timer.tick());
        }
    }
    
    availableTime -= elapsedTime;
}


///////////////////////////////////////////////////////////////////////////
// osgGL2::ProgramObject
///////////////////////////////////////////////////////////////////////////

ProgramObject::ProgramObject()
{
    // To ensure all PCPOs consistently get the same values, we must
    // postpone updates until all PCPOs have been created.
    // They are created during ProgramObject::apply(), so let a frame
    // go by before sending the updates.
    _frameNumberOfLastPCPOUpdate = 1;
    _enabled = true;
}


ProgramObject::ProgramObject(const ProgramObject& rhs, const osg::CopyOp& copyop):
    osg::StateAttribute(rhs, copyop)
{
    osg::notify(osg::FATAL) << "how got here?" << std::endl;
}


// virtual
ProgramObject::~ProgramObject()
{
    for( unsigned int cxt=0; cxt < _pcpoList.size(); ++cxt )
    {
        if( ! _pcpoList[cxt] ) continue;

	PerContextProgObj* pcpo = _pcpoList[cxt].get();

	deleteObject( cxt, pcpo->getHandle() );
	// TODO add shader objects to delete list.
	_pcpoList[cxt] = 0;
    }
}


// mark all PCPOs as needing a relink
void ProgramObject::dirtyProgramObject()
{
    for( unsigned int cxt=0; cxt < _pcpoList.size(); ++cxt )
    {
        if( ! _pcpoList[cxt] ) continue;

	PerContextProgObj* pcpo = _pcpoList[cxt].get();
	pcpo->markAsDirty();
    }
}


// mark all attached ShaderObjects as needing a rebuild
void ProgramObject::dirtyShaderObjects()
{
    for( unsigned int i=0; i < _shaderObjectList.size() ; ++i )
    {
	_shaderObjectList[i]->dirtyShaderObject();
    }
}


void ProgramObject::addShader( ShaderObject* shadObj )
{
    _shaderObjectList.push_back( shadObj );
    shadObj->addProgObjRef( this );
    dirtyProgramObject();
}


void ProgramObject::setUniform( const char* uniformName, int value )
{
    _univalList.push_back( new UniformValue_int( uniformName, value ) );
}

void ProgramObject::setUniform( const char* uniformName, float value )
{
    _univalList.push_back( new UniformValue_float( uniformName, value ) );
}

void ProgramObject::setUniform( const char* uniformName, osg::Vec2 value )
{
    _univalList.push_back( new UniformValue_Vec2( uniformName, value ) );
}

void ProgramObject::setUniform( const char* uniformName, osg::Vec3 value )
{
    _univalList.push_back( new UniformValue_Vec3( uniformName, value ) );
}

void ProgramObject::setUniform( const char* uniformName, osg::Vec4 value )
{
    _univalList.push_back( new UniformValue_Vec4( uniformName, value ) );
}


void ProgramObject::apply(osg::State& state) const
{
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = Extensions::Get(contextID,true);

    // if there are no ShaderObjects on this ProgramObject,
    // use GL 1.x "fixed functionality" rendering.
    if( !_enabled || _shaderObjectList.empty() )
    {
	if( extensions->isShaderObjectsSupported() )
	{
	    extensions->glUseProgramObject( 0 );
	}
	return;
    }

    if( ! extensions->isShaderObjectsSupported() )
    {
	osg::notify(osg::WARN) << "ARB_shader_objects not supported by OpenGL driver" << std::endl;
        return;
    }

    const osg::FrameStamp* frameStamp = state.getFrameStamp();
    const int frameNumber = (frameStamp) ? frameStamp->getFrameNumber() : -1;

    updateUniforms( frameNumber );

    PerContextProgObj* pcpo = getPCPO( contextID );

    if( pcpo->isDirty() )
    {
	for( unsigned int i=0; i < _shaderObjectList.size() ; ++i )
	{
	    _shaderObjectList[i]->build( contextID );
	}
	pcpo->build();
    }


    // make this glProgramObject part of current GL state
    pcpo->use();

    // consume any pending setUniform messages
    pcpo->applyUniformValues();
}


ProgramObject::PerContextProgObj* ProgramObject::getPCPO(unsigned int contextID) const
{
    if( ! _pcpoList[contextID].valid() )
    {
	_pcpoList[contextID] = new PerContextProgObj( this, contextID );

	// attach all PCSOs to this new PCPO
	for( unsigned int i=0; i < _shaderObjectList.size() ; ++i )
	{
	    _shaderObjectList[i]->attach( contextID, _pcpoList[contextID]->getHandle() );
	}
    }
    return _pcpoList[contextID].get();
}


void ProgramObject::updateUniforms( int frameNumber ) const
{
    if( frameNumber <= _frameNumberOfLastPCPOUpdate )
	return;

    _frameNumberOfLastPCPOUpdate = frameNumber;

    if( _univalList.empty() )
	return;

    for( unsigned int cxt=0; cxt < _pcpoList.size(); ++cxt )
    {
	if( ! _pcpoList[cxt] ) continue;

	PerContextProgObj* pcpo = _pcpoList[cxt].get();
	pcpo->updateUniforms( _univalList );
    }
    _univalList.clear();
}


///////////////////////////////////////////////////////////////////////////
// PCPO : OSG abstraction of the per-context Program Object

ProgramObject::PerContextProgObj::PerContextProgObj(const ProgramObject* progObj, unsigned int contextID ) :
	osg::Referenced(),
	_contextID( contextID )
{
    _progObj = progObj;
    _extensions = Extensions::Get( _contextID, true );
    _glProgObjHandle = _extensions->glCreateProgramObject();
    markAsDirty();
}

ProgramObject::PerContextProgObj::PerContextProgObj(const PerContextProgObj& rhs) :
	osg::Referenced(),
	_contextID( rhs._contextID )
{
    _progObj = rhs._progObj;
    _extensions = rhs._extensions;
    _glProgObjHandle = rhs._glProgObjHandle ;
    _dirty = rhs._dirty;
}

ProgramObject::PerContextProgObj::~PerContextProgObj()
{
}

void ProgramObject::PerContextProgObj::build()
{
    int linked;

    _extensions->glLinkProgram( _glProgObjHandle );
    _extensions->glGetObjectParameteriv(_glProgObjHandle,
	    GL_OBJECT_LINK_STATUS_ARB, &linked);

    _dirty = (linked == 0);
    if( _dirty )
    {
	osg::notify(osg::WARN) << "glLinkProgram FAILED:" << std::endl;
	printInfoLog(osg::WARN);
    }
}


void ProgramObject::PerContextProgObj::use() const
{
    _extensions->glUseProgramObject( _glProgObjHandle  );
}

void ProgramObject::PerContextProgObj::updateUniforms( const UniformValueList& univalList )
{
    // TODO: should the incoming list be appended rather than assigned?
    _univalList = univalList;
}

void ProgramObject::PerContextProgObj::applyUniformValues()
{
    Extensions *ext = _extensions.get();
    for( unsigned int i=0; i < _univalList.size() ; ++i )
    {
	_univalList[i]->apply( ext, _glProgObjHandle );
    }
    _univalList.clear();
}


void ProgramObject::PerContextProgObj::printInfoLog(osg::NotifySeverity severity) const
{
    int blen = 0;	// length of buffer to allocate
    int slen = 0;	// strlen GL actually wrote to buffer

    _extensions->glGetObjectParameteriv(_glProgObjHandle, GL_OBJECT_INFO_LOG_LENGTH_ARB , &blen);
    if (blen > 1)
    {
	GLcharARB* infoLog = new GLcharARB[blen];
	_extensions->glGetInfoLog(_glProgObjHandle, blen, &slen, infoLog);
	osg::notify(severity) << infoLog << std::endl;
	delete infoLog;
    }
}


///////////////////////////////////////////////////////////////////////////
// osgGL2::ShaderObject
///////////////////////////////////////////////////////////////////////////

ShaderObject::ShaderObject() :
	_type(UNKNOWN)
{
}

ShaderObject::ShaderObject(Type type) :
	_type(type)
{
}

ShaderObject::ShaderObject(Type type, const char* sourceText) :
	_type(type)
{
    setShaderSource(sourceText);
}

ShaderObject::ShaderObject(const ShaderObject& rhs, const osg::CopyOp& copyop):
    osg::Object(rhs, copyop)
{
    /*TODO*/
}

ShaderObject::~ShaderObject()
{
    /*TODO*/
}

// mark each PCSO (per-context Shader Object) as needing a recompile
void ShaderObject::dirtyShaderObject()
{
    for( unsigned int cxt=0; cxt < _pcsoList.size(); ++cxt )
    {
        if( ! _pcsoList[cxt] ) continue;

	PerContextShaderObj* pcso = _pcsoList[cxt].get();
	pcso->markAsDirty();
    }

    // mark attached ProgramObjects dirty as well
    for( unsigned int i=0; i < _programObjectList.size(); ++i )
    {
	_programObjectList[i]->dirtyProgramObject();
    }
}

void ShaderObject::setShaderSource( const char* sourceText )
{
    _shaderSource = sourceText;
    dirtyShaderObject();
}

bool ShaderObject::loadShaderSourceFromFile( const char* fileName )
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
    delete text;
    return true;
}


const char* ShaderObject::getTypename() const
{
    switch( getType() )
    {
	case VERTEX:	return "Vertex";
	case FRAGMENT:	return "Fragment";
        default:        return "UNKNOWN";
    }
}


void ShaderObject::build(unsigned int contextID ) const
{
    PerContextShaderObj* pcso = getPCSO( contextID );

    if( pcso->isDirty() )
    {
	pcso->build();
    }
}


ShaderObject::PerContextShaderObj* ShaderObject::getPCSO(unsigned int contextID) const
{
    if( ! _pcsoList[contextID].valid() )
    {
	_pcsoList[contextID] = new PerContextShaderObj( this, contextID );
    }
    return _pcsoList[contextID].get();
}


void ShaderObject::attach(unsigned int contextID, GLhandleARB progObj) const
{
    getPCSO( contextID )->attach( progObj );
}


void ShaderObject::addProgObjRef( ProgramObject* progObj )
{
    _programObjectList.push_back( progObj );
}


///////////////////////////////////////////////////////////////////////////
// PCSO : OSG abstraction of the per-context Shader Object

ShaderObject::PerContextShaderObj::PerContextShaderObj(const ShaderObject* shadObj, unsigned int contextID) :
	osg::Referenced(),
	_contextID( contextID )
{
    _shadObj = shadObj;
    _extensions = Extensions::Get( _contextID, true );
    _glShaderObjHandle = _extensions->glCreateShaderObject( shadObj->getType() );
    markAsDirty();
}

ShaderObject::PerContextShaderObj::PerContextShaderObj(const PerContextShaderObj& rhs) :
	osg::Referenced(),
	_contextID( rhs._contextID )
{
    _shadObj = rhs._shadObj;
    _extensions = rhs._extensions;
    _glShaderObjHandle = rhs._glShaderObjHandle;
    _dirty = rhs._dirty;
}

ShaderObject::PerContextShaderObj::~PerContextShaderObj()
{
}

void ShaderObject::PerContextShaderObj::build()
{
    int compiled;
    const char* sourceText = _shadObj->getShaderSource().c_str();

    _extensions->glShaderSource( _glShaderObjHandle, 1, &sourceText, NULL );
    _extensions->glCompileShader( _glShaderObjHandle );
    _extensions->glGetObjectParameteriv(_glShaderObjHandle,
	    GL_OBJECT_COMPILE_STATUS_ARB, &compiled);

    _dirty = (compiled == 0);
    if( _dirty )
    {
	osg::notify(osg::WARN) << _shadObj->getTypename() << " glCompileShader FAILED:" << std::endl;
	printInfoLog(osg::WARN);
    }
}

void ShaderObject::PerContextShaderObj::attach(GLhandleARB progObj) const
{
    _extensions->glAttachObject( progObj, _glShaderObjHandle );
}

void ShaderObject::PerContextShaderObj::printInfoLog(osg::NotifySeverity severity) const
{
    int blen = 0;	// length of buffer to allocate
    int slen = 0;	// strlen GL actually wrote to buffer

    _extensions->glGetObjectParameteriv(_glShaderObjHandle, GL_OBJECT_INFO_LOG_LENGTH_ARB , &blen);
    if (blen > 1)
    {
	GLcharARB* infoLog = new GLcharARB[blen];
	_extensions->glGetInfoLog(_glShaderObjHandle, blen, &slen, infoLog);
	osg::notify(severity) << infoLog << std::endl;
	delete infoLog;
    }
}

/*EOF*/

