/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 * Copyright (C) 2003-2005 3Dlabs Inc. Ltd.
 * Copyright (C) 2004-2005 Nathan Cournia
 * Copyright (C) 2008 Zebra Imaging
 * Copyright (C) 2010 VIRES Simulationstechnologie GmbH
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

/* file:   src/osg/Shader.cpp
 * author: Mike Weiblen 2008-01-02
 *         Holger Helmich 2010-10-21
*/

#include <fstream>
#include <list>
#include <sstream>
#include <iomanip>

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


///////////////////////////////////////////////////////////////////////////////////
//
//  ShaderComponent
//
ShaderComponent::ShaderComponent()
{
}

ShaderComponent::ShaderComponent(const ShaderComponent& sc,const CopyOp& copyop):
    osg::Object(sc, copyop),
    _shaders(sc._shaders)
{
}

unsigned int ShaderComponent::addShader(osg::Shader* shader)
{
    for(unsigned int i=0; i<_shaders.size();++i)
    {
        if (_shaders[i]==shader) return i;
    }
    _shaders.push_back(shader);
    return _shaders.size()-1;
}

void ShaderComponent::removeShader(unsigned int i)
{
    _shaders.erase(_shaders.begin()+i);
}

void ShaderComponent::compileGLObjects(State& state) const
{
    for(Shaders::const_iterator itr = _shaders.begin();
        itr != _shaders.end();
        ++itr)
    {
        (*itr)->compileShader(state);
    }
}

void ShaderComponent::resizeGLObjectBuffers(unsigned int maxSize)
{
    for(Shaders::const_iterator itr = _shaders.begin();
        itr != _shaders.end();
        ++itr)
    {
        (*itr)->resizeGLObjectBuffers(maxSize);
    }
}

void ShaderComponent::releaseGLObjects(State* state) const
{
    for(Shaders::const_iterator itr = _shaders.begin();
        itr != _shaders.end();
        ++itr)
    {
        (*itr)->releaseGLObjects(state);
    }
}



///////////////////////////////////////////////////////////////////////////////////
//
//  ShaderBinary
//
ShaderBinary::ShaderBinary()
{
}

ShaderBinary::ShaderBinary(const ShaderBinary& rhs, const osg::CopyOp& copyop):
    osg::Object(rhs, copyop),
    _data(rhs._data)
{
}

void ShaderBinary::allocate(unsigned int size)
{
    _data.clear();
    _data.resize(size);
}

void ShaderBinary::assign(unsigned int size, const unsigned char* data)
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

ShaderBinary* ShaderBinary::readShaderBinaryFile(const std::string& fileName)
{
    std::ifstream fin;
    fin.open(fileName.c_str(), std::ios::binary);
    if (!fin) return 0;

    fin.seekg(0, std::ios::end);
    int length = fin.tellg();
    if (length==0) return 0;

    osg::ref_ptr<ShaderBinary> shaderBinary = new osg::ShaderBinary;
    shaderBinary->allocate(length);
    fin.seekg(0, std::ios::beg);
    fin.read(reinterpret_cast<char*>(shaderBinary->getData()), length);
    fin.close();

    return shaderBinary.release();
}

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

void Shader::discardDeletedGlShaders(unsigned int contextID)
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lock(s_mutex_deletedGlShaderCache);

    GlShaderHandleList& pList = s_deletedGlShaderCache[contextID];
    pList.clear();
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

Shader::Shader(Type type, ShaderBinary* shaderBinary) :
    _type(type),
    _shaderBinary(shaderBinary)
{
}


Shader::Shader(const Shader& rhs, const osg::CopyOp& copyop):
    osg::Object( rhs, copyop ),
    _type(rhs._type),
    _shaderFileName(rhs._shaderFileName),
    _shaderSource(rhs._shaderSource),
    _shaderBinary(rhs._shaderBinary),
    _codeInjectionMap(rhs._codeInjectionMap)
{
}

Shader::~Shader()
{
}

bool Shader::setType(Type t)
{
    if (_type==t) return true;

    if (_type != UNDEFINED)
    {
        OSG_WARN << "cannot change type of Shader" << std::endl;
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

    if( getShaderBinary() < rhs.getShaderBinary() ) return -1;
    if( rhs.getShaderBinary() < getShaderBinary() ) return 1;

    if( getFileName() < rhs.getFileName() ) return -1;
    if( rhs.getFileName() < getFileName() ) return 1;
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
        OSG_WARN<<"Error: can't open file \""<<fileName<<"\""<<std::endl;
        return false;
    }

    OSG_INFO<<"Loading shader source file \""<<fileName<<"\""<<std::endl;
    _shaderFileName = fileName;

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
        case TESSCONTROL: return "TESSCONTROL";
        case TESSEVALUATION: return "TESSEVALUATION";
        case GEOMETRY:  return "GEOMETRY";
        case FRAGMENT:  return "FRAGMENT";
        case COMPUTE:  return "COMPUTE";
        default:        return "UNDEFINED";
    }
}


Shader::Type Shader::getTypeId( const std::string& tname )
{
    if( tname == "VERTEX" )     return VERTEX;
    if( tname == "TESSCONTROL" ) return TESSCONTROL;
    if( tname == "TESSEVALUATION") return TESSEVALUATION;
    if( tname == "GEOMETRY" )   return GEOMETRY;
    if( tname == "FRAGMENT" )   return FRAGMENT;
    if( tname == "COMPUTE" )   return COMPUTE;
    return UNDEFINED;
}

void Shader::resizeGLObjectBuffers(unsigned int maxSize)
{
    _pcsList.resize(maxSize);
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

void Shader::compileShader( osg::State& state ) const
{
    PerContextShader* pcs = getPCS( state.getContextID() );
    if( pcs ) pcs->compileShader( state );
}


Shader::PerContextShader* Shader::getPCS(unsigned int contextID) const
{
    if( getType() == UNDEFINED )
    {
        OSG_WARN << "Shader type is UNDEFINED" << std::endl;
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

void Shader::detachShader(unsigned int contextID, GLuint program) const
{
    PerContextShader* pcs = getPCS( contextID );
    if( pcs ) pcs->detachShader( program );
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

namespace
{
    std::string insertLineNumbers(const std::string& source)
    {
        if (source.empty()) return source;

        unsigned int lineNum = 1;       // Line numbers start at 1
        std::ostringstream ostr;

        std::string::size_type previous_pos = 0;
        do
        {
            std::string::size_type pos = source.find_first_of("\n", previous_pos);
            if (pos != std::string::npos)
            {
                ostr << std::setw(5)<<std::right<<lineNum<<": "<<source.substr(previous_pos, pos-previous_pos)<<std::endl;
                previous_pos = pos+1<source.size() ? pos+1 : std::string::npos;
            }
            else
            {
                ostr << std::setw(5)<<std::right<<lineNum<<": "<<source.substr(previous_pos, std::string::npos)<<std::endl;
                previous_pos = std::string::npos;
            }
            ++lineNum;

        } while (previous_pos != std::string::npos);

        return ostr.str();
    }
}

void Shader::PerContextShader::compileShader(osg::State& state)
{
    if( ! _needsCompile ) return;
    _needsCompile = false;

#if defined(OSG_GLES2_AVAILABLE)
    if (_shader->getShaderBinary())
    {
        GLint numFormats = 0;
        glGetIntegerv(GL_NUM_SHADER_BINARY_FORMATS, &numFormats);

        if (numFormats>0)
        {
            std::vector<GLint> formats(numFormats);
            glGetIntegerv(GL_SHADER_BINARY_FORMATS, &formats[0]);

            for(GLint i=0; i<numFormats; ++i)
            {
                OSG_NOTICE<<"  format="<<formats[i]<<std::endl;
                GLenum shaderBinaryFormat = formats[i];
                glShaderBinary(1, &_glShaderHandle, shaderBinaryFormat, _shader->getShaderBinary()->getData(), _shader->getShaderBinary()->getSize());
                if (glGetError() == GL_NO_ERROR)
                {
                    _isCompiled = true;
                    return;
                }
            }

            if (_shader->getShaderSource().empty())
            {
                OSG_WARN<<"Warning: No suitable shader of supported format by GLES driver found in shader binary, unable to compile shader."<<std::endl;
                _isCompiled = false;
                return;
            }
            else
            {
                OSG_NOTICE<<"osg::Shader::compileShader(): No suitable shader of supported format by GLES driver found in shader binary, falling back to shader source."<<std::endl;
            }
        }
        else
        {
            if (_shader->getShaderSource().empty())
            {
                OSG_WARN<<"Warning: No shader binary formats supported by GLES driver, unable to compile shader."<<std::endl;
                _isCompiled = false;
                return;
            }
            else
            {
                OSG_NOTICE<<"osg::Shader::compileShader(): No shader binary formats supported by GLES driver, falling back to shader source."<<std::endl;
            }
        }
    }
#endif

    std::string source = _shader->getShaderSource();
    if (_shader->getType()==osg::Shader::VERTEX && (state.getUseVertexAttributeAliasing() || state.getUseModelViewAndProjectionUniforms()))
    {
        state.convertVertexShaderSourceToOsgBuiltIns(source);
    }


    if (osg::getNotifyLevel()>=osg::INFO)
    {
        std::string sourceWithLineNumbers = insertLineNumbers(source);
        OSG_INFO << "\nCompiling " << _shader->getTypename()
                 << " source:\n" << sourceWithLineNumbers << std::endl;
    }

    GLint compiled = GL_FALSE;
    const GLchar* sourceText = reinterpret_cast<const GLchar*>(source.c_str());
    _extensions->glShaderSource( _glShaderHandle, 1, &sourceText, NULL );
    _extensions->glCompileShader( _glShaderHandle );
    _extensions->glGetShaderiv( _glShaderHandle, GL_COMPILE_STATUS, &compiled );

    _isCompiled = (compiled == GL_TRUE);
    if( ! _isCompiled )
    {
        OSG_WARN << _shader->getTypename() << " glCompileShader \""
            << _shader->getName() << "\" FAILED" << std::endl;

        std::string infoLog;
        if( getInfoLog(infoLog) )
        {
            OSG_WARN << _shader->getTypename() << " Shader \""
                << _shader->getName() << "\" infolog:\n" << infoLog << std::endl;
        }
    }
    else
    {
        std::string infoLog;
        if( getInfoLog(infoLog) )
        {
            OSG_INFO << _shader->getTypename() << " Shader \""
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
