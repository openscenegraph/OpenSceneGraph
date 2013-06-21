// Copyright Eric Wing
// This plugin is the bridge to OS X's ImageIO framework
// which provides access to all of Apple's supported image types.
// This plugin plus the QTKit plugin obsoletes the old QuickTime plugin.
// This requires 10.4+. (The old QuickTime plugin will not support 64-bit.)


// Needs testing, especially in:
// 8-bits per pixel (256 color vs GL_ALPHA, and what about GL_LUMINANCE)?
// 16-bits per pixel (is GL_LUMINANCE_ALPHA a safe assumption?)
// Non-power-of-two textures (especially odd sizes)
// istream code path
// ostream code path
// write image, especially GL_LUMINANCE and GL_ALPHA paths and image formats other than PNG/JPEG

// Enhancements needed:
// Way to provide image type hint to ImageIO calls (via CFDictionary),
// probably especially important for istream which lacks extension information.
// Is there information we can use in the OSG options parameter?


#import "TargetConditionals.h"
#if (TARGET_OS_IPHONE)
    #import <UIKit/UIKit.h>
    #import <ImageIO/ImageIO.h>
    #import <CoreGraphics/CoreGraphics.h>
    #import <Foundation/Foundation.h>
    #import <MobileCoreServices/MobileCoreServices.h>
#else
    #include <ApplicationServices/ApplicationServices.h>
#endif
// For the vImage framework (part of the Accerlate framework)
#include <Accelerate/Accelerate.h>

// Used because CGDataProviderCreate became deprecated in 10.5
#include <AvailabilityMacros.h>


#include <osg/GL>
#include <osg/Notify>
#include <osg/Image>

#include <osgDB/Registry>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <sstream> // for istream
#include <iostream> // for ios::


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

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1050 // CGDataProviderCreateSequential was introduced in 10.5; CGDataProviderCreate is deprecated
off_t MyProviderSkipForwardBytesCallback(void* istream_userdata, off_t the_count)
{
    std::istream* the_istream = (std::istream*)istream_userdata;
    off_t start_position = the_istream->tellg();
    the_istream->seekg(the_count, std::ios::cur);
    off_t end_position = the_istream->tellg();
    return (end_position - start_position);
}
#else // CGDataProviderCreate was deprecated in 10.5
void MyProviderSkipBytesCallback(void* istream_userdata, size_t the_count)
{
    std::istream* the_istream = (std::istream*)istream_userdata;
    the_istream->seekg(the_count, std::ios::cur);
}
#endif

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

#if MAC_OS_X_VERSION_MAX_ALLOWED >= 1050 // CGDataProviderCreateSequential was introduced in 10.5; CGDataProviderCreate is deprecated
    CGDataProviderSequentialCallbacks provider_callbacks =
    {
        0,
        MyProviderGetBytesCallback,
        MyProviderSkipForwardBytesCallback,
        MyProviderRewindCallback,
        MyProviderReleaseInfoCallback
    };

    CGDataProviderRef data_provider = CGDataProviderCreateSequential(&fin, &provider_callbacks);


#else // CGDataProviderCreate was deprecated in 10.5

    CGDataProviderCallbacks provider_callbacks =
    {
        MyProviderGetBytesCallback,
        MyProviderSkipBytesCallback,
        MyProviderRewindCallback,
        MyProviderReleaseInfoCallback
    };

    CGDataProviderRef data_provider = CGDataProviderCreate(&fin, &provider_callbacks);
#endif
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


/* Create a CGImageSourceRef from a file. */
/* Remember to CFRelease the created image when done. */
CGImageRef CreateCGImageFromFile(const char* the_path)
{
    CFURLRef the_url = NULL;
    CGImageRef image_ref = NULL;
    CGImageSourceRef source_ref = NULL;
    CFStringRef cf_string = NULL;

    /* Create a CFString from a C string */
    cf_string = CFStringCreateWithCString(
        NULL,
        the_path,
        kCFStringEncodingUTF8
    );
    if(!cf_string)
    {
        OSG_WARN << "CreateCGImageFromFile :: could not create CCFSTring" << std::endl;
        return NULL;
    }

    /* Create a CFURL from a CFString */
    the_url = CFURLCreateWithFileSystemPath(
        NULL,
        cf_string,
        kCFURLPOSIXPathStyle,
        false
    );

    /* Don't need the CFString any more (error or not) */
    CFRelease(cf_string);

    if(!the_url)
    {
        OSG_WARN << "CreateCGImageFromFile :: could not create CFUrl" << std::endl;
        return NULL;
    }


    source_ref = CGImageSourceCreateWithURL(the_url, NULL);
    /* Don't need the URL any more (error or not) */
    CFRelease(the_url);

    if(!source_ref)
    {
        OSG_WARN << "CreateCGImageFromFile :: could not create ImageSource" << std::endl;
        return NULL;
    }

    // Get the first item in the image source (some image formats may
    // contain multiple items).
    image_ref = CGImageSourceCreateImageAtIndex(source_ref, 0, NULL);
    if (!image_ref) {
        OSG_WARN << "CreateCGImageFromFile :: could not get Image" << std::endl;
    }

    /* Don't need the SourceRef any more (error or not) */
    CFRelease(source_ref);

    return image_ref;
}

/* Once we have our image (CGImageRef), we need to get it into an osg::Image */
osg::Image* CreateOSGImageFromCGImage(CGImageRef image_ref)
{
    /* This code is adapted from Apple's Documentation found here:
     * http://developer.apple.com/documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/index.html
     * Listing 9-4Using a Quartz image as a texture source.
     * Unfortunately, this guide doesn't show what to do about
     * non-RGBA image formats so I'm making the rest up
     * (and it's probably all wrong).
     */

    size_t the_width = CGImageGetWidth(image_ref);
    size_t the_height = CGImageGetHeight(image_ref);
    CGRect the_rect = {{0, 0}, {the_width, the_height}};

    size_t bits_per_pixel = CGImageGetBitsPerPixel(image_ref);
    size_t bytes_per_row = CGImageGetBytesPerRow(image_ref);
//    size_t bits_per_component = CGImageGetBitsPerComponent(image_ref);
    size_t bits_per_component = 8;

    CGImageAlphaInfo alpha_info = CGImageGetAlphaInfo(image_ref);

    GLint internal_format;
    GLenum pixel_format;
    GLenum data_type;

    void* image_data = calloc(the_width * 4, the_height);

    CGColorSpaceRef color_space;
    CGBitmapInfo bitmap_info = CGImageGetBitmapInfo(image_ref);

    switch(bits_per_pixel)
    {
        // Drat, if 8-bit, how do you distinguish
        // between a 256 color GIF, a LUMINANCE map
        // or an ALPHA map?
        case 8:
        {
            // I probably did the formats all wrong for this case,
            // especially the ALPHA case.
            if(kCGImageAlphaNone == alpha_info)
            {
                /*
                 internal_format = GL_LUMINANCE;
                 pixel_format = GL_LUMINANCE;
                 */
                internal_format = GL_RGBA8;
                pixel_format = GL_BGRA_EXT;
                data_type = GL_UNSIGNED_INT_8_8_8_8_REV;

                bytes_per_row = the_width*4;
//                color_space = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
                color_space = CGColorSpaceCreateDeviceRGB();
//                bitmap_info = kCGImageAlphaPremultipliedFirst;
#if __BIG_ENDIAN__
                bitmap_info = kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Big; /* XRGB Big Endian */
#else
                bitmap_info = kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little; /* XRGB Little Endian */
#endif
            }
            else
            {
                internal_format = GL_ALPHA;
                pixel_format = GL_ALPHA;
                data_type = GL_UNSIGNED_BYTE;
                //            bytes_per_row = the_width;
//                color_space = CGColorSpaceCreateWithName(kCGColorSpaceGenericGray);
                color_space = CGColorSpaceCreateDeviceGray();
            }

            break;
        }
        case 24:
        {
            internal_format = GL_RGBA8;
            pixel_format = GL_BGRA_EXT;
            data_type = GL_UNSIGNED_INT_8_8_8_8_REV;
            bytes_per_row = the_width*4;
//            color_space = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
            color_space = CGColorSpaceCreateDeviceRGB();
//            bitmap_info = kCGImageAlphaNone;
#if __BIG_ENDIAN__
            bitmap_info = kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Big; /* XRGB Big Endian */
#else
            bitmap_info = kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little; /* XRGB Little Endian */
#endif
            break;
        }
        //
        // Tatsuhiro Nishioka
        // 16 bpp grayscale (8 bit white and 8 bit alpha) causes invalid argument combination
        // in CGBitmapContextCreate.
        // I guess it is safer to handle 16 bit grayscale image as 32-bit RGBA image.
        // It works at least on FlightGear
        //
        case 16:
        case 32:
        case 48:
        case 64:
        {

            internal_format = GL_RGBA8;
            pixel_format = GL_BGRA_EXT;
            data_type = GL_UNSIGNED_INT_8_8_8_8_REV;

            bytes_per_row = the_width*4;
//            color_space = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
            color_space = CGColorSpaceCreateDeviceRGB();
//            bitmap_info = kCGImageAlphaPremultipliedFirst;

#if __BIG_ENDIAN__
            bitmap_info = kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Big; /* XRGB Big Endian */
#else
            bitmap_info = kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little; /* XRGB Little Endian */
#endif
            break;
        }
        default:
        {
            // OSG_WARN << "Unknown file type in " << fileName.c_str() << " with " << origDepth << std::endl;
            return NULL;
            break;
        }

    }
    

    // Sets up a context to be drawn to with image_data as the area to be drawn to
    CGContextRef bitmap_context = CGBitmapContextCreate(
        image_data,
        the_width,
        the_height,
        bits_per_component,
        bytes_per_row,
        color_space,
        bitmap_info
    );
    
    CGContextTranslateCTM(bitmap_context, 0, the_height);
    CGContextScaleCTM(bitmap_context, 1.0, -1.0);
    // Draws the image into the context's image_data
    CGContextDrawImage(bitmap_context, the_rect, image_ref);

    CGContextRelease(bitmap_context);
    
    if (!image_data)
        return NULL;

    // alpha is premultiplied with rgba, undo it
    
    vImage_Buffer vb;
    vb.data = image_data;
    vb.height = the_height;
    vb.width = the_width;
    vb.rowBytes = the_width * 4;
    vImageUnpremultiplyData_RGBA8888(&vb, &vb, 0);
    
    // changing it to GL_UNSIGNED_BYTE seems working, but I'm not sure if this is a right way.
    //
    data_type = GL_UNSIGNED_BYTE;
    osg::Image* osg_image = new osg::Image;

    osg_image->setImage(
        the_width,
        the_height,
        1,
        internal_format,
        pixel_format,
        data_type,
        (unsigned char*)image_data,
        osg::Image::USE_MALLOC_FREE // Assumption: osg_image takes ownership of image_data and will free
    );

    return osg_image;



}
/**************************************************************
***** End Support functions for reading (stream and file) *****
**************************************************************/


/**************************************************************
***** Begin Support functions for writing (stream and file)****
**************************************************************/

/* Create a CGImageRef from osg::Image.
 * Code adapted from
 * http://developer.apple.com/samplecode/OpenGLScreenSnapshot/listing2.html
 */
CGImageRef CreateCGImageFromOSGData(const osg::Image& osg_image)
{
    size_t image_width = osg_image.s();
    size_t image_height = osg_image.t();
    /* From Apple's header for CGBitmapContextCreate()
     * Each row of the bitmap consists of `bytesPerRow' bytes, which must be at
     * least `(width * bitsPerComponent * number of components + 7)/8' bytes.
     */
    size_t target_bytes_per_row;

    CGColorSpaceRef color_space;
    CGBitmapInfo bitmap_info;
    /* From what I can figure out so far...
     * We need to create a CGContext connected to the data we want to save
     * and then call CGBitmapContextCreateImage() on that context to get
     * a CGImageRef.
     * However, OS X only allows 4-component image formats (e.g. RGBA) and not
     * just RGB for the RGB-based CGContext. So for a 24-bit image coming in,
     * we need to expand the data to 32-bit.
     * The easiest and fastest way to do that is through the vImage framework
     * which is part of the Accelerate framework.
     * Also, the osg::Image data coming in is inverted from what we want, so
     * we need to invert the image too. Since the osg::Image is const,
     * we don't want to touch the data, so again we turn to the vImage framework
     * and invert the data.
     */
    vImage_Buffer vimage_buffer_in =
    {
        (void*)osg_image.data(), // need to override const, but we don't modify the data so it's safe
        image_height,
        image_width,
        osg_image.getRowSizeInBytes()
    };

    void* out_image_data;
    vImage_Buffer vimage_buffer_out =
    {
        NULL, // will fill-in in switch
        image_height,
        image_width,
        0 // will fill-in in switch
    };
    vImage_Error vimage_error_flag;

    // FIXME: Do I want to use format, type, or internalFormat?
    switch(osg_image.getPixelFormat())
    {
        case GL_LUMINANCE:
        {
            bitmap_info = kCGImageAlphaNone;
            target_bytes_per_row = (image_width * 8 + 7)/8;
            //color_space = CGColorSpaceCreateWithName(kCGColorSpaceGenericGray);
            color_space = CGColorSpaceCreateDeviceGray();
            if(NULL == color_space)
            {
                return NULL;
            }

            //    out_image_data = calloc(target_bytes_per_row, image_height);
            out_image_data = malloc(target_bytes_per_row * image_height);
            if(NULL == out_image_data)
            {
                OSG_WARN << "In CreateCGImageFromOSGData, malloc failed" << std::endl;
                CGColorSpaceRelease(color_space);
                return NULL;
            }

            vimage_buffer_out.data = out_image_data;
            vimage_buffer_out.rowBytes = target_bytes_per_row;

            // Now invert the image
            vimage_error_flag = vImageVerticalReflect_Planar8(
                &vimage_buffer_in, // since the osg_image is const...
                &vimage_buffer_out, // don't reuse the buffer
                kvImageNoFlags
            );
            if(vimage_error_flag != kvImageNoError)
            {
                OSG_WARN << "In CreateCGImageFromOSGData for GL_LUMINANCE, vImageVerticalReflect_Planar8 failed with vImage Error Code: " << vimage_error_flag << std::endl;
                free(out_image_data);
                CGColorSpaceRelease(color_space);
                return NULL;
            }


            break;
        }
        case GL_ALPHA:
        {
            bitmap_info = kCGImageAlphaOnly;
            target_bytes_per_row = (image_width * 8 + 7)/8;
            // According to:
            // http://developer.apple.com/qa/qa2001/qa1037.html
            // colorSpace=NULL is for alpha only
            color_space = NULL;

            //    out_image_data = calloc(target_bytes_per_row, image_height);
            out_image_data = malloc(target_bytes_per_row * image_height);
            if(NULL == out_image_data)
            {
                OSG_WARN << "In CreateCGImageFromOSGData, malloc failed" << std::endl;
                return NULL;
            }

            vimage_buffer_out.data = out_image_data;
            vimage_buffer_out.rowBytes = target_bytes_per_row;

            // Now invert the image
            vimage_error_flag = vImageVerticalReflect_Planar8(
                &vimage_buffer_in, // since the osg_image is const...
                &vimage_buffer_out, // don't reuse the buffer
                kvImageNoFlags
            );
            if(vimage_error_flag != kvImageNoError)
            {
                OSG_WARN << "In CreateCGImageFromOSGData for GL_ALPHA, vImageVerticalReflect_Planar8 failed with vImage Error Code: " << vimage_error_flag << std::endl;
                free(out_image_data);
                return NULL;
            }


            break;
        }
/*
        case GL_LUMINANCE_ALPHA:
        {
            // I don't know if we can support this.
            // The qa1037 doesn't show both gray+alpha.
            break;
        }
*/
        case GL_RGB:
        {
            bitmap_info = kCGImageAlphaNoneSkipFirst;
            target_bytes_per_row = (image_width * 8 * 4 + 7)/8;
            //color_space = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
            color_space = CGColorSpaceCreateDeviceRGB();
             if(NULL == color_space)
            {
                OSG_WARN << "In CreateCGImageFromOSGData, CGColorSpaceCreateWithName failed" << std::endl;
                return NULL;
            }

            //    out_image_data = calloc(target_bytes_per_row, image_height);
            out_image_data = malloc(target_bytes_per_row * image_height);
            if(NULL == out_image_data)
            {
                OSG_WARN << "In CreateCGImageFromOSGData, malloc failed" << std::endl;
                CGColorSpaceRelease(color_space);
                return NULL;
            }

            // Use vImage to get an RGB buffer into ARGB.
            vimage_buffer_out.data = out_image_data;
            vimage_buffer_out.rowBytes = target_bytes_per_row;
            vimage_error_flag = vImageConvert_RGB888toARGB8888(
                &vimage_buffer_in,
                NULL, // we don't have a buffer containing alpha values
                255, // The alpha value we want given to all pixels since we don't have a buffer
                &vimage_buffer_out,
                0, // premultiply?
                kvImageNoFlags // Only responds to kvImageDoNotTile, but I think we want tiling/threading
            );
            if(vimage_error_flag != kvImageNoError)
            {
                OSG_WARN << "In CreateCGImageFromOSGData, vImageConvert_RGB888toARGB8888 failed with vImage Error Code: " << vimage_error_flag << std::endl;
                free(out_image_data);
                CGColorSpaceRelease(color_space);
                return NULL;
            }
            // Now invert the image
            vimage_error_flag = vImageVerticalReflect_ARGB8888(
                &vimage_buffer_out,
                &vimage_buffer_out, // reuse the same buffer
                kvImageNoFlags
            );
            if(vimage_error_flag != kvImageNoError)
            {
                OSG_WARN << "In CreateCGImageFromOSGData, vImageAffineWarp_ARGB8888 failed with vImage Error Code: " << vimage_error_flag << std::endl;
                free(out_image_data);
                CGColorSpaceRelease(color_space);
                return NULL;
            }

            break;
        }
        case GL_RGBA:
        {
            bitmap_info = kCGImageAlphaPremultipliedLast;
            target_bytes_per_row = osg_image.getRowSizeInBytes();
            //color_space = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
            color_space = CGColorSpaceCreateDeviceRGB();
            if(NULL == color_space)
            {
                OSG_WARN << "In CreateCGImageFromOSGData, CGColorSpaceCreateWithName failed" << std::endl;
                return NULL;
            }
            //    out_image_data = calloc(target_bytes_per_row, image_height);
            out_image_data = malloc(target_bytes_per_row * image_height);
            if(NULL == out_image_data)
            {
                OSG_WARN << "In CreateCGImageFromOSGData, malloc failed" << std::endl;
                CGColorSpaceRelease(color_space);
                return NULL;
            }
            vimage_buffer_out.data = out_image_data;
            vimage_buffer_out.rowBytes = target_bytes_per_row;
            // Invert the image
            vimage_error_flag = vImageVerticalReflect_ARGB8888(
                &vimage_buffer_in, // since the osg_image is const...
                &vimage_buffer_out, // don't reuse the buffer
                kvImageNoFlags
            );
            if(vimage_error_flag != kvImageNoError)
            {
                OSG_WARN << "In CreateCGImageFromOSGData, vImageAffineWarp_ARGB8888 failed with vImage Error Code: " << vimage_error_flag << std::endl;
                free(out_image_data);
                CGColorSpaceRelease(color_space);
                return NULL;
            }
            break;
        }
        case GL_BGRA:
        {
            if(GL_UNSIGNED_INT_8_8_8_8_REV == osg_image.getDataType())
            {
#if __BIG_ENDIAN__
                bitmap_info = kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Big; /* XRGB Big Endian */
#else
                bitmap_info = kCGImageAlphaPremultipliedFirst | kCGBitmapByteOrder32Little; /* XRGB Little Endian */
#endif
            }
            else
            {
                // FIXME: Don't know how to handle this case
                bitmap_info = kCGImageAlphaPremultipliedLast;
            }

            target_bytes_per_row = osg_image.getRowSizeInBytes();
            //color_space = CGColorSpaceCreateWithName(kCGColorSpaceGenericRGB);
            color_space = CGColorSpaceCreateDeviceRGB();
            if(NULL == color_space)
            {
                OSG_WARN << "In CreateCGImageFromOSGData, CGColorSpaceCreateWithName failed" << std::endl;
                return NULL;
            }
            //    out_image_data = calloc(target_bytes_per_row, image_height);
            out_image_data = malloc(target_bytes_per_row * image_height);
            if(NULL == out_image_data)
            {
                OSG_WARN << "In CreateCGImageFromOSGData, malloc failed" << std::endl;
                CGColorSpaceRelease(color_space);
                return NULL;
            }
            vimage_buffer_out.data = out_image_data;
            vimage_buffer_out.rowBytes = target_bytes_per_row;
            // Invert the image
            vimage_error_flag = vImageVerticalReflect_ARGB8888(
                                                               &vimage_buffer_in, // since the osg_image is const...
                                                               &vimage_buffer_out, // don't reuse the buffer
                                                               kvImageNoFlags
                                                               );
            if(vimage_error_flag != kvImageNoError)
            {
                OSG_WARN << "In CreateCGImageFromOSGData, vImageAffineWarp_ARGB8888 failed with vImage Error Code: " << vimage_error_flag << std::endl;
                free(out_image_data);
                CGColorSpaceRelease(color_space);
                return NULL;
            }
            break;
        }
        // FIXME: Handle other cases.
        // Use vImagePermuteChannels_ARGB8888 to swizzle bytes
        default:
        {
            OSG_WARN << "In CreateCGImageFromOSGData: Sorry support for this format is not implemented." << std::endl;
            return NULL;
            break;
        }
    }

    CGContextRef bitmap_context = CGBitmapContextCreate(
        vimage_buffer_out.data,
        vimage_buffer_out.width,
        vimage_buffer_out.height,
        8,
        vimage_buffer_out.rowBytes,
        color_space,
        bitmap_info
    );
    /* Done with color space */
    CGColorSpaceRelease(color_space);

    if(NULL == bitmap_context)
    {
        free(out_image_data);
        return NULL;
    }


    /* Make an image out of our bitmap; does a cheap vm_copy of the bitmap */
    CGImageRef image_ref = CGBitmapContextCreateImage(bitmap_context);

    /* Done with data */
    free(out_image_data);

    /* Done with bitmap_context */
    CGContextRelease(bitmap_context);

    return image_ref;
}


/* Create a CGImageDestinationRef from a file. */
/* Remember to CFRelease when done. */
CGImageDestinationRef CreateCGImageDestinationFromFile(const char* the_path,  const osgDB::ReaderWriter::Options* the_options)
{
    CFURLRef the_url = NULL;
    CFStringRef cf_string = NULL;
    CFStringRef uti_type = NULL;
    CGImageDestinationRef dest_ref = NULL;
    bool found_png_option = false;
    bool found_jpeg_option = false;
    float compression_quality = 1.0f;

    /* Create a CFString from a C string */
    cf_string = CFStringCreateWithCString(
        NULL,
        the_path,
        kCFStringEncodingUTF8
    );
    if(!cf_string)
    {
        return NULL;
    }

    /* Create a CFURL from a CFString */
    the_url = CFURLCreateWithFileSystemPath(
        NULL,
        cf_string,
        kCFURLPOSIXPathStyle,
        false
    );

    /* Don't need the CFString any more (error or not) */
    CFRelease(cf_string);

    if(!the_url)
    {
        return NULL;
    }

    if(the_options)
    {
        std::istringstream iss(the_options->getOptionString());
        std::string opt;
        while (iss >> opt)
        {
            // Not handled: The user could do something stupid and specify both PNG and JPEG options.

            if(opt=="PNG_COMPRESSION")
            {
                found_png_option = true;
                // I don't see an option to set PNG compression levels in the API so this info is unused.
                int level;
                iss >> level;

            }
            else if(opt=="JPEG_QUALITY")
            {
                found_jpeg_option = true;
                // Chances are that people are specifying values in libjpeg ranges and not ImageIO ranges.
                // ImageIO is normalized between 0.0 to 1.0 where 1.0 is lossless and 0 is max compression.
                // I am uncertain what libjpeg's range is. I'm guessing 0-100.
                int quality;
                iss >> quality;
                compression_quality = (float)quality/100.0f;
            }
        }
    }


    CFStringRef path_extension = CFURLCopyPathExtension(the_url);
    if(NULL == path_extension)
    {
        if(found_jpeg_option)
        {
            uti_type = UTTypeCreatePreferredIdentifierForTag(
                 kUTTagClassFilenameExtension,
                 CFSTR("jpg"),
                 kUTTypeImage // "public.image"
            );
        }
        else
        {
            uti_type = UTTypeCreatePreferredIdentifierForTag(
                 kUTTagClassFilenameExtension,
                 CFSTR("png"),
                 kUTTypeImage // "public.image"
            );
        }
    }
    else
    {
        uti_type = UTTypeCreatePreferredIdentifierForTag(
            kUTTagClassFilenameExtension,
            path_extension,
            kUTTypeImage // "public.image"
        );
        CFRelease(path_extension);
    }


    dest_ref =  CGImageDestinationCreateWithURL(
        the_url,
        uti_type,
        1, // image file will contain only one image
        NULL
    );


    CFRelease(uti_type);
    CFRelease(the_url);


    // Not handled: The user could do something stupid and specify both PNG and JPEG options.
    if(found_jpeg_option)
    {
        // Do a bunch of work to setup a CFDictionary containing the jpeg compression properties.
        CFStringRef the_keys[1];
        CFNumberRef the_values[1];
        CFDictionaryRef the_dict;

        the_keys[0] = kCGImageDestinationLossyCompressionQuality;
        the_values[0] = CFNumberCreate(
            NULL,
            kCFNumberFloat32Type,
            &compression_quality
        );

        the_dict = CFDictionaryCreate(NULL, (const void**)&the_keys, (const void**)&the_values, 1, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        CFRelease(the_values[0]);

        // Now that we have the dict, actually set the property.
        CGImageDestinationSetProperties(dest_ref, the_dict);

        CFRelease(the_dict);
    }

    return dest_ref;
}


/* Create a CGImageDestinationRef from a file. */
/* Remember to CFRelease when done. */
CGImageDestinationRef CreateCGImageDestinationFromDataStream(std::ostream& fout,  const osgDB::ReaderWriter::Options* the_options)
{
    CFStringRef uti_type = NULL;
    CGImageDestinationRef dest_ref = NULL;
    bool found_png_option = false;
    bool found_jpeg_option = false;
    float compression_quality = 1.0f;

    CGDataConsumerCallbacks consumer_callbacks =
    {
        MyConsumerPutBytesCallback,
        MyConsumerReleaseInfoCallback
    };

    CGDataConsumerRef data_consumer = CGDataConsumerCreate(&fout, &consumer_callbacks);

    if(the_options)
    {
        std::istringstream iss(the_options->getOptionString());
        std::string opt;
        while (iss >> opt)
        {
            // Not handled: The user could do something stupid and specify both PNG and JPEG options.

            if(opt=="PNG_COMPRESSION")
            {
                found_png_option = true;
                // I don't see an option to set PNG compression levels in the API so this info is unused.
                int level;
                iss >> level;

            }
            else if(opt=="JPEG_QUALITY")
            {
                found_jpeg_option = true;
                // Chances are that people are specifying values in libjpeg ranges and not ImageIO ranges.
                // ImageIO is normalized between 0.0 to 1.0 where 1.0 is lossless and 0 is max compression.
                // I am uncertain what libjpeg's range is. I'm guessing 0-100.
                int quality;
                iss >> quality;
                compression_quality = (float)quality/100.0f;
            }
        }
    }


    if(found_jpeg_option)
    {
        uti_type = UTTypeCreatePreferredIdentifierForTag(
            kUTTagClassFilenameExtension,
            CFSTR("jpg"),
            kUTTypeImage // "public.image"
        );
    }
    else // default to png
    {
        uti_type = UTTypeCreatePreferredIdentifierForTag(
            kUTTagClassFilenameExtension,
            CFSTR("png"),
            kUTTypeImage // "public.image"
        );
    }


    // If we had a way of hinting at what the data type is, we could
    // pass this hint in the second parameter.
    dest_ref = CGImageDestinationCreateWithDataConsumer(
        data_consumer,
        uti_type,
        1, // image file will contain only one image
        NULL
    );

    CGDataConsumerRelease(data_consumer);
    CFRelease(uti_type);


    // Not handled: The user could do something stupid and specify both PNG and JPEG options.
    if(found_jpeg_option)
    {
        // Do a bunch of work to setup a CFDictionary containing the jpeg compression properties.
        CFStringRef the_keys[1];
        CFNumberRef the_values[1];
        CFDictionaryRef the_dict;

        the_keys[0] = kCGImageDestinationLossyCompressionQuality;
        the_values[0] = CFNumberCreate(
                                       NULL,
                                       kCFNumberFloat32Type,
                                       &compression_quality
                                       );

        the_dict = CFDictionaryCreate(NULL, (const void**)&the_keys, (const void**)&the_values, 1, &kCFCopyStringDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks);
        CFRelease(the_values[0]);

        // Now that we have the dict, actually set the property.
        CGImageDestinationSetProperties(dest_ref, the_dict);

        CFRelease(the_dict);
    }

    return dest_ref;
}
/**************************************************************
***** End Support functions for writing (stream and file) *****
**************************************************************/



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
        OSG_INFO << "imageio readImageFile: " << file_name << std::endl;

        // Call ImageIO to load the image.
        CGImageRef cg_image_ref = CreateCGImageFromFile(file_name.c_str());
        if (NULL == cg_image_ref) return ReadResult::FILE_NOT_FOUND;

        // Create an osg::Image from the CGImageRef.
        osg::Image* osg_image = CreateOSGImageFromCGImage(cg_image_ref);

        CFRelease(cg_image_ref);
        if (!osg_image)
            return ReadResult::INSUFFICIENT_MEMORY_TO_LOAD;
        
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
        osgDB::ifstream istream(full_file_name.c_str(), std::ios::in | std::ios::binary);
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
        if (!osg_image.isDataContiguous())
        {
            return WriteResult::FILE_NOT_HANDLED;
        }

        WriteResult ret_val = WriteResult::ERROR_IN_WRITING_FILE;

        CGImageDestinationRef cg_dest_ref = CreateCGImageDestinationFromDataStream(fout, the_options);
        if (NULL == cg_dest_ref) return WriteResult::ERROR_IN_WRITING_FILE;

        CGImageRef cg_image_ref = CreateCGImageFromOSGData(osg_image);
        if(NULL == cg_image_ref)
        {
            CFRelease(cg_dest_ref);
            return WriteResult::ERROR_IN_WRITING_FILE;
        }

        CGImageDestinationAddImage(cg_dest_ref, cg_image_ref, NULL);
        if(CGImageDestinationFinalize(cg_dest_ref))
        {
            ret_val = WriteResult::FILE_SAVED;
        }
        else
        {
            ret_val = WriteResult::ERROR_IN_WRITING_FILE;
        }

        CFRelease(cg_image_ref);
        CFRelease(cg_dest_ref);

        return WriteResult::FILE_SAVED;
    }

    virtual WriteResult writeImage(const osg::Image& osg_image, std::ostream& fout, const osgDB::ReaderWriter::Options* the_options) const
    {
        WriteResult write_result = writeImageStream(osg_image, fout, the_options);
        return write_result;
    }

    WriteResult writeImageFile(const osg::Image& osg_image, const std::string& full_file_name, const osgDB::ReaderWriter::Options* the_options) const
    {
        if (!osg_image.isDataContiguous())
        {
            return WriteResult::FILE_NOT_HANDLED;
        }

        WriteResult ret_val = WriteResult::ERROR_IN_WRITING_FILE;

        // Call ImageIO to load the image.
        CGImageDestinationRef cg_dest_ref = CreateCGImageDestinationFromFile(full_file_name.c_str(), the_options);
        if (NULL == cg_dest_ref) return WriteResult::ERROR_IN_WRITING_FILE;

        CGImageRef cg_image_ref = CreateCGImageFromOSGData(osg_image);
        if(NULL == cg_image_ref)
        {
            CFRelease(cg_dest_ref);
            return WriteResult::ERROR_IN_WRITING_FILE;
        }

        CGImageDestinationAddImage(cg_dest_ref, cg_image_ref, NULL);
        if(CGImageDestinationFinalize(cg_dest_ref))
        {
            ret_val = WriteResult::FILE_SAVED;
        }
        else
        {
            ret_val = WriteResult::ERROR_IN_WRITING_FILE;
        }

        CFRelease(cg_image_ref);
        CFRelease(cg_dest_ref);

        return WriteResult::FILE_SAVED;
    }

    virtual WriteResult writeImage(const osg::Image& osg_image, const std::string& file_name, const osgDB::ReaderWriter::Options* the_options) const
    {
        std::string ext = osgDB::getFileExtension(file_name);
        if (!acceptsExtension(ext)) return WriteResult::FILE_NOT_HANDLED;

        if (!osg_image.isDataContiguous())
        {
            return WriteResult::FILE_NOT_HANDLED;
        }

        WriteResult ret_val = WriteResult::ERROR_IN_WRITING_FILE;
#if 1
        // FIXME: Something may need to provide a proper writable location for the files.
        std::string full_file_name;
        full_file_name = file_name;
        return writeImageFile(osg_image, full_file_name, the_options);
#else
        // Only here to help test ostream backend. The file version is better because
        // the filenname.extension could potentially be used by ImageIO to hint what the format type is.
        osgDB::ofstream fout(file_name.c_str(), std::ios::out | std::ios::binary);
        if(!fout) return WriteResult::ERROR_IN_WRITING_FILE;
        return writeImage(osg_image, fout, the_options);
#endif
    }

    virtual ReadResult readObject(std::istream& fin,const osgDB::ReaderWriter::Options* options =NULL) const
    {
        return readImage(fin, options);
    }

    virtual ReadResult readObject(const std::string& file, const osgDB::ReaderWriter::Options* options =NULL) const
    {
        return readImage(file, options);
    }

};

// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(imageio, ReaderWriterImageIO)
