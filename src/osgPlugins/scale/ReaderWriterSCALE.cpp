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

/* file:	src/osgPlugins/scale/ReaderWriterSCALE.cpp
 * author:	Mike Weiblen http://mew.cx/ 2004-07-15
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

#define EXTENSION_NAME "scale"

///////////////////////////////////////////////////////////////////////////

/**
 * An OSG reader plugin for the ".scale" pseudo-loader, which inserts a
 * scale transform above the loaded geometry.
 * This pseudo-loader make it simple to change the size of a saved model
 * by specifying a correcting scale factor as part of the filename.
 *
 * Usage: <modelfile.ext>.<sx>,<sy>,<sz>.globe
 *	  <modelfile.ext>.<su>.globe
 * where:
 *	<modelfile.ext> = an model filename.
 *	<sx> = scale factor along the X axis.
 *	<sy> = scale factor along the Y axis.
 *	<sz> = scale factor along the Z axis.
 *	<su> = uniform scale factor applied to all axes.
 *
 * example: osgviewer cow.osg.5.scale cessna.osg
 */

class ReaderWriterSCALE : public osgDB::ReaderWriter
{
public:
    ReaderWriterSCALE() { }
    
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

	int sx, sy, sz;
	int count = sscanf( params.c_str(), "%d,%d,%d", &sx, &sy, &sz );
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
osgDB::RegisterReaderWriterProxy<ReaderWriterSCALE> g_readerWriter_SCALE_Proxy;

/*EOF*/

