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

/* file:        src/osgPlugins/rot/ReaderWriterROT.cpp
 * author:	Mike Weiblen http://mew.cx/ 2004-04-25
 * copyright:	(C) 2004 Michael Weiblen
 * license:	OpenSceneGraph Public License (OSGPL)
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

///////////////////////////////////////////////////////////////////////////

/**
 * An OSG reader plugin for the rotation pseudo-loader.
 */
class ReaderWriterROT : public osgDB::ReaderWriter
{
public:
    ReaderWriterROT() { }
    
    virtual const char* className() { return "rotation pseudo-loader"; }

    virtual bool acceptsExtension(const std::string& extension)
    { 
        return osgDB::equalCaseInsensitive( extension, EXTENSION_NAME );
    }

    virtual ReadResult readNode(const std::string& fileName,
		const osgDB::ReaderWriter::Options* /*options*/)
    {
	std::string ext = osgDB::getLowerCaseFileExtension(fileName);
	if( !acceptsExtension(ext) )
	    return ReadResult::FILE_NOT_HANDLED;

	osg::notify(osg::INFO) << "ReaderWriterROT( \"" << fileName << "\" )" << std::endl;

	// strip the pseudo-loader extension
	std::string tmpName = osgDB::getNameLessExtension( fileName );

	// get the next "extension", which actually contains the pseudo-loader parameters
	std::string params = osgDB::getFileExtension( tmpName );
	if( params.empty() )
	{
	    osg::notify(osg::WARN) << "Missing parameters for " EXTENSION_NAME " pseudo-loader" << std::endl;
	    return ReadResult::FILE_NOT_HANDLED;
	}

	// strip the "params extension", which must leave a sub-filename.
	std::string subFileName = osgDB::getNameLessExtension( tmpName );
	if( subFileName == tmpName )
	{
	    osg::notify(osg::WARN) << "Missing subfilename for " EXTENSION_NAME " pseudo-loader" << std::endl;
	    return ReadResult::FILE_NOT_HANDLED;
	}

	osg::notify(osg::INFO) << EXTENSION_NAME " params = \"" << params << "\"" << std::endl;

	int h, p, r;
	int count = sscanf( params.c_str(), "%d,%d,%d", &h, &p, &r );
	if( count != 3 )
	{
	    osg::notify(osg::WARN) << "Bad parameters for " EXTENSION_NAME " pseudo-loader: \"" << params << "\"" << std::endl;
	    return ReadResult::FILE_NOT_HANDLED;
	}

	// recursively load the subfile.
	osg::Node *node = osgDB::readNodeFile( subFileName );
	if( !node )
	{
	    // propagate the read failure upwards
	    osg::notify(osg::WARN) << "Subfile \"" << subFileName << "\" could not be loaded" << std::endl;
	    return ReadResult::FILE_NOT_HANDLED;
	}

	osg::MatrixTransform *xform = new osg::MatrixTransform;
	xform->setDataVariance( osg::Object::STATIC );
	xform->setMatrix( osg::Matrix::rotate(
		osg::DegreesToRadians( static_cast<float>(r) ), osg::Vec3( 0, 1, 0 ),
		osg::DegreesToRadians( static_cast<float>(p) ), osg::Vec3( 1, 0, 0 ),
		osg::DegreesToRadians( static_cast<float>(h) ), osg::Vec3( 0, 0, 1 ) ));
	xform->addChild( node );
	return xform;
    }
};


// Add ourself to the Registry to instantiate the reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterROT> g_readerWriter_ROT_Proxy;

/*EOF*/

