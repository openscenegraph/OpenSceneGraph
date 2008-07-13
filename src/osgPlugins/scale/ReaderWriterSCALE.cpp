/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2004 Robert Osfield 
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

/* file:        src/osgPlugins/scale/ReaderWriterSCALE.cpp
 * author:        Mike Weiblen http://mew.cx/ 2004-07-15
 * copyright:        (C) 2004 Michael Weiblen
 * license:        OpenSceneGraph Public License (OSGPL)
*/

#include <osg/Notify>
#include <osg/Matrix>
#include <osg/MatrixTransform>

#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <stdio.h>

#define EXTENSION_NAME "scale"


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

/**
 * An OSG reader plugin for the ".scale" pseudo-loader, which inserts a
 * scale transform above the loaded geometry.
 * This pseudo-loader make it simple to change the size of a saved model
 * by specifying a correcting scale factor as part of the filename.
 *
 * Usage: <modelfile.ext>.<sx>,<sy>,<sz>.globe
 *        <modelfile.ext>.<su>.globe
 * where:
 *      <modelfile.ext> = an model filename.
 *      <sx> = scale factor along the X axis.
 *      <sy> = scale factor along the Y axis.
 *      <sz> = scale factor along the Z axis.
 *      <su> = uniform scale factor applied to all axes.
 *
 * example: osgviewer cow.osg.5.scale cessna.osg
 */

class ReaderWriterSCALE : public osgDB::ReaderWriter
{
public:
    ReaderWriterSCALE()
    {
        supportsExtension(EXTENSION_NAME,"Scale Pseudo loader");
    }
    
    virtual const char* className() const { return "scaling pseudo-loader"; }

    virtual bool acceptsExtension(const std::string& extension) const
    { 
        return osgDB::equalCaseInsensitive( extension, EXTENSION_NAME );
    }

    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(fileName);
        if( !acceptsExtension(ext) )
            return ReadResult::FILE_NOT_HANDLED;

        osg::notify(osg::INFO) << "ReaderWriterSCALE( \"" << fileName << "\" )" << std::endl;

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

        float sx, sy, sz;
        int count = sscanf( params.c_str(), "%f,%f,%f", &sx, &sy, &sz );
        if( count == 1 )
        {
            // if only one value supplied, apply uniform scaling
            sy = sx;
            sz = sx;
        }
        else if( count != 3 )
        {
            osg::notify(osg::WARN) << "Bad parameters for " EXTENSION_NAME " pseudo-loader: \"" << params << "\"" << std::endl;
            return ReadResult::FILE_NOT_HANDLED;
        }

        // recursively load the subfile.
        osg::Node *node = osgDB::readNodeFile( subFileName, options );
        if( !node )
        {
            // propagate the read failure upwards
            osg::notify(osg::WARN) << "Subfile \"" << subFileName << "\" could not be loaded" << std::endl;
            return ReadResult::FILE_NOT_HANDLED;
        }

        osg::MatrixTransform *xform = new osg::MatrixTransform;
        xform->setDataVariance( osg::Object::STATIC );
        xform->setMatrix( osg::Matrix::scale( sx, sy, sz ) );
        xform->addChild( node );

        // turn on GL_NORMALIZE to prevent problems with scaled normals
        osg::StateSet* ss = xform->getOrCreateStateSet();
        ss->setMode( GL_NORMALIZE, osg::StateAttribute::ON );

        return xform;
    }
};


// Add ourself to the Registry to instantiate the reader/writer.
REGISTER_OSGPLUGIN(scale, ReaderWriterSCALE)

/*EOF*/

