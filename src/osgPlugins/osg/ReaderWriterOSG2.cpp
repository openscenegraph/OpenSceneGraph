/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2010 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/
// Written by Wang Rui, (C) 2010

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/Registry>
#include <osgDB/ObjectWrapper>

using namespace osgDB;

class ReaderWriterOSG2 : public osgDB::ReaderWriter
{
public:
    ReaderWriterOSG2()
    {
        supportsExtension( "osg2", "OpenSceneGraph extendable format" );
        supportsExtension( "osgt", "OpenSceneGraph extendable ascii format" );
        supportsExtension( "osgb", "OpenSceneGraph extendable binary format" );
        
        supportsOption( "Ascii", "Import/Export option: Force the writer export ascii file" );
        supportsOption( "SchemaFile=<file>", "Import/Export option: Use/Record a ascii schema file" );
        supportsOption( "Compressor=<name>", "Export option: Use an inbuilt or user-defined compressor" );
        supportsOption( "WriteImageHint=<hint>", "Export option: Hint of writing image to stream: "
                        "<IncludeData> writes Image::data() directly; "
                        "<IncludeFile> writes the image file itself to stream; "
                        "<UseExternal> writes only the filename; "
                        "<WriteOut> writes Image::data() to disk as external file." );

        std::string filename = osgDB::Registry::instance()->createLibraryNameForExtension("serializers_osg");
        if (osgDB::Registry::instance()->loadLibrary(filename)==osgDB::Registry::LOADED)
        {
            osg::notify(osg::NOTICE)<<"Constructor ReaderWriterOSG2 - loaded OK"<<std::endl;
        }
        else
        {
            osg::notify(osg::NOTICE)<<"Constructor ReaderWriterOSG2 - failed to load"<<std::endl;
        }
    }
    
    virtual const char* className() const
    { return "OpenSceneGraph Native Format Reader/Writer"; }
    
    virtual ReadResult readObject( const std::string& file, const Options* options ) const
    { return readNode(file, options); }
    
    virtual ReadResult readObject( std::istream& fin, const Options* options ) const
    { return readNode(fin, options); }
    
    virtual ReadResult readImage( const std::string& file, const Options* options ) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension( file );
        if ( !acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;
        std::string fileName = osgDB::findDataFile( file, options );
        if ( fileName.empty() ) return ReadResult::FILE_NOT_FOUND;
        
        osg::ref_ptr<Options> local_opt = options ?
            static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
        local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));
        
        osgDB::ifstream istream( fileName.c_str(), std::ios::out|std::ios::binary );
        return readImage( istream, local_opt.get() );
    }
    
    virtual ReadResult readImage( std::istream& fin, const Options* options ) const
    {
        try
        {
            InputStream is( &fin, options );
            if ( is.start()!=InputStream::READ_IMAGE )
                return ReadResult::FILE_NOT_HANDLED;
            is.decompress();
            return is.readImage();
        }
        catch ( InputException e )
        {
            return e.getError() + " At " + e.getField();
        }
    }
    
    virtual ReadResult readNode( const std::string& file, const Options* options ) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension( file );
        if ( !acceptsExtension(ext) ) return ReadResult::FILE_NOT_HANDLED;
        std::string fileName = osgDB::findDataFile( file, options );
        if ( fileName.empty() ) return ReadResult::FILE_NOT_FOUND;
        
        osg::ref_ptr<Options> local_opt = options ?
            static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
        local_opt->getDatabasePathList().push_front(osgDB::getFilePath(fileName));
        
        osgDB::ifstream istream( fileName.c_str(), std::ios::out|std::ios::binary );
        return readNode( istream, local_opt.get() );
    }
    
    virtual ReadResult readNode( std::istream& fin, const Options* options ) const
    {
        try
        {
            InputStream is( &fin, options );
            if ( is.start()!=InputStream::READ_SCENE )
                return ReadResult::FILE_NOT_HANDLED;
            is.decompress();
            return dynamic_cast<osg::Node*>( is.readObject() );
        }
        catch ( InputException e )
        {
            return e.getError() + " At " + e.getField();
        }
    }
    
    virtual WriteResult writeObject( const osg::Object& object, const std::string& fileName, const Options* options ) const
    {
        const osg::Node* node = dynamic_cast<const osg::Node*>( &object );
        if ( node ) return writeNode( *node, fileName, options );
        
        const osg::Image* image = dynamic_cast<const osg::Image*>( &object );
        if ( image ) return writeImage( *image, fileName, options );
        return WriteResult::FILE_NOT_HANDLED;
    }
    
    virtual WriteResult writeObject( const osg::Object& object, std::ostream& fout, const Options* options ) const
    {
        const osg::Node* node = dynamic_cast<const osg::Node*>( &object );
        if ( node ) return writeNode( *node, fout, options );
        
        const osg::Image* image = dynamic_cast<const osg::Image*>( &object );
        if ( image ) return writeImage( *image, fout, options );
        return WriteResult::FILE_NOT_HANDLED;
    }
    
    virtual WriteResult writeImage( const osg::Image& image, const std::string& fileName, const Options* options ) const
    {
        std::string ext = osgDB::getFileExtension( fileName );
        if ( !acceptsExtension(ext) ) return WriteResult::FILE_NOT_HANDLED;
        
        osg::ref_ptr<Options> local_opt = options ?
            static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
        if( local_opt->getDatabasePathList().empty() )
            local_opt->setDatabasePath( osgDB::getFilePath(fileName) );
        if ( ext=="osgt" ) local_opt->setOptionString( local_opt->getOptionString() + " Ascii" );
        
        osgDB::ofstream fout( fileName.c_str(), std::ios::out|std::ios::binary );
        if ( !fout ) return WriteResult::ERROR_IN_WRITING_FILE;
        
        WriteResult result = writeImage( image, fout, local_opt.get() );
        fout.close();
        return result;
    }
    
    virtual WriteResult writeImage( const osg::Image& image, std::ostream& fout, const Options* options ) const
    {
        try
        {
            OutputStream os( &fout, options );
            os.start( OutputStream::WRITE_IMAGE );
            os.writeImage( &image );
            os.compress( &fout );
            
            if ( fout.fail() ) return WriteResult::ERROR_IN_WRITING_FILE;
            return WriteResult::FILE_SAVED;
        }
        catch ( OutputException e )
        {
            osg::notify(osg::WARN) << "ReaderWriterOSG2::writeImage(): " << e.getError()
                                   << " At " << e.getField() << std::endl;
        }
        return WriteResult::FILE_NOT_HANDLED;
    }
    
    virtual WriteResult writeNode( const osg::Node& node, const std::string& fileName, const Options* options ) const
    {
        std::string ext = osgDB::getFileExtension( fileName );
        if ( !acceptsExtension(ext) ) return WriteResult::FILE_NOT_HANDLED;
        
        osg::ref_ptr<Options> local_opt = options ?
            static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
        if ( local_opt->getDatabasePathList().empty() )
            local_opt->setDatabasePath( osgDB::getFilePath(fileName) );
        if ( ext=="osgt" ) local_opt->setOptionString( local_opt->getOptionString() + " Ascii" );
        
        osgDB::ofstream fout( fileName.c_str(), std::ios::out|std::ios::binary );
        if ( !fout ) return WriteResult::ERROR_IN_WRITING_FILE;
        
        WriteResult result = writeNode( node, fout, local_opt.get() );
        fout.close();
        return result;
    }
    
    virtual WriteResult writeNode( const osg::Node& node, std::ostream& fout, const Options* options ) const
    {
        try
        {
            OutputStream os( &fout, options );
            os.start( OutputStream::WRITE_SCENE );
            os.writeObject( &node );
            os.compress( &fout );
            
            if ( fout.fail() ) return WriteResult::ERROR_IN_WRITING_FILE;
            return WriteResult::FILE_SAVED;
        }
        catch ( OutputException e )
        {
            osg::notify(osg::WARN) << "ReaderWriterOSG2::writeNode(): " << e.getError()
                                   << " At " << e.getField() << std::endl;
        }
        return WriteResult::FILE_NOT_HANDLED;
    }
};

REGISTER_OSGPLUGIN( osg2, ReaderWriterOSG2 )
