/* OpenSceneGraph example, osgvolume.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/Texture3D>
#include <osg/Texture1D>
#include <osg/ImageSequence>
#include <osg/TexGen>
#include <osg/Geode>
#include <osg/Billboard>
#include <osg/PositionAttitudeTransform>
#include <osg/ClipNode>
#include <osg/AlphaFunc>
#include <osg/TexGenNode>
#include <osg/TexEnv>
#include <osg/TexEnvCombine>
#include <osg/Material>
#include <osg/PrimitiveSet>
#include <osg/Endian>
#include <osg/BlendFunc>
#include <osg/BlendEquation>
#include <osg/TransferFunction>
#include <osg/MatrixTransform>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <osgGA/EventVisitor>
#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/KeySwitchMatrixManipulator>

#include <osgUtil/CullVisitor>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osg/io_utils>

#include <algorithm>
#include <iostream>

#include <osg/ImageUtils>
#include <osgVolume/Volume>
#include <osgVolume/VolumeTile>
#include <osgVolume/RayTracedTechnique>
#include <osgVolume/FixedFunctionTechnique>

typedef std::vector< osg::ref_ptr<osg::Image> > ImageList;

enum ShadingModel
{
    Standard,
    Light,
    Isosurface,
    MaximumIntensityProjection
};

struct PassThroughTransformFunction
{
    unsigned char operator() (unsigned char c) const { return c; }
};


struct ProcessRow
{
    virtual ~ProcessRow() {}

    virtual void operator() (unsigned int num,
                    GLenum source_pixelFormat, unsigned char* source, 
                    GLenum dest_pixelFormat, unsigned char* dest) const 
    {
        switch(source_pixelFormat)
        {
        case(GL_LUMINANCE):
        case(GL_ALPHA):
            switch(dest_pixelFormat)
            {
            case(GL_LUMINANCE): 
            case(GL_ALPHA): A_to_A(num, source, dest); break;
            case(GL_LUMINANCE_ALPHA): A_to_LA(num, source, dest); break;
            case(GL_RGB): A_to_RGB(num, source, dest); break;
            case(GL_RGBA): A_to_RGBA(num, source, dest); break;
            }
            break;
        case(GL_LUMINANCE_ALPHA):
            switch(dest_pixelFormat)
            {
            case(GL_LUMINANCE): 
            case(GL_ALPHA): LA_to_A(num, source, dest); break;
            case(GL_LUMINANCE_ALPHA): LA_to_LA(num, source, dest); break;
            case(GL_RGB): LA_to_RGB(num, source, dest); break;
            case(GL_RGBA): LA_to_RGBA(num, source, dest); break;
            }
            break;
        case(GL_RGB):
            switch(dest_pixelFormat)
            {
            case(GL_LUMINANCE): 
            case(GL_ALPHA): RGB_to_A(num, source, dest); break;
            case(GL_LUMINANCE_ALPHA): RGB_to_LA(num, source, dest); break;
            case(GL_RGB): RGB_to_RGB(num, source, dest); break;
            case(GL_RGBA): RGB_to_RGBA(num, source, dest); break;
            }
            break;
        case(GL_RGBA):
            switch(dest_pixelFormat)
            {
            case(GL_LUMINANCE): 
            case(GL_ALPHA): RGBA_to_A(num, source, dest); break;
            case(GL_LUMINANCE_ALPHA): RGBA_to_LA(num, source, dest); break;
            case(GL_RGB): RGBA_to_RGB(num, source, dest); break;
            case(GL_RGBA): RGBA_to_RGBA(num, source, dest); break;
            }
            break;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    // alpha sources..    
    virtual void A_to_A(unsigned int num, unsigned char* source, unsigned char* dest) const
    {
        for(unsigned int i=0;i<num;++i)
        {
            *dest++ = *source++;
        }
    }

    virtual void A_to_LA(unsigned int num, unsigned char* source, unsigned char* dest) const
    {
        for(unsigned int i=0;i<num;++i)
        {
            *dest++ = *source;
            *dest++ = *source++;
        }
    }
                    
    virtual void A_to_RGB(unsigned int num, unsigned char* source, unsigned char* dest) const
    {
        for(unsigned int i=0;i<num;++i)
        {
            *dest++ = *source;
            *dest++ = *source;
            *dest++ = *source++;
        }
    }

    virtual void A_to_RGBA(unsigned int num, unsigned char* source, unsigned char* dest) const
    {
        for(unsigned int i=0;i<num;++i)
        {
            *dest++ = *source;
            *dest++ = *source;
            *dest++ = *source;
            *dest++ = *source++;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    // alpha luminance sources..    
    virtual void LA_to_A(unsigned int num, unsigned char* source, unsigned char* dest) const
    {
        for(unsigned int i=0;i<num;++i)
        {
            ++source;
            *dest++ = *source++;
        }
    }

    virtual void LA_to_LA(unsigned int num, unsigned char* source, unsigned char* dest) const
    {
        for(unsigned int i=0;i<num;++i)
        {
            *dest++ = *source++;
            *dest++ = *source++;
        }
    }
                    
    virtual void LA_to_RGB(unsigned int num, unsigned char* source, unsigned char* dest) const
    {
        for(unsigned int i=0;i<num;++i)
        {
            *dest++ = *source;
            *dest++ = *source;
            *dest++ = *source;
            source+=2;
        }
    }

    virtual void LA_to_RGBA(unsigned int num, unsigned char* source, unsigned char* dest) const
    {
        for(unsigned int i=0;i<num;++i)
        {
            *dest++ = *source;
            *dest++ = *source;
            *dest++ = *source++;
            *dest++ = *source++;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    // RGB sources..    
    virtual void RGB_to_A(unsigned int num, unsigned char* source, unsigned char* dest) const
    {
        for(unsigned int i=0;i<num;++i)
        {
            unsigned char val = *source;
            *dest++ = val;
            source += 3;
        }
    }

    virtual void RGB_to_LA(unsigned int num, unsigned char* source, unsigned char* dest) const
    {
        for(unsigned int i=0;i<num;++i)
        {
            unsigned char val = *source;
            *dest++ = val;
            *dest++ = val;
            source += 3;
        }
    }
                    
    virtual void RGB_to_RGB(unsigned int num, unsigned char* source, unsigned char* dest) const
    {
        for(unsigned int i=0;i<num;++i)
        {
            *dest++ = *source++;
            *dest++ = *source++;
            *dest++ = *source++;
        }
    }

    virtual void RGB_to_RGBA(unsigned int num, unsigned char* source, unsigned char* dest) const
    {
        for(unsigned int i=0;i<num;++i)
        {
            unsigned char val = *source;
            *dest++ = *source++;
            *dest++ = *source++;
            *dest++ = *source++;
            *dest++ = val;
        }
    }

    ///////////////////////////////////////////////////////////////////////////////
    // RGBA sources..    
    virtual void RGBA_to_A(unsigned int num, unsigned char* source, unsigned char* dest) const
    {
        for(unsigned int i=0;i<num;++i)
        {
            source += 3;
            *dest++ = *source++;
        }
    }

    virtual void RGBA_to_LA(unsigned int num, unsigned char* source, unsigned char* dest) const
    {
        for(unsigned int i=0;i<num;++i)
        {
            unsigned char val = *source;
            source += 3;
            *dest++ = val;
            *dest++ = *source++;
        }
    }
                    
    virtual void RGBA_to_RGB(unsigned int num, unsigned char* source, unsigned char* dest) const
    {
        for(unsigned int i=0;i<num;++i)
        {
            *dest++ = *source++;
            *dest++ = *source++;
            *dest++ = *source++;
            ++source;
        }
    }

    virtual void RGBA_to_RGBA(unsigned int num, unsigned char* source, unsigned char* dest) const
    {
        for(unsigned int i=0;i<num;++i)
        {
            *dest++ = *source++;
            *dest++ = *source++;
            *dest++ = *source++;
            *dest++ = *source++;
        }
    }
};


void clampToNearestValidPowerOfTwo(int& sizeX, int& sizeY, int& sizeZ, int s_maximumTextureSize, int t_maximumTextureSize, int r_maximumTextureSize)
{
    // compute nearest powers of two for each axis.
    int s_nearestPowerOfTwo = 1;
    while(s_nearestPowerOfTwo<sizeX && s_nearestPowerOfTwo<s_maximumTextureSize) s_nearestPowerOfTwo*=2;

    int t_nearestPowerOfTwo = 1;
    while(t_nearestPowerOfTwo<sizeY && t_nearestPowerOfTwo<t_maximumTextureSize) t_nearestPowerOfTwo*=2;

    int r_nearestPowerOfTwo = 1;
    while(r_nearestPowerOfTwo<sizeZ && r_nearestPowerOfTwo<r_maximumTextureSize) r_nearestPowerOfTwo*=2;

    sizeX = s_nearestPowerOfTwo;
    sizeY = t_nearestPowerOfTwo;
    sizeZ = r_nearestPowerOfTwo;
}

osg::Image* createTexture3D(ImageList& imageList, ProcessRow& processRow, 
            unsigned int numComponentsDesired, 
            int s_maximumTextureSize,
            int t_maximumTextureSize,
            int r_maximumTextureSize,
            bool resizeToPowerOfTwo)
{
    int max_s = 0;
    int max_t = 0;
    unsigned int max_components = 0;
    int total_r = 0;
    ImageList::iterator itr;
    for(itr=imageList.begin();
        itr!=imageList.end();
        ++itr)
    {
        osg::Image* image = itr->get();
        GLenum pixelFormat = image->getPixelFormat();
        if (pixelFormat==GL_ALPHA || 
            pixelFormat==GL_INTENSITY || 
            pixelFormat==GL_LUMINANCE || 
            pixelFormat==GL_LUMINANCE_ALPHA || 
            pixelFormat==GL_RGB || 
            pixelFormat==GL_RGBA)
        {
            max_s = osg::maximum(image->s(), max_s);
            max_t = osg::maximum(image->t(), max_t);
            max_components = osg::maximum(osg::Image::computeNumComponents(pixelFormat), max_components);
            total_r += image->r();
        }
        else
        {
            osg::notify(osg::NOTICE)<<"Image "<<image->getFileName()<<" has unsuitable pixel format"<< std::hex<< pixelFormat << std::dec << std::endl;
        }
    }
    
    if (numComponentsDesired!=0) max_components = numComponentsDesired;
    
    GLenum desiredPixelFormat = 0;
    switch(max_components)
    {
    case(1):
        osg::notify(osg::NOTICE)<<"desiredPixelFormat = GL_LUMINANCE" << std::endl;
        desiredPixelFormat = GL_LUMINANCE;
        break;
    case(2):
        osg::notify(osg::NOTICE)<<"desiredPixelFormat = GL_LUMINANCE_ALPHA" << std::endl;
        desiredPixelFormat = GL_LUMINANCE_ALPHA;
        break;
    case(3):
        osg::notify(osg::NOTICE)<<"desiredPixelFormat = GL_RGB" << std::endl;
        desiredPixelFormat = GL_RGB;
        break;
    case(4):
        osg::notify(osg::NOTICE)<<"desiredPixelFormat = GL_RGBA" << std::endl;
        desiredPixelFormat = GL_RGBA;
        break;
    }    
    if (desiredPixelFormat==0) return 0;
    
    // compute nearest powers of two for each axis.
    
    int s_nearestPowerOfTwo = 1;
    int t_nearestPowerOfTwo = 1;
    int r_nearestPowerOfTwo = 1;

    if (resizeToPowerOfTwo)
    {
        while(s_nearestPowerOfTwo<max_s && s_nearestPowerOfTwo<s_maximumTextureSize) s_nearestPowerOfTwo*=2;
        while(t_nearestPowerOfTwo<max_t && t_nearestPowerOfTwo<t_maximumTextureSize) t_nearestPowerOfTwo*=2;
        while(r_nearestPowerOfTwo<total_r && r_nearestPowerOfTwo<r_maximumTextureSize) r_nearestPowerOfTwo*=2;

        osg::notify(osg::NOTICE)<<"max image width = "<<max_s<<"  nearest power of two = "<<s_nearestPowerOfTwo<<std::endl;
        osg::notify(osg::NOTICE)<<"max image height = "<<max_t<<"  nearest power of two = "<<t_nearestPowerOfTwo<<std::endl;
        osg::notify(osg::NOTICE)<<"max image depth = "<<total_r<<"  nearest power of two = "<<r_nearestPowerOfTwo<<std::endl;
    }
    else
    {
        s_nearestPowerOfTwo = max_s;
        t_nearestPowerOfTwo = max_t;
        r_nearestPowerOfTwo = total_r;
    }
    
    // now allocate the 3d texture;
    osg::ref_ptr<osg::Image> image_3d = new osg::Image;
    image_3d->allocateImage(s_nearestPowerOfTwo,t_nearestPowerOfTwo,r_nearestPowerOfTwo,
                            desiredPixelFormat,GL_UNSIGNED_BYTE);
        

    unsigned int r_offset = (total_r<r_nearestPowerOfTwo) ? r_nearestPowerOfTwo/2 - total_r/2 : 0;

    int curr_dest_r = r_offset;

    // copy across the values from the source images into the image_3d.
    for(itr=imageList.begin();
        itr!=imageList.end();
        ++itr)
    {
        osg::Image* image = itr->get();
        GLenum pixelFormat = image->getPixelFormat();
        if (pixelFormat==GL_ALPHA || 
            pixelFormat==GL_LUMINANCE || 
            pixelFormat==GL_INTENSITY || 
            pixelFormat==GL_LUMINANCE_ALPHA || 
            pixelFormat==GL_RGB || 
            pixelFormat==GL_RGBA)
        {
        
            int num_r = osg::minimum(image->r(), (image_3d->r() - curr_dest_r));
            int num_t = osg::minimum(image->t(), image_3d->t());
            int num_s = osg::minimum(image->s(), image_3d->s());
        
            unsigned int s_offset_dest = (image->s()<s_nearestPowerOfTwo) ? s_nearestPowerOfTwo/2 - image->s()/2 : 0;
            unsigned int t_offset_dest = (image->t()<t_nearestPowerOfTwo) ? t_nearestPowerOfTwo/2 - image->t()/2 : 0;

            for(int r=0;r<num_r;++r, ++curr_dest_r)
            {
                for(int t=0;t<num_t;++t)
                {
                    unsigned char* dest = image_3d->data(s_offset_dest,t+t_offset_dest,curr_dest_r);
                    unsigned char* source = image->data(0,t,r);

                    processRow(num_s, image->getPixelFormat(), source, image_3d->getPixelFormat(), dest);
                }
            }
        }
    } 
    return image_3d.release();
}


struct ScaleOperator
{
    ScaleOperator():_scale(1.0f) {}
    ScaleOperator(float scale):_scale(scale) {}
    ScaleOperator(const ScaleOperator& so):_scale(so._scale) {}
    
    ScaleOperator& operator = (const ScaleOperator& so) { _scale = so._scale; return *this; }

    float _scale;

    inline void luminance(float& l) const { l*= _scale; } 
    inline void alpha(float& a) const { a*= _scale; } 
    inline void luminance_alpha(float& l,float& a) const { l*= _scale; a*= _scale;  } 
    inline void rgb(float& r,float& g,float& b) const { r*= _scale; g*=_scale; b*=_scale; }
    inline void rgba(float& r,float& g,float& b,float& a) const { r*= _scale; g*=_scale; b*=_scale; a*=_scale; }
};

struct RecordRowOperator
{
    RecordRowOperator(unsigned int num):_colours(num),_pos(0) {}

    mutable std::vector<osg::Vec4>  _colours;
    mutable unsigned int            _pos;
    
    inline void luminance(float l) const { rgba(l,l,l,1.0f); } 
    inline void alpha(float a) const { rgba(1.0f,1.0f,1.0f,a); } 
    inline void luminance_alpha(float l,float a) const { rgba(l,l,l,a);  } 
    inline void rgb(float r,float g,float b) const { rgba(r,g,b,1.0f); }
    inline void rgba(float r,float g,float b,float a) const { _colours[_pos++].set(r,g,b,a); }
};

struct WriteRowOperator
{
    WriteRowOperator():_pos(0) {}
    WriteRowOperator(unsigned int num):_colours(num),_pos(0) {}

    std::vector<osg::Vec4>  _colours;
    mutable unsigned int    _pos;
    
    inline void luminance(float& l) const { l = _colours[_pos++].r(); } 
    inline void alpha(float& a) const { a = _colours[_pos++].a(); } 
    inline void luminance_alpha(float& l,float& a) const { l = _colours[_pos].r(); a = _colours[_pos++].a(); } 
    inline void rgb(float& r,float& g,float& b) const { r = _colours[_pos].r(); g = _colours[_pos].g(); b = _colours[_pos].b(); }
    inline void rgba(float& r,float& g,float& b,float& a) const {  r = _colours[_pos].r(); g = _colours[_pos].g(); b = _colours[_pos].b(); a = _colours[_pos++].a(); }
};

osg::Image* readRaw(int sizeX, int sizeY, int sizeZ, int numberBytesPerComponent, int numberOfComponents, const std::string& endian, const std::string& raw_filename)
{
    osgDB::ifstream fin(raw_filename.c_str(), std::ifstream::binary);
    if (!fin) return 0;

    GLenum pixelFormat;
    switch(numberOfComponents)
    {
        case 1 : pixelFormat = GL_LUMINANCE; break;
        case 2 : pixelFormat = GL_LUMINANCE_ALPHA; break;
        case 3 : pixelFormat = GL_RGB; break;
        case 4 : pixelFormat = GL_RGBA; break;
        default :
            osg::notify(osg::NOTICE)<<"Error: numberOfComponents="<<numberOfComponents<<" not supported, only 1,2,3 or 4 are supported."<<std::endl;
            return 0;
    }

    
    GLenum dataType;
    switch(numberBytesPerComponent)
    {
        case 1 : dataType = GL_UNSIGNED_BYTE; break;
        case 2 : dataType = GL_UNSIGNED_SHORT; break;
        case 4 : dataType = GL_UNSIGNED_INT; break;
        default : 
            osg::notify(osg::NOTICE)<<"Error: numberBytesPerComponent="<<numberBytesPerComponent<<" not supported, only 1,2 or 4 are supported."<<std::endl;
            return 0;
    }
    
    int s_maximumTextureSize=256, t_maximumTextureSize=256, r_maximumTextureSize=256;
    
    int sizeS = sizeX;
    int sizeT = sizeY;
    int sizeR = sizeZ;
    clampToNearestValidPowerOfTwo(sizeS, sizeT, sizeR, s_maximumTextureSize, t_maximumTextureSize, r_maximumTextureSize);

    osg::ref_ptr<osg::Image> image = new osg::Image;
    image->allocateImage(sizeS, sizeT, sizeR, pixelFormat, dataType);
    
    
    bool endianSwap = (osg::getCpuByteOrder()==osg::BigEndian) ? (endian!="big") : (endian=="big");
    
    unsigned int r_offset = (sizeZ<sizeR) ? sizeR/2 - sizeZ/2 : 0;
    
    int offset = endianSwap ? numberBytesPerComponent : 0;
    int delta = endianSwap ? -1 : 1;
    for(int r=0;r<sizeZ;++r)
    {
        for(int t=0;t<sizeY;++t)
        {
            char* data = (char*) image->data(0,t,r+r_offset);
            for(int s=0;s<sizeX;++s)
            {
                if (!fin) return 0;
                
                for(int c=0;c<numberOfComponents;++c)
                {
                    char* ptr = data+offset;
                    for(int b=0;b<numberBytesPerComponent;++b)
                    {
                        fin.read((char*)ptr, 1);
                        ptr += delta;
                    }
                    data += numberBytesPerComponent;
                }
            }
        }
    }


    // normalise texture
    {
        // compute range of values
        osg::Vec4 minValue, maxValue;
        osg::computeMinMax(image.get(), minValue, maxValue);
        osg::modifyImage(image.get(),ScaleOperator(1.0f/maxValue.r())); 
    }
    
    
    fin.close();

    if (dataType!=GL_UNSIGNED_BYTE)
    {
        // need to convert to ubyte
        
        osg::ref_ptr<osg::Image> new_image = new osg::Image;
        new_image->allocateImage(sizeS, sizeT, sizeR, pixelFormat, GL_UNSIGNED_BYTE);
        
        RecordRowOperator readOp(sizeS);
        WriteRowOperator writeOp;

        for(int r=0;r<sizeR;++r)
        {
            for(int t=0;t<sizeT;++t)
            {
                // reset the indices to beginning
                readOp._pos = 0;
                writeOp._pos = 0;
            
                // read the pixels into readOp's _colour array
                osg::readRow(sizeS, pixelFormat, dataType, image->data(0,t,r), readOp);
                                
                // pass readOp's _colour array contents over to writeOp (note this is just a pointer swap).
                writeOp._colours.swap(readOp._colours);
                
                osg::modifyRow(sizeS, pixelFormat, GL_UNSIGNED_BYTE, new_image->data(0,t,r), writeOp);

                // return readOp's _colour array contents back to its rightful owner.
                writeOp._colours.swap(readOp._colours);
            }
        }
        
        image = new_image;
    }
    
    return image.release();
    
    
}

enum ColourSpaceOperation
{
    NO_COLOUR_SPACE_OPERATION,
    MODULATE_ALPHA_BY_LUMINANCE,
    MODULATE_ALPHA_BY_COLOUR,
    REPLACE_ALPHA_WITH_LUMINANACE,
    REPLACE_RGB_WITH_LUMINANCE
};

struct ModulateAlphaByLuminanceOperator
{
    ModulateAlphaByLuminanceOperator() {}

    inline void luminance(float&) const {} 
    inline void alpha(float&) const {} 
    inline void luminance_alpha(float& l,float& a) const { a*= l; } 
    inline void rgb(float&,float&,float&) const {}
    inline void rgba(float& r,float& g,float& b,float& a) const { float l = (r+g+b)*0.3333333; a *= l;}
};

struct ModulateAlphaByColourOperator
{
    ModulateAlphaByColourOperator(const osg::Vec4& colour):_colour(colour) { _lum = _colour.length(); }
    
    osg::Vec4 _colour;
    float _lum;

    inline void luminance(float&) const {} 
    inline void alpha(float&) const {} 
    inline void luminance_alpha(float& l,float& a) const { a*= l*_lum; } 
    inline void rgb(float&,float&,float&) const {}
    inline void rgba(float& r,float& g,float& b,float& a) const { a = (r*_colour.r()+g*_colour.g()+b*_colour.b()+a*_colour.a()); }
};

struct ReplaceAlphaWithLuminanceOperator
{
    ReplaceAlphaWithLuminanceOperator() {}

    inline void luminance(float&) const {} 
    inline void alpha(float&) const {} 
    inline void luminance_alpha(float& l,float& a) const { a= l; } 
    inline void rgb(float&,float&,float&) const { }
    inline void rgba(float& r,float& g,float& b,float& a) const { float l = (r+g+b)*0.3333333; a = l; }
};

osg::Image* doColourSpaceConversion(ColourSpaceOperation op, osg::Image* image, osg::Vec4& colour)
{
    switch(op)
    {
        case (MODULATE_ALPHA_BY_LUMINANCE):
        {
            std::cout<<"doing conversion MODULATE_ALPHA_BY_LUMINANCE"<<std::endl;
            osg::modifyImage(image,ModulateAlphaByLuminanceOperator()); 
            return image;
        }
        case (MODULATE_ALPHA_BY_COLOUR):
        {
            std::cout<<"doing conversion MODULATE_ALPHA_BY_COLOUR"<<std::endl;
            osg::modifyImage(image,ModulateAlphaByColourOperator(colour)); 
            return image;
        }
        case (REPLACE_ALPHA_WITH_LUMINANACE):
        {
            std::cout<<"doing conversion REPLACE_ALPHA_WITH_LUMINANACE"<<std::endl;
            osg::modifyImage(image,ReplaceAlphaWithLuminanceOperator()); 
            return image;
        }
        case (REPLACE_RGB_WITH_LUMINANCE):
        {
            std::cout<<"doing conversion REPLACE_ALPHA_WITH_LUMINANACE"<<std::endl;
            osg::Image* newImage = new osg::Image;
            newImage->allocateImage(image->s(), image->t(), image->r(), GL_LUMINANCE, image->getDataType());
            osg::copyImage(image, 0, 0, 0, image->s(), image->t(), image->r(),
                           newImage, 0, 0, 0, false);
            return newImage;
        }
        default:
            return image;
    }
}


osg::TransferFunction1D* readTransferFunctionFile(const std::string& filename)
{
    std::string foundFile = osgDB::findDataFile(filename);
    if (foundFile.empty()) 
    {
        std::cout<<"Error: could not find transfer function file : "<<filename<<std::endl;
        return 0;
    }
    
    std::cout<<"Reading transfer function "<<filename<<std::endl;

    osg::TransferFunction1D::ColorMap colorMap;
    osgDB::ifstream fin(foundFile.c_str());
    while(fin)
    {
        float value, red, green, blue, alpha;
        fin >> value >> red >> green >> blue >> alpha;
        if (fin) 
        {
            std::cout<<"value = "<<value<<" ("<<red<<", "<<green<<", "<<blue<<", "<<alpha<<")"<<std::endl;
            colorMap[value] = osg::Vec4(red,green,blue,alpha);
        }
    }
    
    if (colorMap.empty())
    {
        std::cout<<"Error: No values read from transfer function file: "<<filename<<std::endl;
        return 0;
    }
    
    osg::TransferFunction1D* tf = new osg::TransferFunction1D;
    tf->assign(colorMap);
    
    return tf;
}


class TestSupportOperation: public osg::GraphicsOperation
{
public:

    TestSupportOperation():
        osg::GraphicsOperation("TestSupportOperation",false),
        supported(true),
        errorMessage(),
        maximumTextureSize(256) {}

    virtual void operator () (osg::GraphicsContext* gc)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(mutex);

        glGetIntegerv( GL_MAX_3D_TEXTURE_SIZE, &maximumTextureSize );
        
        osg::notify(osg::NOTICE)<<"Max texture size="<<maximumTextureSize<<std::endl;
    }
        
    OpenThreads::Mutex  mutex;
    bool                supported;
    std::string         errorMessage;
    GLint               maximumTextureSize;
};



int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use of 3D textures.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("-s <numSlices>","Number of slices to create.");
    arguments.getApplicationUsage()->addCommandLineOption("--images [filenames]","Specify a stack of 2d images to build the 3d volume from.");
    arguments.getApplicationUsage()->addCommandLineOption("--shader","Use OpenGL Shading Language. (default)");
    arguments.getApplicationUsage()->addCommandLineOption("--no-shader","Disable use of OpenGL Shading Language.");
    arguments.getApplicationUsage()->addCommandLineOption("--gpu-tf","Aply the transfer function on the GPU. (default)");
    arguments.getApplicationUsage()->addCommandLineOption("--cpu-tf","Apply the transfer function on the CPU.");
    arguments.getApplicationUsage()->addCommandLineOption("--mip","Use Maximum Intensity Projection (MIP) filtering.");
    arguments.getApplicationUsage()->addCommandLineOption("--xSize <size>","Relative width of rendered brick.");
    arguments.getApplicationUsage()->addCommandLineOption("--ySize <size>","Relative length of rendered brick.");
    arguments.getApplicationUsage()->addCommandLineOption("--zSize <size>","Relative height of rendered brick.");
    arguments.getApplicationUsage()->addCommandLineOption("--xMultiplier <multiplier>","Tex coord x mulitplier.");
    arguments.getApplicationUsage()->addCommandLineOption("--yMultiplier <multiplier>","Tex coord y mulitplier.");
    arguments.getApplicationUsage()->addCommandLineOption("--zMultiplier <multiplier>","Tex coord z mulitplier.");
    arguments.getApplicationUsage()->addCommandLineOption("--clip <ratio>","clip volume as a ratio, 0.0 clip all, 1.0 clip none.");
    arguments.getApplicationUsage()->addCommandLineOption("--maxTextureSize <size>","Set the texture maximum resolution in the s,t,r (x,y,z) dimensions.");
    arguments.getApplicationUsage()->addCommandLineOption("--s_maxTextureSize <size>","Set the texture maximum resolution in the s (x) dimension.");
    arguments.getApplicationUsage()->addCommandLineOption("--t_maxTextureSize <size>","Set the texture maximum resolution in the t (y) dimension.");
    arguments.getApplicationUsage()->addCommandLineOption("--r_maxTextureSize <size>","Set the texture maximum resolution in the r (z) dimension.");
    arguments.getApplicationUsage()->addCommandLineOption("--compressed","Enable the usage of compressed textures.");
    arguments.getApplicationUsage()->addCommandLineOption("--compressed-arb","Enable the usage of OpenGL ARB compressed textures.");
    arguments.getApplicationUsage()->addCommandLineOption("--compressed-dxt1","Enable the usage of S3TC DXT1 compressed textures.");
    arguments.getApplicationUsage()->addCommandLineOption("--compressed-dxt3","Enable the usage of S3TC DXT3 compressed textures.");
    arguments.getApplicationUsage()->addCommandLineOption("--compressed-dxt5","Enable the usage of S3TC DXT5 compressed textures.");
    arguments.getApplicationUsage()->addCommandLineOption("--modulate-alpha-by-luminance","For each pixel multiply the alpha value by the luminance.");
    arguments.getApplicationUsage()->addCommandLineOption("--replace-alpha-with-luminance","For each pixel set the alpha value to the luminance.");
    arguments.getApplicationUsage()->addCommandLineOption("--replace-rgb-with-luminance","For each rgb pixel convert to the luminance.");
    arguments.getApplicationUsage()->addCommandLineOption("--num-components <num>","Set the number of components to in he target image.");
    arguments.getApplicationUsage()->addCommandLineOption("--no-rescale","Disable the rescaling of the pixel data to 0.0 to 1.0 range");
    arguments.getApplicationUsage()->addCommandLineOption("--rescale","Enable the rescale of the pixel data to 0.0 to 1.0 range (default).");
    arguments.getApplicationUsage()->addCommandLineOption("--shift-min-to-zero","Shift the pixel data so min value is 0.0.");
    arguments.getApplicationUsage()->addCommandLineOption("--sequence-length <num>","Set the length of time that a sequence of images with run for.");
    arguments.getApplicationUsage()->addCommandLineOption("--sd <num>","Short hand for --sequence-length");
//    arguments.getApplicationUsage()->addCommandLineOption("--raw <sizeX> <sizeY> <sizeZ> <numberBytesPerComponent> <numberOfComponents> <endian> <filename>","read a raw image data");

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    // add the window size toggle handler
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);
        
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

        keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
        
        osgGA::FlightManipulator* flightManipulator = new osgGA::FlightManipulator();
        flightManipulator->setYawControlMode(osgGA::FlightManipulator::NO_AUTOMATIC_YAW);
        keyswitchManipulator->addMatrixManipulator( '2', "Flight", flightManipulator );

        viewer.setCameraManipulator( keyswitchManipulator.get() );
    }

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    viewer.getCamera()->setClearColor(osg::Vec4(0.0f,0.0f,0.0f,0.0f));

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    std::string outputFile;
    while (arguments.read("-o",outputFile)) {}



    osg::ref_ptr<osg::TransferFunction1D> transferFunction;
    std::string tranferFunctionFile;
    while (arguments.read("--tf",tranferFunctionFile))
    {
        transferFunction = readTransferFunctionFile(tranferFunctionFile);
    }
    
    while(arguments.read("--test"))
    {
        transferFunction = new osg::TransferFunction1D;
        transferFunction->setColor(0.0, osg::Vec4(1.0,0.0,0.0,0.0));
        transferFunction->setColor(0.5, osg::Vec4(1.0,1.0,0.0,0.5));
        transferFunction->setColor(1.0, osg::Vec4(0.0,0.0,1.0,1.0));
    }

    while(arguments.read("--test2"))
    {
        transferFunction = new osg::TransferFunction1D;
        transferFunction->setColor(0.0, osg::Vec4(1.0,0.0,0.0,0.0));
        transferFunction->setColor(0.5, osg::Vec4(1.0,1.0,0.0,0.5));
        transferFunction->setColor(1.0, osg::Vec4(0.0,0.0,1.0,1.0));
        transferFunction->assign(transferFunction->getColorMap());
    }

    unsigned int numSlices=500;
    while (arguments.read("-s",numSlices)) {}
    
    
    float sliceEnd=1.0f;
    while (arguments.read("--clip",sliceEnd)) {}

    float alphaFunc=0.02f;
    while (arguments.read("--alphaFunc",alphaFunc)) {}


    
    ShadingModel shadingModel = Standard;
    while(arguments.read("--mip")) shadingModel =  MaximumIntensityProjection;

    while (arguments.read("--isosurface")) shadingModel = Isosurface;

    while (arguments.read("--light")) shadingModel = Light;

    float xSize=0.0f, ySize=0.0f, zSize=0.0f;
    while (arguments.read("--xSize",xSize)) {}
    while (arguments.read("--ySize",ySize)) {}
    while (arguments.read("--zSize",zSize)) {}

    osg::ref_ptr<TestSupportOperation> testSupportOperation = new TestSupportOperation;
    viewer.setRealizeOperation(testSupportOperation.get());
    
    viewer.realize();

    int maximumTextureSize = testSupportOperation->maximumTextureSize;
    int s_maximumTextureSize = maximumTextureSize;
    int t_maximumTextureSize = maximumTextureSize;
    int r_maximumTextureSize = maximumTextureSize;
    while(arguments.read("--maxTextureSize",maximumTextureSize))
    {
        s_maximumTextureSize = maximumTextureSize;
        t_maximumTextureSize = maximumTextureSize;
        r_maximumTextureSize = maximumTextureSize;
    }
    while(arguments.read("--s_maxTextureSize",s_maximumTextureSize)) {}
    while(arguments.read("--t_maxTextureSize",t_maximumTextureSize)) {}
    while(arguments.read("--r_maxTextureSize",r_maximumTextureSize)) {}

    osg::Texture::InternalFormatMode internalFormatMode = osg::Texture::USE_IMAGE_DATA_FORMAT;
    while(arguments.read("--compressed") || arguments.read("--compressed-arb")) { internalFormatMode = osg::Texture::USE_ARB_COMPRESSION; }

    while(arguments.read("--compressed-dxt1")) { internalFormatMode = osg::Texture::USE_S3TC_DXT1_COMPRESSION; }
    while(arguments.read("--compressed-dxt3")) { internalFormatMode = osg::Texture::USE_S3TC_DXT3_COMPRESSION; }
    while(arguments.read("--compressed-dxt5")) { internalFormatMode = osg::Texture::USE_S3TC_DXT5_COMPRESSION; }
    
    
    // set up colour space operation.
    ColourSpaceOperation colourSpaceOperation = NO_COLOUR_SPACE_OPERATION;
    osg::Vec4 colourModulate(0.25f,0.25f,0.25f,0.25f);
    while(arguments.read("--modulate-alpha-by-luminance")) { colourSpaceOperation = MODULATE_ALPHA_BY_LUMINANCE; }
    while(arguments.read("--modulate-alpha-by-colour", colourModulate.x(),colourModulate.y(),colourModulate.z(),colourModulate.w() )) { colourSpaceOperation = MODULATE_ALPHA_BY_COLOUR; }
    while(arguments.read("--replace-alpha-with-luminance")) { colourSpaceOperation = REPLACE_ALPHA_WITH_LUMINANACE; }
    while(arguments.read("--replace-rgb-with-luminance")) { colourSpaceOperation = REPLACE_RGB_WITH_LUMINANCE; }


    enum RescaleOperation
    {
        NO_RESCALE,
        RESCALE_TO_ZERO_TO_ONE_RANGE,
        SHIFT_MIN_TO_ZERO
    };
    
    RescaleOperation rescaleOperation = RESCALE_TO_ZERO_TO_ONE_RANGE;
    while(arguments.read("--no-rescale")) rescaleOperation = NO_RESCALE;
    while(arguments.read("--rescale")) rescaleOperation = RESCALE_TO_ZERO_TO_ONE_RANGE;
    while(arguments.read("--shift-min-to-zero")) rescaleOperation = SHIFT_MIN_TO_ZERO;

        
    bool resizeToPowerOfTwo = false;
    
    unsigned int numComponentsDesired = 0; 
    while(arguments.read("--num-components", numComponentsDesired)) {}

    bool useShader = true; 
    while(arguments.read("--shader")) { useShader = true; }
    while(arguments.read("--no-shader")) { useShader = false; }

    bool gpuTransferFunction = true; 
    while(arguments.read("--gpu-tf")) { gpuTransferFunction = true; }
    while(arguments.read("--cpu-tf")) { gpuTransferFunction = false; }

    double sequenceLength = 10.0;
    while(arguments.read("--sequence-duration", sequenceLength) || 
          arguments.read("--sd", sequenceLength)) {}

    typedef std::list< osg::ref_ptr<osg::Image> > Images;
    Images images;


    std::string vh_filename;
    while (arguments.read("--vh", vh_filename)) 
    {
        std::string raw_filename, transfer_filename;
	int xdim(0), ydim(0), zdim(0);

        osgDB::ifstream header(vh_filename.c_str());
        if (header)
        {
            header >> raw_filename >> transfer_filename >> xdim >> ydim >> zdim >> xSize >> ySize >> zSize;
        }
        
        if (xdim*ydim*zdim==0)
        {
            std::cout<<"Error in reading volume header "<<vh_filename<<std::endl;
            return 1;
        }
        
        if (!raw_filename.empty())
        {
            images.push_back(readRaw(xdim, ydim, zdim, 1, 1, "little", raw_filename));
        }
        
        if (!transfer_filename.empty())
        {
            osgDB::ifstream fin(transfer_filename.c_str());
            if (fin)
            {
                osg::TransferFunction1D::ColorMap colorMap;
                float value = 0.0;
                while(fin && value<=1.0)
                {
                    float red, green, blue, alpha;
                    fin >> red >> green >> blue >> alpha;
                    if (fin) 
                    {
                        colorMap[value] = osg::Vec4(red/255.0f,green/255.0f,blue/255.0f,alpha/255.0f);
                        std::cout<<"value = "<<value<<" ("<<red<<", "<<green<<", "<<blue<<", "<<alpha<<")";
                        std::cout<<"  ("<<colorMap[value]<<")"<<std::endl;
                    }
                    value += 1/255.0;
                }

                if (colorMap.empty())
                {
                    std::cout<<"Error: No values read from transfer function file: "<<transfer_filename<<std::endl;
                    return 0;
                }

                transferFunction = new osg::TransferFunction1D;
                transferFunction->assign(colorMap);
            }
        }

    }
    

    int sizeX, sizeY, sizeZ, numberBytesPerComponent, numberOfComponents;
    std::string endian, raw_filename;
    while (arguments.read("--raw", sizeX, sizeY, sizeZ, numberBytesPerComponent, numberOfComponents, endian, raw_filename)) 
    {
        images.push_back(readRaw(sizeX, sizeY, sizeZ, numberBytesPerComponent, numberOfComponents, endian, raw_filename));
    }

    int images_pos = arguments.find("--images");
    if (images_pos>=0)
    {
        ImageList imageList;
        int pos=images_pos+1;
        for(;pos<arguments.argc() && !arguments.isOption(pos);++pos)
        {
            // not an option so assume string is a filename.
            osg::Image *image = osgDB::readImageFile( arguments[pos]);

            if(image)
            {
                imageList.push_back(image);
            }
        }
        
        arguments.remove(images_pos, pos-images_pos);
        
        // pack the textures into a single texture.
        ProcessRow processRow;
        images.push_back(createTexture3D(imageList, processRow, numComponentsDesired, s_maximumTextureSize, t_maximumTextureSize, r_maximumTextureSize, resizeToPowerOfTwo));
    }


    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occurred when parsing the program arguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    

    // assume remaining arguments are file names of textures.
    for(int pos=1;pos<arguments.argc();++pos)
    {
        if (!arguments.isOption(pos))
        {
            std::string filename = arguments[pos];
            if (osgDB::getLowerCaseFileExtension(filename)=="dicom")
            {
                // not an option so assume string is a filename.
                osg::Image *image = osgDB::readImageFile(filename);
                if(image)
                {
                    images.push_back(image);
                }
            }
            else
            {
                osgDB::FileType fileType = osgDB::fileType(filename);
                if (fileType == osgDB::FILE_NOT_FOUND)
                {
                    filename = osgDB::findDataFile(filename);
                    fileType = osgDB::fileType(filename);
                }

                if (fileType == osgDB::DIRECTORY)
                {
                   osg::Image *image = osgDB::readImageFile(filename+".dicom");
                    if(image)
                    {
                        images.push_back(image);
                    } 
                }
                else if (fileType == osgDB::REGULAR_FILE)
                {
                    // not an option so assume string is a filename.
                    images.push_back(osgDB::readImageFile( filename ));
                }
                else
                {
                    osg::notify(osg::NOTICE)<<"Error: could not find file: "<<filename<<std::endl;
                    return 1;
                }
            }            
        }
    }
    
    if (images.empty()) 
    {
        std::cout<<"No model loaded, please specify and volumetric image file on the command line."<<std::endl;
        return 1;
    }


    Images::iterator sizeItr = images.begin();
    int image_s = (*sizeItr)->s();
    int image_t = (*sizeItr)->t();
    int image_r = (*sizeItr)->r();
    ++sizeItr;

    for(;sizeItr != images.end(); ++sizeItr)
    {
        if ((*sizeItr)->s() != image_s || 
            (*sizeItr)->t() != image_t ||
            (*sizeItr)->r() != image_r)
        {
            std::cout<<"Images in sequence are not of the same dimensions."<<std::endl;
            return 1;
        }
    }


    osg::ref_ptr<osg::RefMatrix> matrix = dynamic_cast<osg::RefMatrix*>(images.front()->getUserData());

    if (!matrix)
    {
        if (xSize==0.0) xSize = static_cast<float>(image_s);
        if (ySize==0.0) ySize = static_cast<float>(image_t);
        if (zSize==0.0) zSize = static_cast<float>(image_r);
        
        matrix = new osg::RefMatrix(xSize, 0.0,   0.0,   0.0,
                                    0.0,   ySize, 0.0,   0.0,
                                    0.0,   0.0,   zSize, 0.0,
                                    0.0,   0.0,   0.0,   1.0);
    }

    osg::Vec4 minValue(FLT_MAX, FLT_MAX, FLT_MAX, FLT_MAX);
    osg::Vec4 maxValue(-FLT_MAX, -FLT_MAX, -FLT_MAX, -FLT_MAX);
    bool computeMinMax = false;
    for(Images::iterator itr = images.begin();
        itr != images.end();
        ++itr)
    {
        osg::Vec4 localMinValue, localMaxValue;
        if (osg::computeMinMax(itr->get(), localMinValue, localMaxValue))
        {
            if (localMinValue.r()<minValue.r()) minValue.r() = localMinValue.r();
            if (localMinValue.g()<minValue.g()) minValue.g() = localMinValue.g();
            if (localMinValue.b()<minValue.b()) minValue.b() = localMinValue.b();
            if (localMinValue.a()<minValue.a()) minValue.a() = localMinValue.a();

            if (localMaxValue.r()>maxValue.r()) maxValue.r() = localMaxValue.r();
            if (localMaxValue.g()>maxValue.g()) maxValue.g() = localMaxValue.g();
            if (localMaxValue.b()>maxValue.b()) maxValue.b() = localMaxValue.b();
            if (localMaxValue.a()>maxValue.a()) maxValue.a() = localMaxValue.a();

            osg::notify(osg::NOTICE)<<"  ("<<localMinValue<<") ("<<localMaxValue<<") "<<(*itr)->getFileName()<<std::endl;

            computeMinMax = true;
        }
    }
    
    if (computeMinMax)
    {
        osg::notify(osg::NOTICE)<<"Min value "<<minValue<<std::endl;
        osg::notify(osg::NOTICE)<<"Max value "<<maxValue<<std::endl;

        float minComponent = minValue[0];
        minComponent = osg::minimum(minComponent,minValue[1]);
        minComponent = osg::minimum(minComponent,minValue[2]);
        minComponent = osg::minimum(minComponent,minValue[3]);

        float maxComponent = maxValue[0];
        maxComponent = osg::maximum(maxComponent,maxValue[1]);
        maxComponent = osg::maximum(maxComponent,maxValue[2]);
        maxComponent = osg::maximum(maxComponent,maxValue[3]);


        switch(rescaleOperation)
        {
            case(NO_RESCALE):
                break;

            case(RESCALE_TO_ZERO_TO_ONE_RANGE):
            {
                float scale = 0.99f/(maxComponent-minComponent);
                float offset = -minComponent * scale;

                for(Images::iterator itr = images.begin();
                    itr != images.end();
                    ++itr)
                {        
                    osg::offsetAndScaleImage(itr->get(), 
                        osg::Vec4(offset, offset, offset, offset),
                        osg::Vec4(scale, scale, scale, scale));
                }
                break;
            }
            case(SHIFT_MIN_TO_ZERO):
            {
                float offset = -minComponent;

                for(Images::iterator itr = images.begin();
                    itr != images.end();
                    ++itr)
                {        
                    osg::offsetAndScaleImage(itr->get(), 
                        osg::Vec4(offset, offset, offset, offset),
                        osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
                }
                break;
            }
        };

    }

    
    if (colourSpaceOperation!=NO_COLOUR_SPACE_OPERATION)
    {
        for(Images::iterator itr = images.begin();
            itr != images.end();
            ++itr)
        {        
            (*itr) = doColourSpaceConversion(colourSpaceOperation, itr->get(), colourModulate);
        }
    }
    
    if (!gpuTransferFunction && transferFunction.valid())
    {
        for(Images::iterator itr = images.begin();
            itr != images.end();
            ++itr)
        {        
            *itr = osgVolume::applyTransferFunction(itr->get(), transferFunction.get());
        }
    }
    
    osg::ref_ptr<osg::Image> image_3d = 0;

    if (images.size()==1)
    {
        osg::notify(osg::NOTICE)<<"Single image "<<images.size()<<" volumes."<<std::endl;
        image_3d = images.front();
    }
    else
    {
        osg::notify(osg::NOTICE)<<"Creating sequence of "<<images.size()<<" volumes."<<std::endl;
    
        osg::ref_ptr<osg::ImageSequence> imageSequence = new osg::ImageSequence;
        imageSequence->setLength(sequenceLength);
        image_3d = imageSequence.get();
        for(Images::iterator itr = images.begin();
            itr != images.end();
            ++itr)
        {        
            imageSequence->addImage(itr->get());
        }
        imageSequence->play();
    }
    
    osg::ref_ptr<osgVolume::Volume> volume = new osgVolume::Volume;
    osg::ref_ptr<osgVolume::VolumeTile> tile = new osgVolume::VolumeTile;
    volume->addChild(tile.get());

    osg::ref_ptr<osgVolume::Layer> layer = new osgVolume::ImageLayer(image_3d.get());

    osgVolume::Locator* locator = new osgVolume::Locator(*matrix);
    layer->setLocator(locator);
    tile->setLocator(locator);

    tile->setLayer(layer.get());

    tile->setEventCallback(new osgVolume::PropertyAdjustmentCallback());

    if (useShader)
    {

        osgVolume::SwitchProperty* sp = new osgVolume::SwitchProperty;
        sp->setActiveProperty(0);

        osgVolume::AlphaFuncProperty* ap = new osgVolume::AlphaFuncProperty(alphaFunc);
        osgVolume::SampleDensityProperty* sd = new osgVolume::SampleDensityProperty(0.005);
        osgVolume::TransparencyProperty* tp = new osgVolume::TransparencyProperty(1.0);
        osgVolume::TransferFunctionProperty* tfp = transferFunction.valid() ? new osgVolume::TransferFunctionProperty(transferFunction.get()) : 0;

        {
            // Standard
            osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
            cp->addProperty(ap);
            cp->addProperty(sd);
            cp->addProperty(tp);
            if (tfp) cp->addProperty(tfp);

            sp->addProperty(cp);
        }

        {
            // Light
            osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
            cp->addProperty(ap);
            cp->addProperty(sd);
            cp->addProperty(tp);
            cp->addProperty(new osgVolume::LightingProperty);
            if (tfp) cp->addProperty(tfp);

            sp->addProperty(cp);
        }

        {
            // Isosurface
            osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
            cp->addProperty(sd);
            cp->addProperty(tp);
            cp->addProperty(new osgVolume::IsoSurfaceProperty(alphaFunc));
            if (tfp) cp->addProperty(tfp);

            sp->addProperty(cp);
        }

        {
            // MaximumIntensityProjection
            osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
            cp->addProperty(ap);
            cp->addProperty(sd);
            cp->addProperty(tp);
            cp->addProperty(new osgVolume::MaximumIntensityProjectionProperty);
            if (tfp) cp->addProperty(tfp);

            sp->addProperty(cp);
        }

        switch(shadingModel)
        {
            case(Standard):                     sp->setActiveProperty(0); break;
            case(Light):                        sp->setActiveProperty(1); break;
            case(Isosurface):                   sp->setActiveProperty(2); break;
            case(MaximumIntensityProjection):   sp->setActiveProperty(3); break;
        }
        layer->addProperty(sp);


        tile->setVolumeTechnique(new osgVolume::RayTracedTechnique);
    }
    else
    {
        layer->addProperty(new osgVolume::AlphaFuncProperty(alphaFunc));
        tile->setVolumeTechnique(new osgVolume::FixedFunctionTechnique);
    }
        
    if (!outputFile.empty())
    {   
        std::string ext = osgDB::getFileExtension(outputFile);
        std::string name_no_ext = osgDB::getNameLessExtension(outputFile);
        if (ext=="osg")
        {
            if (image_3d.valid())
            {
                image_3d->setFileName(name_no_ext + ".dds");            
                osgDB::writeImageFile(*image_3d, image_3d->getFileName());
            }
            osgDB::writeNodeFile(*volume, outputFile);
        }
        else if (ext=="ive")
        {
            osgDB::writeNodeFile(*volume, outputFile);        
        }
        else if (ext=="dds")
        {
            osgDB::writeImageFile(*image_3d, outputFile);
        }
        else
        {
            std::cout<<"Extension not support for file output, not file written."<<std::endl;
        }
        
        return 0;
    }


    if (volume.valid()) 
    {

        // set the scene to render
        viewer.setSceneData(volume.get());
        
        // the the viewers main frame loop
        viewer.run();
    }    
    
    return 0;

}
