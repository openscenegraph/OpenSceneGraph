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
#include <memory>

#include <osg/Notify>
#include <osgDB/ReaderWriter>
#include <osgDB/FileNameUtils>
#include <osgDB/Registry>

#include <OpenThreads/ScopedLock>

#include "ReaderWriterDAE.h"
#include "daeReader.h"
#include "daeWriter.h"

#ifdef WIN32
#include "windows.h"
#endif

#define SERIALIZER() OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_serializerMutex)  

osgDB::ReaderWriter::ReadResult
ReaderWriterDAE::readNode(const std::string& fname,
        const osgDB::ReaderWriter::Options* options) const
{
    SERIALIZER();

    bool bOwnDAE = false;
    DAE* pDAE = NULL;
   
    if ( options )
        pDAE = (DAE*) options->getPluginData("DAE");        
    
    std::string ext( osgDB::getLowerCaseFileExtension(fname) );
    if( ! acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;

    std::string fileName( osgDB::findDataFile( fname, options ) );
    if( fileName.empty() ) return ReadResult::FILE_NOT_FOUND;

    osg::notify(osg::INFO) << "ReaderWriterDAE( \"" << fileName << "\" )" << std::endl;

    if (NULL == pDAE)
    {
        bOwnDAE = true;
        pDAE = new DAE;
    }

    osgdae::daeReader daeReader(pDAE) ;

    // Convert file name to URI
    std::string fileURI = ConvertFilePathToColladaCompatibleURI(fileName);

    if ( ! daeReader.convert( fileURI ) )
    {
        osg::notify( osg::WARN ) << "Load failed in COLLADA DOM conversion" << std::endl;
        return ReadResult::ERROR_IN_READING_FILE;
    }

    if ( options )
    {
        // Return the document URI
        if (options->getPluginData("DAE-DocumentURI"))
            *(std::string*)options->getPluginData("DAE-DocumentURI") = fileURI;
        // Return some additional information about the document
        if (options->getPluginData("DAE-AssetUnitName"))
             *(std::string*)options->getPluginData("DAE-AssetUnitName") = daeReader.m_AssetUnitName;
        if (options->getPluginData("DAE-AssetUnitMeter"))
            *(float*)options->getPluginData("DAE-AssetUnitMeter") = daeReader.m_AssetUnitMeter;
        if (options->getPluginData("DAE-AssetUp_axis"))
            *(domUpAxisType*)options->getPluginData("DAE-AssetUp_axis") = daeReader.m_AssetUp_axis;
   }

    if (bOwnDAE)
        delete pDAE;

    osg::Node* rootNode( daeReader.getRootNode() );
    return rootNode;
}

///////////////////////////////////////////////////////////////////////////

osgDB::ReaderWriter::WriteResult
ReaderWriterDAE::writeNode( const osg::Node& node,
        const std::string& fname, const osgDB::ReaderWriter::Options* options ) const
{
    SERIALIZER();

    bool bOwnDAE = false;
    DAE* pDAE = NULL;

    std::string ext( osgDB::getLowerCaseFileExtension(fname) );
    if( ! acceptsExtension(ext) ) return WriteResult::FILE_NOT_HANDLED;

    // Process options
    bool usePolygon(false);
    bool GoogleMode(false);
    if( options )
    {
        pDAE = (DAE*) options->getPluginData("DAE");        
        std::istringstream iss( options->getOptionString() );
        std::string opt;

      while( std::getline( iss, opt, ',' ) )
      {
        if( opt == "polygon")  usePolygon = true;
        else if (opt == "GoogleMode") GoogleMode = true;
        else
        {
          osg::notify(osg::WARN)
              << "\n" "COLLADA dae plugin: unrecognized option \"" << opt << "\"\n"
              << "comma-delimited options:\n"
              << "\tpolygon = use polygons instead of polylists for element\n"
              << "\tGoogleMode = write files suitable for use by Google products\n"
              << "example: osgviewer -O polygon bar.dae" "\n"
              << std::endl;
        }
      }
    }
    
    if (NULL == pDAE)
    {
        bOwnDAE = true;
        pDAE = new DAE;
    }

    // Convert file name to URI
    std::string fileURI = ConvertFilePathToColladaCompatibleURI(fname);

    osgdae::daeWriter daeWriter(pDAE, fileURI, usePolygon, GoogleMode );
    daeWriter.setRootNode( node );
    const_cast<osg::Node*>(&node)->accept( daeWriter );

    osgDB::ReaderWriter::WriteResult retVal( WriteResult::ERROR_IN_WRITING_FILE );
    if ( daeWriter.isSuccess() )
    {
        if ( daeWriter.writeFile() )
            retVal = WriteResult::FILE_SAVED;
    }
    
    if ( options )
    {
        if (!bOwnDAE)
        {
            // Return the document URI used so that users of an external DAE object
            // can locate the correct database
            if (options->getPluginData("DAE-DocumentURI"))
                *(std::string*)options->getPluginData("DAE-DocumentURI") = fileURI;
        }
    }

    if (bOwnDAE)
        delete pDAE;

    return retVal;
}

std::string ReaderWriterDAE::ConvertFilePathToColladaCompatibleURI(const std::string& FilePath)
{
    return cdom::nativePathToUri(FilePath);
}


///////////////////////////////////////////////////////////////////////////
// Add ourself to the Registry to instantiate the reader/writer.

REGISTER_OSGPLUGIN(dae, ReaderWriterDAE)

// vim: set sw=4 ts=8 et ic ai:
