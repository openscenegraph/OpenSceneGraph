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
 * author:	Mike Weiblen 2003-07-14
 *
 * See http://www.3dlabs.com/opengl2/ for more information regarding
 * the OpenGL Shading Language.
*/

#include <fstream>

#include <osg/Notify>
#include <osg/State>
#include <osg/ref_ptr>

#include <osgGL2/ProgramObject>
#include <osgGL2/Extensions>

using namespace osgGL2;

///////////////////////////////////////////////////////////////////////////
// static cache of deleted GL2 objects which may only 
// by actually deleted in the correct GL context.

typedef std::vector<GLhandleARB> GL2ObjectVector;
typedef std::map<unsigned int, GL2ObjectVector> DeletedGL2ObjectCache;
static DeletedGL2ObjectCache s_deletedGL2ObjectCache;

void osgGL2::DeleteObject(unsigned int contextID, GLhandleARB handle)
{
    if (handle!=0)
    {
        // add handle to the cache for the appropriate context.
        s_deletedGL2ObjectCache[contextID].push_back(handle);
    }
}

void osgGL2::FlushDeletedGL2Objects(unsigned int contextID)
{
    const Extensions* extensions = Extensions::Get(contextID,true);

    if (!extensions->isShaderObjectsSupported())
        return;

    DeletedGL2ObjectCache::iterator citr = s_deletedGL2ObjectCache.find(contextID);
    if( citr != s_deletedGL2ObjectCache.end() )
    {
        GL2ObjectVector vpObjectSet;

        // this swap will transfer the content of and empty citr->second
        // in one quick pointer change.
        vpObjectSet.swap(citr->second);
        for( GL2ObjectVector::iterator titr=vpObjectSet.begin();
            titr!=vpObjectSet.end();
            ++titr )
        {
            extensions->glDeleteObject( *titr );
        }
    }
}

///////////////////////////////////////////////////////////////////////////
// osgGL2::ProgramObject
///////////////////////////////////////////////////////////////////////////

ProgramObject::ProgramObject()
{
}


ProgramObject::ProgramObject(const ProgramObject& rhs, const osg::CopyOp& copyop):
    osg::StateAttribute(rhs, copyop)
{
}


// virtual
ProgramObject::~ProgramObject()
{
    for( unsigned int cxt=0; cxt<_pcpoList.size(); ++cxt )
    {
        if( ! _pcpoList[cxt] ) continue;

	PerContextProgObj* pcpo = _pcpoList[cxt].get();

	DeleteObject( cxt, pcpo->getHandle() );
	// TODO add shader objects to delete list.
	_pcpoList[cxt] = 0;
    }
}


// mark each PCPO (per-context ProgramObject) as needing a relink
void ProgramObject::dirtyProgramObject()
{
    for( unsigned int cxt=0; cxt<_pcpoList.size(); ++cxt )
    {
        if( ! _pcpoList[cxt] ) continue;

	PerContextProgObj* pcpo = _pcpoList[cxt].get();
	pcpo->markAsDirty();
    }
}

void ProgramObject::addShader( ShaderObject* shader )
{
    _shaderObjectList.push_back(shader);
    dirtyProgramObject();
}


void ProgramObject::apply(osg::State& state) const
{
    const unsigned int contextID = state.getContextID();
    const Extensions* extensions = Extensions::Get(contextID,true);

    if (!extensions->isShaderObjectsSupported())
        return;

    // if there are no ShaderObjects attached (ie it is "empty"),
    // indicates to use GL 1.x "fixed functionality" rendering.
    if( _shaderObjectList.size() == 0 )
    {
	// glProgramObject handle 0 == GL 1.x fixed functionality
        extensions->glUseProgramObject( 0 );
	return;
    }

    PerContextProgObj* pcpo = getPCPO( contextID );

    // if the first apply(), attach glShaderObjects to the glProgramObject
    if( pcpo->isUnattached() )
    {
	for( unsigned int i=0; i < _shaderObjectList.size() ; ++i )
	{
	    if( ! _shaderObjectList[i] ) continue;
	    _shaderObjectList[i]->attach( contextID, pcpo->getHandle() );
	}
	pcpo->markAsAttached();
    }

    //  if we're dirty, build all attached objects, then build ourself
    if( pcpo->isDirty() )
    {
	for( unsigned int i=0; i < _shaderObjectList.size() ; ++i )
	{
	    if( ! _shaderObjectList[i] ) continue;
	    _shaderObjectList[i]->build( contextID );
	}

	if( pcpo->build() )
	    pcpo->markAsClean();
    }

    // make this glProgramObject part of current GL state
    pcpo->use();
}


ProgramObject::PerContextProgObj* ProgramObject::getPCPO(unsigned int contextID) const
{
    if( ! _pcpoList[contextID].valid() )
    {
	_pcpoList[contextID] = new PerContextProgObj( this, Extensions::Get(contextID,true) );
    }
    return _pcpoList[contextID].get();
}

///////////////////////////////////////////////////////////////////////////
// PCPO : OSG abstraction of the per-context Program Object

ProgramObject::PerContextProgObj::PerContextProgObj(const ProgramObject* parent, Extensions* extensions) :
	Referenced()
{
    _parent = parent;
    _extensions = extensions;
    _handle= _extensions->glCreateProgramObject();
    markAsDirty();
    _unattached = true;
}

ProgramObject::PerContextProgObj::PerContextProgObj(const PerContextProgObj& rhs) :
	Referenced()
{
    _parent = rhs._parent;
    _extensions = rhs._extensions;
    _handle= rhs._handle;
    _dirty = rhs._dirty;
    _unattached = rhs._unattached;
}

ProgramObject::PerContextProgObj::~PerContextProgObj()
{
}

bool ProgramObject::PerContextProgObj::build() const
{
    _extensions->glLinkProgram(_handle);
    return true;
}

void ProgramObject::PerContextProgObj::use() const
{
    _extensions->glUseProgramObject( _handle );
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
    for( unsigned int cxt=0; cxt<_pcsoList.size(); ++cxt )
    {
        if( ! _pcsoList[cxt] ) continue;

	PerContextShaderObj* pcso = _pcsoList[cxt].get();
	pcso->markAsDirty();
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



bool ShaderObject::build(unsigned int contextID ) const
{
    PerContextShaderObj* pcso = getPCSO( contextID );

    if( pcso->isDirty() )
    {
	if( pcso->build() )
	    pcso->markAsClean();
    }
    return true; /*TODO*/
}


ShaderObject::PerContextShaderObj* ShaderObject::getPCSO(unsigned int contextID) const
{
    if( ! _pcsoList[contextID].valid() )
    {
	_pcsoList[contextID] = new PerContextShaderObj( this, Extensions::Get(contextID,true) );
    }
    return _pcsoList[contextID].get();
}

void ShaderObject::attach(unsigned int contextID, GLhandleARB progObj) const
{
    getPCSO( contextID )->attach( progObj );
}

///////////////////////////////////////////////////////////////////////////
// PCSO : OSG abstraction of the per-context Shader Object

ShaderObject::PerContextShaderObj::PerContextShaderObj(const ShaderObject* parent, Extensions* extensions) :
	Referenced()
{
    _parent = parent;
    _extensions = extensions;
    _handle = _extensions->glCreateShaderObject( parent->getType() );
    markAsDirty();
}

ShaderObject::PerContextShaderObj::PerContextShaderObj(const PerContextShaderObj& rhs) :
	Referenced()
{
    _parent = rhs._parent;
    _extensions = rhs._extensions;
    _handle = rhs._handle;
    _dirty = rhs._dirty;
}

ShaderObject::PerContextShaderObj::~PerContextShaderObj()
{
}

bool ShaderObject::PerContextShaderObj::build()
{
    const char* sourceText = _parent->getShaderSource().c_str();

    _extensions->glShaderSource( _handle, 1, &sourceText, NULL );
    _extensions->glCompileShader( _handle );

    // _extensions->glAttachObject( _handle, vertShaderObject );

    return true;
}

void ShaderObject::PerContextShaderObj::attach(GLhandleARB progObj)
{
    _extensions->glAttachObject(progObj, _handle);
}

/*EOF*/

