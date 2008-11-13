#include <osg/Shader>
#include <osg/Notify>
#include <osg/GL>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/fstream>


class ReaderWriterGLSL : public osgDB::ReaderWriter
{
    public:
    
        ReaderWriterGLSL()
        {
            supportsExtension("gl","OpenGL Shader Language format");
            supportsExtension("frag","OpenGL Shader Language format");
            supportsExtension("vert","OpenGL Shader Language format");
            supportsExtension("glsl","OpenGL Shader Language format");
        }
    
        virtual const char* className() const { return "GLSL Shader Reader"; }

        virtual ReadResult readShader(std::istream& fin,const Options* options) const
        {
            // read source
            fin.seekg(0, std::ios::end);
            int length = fin.tellg();
            char *text = new char[length + 1];
            fin.seekg(0, std::ios::beg);
            fin.read(text, length);
            text[length] = '\0';
        
            // create shader
            osg::Shader* shader = new osg::Shader();
            shader->setShaderSource( text );

            // check options which can define the type of the shader program
            if (options)
            {
                if (options->getOptionString().find("fragment")!=std::string::npos) shader->setType(osg::Shader::FRAGMENT);
                if (options->getOptionString().find("vertex")!=std::string::npos) shader->setType(osg::Shader::VERTEX);
                if (options->getOptionString().find("geometry")!=std::string::npos) shader->setType(osg::Shader::GEOMETRY);
            }

            // cleanup
            delete [] text;

            // return valid shader
            return shader;
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
            if(rr.validShader()) rr.getShader()->setFileName(file);
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
