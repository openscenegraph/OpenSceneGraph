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

/* file:        src/osgPlugins/trans/ReaderWriterTRANS.cpp
 * author:      Mike Weiblen http://mew.cx/ 2004-07-15
 * copyright:   (C) 2004 Michael Weiblen
 * license:     OpenSceneGraph Public License (OSGPL)
*/

#include <osg/Notify>
#include <osg/Matrix>
#include <osg/MatrixTransform>

#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <stdio.h>

#define EXTENSION_NAME "trans"


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
 * An OSG reader plugin for the ".trans" pseudo-loader, which inserts a
 * translation transform above the loaded geometry.
 * This pseudo-loader make it simple to change the origin of a saved model
 * by specifying a correcting translation as part of the filename.
 *
 * Usage: <modelfile.ext>.<tx>,<ty>,<tz>.globe
 * where:
 *      <modelfile.ext> = an model filename.
 *      <tx> = translation along the X axis.
 *      <ty> = translation along the Y axis.
 *      <tz> = translation along the Z axis.
 *
 * example: osgviewer cow.osg.25,0,0.trans cessna.osg
 */

class ReaderWriterTRANS : public osgDB::ReaderWriter
{
public:
    ReaderWriterTRANS() { }
    
    virtual const char* className() const { return "translation pseudo-loader"; }

    virtual bool acceptsExtension(const std::string& extension) const
    { 
        return osgDB::equalCaseInsensitive( extension, EXTENSION_NAME );
    }

    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(fileName);
        if( !acceptsExtension(ext) )
            return ReadResult::FILE_NOT_HANDLED;

        osg::notify(osg::INFO) << "ReaderWriterTRANS( \"" << fileName << "\" )" << std::endl;

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

        float tx, ty, tz;
        int count = sscanf( params.c_str(), "%f,%f,%f", &tx, &ty, &tz );
        if( count != 3 )
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
        xform->setMatrix( osg::Matrix::translate( tx, ty, tz ) );
        xform->addChild( node );
        return xform;
    }
};


// Add ourself to the Registry to instantiate the reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterTRANS> g_readerWriter_TRANS_Proxy;

/*EOF*/

