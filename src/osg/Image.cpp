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
    _fileName        = "";
    _s = _t = _r     = 0;
    _internalFormat  = 0;
    _pixelFormat     = (unsigned int)0;
    _dataType        = (unsigned int)0;
    _packing         = 4;
    _pixelSizeInBits = 0;

    _data = (unsigned char *)0L;

    _modifiedTag = 0;
}

Image::Image(const Image& image,const CopyOp& copyop):
    Object(image,copyop),
    _fileName(image._fileName),
    _s(image._s), _t(image._t), _r(image._r),
    _internalFormat(image._internalFormat),
    _pixelFormat(image._pixelFormat),
    _dataType(image._dataType),
    _packing(image._packing),
    _pixelSizeInBits(image._pixelSizeInBits),
    _data(0L),
    _modifiedTag(image._modifiedTag)
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


void Image::computePixelSize()
{


//     /* Data types */

//     #define GL_BITMAP				0x1A00

//     #define GL_BYTE					0x1400
//     #define GL_UNSIGNED_BYTE			0x1401
//     #define GL_SHORT				0x1402
//     #define GL_UNSIGNED_SHORT			0x1403
//     #define GL_INT					0x1404
//     #define GL_UNSIGNED_INT				0x1405
//     #define GL_FLOAT				0x1406
//     #define GL_DOUBLE				0x140A
//     #define GL_2_BYTES				0x1407
//     #define GL_3_BYTES				0x1408
//     #define GL_4_BYTES				0x1409
// 
#ifndef GL_VERSION_1_1
    /* Internal texture formats (GL 1.1) */
    #define GL_ALPHA4				0x803B
    #define GL_ALPHA8				0x803C
    #define GL_ALPHA12				0x803D
    #define GL_ALPHA16				0x803E
    #define GL_LUMINANCE4				0x803F
    #define GL_LUMINANCE8				0x8040
    #define GL_LUMINANCE12				0x8041
    #define GL_LUMINANCE16				0x8042
    #define GL_LUMINANCE4_ALPHA4			0x8043
    #define GL_LUMINANCE6_ALPHA2			0x8044
    #define GL_LUMINANCE8_ALPHA8			0x8045
    #define GL_LUMINANCE12_ALPHA4			0x8046
    #define GL_LUMINANCE12_ALPHA12			0x8047
    #define GL_LUMINANCE16_ALPHA16			0x8048
    #define GL_INTENSITY				0x8049
    #define GL_INTENSITY4				0x804A
    #define GL_INTENSITY8				0x804B
    #define GL_INTENSITY12				0x804C
    #define GL_INTENSITY16				0x804D
    #define GL_R3_G3_B2				0x2A10
    #define GL_RGB4					0x804F
    #define GL_RGB5					0x8050
    #define GL_RGB8					0x8051
    #define GL_RGB10				0x8052
    #define GL_RGB12				0x8053
    #define GL_RGB16				0x8054
    #define GL_RGBA2				0x8055
    #define GL_RGBA4				0x8056
    #define GL_RGB5_A1				0x8057
    #define GL_RGBA8				0x8058
    #define GL_RGB10_A2				0x8059
    #define GL_RGBA12				0x805A
    #define GL_RGBA16				0x805B
#endif

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

    // temporary code.. to be filled in properly ASAP. RO.
    _pixelSizeInBits=32;
}

void Image::setImage(const int s,const int t,const int r,
                     const int internalFormat,
                     const unsigned int pixelFormat,
                     const unsigned int dataType,
                     unsigned char *data,
                     const int packing)
{
    if (_data) ::free(_data);

    _s = s;
    _t = t;
    _r = r;

    _internalFormat = internalFormat;
    _pixelFormat    = pixelFormat;
    _dataType       = dataType;

    _data = data;

    if (packing<0)
    {
        if (_s%4==0)
            _packing = 4;
        else
            _packing = 1;
    }
    else
        _packing = packing;
        
    computePixelSize();

    // test scaling...
    //    scaleImageTo(16,16,_r);
    
    ++_modifiedTag;

}

void Image::readPixels(int x,int y,int width,int height,
                       unsigned int pixelFormat,unsigned int dataType,
                       const int packing)
{
    
    if (width!=_s || height!=_s || _t != 1 ||
        _pixelFormat!=pixelFormat ||
        _dataType!=dataType)
    {

        if (_data) ::free(_data);

        _s = width;
        _t = height;
        _r = 1;
        _pixelFormat = pixelFormat;
        _dataType = dataType;

        if (packing<0)
        {
            if (_s%4==0)
                _packing = 4;
            else
                _packing = 1;
        }
        else
            _packing = packing;

        computePixelSize();

        // need to sort out what size to really use...
        _data = (unsigned char *)malloc(2 * (_s+1)*(_t+1)*4);
        
    }

    glPixelStorei(GL_PACK_ALIGNMENT,_packing);

    glReadPixels(x,y,width,height,pixelFormat,dataType,_data);
}


void Image::scaleImage(const int s,const int t,const int /*r*/)
{
    if (_data==NULL) return;

    // need to sort out what size to really use...
    unsigned char* newData = (unsigned char *)malloc(2 * (s+1)*(t+1)*4);

    glPixelStorei(GL_PACK_ALIGNMENT,_packing);
    glPixelStorei(GL_UNPACK_ALIGNMENT,_packing);

    GLint status = gluScaleImage((GLenum)_pixelFormat,
        _s,
        _t,
        (GLenum)_dataType,
        _data,
        s,
        t,
        (GLenum)_dataType,
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


void Image::ensureDimensionsArePowerOfTwo()
{
    float sp2 = logf((float)_s)/logf(2.0f);
    float rounded_sp2 = floorf(sp2+0.5f);
    int new_s = (int)(powf(2.0f,rounded_sp2));

    float tp2 = logf((float)_t)/logf(2.0f);
    float rounded_tp2 = floorf(tp2+0.5f);
    int new_t = (int)(powf(2.0f,rounded_tp2));
    
    static GLint max_size=256;

    static bool init = true;
    if (init)
    {
        init = false;
        glGetIntegerv(GL_MAX_TEXTURE_SIZE,&max_size);
        notify(INFO) << "Max texture size "<<max_size<<std::endl;
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
