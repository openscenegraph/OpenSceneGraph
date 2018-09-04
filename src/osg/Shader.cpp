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
#include <osg/ContextData>

namespace osg
{

template<typename M>
inline std::string::size_type find_first(const std::string& str, const M& match, std::string::size_type startpos, std::string::size_type endpos=std::string::npos)
{
    std::string::size_type endp = (endpos!=std::string::npos) ? endpos : str.size();

    while(startpos<endp)
    {
        if (match(str[startpos])) return startpos;

        ++startpos;
    }
    return endpos;
}

struct EqualTo
{
    EqualTo(char c): _c(c) {}
    bool operator() (char rhs) const { return rhs==_c; }
    char _c;
};

struct OneOf
{
    OneOf(const char* str) : _str(str) {}
    bool operator() (char rhs) const
    {
        const char* ptr = _str;
        while(*ptr!=0 && rhs!=*ptr) ++ptr;
        return (*ptr!=0);
    }
    const char* _str;
};

struct NotEqualTo
{
    NotEqualTo(char c): _c(c) {}
    bool operator() (char rhs) const { return rhs!=_c; }
    char _c;
};

struct NoneOf
{
    NoneOf(const char* str) : _str(str) {}
    bool operator() (char rhs) const
    {
        const char* ptr = _str;
        while(*ptr!=0 && rhs!=*ptr) ++ptr;
        return (*ptr==0);
    }
    const char* _str;
};

// Replaces all occurrences of "from" with the contents of "to"
// It does only one pass, i.e. new matches created in a position before the current match are not replaced
void replaceAll(std::string& str, const std::string& from, const std::string& to)
{
    std::string::size_type pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos)
    {
        str.replace(pos, from.length(), to);

        pos += to.length();
    }
}

}

using namespace osg;

class GLShaderManager : public GLObjectManager
{
public:
    GLShaderManager(unsigned int contextID) : GLObjectManager("GLShaderManager",contextID) {}

    virtual void deleteGLObject(GLuint globj)
    {
        const GLExtensions* extensions = GLExtensions::Get(_contextID,true);
        if (extensions->isGlslSupported) extensions->glDeleteShader( globj );
    }
};

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
// osg::Shader
///////////////////////////////////////////////////////////////////////////

Shader::Shader(Type type) :
    _type(type),
    _shaderDefinesMode(USE_SHADER_PRAGMA)
{
}

Shader::Shader(Type type, const std::string& source) :
    _type(type),
    _shaderDefinesMode(USE_SHADER_PRAGMA)
{
    setShaderSource( source);
}

Shader::Shader(Type type, ShaderBinary* shaderBinary) :
    _type(type),
    _shaderBinary(shaderBinary),
    _shaderDefinesMode(USE_SHADER_PRAGMA)
{
}


Shader::Shader(const Shader& rhs, const osg::CopyOp& copyop):
    osg::Object( rhs, copyop ),
    _type(rhs._type),
    _shaderFileName(rhs._shaderFileName),
    _shaderSource(rhs._shaderSource),
    _shaderBinary(rhs._shaderBinary),
    _codeInjectionMap(rhs._codeInjectionMap),
    _shaderDefinesMode(rhs._shaderDefinesMode)
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

    _computeShaderDefines();

    dirtyShader();
}

#ifdef OSG_USE_DEPRECATED_API
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
#endif

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
    PerContextShader* pcs = getPCS( state );
    if( pcs ) pcs->compileShader( state );
}


Shader::ShaderObjects::ShaderObjects(const osg::Shader* shader, unsigned int contextID):
    _contextID(contextID),
    _shader(shader)
{
}


Shader::PerContextShader* Shader::ShaderObjects::getPCS(const std::string& defineStr) const
{
    for(PerContextShaders::const_iterator itr = _perContextShaders.begin();
        itr != _perContextShaders.end();
        ++itr)
    {
        if ((*itr)->getDefineString()==defineStr)
        {
            // OSG_NOTICE<<"ShaderPtr = "<<_shader<<" FileName = '"<<_shader->getFileName()<<"' returning PCS "<<itr->get()<<" DefineString = "<<(*itr)->getDefineString()<<std::endl;
            return itr->get();
        }
    }
    // OSG_NOTICE<<"ShaderPtr = "<<_shader<<" FileName = '"<<_shader->getFileName()<<"' returning NULL"<<std::endl;
    return 0;
}

Shader::PerContextShader* Shader::ShaderObjects::createPerContextShader(const std::string& defineStr)
{
    Shader::PerContextShader* pcs = new PerContextShader( _shader, _contextID );
    _perContextShaders.push_back( pcs );
    pcs->setDefineString(defineStr);
    // OSG_NOTICE<<"ShaderPtr = "<<_shader<<" FileName = '"<<_shader->getFileName()<<"' Creating PCS "<<pcs<<" DefineString = ["<<pcs->getDefineString()<<"]"<<std::endl;
    return pcs;
}

void Shader::ShaderObjects::requestCompile()
{
    for(PerContextShaders::const_iterator itr = _perContextShaders.begin();
        itr != _perContextShaders.end();
        ++itr)
    {
        (*itr)->requestCompile();
    }
}


Shader::PerContextShader* Shader::getPCS(osg::State& state) const
{
    if( getType() == UNDEFINED )
    {
        OSG_WARN << "Shader type is UNDEFINED" << std::endl;
        return 0;
    }

    if (!state.supportsShaderRequirements(_shaderRequirements))
    {
        // OSG_NOTICE<<"Shader not supported "<<_shaderRequirements.size()<<std::endl;
        return 0;
    }

    unsigned int contextID = state.getContextID();
    if( ! _pcsList[contextID].valid() )
    {
        _pcsList[contextID] = new ShaderObjects( this, contextID );
    }

    const std::string defineStr = state.getDefineString(getShaderDefines());
    PerContextShader* pcs = _pcsList[contextID]->getPCS(defineStr);
    if (pcs) return pcs;

    if (state.supportsShaderRequirements(_shaderRequirements))
    {
        pcs = _pcsList[contextID]->createPerContextShader(defineStr);
    }

    return pcs;
}


/////////////////////////////////////////////////////////////////////////
// A Shader stores pointers to the osg::Programs to which it is attached,
// so that if the Shader is marked for recompilation with
// Shader::dirtyShader(), the upstream Program can be marked for relinking.
// _programSet does not use ref_ptrs, as that would cause a cyclical
// dependency, and neither the Program nor the Shader would be deleted.

bool Shader::addProgramRef( Program* program )
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lk(_programSetMutex);
    ProgramSet::iterator itr = _programSet.find(program);
    if( itr != _programSet.end() ) return false;

    _programSet.insert( program );
    return true;
}

bool Shader::removeProgramRef( Program* program )
{
    OpenThreads::ScopedLock<OpenThreads::Mutex> lk(_programSetMutex);
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

    OpenThreads::ScopedLock<OpenThreads::Mutex> lk(_programSetMutex);
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
    _extensions = GLExtensions::Get( _contextID, true );
    _glShaderHandle = _extensions->glCreateShader( shader->getType() );
    requestCompile();
}


Shader::PerContextShader::~PerContextShader()
{
    osg::get<GLShaderManager>(_contextID)->scheduleGLObjectForDeletion(_glShaderHandle);
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
            std::string::size_type pos = find_first(source, EqualTo('\n'), previous_pos);
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

#if defined(OSG_GLES2_AVAILABLE) || defined(OSG_GLES3_AVAILABLE)
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
    // if (_shader->getType()==osg::Shader::VERTEX && (state.getUseVertexAttributeAliasing() || state.getUseModelViewAndProjectionUniforms()))
    {
        state.convertVertexShaderSourceToOsgBuiltIns(source);
    }


    GLint compiled = GL_FALSE;

    // OSG_NOTICE<<"Compiling PerContextShader "<<this<<" DefineString="<<getDefineString()<<std::endl;

    if (_defineStr.empty())
    {
        const GLchar* sourceText = reinterpret_cast<const GLchar*>(source.c_str());
        _extensions->glShaderSource( _glShaderHandle, 1, &sourceText, NULL );

        if (osg::getNotifyLevel()>=osg::INFO)
        {
            std::string sourceWithLineNumbers = insertLineNumbers(source);
            OSG_INFO << "\nCompiling A :" << _shader->getTypename()
                    << " source:\n" << sourceWithLineNumbers << std::endl;
        }
    }
    else
    {
        // Convert all windows line endings to \n
        replaceAll(source, "\r\n", " \n");

        std::string versionLine;
        unsigned int lineNum = 0;
        std::string::size_type previous_pos = 0;
        do
        {
            std::string::size_type start_of_line = find_first(source, NoneOf(" \t"), previous_pos);
            std::string::size_type end_of_line = (start_of_line != std::string::npos) ? find_first(source, OneOf("\n\r"), start_of_line) : std::string::npos;
            if (end_of_line != std::string::npos)
            {
                // OSG_NOTICE<<"A Checking line "<<lineNum<<" ["<<source.substr(start_of_line, end_of_line-start_of_line)<<"]"<<std::endl;
                if ((end_of_line-start_of_line)>=8 && source.compare(start_of_line, 8, "#version")==0)
                {
                    versionLine = source.substr(start_of_line, end_of_line-start_of_line+1);
                    if (versionLine[versionLine.size()-1]!='\n') versionLine.push_back('\n');

                    source.insert(start_of_line, "// following version spec has been automatically reassigned to start of source list: ");

                    break;
                }
                previous_pos = end_of_line+1<source.size() ? end_of_line+1 : std::string::npos;
            }
            else
            {
                // OSG_NOTICE<<"B Checking line "<<lineNum<<" ["<<source.substr(start_of_line, end_of_line-start_of_line)<<"]"<<std::endl;
                previous_pos = std::string::npos;
            }
            ++lineNum;

        } while (previous_pos != std::string::npos);

        if (!versionLine.empty())
        {
            const GLchar* sourceText[3];
            sourceText[0] = reinterpret_cast<const GLchar*>(versionLine.c_str());
            sourceText[1] = reinterpret_cast<const GLchar*>(_defineStr.c_str());
            sourceText[2] = reinterpret_cast<const GLchar*>(source.c_str());
            _extensions->glShaderSource( _glShaderHandle, 3, sourceText, NULL );

            if (osg::getNotifyLevel()>=osg::INFO)
            {
                std::string sourceWithLineNumbers = insertLineNumbers(versionLine+_defineStr+source);
                OSG_INFO << "\nCompiling B: " << _shader->getTypename()
                        << " source:\n" << sourceWithLineNumbers << std::endl;
            }

            // OSG_NOTICE<<"   Version Line : ["<<std::endl<<versionLine<<"]"<<std::endl;
            // OSG_NOTICE<<"   DefineStr : ["<<std::endl<<_defineStr<<"]"<<std::endl;
            // OSG_NOTICE<<"   Source : ["<<std::endl<<source<<"]"<<std::endl;
        }
        else
        {
            const GLchar* sourceText[2];
            sourceText[0] = reinterpret_cast<const GLchar*>(_defineStr.c_str());
            sourceText[1] = reinterpret_cast<const GLchar*>(source.c_str());
            _extensions->glShaderSource( _glShaderHandle, 2, sourceText, NULL );


            if (osg::getNotifyLevel()>=osg::INFO)
            {
                std::string sourceWithLineNumbers = insertLineNumbers(_defineStr+source);
                OSG_INFO << "\nCompiling C: " << _shader->getTypename()
                        << " source:\n" << sourceWithLineNumbers << std::endl;
            }

            // OSG_NOTICE<<"   DefineStr : ["<<std::endl<<_defineStr<<"]"<<std::endl;
            // OSG_NOTICE<<"   Source : ["<<std::endl<<source<<"]"<<std::endl;
        }
    }
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

        _extensions->debugObjectLabel(GL_SHADER, _glShaderHandle, _shader->getName());
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

void Shader::_parseShaderDefines(const std::string& str, ShaderDefines& defines)
{
    OSG_INFO<<"Shader::_parseShaderDefines("<<str<<")"<<std::endl;
    std::string::size_type start_of_parameter = 0;
    do
    {
        // skip spaces, tabs, commans
        start_of_parameter = find_first(str, NoneOf(" \t,"), start_of_parameter);
        if (start_of_parameter==std::string::npos) break;

        // find end of the parameter
        std::string::size_type end_of_parameter = find_first(str, OneOf(" \t,)"), start_of_parameter);

        if (end_of_parameter!=std::string::npos)
        {
            std::string::size_type start_of_open_brackets = find_first(str, EqualTo('('), start_of_parameter);
            if (start_of_open_brackets<end_of_parameter) ++end_of_parameter;
        }
        else
        {
            end_of_parameter = str.size();
        }

        if (start_of_parameter<end_of_parameter)
        {
            std::string parameter = str.substr(start_of_parameter, end_of_parameter-start_of_parameter);
            defines.insert(parameter);
            OSG_INFO<<"   defines.insert("<<parameter<<")"<<std::endl;
        }

        start_of_parameter = end_of_parameter+1;

    } while (start_of_parameter<str.size());
}


void Shader::_computeShaderDefines()
{
    if (_shaderDefinesMode==USE_MANUAL_SETTINGS) return;

    std::string::size_type pos = 0;

    _shaderDefines.clear();
    _shaderRequirements.clear();

    while ((pos = _shaderSource.find("#pragma", pos))!=std::string::npos)
    {
        // skip over #pragma characters
        pos += 7;
        std::string::size_type eol = find_first(_shaderSource, OneOf("\n\r"), pos);

        std::string::size_type first_chararcter = find_first(_shaderSource, NoneOf(" \t"), pos, eol);

        OSG_INFO<<"\nFound pragma line ["<<_shaderSource.substr(first_chararcter, eol-first_chararcter)<<"]"<<std::endl;

        if (first_chararcter<eol)
        {
            std::string::size_type end_of_keyword = find_first(_shaderSource, OneOf(" \t("), first_chararcter, eol);

            std::string keyword = _shaderSource.substr(first_chararcter, end_of_keyword-first_chararcter);

            std::string::size_type open_brackets = find_first(_shaderSource, EqualTo('('), end_of_keyword, eol);
            if (open_brackets<eol)
            {
                std::string str(_shaderSource, open_brackets+1, eol-open_brackets-1);

                // OSG_NOTICE<<"    parameter str = ["<<str<<"]"<<std::endl;
                if (keyword == "import_defines") _parseShaderDefines(str, _shaderDefines);
                else if (keyword == "requires") _parseShaderDefines(str, _shaderRequirements);
                else {
                    //OSG_NOTICE<<"  keyword not matched ["<<keyword<<"]"<<std::endl;
                    _parseShaderDefines(str, _shaderDefines);
                }
#if 1
                for(ShaderDefines::iterator itr = _shaderDefines.begin();
                    itr != _shaderDefines.end();
                    ++itr)
                {
                    OSG_INFO<<"      define ["<<*itr<<"]"<<std::endl;
                }

                for(ShaderDefines::iterator itr = _shaderRequirements.begin();
                    itr != _shaderRequirements.end();
                    ++itr)
                {
                    OSG_INFO<<"      requirements ["<<*itr<<"]"<<std::endl;
                }
#endif

            }
#if 1
            else
            {
                OSG_INFO<<"    Found keyword ["<<keyword<<"] but not matched ()\n"<<std::endl;
            }
#endif
        }

        pos = eol;
    }

}

