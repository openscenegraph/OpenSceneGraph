#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/Texture3D>
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

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>

#include <osgGA/EventVisitor>

#include <osgUtil/CullVisitor>

#include <osgProducer/Viewer>


typedef std::vector< osg::ref_ptr<osg::Image> > ImageList;

//  example ReadOperator
// struct ReadOperator
// {
//     inline void luminance(float l) const { rgba(l,l,l,1.0f); }
//     inline void alpha(float a) const { rgba(1.0f,1.0f,1.0f,a); }
//     inline void luminance_alpha(float l,float a) const { rgba(l,l,l,a); }
//     inline void rgb(float r,float g,float b) const { rgba(r,g,b,1.0f); }
//     inline void rgba(float r,float g,float b,float a) const { std::cout<<"pixel("<<r<<", "<<g<<", "<<b<<", "<<a<<")"<<std::endl; }
// };


template <typename T, class O>    
void _readRow(unsigned int num, GLenum pixelFormat, T* data,float scale, const O& operation)
{
    switch(pixelFormat)
    {
        case(GL_LUMINANCE):         { for(unsigned int i=0;i<num;++i) { float l = float(*data++)*scale; operation.luminance(l); } }  break;
        case(GL_ALPHA):             { for(unsigned int i=0;i<num;++i) { float a = float(*data++)*scale; operation.alpha(a); } }  break;
        case(GL_LUMINANCE_ALPHA):   { for(unsigned int i=0;i<num;++i) { float l = float(*data++)*scale; float a = float(*data++)*scale; operation.luminance_alpha(l,a); } }  break;
        case(GL_RGB):               { for(unsigned int i=0;i<num;++i) { float r = float(*data++)*scale; float g = float(*data++)*scale; float b = float(*data++)*scale; operation.rgb(r,g,b); } }  break;
        case(GL_RGBA):              { for(unsigned int i=0;i<num;++i) { float r = float(*data++)*scale; float g = float(*data++)*scale; float b = float(*data++)*scale; float a = float(*data++)*scale; operation.rgba(r,g,b,a); } }  break;
        case(GL_BGR):               { for(unsigned int i=0;i<num;++i) { float b = float(*data++)*scale; float g = float(*data++)*scale; float r = float(*data++)*scale; operation.rgb(r,g,b); } }  break;
        case(GL_BGRA):              { for(unsigned int i=0;i<num;++i) { float b = float(*data++)*scale; float g = float(*data++)*scale; float r = float(*data++)*scale; float a = float(*data++)*scale; operation.rgba(r,g,b,a); } }  break;
    }
}

template <class O>    
void readRow(unsigned int num, GLenum pixelFormat, GLenum dataType, unsigned char* data, const O& operation)
{
    switch(dataType)
    {
        case(GL_BYTE):              _readRow(num,pixelFormat, (char*)data,            1.0f/128.0f,        operation); break;
        case(GL_UNSIGNED_BYTE):     _readRow(num,pixelFormat, (unsigned char*)data,   1.0f/255.0f,        operation); break;
        case(GL_SHORT):             _readRow(num,pixelFormat, (short*) data,          1.0f/32768.0f,      operation); break;
        case(GL_UNSIGNED_SHORT):    _readRow(num,pixelFormat, (unsigned short*)data,  1.0f/65535.0f,      operation); break;
        case(GL_INT):               _readRow(num,pixelFormat, (int*) data,            1.0f/2147483648.0f, operation); break;
        case(GL_UNSIGNED_INT):      _readRow(num,pixelFormat, (unsigned int*) data,   1.0f/4294967295.0f, operation); break;
        case(GL_FLOAT):             _readRow(num,pixelFormat, (float*) data,          1.0f,               operation); break;
    }
}

template <class O>    
void readImage(osg::Image* image, const O& operation)
{
    if (!image) return;
    
    for(int r=0;r<image->r();++r)
    {
        for(int t=0;t<image->t();++t)
        {
            readRow(image->s(), image->getPixelFormat(), image->getDataType(), image->data(0,t,r), operation);
        }
    }
}

//  example ModifyOperator
// struct ModifyOperator
// {
//     inline void luminance(float& l) const {} 
//     inline void alpha(float& a) const {} 
//     inline void luminance_alpha(float& l,float& a) const {} 
//     inline void rgb(float& r,float& g,float& b) const {}
//     inline void rgba(float& r,float& g,float& b,float& a) const {}
// };


template <typename T, class M>    
void _modifyRow(unsigned int num, GLenum pixelFormat, T* data,float scale, const M& operation)
{
    float inv_scale = 1.0f/scale;
    switch(pixelFormat)
    {
        case(GL_LUMINANCE):         { for(unsigned int i=0;i<num;++i) { float l = float(*data)*scale; operation.luminance(l); *data++ = T(l*inv_scale); } }  break;
        case(GL_ALPHA):             { for(unsigned int i=0;i<num;++i) { float a = float(*data)*scale; operation.alpha(a); *data++ = T(a*inv_scale); } }  break;
        case(GL_LUMINANCE_ALPHA):   { for(unsigned int i=0;i<num;++i) { float l = float(*data)*scale; float a = float(*(data+1))*scale; operation.luminance_alpha(l,a); *data++ = T(l*inv_scale); *data++ = T(a*inv_scale); } }  break;
        case(GL_RGB):               { for(unsigned int i=0;i<num;++i) { float r = float(*data)*scale; float g = float(*(data+1))*scale; float b = float(*(data+2))*scale; operation.rgb(r,g,b); *data++ = T(r*inv_scale); *data++ = T(g*inv_scale); *data++ = T(b*inv_scale); } }  break;
        case(GL_RGBA):              { for(unsigned int i=0;i<num;++i) { float r = float(*data)*scale; float g = float(*(data+1))*scale; float b = float(*(data+2))*scale; float a = float(*(data+3))*scale; operation.rgba(r,g,b,a); *data++ = T(r*inv_scale); *data++ = T(g*inv_scale); *data++ = T(g*inv_scale); *data++ = T(a*inv_scale); } }  break;
        case(GL_BGR):               { for(unsigned int i=0;i<num;++i) { float b = float(*data)*scale; float g = float(*(data+1))*scale; float r = float(*(data+2))*scale; operation.rgb(r,g,b); *data++ = T(b*inv_scale); *data++ = T(g*inv_scale); *data++ = T(r*inv_scale); } }  break;
        case(GL_BGRA):              { for(unsigned int i=0;i<num;++i) { float b = float(*data)*scale; float g = float(*(data+1))*scale; float r = float(*(data+2))*scale; float a = float(*(data+3))*scale; operation.rgba(r,g,b,a); *data++ = T(g*inv_scale); *data++ = T(b*inv_scale); *data++ = T(r*inv_scale); *data++ = T(a*inv_scale); } }  break;
    }
}

template <class M>    
void modifyRow(unsigned int num, GLenum pixelFormat, GLenum dataType, unsigned char* data, const M& operation)
{
    switch(dataType)
    {
        case(GL_BYTE):              _modifyRow(num,pixelFormat, (char*)data,            1.0f/128.0f,        operation); break;
        case(GL_UNSIGNED_BYTE):     _modifyRow(num,pixelFormat, (unsigned char*)data,   1.0f/255.0f,        operation); break;
        case(GL_SHORT):             _modifyRow(num,pixelFormat, (short*) data,          1.0f/32768.0f,      operation); break;
        case(GL_UNSIGNED_SHORT):    _modifyRow(num,pixelFormat, (unsigned short*)data,  1.0f/65535.0f,      operation); break;
        case(GL_INT):               _modifyRow(num,pixelFormat, (int*) data,            1.0f/2147483648.0f, operation); break;
        case(GL_UNSIGNED_INT):      _modifyRow(num,pixelFormat, (unsigned int*) data,   1.0f/4294967295.0f, operation); break;
        case(GL_FLOAT):             _modifyRow(num,pixelFormat, (float*) data,          1.0f,               operation); break;
    }
}

template <class M>    
void modifyImage(osg::Image* image, const M& operation)
{
    if (!image) return;
    
    for(int r=0;r<image->r();++r)
    {
        for(int t=0;t<image->t();++t)
        {
            modifyRow(image->s(), image->getPixelFormat(), image->getDataType(), image->data(0,t,r), operation);
        }
    }
}

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
    // alpha luminiance sources..    
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
            int r_maximumTextureSize )
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
    while(s_nearestPowerOfTwo<max_s && s_nearestPowerOfTwo<s_maximumTextureSize) s_nearestPowerOfTwo*=2;

    int t_nearestPowerOfTwo = 1;
    while(t_nearestPowerOfTwo<max_t && t_nearestPowerOfTwo<t_maximumTextureSize) t_nearestPowerOfTwo*=2;

    int r_nearestPowerOfTwo = 1;
    while(r_nearestPowerOfTwo<total_r && r_nearestPowerOfTwo<r_maximumTextureSize) r_nearestPowerOfTwo*=2;


    osg::notify(osg::NOTICE)<<"max image width = "<<max_s<<"  nearest power of two = "<<s_nearestPowerOfTwo<<std::endl;
    osg::notify(osg::NOTICE)<<"max image height = "<<max_t<<"  nearest power of two = "<<t_nearestPowerOfTwo<<std::endl;
    osg::notify(osg::NOTICE)<<"max image depth = "<<total_r<<"  nearest power of two = "<<r_nearestPowerOfTwo<<std::endl;
    
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


osg::Image* createNormalMapTexture(osg::Image* image_3d)
{
    unsigned int sourcePixelIncrement = 1;
    unsigned int alphaOffset = 0; 
    switch(image_3d->getPixelFormat())
    {
    case(GL_ALPHA):
    case(GL_LUMINANCE):
        sourcePixelIncrement = 1;
        alphaOffset = 0;
        break;
    case(GL_LUMINANCE_ALPHA):
        sourcePixelIncrement = 2;
        alphaOffset = 1;
        break;
    case(GL_RGB):
        sourcePixelIncrement = 3;
        alphaOffset = 0;
        break;
    case(GL_RGBA):
        sourcePixelIncrement = 4;
        alphaOffset = 3;
        break;
    default:
        osg::notify(osg::NOTICE)<<"Source pixel format not support for normal map generation."<<std::endl;
        return 0;
    }
    
    osg::ref_ptr<osg::Image> normalmap_3d = new osg::Image;
    normalmap_3d->allocateImage(image_3d->s(),image_3d->t(),image_3d->r(),
                            GL_RGBA,GL_UNSIGNED_BYTE);

    if (osg::getCpuByteOrder()==osg::LittleEndian) alphaOffset = sourcePixelIncrement-alphaOffset-1;

    for(int r=1;r<image_3d->r()-1;++r)
    {
        for(int t=1;t<image_3d->t()-1;++t)
        {
            unsigned char* ptr = image_3d->data(1,t,r)+alphaOffset;
            unsigned char* left = image_3d->data(0,t,r)+alphaOffset;
            unsigned char* right = image_3d->data(2,t,r)+alphaOffset;
            unsigned char* above = image_3d->data(1,t+1,r)+alphaOffset;
            unsigned char* below = image_3d->data(1,t-1,r)+alphaOffset;
            unsigned char* in = image_3d->data(1,t,r+1)+alphaOffset;
            unsigned char* out = image_3d->data(1,t,r-1)+alphaOffset;

            unsigned char* destination = (unsigned char*) normalmap_3d->data(1,t,r);

            for(int s=1;s<image_3d->s()-1;++s)
            {

                osg::Vec3 grad((float)(*left)-(float)(*right),
                               (float)(*below)-(float)(*above),
                               (float)(*out) -(float)(*in));

                grad.normalize();

                if (grad.x()==0.0f && grad.y()==0.0f && grad.z()==0.0f)
                {
                    grad.set(128.0f,128.0f,128.0f);
                }
                else
                {
                    grad.x() = osg::clampBetween((grad.x()+1.0f)*128.0f,0.0f,255.0f);
                    grad.y() = osg::clampBetween((grad.y()+1.0f)*128.0f,0.0f,255.0f);
                    grad.z() = osg::clampBetween((grad.z()+1.0f)*128.0f,0.0f,255.0f);
                }

                *(destination++) = (unsigned char)(grad.x()); // scale and bias X.
                *(destination++) = (unsigned char)(grad.y()); // scale and bias Y.
                *(destination++) = (unsigned char)(grad.z()); // scale and bias Z.

                *destination++ = *ptr;

                ptr += sourcePixelIncrement;
                left += sourcePixelIncrement;
                right += sourcePixelIncrement;
                above += sourcePixelIncrement;
                below += sourcePixelIncrement;
                in += sourcePixelIncrement;
                out += sourcePixelIncrement;
            }
        }
    }
    
    return normalmap_3d.release();
}



osg::Node* createCube(float size,float alpha, unsigned int numSlices, float sliceEnd=1.0f)
{

    // set up the Geometry.
    osg::Geometry* geom = new osg::Geometry;

    float halfSize = size*0.5f;
    float y = halfSize;
    float dy =-size*1.4/(float)(numSlices-1)*sliceEnd;

    //y = -halfSize;
    //dy *= 0.5;

    osg::Vec3Array* coords = new osg::Vec3Array(4*numSlices);
    geom->setVertexArray(coords);
    for(unsigned int i=0;i<numSlices;++i, y+=dy)
    {
        (*coords)[i*4+0].set(-halfSize,y,halfSize);
        (*coords)[i*4+1].set(-halfSize,y,-halfSize);
        (*coords)[i*4+2].set(halfSize,y,-halfSize);
        (*coords)[i*4+3].set(halfSize,y,halfSize);
    }
    
    osg::Vec3Array* normals = new osg::Vec3Array(1);
    (*normals)[0].set(0.0f,-1.0f,0.0f);
    geom->setNormalArray(normals);
    geom->setNormalBinding(osg::Geometry::BIND_OVERALL);

    osg::Vec4Array* colors = new osg::Vec4Array(1);
    (*colors)[0].set(1.0f,1.0f,1.0f,alpha);
    geom->setColorArray(colors);
    geom->setColorBinding(osg::Geometry::BIND_OVERALL);

    geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,coords->size()));

    osg::Billboard* billboard = new osg::Billboard;
    billboard->setMode(osg::Billboard::POINT_ROT_WORLD);
    billboard->addDrawable(geom);
    billboard->setPosition(0,osg::Vec3(0.0f,0.0f,0.0f));
    
    return billboard;
}

class FollowMouseCallback : public osgGA::GUIEventHandler, public osg::StateSet::Callback
{
    public:
    
        FollowMouseCallback()
        {
            _updateTransparency = false;
            _updateAlphaCutOff = false;
            _updateSampleDensity = false;
        }

        FollowMouseCallback(const FollowMouseCallback&,const osg::CopyOp&) {}

        META_Object(osg,FollowMouseCallback);

        virtual void operator() (osg::StateSet* stateset, osg::NodeVisitor* nv)
        {
            if (nv->getVisitorType()==osg::NodeVisitor::EVENT_VISITOR)
            {
                osgGA::EventVisitor* ev = dynamic_cast<osgGA::EventVisitor*>(nv);
                if (ev)
                {
                    osgGA::GUIActionAdapter* aa = ev->getActionAdapter();
                    osgGA::EventQueue::Events& events = ev->getEvents();
                    for(osgGA::EventQueue::Events::iterator itr=events.begin();
                        itr!=events.end();
                        ++itr)
                    {
                        handle(*(*itr), *aa, stateset, ev);
                    }
                }
            }
        }
        
        virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&, osg::Object* object, osg::NodeVisitor*)
        {
            osg::StateSet* stateset = dynamic_cast<osg::StateSet*>(object);
            if (!stateset) return false;
            
            switch(ea.getEventType())
            {
                case(osgGA::GUIEventAdapter::MOVE):
                case(osgGA::GUIEventAdapter::DRAG):
                {
                    float v = ea.getY()*0.5f+0.5f;
                    osg::Uniform* uniform = 0;
                    if (_updateTransparency && (uniform = stateset->getUniform("transparency"))) uniform->set(v);
                    if (_updateAlphaCutOff && (uniform = stateset->getUniform("alphaCutOff"))) uniform->set(v);
                    if (_updateSampleDensity && (uniform = stateset->getUniform("sampleDensity"))) uniform->set(powf(v,5));
                    break;
                }
                case(osgGA::GUIEventAdapter::KEYDOWN):
                {
                    if (ea.getKey()=='t') _updateTransparency = true;
                    if (ea.getKey()=='a') _updateAlphaCutOff = true;
                    if (ea.getKey()=='d') _updateSampleDensity = true;
                    break;
                }
                case(osgGA::GUIEventAdapter::KEYUP):
                {
                    if (ea.getKey()=='t') _updateTransparency = false;
                    if (ea.getKey()=='a') _updateAlphaCutOff = false;
                    if (ea.getKey()=='d') _updateSampleDensity = false;
                    break;
                }
                default:
                    break;
            }
            return false;
        }
                
        virtual void accept(osgGA::GUIEventHandlerVisitor& v)
        {
            v.visit(*this);
        }
        
        bool _updateTransparency;
        bool _updateAlphaCutOff;
        bool _updateSampleDensity;

};

osg::Node* createShaderModel(osg::ref_ptr<osg::Image>& image_3d, osg::ref_ptr<osg::Image>& /*normalmap_3d*/,
                       osg::Texture::InternalFormatMode internalFormatMode,
                       float /*xSize*/, float /*ySize*/, float /*zSize*/,
                       float /*xMultiplier*/, float /*yMultiplier*/, float /*zMultiplier*/,
                       unsigned int /*numSlices*/=500, float /*sliceEnd*/=1.0f, float alphaFuncValue=0.02f)
{
    osg::Geode* geode = new osg::Geode;
    osg::StateSet* stateset = geode->getOrCreateStateSet();
    
    stateset->setEventCallback(new FollowMouseCallback);

    // set up the 3d texture itself,
    // note, well set the filtering up so that mip mapping is disabled,
    // gluBuild3DMipsmaps doesn't do a very good job of handled the
    // inbalanced dimensions of the 256x256x4 texture.
    osg::Texture3D* texture3D = new osg::Texture3D;
    texture3D->setFilter(osg::Texture3D::MIN_FILTER,osg::Texture3D::LINEAR);
    texture3D->setFilter(osg::Texture3D::MAG_FILTER,osg::Texture3D::LINEAR);
    texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP);
    texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP);
    texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP);
    if (image_3d->getPixelFormat()==GL_ALPHA || 
        image_3d->getPixelFormat()==GL_LUMINANCE)
    {
        texture3D->setInternalFormatMode(osg::Texture3D::USE_USER_DEFINED_FORMAT);
        texture3D->setInternalFormat(GL_INTENSITY);
    }
    else
    {
        texture3D->setInternalFormatMode(internalFormatMode);
    }

    texture3D->setImage(image_3d.get());

    stateset->setTextureAttributeAndModes(0,texture3D,osg::StateAttribute::ON);

    osg::Program* program = new osg::Program;
    stateset->setAttribute(program);

    // get shaders from source
    std::string vertexShaderFile = osgDB::findDataFile("volume.vert");
    if (!vertexShaderFile.empty())
    {
        program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, vertexShaderFile));
    }
    else
    {
        char vertexShaderSource[] = 
            "varying vec3 texcoord;\n"
            "varying vec3 deltaTexCoord;\n"
            "\n"
            "void main(void)\n"
            "{\n"
            "    texcoord = gl_MultiTexCoord0.xyz;\n"
            "    gl_Position     = ftransform();  \n"
            "    deltaTexCoord = normalize(gl_ModelViewMatrixInverse * vec4(0,0,0,1) - gl_Vertex);\n"
            "}\n";

        osg::Shader* vertex_shader = new osg::Shader(osg::Shader::VERTEX, vertexShaderSource);
        program->addShader(vertex_shader);

    }
    
    std::string fragmentShaderFile = osgDB::findDataFile("volume.frag");
    if (!fragmentShaderFile.empty())
    {
        program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, fragmentShaderFile));
    }
    else
    {
        //////////////////////////////////////////////////////////////////
        // fragment shader
        //
        char fragmentShaderSource[] = 
            "uniform sampler3D baseTexture;\n"
            "uniform float sampleDensity;\n"
            "uniform float transparency;\n"
            "uniform float alphaCutOff;\n"
            "\n"
            "varying vec3 deltaTexCoord;\n"
            "varying vec3 texcoord;\n"
            "void main(void) \n"
            "{ \n"
            "    vec3 deltaTexCoord2 = normalize(deltaTexCoord)*sampleDensity; \n"
            "\n"
            "    gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0); \n"
            "    \n"
            "    while (texcoord.x>=0.0 && texcoord.x<=1.0 &&\n"
            "           texcoord.y>=0.0 && texcoord.y<=1.0 &&\n"
            "           texcoord.z>=0.0 && texcoord.z<=1.0)\n"
            "    {\n"
            "       vec4 color = texture3D( baseTexture, texcoord);\n"
            "       float r = color[3]*transparency;\n"
            "       if (r>alphaCutOff)\n"
            "       {\n"
            "         gl_FragColor.xyz = gl_FragColor.xyz*(1.0-r)+color.xyz*r;\n"
            "         gl_FragColor.w += r;\n"
            "       }\n"
            "       texcoord += deltaTexCoord2; \n"
            "    }\n"
            "    if (gl_FragColor.w>1.0) gl_FragColor.w = 1.0; \n"
            "}\n";

        osg::Shader* fragment_shader = new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource);
        program->addShader(fragment_shader);
    }

    osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
    stateset->addUniform(baseTextureSampler);

    osg::Uniform* sampleDensity = new osg::Uniform("sampleDensity", 0.01f);
    stateset->addUniform(sampleDensity);

    osg::Uniform* transpancy = new osg::Uniform("transparency",0.5f);
    stateset->addUniform(transpancy);

    osg::Uniform* alphaCutOff = new osg::Uniform("alphaCutOff",alphaFuncValue);
    stateset->addUniform(alphaCutOff);

    stateset->setMode(GL_CULL_FACE, osg::StateAttribute::ON);

    {
        osg::Geometry* geom = new osg::Geometry;

        osg::Vec3Array* coords = new osg::Vec3Array(8);
        (*coords)[0].set(0,0,0);
        (*coords)[1].set(1,0,0);
        (*coords)[2].set(1,1,0);
        (*coords)[3].set(0,1,0);
        (*coords)[4].set(0,0,1);
        (*coords)[5].set(1,0,1);
        (*coords)[6].set(1,1,1);
        (*coords)[7].set(0,1,1);
        geom->setVertexArray(coords);

        osg::Vec3Array* tcoords = new osg::Vec3Array(8);
        (*tcoords)[0].set(0,0,0);
        (*tcoords)[1].set(1,0,0);
        (*tcoords)[2].set(1,1,0);
        (*tcoords)[3].set(0,1,0);
        (*tcoords)[4].set(0,0,1);
        (*tcoords)[5].set(1,0,1);
        (*tcoords)[6].set(1,1,1);
        (*tcoords)[7].set(0,1,1);
        geom->setTexCoordArray(0,tcoords);

        osg::Vec4Array* colours = new osg::Vec4Array(1);
        (*colours)[0].set(1.0f,1.0f,1.0,1.0f);
        geom->setColorArray(colours);
        geom->setColorBinding(osg::Geometry::BIND_OVERALL);

        osg::DrawElementsUShort* drawElements = new osg::DrawElementsUShort(GL_QUADS);
        // bottom
        drawElements->push_back(0);
        drawElements->push_back(1);
        drawElements->push_back(2);
        drawElements->push_back(3);
        
        // bottom
        drawElements->push_back(3);
        drawElements->push_back(2);
        drawElements->push_back(6);
        drawElements->push_back(7);

        // left
        drawElements->push_back(0);
        drawElements->push_back(3);
        drawElements->push_back(7);
        drawElements->push_back(4);

        // right
        drawElements->push_back(5);
        drawElements->push_back(6);
        drawElements->push_back(2);
        drawElements->push_back(1);

        // front
        drawElements->push_back(1);
        drawElements->push_back(0);
        drawElements->push_back(4);
        drawElements->push_back(5);

        // top
        drawElements->push_back(7);
        drawElements->push_back(6);
        drawElements->push_back(5);
        drawElements->push_back(4);

        geom->addPrimitiveSet(drawElements);

        geode->addDrawable(geom);

    } 

    return geode;
}

osg::Node* createModel(osg::ref_ptr<osg::Image>& image_3d, osg::ref_ptr<osg::Image>& normalmap_3d,
                       osg::Texture::InternalFormatMode internalFormatMode,
                       float xSize, float ySize, float zSize,
                       float xMultiplier, float yMultiplier, float zMultiplier,
                       unsigned int numSlices=500, float sliceEnd=1.0f, float alphaFuncValue=0.02f)
{
    bool two_pass = normalmap_3d.valid() && (image_3d->getPixelFormat()==GL_RGB || image_3d->getPixelFormat()==GL_RGBA);

    osg::Group* group = new osg::Group;
    
    osg::TexGenNode* texgenNode_0 = new osg::TexGenNode;
    texgenNode_0->setTextureUnit(0);
    texgenNode_0->getTexGen()->setMode(osg::TexGen::EYE_LINEAR);
    texgenNode_0->getTexGen()->setPlane(osg::TexGen::S, osg::Vec4(xMultiplier,0.0f,0.0f,0.5f));
    texgenNode_0->getTexGen()->setPlane(osg::TexGen::T, osg::Vec4(0.0f,yMultiplier,0.0f,0.5f));
    texgenNode_0->getTexGen()->setPlane(osg::TexGen::R, osg::Vec4(0.0f,0.0f,zMultiplier,0.5f));
    
    if (two_pass)
    {
        osg::TexGenNode* texgenNode_1 = new osg::TexGenNode;
        texgenNode_1->setTextureUnit(1);
        texgenNode_1->getTexGen()->setMode(osg::TexGen::EYE_LINEAR);
        texgenNode_1->getTexGen()->setPlane(osg::TexGen::S, texgenNode_0->getTexGen()->getPlane(osg::TexGen::S));
        texgenNode_1->getTexGen()->setPlane(osg::TexGen::T, texgenNode_0->getTexGen()->getPlane(osg::TexGen::T));
        texgenNode_1->getTexGen()->setPlane(osg::TexGen::R, texgenNode_0->getTexGen()->getPlane(osg::TexGen::R));

        texgenNode_1->addChild(texgenNode_0);

        group->addChild(texgenNode_1);
    }
    else
    {  
        group->addChild(texgenNode_0);
    }

    osg::BoundingBox bb(-xSize*0.5f,-ySize*0.5f,-zSize*0.5f,xSize*0.5f,ySize*0.5f,zSize*0.5f);

    osg::ClipNode* clipnode = new osg::ClipNode;
    clipnode->addChild(createCube(1.0f,1.0f, numSlices,sliceEnd));
    clipnode->createClipBox(bb);

    {
        // set up the Geometry to enclose the clip volume to prevent near/far clipping from affecting billboard
        osg::Geometry* geom = new osg::Geometry;

        osg::Vec3Array* coords = new osg::Vec3Array();
        coords->push_back(bb.corner(0));
        coords->push_back(bb.corner(1));
        coords->push_back(bb.corner(2));
        coords->push_back(bb.corner(3));
        coords->push_back(bb.corner(4));
        coords->push_back(bb.corner(5));
        coords->push_back(bb.corner(6));
        coords->push_back(bb.corner(7));

        geom->setVertexArray(coords);

        osg::Vec4Array* colors = new osg::Vec4Array(1);
        (*colors)[0].set(1.0f,1.0f,1.0f,1.0f);
        geom->setColorArray(colors);
        geom->setColorBinding(osg::Geometry::BIND_OVERALL);

        geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,coords->size()));

        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(geom);
        
        clipnode->addChild(geode);
        
    }

    texgenNode_0->addChild(clipnode);

    osg::StateSet* stateset = texgenNode_0->getOrCreateStateSet();

    stateset->setMode(GL_LIGHTING,osg::StateAttribute::ON);
    stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
    stateset->setAttribute(new osg::AlphaFunc(osg::AlphaFunc::GREATER,alphaFuncValue));
    
    osg::Material* material = new osg::Material;
    material->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    stateset->setAttributeAndModes(material);
    
    osg::Vec3 lightDirection(1.0f,-1.0f,1.0f);
    lightDirection.normalize();

    if (normalmap_3d.valid())
    {
        if (two_pass)
        {

            // set up normal texture
            osg::Texture3D* bump_texture3D = new osg::Texture3D;
            bump_texture3D->setFilter(osg::Texture3D::MIN_FILTER,osg::Texture3D::LINEAR);
            bump_texture3D->setFilter(osg::Texture3D::MAG_FILTER,osg::Texture3D::LINEAR);
            bump_texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP);
            bump_texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP);
            bump_texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP);
            bump_texture3D->setImage(normalmap_3d.get());

            bump_texture3D->setInternalFormatMode(internalFormatMode);

            stateset->setTextureAttributeAndModes(0,bump_texture3D,osg::StateAttribute::ON);

            osg::TexEnvCombine* tec = new osg::TexEnvCombine;
            tec->setConstantColorAsLightDirection(lightDirection);

            tec->setCombine_RGB(osg::TexEnvCombine::DOT3_RGB);
            tec->setSource0_RGB(osg::TexEnvCombine::CONSTANT);
            tec->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
            tec->setSource1_RGB(osg::TexEnvCombine::TEXTURE);
            tec->setOperand1_RGB(osg::TexEnvCombine::SRC_COLOR);

            tec->setCombine_Alpha(osg::TexEnvCombine::REPLACE);
            tec->setSource0_Alpha(osg::TexEnvCombine::PRIMARY_COLOR);
            tec->setOperand0_Alpha(osg::TexEnvCombine::SRC_ALPHA);
            tec->setSource1_Alpha(osg::TexEnvCombine::TEXTURE);
            tec->setOperand1_Alpha(osg::TexEnvCombine::SRC_ALPHA);

            stateset->setTextureAttributeAndModes(0, tec, osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

            stateset->setTextureMode(0,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
            stateset->setTextureMode(0,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
            stateset->setTextureMode(0,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);


            // set up color texture
            osg::Texture3D* texture3D = new osg::Texture3D;
            texture3D->setFilter(osg::Texture3D::MIN_FILTER,osg::Texture3D::LINEAR);
            texture3D->setFilter(osg::Texture3D::MAG_FILTER,osg::Texture3D::LINEAR);
            texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP);
            texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP);
            texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP);
            if (image_3d->getPixelFormat()==GL_ALPHA || 
                image_3d->getPixelFormat()==GL_LUMINANCE)
            {
                texture3D->setInternalFormatMode(osg::Texture3D::USE_USER_DEFINED_FORMAT);
                texture3D->setInternalFormat(GL_INTENSITY);
            }
            else
            {
                texture3D->setInternalFormatMode(internalFormatMode);
            }
            texture3D->setImage(image_3d.get());

            stateset->setTextureAttributeAndModes(1,texture3D,osg::StateAttribute::ON);

            stateset->setTextureMode(1,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
            stateset->setTextureMode(1,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
            stateset->setTextureMode(1,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);

            stateset->setTextureAttributeAndModes(1,new osg::TexEnv(),osg::StateAttribute::ON);

        }
        else
        {
            osg::ref_ptr<osg::Image> normalmap_3d = createNormalMapTexture(image_3d.get());
            osg::Texture3D* bump_texture3D = new osg::Texture3D;
            bump_texture3D->setFilter(osg::Texture3D::MIN_FILTER,osg::Texture3D::LINEAR);
            bump_texture3D->setFilter(osg::Texture3D::MAG_FILTER,osg::Texture3D::LINEAR);
            bump_texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP);
            bump_texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP);
            bump_texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP);
            bump_texture3D->setImage(normalmap_3d.get());

            bump_texture3D->setInternalFormatMode(internalFormatMode);

            stateset->setTextureAttributeAndModes(0,bump_texture3D,osg::StateAttribute::ON);

            osg::TexEnvCombine* tec = new osg::TexEnvCombine;
            tec->setConstantColorAsLightDirection(lightDirection);

            tec->setCombine_RGB(osg::TexEnvCombine::DOT3_RGB);
            tec->setSource0_RGB(osg::TexEnvCombine::CONSTANT);
            tec->setOperand0_RGB(osg::TexEnvCombine::SRC_COLOR);
            tec->setSource1_RGB(osg::TexEnvCombine::TEXTURE);
            tec->setOperand1_RGB(osg::TexEnvCombine::SRC_COLOR);

            tec->setCombine_Alpha(osg::TexEnvCombine::MODULATE);
            tec->setSource0_Alpha(osg::TexEnvCombine::PRIMARY_COLOR);
            tec->setOperand0_Alpha(osg::TexEnvCombine::SRC_ALPHA);
            tec->setSource1_Alpha(osg::TexEnvCombine::TEXTURE);
            tec->setOperand1_Alpha(osg::TexEnvCombine::SRC_ALPHA);

            stateset->setTextureAttributeAndModes(0, tec, osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

            stateset->setTextureMode(0,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
            stateset->setTextureMode(0,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
            stateset->setTextureMode(0,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);

            image_3d = normalmap_3d;
        }
    }
    else
    {     
        // set up the 3d texture itself,
        // note, well set the filtering up so that mip mapping is disabled,
        // gluBuild3DMipsmaps doesn't do a very good job of handled the
        // inbalanced dimensions of the 256x256x4 texture.
        osg::Texture3D* texture3D = new osg::Texture3D;
        texture3D->setFilter(osg::Texture3D::MIN_FILTER,osg::Texture3D::LINEAR);
        texture3D->setFilter(osg::Texture3D::MAG_FILTER,osg::Texture3D::LINEAR);
        texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP);
        texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP);
        texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP);
        if (image_3d->getPixelFormat()==GL_ALPHA || 
            image_3d->getPixelFormat()==GL_LUMINANCE)
        {
            texture3D->setInternalFormatMode(osg::Texture3D::USE_USER_DEFINED_FORMAT);
            texture3D->setInternalFormat(GL_INTENSITY);
        }
        else
        {
            texture3D->setInternalFormatMode(internalFormatMode);
        }

        texture3D->setImage(image_3d.get());

        stateset->setTextureAttributeAndModes(0,texture3D,osg::StateAttribute::ON);

        stateset->setTextureMode(0,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
        stateset->setTextureMode(0,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
        stateset->setTextureMode(0,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);

        stateset->setTextureAttributeAndModes(0,new osg::TexEnv(),osg::StateAttribute::ON);
    }
 
    return group;
}

struct FindRangeOperator
{
    FindRangeOperator():
        _rmin(FLT_MAX),
        _rmax(-FLT_MAX),
        _gmin(FLT_MAX),
        _gmax(-FLT_MAX),
        _bmin(FLT_MAX),
        _bmax(-FLT_MAX),
        _amin(FLT_MAX),
        _amax(-FLT_MAX) {}
        
    mutable float _rmin, _rmax, _gmin, _gmax, _bmin, _bmax, _amin, _amax;

    inline void luminance(float l) const { rgb(l,l,l); } 
    inline void alpha(float a) const { _amin = osg::minimum(a,_amin); _amax = osg::maximum(a,_amax); } 
    inline void luminance_alpha(float l,float a) const { rgb(l,l,l); alpha(a); } 
    inline void rgb(float r,float g,float b) const { _rmin = osg::minimum(r,_rmin); _rmax = osg::maximum(r,_rmax); _gmin = osg::minimum(g,_gmin); _gmax = osg::maximum(g,_gmax); _bmin = osg::minimum(b,_bmin); _bmax = osg::maximum(b,_bmax);  }
    inline void rgba(float r,float g,float b,float a) const { rgb(r,g,b); alpha(a); }
};
 
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
    std::ifstream fin(raw_filename.c_str(), std::ifstream::binary);
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
        FindRangeOperator rangeOp;    
        readImage(image.get(), rangeOp);
        modifyImage(image.get(),ScaleOperator(1.0f/rangeOp._rmax)); 
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
                // reset the indices to begining
                readOp._pos = 0;
                writeOp._pos = 0;
            
                // read the pixels into readOp's _colour array
                readRow(sizeS, pixelFormat, dataType, image->data(0,t,r), readOp);
                                
                // pass readOp's _colour array contents over to writeOp (note this is just a pointer swap).
                writeOp._colours.swap(readOp._colours);
                
                modifyRow(sizeS, pixelFormat, GL_UNSIGNED_BYTE, new_image->data(0,t,r), writeOp);

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
    REPLACE_ALPHA_WITH_LUMINACE
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

void doColourSpaceConversion(ColourSpaceOperation op, osg::Image* image, osg::Vec4& colour)
{
    switch(op)
    {
        case (MODULATE_ALPHA_BY_LUMINANCE):
            std::cout<<"doing conversion MODULATE_ALPHA_BY_LUMINANCE"<<std::endl;
            modifyImage(image,ModulateAlphaByLuminanceOperator()); 
            break;
        case (MODULATE_ALPHA_BY_COLOUR):
            std::cout<<"doing conversion MODULATE_ALPHA_BY_COLOUR"<<std::endl;
            modifyImage(image,ModulateAlphaByColourOperator(colour)); 
            break;
        case (REPLACE_ALPHA_WITH_LUMINACE):
            std::cout<<"doing conversion REPLACE_ALPHA_WITH_LUMINACE"<<std::endl;
            modifyImage(image,ReplaceAlphaWithLuminanceOperator()); 
            break;
        default:
            break;
    }
}

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is the example which demonstrates use of 3D textures.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    arguments.getApplicationUsage()->addCommandLineOption("-n","Create normal map for per voxel lighting.");
    arguments.getApplicationUsage()->addCommandLineOption("-s <numSlices>","Number of slices to create.");
    arguments.getApplicationUsage()->addCommandLineOption("--images [filenames]","Specify a stack of 2d images to build the 3d volume from.");
    arguments.getApplicationUsage()->addCommandLineOption("--shader","Use OpenGL Shading Language.");
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
    arguments.getApplicationUsage()->addCommandLineOption("--modulate-alpha-by-luminance","For each pixel multiple the alpha value by the luminance.");
    arguments.getApplicationUsage()->addCommandLineOption("--replace-alpha-with-luminance","For each pixel mSet the alpha value to the luminance.");
    arguments.getApplicationUsage()->addCommandLineOption("--num-components <num>","Set the number of components to in he target image.");
//    arguments.getApplicationUsage()->addCommandLineOption("--raw <sizeX> <sizeY> <sizeZ> <numberBytesPerComponent> <numberOfComponents> <endian> <filename>","read a raw image data");

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    std::string outputFile;
    while (arguments.read("-o",outputFile)) {}


    unsigned int numSlices=500;
    while (arguments.read("-s",numSlices)) {}
    
    
    float sliceEnd=1.0f;
    while (arguments.read("--clip",sliceEnd)) {}

    float alphaFunc=0.02f;
    while (arguments.read("--alphaFunc",alphaFunc)) {}


    bool createNormalMap = false;
    while (arguments.read("-n")) createNormalMap=true;

    float xSize=1.0f, ySize=1.0f, zSize=1.0f;
    while (arguments.read("--xSize",xSize)) {}
    while (arguments.read("--ySize",ySize)) {}
    while (arguments.read("--zSize",zSize)) {}

    float xMultiplier=1.0f, yMultiplier=1.0f, zMultiplier=1.0f;
    while (arguments.read("--xMultiplier",xMultiplier)) {}
    while (arguments.read("--yMultiplier",yMultiplier)) {}
    while (arguments.read("--zMultiplier",zMultiplier)) {}

    int s_maximumTextureSize = 256;
    int t_maximumTextureSize = 256;
    int r_maximumTextureSize = 256;
    int maximumTextureSize = 256;
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
    while(arguments.read("--replace-alpha-with-luminance")) { colourSpaceOperation = REPLACE_ALPHA_WITH_LUMINACE; }
        
    
    unsigned int numComponentsDesired = 0; 
    while(arguments.read("--num-components", numComponentsDesired)) {}

    bool useShader = false; 
    while(arguments.read("--shader")) { useShader = true; }

    osg::ref_ptr<osg::Image> image_3d;

    int sizeX, sizeY, sizeZ, numberBytesPerComponent, numberOfComponents;
    std::string endian, raw_filename;
    while (arguments.read("--raw", sizeX, sizeY, sizeZ, numberBytesPerComponent, numberOfComponents, endian, raw_filename)) 
    {
        image_3d = readRaw(sizeX, sizeY, sizeZ, numberBytesPerComponent, numberOfComponents, endian, raw_filename);
    }

    while (arguments.read("--images")) 
    {
        ImageList imageList;
        for(int pos=1;pos<arguments.argc() && !arguments.isOption(pos);++pos)
        {
            // not an option so assume string is a filename.
            osg::Image *image = osgDB::readImageFile( arguments[pos]);

            if(image)
            {
                imageList.push_back(image);
            }
        }
        
        // pack the textures into a single texture.
        ProcessRow processRow;
        image_3d = createTexture3D(imageList, processRow, numComponentsDesired, s_maximumTextureSize, t_maximumTextureSize, r_maximumTextureSize);
    }


    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }

    // assume remaining argments are file names of textures.
    for(int pos=1;pos<arguments.argc() && !image_3d;++pos)
    {
        if (!arguments.isOption(pos))
        {
            // not an option so assume string is a filename.
            image_3d = osgDB::readImageFile( arguments[pos]);
        }
    }
    
    if (!image_3d) return 0;
    
    if (colourSpaceOperation!=NO_COLOUR_SPACE_OPERATION)
    {
        doColourSpaceConversion(colourSpaceOperation, image_3d.get(), colourModulate);
    }
    
    osg::ref_ptr<osg::Image> normalmap_3d = createNormalMap ? createNormalMapTexture(image_3d.get()) : 0;



    // create a model from the images.
    osg::Node* rootNode = 0;
    
    if (useShader)
    {
        rootNode = createShaderModel(image_3d, normalmap_3d, 
                               internalFormatMode,
                               xSize, ySize, zSize,
                               xMultiplier, yMultiplier, zMultiplier,
                               numSlices, sliceEnd, alphaFunc);
    }
    else
    {
        rootNode = createModel(image_3d, normalmap_3d, 
                               internalFormatMode,
                               xSize, ySize, zSize,
                               xMultiplier, yMultiplier, zMultiplier,
                               numSlices, sliceEnd, alphaFunc);
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
            if (normalmap_3d.valid())
            {
                normalmap_3d->setFileName(name_no_ext + "_normalmap.dds");            
                osgDB::writeImageFile(*normalmap_3d, normalmap_3d->getFileName());
            }
            
            osgDB::writeNodeFile(*rootNode, outputFile);
        }
        else if (ext=="ive")
        {
            osgDB::writeNodeFile(*rootNode, outputFile);        
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


    if (rootNode) 
    {

        // set the scene to render
        viewer.setSceneData(rootNode);
        
        // create the windows and run the threads.
        viewer.realize();

        while( !viewer.done() )
        {
            // wait for all cull and draw threads to complete.
            viewer.sync();

            // update the scene by traversing it with the the update visitor which will
            // call all node update callbacks and animations.
            viewer.update();

            // fire off the cull and draw traversals of the scene.
            viewer.frame();

        }
        
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // run a clean up frame to delete all OpenGL objects.
        viewer.cleanup_frame();

        // wait for all the clean up frame to complete.
        viewer.sync();
    }    
    
    return 0;

}
