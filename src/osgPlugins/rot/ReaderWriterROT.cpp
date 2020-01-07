/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This application is open source and may be redistributed and/or modified
 * freely and without restriction, both in commercial and non commercial
 * applications, as long as this copyright notice is maintained.
 *
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
*/

/* file:    src/osgPlugins/rot/ReaderWriterROT.cpp
 * author:    Mike Weiblen http://mew.cx/ 2005-06-06
 * copyright:    (C) 2005 Michael Weiblen
 * license:    OpenSceneGraph Public License (OSGPL)
*/

#include <osg/Notify>
#include <osg/Matrix>
#include <osg/MatrixTransform>

#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <stdio.h>

#define EXTENSION_NAME "rot"

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
            OSG_WARN << "Missing parameters for " EXTENSION_NAME " pseudo-loader" << std::endl;
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
 * An OSG reader plugin for the ".rot" pseudo-loader, which inserts a
 * rotation transform above the loaded geometry.
 * This pseudo-loader makes it simple to change the orientation of a saved
 * model by specifying a correcting rotation as part of the filename.
 *
 * Usage: <modelfile.ext>.<rx>,<ry>,<rz>.globe
 * where:
 *    <modelfile.ext> = an model filename.
 *    <rx> = rotation around X axis [degrees]
 *    <ry> = rotation around Y axis [degrees]
 *    <rz> = rotation around Z axis [degrees]
 *
 * example: osgviewer cow.osg.30,60,-90.rot
 */

class ReaderWriterROT : public osgDB::ReaderWriter
{
public:
    ReaderWriterROT()
    {
        supportsExtension(EXTENSION_NAME,"Rotation pseudo loader");
    }

    virtual const char* className() const { return "rotation pseudo-loader"; }

    virtual ReadResult readObject(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
    {
        return readNode(fileName, options); 
    }

    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(fileName);
        if( !acceptsExtension(ext) )
            return ReadResult::FILE_NOT_HANDLED;

        OSG_INFO << "ReaderWriterROT( \"" << fileName << "\" )" << std::endl;

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
            OSG_WARN << "Missing subfilename for " EXTENSION_NAME " pseudo-loader" << std::endl;
            return ReadResult::FILE_NOT_HANDLED;
        }

        OSG_INFO << " params = \"" << params << "\"" << std::endl;
        OSG_INFO << " subFileName = \"" << subFileName << "\"" << std::endl;

        float rx, ry, rz;
        int count = sscanf( params.c_str(), "%f,%f,%f", &rx, &ry, &rz );
        if( count != 3 )
        {
            OSG_WARN << "Bad parameters for " EXTENSION_NAME " pseudo-loader: \"" << params << "\"" << std::endl;
            return ReadResult::FILE_NOT_HANDLED;
        }

        // recursively load the subfile.
        osg::ref_ptr<osg::Node> node = osgDB::readRefNodeFile( subFileName, options );
        if( !node )
        {
            // propagate the read failure upwards
            OSG_WARN << "Subfile \"" << subFileName << "\" could not be loaded" << std::endl;
            return ReadResult::FILE_NOT_HANDLED;
        }

        osg::ref_ptr<osg::MatrixTransform> xform = new osg::MatrixTransform;
        xform->setDataVariance( osg::Object::STATIC );
        xform->setMatrix( osg::Matrix::rotate(
            osg::DegreesToRadians( rx ), osg::Vec3( 1, 0, 0 ),
            osg::DegreesToRadians( ry ), osg::Vec3( 0, 1, 0 ),
            osg::DegreesToRadians( rz ), osg::Vec3( 0, 0, 1 ) ));
        xform->addChild( node );
        return xform;
    }
};


// Add ourself to the Registry to instantiate the reader/writer.
REGISTER_OSGPLUGIN(rot, ReaderWriterROT)

/*EOF*/

