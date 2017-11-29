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
            supportsExtension("cs","OpenGL Shader Language format");
            supportsExtension("gs","OpenGL Shader Language format");
            supportsExtension("vs","OpenGL Shader Language format");
            supportsExtension("fs","OpenGL Shader Language format");
        }

        virtual const char* className() const { return "GLSL Shader Reader"; }

        void processIncludes( osg::Shader& shader, const Options* options ) const
        {
            std::string code = shader.getShaderSource();
            std::string startOfIncludeMarker("// Start of include code : ");
            std::string endOfIncludeMarker("// End of include code : ");
            std::string failedLoadMarker("// Failed to load include code : ");

            #if defined(__APPLE__)
            std::string endOfLine("\r");
            #elif defined(_WIN32)
            std::string endOfLine("\r\n");
            #else
            std::string endOfLine("\n");
            #endif


            std::string::size_type pos = 0;
            std::string::size_type pragma_pos = 0;
            std::string::size_type include_pos = 0;
            while( (pos !=std::string::npos) && (((pragma_pos=code.find( "#pragma", pos )) != std::string::npos) || (include_pos=code.find( "#include", pos )) != std::string::npos))
            {
                pos = (pragma_pos!= std::string::npos) ? pragma_pos : include_pos;

                std::string::size_type start_of_pragma_line = pos;
                std::string::size_type end_of_line = code.find_first_of("\n\r", pos );

                if (pragma_pos!= std::string::npos)
                {
                    // we have #pragma usage so skip to the start of the first non white space
                    pos = code.find_first_not_of(" \t", pos+7 );
                    if (pos==std::string::npos) break;


                    // check for include part of #pragma imclude usage
                    if (code.compare(pos, 7, "include")!=0)
                    {
                        pos = end_of_line;
                        continue;
                    }

                    // found include entry so skip to next non white space
                    pos = code.find_first_not_of(" \t", pos+7 );
                    if (pos==std::string::npos) break;
                }
                else
                {
                    // we have #include usage so skip to next non white space
                    pos = code.find_first_not_of(" \t", pos+8 );
                    if (pos==std::string::npos) break;
                }


                std::string::size_type num_characters = (end_of_line==std::string::npos) ? code.size()-pos : end_of_line-pos;
                if (num_characters==0) continue;

                // prune trailing white space
                while(num_characters>0 && (code[pos+num_characters-1]==' ' || code[pos+num_characters-1]=='\t')) --num_characters;

                if (code[pos]=='\"')
                {
                    if (code[pos+num_characters-1]!='\"')
                    {
                        num_characters -= 1;
                    }
                    else
                    {
                        num_characters -= 2;
                    }

                    ++pos;
                }

                std::string filename(code, pos, num_characters);

                code.erase(start_of_pragma_line, (end_of_line==std::string::npos) ? code.size()-start_of_pragma_line : end_of_line-start_of_pragma_line);
                pos = start_of_pragma_line;

                osg::ref_ptr<osg::Shader> innerShader = osgDB::readRefShaderFile( filename, options );

                if (innerShader.valid())
                {
                    if (!startOfIncludeMarker.empty())
                    {
                        code.insert(pos, startOfIncludeMarker); pos += startOfIncludeMarker.size();
                        code.insert(pos, filename); pos += filename.size();
                        code.insert(pos, endOfLine); pos += endOfLine.size();
                    }

                    code.insert(pos, innerShader->getShaderSource() ); pos += innerShader->getShaderSource().size();

                    if (!endOfIncludeMarker.empty())
                    {
                        code.insert(pos, endOfIncludeMarker); pos += endOfIncludeMarker.size();
                        code.insert(pos, filename); pos += filename.size();
                        code.insert(pos, endOfLine); pos += endOfLine.size();
                    }
                }
                else
                {
                    if (!failedLoadMarker.empty())
                    {
                        code.insert(pos, failedLoadMarker); pos += failedLoadMarker.size();
                        code.insert(pos, filename); pos += filename.size();
                        code.insert(pos, endOfLine); pos += endOfLine.size();
                    }
                }
            }

            shader.setShaderSource(code);
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

            processIncludes( *shader, options );

            // return valid shader
            return shader.get();
        }

        virtual ReadResult readShader(const std::string& file, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
            local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));

            osgDB::ifstream istream(fileName.c_str(), std::ios::in | std::ios::binary);
            if(!istream) return ReadResult::FILE_NOT_HANDLED;
            ReadResult rr = readShader(istream, local_opt.get());
            if(rr.validShader())
            {
                osg::Shader* shader = rr.getShader();
                shader->setFileName(file);
                if (shader->getType() == osg::Shader::UNDEFINED)
                {
                    // set type based on filename extension, where possible
                    if (ext == "frag") shader->setType(osg::Shader::FRAGMENT);
                    if (ext == "fs") shader->setType(osg::Shader::FRAGMENT);
                    if (ext == "vert") shader->setType(osg::Shader::VERTEX);
                    if (ext == "vs") shader->setType(osg::Shader::VERTEX);
                    if (ext == "geom") shader->setType(osg::Shader::GEOMETRY);
                    if (ext == "gs") shader->setType(osg::Shader::GEOMETRY);
                    if (ext == "tctrl") shader->setType(osg::Shader::TESSCONTROL);
                    if (ext == "teval") shader->setType(osg::Shader::TESSEVALUATION);
                    if (ext == "compute") shader->setType(osg::Shader::COMPUTE);
                    if (ext == "cs") shader->setType(osg::Shader::COMPUTE);
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
