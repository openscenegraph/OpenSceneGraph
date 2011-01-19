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

#include <sstream>
#include <memory>

#include <osg/Notify>
#include <osg/NodeVisitor>
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
        pDAE = (DAE*)options->getPluginData("DAE");

    std::string ext( osgDB::getLowerCaseFileExtension(fname) );
    if( ! acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;

    std::string fileName( osgDB::findDataFile( fname, options ) );
    if( fileName.empty() ) return ReadResult::FILE_NOT_FOUND;

    OSG_INFO << "ReaderWriterDAE( \"" << fileName << "\" )" << std::endl;

    if (NULL == pDAE)
    {
        bOwnDAE = true;
        pDAE = new DAE;
    }

    osgDAE::daeReader daeReader(pDAE,
                                options && options->getOptionString().find("StrictTransparency") != std::string::npos,
                                options ? options->getPrecisionHint() : 0 ) ;

    // Convert file name to URI
    std::string fileURI = ConvertFilePathToColladaCompatibleURI(fileName);

    if ( ! daeReader.convert( fileURI ) )
    {
        OSG_WARN << "Load failed in COLLADA DOM conversion" << std::endl;
        return ReadResult::ERROR_IN_READING_FILE;
    }

    if ( options )
    {
        // Return the document URI
        if (options->getPluginData("DAE-DocumentURI"))
            *(std::string*)options->getPluginData("DAE-DocumentURI") = fileURI;
        // Return some additional information about the document
        if (options->getPluginData("DAE-AssetUnitName"))
             *(std::string*)options->getPluginData("DAE-AssetUnitName") = daeReader.getAssetUnitName();
        if (options->getPluginData("DAE-AssetUnitMeter"))
            *(float*)options->getPluginData("DAE-AssetUnitMeter") = daeReader.getAssetUnitMeter();
        if (options->getPluginData("DAE-AssetUp_axis"))
            *(domUpAxisType*)options->getPluginData("DAE-AssetUp_axis") = daeReader.getAssetUpAxis();
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
    bool usePolygon(false);   // Use plugin option "polygon" to enable
    bool googleMode(false);   // Use plugin option "GoogleMode" to enable
    bool writeExtras(true);   // Use plugin option "NoExtras" to disable
    bool earthTex(false);     // Use plugin option "DaeEarthTex" to enable
    bool zUpAxis(false);      // Use plugin option "ZUpAxis" to enable
    bool forceTexture(false); // Use plugin option "ForceTexture" to enable
    if( options )
    {
        pDAE = (DAE*)options->getPluginData("DAE");
        // Sukender's note: I don't know why DAE seems to accept comma-sparated options instead of space-separated options as other ReaderWriters. However, to avoid breaking compatibility, here's a workaround:
        std::string optString( options->getOptionString() );
        for(std::string::iterator it=optString.begin(); it!=optString.end(); ++it) {
            if (*it == ' ') *it = ',';
        }
        std::istringstream iss( optString );
        std::string opt;

        //while (iss >> opt)
        while( std::getline( iss, opt, ',' ) )
        {
            if( opt == "polygon")  usePolygon = true;
            else if (opt == "GoogleMode") googleMode = true;
            else if (opt == "NoExtras") writeExtras = false;
            else if (opt == "daeEarthTex") earthTex = true;
            else if (opt == "daeZUpAxis") zUpAxis = true;
            else if (opt == "daeForceTexture") forceTexture = true;
            else if (!opt.empty())
            {
                OSG_NOTICE << std::endl << "COLLADA dae plugin: unrecognized option \"" << opt <<  std::endl;
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

    osg::NodeVisitor::TraversalMode traversalMode = writeExtras ? osg::NodeVisitor::TRAVERSE_ALL_CHILDREN : osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN;

    osgDAE::daeWriter daeWriter(pDAE, fileURI, usePolygon, googleMode, traversalMode, writeExtras, earthTex, zUpAxis, forceTexture);
    daeWriter.setRootNode( node );
    const_cast<osg::Node*>(&node)->accept( daeWriter );

    osgDB::ReaderWriter::WriteResult retVal( WriteResult::ERROR_IN_WRITING_FILE );
    if ( daeWriter.isSuccess() )
    {
        if (pDAE->write(fileURI))
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

static void replace(std::string & str, const char from, const std::string & to) {
    // Replace for all occurences
    for(std::string::size_type pos=str.find(from); pos!=std::string::npos; pos=str.find(from))
    {
        str.replace(pos, 1, to);
    }
}

std::string ReaderWriterDAE::ConvertFilePathToColladaCompatibleURI(const std::string& FilePath)
{
    std::string path( cdom::nativePathToUri(FilePath) );

    // Unfortunately, cdom::nativePathToUri() does not convert '#' characters to "%23" as expected.
    // So having /a/#b/c will generate a wrong conversion, as '#' will be misinterpreted as an URI fragment.
    // Here are listed all special chars, but only # was found problematic. I (Sukender) tested #{}^~[]`;@=&$ under Windows.
    // Uncomment lines if you find issues with some other special characters.

    //replace(path, '%', "%25");        // % at first
    //replace(path, ' ', "%20");
    replace(path, '#', "%23");
    //replace(path, '<', "%3C");
    //replace(path, '>', "%3E");
    //replace(path, '{', "%7B");
    //replace(path, '}', "%7D");
    //replace(path, '|', "%7C");
    //replace(path, '^', "%5E");
    //replace(path, '~', "%7E");
    //replace(path, '[', "%5B");
    //replace(path, '}', "%5D");
    //replace(path, '`', "%60");
    //replace(path, ';', "%3B");
    //replace(path, '?', "%3F");
    //replace(path, '@', "%40");
    //replace(path, '=', "%3D");
    //replace(path, '&', "%26");
    //replace(path, '$', "%24");
    return path;
}


///////////////////////////////////////////////////////////////////////////
// Add ourself to the Registry to instantiate the reader/writer.

REGISTER_OSGPLUGIN(dae, ReaderWriterDAE)

// vim: set sw=4 ts=8 et ic ai:
