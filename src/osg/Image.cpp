#include <osg/GL>
#include <osg/GLU>

#include <osg/Image>
#include <osg/Notify>

#include <osg/Geode>
#include <osg/GeoSet>
#include <osg/StateSet>
#include <osg/Texture>


using namespace osg;
using namespace std;

Image::Image()
{
    _fileName               = "";
    _s = _t = _r            = 0;
    _internalTextureFormat  = 0;
    _pixelFormat            = (unsigned int)0;
    _dataType               = (unsigned int)0;
    _packing                = 4;

    _data = (unsigned char *)0L;

    _modifiedTag = 0;
}

Image::Image(const Image& image,const CopyOp& copyop):
    Object(image,copyop),
    _fileName(image._fileName),
    _s(image._s), _t(image._t), _r(image._r),
    _internalTextureFormat(image._internalTextureFormat),
    _pixelFormat(image._pixelFormat),
    _dataType(image._dataType),
    _packing(image._packing),
    _data(0L),
    _modifiedTag(image._modifiedTag),
    _mipmapData(image._mipmapData)
{
    if (image._data)
    {
        int num_components = 
            _pixelFormat == GL_LUMINANCE ? 1 :
            _pixelFormat == GL_LUMINANCE_ALPHA ? 2 :
            _pixelFormat == GL_RGB ? 3 :
            _pixelFormat == GL_RGBA ? 4 : 4;

        int size = _s*_t*_r*num_components;
        _data = (unsigned char*) malloc(size);
        memcpy(_data,image._data,size);
    }

}

Image::~Image()
{
    if (_data) ::free(_data);
}


void Image::setFileName(const std::string& fileName)
{
    _fileName = fileName;
}

#ifndef GL_VERSION_1_2
    /* 1.2 definitions...*/
    #define GL_BGR					0x80E0
    #define GL_BGRA					0x80E1
    #define GL_UNSIGNED_BYTE_3_3_2			0x8032
    #define GL_UNSIGNED_BYTE_2_3_3_REV		0x8362
    #define GL_UNSIGNED_SHORT_5_6_5			0x8363
    #define GL_UNSIGNED_SHORT_5_6_5_REV		0x8364
    #define GL_UNSIGNED_SHORT_4_4_4_4		0x8033
    #define GL_UNSIGNED_SHORT_4_4_4_4_REV		0x8365
    #define GL_UNSIGNED_SHORT_5_5_5_1		0x8034
    #define GL_UNSIGNED_SHORT_1_5_5_5_REV		0x8366
    #define GL_UNSIGNED_INT_8_8_8_8			0x8035
    #define GL_UNSIGNED_INT_8_8_8_8_REV		0x8367
    #define GL_UNSIGNED_INT_10_10_10_2		0x8036
    #define GL_UNSIGNED_INT_2_10_10_10_REV		0x8368
#endif

const bool Image::isPackedType(GLenum type)
{
    switch(type)
    {
        case(GL_UNSIGNED_BYTE_3_3_2):
        case(GL_UNSIGNED_BYTE_2_3_3_REV):
        case(GL_UNSIGNED_SHORT_5_6_5):
        case(GL_UNSIGNED_SHORT_5_6_5_REV):
        case(GL_UNSIGNED_SHORT_4_4_4_4):
        case(GL_UNSIGNED_SHORT_4_4_4_4_REV):
        case(GL_UNSIGNED_SHORT_5_5_5_1):
        case(GL_UNSIGNED_SHORT_1_5_5_5_REV):
        case(GL_UNSIGNED_INT_8_8_8_8):
        case(GL_UNSIGNED_INT_8_8_8_8_REV):
        case(GL_UNSIGNED_INT_10_10_10_2):
        case(GL_UNSIGNED_INT_2_10_10_10_REV): return true;
        default: return false;
    }    
}

const unsigned int Image::computeNumComponents(GLenum format)
{
    switch(format)
    {
        case(GL_COLOR_INDEX): return 1;
        case(GL_STENCIL_INDEX): return 1;
        case(GL_DEPTH_COMPONENT): return 1;
        case(GL_RED): return 1;
        case(GL_GREEN): return 1;
        case(GL_BLUE): return 1;
        case(GL_ALPHA): return 1;
        case(GL_RGB): return 3;
        case(GL_BGR): return 3;
        case(GL_RGBA): return 4;
        case(GL_BGRA): return 4;
        case(GL_LUMINANCE): return 1;
        case(GL_LUMINANCE_ALPHA): return 2;
        default: return 0;
    }        
}


const unsigned int Image::computePixelSizeInBits(GLenum format,GLenum type)
{
    switch(type)
    {
        case(GL_BITMAP): return computeNumComponents(format);
        
        case(GL_BYTE):
        case(GL_UNSIGNED_BYTE): return 8*computeNumComponents(format);
        
        case(GL_SHORT):
        case(GL_UNSIGNED_SHORT): return 16*computeNumComponents(format);
        
        case(GL_INT):
        case(GL_UNSIGNED_INT):
        case(GL_FLOAT): return 32*computeNumComponents(format);
    
    
        case(GL_UNSIGNED_BYTE_3_3_2): 
        case(GL_UNSIGNED_BYTE_2_3_3_REV): return 8;
        
        case(GL_UNSIGNED_SHORT_5_6_5):
        case(GL_UNSIGNED_SHORT_5_6_5_REV):
        case(GL_UNSIGNED_SHORT_4_4_4_4):
        case(GL_UNSIGNED_SHORT_4_4_4_4_REV):
        case(GL_UNSIGNED_SHORT_5_5_5_1):
        case(GL_UNSIGNED_SHORT_1_5_5_5_REV): return 16;
        
        case(GL_UNSIGNED_INT_8_8_8_8):
        case(GL_UNSIGNED_INT_8_8_8_8_REV):
        case(GL_UNSIGNED_INT_10_10_10_2):
        case(GL_UNSIGNED_INT_2_10_10_10_REV): return 32;
        default: return 0;
    }    

}

const unsigned int Image::computeRowWidthInBytes(int width,GLenum format,GLenum type,int packing)
{
    unsigned int pixelSize = computePixelSizeInBits(format,type);
    int widthInBits = width*pixelSize;
    int packingInBits = packing*8;
    return (widthInBits/packingInBits + ((widthInBits%packingInBits)?1:0))*packing;
}


void Image::setInternalTextureFormat(GLint internalFormat)
{
    // won't do any sanity checking right now, leave it to 
    // OpenGL to make the call.
    _internalTextureFormat = internalFormat;
}

void Image::setPixelFormat(const GLenum format)
{
    if (_pixelFormat==format) return; // do nothing if the same.

    if (computeNumComponents(_pixelFormat)==computeNumComponents(format))
    {
       // if the two formats have the same number of componets then
       // we can do a straight swap.
        _pixelFormat = format;
    }
    else
    {
        notify(WARN)<<"Image::setPixelFormat(..) - warning, attempt to reset the pixel format with a different number of components."<<std::endl;
    }
}

void Image::createImage(int s,int t,int r,
                        GLenum format,GLenum type,
                        int packing)
{
    _mipmapData.clear();

    unsigned int previousTotalSize = computeRowWidthInBytes(_s,_pixelFormat,_dataType,_packing)*_t*_r;
    
    unsigned int newTotalSize = computeRowWidthInBytes(s,format,type,packing)*t*r;

    if (newTotalSize!=previousTotalSize)
    {
        if (_data) ::free(_data);
        
        if (newTotalSize)
            _data = (unsigned char *)malloc (newTotalSize);
        else
            _data = 0L;
    }

    if (_data)
    {
        _s = s;
        _t = t;
        _r = r;
        _pixelFormat = format;
        _dataType = type;
        _packing = packing;

    }
    else
    {
        // throw exception?? not for now, will simply set values to 0.
        _s = 0;
        _t = 0;
        _r = 0;
        _pixelFormat = 0;
        _dataType = 0;
        _packing = 0;
    }
    
    ++_modifiedTag;
}

void Image::setImage(int s,int t,int r,
                     GLint internalTextureFormat,
                     GLenum format,GLenum type,
                     unsigned char *data,
                     int packing)
{
    if (_data) ::free(_data);
    _mipmapData.clear();

    _s = s;
    _t = t;
    _r = r;

    _internalTextureFormat = internalTextureFormat;
    _pixelFormat    = format;
    _dataType       = type;

    _data = data;
    _packing = packing;
        
    // test scaling...
    //    scaleImageTo(16,16,_r);
    
    ++_modifiedTag;

}

void Image::readPixels(int x,int y,int width,int height,
                       GLenum format,GLenum type)
{
    createImage(width,height,1,format,type);

    glPixelStorei(GL_PACK_ALIGNMENT,_packing);

    glReadPixels(x,y,width,height,format,type,_data);
}


void Image::scaleImage(const int s,const int t,const int r)
{
    if (_data==NULL)
    {
        notify(WARN) << "Error Image::scaleImage() do not succeed : cannot scale NULL image."<<std::endl;
        return;
    }

    if (_r!=1 || r!=1)
    {
        notify(WARN) << "Error Image::scaleImage() do not succeed : scaling of volumes not implemented."<<std::endl;
        return;
    }
    

    unsigned int newTotalSize = computeRowWidthInBytes(s,_pixelFormat,_dataType,_packing)*t;

    // need to sort out what size to really use...
    unsigned char* newData = (unsigned char *)malloc(newTotalSize);
    if (!newData)
    {
        // should we throw an exception???  Just return for time being.
        notify(FATAL) << "Error Image::scaleImage() do not succeed : out of memory."<<newTotalSize<<std::endl;
        return;
    }

    glPixelStorei(GL_PACK_ALIGNMENT,_packing);
    glPixelStorei(GL_UNPACK_ALIGNMENT,_packing);

    GLint status = gluScaleImage(_pixelFormat,
        _s,
        _t,
        _dataType,
        _data,
        s,
        t,
        _dataType,
        newData);

    if (status==0)
    {

        // free old image.
        ::free(_data);

        _s = s;
        _t = t;
        _data = newData;
    }
    else
    {
        ::free(newData);

        notify(WARN) << "Error Image::scaleImage() do not succeed : errorString = "<<gluErrorString((GLenum)status)<<std::endl;
    }
    
    ++_modifiedTag;
}

void Image::flipHorizontal(int image)
{
    if (_data==NULL)
    {
        notify(WARN) << "Error Image::flipHorizontal() do not succeed : cannot flip NULL image."<<std::endl;
        return;
    }

    unsigned int elemSize = getPixelSizeInBits()/8;

    for (int t=0; t<_t; ++t)
    {
        unsigned char* rowData = _data+t*getRowSizeInBytes()+image*getImageSizeInBytes();
        unsigned char* left  = rowData ;
        unsigned char* right = rowData + ((_s-1)*getPixelSizeInBits())/8;

        while (left < right)
        {
            char tmp[32];  // max elem size is four floats
            memcpy(tmp, left, elemSize);
            memcpy(left, right, elemSize);
            memcpy(right, tmp, elemSize);
            left  += elemSize;
            right -= elemSize;
        }
    }

    ++_modifiedTag;
}


void Image::flipVertical(int image)
{
    if (_data==NULL)
    {
        notify(WARN) << "Error Image::flipVertical() do not succeed : cannot flip NULL image."<<std::endl;
        return;
    }

    unsigned int rowSizeInBytes = getRowSizeInBytes();
    unsigned int imageSizeInBytes = getImageSizeInBytes();
    unsigned char* imageData = _data+image*imageSizeInBytes;

    // make temp. buffer for one image
    unsigned char *tmpData = (unsigned char*) osgMalloc(imageSizeInBytes);

    for (int t=0; t<_t; ++t)
    {
        unsigned char* srcRowData = imageData+t*rowSizeInBytes;
        unsigned char* dstRowData = tmpData+(_t-1-t)*rowSizeInBytes;
        memcpy(dstRowData, srcRowData, rowSizeInBytes);
    }

    // insert fliped image
    memcpy(imageData, tmpData, imageSizeInBytes);

    osgFree(tmpData);
    ++_modifiedTag;
}



void Image::ensureValidSizeForTexturing()
{
    int new_s = _s;
    int new_t = _t;
    
    // check if _s is a power of 2 already.
    if ((_s & (_s-1))!=0)
    {
        // it isn't so lets find the closest power of two.
        // yes, logf and powf are slow, but this code should
        // only be called during scene graph initilization,
        // if at all, so not critical in the greater scheme.
        float p2 = logf((float)_s)/logf(2.0f);
        float rounded_p2 = floorf(p2+0.5f);
        new_s = (int)(powf(2.0f,rounded_p2));
    }

    if ((_t & (_t-1))!=0)
    {
        // it isn't so lets find the closest power of two.
        // yes, logf and powf are slow, but this code should
        // only be called during scene graph initilization,
        // if at all, so not critical in the greater scheme.
        float p2 = logf((float)_t)/logf(2.0f);
        float rounded_p2 = floorf(p2+0.5f);
        new_t = (int)(powf(2.0f,rounded_p2));
    }
    
    static GLint max_size=256;

    static bool init = true;
    if (init)
    {
        init = false;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max_size);
        notify(INFO) << "GL_MAX_TEXTURE_SIZE "<<max_size<<std::endl;
        
        char *ptr;
        if( (ptr = getenv("OSG_MAX_TEXTURE_SIZE")) != 0)
        {
            GLint osg_max_size = atoi(ptr);
            
            notify(INFO) << "OSG_MAX_TEXTURE_SIZE "<<osg_max_size<<std::endl;
            
            if (osg_max_size<max_size)
            {
                
                max_size = osg_max_size;
            }
            
        }      
        notify(INFO) << "Selected max texture size "<<max_size<<std::endl;
    }
    //max_size = 64;
    
    if (new_s>max_size) new_s = max_size;
    if (new_t>max_size) new_t = max_size;
    
    if (new_s!=_s || new_t!=_t)
    {
        if (!_fileName.empty()) notify(NOTICE) << "Scaling image '"<<_fileName<<"' from ("<<_s<<","<<_t<<") to ("<<new_s<<","<<new_t<<")"<<std::endl;
        else notify(NOTICE) << "Scaling image from ("<<_s<<","<<_t<<") to ("<<new_s<<","<<new_t<<")"<<std::endl;

        scaleImage(new_s,new_t,_r);
    }
}


Geode* osg::createGeodeForImage(osg::Image* image)
{
    return createGeodeForImage(image,image->s(),image->t());
}


Geode* osg::createGeodeForImage(osg::Image* image,const float s,const float t)
{
    if (image)
    {
        if (s>0 && t>0)
        {

            float y = 1.0;
            float x = y*(s/t);

            // set up the texture.
            osg::Texture* texture = osgNew osg::Texture;
            texture->setImage(image);

            // set up the drawstate.
            osg::StateSet* dstate = osgNew osg::StateSet;
            dstate->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
            dstate->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
            dstate->setAttributeAndModes(texture,osg::StateAttribute::ON);

            // set up the geoset.
            osg::GeoSet* gset = osgNew osg::GeoSet;
            gset->setStateSet(dstate);

            osg::Vec3* coords = osgNew Vec3[4];
            coords[0].set(-x,0.0f,y);
            coords[1].set(-x,0.0f,-y);
            coords[2].set(x,0.0f,-y);
            coords[3].set(x,0.0f,y);
            gset->setCoords(coords);

            osg::Vec2* tcoords = osgNew Vec2[4];
            tcoords[0].set(0.0f,1.0f);
            tcoords[1].set(0.0f,0.0f);
            tcoords[2].set(1.0f,0.0f);
            tcoords[3].set(1.0f,1.0f);
            gset->setTextureCoords(tcoords);
            gset->setTextureBinding(osg::GeoSet::BIND_PERVERTEX);

            osg::Vec4* colours = osgNew Vec4[1];
            colours->set(1.0f,1.0f,1.0,1.0f);
            gset->setColors(colours);
            gset->setColorBinding(osg::GeoSet::BIND_OVERALL);

            gset->setNumPrims(1);
            gset->setPrimType(osg::GeoSet::QUADS);

            // set up the geode.
            osg::Geode* geode = osgNew osg::Geode;
            geode->addDrawable(gset);

            return geode;

        }
        else
        {
            return NULL;
        }
    }
    else
    {
        return NULL;
    }
}
