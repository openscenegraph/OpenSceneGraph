#include "EXIF_Orientation.h"

#include <stdio.h>
#include <string.h>

#include <osg/Notify>
#include <osg/Endian>

#define EXIF_IDENT_STRING  "Exif\000\000"

static unsigned short de_get16(void *ptr, bool byteSwap)
{
       unsigned short val;

       memcpy(&val, ptr, sizeof(val));
       if (byteSwap) osg::swapBytes(val);

       return val;
}

static unsigned int de_get32(void *ptr, bool byteSwap)
{
       unsigned int val;

       memcpy(&val, ptr, sizeof(val));
       if (byteSwap) osg::swapBytes(val);

       return val;
}

int EXIF_Orientation (j_decompress_ptr cinfo)
{
    OSG_INFO<<"get_orientation()"<<std::endl;
    jpeg_saved_marker_ptr exif_marker;  /* Location of the Exif APP1 marker */
    jpeg_saved_marker_ptr cmarker;      /* Location to check for Exif APP1 marker */

    /* check for Exif marker (also called the APP1 marker) */
    exif_marker = NULL;
    cmarker = cinfo->marker_list;
    while (cmarker)
    {
        if (cmarker->marker == EXIF_JPEG_MARKER)
        {
            /* The Exif APP1 marker should contain a unique
                identification string ("Exif\0\0"). Check for it. */
            if (!memcmp (cmarker->data, EXIF_IDENT_STRING, 6))
            {
                exif_marker = cmarker;
            }
        }
        cmarker = cmarker->next;
    }

    if (exif_marker==NULL)
    {
        OSG_INFO<<"exif_marker not found "<<std::endl;
        return 0;
    }

    OSG_INFO<<"exif_marker found "<<exif_marker<<std::endl;

    /* Do we have enough data? */
    if (exif_marker->data_length < 32)
    {
        OSG_INFO<<"exif_marker too short : "<<exif_marker->data_length<<std::endl;
        return 0;
    }


    osg::Endian tiffHeaderEndian = osg::LittleEndian;

    const char leth[]  = {0x49, 0x49, 0x2a, 0x00};  // Little endian TIFF header
    const char beth[]  = {0x4d, 0x4d, 0x00, 0x2a};  // Big endian TIFF header

    /* Just skip data until TIFF header - it should be within 16 bytes from marker start.
        Normal structure relative to APP1 marker -
            0x0000: APP1 marker entry = 2 bytes
            0x0002: APP1 length entry = 2 bytes
            0x0004: Exif Identifier entry = 6 bytes
            0x000A: Start of TIFF header (Byte order entry) - 4 bytes
                    - This is what we look for, to determine endianness.
            0x000E: 0th IFD offset pointer - 4 bytes

            exif_marker->data points to the first data after the APP1 marker
            and length entries, which is the exif identification string.
            The TIFF header should thus normally be found at i=6, below,
            and the pointer to IFD0 will be at 6+4 = 10.
    */


    /* Check for TIFF header and catch endianness */
    unsigned int i = 0;
    for(i=0; i < 16; ++i)
    {
        /* Little endian TIFF header */
        if (memcmp (&exif_marker->data[i], leth, 4) == 0)
        {
            tiffHeaderEndian = osg::LittleEndian;
            break;
        }
        /* Big endian TIFF header */
        else if (memcmp (&exif_marker->data[i], beth, 4) == 0)
        {
            tiffHeaderEndian = osg::BigEndian;
            break;
        }
    }

    /* So did we find a TIFF header or did we just hit end of buffer? */
    if (i >= 16)
    {
        OSG_INFO<<"Could not find TIFF header"<<std::endl;
        return 0;
    }

    OSG_INFO<<"Found TIFF header = "<<i<<" endian = "<<((tiffHeaderEndian==osg::BigEndian)?"BigEndian":"LittleEndian")<< std::endl;

    bool swapBytes = osg::getCpuByteOrder()!=tiffHeaderEndian;
    OSG_INFO<<"swapBytes = "<<swapBytes<< std::endl;

    /* Read out the offset pointer to IFD0 */
    unsigned int offset  = de_get32(&exif_marker->data[i] + 4, swapBytes);
    i += offset;

    OSG_INFO<<"offset = "<<offset<<std::endl;

    /* Check that we still are within the buffer and can read the tag count */
    if ((i + 2) > exif_marker->data_length)
        return 0;

    /* Find out how many tags we have in IFD0. As per the TIFF spec, the first
    two bytes of the IFD contain a count of the number of tags. */
    unsigned int tags    = de_get16(&exif_marker->data[i], swapBytes);
    i += 2;

    OSG_INFO<<"tags = "<<tags<<std::endl;

    /* Check that we still have enough data for all tags to check. The tags
    are listed in consecutive 12-byte blocks. The tag ID, type, size, and
    a pointer to the actual value, are packed into these 12 byte entries. */
    if ((i + tags * 12) > exif_marker->data_length)
    {
        OSG_INFO<<"Not enough length for requied tags"<<std::endl;
        return 0;
    }

    /* Endian the orientation tag ID, to locate it more easily */
    unsigned int orient_tag_id = 0x112;

    /* Check through IFD0 for tags of interest */
    while (tags--)
    {
        unsigned int tag = de_get16(&exif_marker->data[i], swapBytes);
        unsigned int type   = de_get16(&exif_marker->data[i + 2], swapBytes);
        unsigned int count  = de_get32(&exif_marker->data[i + 4], swapBytes);

        OSG_INFO<<"  tag=0x"<<std::hex<<tag<<std::dec<<", type="<<type<<", count="<<count<<std::endl;

        /* Is this the orientation tag? */
        if (tag==orient_tag_id)
        {
            /* Check that type and count fields are OK. The orientation field
                will consist of a single (count=1) 2-byte integer (type=3). */
            if (type != 3 || count != 1) return 0;


            /* Return the orientation value. Within the 12-byte block, the
                pointer to the actual data is at offset 8. */
            unsigned int ret =  de_get16(&exif_marker->data[i + 8], swapBytes);

            OSG_INFO<<"Found orientationTag, ret = "<<ret<<std::endl;
            return ret <= 8 ? ret : 0;
        }
        /* move the pointer to the next 12-byte tag field. */
        i = i + 12;
    }

    OSG_INFO<<"Could not find EXIF Orientation tag"<<std::endl;

    return 0; /* No EXIF Orientation tag found */
}

