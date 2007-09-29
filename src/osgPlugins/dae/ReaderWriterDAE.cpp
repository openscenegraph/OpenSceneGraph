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
#include <OpenThreads/ReentrantMutex>

#include "daeReader.h"
#include "daeWriter.h"

#define EXTENSION_NAME "dae"

#define SERIALIZER() OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_serializerMutex)  

///////////////////////////////////////////////////////////////////////////
// OSG reader/writer plugin for the COLLADA 1.4.x ".dae" format.
// See http://collada.org/ and http://khronos.org/collada/

class ReaderWriterDAE : public osgDB::ReaderWriter
{
public:
    ReaderWriterDAE() : _dae(NULL)
    {
    }

    ~ReaderWriterDAE()
    {
        if(_dae != NULL){
            delete _dae;
            DAE::cleanup();
            _dae = NULL;
        }
    }

    const char* className() const { return "COLLADA 1.4.x DAE reader/writer"; }

    bool acceptsExtension(const std::string& extension) const
    { 
        return osgDB::equalCaseInsensitive( extension, EXTENSION_NAME );
    }

    ReadResult readNode(const std::string&, const Options*) const;

    WriteResult writeNode(const osg::Node&, const std::string&, const Options*) const;
  
private:

    mutable DAE *_dae;
    mutable OpenThreads::ReentrantMutex _serializerMutex;

};

///////////////////////////////////////////////////////////////////////////

osgDB::ReaderWriter::ReadResult
ReaderWriterDAE::readNode(const std::string& fname,
        const osgDB::ReaderWriter::Options* options) const
{
    SERIALIZER();
   
    DAE* daeptr = 0L;
   
    if ( options ) {        
        daeptr = (DAE*) options->getPluginData("DAE");        
    }

    std::string ext( osgDB::getLowerCaseFileExtension(fname) );
    if( ! acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;

    std::string fileName( osgDB::findDataFile( fname, options ) );
    if( fileName.empty() ) return ReadResult::FILE_NOT_FOUND;

    osg::notify(osg::INFO) << "ReaderWriterDAE( \"" << fileName << "\" )" << std::endl;

   
    if (daeptr == NULL) {
        if (_dae == NULL) 
                _dae = new DAE();
        daeptr = _dae;
    }

    osgdae::daeReader daeReader(daeptr) ;
    std::string fileURI( osgDB::convertFileNameToUnixStyle(fileName) );
    if ( ! daeReader.convert( fileURI ) )
    {
        osg::notify( osg::WARN ) << "Load failed in COLLADA DOM conversion" << std::endl;
        return ReadResult::ERROR_IN_READING_FILE;
    }

    if ( options ) {
        // return DAE* used
        options->setPluginData("DAE", daeptr);
        // and filename document was stored as in database, does not have to be
        // the same as fname
        options->setPluginData("DAE-DocumentFileName", ( fileURI[1] == ':' ?  
              (void*) new std::auto_ptr<std::string>(new std::string('/'+fileURI)) :
              (void*) new std::auto_ptr<std::string>(new std::string(fileURI)) ) 
        );
    }

    osg::Node* rootNode( daeReader.getRootNode() );
    return rootNode;
}

///////////////////////////////////////////////////////////////////////////

osgDB::ReaderWriter::WriteResult
ReaderWriterDAE::writeNode( const osg::Node& node,
        const std::string& fname, const osgDB::ReaderWriter::Options* options ) const
{
    SERIALIZER();

    DAE* daeptr = 0L;

    std::string ext( osgDB::getLowerCaseFileExtension(fname) );
    if( ! acceptsExtension(ext) ) return WriteResult::FILE_NOT_HANDLED;

    if ( options ) {        
        daeptr = (DAE*) options->getPluginData("DAE");        
    }

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
    
    if (daeptr == NULL) {
        if (_dae == NULL) 
                _dae = new DAE();
        daeptr = _dae;
    }

    osgdae::daeWriter daeWriter(daeptr, fname, usePolygon );
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
    
    if ( options ) {
        // return DAE* used
        options->setPluginData("DAE", daeptr);

        // saving filename so read and write work the same way,
        // this could be skipped since write does not currently modify the
        // filename which load might do (under windows for example)
        options->setPluginData("DAE-DocumentFileName", (void*) new
                std::auto_ptr<std::string>(new std::string(fname)));
    }

    return retVal;
}

///////////////////////////////////////////////////////////////////////////
// Add ourself to the Registry to instantiate the reader/writer.

REGISTER_OSGPLUGIN(dae, ReaderWriterDAE)

// vim: set sw=4 ts=8 et ic ai:
