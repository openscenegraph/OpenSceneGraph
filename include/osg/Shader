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
*/

/* file:   include/osg/Shader
 * author: Mike Weiblen 2008-01-02
 *         Holger Helmich 2010-10-21
*/

#ifndef OSG_SHADER
#define OSG_SHADER 1


#include <osg/GLExtensions>
#include <osg/Object>
#include <osg/buffered_value>

#include <set>
#include <map>

namespace osg {

class Program;

// set of shader define strings that the shader is dependent upon.
typedef std::set<std::string> ShaderDefines;

/** Simple class for wrapping up the data used in OpenGL ES 2's glShaderBinary calls.
  * ShaderBinary is set up with the binary data then assigned to one or more osg::Shader. */
class OSG_EXPORT ShaderBinary : public osg::Object
{
    public:

        ShaderBinary();

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        ShaderBinary(const ShaderBinary& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(osg, ShaderBinary);

        /** Allocated a data buffer of specified size.*/
        void allocate(unsigned int size);

        /** Assign shader binary data, copying the specified data into locally stored data buffer, the original data can then be deleted.*/
        void assign(unsigned int size, const unsigned char* data);

        /** Get the size of the shader binary data.*/
        unsigned int getSize() const { return static_cast<unsigned int>(_data.size()); }

        /** Get a ptr to the shader binary data.*/
        unsigned char* getData() { return _data.empty() ? 0 : &(_data.front()); }

        /** Get a const ptr to the shader binary data.*/
        const unsigned char* getData() const { return _data.empty() ? 0 : &(_data.front()); }

        /** Read shader binary from file.
          * Return the resulting Shader or 0 if no valid shader binary could be read.*/
        static ShaderBinary* readShaderBinaryFile(const std::string& fileName);

    protected:

        typedef std::vector<unsigned char> Data;
        Data _data;
};


///////////////////////////////////////////////////////////////////////////
/** osg::Shader is an application-level abstraction of an OpenGL glShader.
  * It is a container to load the shader source code text and manage its
  * compilation.
  * An osg::Shader may be attached to more than one osg::Program.
  * Shader will automatically manage per-context instancing of the
  * internal objects, if that is necessary for a particular display
  * configuration.
  */

class OSG_EXPORT Shader : public osg::Object
{
    public:

        enum Type {
            VERTEX = GL_VERTEX_SHADER,
            TESSCONTROL = GL_TESS_CONTROL_SHADER,
            TESSEVALUATION = GL_TESS_EVALUATION_SHADER,
            GEOMETRY = GL_GEOMETRY_SHADER,
            FRAGMENT = GL_FRAGMENT_SHADER,
            COMPUTE = GL_COMPUTE_SHADER,
            UNDEFINED = -1
        };

        Shader(Type type = UNDEFINED);
        Shader(Type type, const std::string& source );
        Shader(Type type, ShaderBinary* shaderBinary );

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        Shader(const Shader& rhs, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(osg, Shader);

        int compare(const Shader& rhs) const;

        /** Set the Shader type as an enum. */
        bool setType(Type t);

        /** Get the Shader type as an enum. */
        inline Type getType() const { return _type; }

        /** Get the Shader type as a descriptive string. */
        const char* getTypename() const;


        /** Set file name for the shader source code. */
        inline void setFileName(const std::string& fileName) { _shaderFileName = fileName; }

        /** Get filename to which the shader source code belongs. */
        inline const std::string& getFileName() const { return _shaderFileName; }


        /** Set the Shader's source code text from a string. */
        void setShaderSource(const std::string& sourceText);

        /** Query the shader's source code text */
        inline const std::string& getShaderSource() const { return _shaderSource; }


        enum ShaderDefinesMode
        {
            USE_SHADER_PRAGMA,
            USE_MANUAL_SETTINGS
        };

        void setShaderDefinesMode(ShaderDefinesMode sdm) { _shaderDefinesMode = sdm; }
        ShaderDefinesMode getShaderDefinesMode() const { return _shaderDefinesMode; }


        void setShaderDefines(const ShaderDefines& shaderDefs) { _shaderDefines = shaderDefs; }
        ShaderDefines& getShaderDefines() { return _shaderDefines; }
        const ShaderDefines& getShaderDefines() const { return _shaderDefines; }

        void setShaderRequirements(const ShaderDefines& shaderDefs) { _shaderRequirements = shaderDefs; }
        ShaderDefines& getShaderRequirements() { return _shaderRequirements; }
        const ShaderDefines& getShaderRequirements() const { return _shaderRequirements; }


        /** Set the Shader using a ShaderBinary. */
        void setShaderBinary(ShaderBinary* shaderBinary) { _shaderBinary = shaderBinary; }

        /** Get the Shader's ShaderBinary, return NULL if none is assigned. */
        ShaderBinary* getShaderBinary() { return _shaderBinary.get(); }

        /** Get the const Shader's ShaderBinary, return NULL if none is assigned. */
        const ShaderBinary* getShaderBinary() const { return _shaderBinary.get(); }

#ifdef OSG_USE_DEPRECATED_API
        /** Deorecated use osgDB::readRefShaderFile().*/
        static Shader* readShaderFile( Type type, const std::string& fileName );

        /** Deorecated use osgDB::readRefShaderFile(). */
        bool loadShaderSourceFromFile( const std::string& fileName );
#endif

        /** The code injection map used when generating the main shader during main shader composition.*/
        typedef std::multimap<float, std::string> CodeInjectionMap;

        /** Add code injection that will be placed in the main shader to enable support for this shader.
          * The position is set up so that code to be inserted before the main() will have a negative value,
          * a position between 0 and 1.0 will be inserted in main() and a position greater than 1.0 will
          * be placed after the main().
          * During shader composition all the code injections are sorted in ascending order and then
          * placed in the appropriate section of the main shader. */
        void addCodeInjection(float position, const std::string& code) { _codeInjectionMap.insert(CodeInjectionMap::value_type(position, code)); }

        /** Get the code injection map.*/
        CodeInjectionMap& getCodeInjectionMap() { return _codeInjectionMap; }

        /** Get the const code injection map.*/
        const CodeInjectionMap& getCodeInjectionMap() const { return _codeInjectionMap; }



        /** Resize any per context GLObject buffers to specified size. */
        virtual void resizeGLObjectBuffers(unsigned int maxSize);

        /** release OpenGL objects in specified graphics context if State
            object is passed, otherwise release OpenGL objects for all graphics context if
            State object pointer NULL.*/
        void releaseGLObjects(osg::State* state=0) const;

        /** Mark our PCSs as needing recompilation.
          * Also mark Programs that depend on us as needing relink */
        void dirtyShader();

        /** If needed, compile the PCS's glShader */
        void compileShader(osg::State& state) const;

        static Shader::Type getTypeId( const std::string& tname );

    public:
        /** PerContextShader (PCS) is an OSG-internal encapsulation of glShader per-GL context. */
        class OSG_EXPORT PerContextShader : public osg::Referenced
        {
            public:
                PerContextShader(const Shader* shader, unsigned int contextID);

                void setDefineString(const std::string& defStr) { _defineStr = defStr; }
                const std::string& getDefineString() const { return _defineStr; }

                GLuint getHandle() const {return _glShaderHandle;}

                void requestCompile();
                void compileShader(osg::State& state);
                bool needsCompile() const {return _needsCompile;}
                bool isCompiled() const {return _isCompiled;}
                bool getInfoLog( std::string& infoLog ) const;

                /** Attach our glShader to a glProgram */
                void attachShader(GLuint program) const;

                /** Detach our glShader from a glProgram */
                void detachShader(GLuint program) const;

            protected:        /*methods*/
                ~PerContextShader();

            protected:        /*data*/
                /** Pointer to our parent osg::Shader */
                const Shader* _shader;

                /** Pointer to this context's extension functions. */
                osg::ref_ptr<osg::GLExtensions> _extensions;

                /** Handle to the actual glShader. */
                GLuint _glShaderHandle;

                /** Define string passed on to Shaders to help configure them.*/
                std::string _defineStr;

                /** Does our glShader need to be recompiled? */
                bool _needsCompile;

                /** Is our glShader successfully compiled? */
                bool _isCompiled;

                const unsigned int _contextID;

            private:
                PerContextShader();        // disallowed
                PerContextShader(const PerContextShader&);        // disallowed
                PerContextShader& operator=(const PerContextShader&);        // disallowed
        };


        struct OSG_EXPORT ShaderObjects : public osg::Referenced
        {
            typedef std::vector< osg::ref_ptr<PerContextShader> > PerContextShaders;

            ShaderObjects(const Shader* shader, unsigned int contextID);

            unsigned int        _contextID;
            const Shader*       _shader;
            mutable PerContextShaders  _perContextShaders;

            PerContextShader* getPCS(const std::string& defineStr) const;
            PerContextShader* createPerContextShader(const std::string& defineStr);
            void requestCompile();
        };

        PerContextShader* getPCS(osg::State& state) const;

    protected:        /*methods*/
        virtual ~Shader();


        friend class osg::Program;
        bool addProgramRef( osg::Program* program );
        bool removeProgramRef( osg::Program* program );

        void _computeShaderDefines();
        void _parseShaderDefines(const std::string& str, ShaderDefines& defines);


        protected:        /*data*/
        Type                            _type;
        std::string                     _shaderFileName;
        std::string                     _shaderSource;
        osg::ref_ptr<ShaderBinary>      _shaderBinary;

        CodeInjectionMap                _codeInjectionMap;

        // ShaderDefines variables
        ShaderDefinesMode               _shaderDefinesMode;
        ShaderDefines                   _shaderDefines;
        ShaderDefines                   _shaderRequirements;

        /** osg::Programs that this osg::Shader is attached to */
        typedef std::set< osg::Program* > ProgramSet;
        ProgramSet                      _programSet;
        OpenThreads::Mutex _programSetMutex;
        mutable osg::buffered_value< osg::ref_ptr<ShaderObjects> > _pcsList;

    private:
        Shader& operator=(const Shader&);        // disallowed
};


class OSG_EXPORT ShaderComponent : public osg::Object
{
    public:

        ShaderComponent();
        ShaderComponent(const ShaderComponent& sc,const CopyOp& copyop=CopyOp::SHALLOW_COPY);

        META_Object(osg, ShaderComponent);

        unsigned int addShader(osg::Shader* shader);
        void removeShader(unsigned int i);

        osg::Shader* getShader(unsigned int i) { return _shaders[i].get(); }
        const osg::Shader* getShader(unsigned int i) const { return _shaders[i].get(); }

        unsigned int getNumShaders() const { return static_cast<unsigned int>(_shaders.size()); }

        virtual void compileGLObjects(State& state) const;
        virtual void resizeGLObjectBuffers(unsigned int maxSize);
        virtual void releaseGLObjects(State* state=0) const;

    protected:

       typedef std::vector< osg::ref_ptr<osg::Shader> >  Shaders;
       Shaders _shaders;
};


}

#endif
