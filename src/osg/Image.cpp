#include <osg/GL>
#include <osg/GLU>

#include <osg/Image>
#include <osg/Notify>

#include <osg/Geode>
#include <osg/GeoSet>
#include <osg/StateSet>
#include <osg/Texture>


using namespace osg;

Image::Image()
{
    _fileName        = "";
    _s = _t = _r     = 0;
    _internalFormat  = 0;
    _pixelFormat     = (unsigned int)0;
    _dataType        = (unsigned int)0;
    _packing         = 4;

    _data = (unsigned char *)0L;

    _modifiedTag = 0;
}


Image::~Image()
{
    if (_data) ::free(_data);
}


void Image::setFileName(const std::string& fileName)
{
    _fileName = fileName;
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

    // test scaling...
    //    scaleImageTo(16,16,_r);
    
    ++_modifiedTag;

}

void Image::scaleImage(const int s,const int t,const int /*r*/)
{
    if (_data==NULL) return;

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

        notify(WARN) << "Error Image::scaleImage() do not succeed : errorString = "<<gluErrorString((GLenum)status)<<endl;
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
    
    
    if (new_s!=_s || new_t!=_t)
    {
        if (!_fileName.empty()) notify(NOTICE) << "Scaling image '"<<_fileName<<"' from ("<<_s<<","<<_t<<") to ("<<new_s<<","<<new_t<<")"<<endl;
        else notify(NOTICE) << "Scaling image from ("<<_s<<","<<_t<<") to ("<<new_s<<","<<new_t<<")"<<endl;

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
            osg::Texture* texture = new osg::Texture;
            texture->setImage(image);

            // set up the drawstate.
            osg::StateSet* dstate = new osg::StateSet;
            dstate->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
            dstate->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
            dstate->setAttributeAndModes(texture,osg::StateAttribute::ON);

            // set up the geoset.
            osg::GeoSet* gset = new osg::GeoSet;
            gset->setStateSet(dstate);

            osg::Vec3* coords = new Vec3 [4];
            coords[0].set(-x,0.0f,y);
            coords[1].set(-x,0.0f,-y);
            coords[2].set(x,0.0f,-y);
            coords[3].set(x,0.0f,y);
            gset->setCoords(coords);

            osg::Vec2* tcoords = new Vec2 [4];
            tcoords[0].set(0.0f,1.0f);
            tcoords[1].set(0.0f,0.0f);
            tcoords[2].set(1.0f,0.0f);
            tcoords[3].set(1.0f,1.0f);
            gset->setTextureCoords(tcoords);
            gset->setTextureBinding(osg::GeoSet::BIND_PERVERTEX);

            osg::Vec4* colours = new Vec4;
            colours->set(1.0f,1.0f,1.0,1.0f);
            gset->setColors(colours);
            gset->setColorBinding(osg::GeoSet::BIND_OVERALL);

            gset->setNumPrims(1);
            gset->setPrimType(osg::GeoSet::QUADS);

            // set up the geode.
            osg::Geode* geode = new osg::Geode;
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
