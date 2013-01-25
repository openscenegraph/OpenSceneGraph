#include <sstream>
#include <osg/Shader>
#include <osg/Notify>
#include <osg/GL>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgDB/fstream>


class ReaderWriterGLSL : public osgDB::ReaderWriter
{
    public:

        ReaderWriterGLSL()
        {
            supportsExtension("gl","OpenGL Shader Language format");
            supportsExtension("frag","OpenGL Shader Language format");
            supportsExtension("vert","OpenGL Shader Language format");
            supportsExtension("geom","OpenGL Shader Language format");
            supportsExtension("glsl","OpenGL Shader Language format");
            supportsExtension("tctrl","OpenGL Shader Language format");
            supportsExtension("teval","OpenGL Shader Language format");
            supportsExtension("compute","OpenGL Shader Language format");
        }

        virtual const char* className() const { return "GLSL Shader Reader"; }

        osg::Shader* processIncludes( const osg::Shader* shader, const Options* options ) const
        {
            std::string code = shader->getShaderSource();

            std::string::size_type pos = 0;

            static std::string::size_type includeLen = 8;

            while( ( pos = code.find( "#include", pos ) ) != std::string::npos )
            {
                // we found an include
                std::string::size_type pos2 = code.find_first_not_of( " ", pos + includeLen );

                if ( ( pos2 == std::string::npos ) || ( code[ pos2 ] != '\"' ) )
                {
                    // error, bail out
                    return NULL;
                }

                // we found an "
                std::string::size_type pos3 = code.find( "\"", pos2 + 1 );

                if ( pos3 == std::string::npos )
                {
                    return NULL;
                }

                const std::string filename = code.substr( pos2 + 1, pos3 - pos2 - 1 );

                osg::ref_ptr<osg::Shader> innerShader = osgDB::readShaderFile( shader->getType(), filename, options );

                if ( !innerShader.valid() )
                {
                    return NULL;
                }

                code.replace( pos, pos3 - pos + 1, innerShader->getShaderSource() );

                pos += innerShader->getShaderSource().size();
            }

            return new osg::Shader( shader->getType(), code );
        }

        virtual ReadResult readObject(std::istream& fin,const Options* options) const
        {
            return readShader(fin, options);
        }

        virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            return readShader(file, options);
        }

        virtual ReadResult readShader(std::istream& fin,const Options* options) const
        {
            // create shader
            osg::ref_ptr<osg::Shader> shader = new osg::Shader();

            {
                std::stringstream ss;
                ss << fin.rdbuf();
                shader->setShaderSource( ss.str() );
            }

            // check options which can define the type of the shader program
            if (options)
            {
                if (options->getOptionString().find("fragment")!=std::string::npos) shader->setType(osg::Shader::FRAGMENT);
                if (options->getOptionString().find("vertex")!=std::string::npos) shader->setType(osg::Shader::VERTEX);
                if (options->getOptionString().find("geometry")!=std::string::npos) shader->setType(osg::Shader::GEOMETRY);
                if (options->getOptionString().find("tesscontrol")!=std::string::npos) shader->setType(osg::Shader::TESSCONTROL);
                if (options->getOptionString().find("tessevaluation")!=std::string::npos) shader->setType(osg::Shader::TESSEVALUATION);
                if (options->getOptionString().find("compute")!=std::string::npos) shader->setType(osg::Shader::COMPUTE);
            }

            // return valid shader
            return processIncludes( shader.get(), options );
        }

        virtual ReadResult readShader(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            osgDB::ifstream istream(fileName.c_str(), std::ios::in | std::ios::binary);
            if(!istream) return ReadResult::FILE_NOT_HANDLED;
            ReadResult rr = readShader(istream, options);
            if(rr.validShader())
            {
                osg::Shader* shader = rr.getShader();
                shader->setFileName(file);
                if (shader->getType() == osg::Shader::UNDEFINED)
                {
                    // set type based on filename extension, where possible
                    if (ext == "frag") shader->setType(osg::Shader::FRAGMENT);
                    if (ext == "vert") shader->setType(osg::Shader::VERTEX);
                    if (ext == "geom") shader->setType(osg::Shader::GEOMETRY);
                    if (ext == "tctrl") shader->setType(osg::Shader::TESSCONTROL);
                    if (ext == "teval") shader->setType(osg::Shader::TESSEVALUATION);
                    if (ext == "compute") shader->setType(osg::Shader::COMPUTE);
                }
            }
            return rr;
        }

        virtual WriteResult writeShader(const osg::Shader& shader,std::ostream& fout,const Options* = NULL) const
        {
            // get shader source
            std::string source = shader.getShaderSource();

            // write source to file
            fout << source;

            // return all things went fine
            return WriteResult::FILE_SAVED;
        }

        virtual WriteResult writeShader(const osg::Shader &shader,const std::string& fileName, const osgDB::ReaderWriter::Options*) const
        {
            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

            osgDB::ofstream fout(fileName.c_str(), std::ios::out | std::ios::binary);
            if(!fout) return WriteResult::ERROR_IN_WRITING_FILE;

            return writeShader(shader, fout);
        }
};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(glsl, ReaderWriterGLSL)
