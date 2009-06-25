#include <iostream>
#include <sstream>
#include <math.h>
#include <stdlib.h>
#include <osg/Notify>
#include <osg/Group>
#include <osgDB/ReadFile>

#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#include "Normals.h"

class NormalsReader: public osgDB::ReaderWriter
{
    public:
        NormalsReader()
        {
            supportsExtension("normals","Normals Pseudo loader");
        }

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
                        usage();
                    }
                    else
                    {
                        size_t index = opt.find( "=" );
                        if (index == std::string::npos) {
                            usage();
                        } else {
                            std::string key = opt.substr(0, index);
                            std::string value = opt.substr(index+1);
                            if( key == "scale" || key == "SCALE" )
                            {
                                scale = osg::asciiToFloat( value.c_str() );
                            }
                            else if( key == "mode" || key == "MODE" )
                            {
                                if( value == "VertexNormals" )
                                    mode = Normals::VertexNormals;
                                else if( value == "SurfaceNormals" )
                                    mode = Normals::SurfaceNormals;
                                else
                                    mode = Normals::VertexNormals;
                            }
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

                    const osg::BoundingSphere& bsph = group->getBound();
                    scale = bsph.radius() * 0.05f * scale; // default is 5% of bounding-sphere radius

                    if( mode == Normals::VertexNormals ) 
                        group->addChild( new VertexNormals( node.get(), scale ));
                    else if( mode == Normals::SurfaceNormals )
                        group->addChild( new SurfaceNormals( node.get(), scale ));

                    return group.get();
                }
            }
            return 0L;
        }

    private:
        void usage() const {
            osg::notify( osg::INFO ) << 
                "Normals Plugin usage:  <application> [-O options] <model.ext>.normals\n"
                "     options: \"scale=<scale>\"                        (default = 1.0)\n"
                "              \"mode=<VertexNormals|SurfaceNormals>\"  (default = VertexNormals)" << std::endl;
        }
};

REGISTER_OSGPLUGIN(normals, NormalsReader)
