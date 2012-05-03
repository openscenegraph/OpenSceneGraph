#include <osg/ImageSequence>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

// _fileNames
static bool checkFileNames( const osg::ImageSequence& image )
{
    return image.getNumImageFiles()>0;
}

static bool readFileNames( osgDB::InputStream& is, osg::ImageSequence& image )
{
    unsigned int files = 0; is >> files >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<files; ++i )
    {
        std::string filename; is.readWrappedString( filename );
        image.addImageFile( filename );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeFileNames( osgDB::OutputStream& os, const osg::ImageSequence& image )
{
    const osg::ImageSequence::FileNames& files = image.getFileNames();
    os.writeSize(files.size()); os << os.BEGIN_BRACKET << std::endl;
    for ( osg::ImageSequence::FileNames::const_iterator itr=files.begin();
          itr!=files.end(); ++itr )
    {
        os.writeWrappedString( *itr );
        os << std::endl;
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

// _images
static bool checkImages( const osg::ImageSequence& image )
{
    return image.getNumImages()>0;
}

static bool readImages( osgDB::InputStream& is, osg::ImageSequence& image )
{
    unsigned int images = is.readSize(); is >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<images; ++i )
    {
        osg::Image* img = dynamic_cast<osg::Image*>( is.readObject() );
        if ( img ) image.addImage( img );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeImages( osgDB::OutputStream& os, const osg::ImageSequence& image )
{
    const osg::ImageSequence::Images& images = image.getImages();
    os.writeSize(images.size()); os << os.BEGIN_BRACKET << std::endl;
    for ( osg::ImageSequence::Images::const_iterator itr=images.begin();
          itr!=images.end(); ++itr )
    {
        os.writeObject( (*itr).get() );
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( ImageSequence,
                         new osg::ImageSequence,
                         osg::ImageSequence,
                         "osg::Object osg::Image osg::ImageStream osg::ImageSequence" )
{
    ADD_DOUBLE_SERIALIZER( ReferenceTime, DBL_MAX );  // _referenceTime
    ADD_DOUBLE_SERIALIZER( TimeMultiplier, 1.0 );  // _timeMultiplier

    BEGIN_ENUM_SERIALIZER( Mode, PRE_LOAD_ALL_IMAGES );
        ADD_ENUM_VALUE( PRE_LOAD_ALL_IMAGES );
        ADD_ENUM_VALUE( PAGE_AND_RETAIN_IMAGES );
        ADD_ENUM_VALUE( PAGE_AND_DISCARD_USED_IMAGES );
    END_ENUM_SERIALIZER();  // _mode

    ADD_DOUBLE_SERIALIZER( Length, 1.0 );  // _length
    ADD_USER_SERIALIZER( FileNames );  // _fileNames
    ADD_USER_SERIALIZER( Images );  // _images
}
