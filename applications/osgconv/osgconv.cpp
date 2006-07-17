#include <stdio.h>

#include <osg/ArgumentParser>
#include <osg/ApplicationUsage>
#include <osg/Group>
#include <osg/Notify>
#include <osg/Vec3>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/Texture3D>
#include <osg/BlendFunc>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osgDB/ReaderWriter>

#include <osgUtil/Optimizer>
#include <osgUtil/SmoothingVisitor>

#include <osgProducer/Viewer>

#include <iostream>

#include "OrientationConverter.h"

typedef std::vector<std::string> FileNameList;

class GraphicsContext {
    public:
        GraphicsContext()
        {
            rs = new Producer::RenderSurface;
            rs->setWindowRectangle(0,0,1,1);
            rs->useBorder(false);
            rs->useConfigEventThread(false);
            rs->realize();
            std::cout<<"Realized window"<<std::endl;
        }

        virtual ~GraphicsContext()
        {
        }
        
    private:
        Producer::ref_ptr<Producer::RenderSurface> rs;
};

class CompressTexturesVisitor : public osg::NodeVisitor
{
public:

    CompressTexturesVisitor(osg::Texture::InternalFormatMode internalFormatMode):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _internalFormatMode(internalFormatMode) {}

    virtual void apply(osg::Node& node)
    {
        if (node.getStateSet()) apply(*node.getStateSet());
        traverse(node);
    }
    
    virtual void apply(osg::Geode& node)
    {
        if (node.getStateSet()) apply(*node.getStateSet());
        
        for(unsigned int i=0;i<node.getNumDrawables();++i)
        {
            osg::Drawable* drawable = node.getDrawable(i);
            if (drawable && drawable->getStateSet()) apply(*drawable->getStateSet());
        }
        
        traverse(node);
    }
    
    virtual void apply(osg::StateSet& stateset)
    {
        // search for the existance of any texture object attributes
        for(unsigned int i=0;i<stateset.getTextureAttributeList().size();++i)
        {
            osg::Texture* texture = dynamic_cast<osg::Texture*>(stateset.getTextureAttribute(i,osg::StateAttribute::TEXTURE));
            if (texture)
            {
                _textureSet.insert(texture);
            }
        }
    }
    
    void compress()
    {
        GraphicsContext context;

        osg::ref_ptr<osg::State> state = new osg::State;

        for(TextureSet::iterator itr=_textureSet.begin();
            itr!=_textureSet.end();
            ++itr)
        {
            osg::Texture* texture = const_cast<osg::Texture*>(itr->get());
            
            osg::Texture2D* texture2D = dynamic_cast<osg::Texture2D*>(texture);
            osg::Texture3D* texture3D = dynamic_cast<osg::Texture3D*>(texture);
            
            osg::ref_ptr<osg::Image> image = texture2D ? texture2D->getImage() : (texture3D ? texture3D->getImage() : 0);
            if (image.valid() && 
                (image->getPixelFormat()==GL_RGB || image->getPixelFormat()==GL_RGBA) &&
                (image->s()>=32 && image->t()>=32))
            {
                texture->setInternalFormatMode(_internalFormatMode);
                
                // need to disable the unref after apply, other the image could go out of scope.
                bool unrefImageDataAfterApply = texture->getUnRefImageDataAfterApply();
                texture->setUnRefImageDataAfterApply(false);
                
                // get OpenGL driver to create texture from image.
                texture->apply(*state);

                // restore the original setting
                texture->setUnRefImageDataAfterApply(unrefImageDataAfterApply);

                image->readImageFromCurrentTexture(0,true);

                texture->setInternalFormatMode(osg::Texture::USE_IMAGE_DATA_FORMAT);
            }
        }
    }
    
    typedef std::set< osg::ref_ptr<osg::Texture> > TextureSet;
    TextureSet                          _textureSet;
    osg::Texture::InternalFormatMode    _internalFormatMode;
    
};


class FixTransparencyVisitor : public osg::NodeVisitor
{
public:

    enum FixTransparencyMode
    {
        NO_TRANSPARANCY_FIXING,
        MAKE_OPAQUE_TEXTURE_STATESET_OPAQUE,
        MAKE_ALL_STATESET_OPAQUE,
    };

    FixTransparencyVisitor(FixTransparencyMode mode=MAKE_OPAQUE_TEXTURE_STATESET_OPAQUE):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _numTransparent(0),
        _numOpaque(0),
        _numTransparentMadeOpaque(0),
        _mode(mode)
    {
        std::cout<<"Running FixTransparencyVisitor..."<<std::endl;
    }
        
    ~FixTransparencyVisitor()
    {
        std::cout<<"  Number of Transparent StateSet "<<_numTransparent<<std::endl;
        std::cout<<"  Number of Opaque StateSet "<<_numOpaque<<std::endl;
        std::cout<<"  Number of Transparent State made Opaque "<<_numTransparentMadeOpaque<<std::endl;
    }

    virtual void apply(osg::Node& node)
    {
        if (node.getStateSet()) isTransparent(*node.getStateSet());
        traverse(node);
    }
    
    virtual void apply(osg::Geode& node)
    {
        if (node.getStateSet()) isTransparent(*node.getStateSet());
        
        for(unsigned int i=0;i<node.getNumDrawables();++i)
        {
            osg::Drawable* drawable = node.getDrawable(i);
            if (drawable && drawable->getStateSet()) isTransparent(*drawable->getStateSet());
        }
        
        traverse(node);
    }
    
    virtual bool isTransparent(osg::StateSet& stateset)
    {
        bool hasTranslucentTexture = false;
        bool hasBlendFunc = dynamic_cast<osg::BlendFunc*>(stateset.getAttribute(osg::StateAttribute::BLENDFUNC))!=0;
        bool hasTransparentRenderingHint = stateset.getRenderingHint()==osg::StateSet::TRANSPARENT_BIN;
        bool hasDepthSortBin = (stateset.getRenderBinMode()==osg::StateSet::USE_RENDERBIN_DETAILS)?(stateset.getBinName()=="DepthSortedBin"):false;
        bool hasTexture = false;


        // search for the existance of any texture object attributes
        for(unsigned int i=0;i<stateset.getTextureAttributeList().size();++i)
        {
            osg::Texture* texture = dynamic_cast<osg::Texture*>(stateset.getTextureAttribute(i,osg::StateAttribute::TEXTURE));
            if (texture)
            {
                hasTexture = true;
                for (unsigned int im=0;im<texture->getNumImages();++im)
                {
                    osg::Image* image = texture->getImage(im);
                    if (image && image->isImageTranslucent()) hasTranslucentTexture = true;   
                }
            }
        }
        
        if (hasTranslucentTexture || hasBlendFunc || hasTransparentRenderingHint || hasDepthSortBin)
        {
            ++_numTransparent;
            
            bool makeNonTransparent = false;

            switch(_mode)
            {
            case(MAKE_OPAQUE_TEXTURE_STATESET_OPAQUE):
                if (hasTexture && !hasTranslucentTexture)
                {
                    makeNonTransparent = true;
                }
                break;
            case(MAKE_ALL_STATESET_OPAQUE):
                makeNonTransparent = true;
                break;
            default:
                makeNonTransparent = false;
                break;
            }
        
            if (makeNonTransparent)
            {
                stateset.removeAttribute(osg::StateAttribute::BLENDFUNC);
                stateset.removeMode(GL_BLEND);
                stateset.setRenderingHint(osg::StateSet::DEFAULT_BIN);
                ++_numTransparentMadeOpaque;
            }


            return true;
        }
        else
        {
            ++_numOpaque;
            return false;
        }
    }

    unsigned int _numTransparent;
    unsigned int _numOpaque;    
    unsigned int _numTransparentMadeOpaque;
    FixTransparencyMode _mode;
};

class PruneStateSetVisitor : public osg::NodeVisitor
{
public:

    PruneStateSetVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _numStateSetRemoved(0)
    {
        std::cout<<"Running PruneStateSet..."<<std::endl;
    }
        
    ~PruneStateSetVisitor()
    {
        std::cout<<"  Number of StateState removed "<<_numStateSetRemoved<<std::endl;
    }

    virtual void apply(osg::Node& node)
    {
        if (node.getStateSet())
        {
            node.setStateSet(0);
            ++_numStateSetRemoved;
        }
        traverse(node);
    }
    
    virtual void apply(osg::Geode& node)
    {
        if (node.getStateSet())
        {
            node.setStateSet(0);
            ++_numStateSetRemoved;
        }
        
        for(unsigned int i=0;i<node.getNumDrawables();++i)
        {
            osg::Drawable* drawable = node.getDrawable(i);
            if (drawable && drawable->getStateSet())
            {
                drawable->setStateSet(0);
                ++_numStateSetRemoved;
            }
        }
        
        traverse(node);
    }
    
    unsigned int _numStateSetRemoved;
};

/** Add missing colours to osg::Geometry.*/
class AddMissingColoursToGeometryVisitor : public osg::NodeVisitor
{
public:

    AddMissingColoursToGeometryVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

    virtual void apply(osg::Geode& geode)
    {
        for(unsigned int i=0;i<geode.getNumDrawables();++i)
        {
            osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
            if (geometry)
            {
                if (geometry->getColorArray()==0 || geometry->getColorArray()->getNumElements()==0)
                {
                    osg::Vec4Array* colours = new osg::Vec4Array(1);
                    (*colours)[0].set(1.0f,1.0f,1.0f,1.0f);
                    geometry->setColorArray(colours);
                    geometry->setColorBinding(osg::Geometry::BIND_OVERALL);
                }
            }
        }
    }

    virtual void apply(osg::Node& node) { traverse(node); }

};


static void usage( const char *prog, const char *msg )
{
    if (msg)
    {
        osg::notify(osg::NOTICE)<< std::endl;
        osg::notify(osg::NOTICE) << msg << std::endl;
    }
    osg::notify(osg::NOTICE)<< std::endl;
    osg::notify(osg::NOTICE)<<"usage:"<< std::endl;
    osg::notify(osg::NOTICE)<<"    " << prog << " [options] infile1 [infile2 ...] outfile"<< std::endl;
    osg::notify(osg::NOTICE)<< std::endl;
    osg::notify(osg::NOTICE)<<"options:"<< std::endl;
    osg::notify(osg::NOTICE)<<"    -O option          - ReaderWriter option"<< std::endl;
    osg::notify(osg::NOTICE)<< std::endl;
    osg::notify(osg::NOTICE)<<"    --compressed       - Enable the usage of compressed textures,"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         defaults to OpenGL ARB compressed textures."<< std::endl;
    osg::notify(osg::NOTICE)<<"    --compressed-arb   - Enable the usage of OpenGL ARB compressed textures"<< std::endl;
    osg::notify(osg::NOTICE)<<"    --compressed-dxt1  - Enable the usage of S3TC DXT1 compressed textures"<< std::endl;
    osg::notify(osg::NOTICE)<<"    --compressed-dxt3  - Enable the usage of S3TC DXT3 compressed textures"<< std::endl;
    osg::notify(osg::NOTICE)<<"    --compressed-dxt5  - Enable the usage of S3TC DXT5 compressed textures"<< std::endl;
    osg::notify(osg::NOTICE)<< std::endl;
    osg::notify(osg::NOTICE)<<"    --fix-transparency  - fix stateset which are curerntly declared as transprent,"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         but should be opaque. Defaults to using the "<< std::endl;
    osg::notify(osg::NOTICE)<<"                         fixTranspancyMode MAKE_OPAQUE_TEXTURE_STATESET_OPAQUE."<< std::endl;
    osg::notify(osg::NOTICE)<<"    --fix-transparency-mode <mode_string>  - fix stateset which are curerntly declared as"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         transprent but should be opaque. The mode_string determines"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         algorithm is used to fix the transparency, options are:    "<< std::endl;
    osg::notify(osg::NOTICE)<<"                                 MAKE_OPAQUE_TEXTURE_STATESET_OPAQUE,"<<std::endl;
    osg::notify(osg::NOTICE)<<"                                 MAKE_ALL_STATESET_OPAQUE."<<std::endl;

    osg::notify(osg::NOTICE)<< std::endl;
    osg::notify(osg::NOTICE)<<"    -l libraryName     - load plugin of name libraryName"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         i.e. -l osgdb_pfb"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         Useful for loading reader/writers which can load"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         other file formats in addition to its extension."<< std::endl;
    osg::notify(osg::NOTICE)<<"    -e extensionName   - load reader/wrter plugin for file extension"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         i.e. -e pfb"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         Useful short hand for specifying full library name as"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         done with -l above, as it automatically expands to the"<< std::endl;
    osg::notify(osg::NOTICE)<<"                         full library name appropriate for each platform."<< std::endl;
    osg::notify(osg::NOTICE)<<"    -o orientation     - Convert geometry from input files to output files."<< std::endl;
    osg::notify(osg::NOTICE)<<
                              "                         Format of orientation argument must be the following:\n"
                              "\n"
                              "                             X1,Y1,Z1-X2,Y2,Z2\n"
                              "                         or\n"
                              "                             degrees-A0,A1,A2\n"
                              "\n"
                              "                         where X1,Y1,Z1 represent the UP vector in the input\n"
                              "                         files and X2,Y2,Z2 represent the UP vector of the\n"
                              "                         output file, or degrees is the rotation angle in degrees\n"
                              "                         around axis (A0,A1,A2).  For example, to convert a model\n"
                              "                         built in a Y-Up coordinate system to a model with a Z-up\n"
                              "                         coordinate system, the argument may look like\n"
                              "\n"
                              "                             0,1,0-0,0,1"
                              "\n"
                              "                          or\n"
                              "                             -90-1,0,0\n"
                              "\n" << std::endl;
    osg::notify(osg::NOTICE)<<"    -t translation     - Convert spatial position of output files.  Format of\n"
                              "                         translation argument must be the following :\n"
                              "\n"
                              "                             X,Y,Z\n"
                              "\n"
                              "                         where X, Y, and Z represent the coordinates of the\n"
                              "                         absolute position in world space\n"
                              << std::endl;
    osg::notify(osg::NOTICE)<<"    -s scale           - Scale size of model.  Scale argument must be the \n"
                              "                         following :\n"
                              "\n"
                              "                             SX,SY,SZ\n"
                              "\n"
                              "                         where SX, SY, and SZ represent the scale factors\n"
                              "                         Caution: Scaling will be done in destination orientation\n"
                              << std::endl;
    osg::notify(osg::NOTICE)<<"    --smooth           - Smooth the surface by regenerating surface normals on\n"
                              "                         all geometry"<< std::endl;
    osg::notify(osg::NOTICE)<<"    --addMissingColors - Adding a white color value to all geometry that don't have\n"
                              "                         their own color values (--addMissingColours also accepted)."<< std::endl;

}


int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is a utility for converting between various input and output databases formats.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display command line paramters");
    arguments.getApplicationUsage()->addCommandLineOption("--help-env","Display environmental variables available");


    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    { 
        usage( arguments.getApplicationName().c_str(), 0 );
        //arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }
    
    if (arguments.read("--help-env"))
    { 
        arguments.getApplicationUsage()->write(std::cout, osg::ApplicationUsage::ENVIRONMENTAL_VARIABLE);
        return 1;
    }
    
    if (arguments.argc()<=1)
    {
        arguments.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);
        return 1;
    }

    FileNameList fileNames;
    OrientationConverter oc;
    bool do_convert = false;

    std::string str;
    while (arguments.read("-O",str))
    {
        osgDB::ReaderWriter::Options* options = new osgDB::ReaderWriter::Options;
        options->setOptionString(str);
        osgDB::Registry::instance()->setOptions(options);
    }

    std::string ext;
    while (arguments.read("-e",ext))
    {
        std::string libName = osgDB::Registry::instance()->createLibraryNameForExtension(ext);
        osgDB::Registry::instance()->loadLibrary(libName);
    }
    
    std::string libName;
    while (arguments.read("-l",libName))
    {
        osgDB::Registry::instance()->loadLibrary(libName);
    }

    while (arguments.read("-o",str))
    {
        osg::Vec3 from, to;
        if( sscanf( str.c_str(), "%f,%f,%f-%f,%f,%f",
                &from[0], &from[1], &from[2],
                &to[0], &to[1], &to[2]  )
            != 6 )
        {
            float degrees;
            osg::Vec3 axis;
            // Try deg-axis format
            if( sscanf( str.c_str(), "%f-%f,%f,%f",
                    &degrees, &axis[0], &axis[1], &axis[2]  ) != 4 )
            {
                usage( argv[0], "Orientation argument format incorrect." );
                return 1;
            }
            else
            {
                oc.setRotation( degrees, axis );
                do_convert = true;
            }
        }
        else
        {
            oc.setRotation( from, to );
            do_convert = true;
        }
    }    

    while (arguments.read("-s",str))
    {
        osg::Vec3 scale(0,0,0);
        if( sscanf( str.c_str(), "%f,%f,%f",
                &scale[0], &scale[1], &scale[2] ) != 3 )
        {
            usage( argv[0], "Scale argument format incorrect." );
            return 1;
        }
        oc.setScale( scale );
        do_convert = true;
    }

    while (arguments.read("-t",str))
    {
        osg::Vec3 trans(0,0,0);
        if( sscanf( str.c_str(), "%f,%f,%f",
                &trans[0], &trans[1], &trans[2] ) != 3 )
        {
            usage( argv[0], "Translation argument format incorrect." );
            return 1;
        }
        oc.setTranslation( trans );
        do_convert = true;
    }

    
    FixTransparencyVisitor::FixTransparencyMode fixTransparencyMode = FixTransparencyVisitor::NO_TRANSPARANCY_FIXING;
    std::string fixString;
    while(arguments.read("--fix-transparency")) fixTransparencyMode = FixTransparencyVisitor::MAKE_OPAQUE_TEXTURE_STATESET_OPAQUE;
    while(arguments.read("--fix-transparency-mode",fixString))
    {
         if (fixString=="MAKE_OPAQUE_TEXTURE_STATESET_OPAQUE") fixTransparencyMode = FixTransparencyVisitor::MAKE_OPAQUE_TEXTURE_STATESET_OPAQUE;
         if (fixString=="MAKE_ALL_STATESET_OPAQUE") fixTransparencyMode = FixTransparencyVisitor::MAKE_ALL_STATESET_OPAQUE;
    };
    
    bool pruneStateSet = false;
    while(arguments.read("--prune-StateSet")) pruneStateSet = true;

    osg::Texture::InternalFormatMode internalFormatMode = osg::Texture::USE_IMAGE_DATA_FORMAT;
    while(arguments.read("--compressed") || arguments.read("--compressed-arb")) { internalFormatMode = osg::Texture::USE_ARB_COMPRESSION; }

    while(arguments.read("--compressed-dxt1")) { internalFormatMode = osg::Texture::USE_S3TC_DXT1_COMPRESSION; }
    while(arguments.read("--compressed-dxt3")) { internalFormatMode = osg::Texture::USE_S3TC_DXT3_COMPRESSION; }
    while(arguments.read("--compressed-dxt5")) { internalFormatMode = osg::Texture::USE_S3TC_DXT5_COMPRESSION; }

    bool smooth = false;
    while(arguments.read("--smooth")) { smooth = true; }
    
    bool addMissingColours = false;
    while(arguments.read("--addMissingColours") || arguments.read("--addMissingColors")) { addMissingColours = true; }

    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    for(int pos=1;pos<arguments.argc();++pos)
    {
        if (!arguments.isOption(pos))
        {
            fileNames.push_back(arguments[pos]);
        }
    }


    std::string fileNameOut("converted.osg");
    if (fileNames.size()>1)
    {
        fileNameOut = fileNames.back();
        fileNames.pop_back();
    }

    osg::ref_ptr<osg::Node> root = osgDB::readNodeFiles(fileNames);

    if (pruneStateSet)
    {
        PruneStateSetVisitor pssv;
        root->accept(pssv);
    }

    if (fixTransparencyMode != FixTransparencyVisitor::NO_TRANSPARANCY_FIXING)
    {
        FixTransparencyVisitor atv(fixTransparencyMode);
        root->accept(atv);
    }

    if ( root.valid() )
    {
    
        if (smooth)
        {
            osgUtil::SmoothingVisitor sv;
            root->accept(sv);
        }    
    
        if (addMissingColours)
        {
            AddMissingColoursToGeometryVisitor av;
            root->accept(av);
        }

        // optimize the scene graph, remove rendundent nodes and state etc.
        osgUtil::Optimizer optimizer;
        optimizer.optimize(root.get());
        
        if( do_convert )
            root = oc.convert( root.get() );
            
        if (internalFormatMode != osg::Texture::USE_IMAGE_DATA_FORMAT)
        {
            std::string ext = osgDB::getFileExtension(fileNameOut);
            if (ext=="ive")
            {
                CompressTexturesVisitor ctv(internalFormatMode);
                root->accept(ctv);
                ctv.compress();
            }
            else
            {
                std::cout<<"Warning: compressing texture only supported when outputing to .ive"<<std::endl;
            }
        }

        if (osgDB::writeNodeFile(*root,fileNameOut))
        {
            osg::notify(osg::NOTICE)<<"Data written to '"<<fileNameOut<<"'."<< std::endl;
        }
    }
    else
    {
        osg::notify(osg::NOTICE)<<"Error no data loaded."<< std::endl;
        return 1;
    }

    return 0;
}
