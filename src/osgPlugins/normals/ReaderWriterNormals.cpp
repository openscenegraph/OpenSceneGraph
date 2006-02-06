#include <iostream>
#include <sstream>
#include <math.h>
#include <osg/Notify>
#include <osg/Group>
#include <osgDB/ReadFile>

#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#include "Normals.h"

class NormalsReader: public osgDB::ReaderWriter
{
    public:
        NormalsReader() {}

        virtual const char* className() { return "Normals Pseudo Loader"; }

        virtual bool acceptsExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive(extension,"normals");
        }

        virtual ReadResult readObject(const std::string& fileName, const Options* opt) const
        { return readNode(fileName,opt); }

        virtual ReadResult readNode(const std::string& fileName, const Options* options) const
        {
            std::string ext = osgDB::getFileExtension(fileName);
            if (!acceptsExtension(ext))
                return ReadResult::FILE_NOT_HANDLED;

            float scale = 1.0;
            Normals::Mode mode = Normals::VertexNormals;

            if (options)
            {
                std::istringstream iss(options->getOptionString());
                std::string opt;
                while (iss >> opt)
                {
                    if( opt == "help" || opt == "HELP" )
                    {
                        osg::notify( osg::INFO ) << 
                            "Normals Plugin usage:  <application> [-O options] <model.ext>.normals\n"
                            "     options: \"scale=<scale>\"                        (default = 1.0)\n"
                            "              \"mode=<VertexNormals|SurfaceNormals>\"  (default = VertexNormals)" << std::endl;

                    }
                    else
                    {
                        int index = opt.find( "=" );
                        if( opt.substr( 0, index ) == "scale" ||
                            opt.substr( 0, index ) == "SCALE" )
                        {
                            scale = atof( opt.substr( index+1 ).c_str() );
                        }
                        else if( opt.substr( 0, index ) == "mode" || opt.substr( 0, index ) == "MODE" )
                        {
                            std::string modestr = opt.substr(index+1);
                            if( modestr == "VertexNormals" )
                                mode = Normals::VertexNormals;
                            else if( modestr == "SurfaceNormals" )
                                mode = Normals::SurfaceNormals;
                            else
                                mode = Normals::VertexNormals;
                        }
                    }
                }
            }

            std::string nodeName = osgDB::getNameLessExtension( fileName );
            if( !nodeName.empty() )
            {
                osg::ref_ptr<osg::Node> node = osgDB::readNodeFile( nodeName );
                if( node.valid() )
                {
                    osg::ref_ptr<osg::Group> group = new osg::Group;
                    group->addChild( node.get() );
                    if( mode == Normals::VertexNormals ) 
                        group->addChild( new VertexNormals( node.get(), scale ));
                    else if( mode == Normals::SurfaceNormals )
                        group->addChild( new SurfaceNormals( node.get(), scale ));

                    return group.get();
                }
            }
            return 0L;
        }
};

osgDB::RegisterReaderWriterProxy<NormalsReader> g_normalsReader_Proxy;


