#include <osg/GL>
#include <osg/GLU>

#include <osg/Image>
#include <osg/Notify>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/StateSet>
#include <osg/Texture2D>


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


bool Image::isPackedType(GLenum type)
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

unsigned int Image::computeNumComponents(GLenum format)
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


unsigned int Image::computePixelSizeInBits(GLenum format,GLenum type)
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

unsigned int Image::computeRowWidthInBytes(int width,GLenum format,GLenum type,int packing)
{
    unsigned int pixelSize = computePixelSizeInBits(format,type);
    int widthInBits = width*pixelSize;
    int packingInBits = packing*8;
    return (widthInBits/packingInBits + ((widthInBits%packingInBits)?1:0))*packing;
}

unsigned int Image::computeNearestPowerOfTwo(unsigned int s,float bias)
{
    if ((s & (s-1))!=0)
    {
        // it isn't so lets find the closest power of two.
        // yes, logf and powf are slow, but this code should
        // only be called during scene graph initilization,
        // if at all, so not critical in the greater scheme.
        float p2 = logf((float)s)/logf(2.0f);
        float rounded_p2 = floorf(p2+bias);
        s = (int)(powf(2.0f,rounded_p2));
    }
    return s;
}

void Image::setInternalTextureFormat(GLint internalFormat)
{
    // won't do any sanity checking right now, leave it to 
    // OpenGL to make the call.
    _internalTextureFormat = internalFormat;
}

void Image::setPixelFormat(GLenum format)
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

void Image::allocateImage(int s,int t,int r,
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
    allocateImage(width,height,1,format,type);

    glPixelStorei(GL_PACK_ALIGNMENT,_packing);

    glReadPixels(x,y,width,height,format,type,_data);
}


void Image::scaleImage(int s,int t,int r)
{
    if (_s==s && _t==t && _r==r) return;

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

void Image::copySubImage(int s_offset,int t_offset,int r_offset,osg::Image* source)
{
    if (!source) return;
    if (s_offset<0 || t_offset<0 || r_offset<0) 
    {
        notify(WARN)<<"Warning: negative offsets passed to Image::copySubImage(..) not supported, operation ignored."<<std::endl;
        return;
    }
    
    if (!_data)
    {
        cout<<"allocating image"<<endl;
        allocateImage(s_offset+source->r(),t_offset+source->t(),r_offset+source->t(),
                    source->getPixelFormat(),source->getDataType(),
                    source->getPacking());
    }
    
    if (s_offset>=_s || t_offset>=_t  || r_offset>=_r)
    {
        notify(WARN)<<"Warning: offsets passed to Image::copySubImage(..) outside destination image, operation ignored."<<std::endl;
        return;
    }
    
    
    if (_pixelFormat != source->getPixelFormat())
    {
        notify(WARN)<<"Warning: image with an incompatible pixel formats passed to Image::copySubImage(..), operation ignored."<<std::endl;
        return;
    }

    void* data_destination = data(s_offset,t_offset,r_offset);
    
    glPixelStorei(GL_PACK_ALIGNMENT,source->getPacking());
    glPixelStorei(GL_PACK_ROW_LENGTH,_s);

    glPixelStorei(GL_UNPACK_ALIGNMENT,_packing);
    
    GLint status = gluScaleImage(_pixelFormat,
        source->s(),
        source->t(),
        source->getDataType(),
        source->data(),
        source->s(),
        source->t(),
        _dataType,
        data_destination);

    glPixelStorei(GL_PACK_ROW_LENGTH,0);

    if (status!=0)
    {
        notify(WARN) << "Error Image::scaleImage() do not succeed : errorString = "<<gluErrorString((GLenum)status)<<std::endl;
    }

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
    int new_s = computeNearestPowerOfTwo(_s);
    int new_t = computeNearestPowerOfTwo(_t);
    
    static GLint max_size=Texture::getMaxTextureSize();
    
    if (new_s>max_size) new_s = max_size;
    if (new_t>max_size) new_t = max_size;
    
    if (new_s!=_s || new_t!=_t)
    {
        if (!_fileName.empty()) notify(NOTICE) << "Scaling image '"<<_fileName<<"' from ("<<_s<<","<<_t<<") to ("<<new_s<<","<<new_t<<")"<<std::endl;
        else notify(NOTICE) << "Scaling image from ("<<_s<<","<<_t<<") to ("<<new_s<<","<<new_t<<")"<<std::endl;

        scaleImage(new_s,new_t,_r);
    }
}

void Image::computeMipMaps()
{
}


///////////////////////////////////////////////////////////////////////////////


Geode* osg::createGeodeForImage(osg::Image* image)
{
    return createGeodeForImage(image,image->s(),image->t());
}


Geode* osg::createGeodeForImage(osg::Image* image,float s,float t)
{
    if (image)
    {
        if (s>0 && t>0)
        {

            float y = 1.0;
            float x = y*(s/t);

            // set up the texture.
            osg::Texture2D* texture = osgNew osg::Texture2D;
            texture->setImage(image);

            // set up the drawstate.
            osg::StateSet* dstate = osgNew osg::StateSet;
            dstate->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
            dstate->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
            dstate->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON);

            // set up the geoset.
            Geometry* geom = osgNew Geometry;
            geom->setStateSet(dstate);

            Vec3Array* coords = osgNew Vec3Array(4);
            (*coords)[0].set(-x,0.0f,y);
            (*coords)[1].set(-x,0.0f,-y);
            (*coords)[2].set(x,0.0f,-y);
            (*coords)[3].set(x,0.0f,y);
            geom->setVertexArray(coords);

            Vec2Array* tcoords = osgNew Vec2Array(4);
            (*tcoords)[0].set(0.0f,1.0f);
            (*tcoords)[1].set(0.0f,0.0f);
            (*tcoords)[2].set(1.0f,0.0f);
            (*tcoords)[3].set(1.0f,1.0f);
            geom->setTexCoordArray(0,tcoords);

            osg::Vec4Array* colours = osgNew osg::Vec4Array(1);
            (*colours)[0].set(1.0f,1.0f,1.0,1.0f);
            geom->setColorArray(colours);
            geom->setColorBinding(Geometry::BIND_OVERALL);

            geom->addPrimitive(osgNew DrawArrays(Primitive::QUADS,0,4));

            // set up the geode.
            osg::Geode* geode = osgNew osg::Geode;
            geode->addDrawable(geom);

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

