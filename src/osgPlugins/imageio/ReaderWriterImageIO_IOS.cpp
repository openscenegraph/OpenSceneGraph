#include <osg/GL>
#include <osg/Notify>
#include <osg/Image>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <sstream> // for istream
#include <iostream> // for ios::

#import <UIKit/UIImage.h>
#import <CoreGraphics/CoreGraphics.h>
#import <Foundation/Foundation.h>
#import <ImageIO/CGImageSource.h>

/**************************************************************
 ***** Begin Callback functions for istream block reading *****
 **************************************************************/

// This callback reads some bytes from an istream and copies it
// to a Quartz buffer (supplied by Apple framework).
size_t MyProviderGetBytesCallback(void* istream_userdata, void* quartz_buffer, size_t the_count)
{
    std::istream* the_istream = (std::istream*)istream_userdata;
    the_istream->read((char*)quartz_buffer, the_count);
    return the_istream->gcount(); // return the actual number of bytes read
}

// This callback is triggered when the data provider is released
// so you can clean up any resources.
void MyProviderReleaseInfoCallback(void* istream_userdata)
{
    // What should I put here? Do I need to close the istream?
    // The png and tga don't seem to.
    //    std::istream* the_istream = (std::istream*)istream_userdata;
}

void MyProviderRewindCallback(void* istream_userdata)
{
    std::istream* the_istream = (std::istream*)istream_userdata;
    the_istream->seekg(0, std::ios::beg);
}

off_t MyProviderSkipForwardBytesCallback(void* istream_userdata, off_t the_count)
{
    std::istream* the_istream = (std::istream*)istream_userdata;
    off_t start_position = the_istream->tellg();
    the_istream->seekg(the_count, std::ios::cur);
    off_t end_position = the_istream->tellg();
    return (end_position - start_position);
}

/**************************************************************
 ***** End Callback functions for istream block reading ********
 **************************************************************/


/**************************************************************
 ***** Begin Callback functions for ostream block writing ******
 **************************************************************/
size_t MyConsumerPutBytesCallback(void* ostream_userdata, const void* quartz_buffer, size_t the_count)
{
    std::ostream* the_ostream = (std::ostream*)ostream_userdata;
    the_ostream->write((char*)quartz_buffer, the_count);
    // Don't know how to get number of bytes actually written, so
    // just returning the_count.
    return the_count;
}

void MyConsumerReleaseInfoCallback(void* ostream_userdata)
{
    std::ostream* the_ostream = (std::ostream*)ostream_userdata;
    the_ostream->flush();
}
/**************************************************************
 ***** End Callback functions for ostream block writing ********
 **************************************************************/


/**************************************************************
 ***** Begin Support functions for reading (stream and file) ***
 **************************************************************/

/* Create a CGImageSourceRef from raw data */
CGImageRef CreateCGImageFromDataStream(std::istream& fin)
{
    CGImageRef image_ref = NULL;
    CGImageSourceRef source_ref;
    /* The easy way would be to use CGImageSourceCreateWithData,
     * but this presumes you have a known fixed-length buffer of data.
     * The istream makes this harder to know, so we use the ProviderCallbacks APIs
     CFDataRef the_cf_data = CFDataCreateWithBytesNoCopy(
     kCFAllocatorDefault,
     (const UInt8*)the_data,
     CFIndex length,
     kCFAllocatorNull // do not free data buffer, must do it yourself
     );
     source_ref = CGImageSourceCreateWithData(the_cf_data, NULL);
     */

    CGDataProviderSequentialCallbacks provider_callbacks =
    {
        0,
        MyProviderGetBytesCallback,
        MyProviderSkipForwardBytesCallback,
        MyProviderRewindCallback,
        MyProviderReleaseInfoCallback
    };

    CGDataProviderRef data_provider = CGDataProviderCreateSequential(&fin, &provider_callbacks);

    // If we had a way of hinting at what the data type is, we could
    // pass this hint in the second parameter.
    source_ref = CGImageSourceCreateWithDataProvider(data_provider, NULL);

    CGDataProviderRelease(data_provider);


    if(!source_ref)
    {
        return NULL;
    }

    image_ref = CGImageSourceCreateImageAtIndex(source_ref, 0, NULL);

    /* Don't need the SourceRef any more (error or not) */
    CFRelease(source_ref);

    return image_ref;
}

static NSString* toNSString(const std::string& text, NSStringEncoding nsse)
{
    NSString*  nstr = nil;

    if (!text.empty())
    {
        nstr = [NSString stringWithCString:text.c_str() encoding:nsse];
        //nstr = [NSString stringWithUTF8String:text.c_str()];// encoding:nsse]
    }

    if (nstr == nil)
    {
        nstr = @"";
    }

    return nstr;
}

// std::string to NSString with the UTF8 encoding

static NSString* toNSString(const std::string& text)
{
    return toNSString(text, NSUTF8StringEncoding);
}

//
//really basic image io for IOS
//
osg::Image* ReadCoreGraphicsImageFromFile(std::string file)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    //chop the extension off
    //std::string strExt = osgDB::getFileExtension(file);
    //std::string strPath = osgDB::getFilePath(file);
    //std::string strName = osgDB::getStrippedName(file);
    //std::string strFile = strPath + "/" + strName;

    //NSString* path = [NSString stringWithCString:strName.c_str() encoding:NSUTF8StringEncoding];
    //NSString* ext = [NSString stringWithCString:strExt.c_str() encoding:NSUTF8StringEncoding];

    //CGImageRef textureImage = [UIImage imageNamed:path].CGImage;
    //CGImageRef textureImage = [UIImage imageWithContentsOfFile:[[NSBundle mainBundle] pathForResource:path ofType:ext]].CGImage;
    NSString* path = [NSString stringWithCString:file.c_str() encoding:NSUTF8StringEncoding];
    //NSLog(@"imageio: About to open %@.\n", path);
    UIImage *img = [UIImage imageWithContentsOfFile:path];
    if (!img) {
        NSLog(@"imageio: failed to load UIImage image '%@'.\n", path);
        [pool release];
        return NULL;
    }
    CGImageRef textureImage = img.CGImage;
    if (!textureImage) {
        NSLog(@"imageio: failed to create CGImageRef.\n");
        [pool release];
        return NULL;
    }

    size_t texWidth = CGImageGetWidth(textureImage);
    size_t texHeight = CGImageGetHeight(textureImage);
    GLubyte *textureData = (GLubyte *)malloc(texWidth * texHeight * 4);
    if (!textureData) {
        NSLog(@"imageio: out of memory.\n");
        [pool release];
        return NULL;
    }

    CGColorSpaceRef csref = CGColorSpaceCreateDeviceRGB();
    if (!csref) {
        NSLog(@"imageio: failed to create CGColorSpaceRef.\n");
        free(textureData);
        [pool release];
        return NULL;
    }

    CGContextRef textureContext = CGBitmapContextCreate(textureData,
                                                        texWidth, texHeight,
                                                        8, texWidth * 4,
                                                        csref,
                                                        kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(csref);
    if (!textureContext) {
        NSLog(@"imageio: failed to create CGContextRef.\n");
        free(textureData);
        [pool release];
        return NULL;
    }

    //copy into texturedata
    CGContextDrawImage(textureContext,
                       CGRectMake(0.0f, 0.0f, (float)texWidth, (float)texHeight),
                       textureImage);
    CGContextRelease(textureContext);

    //create the osg image
    int s = texWidth;
    int t = texHeight;
    osg::Image* image = new osg::Image();
    image->setImage(s, t, 1,
                    GL_RGBA,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    textureData,
                    osg::Image::USE_MALLOC_FREE);

    //flip vertical
    image->flipVertical();

    //
    // Reverse the premultiplied alpha for avoiding unexpected darker edges
    // by Tatsuhiro Nishioka (based on SDL's workaround on the similar issue)
    // http://bugzilla.libsdl.org/show_bug.cgi?id=868
    //
    int i, j;
    GLubyte *pixels = (GLubyte *)image->data();
    for (i = image->t() * image->s(); i--; ) {

        GLubyte alpha = pixels[3];
        if (alpha && (alpha < 255)) {
            for (j = 0; j < 3; ++j) {
                pixels[j] = (static_cast<int>(pixels[j]) * 255) / alpha;
            }
        }
        pixels += 4;
    }

    [pool release];
    return image;
}

osg::Image* CreateOSGImageFromCGImage(CGImageRef textureImage)
{
    NSAutoreleasePool* pool = [[NSAutoreleasePool alloc] init];
    if (textureImage == nil) {
        [pool release];
        NSLog(@"imageio: failed to load CGImageRef image");
        return NULL;
    }

    size_t texWidth = CGImageGetWidth(textureImage);
    size_t texHeight = CGImageGetHeight(textureImage);

    GLubyte *textureData = (GLubyte *)malloc(texWidth * texHeight * 4);
    if (!textureData) {
        NSLog(@"imageio: out of memory.\n");
        [pool release];
        return NULL;
    }

    CGColorSpaceRef csref = CGColorSpaceCreateDeviceRGB();
    if (!csref) {
        NSLog(@"imageio: failed to create CGColorSpaceRef.\n");
        free(textureData);
        [pool release];
        return NULL;
    }

    CGContextRef textureContext = CGBitmapContextCreate(textureData,
                                                        texWidth, texHeight,
                                                        8, texWidth * 4,
                                                        csref,
                                                        kCGImageAlphaPremultipliedLast);
    CGColorSpaceRelease(csref);
    if (!textureContext) {
        NSLog(@"imageio: failed to create CGContextRef.\n");
        free(textureData);
        [pool release];
        return NULL;
    }

    //copy into texturedata
    CGContextDrawImage(textureContext,
                       CGRectMake(0.0f, 0.0f, (float)texWidth, (float)texHeight),
                       textureImage);
    CGContextFlush(textureContext);
    CGContextRelease(textureContext);


    //create the osg image
    int s = texWidth;
    int t = texHeight;
    osg::Image* image = new osg::Image();
    image->setImage(s, t, 1,
                    GL_RGBA,
                    GL_RGBA,
                    GL_UNSIGNED_BYTE,
                    textureData,
                    osg::Image::USE_MALLOC_FREE);

    //flip vertical
    image->flipVertical();

    //
    // Reverse the premultiplied alpha for avoiding unexpected darker edges
    // by Tatsuhiro Nishioka (based on SDL's workaround on the similar issue)
    // http://bugzilla.libsdl.org/show_bug.cgi?id=868
    //


    int i, j;
    GLubyte *pixels = (GLubyte *)image->data();
    for (i = image->t() * image->s(); i--; ) {

        GLubyte alpha = pixels[3];
        if (alpha && (alpha < 255)) {
            for (j = 0; j < 3; ++j) {
                pixels[j] = (static_cast<int>(pixels[j]) * 255) / alpha;
            }
        }
        pixels += 4;
    }

    [pool release];
    return image;
}

class ReaderWriterImageIO : public osgDB::ReaderWriter

{
public:
    ReaderWriterImageIO()
    {

        supportsExtension("jpg",   "jpg image file");
        supportsExtension("jpeg",  "jpeg image file");
        supportsExtension("jpe",   "jpe image file");
        supportsExtension("jp2",   "jp2 image file");
        supportsExtension("tiff",  "tiff image file");
        supportsExtension("tif",   "tif image file");
        supportsExtension("gif",   "gif image file");
        supportsExtension("png",   "png image file");
        supportsExtension("pict",  "pict image file");
        supportsExtension("pct",   "pct image file");
        supportsExtension("pic",   "pic image file");
        supportsExtension("bmp",   "bmp image file");
        supportsExtension("BMPf",  "BMPf image file");
        supportsExtension("ico",   "ico image file");
        supportsExtension("icns",  "icns image file");
        supportsExtension("tga",   "tga image file");
        supportsExtension("targa", "targa image file");
        supportsExtension("psd",   "psd image file");

        supportsExtension("pdf",   "pdf image file");
        supportsExtension("eps",   "eps image file");
        supportsExtension("epi",   "epi image file");
        supportsExtension("epsf",  "epsf image file");
        supportsExtension("epsi",  "epsi image file");
        supportsExtension("ps",    "postscript image file");

        supportsExtension("dng",   "dng image file");
        supportsExtension("cr2",   "cr2 image file");
        supportsExtension("crw",   "crw image file");
        supportsExtension("fpx",   "fpx image file");
        supportsExtension("fpxi",  "fpxi image file");
        supportsExtension("raf",   "raf image file");
        supportsExtension("dcr",   "dcr image file");
        supportsExtension("ptng",  "ptng image file");
        supportsExtension("pnt",   "pnt image file");
        supportsExtension("mac",   "mac image file");
        supportsExtension("mrw",   "mrw image file");
        supportsExtension("nef",   "nef image file");
        supportsExtension("orf",   "orf image file");
        supportsExtension("exr",   "exr image file");
        supportsExtension("qti",   "qti image file");
        supportsExtension("qtif",  "qtif image file");
        supportsExtension("hdr",   "hdr image file");
        supportsExtension("sgi",   "sgi image file");
        supportsExtension("srf",   "srf image file");
        supportsExtension("cur",   "cur image file");
        supportsExtension("xbm",   "xbm image file");

        supportsExtension("raw",   "raw image file");
    }

    virtual const char* className() const { return "Mac OS X ImageIO based Image Reader/Writer"; }


    virtual bool acceptsExtension(const std::string& extension) const
    {
        // ImageIO speaks in UTIs.
        // http://developer.apple.com/graphicsimaging/workingwithimageio.html
        // The Cocoa drawing guide lists these and says to use the
        // imageFileTypes class method of NSImage to get a complete
        // list of extensions. But remember ImageIO may support more formats
        // than Cocoa.
        // http://developer.apple.com/documentation/Cocoa/Conceptual/CocoaDrawingGuide/Images/chapter_7_section_3.html
        // Apple's UTI guide:
        // http://developer.apple.com/documentation/Carbon/Conceptual/understanding_utis/utilist/chapter_4_section_1.html
        return
         osgDB::equalCaseInsensitive(extension,"jpg") ||
         osgDB::equalCaseInsensitive(extension,"jpeg") ||
         osgDB::equalCaseInsensitive(extension,"jpe") ||
         osgDB::equalCaseInsensitive(extension,"jp2") ||
         osgDB::equalCaseInsensitive(extension,"tiff") ||
         osgDB::equalCaseInsensitive(extension,"tif") ||
         osgDB::equalCaseInsensitive(extension,"gif") ||
         osgDB::equalCaseInsensitive(extension,"png") ||
         osgDB::equalCaseInsensitive(extension,"pict") ||
         osgDB::equalCaseInsensitive(extension,"pct") ||
         osgDB::equalCaseInsensitive(extension,"pic") ||
         osgDB::equalCaseInsensitive(extension,"bmp") ||
         osgDB::equalCaseInsensitive(extension,"BMPf") ||
         osgDB::equalCaseInsensitive(extension,"ico") ||
         osgDB::equalCaseInsensitive(extension,"icns") ||
         osgDB::equalCaseInsensitive(extension,"tga") ||
         osgDB::equalCaseInsensitive(extension,"targa") ||
         osgDB::equalCaseInsensitive(extension,"psd") ||

         osgDB::equalCaseInsensitive(extension,"pdf") ||
         osgDB::equalCaseInsensitive(extension,"eps") ||
         osgDB::equalCaseInsensitive(extension,"epi") ||
         osgDB::equalCaseInsensitive(extension,"epsf") ||
         osgDB::equalCaseInsensitive(extension,"epsi") ||
         osgDB::equalCaseInsensitive(extension,"ps") ||

         osgDB::equalCaseInsensitive(extension,"dng") ||
         osgDB::equalCaseInsensitive(extension,"cr2") ||
         osgDB::equalCaseInsensitive(extension,"crw") ||
         osgDB::equalCaseInsensitive(extension,"fpx") ||
         osgDB::equalCaseInsensitive(extension,"fpxi") ||
         osgDB::equalCaseInsensitive(extension,"raf") ||
         osgDB::equalCaseInsensitive(extension,"dcr") ||
         osgDB::equalCaseInsensitive(extension,"ptng") ||
         osgDB::equalCaseInsensitive(extension,"pnt") ||
         osgDB::equalCaseInsensitive(extension,"mac") ||
         osgDB::equalCaseInsensitive(extension,"mrw") ||
         osgDB::equalCaseInsensitive(extension,"nef") ||
         osgDB::equalCaseInsensitive(extension,"orf") ||
         osgDB::equalCaseInsensitive(extension,"exr") ||
         osgDB::equalCaseInsensitive(extension,"qti") ||
         osgDB::equalCaseInsensitive(extension,"qtif") ||
         osgDB::equalCaseInsensitive(extension,"hdr") ||
         osgDB::equalCaseInsensitive(extension,"sgi") ||
         osgDB::equalCaseInsensitive(extension,"srf") ||
         osgDB::equalCaseInsensitive(extension,"cur") ||
         osgDB::equalCaseInsensitive(extension,"xbm") ||

         osgDB::equalCaseInsensitive(extension,"raw");
    }



    ReadResult readImageStream(std::istream& fin) const
    {
        // Call ImageIO to load the image.
        CGImageRef cg_image_ref = CreateCGImageFromDataStream(fin);
        if (NULL == cg_image_ref) return ReadResult::FILE_NOT_FOUND;

        // Create an osg::Image from the CGImageRef.
        osg::Image* osg_image = CreateOSGImageFromCGImage(cg_image_ref);

        CFRelease(cg_image_ref);
        return osg_image;
    }

    virtual ReadResult readImage(std::istream& fin, const osgDB::ReaderWriter::Options* the_options = NULL) const
    {
        ReadResult read_result = readImageStream(fin);
        return read_result;
    }

    ReadResult readImageFile(const std::string& file_name) const
    {
        //osg::notify(osg::INFO) << "imageio readImageFile: " << file_name << std::endl;

        // Create an osg::Image from the CGImageRef.
        osg::Image* osg_image = ReadCoreGraphicsImageFromFile(file_name);

        return osg_image;
    }

    virtual ReadResult readImage(const std::string& file_name, const osgDB::ReaderWriter::Options* the_options) const
    {
        std::string ext = osgDB::getLowerCaseFileExtension(file_name);
        if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

        std::string full_file_name = osgDB::findDataFile( file_name, the_options );
        if (full_file_name.empty()) return ReadResult::FILE_NOT_FOUND;

#if 1
        ReadResult read_result = readImageFile(full_file_name);
#else
        // Only here to help test istream backend. The file version is better because
        // the filenname.extension could potentially be used by ImageIO to hint what the format type is.
        std::ifstream istream(full_file_name.c_str(), std::ios::in | std::ios::binary);
        if(!istream) return ReadResult::FILE_NOT_HANDLED;
        ReadResult read_result = readImage(istream);
#endif

        if(read_result.validImage())
        {
            read_result.getImage()->setFileName(full_file_name);
        }
        return read_result;
    }


    WriteResult writeImageStream(const osg::Image& osg_image, std::ostream& fout, const osgDB::ReaderWriter::Options* the_options) const
    {
        WriteResult ret_val = WriteResult::ERROR_IN_WRITING_FILE;

        return WriteResult::FILE_SAVED;
    }

    virtual WriteResult writeImage(const osg::Image& osg_image, std::ostream& fout, const osgDB::ReaderWriter::Options* the_options) const
    {
        WriteResult write_result = writeImageStream(osg_image, fout, the_options);
        return write_result;
    }

    WriteResult writeImageFile(const osg::Image& osg_image, const std::string& full_file_name, const osgDB::ReaderWriter::Options* the_options) const
    {
        WriteResult ret_val = WriteResult::ERROR_IN_WRITING_FILE;

        return WriteResult::FILE_SAVED;
    }

    virtual WriteResult writeImage(const osg::Image& osg_image, const std::string& file_name, const osgDB::ReaderWriter::Options* the_options) const
    {
        std::string ext = osgDB::getFileExtension(file_name);
        if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

#if 1
        // FIXME: Something may need to provide a proper writable location for the files.
        std::string full_file_name;
        full_file_name = file_name;
        return writeImageFile(osg_image, full_file_name, the_options);
#else
        // Only here to help test ostream backend. The file version is better because
        // the filenname.extension could potentially be used by ImageIO to hint what the format type is.
        std::ofstream fout(file_name.c_str(), std::ios::out | std::ios::binary);
        if(!fout) return WriteResult::ERROR_IN_WRITING_FILE;
        return writeImage(osg_image, fout, the_options);
#endif
    }

};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(imageio, ReaderWriterImageIO)
