#include "osg/Texture"
#include "osg/Input"
#include "osg/Output"
#include "osg/Registry"
#include "osg/Image"
#include "osg/Referenced"
#include "osg/Notify"

using namespace osg;

Texture::Texture()
{
    _handle        = 0;

    _wrap_s    = CLAMP;
    _wrap_t    = CLAMP;
    _wrap_r    = CLAMP;
    _min_filter    = NEAREST_MIPMAP_LINEAR;
    _mag_filter    = LINEAR;
}


Texture::~Texture()
{
    if (_handle!=0) glDeleteTextures( 1, &_handle );
}


Texture* Texture::instance()
{
    static ref_ptr<Texture> s_texture(new Texture);
    return s_texture.get();
}


bool Texture::readLocalData(Input& fr)
{
    bool iteratorAdvanced = false;
    if (fr[0].matchWord("file") && fr[1].isString())
    {
        _image = fr.readImage(fr[1].getStr());
        fr += 2;
        iteratorAdvanced = true;
    }
    WrapMode wrap;
    if (fr[0].matchWord("wrap_s") && matchWrapStr(fr[1].getStr(),wrap))
    {
        _wrap_s = wrap;
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("wrap_t") && matchWrapStr(fr[1].getStr(),wrap))
    {
        _wrap_t = wrap;
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("wrap_r") && matchWrapStr(fr[1].getStr(),wrap))
    {
        _wrap_r = wrap;
        fr+=2;
        iteratorAdvanced = true;
    }

    FilterMode filter;
    if (fr[0].matchWord("min_filter") && matchFilterStr(fr[1].getStr(),filter))
    {
        _min_filter = filter;
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("mag_filter") && matchFilterStr(fr[1].getStr(),filter))
    {
        _mag_filter = filter;
        fr+=2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool Texture::writeLocalData(Output& fw)
{
    if (_image.valid())
    {
        fw.indent() << "file \""<<_image->getFileName()<<"\""<<endl;
    }

    fw.indent() << "wrap_s " << getWrapStr(_wrap_s) << endl;
    fw.indent() << "wrap_t " << getWrapStr(_wrap_t) << endl;
    fw.indent() << "wrap_r " << getWrapStr(_wrap_r) << endl;

    fw.indent() << "min_filter " << getFilterStr(_min_filter) << endl;
    fw.indent() << "mag_filter " << getFilterStr(_mag_filter) << endl;

    return true;
}


bool Texture::matchWrapStr(const char* str,WrapMode& wrap)
{
    if (strcmp(str,"CLAMP")==0) wrap = CLAMP;
    else if (strcmp(str,"REPEAT")==0) wrap = REPEAT;
    else return false;
    return true;
}


const char* Texture::getWrapStr(WrapMode wrap)
{
    switch(wrap)
    {
        case(CLAMP): return "CLAMP";
        case(REPEAT): return "REPEAT";
    }
    return "";
}


bool Texture::matchFilterStr(const char* str,FilterMode& filter)
{
    if (strcmp(str,"NEAREST")==0) filter = NEAREST;
    else if (strcmp(str,"LINEAR")==0) filter = LINEAR;
    else if (strcmp(str,"NEAREST_MIPMAP_NEAREST")==0) filter = NEAREST_MIPMAP_NEAREST;
    else if (strcmp(str,"LINEAR_MIPMAP_NEAREST")==0) filter = LINEAR_MIPMAP_NEAREST;
    else if (strcmp(str,"NEAREST_MIPMAP_LINEAR")==0) filter = NEAREST_MIPMAP_LINEAR;
    else if (strcmp(str,"LINEAR_MIPMAP_LINEAR")==0) filter = LINEAR_MIPMAP_LINEAR;
    else return false;
    return true;
}


const char* Texture::getFilterStr(FilterMode filter)
{
    switch(filter)
    {
        case(NEAREST): return "NEAREST";
        case(LINEAR): return "LINEAR";
        case(NEAREST_MIPMAP_NEAREST): return "NEAREST_MIPMAP_NEAREST";
        case(LINEAR_MIPMAP_NEAREST): return "LINEAR_MIPMAP_NEAREST";
        case(NEAREST_MIPMAP_LINEAR): return "NEAREST_MIPMAP_LINEAR";
        case(LINEAR_MIPMAP_LINEAR): return "LINEAR_MIPMAP_LINEAR";
    }
    return "";
}

void Texture::setImage(Image* image)
{
    if (_handle!=0) glDeleteTextures( 1, &_handle );
    _handle = 0;
    _image = image; 
}

void Texture::setWrap(WrapParameter which, WrapMode wrap)
{
    switch( which )
    {
        case WRAP_S : _wrap_s = wrap; break;
        case WRAP_T : _wrap_t = wrap; break;
        case WRAP_R : _wrap_r = wrap; break;
        default : notify(WARN)<<"Error: invalid 'which' passed Texture::setWrap("<<(unsigned int)which<<","<<(unsigned int)wrap<<")"<<endl; break;
    }
}

Texture::WrapMode Texture::getWrap(WrapParameter which) const
{
    switch( which )
    {
        case WRAP_S : return _wrap_s;
        case WRAP_T : return _wrap_t;
        case WRAP_R : return _wrap_r;
        default : notify(WARN)<<"Error: invalid 'which' passed Texture::getWrap(which)"<<endl; return _wrap_s;
    }
}


void Texture::setFilter(FilterParameter which, FilterMode filter)
{
    switch( which )
    {
        case MIN_FILTER : _min_filter = filter; break;
        case MAG_FILTER : _mag_filter = filter; break;
        default : notify(WARN)<<"Error: invalid 'which' passed Texture::setFilter("<<(unsigned int)which<<","<<(unsigned int)filter<<")"<<endl; break;
    }
}


Texture::FilterMode Texture::getFilter(FilterParameter which) const
{
    switch( which )
    {
        case MIN_FILTER : return _min_filter;
        case MAG_FILTER : return _mag_filter;
        default : notify(WARN)<<"Error: invalid 'which' passed Texture::getFilter(which)"<<endl; return _min_filter;
    }
}

void Texture::enable( void )
{
    glEnable( GL_TEXTURE_2D );
}


void Texture::disable( void )
{
    glDisable( GL_TEXTURE_2D );
}


void Texture::apply( void )
{
    if(_handle!=0)
    {
        glBindTexture( GL_TEXTURE_2D, _handle );
    }
    else if (_image.valid())
    {

        _image->ensureDimensionsArePowerOfTwo();
        
        glPixelStorei(GL_UNPACK_ALIGNMENT,_image->packing());


        glGenTextures( 1, &_handle );
        glBindTexture( GL_TEXTURE_2D, _handle );

        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, _wrap_s );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, _wrap_t );
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, _min_filter);
        glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, _mag_filter);
        if( _min_filter == LINEAR || _min_filter == NEAREST )
        {


            glTexImage2D( GL_TEXTURE_2D, 0, _image->internalFormat(),
                    _image->s(), _image->t(), 0,
                    (GLenum)_image->pixelFormat(),
                    (GLenum)_image->dataType(),
                    _image->data() );
        }
        else
        {

            gluBuild2DMipmaps( GL_TEXTURE_2D, _image->internalFormat(),
                _image->s(),_image->t(),
                (GLenum)_image->pixelFormat(), (GLenum)_image->dataType(),
                _image->data() );
        }

        glBindTexture( GL_TEXTURE_2D, _handle );

    }
}
