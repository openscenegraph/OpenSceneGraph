#include <sstream>

#include <osg/Image>
#include <osg/Group>
#include <osg/Notify>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/fstream>
#include <osgDB/Registry>
#include <osgDB/Input>
#include <osgDB/Output>

using namespace osg;
using namespace osgDB;

// pull in symbols from individual .o's to enable the static build to work
USE_DOTOSGWRAPPER(AlphaFunc)
USE_DOTOSGWRAPPER(AnimationPath)
USE_DOTOSGWRAPPER(AutoTransform)
USE_DOTOSGWRAPPER(Billboard)
USE_DOTOSGWRAPPER(BlendColor)
USE_DOTOSGWRAPPER(BlendEquation)
USE_DOTOSGWRAPPER(BlendFunc)
USE_DOTOSGWRAPPER(Camera)
USE_DOTOSGWRAPPER(CameraView)
USE_DOTOSGWRAPPER(ClearNode)
USE_DOTOSGWRAPPER(ClipNode)
USE_DOTOSGWRAPPER(ClipPlane)
USE_DOTOSGWRAPPER(ClusterCullingCallback)
USE_DOTOSGWRAPPER(ColorMask)
USE_DOTOSGWRAPPER(ColorMatrix)
USE_DOTOSGWRAPPER(ConvexPlanarOccluder)
USE_DOTOSGWRAPPER(CoordinateSystemNode)
USE_DOTOSGWRAPPER(CullFace)
USE_DOTOSGWRAPPER(Depth)
USE_DOTOSGWRAPPER(Drawable)
USE_DOTOSGWRAPPER(EllipsoidModel)
USE_DOTOSGWRAPPER(Fog)
USE_DOTOSGWRAPPER(FragmentProgram)
USE_DOTOSGWRAPPER(FrontFace)
USE_DOTOSGWRAPPER(Geode)
USE_DOTOSGWRAPPER(Geometry)
USE_DOTOSGWRAPPER(Group)
USE_DOTOSGWRAPPER(Image)
USE_DOTOSGWRAPPER(ImageSequence)
USE_DOTOSGWRAPPER(Light)
USE_DOTOSGWRAPPER(LightModel)
USE_DOTOSGWRAPPER(LightSource)
USE_DOTOSGWRAPPER(LineStipple)
USE_DOTOSGWRAPPER(LineWidth)
USE_DOTOSGWRAPPER(LOD)
USE_DOTOSGWRAPPER(Material)
USE_DOTOSGWRAPPER(MatrixTransform)
USE_DOTOSGWRAPPER(NodeCallback)
USE_DOTOSGWRAPPER(Node)
USE_DOTOSGWRAPPER(Object)
USE_DOTOSGWRAPPER(OccluderNode)
USE_DOTOSGWRAPPER(OcclusionQueryNode)
USE_DOTOSGWRAPPER(PagedLOD)
USE_DOTOSGWRAPPER(Point)
USE_DOTOSGWRAPPER(PointSprite)
USE_DOTOSGWRAPPER(PolygonMode)
USE_DOTOSGWRAPPER(PolygonOffset)
USE_DOTOSGWRAPPER(PositionAttitudeTransform)
USE_DOTOSGWRAPPER(Program)
USE_DOTOSGWRAPPER(Projection)
USE_DOTOSGWRAPPER(ProxyNode)
USE_DOTOSGWRAPPER(Scissor)
USE_DOTOSGWRAPPER(Sequence)
USE_DOTOSGWRAPPER(ShadeModel)
USE_DOTOSGWRAPPER(Shader)
USE_DOTOSGWRAPPER(Sphere)
USE_DOTOSGWRAPPER(Cone)
USE_DOTOSGWRAPPER(Capsule)
USE_DOTOSGWRAPPER(Box)
USE_DOTOSGWRAPPER(HeightField)
USE_DOTOSGWRAPPER(CompositeShape)
USE_DOTOSGWRAPPER(Cylinder)
USE_DOTOSGWRAPPER(ShapeDrawable)
USE_DOTOSGWRAPPER(StateAttribute)
USE_DOTOSGWRAPPER(StateSet)
USE_DOTOSGWRAPPER(Stencil)
USE_DOTOSGWRAPPER(Switch)
USE_DOTOSGWRAPPER(TessellationHints)
USE_DOTOSGWRAPPER(TexEnvCombine)
USE_DOTOSGWRAPPER(TexEnv)
USE_DOTOSGWRAPPER(TexEnvFilter)
USE_DOTOSGWRAPPER(TexGen)
USE_DOTOSGWRAPPER(TexGenNode)
USE_DOTOSGWRAPPER(TexMat)
USE_DOTOSGWRAPPER(Texture1D)
USE_DOTOSGWRAPPER(Texture2D)
USE_DOTOSGWRAPPER(Texture3D)
USE_DOTOSGWRAPPER(Texture)
USE_DOTOSGWRAPPER(TextureCubeMap)
USE_DOTOSGWRAPPER(TextureRectangle)
USE_DOTOSGWRAPPER(Transform)
USE_DOTOSGWRAPPER(Uniform)
USE_DOTOSGWRAPPER(VertexProgram)
USE_DOTOSGWRAPPER(Viewport)

class OSGReaderWriter : public ReaderWriter
{
    public:
    
        OSGReaderWriter()
        {
            supportsExtension("osg","OpenSceneGraph Ascii file format");
            supportsExtension("osgs","Psuedo OpenSceneGraph file loaded, with file encoded in filename string");
            supportsOption("precision","Set the floating point precision when writing out files");
            supportsOption("OutputTextureFiles","Write out the texture images to file");
            supportsOption("OutputRelativeTextures","Write texture images to a subfolder and reference them with relative file names");
        }
    
        virtual const char* className() const { return "OSG Reader/Writer"; }

        virtual ReadResult readObject(const std::string& file, const Options* opt) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
                        
            if (equalCaseInsensitive(ext,"osgs"))
            {   
                std::istringstream fin(osgDB::getNameLessExtension(file));
                if (fin) return readNode(fin,opt);
                return ReadResult::ERROR_IN_READING_FILE;
            }            
            
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, opt );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            // code for setting up the database path so that internally referenced file are searched for on relative paths. 
            osg::ref_ptr<Options> local_opt = opt ? static_cast<Options*>(opt->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
            local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));

            osgDB::ifstream fin(fileName.c_str());
            if (fin)
            {
                return readObject(fin, local_opt.get());
            }
            return 0L;
        }               

        virtual ReadResult readObject(std::istream& fin, const Options* options) const
        {
            fin.imbue(std::locale::classic());

            Input fr;
            fr.attach(&fin);
            fr.setOptions(options);
            
            typedef std::vector<osg::Object*> ObjectList;
            ObjectList objectList;

            // load all nodes in file, placing them in a group.
            while(!fr.eof())
            {
                Object *object = fr.readObject();
                if (object) objectList.push_back(object);
                else fr.advanceOverCurrentFieldOrBlock();
            }

            if  (objectList.empty())
            {
                return ReadResult("No data loaded");
            }
            else if (objectList.size()==1)
            {
                return objectList.front();
            }
            else
            {
                return objectList.front();
            }
        }

        virtual ReadResult readNode(const std::string& file, const Options* opt) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);

            if (equalCaseInsensitive(ext,"osgs"))
            {   
                std::istringstream fin(osgDB::getNameLessExtension(file));
                if (fin) return readNode(fin,opt);
                return ReadResult::ERROR_IN_READING_FILE;
            }            
            
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, opt );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            // code for setting up the database path so that internally referenced file are searched for on relative paths. 
            osg::ref_ptr<Options> local_opt = opt ? static_cast<Options*>(opt->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
            local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));

            osgDB::ifstream fin(fileName.c_str());
            if (fin)
            {
                return readNode(fin, local_opt.get());
            }
            return 0L;
                        
        }
        
        virtual ReadResult readNode(std::istream& fin, const Options* options) const
        {
            fin.imbue(std::locale::classic());

            Input fr;
            fr.attach(&fin);
            fr.setOptions(options);
            
            typedef std::vector<osg::Node*> NodeList;
            NodeList nodeList;

            // load all nodes in file, placing them in a group.
            while(!fr.eof())
            {
                Node *node = fr.readNode();
                if (node) nodeList.push_back(node);
                else fr.advanceOverCurrentFieldOrBlock();
            }

            if  (nodeList.empty())
            {
                return ReadResult("No data loaded");
            }
            else if (nodeList.size()==1)
            {
                return nodeList.front();
            }
            else
            {
                Group* group = new Group;
                group->setName("import group");
                for(NodeList::iterator itr=nodeList.begin();
                    itr!=nodeList.end();
                    ++itr)
                {
                    group->addChild(*itr);
                }
                return group;
            }

        }

        void setPrecision(Output& fout, const osgDB::ReaderWriter::Options* options) const
        {
            if (options)
            {
                std::istringstream iss(options->getOptionString());
                std::string opt;
                while (iss >> opt)
                {
                    if(opt=="PRECISION" || opt=="precision") 
                    {
                        int prec;
                        iss >> prec;
                        fout.precision(prec);
                    }
                    if (opt=="OutputRelativeTextures")
                    {
                        fout.setOutputRelativeTextures(true);
                    }
                    if (opt=="OutputTextureFiles")
                    {
                        fout.setOutputTextureFiles(true);
                    }
                    if (opt=="OutputShaderFiles")
                    {
                        fout.setOutputShaderFiles(true);
                    }
                }
            }
        }            

        virtual WriteResult writeObject(const Object& obj, const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;


            Output fout(fileName.c_str());
            if (fout)
            {
                fout.setOptions(options);

                setPrecision(fout,options);

                fout.imbue(std::locale::classic());

                fout.writeObject(obj);
                fout.close();
                return WriteResult::FILE_SAVED;
            }
            return WriteResult("Unable to open file for output");
        }

        virtual WriteResult writeObject(const Object& obj,std::ostream& fout, const osgDB::ReaderWriter::Options* options) const
        {


            if (fout)
            {
                Output foutput;
                foutput.setOptions(options);

                std::ios &fios = foutput;
                fios.rdbuf(fout.rdbuf());

                fout.imbue(std::locale::classic());

                setPrecision(foutput,options);

                foutput.writeObject(obj);
                return WriteResult::FILE_SAVED;
            }
            return WriteResult("Unable to write to output stream");
        }


        virtual WriteResult writeNode(const Node& node, const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
        {
            std::string ext = getFileExtension(fileName);
            if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;


            Output fout(fileName.c_str());
            if (fout)
            {
                fout.setOptions(options);

                fout.imbue(std::locale::classic());

                setPrecision(fout,options);

                fout.writeObject(node);
                fout.close();
                return WriteResult::FILE_SAVED;
            }
            return WriteResult("Unable to open file for output");
        }

        virtual WriteResult writeNode(const Node& node, std::ostream& fout, const osgDB::ReaderWriter::Options* options) const
        {


            if (fout)
            {
                Output foutput;
                foutput.setOptions(options);

                std::ios &fios = foutput;
                fios.rdbuf(fout.rdbuf());

                foutput.imbue(std::locale::classic());

                setPrecision(foutput,options);

                foutput.writeObject(node);
                return WriteResult::FILE_SAVED;
            }
            return WriteResult("Unable to write to output stream");
        }

};


// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(osg, OSGReaderWriter)
