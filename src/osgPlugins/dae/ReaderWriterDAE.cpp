/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
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

#include <sstream>

#include <osg/Notify>
#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#include "daeReader.h"
#include "daeWriter.h"

#define EXTENSION_NAME "dae"

///////////////////////////////////////////////////////////////////////////
// OSG reader/writer plugin for the COLLADA 1.4.x ".dae" format.
// See http://collada.org/ and http://khronos.org/collada/

class ReaderWriterDAE : public osgDB::ReaderWriter
{
public:
    ReaderWriterDAE() {}
    
    const char* className() const { return "COLLADA 1.4.x DAE reader/writer"; }

    bool acceptsExtension(const std::string& extension) const
    { 
        return osgDB::equalCaseInsensitive( extension, EXTENSION_NAME );
    }

    ReadResult readNode(const std::string&, const Options*) const;

    WriteResult writeNode(const osg::Node&, const std::string&, const Options*) const;
};

///////////////////////////////////////////////////////////////////////////

osgDB::ReaderWriter::ReadResult
ReaderWriterDAE::readNode(const std::string& fname,
        const osgDB::ReaderWriter::Options* options) const
{
    std::string ext( osgDB::getLowerCaseFileExtension(fname) );
    if( ! acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;

    std::string fileName( osgDB::findDataFile( fname, options ) );
    if( fileName.empty() ) return ReadResult::FILE_NOT_FOUND;

    osg::notify(osg::INFO) << "ReaderWriterDAE( \"" << fileName << "\" )" << std::endl;

    osgdae::daeReader daeReader;
    std::string fileURI( osgDB::convertFileNameToUnixStyle(fileName) );
    if ( ! daeReader.convert( fileURI ) )
    {
        osg::notify( osg::WARN ) << "Load failed in COLLADA DOM conversion" << std::endl;
        return ReadResult::ERROR_IN_READING_FILE;
    }

    osg::Node* rootNode( daeReader.getRootNode() );
    return rootNode;
}

///////////////////////////////////////////////////////////////////////////

osgDB::ReaderWriter::WriteResult
ReaderWriterDAE::writeNode( const osg::Node& node,
        const std::string& fname, const osgDB::ReaderWriter::Options* options ) const
{
    std::string ext( osgDB::getLowerCaseFileExtension(fname) );
    if( ! acceptsExtension(ext) ) return WriteResult::FILE_NOT_HANDLED;

    // Process options
    bool usePolygon(false);
    if( options )
    {
      std::istringstream iss( options->getOptionString() );
      std::string opt;

      while( std::getline( iss, opt, ',' ) )
      {
        if( opt == "polygon")  usePolygon = true;
        else
        {
          osg::notify(osg::WARN)
              << "\n" "COLLADA dae plugin: unrecognized option \"" << opt << "\"\n"
              << "comma-delimited options:\n"
              << "\tpolygon = use polygons instead of polylists for element\n"
              << "example: osgviewer -O polygon bar.dae" "\n"
              << std::endl;
        }
      }
    }
    
    osgdae::daeWriter daeWriter( fname, usePolygon );
    daeWriter.setRootNode( node );
    const_cast<osg::Node*>(&node)->accept( daeWriter );

    osgDB::ReaderWriter::WriteResult retVal( WriteResult::ERROR_IN_WRITING_FILE );
    if ( daeWriter.isSuccess() )
    {
        if ( daeWriter.writeFile() )
        {
            retVal = WriteResult::FILE_SAVED;
        }
    }
    
    return retVal;
}

///////////////////////////////////////////////////////////////////////////
// Add ourself to the Registry to instantiate the reader/writer.

osgDB::RegisterReaderWriterProxy<ReaderWriterDAE> g_readerWriter_DAE_Proxy;

// vim: set sw=4 ts=8 et ic ai:
