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
#include <osgDB/ConvertUTF>

#include <OpenThreads/ScopedLock>

#include "ReaderWriterDAE.h"
#include "daeReader.h"
#include "daeWriter.h"

#ifdef WIN32
#include "windows.h"
#endif

#define SERIALIZER() OpenThreads::ScopedLock<OpenThreads::ReentrantMutex> lock(_serializerMutex)

osgDB::ReaderWriter::ReadResult
ReaderWriterDAE::readNode(std::istream& fin,
        const osgDB::ReaderWriter::Options* options) const
{
    SERIALIZER();

    bool bOwnDAE = false;
    DAE* pDAE = NULL;

    // Process options
    osgDAE::daeReader::Options pluginOptions;
    if( options )
    {
        pDAE = (DAE*)options->getPluginData("DAE");

        pluginOptions.precisionHint = options->getPrecisionHint();

        std::istringstream iss( options->getOptionString() );
        std::string opt;
        while (iss >> opt)
        {
            if( opt == "StrictTransparency") pluginOptions.strictTransparency = true;
            else if (opt == "daeTessellateNone") pluginOptions.tessellateMode = osgDAE::daeReader::TESSELLATE_NONE;
            else if (opt == "daeTessellatePolygonsAsTriFans") pluginOptions.tessellateMode = osgDAE::daeReader::TESSELLATE_POLYGONS_AS_TRIFAN;
            else if (opt == "daeTessellatePolygons") pluginOptions.tessellateMode = osgDAE::daeReader::TESSELLATE_POLYGONS;
            else if (opt == "daeUsePredefinedTextureUnits") pluginOptions.usePredefinedTextureUnits = true;
            else if (opt == "daeUseSequencedTextureUnits") pluginOptions.usePredefinedTextureUnits = false;
        }
    }

    if (NULL == pDAE)
    {
        bOwnDAE = true;
        pDAE = new DAE;
    }

    std::auto_ptr<DAE> scopedDae(bOwnDAE ? pDAE : NULL);        // Deallocates locally created structure at scope exit

    osgDAE::daeReader daeReader(pDAE, &pluginOptions);

    if ( ! daeReader.convert( fin ) )
    {
        OSG_WARN << "Load failed in COLLADA DOM conversion" << std::endl;
        return ReadResult::ERROR_IN_READING_FILE;
    }

    if ( options )
    {
        // Return the document URI
        if (options->getPluginData("DAE-DocumentURI"))
            *(std::string*)options->getPluginData("DAE-DocumentURI") = std::string("/dev/null");
        // Return some additional information about the document
        if (options->getPluginData("DAE-AssetUnitName"))
             *(std::string*)options->getPluginData("DAE-AssetUnitName") = daeReader.getAssetUnitName();
        if (options->getPluginData("DAE-AssetUnitMeter"))
            *(float*)options->getPluginData("DAE-AssetUnitMeter") = daeReader.getAssetUnitMeter();
        if (options->getPluginData("DAE-AssetUp_axis"))
            *(domUpAxisType*)options->getPluginData("DAE-AssetUp_axis") = daeReader.getAssetUpAxis();
    }

    osg::Node* rootNode( daeReader.getRootNode() );
    return rootNode;
}


osgDB::ReaderWriter::ReadResult
ReaderWriterDAE::readNode(const std::string& fname,
        const osgDB::ReaderWriter::Options* options) const
{
    SERIALIZER();

    bool bOwnDAE = false;
    DAE* pDAE = NULL;

    // Process options
    osgDAE::daeReader::Options pluginOptions;
    if( options )
    {
        pDAE = (DAE*)options->getPluginData("DAE");

        pluginOptions.precisionHint = options->getPrecisionHint();

        std::istringstream iss( options->getOptionString() );
        std::string opt;
        while (iss >> opt)
        {
            if( opt == "StrictTransparency") pluginOptions.strictTransparency = true;
            else if (opt == "daeTessellateNone")              pluginOptions.tessellateMode = osgDAE::daeReader::TESSELLATE_NONE;
            else if (opt == "daeTessellatePolygonsAsTriFans") pluginOptions.tessellateMode = osgDAE::daeReader::TESSELLATE_POLYGONS_AS_TRIFAN;
            else if (opt == "daeTessellatePolygons")          pluginOptions.tessellateMode = osgDAE::daeReader::TESSELLATE_POLYGONS;
            else if (opt == "daeUsePredefinedTextureUnits") pluginOptions.usePredefinedTextureUnits = true;
            else if (opt == "daeUseSequencedTextureUnits")  pluginOptions.usePredefinedTextureUnits = false;
        }
    }


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
    std::auto_ptr<DAE> scopedDae(bOwnDAE ? pDAE : NULL);        // Deallocates locally created structure at scope exit

    osgDAE::daeReader daeReader(pDAE, &pluginOptions);

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
    osgDAE::daeWriter::Options pluginOptions;
    std::string srcDirectory( osgDB::getFilePath(node.getName().empty() ? fname : node.getName()) );        // Base dir when relativising images paths
    if( options )
    {
        pDAE = (DAE*)options->getPluginData("DAE");

        const std::string & baseDir = options->getPluginStringData("baseImageDir");        // Rename "srcModelPath" (and call getFilePath() on it)?
        if (!baseDir.empty()) srcDirectory = baseDir;

        const std::string & relativiseImagesPathNbUpDirs = options->getPluginStringData("DAE-relativiseImagesPathNbUpDirs");
        if (!relativiseImagesPathNbUpDirs.empty()) {
            std::istringstream iss(relativiseImagesPathNbUpDirs);
            iss >> pluginOptions.relativiseImagesPathNbUpDirs;
        }

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
            if( opt == "polygon") pluginOptions.usePolygons = true;
            else if (opt == "GoogleMode") pluginOptions.googleMode = true;
            else if (opt == "NoExtras") pluginOptions.writeExtras = false;
            else if (opt == "daeEarthTex") pluginOptions.earthTex = true;
            else if (opt == "daeZUpAxis") {}    // Nothing (old option)
            else if (opt == "daeLinkOriginalTexturesNoForce") { pluginOptions.linkOrignialTextures = true; pluginOptions.forceTexture = false; }
            else if (opt == "daeLinkOriginalTexturesForce")   { pluginOptions.linkOrignialTextures = true; pluginOptions.forceTexture = true; }
            else if (opt == "daeNamesUseCodepage") pluginOptions.namesUseCodepage = true;
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
    std::auto_ptr<DAE> scopedDae(bOwnDAE ? pDAE : NULL);        // Deallocates locally created structure at scope exit

    // Convert file name to URI
    std::string fileURI = ConvertFilePathToColladaCompatibleURI(fname);

    osg::NodeVisitor::TraversalMode traversalMode = pluginOptions.writeExtras ? osg::NodeVisitor::TRAVERSE_ALL_CHILDREN : osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN;

    osgDAE::daeWriter daeWriter(pDAE, fileURI, osgDB::getFilePath(fname), srcDirectory, options, traversalMode, &pluginOptions);
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

    return retVal;
}

static void replace(std::string & str, const char from, const std::string & to)
{
    // Replace for all occurrences
    for(std::string::size_type pos=str.find(from); pos!=std::string::npos; pos=str.find(from))
    {
        str.replace(pos, 1, to);
    }
}

static void replace(std::string & str, const std::string & from, const std::string & to)
{
    // Replace for all occurrences
    std::size_t lenFrom = from.size();
    std::size_t lenTo = to.size();
    for(std::string::size_type pos=str.find(from); pos!=std::string::npos; pos = str.find(from, pos+lenTo))
    {
        str.replace(pos, lenFrom, to);
    }
}

std::string ReaderWriterDAE::ConvertFilePathToColladaCompatibleURI(const std::string& FilePath)
{
#ifdef OSG_USE_UTF8_FILENAME
    std::string path( cdom::nativePathToUri( FilePath ) );
#else
    std::string path( cdom::nativePathToUri( osgDB::convertStringFromCurrentCodePageToUTF8(FilePath) ) );
#endif

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

std::string ReaderWriterDAE::ConvertColladaCompatibleURIToFilePath(const std::string& uri)
{
    // Reciprocal of ConvertFilePathToColladaCompatibleURI()
#ifdef OSG_USE_UTF8_FILENAME
    std::string path( cdom::uriToNativePath( uri ) );
#else
    std::string path( osgDB::convertStringFromCurrentCodePageToUTF8( cdom::uriToNativePath(uri) ) );
#endif
    replace(path, "%23", "#");
    return path;
}

///////////////////////////////////////////////////////////////////////////
// Add ourself to the Registry to instantiate the reader/writer.

REGISTER_OSGPLUGIN(dae, ReaderWriterDAE)

// vim: set sw=4 ts=8 et ic ai:
