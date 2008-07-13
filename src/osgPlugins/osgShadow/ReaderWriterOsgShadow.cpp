/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2007 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial
 * applications, as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
*/

#include <osgShadow/ShadowedScene>
#include <osgShadow/ShadowVolume>
#include <osgShadow/ShadowTexture>
#include <osgShadow/ShadowMap>

#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <stdio.h>

#define EXTENSION_NAME "osgShadow"

static bool getFilenameAndParams(const std::string& input, std::string& filename, std::string& params)
{
        // find the start of the params list, accounting for nesting of [] and () brackets,
        // note, we are working backwards.
        int noNestedBrackets = 0;
        std::string::size_type pos = input.size();
        for(; pos>0; )
        {
            --pos;
            char c = input[pos];
            if (c==']') ++noNestedBrackets;
            else if (c=='[') --noNestedBrackets;
            else if (c==')') ++noNestedBrackets;
            else if (c=='(') --noNestedBrackets;
            else if (c=='.' && noNestedBrackets==0) break;
        }

        // get the next "extension", which actually contains the pseudo-loader parameters
        params = input.substr(pos+1, std::string::npos );
        if( params.empty() )
        {
            osg::notify(osg::WARN) << "Missing parameters for " EXTENSION_NAME " pseudo-loader" << std::endl;
            return false;
        }

        // clear the params sting of any brackets.
        std::string::size_type params_pos = params.size();
        for(; params_pos>0; )
        {
            --params_pos;
            char c = params[params_pos];
            if (c==']' || c=='[' || c==')' || c=='(')
            {
                params.erase(params_pos,1);
            }
        }

        // strip the "params extension", which must leave a sub-filename.
        filename = input.substr(0, pos );

        return true;
}

///////////////////////////////////////////////////////////////////////////

class ReaderWriterOsgShadow : public osgDB::ReaderWriter
{
public:
    ReaderWriterOsgShadow()
    {
        supportsExtension("osgShadow","OpenSceneGraph osgShadow extension to .osg ascii format");
        supportsExtension("shadow","OpenSceneGraph osgShadow extension pseudo loader");
    }
    
    virtual const char* className() const { return "osgShadow pseudo-loader"; }

    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(fileName);
        if( !acceptsExtension(ext) )
            return ReadResult::FILE_NOT_HANDLED;

        // strip the pseudo-loader extension
        std::string tmpName = osgDB::getNameLessExtension( fileName );

        if (tmpName.empty())
            return ReadResult::FILE_NOT_HANDLED;

        std::string subFileName, params;
        if (!getFilenameAndParams(tmpName, subFileName, params))
        {
            return ReadResult::FILE_NOT_HANDLED;
        }
        
        if( subFileName.empty())
        {
            osg::notify(osg::WARN) << "Missing subfilename for " EXTENSION_NAME " pseudo-loader" << std::endl;
            return ReadResult::FILE_NOT_HANDLED;
        }

        osg::notify(osg::INFO) << " params = \"" << params << "\"" << std::endl;
        osg::notify(osg::INFO) << " subFileName = \"" << subFileName << "\"" << std::endl;

        osg::ref_ptr<osgShadow::ShadowTechnique> technique;
        if (!params.empty())
        {
            if      (params=="ShadowVolume" || params=="sv")             technique = new osgShadow::ShadowVolume;
            else if (params=="ShadowTexture" || params=="st")            technique = new osgShadow::ShadowTexture;
            else if (params=="ShadowMap" || params=="sm")                technique = new osgShadow::ShadowMap;
//            else if (params=="ParallelSplitShadowMap" || params=="pssm") technique = new osgShadow::ParallelSplitShadowMap;
            else subFileName += std::string(".") + params;
        }

        // default fallback to using ShadowVolume
        if (!technique) technique = new osgShadow::ShadowVolume;

        // recursively load the subfile.
        osg::Node *node = osgDB::readNodeFile( subFileName, options );
        if( !node )
        {
            // propagate the read failure upwards
            osg::notify(osg::WARN) << "Subfile \"" << subFileName << "\" could not be loaded" << std::endl;
            return ReadResult::FILE_NOT_HANDLED;
        }

        osgShadow::ShadowedScene* shadowedScene = new osgShadow::ShadowedScene;
        shadowedScene->setShadowTechnique(technique.get());        
        shadowedScene->addChild( node );
        return shadowedScene;
    }
};


// Add ourself to the Registry to instantiate the reader/writer.
REGISTER_OSGPLUGIN(osgShadow, ReaderWriterOsgShadow)

