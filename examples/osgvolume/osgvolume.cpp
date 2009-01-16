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
#include <osgVolume/ShaderTechnique>
#include <osgVolume/FixedFunctionTechnique>

typedef std::vector< osg::ref_ptr<osg::Image> > ImageList;

enum ShadingModel
{
    Standard,
    Light,
    Isosurface,
    MaximumIntensityProjection
};

//  example ReadOperator
// struct ReadOperator
// {
//     inline void luminance(float l) const { rgba(l,l,l,1.0f); }
//     inline void alpha(float a) const { rgba(1.0f,1.0f,1.0f,a); }
//     inline void luminance_alpha(float l,float a) const { rgba(l,l,l,a); }
//     inline void rgb(float r,float g,float b) const { rgba(r,g,b,1.0f); }
//     inline void rgba(float r,float g,float b,float a) const { std::cout<<"pixel("<<r<<", "<<g<<", "<<b<<", "<<a<<")"<<std::endl; }
// };



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


osg::Image* createNormalMapTexture(osg::Image* image_3d)
{
    osg::notify(osg::NOTICE)<<"Computing NormalMapTexture"<<std::endl;

    GLenum dataType = image_3d->getDataType();

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

            if (dataType==GL_UNSIGNED_BYTE)
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
            else if (dataType==GL_SHORT)
            {
                short* ptr = (short*)(image_3d->data(1,t,r)+alphaOffset);
                short* left = (short*)(image_3d->data(0,t,r)+alphaOffset);
                short* right = (short*)(image_3d->data(2,t,r)+alphaOffset);
                short* above = (short*)(image_3d->data(1,t+1,r)+alphaOffset);
                short* below = (short*)(image_3d->data(1,t-1,r)+alphaOffset);
                short* in = (short*)(image_3d->data(1,t,r+1)+alphaOffset);
                short* out = (short*)(image_3d->data(1,t,r-1)+alphaOffset);

                unsigned char* destination = (unsigned char*) normalmap_3d->data(1,t,r);

                for(int s=1;s<image_3d->s()-1;++s)
                {

                    osg::Vec3 grad((float)(*left)-(float)(*right),
                                   (float)(*below)-(float)(*above),
                                   (float)(*out) -(float)(*in));

                    grad.normalize();

                    //osg::notify(osg::NOTICE)<<"normal "<<grad<<std::endl;

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

                    *destination++ = *ptr/128;

                    ptr += sourcePixelIncrement;
                    left += sourcePixelIncrement;
                    right += sourcePixelIncrement;
                    above += sourcePixelIncrement;
                    below += sourcePixelIncrement;
                    in += sourcePixelIncrement;
                    out += sourcePixelIncrement;
                }
            }
            else if (dataType==GL_UNSIGNED_SHORT)
            {
                unsigned short* ptr = (unsigned short*)(image_3d->data(1,t,r)+alphaOffset);
                unsigned short* left = (unsigned short*)(image_3d->data(0,t,r)+alphaOffset);
                unsigned short* right = (unsigned short*)(image_3d->data(2,t,r)+alphaOffset);
                unsigned short* above = (unsigned short*)(image_3d->data(1,t+1,r)+alphaOffset);
                unsigned short* below = (unsigned short*)(image_3d->data(1,t-1,r)+alphaOffset);
                unsigned short* in = (unsigned short*)(image_3d->data(1,t,r+1)+alphaOffset);
                unsigned short* out = (unsigned short*)(image_3d->data(1,t,r-1)+alphaOffset);

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

                    *destination++ = *ptr/256;

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
    }
    
    
    osg::notify(osg::NOTICE)<<"Created NormalMapTexture"<<std::endl;
    
    return normalmap_3d.release();
}



osg::Node* createCube(float size,float alpha, unsigned int numSlices, float sliceEnd=1.0f)
{

    // set up the Geometry.
    osg::Geometry* geom = new osg::Geometry;

    float halfSize = size*0.5f;
    float y = halfSize;
    float dy =-size/(float)(numSlices-1)*sliceEnd;

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
    
        FollowMouseCallback(bool shader = false):
            _shader(shader)
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
                    float v = (ea.getY()-ea.getYmin())/(ea.getYmax()-ea.getYmin());
                    if (_shader)
                    {
                        osg::Uniform* uniform = 0;
                        if (_updateTransparency && (uniform = stateset->getUniform("transparency"))) uniform->set(v);
                        if (_updateAlphaCutOff && (uniform = stateset->getUniform("alphaCutOff"))) uniform->set(v);
                        if (_updateSampleDensity && (uniform = stateset->getUniform("sampleDensity"))) 
                        {
                            float value = powf(v,5);
                            osg::notify(osg::INFO)<<"sampleDensity = "<<value<<std::endl;
                            uniform->set(value);
                        }
                    }
                    else
                    {                    
                        if (_updateAlphaCutOff)
                        {
                            osg::AlphaFunc* alphaFunc = dynamic_cast<osg::AlphaFunc*>(stateset->getAttribute(osg::StateAttribute::ALPHAFUNC));
                            if (alphaFunc) 
                            {
                                alphaFunc->setReferenceValue(v);
                            }
                        }
                        
                        if (_updateTransparency)
                        {
                            osg::Material* material = dynamic_cast<osg::Material*>(stateset->getAttribute(osg::StateAttribute::MATERIAL));
                            if (material)
                            {
                                material->setAlpha(osg::Material::FRONT_AND_BACK,v);
                            }
                        }
                    }

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
        
        bool _shader;
        bool _updateTransparency;
        bool _updateAlphaCutOff;
        bool _updateSampleDensity;

};

osg::Node* createShaderModel(ShadingModel shadingModel,
                       osg::ref_ptr<osg::Image>& image_3d, 
                       osg::Image* normalmap_3d,
                       osg::TransferFunction1D* tf,
                       osg::Texture::InternalFormatMode internalFormatMode,
                       float xSize, float ySize, float zSize,
                       float /*xMultiplier*/, float /*yMultiplier*/, float /*zMultiplier*/,
                       unsigned int /*numSlices*/=500, float /*sliceEnd*/=1.0f, float alphaFuncValue=0.02f)
{
    osg::Texture::FilterMode minFilter = osg::Texture::LINEAR;
    osg::Texture::FilterMode magFilter = osg::Texture::LINEAR;

    osg::Group* root = new osg::Group;
    
    osg::Geode* geode = new osg::Geode;
    root->addChild(geode);
    
    osg::StateSet* stateset = geode->getOrCreateStateSet();
    
    stateset->setEventCallback(new FollowMouseCallback(true));
    
    stateset->setMode(GL_ALPHA_TEST,osg::StateAttribute::ON);

    
    osg::Program* program = new osg::Program;
    stateset->setAttribute(program);

    // get shaders from source
    
    osg::Shader* vertexShader = osgDB::readShaderFile(osg::Shader::VERTEX, "volume.vert");
    if (vertexShader)
    {
        program->addShader(vertexShader);
    }
    else
    {
        #include "volume_vert.cpp"
        program->addShader(new osg::Shader(osg::Shader::VERTEX, volume_vert));
    }

    if (!(normalmap_3d && tf))
    {
        // set up the 3d texture itself,
        // note, well set the filtering up so that mip mapping is disabled,
        // gluBuild3DMipsmaps doesn't do a very good job of handled the
        // imbalanced dimensions of the 256x256x4 texture.
        osg::Texture3D* texture3D = new osg::Texture3D;
        texture3D->setResizeNonPowerOfTwoHint(false);
        texture3D->setFilter(osg::Texture3D::MIN_FILTER,minFilter);
        texture3D->setFilter(osg::Texture3D::MAG_FILTER, magFilter);
        texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP_TO_EDGE);
        texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP_TO_EDGE);
        texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP_TO_EDGE);
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

        osg::Uniform* baseTextureSampler = new osg::Uniform("baseTexture",0);
        stateset->addUniform(baseTextureSampler);
    }
    

    if (shadingModel==MaximumIntensityProjection)
    {
        if (tf)
        {
            osg::Texture1D* texture1D = new osg::Texture1D;
            texture1D->setImage(tf->getImage());    
            stateset->setTextureAttributeAndModes(1,texture1D,osg::StateAttribute::ON);

            osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "volume_tf_mip.frag");
            if (fragmentShader)
            {
                program->addShader(fragmentShader);
            }
            else
            {
                #include "volume_tf_mip_frag.cpp"
                program->addShader(new osg::Shader(osg::Shader::FRAGMENT, volume_tf_mip_frag));
            }

            osg::Uniform* tfTextureSampler = new osg::Uniform("tfTexture",1);
            stateset->addUniform(tfTextureSampler);

        }
        else
        {    
            osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "volume_mip.frag");
            if (fragmentShader)
            {
                program->addShader(fragmentShader);
            }
            else
            {
                #include "volume_mip_frag.cpp"
                program->addShader(new osg::Shader(osg::Shader::FRAGMENT, volume_mip_frag));
            }
        }
    }
    else if (shadingModel==Isosurface)
    {

        if (tf)
        {
            osg::Texture1D* texture1D = new osg::Texture1D;
            texture1D->setImage(tf->getImage());    
            texture1D->setResizeNonPowerOfTwoHint(false);
            texture1D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
            texture1D->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
            texture1D->setWrap(osg::Texture::WRAP_R,osg::Texture::CLAMP_TO_EDGE);
            stateset->setTextureAttributeAndModes(1,texture1D,osg::StateAttribute::ON);

            osg::Uniform* tfTextureSampler = new osg::Uniform("tfTexture",1);
            stateset->addUniform(tfTextureSampler);

            osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "volume_tf_iso.frag");
            if (fragmentShader)
            {
                program->addShader(fragmentShader);
            }
            else
            {
                #include "volume_tf_iso_frag.cpp"
                program->addShader(new osg::Shader(osg::Shader::FRAGMENT, volume_tf_iso_frag));
            }
        }
        else
        {    
            osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "volume_iso.frag");
            if (fragmentShader)
            {
                program->addShader(fragmentShader);
            }
            else
            {
                #include "volume_iso_frag.cpp"
                program->addShader(new osg::Shader(osg::Shader::FRAGMENT, volume_iso_frag));
            }
        }
    }
    else if (normalmap_3d)
    {
        osg::notify(osg::NOTICE)<<"Setting up normalmapping shader"<<std::endl;

        osg::Uniform* normalMapSampler = new osg::Uniform("normalMap",1);
        stateset->addUniform(normalMapSampler);

        osg::Texture3D* normalMap = new osg::Texture3D;
        normalMap->setImage(normalmap_3d);    
        normalMap->setResizeNonPowerOfTwoHint(false);
        normalMap->setInternalFormatMode(internalFormatMode);
        normalMap->setFilter(osg::Texture3D::MIN_FILTER, osg::Texture::LINEAR);
        normalMap->setFilter(osg::Texture3D::MAG_FILTER, osg::Texture::LINEAR);
        normalMap->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP_TO_EDGE);
        normalMap->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP_TO_EDGE);
        normalMap->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP_TO_EDGE);

        stateset->setTextureAttributeAndModes(1,normalMap,osg::StateAttribute::ON);

        if (tf)
        {
            osg::Texture1D* texture1D = new osg::Texture1D;
            texture1D->setImage(tf->getImage());    
            texture1D->setResizeNonPowerOfTwoHint(false);
            texture1D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
            texture1D->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
            texture1D->setWrap(osg::Texture::WRAP_R,osg::Texture::CLAMP_TO_EDGE);
            stateset->setTextureAttributeAndModes(0,texture1D,osg::StateAttribute::ON);

            osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "volume-tf-n.frag");
            if (fragmentShader)
            {
                program->addShader(fragmentShader);
            }
            else
            {
                #include "volume_tf_n_frag.cpp"
                program->addShader(new osg::Shader(osg::Shader::FRAGMENT, volume_tf_n_frag));
            }

            osg::Uniform* tfTextureSampler = new osg::Uniform("tfTexture",0);
            stateset->addUniform(tfTextureSampler);
        }
        else
        {
            osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "volume-n.frag");
            if (fragmentShader)
            {
                program->addShader(fragmentShader);
            }
            else
            {
                #include "volume_n_frag.cpp"
                program->addShader(new osg::Shader(osg::Shader::FRAGMENT, volume_n_frag));
            }
        }
    } 
    else if (tf)
    {
        osg::Texture1D* texture1D = new osg::Texture1D;
        texture1D->setImage(tf->getImage());    
        texture1D->setResizeNonPowerOfTwoHint(false);
        texture1D->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
        texture1D->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
        texture1D->setWrap(osg::Texture::WRAP_R,osg::Texture::CLAMP_TO_EDGE);
        stateset->setTextureAttributeAndModes(1,texture1D,osg::StateAttribute::ON);

        osg::Uniform* tfTextureSampler = new osg::Uniform("tfTexture",1);
        stateset->addUniform(tfTextureSampler);

        osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "volume-tf.frag");
        if (fragmentShader)
        {
            program->addShader(fragmentShader);
        }
        else
        {
            #include "volume_tf_frag.cpp"
            program->addShader(new osg::Shader(osg::Shader::FRAGMENT, volume_tf_frag));
        }

    }
    else
    {    

        osg::Shader* fragmentShader = osgDB::readShaderFile(osg::Shader::FRAGMENT, "volume.frag");
        if (fragmentShader)
        {
            program->addShader(fragmentShader);
        }
        else
        {
            #include "volume_frag.cpp"
            program->addShader(new osg::Shader(osg::Shader::FRAGMENT, volume_frag));
        }
    }

    osg::Uniform* sampleDensity = new osg::Uniform("sampleDensity", 0.005f);
    stateset->addUniform(sampleDensity);

    osg::Uniform* transpancy = new osg::Uniform("transparency",0.5f);
    stateset->addUniform(transpancy);

    osg::Uniform* alphaCutOff = new osg::Uniform("alphaCutOff",alphaFuncValue);
    stateset->addUniform(alphaCutOff);

    stateset->setMode(GL_CULL_FACE, osg::StateAttribute::ON);

    osg::TexGen* texgen = new osg::TexGen;
    texgen->setMode(osg::TexGen::OBJECT_LINEAR);
    texgen->setPlane(osg::TexGen::S, osg::Plane(1.0f/xSize,0.0f,0.0f,0.0f));
    texgen->setPlane(osg::TexGen::T, osg::Plane(0.0f,1.0f/ySize,0.0f,0.0f));
    texgen->setPlane(osg::TexGen::R, osg::Plane(0.0f,0.0f,1.0f/zSize,0.0f));
    texgen->setPlane(osg::TexGen::Q, osg::Plane(0.0f,0.0f,0.0f,1.0f));
    
    stateset->setTextureAttributeAndModes(0, texgen, osg::StateAttribute::ON);

    {
        osg::Geometry* geom = new osg::Geometry;

        osg::Vec3Array* coords = new osg::Vec3Array(8);
        (*coords)[0].set(0,0,0);
        (*coords)[1].set(xSize,0,0);
        (*coords)[2].set(xSize,ySize,0);
        (*coords)[3].set(0,ySize,0);
        (*coords)[4].set(0,0,zSize);
        (*coords)[5].set(xSize,0,zSize);
        (*coords)[6].set(ySize,ySize,zSize);
        (*coords)[7].set(0,ySize,zSize);
        geom->setVertexArray(coords);

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
    return root;
}

osg::Node* createModel(ShadingModel shadeModel,
                       osg::ref_ptr<osg::Image>& image_3d, 
                       osg::ref_ptr<osg::Image>& normalmap_3d,
                       osg::Texture::InternalFormatMode internalFormatMode,
                       float xSize, float ySize, float zSize,
                       float xMultiplier, float yMultiplier, float zMultiplier,
                       unsigned int numSlices=500, float sliceEnd=1.0f, float alphaFuncValue=0.02f, bool maximumIntensityProjection = false)
{
    bool two_pass = normalmap_3d.valid() && (image_3d->getPixelFormat()==GL_RGB || image_3d->getPixelFormat()==GL_RGBA);

    osg::BoundingBox bb(-xSize*0.5f,-ySize*0.5f,-zSize*0.5f,xSize*0.5f,ySize*0.5f,zSize*0.5f);


    osg::Texture::FilterMode minFilter = osg::Texture::NEAREST;
    osg::Texture::FilterMode magFilter = osg::Texture::NEAREST;

    float maxAxis = xSize;
    if (ySize > maxAxis) maxAxis = ySize;
    if (zSize > maxAxis) maxAxis = zSize;

    osg::Group* group = new osg::Group;
    
    osg::TexGenNode* texgenNode_0 = new osg::TexGenNode;
    texgenNode_0->setTextureUnit(0);
    texgenNode_0->getTexGen()->setMode(osg::TexGen::EYE_LINEAR);
    texgenNode_0->getTexGen()->setPlane(osg::TexGen::S, osg::Plane(xMultiplier/xSize,0.0f,0.0f,0.5f));
    texgenNode_0->getTexGen()->setPlane(osg::TexGen::T, osg::Plane(0.0f,yMultiplier/ySize,0.0f,0.5f));
    texgenNode_0->getTexGen()->setPlane(osg::TexGen::R, osg::Plane(0.0f,0.0f,zMultiplier/zSize,0.5f));
    
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

    float cubeSize = sqrtf(xSize*xSize+ySize*ySize+zSize*zSize);

    osg::ClipNode* clipnode = new osg::ClipNode;
    clipnode->addChild(createCube(cubeSize,1.0f, numSlices,sliceEnd));
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

    stateset->setEventCallback(new FollowMouseCallback(false));
 
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::ON);
    stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
    stateset->setAttributeAndModes(new osg::AlphaFunc(osg::AlphaFunc::GREATER,alphaFuncValue), osg::StateAttribute::ON);
    
    osg::Material* material = new osg::Material;
    material->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(1.0f,1.0f,1.0f,1.0f));
    stateset->setAttributeAndModes(material);
    
    if (shadeModel==MaximumIntensityProjection)
    {
        stateset->setAttribute(new osg::BlendFunc(osg::BlendFunc::ONE, osg::BlendFunc::ONE));
        stateset->setAttribute(new osg::BlendEquation(osg::BlendEquation::RGBA_MAX));
    }
    
    osg::Vec3 lightDirection(1.0f,-1.0f,1.0f);
    lightDirection.normalize();

    if (normalmap_3d.valid())
    {
        if (two_pass)
        {

            // set up normal texture
            osg::Texture3D* bump_texture3D = new osg::Texture3D;
            bump_texture3D->setFilter(osg::Texture3D::MIN_FILTER,minFilter);
            bump_texture3D->setFilter(osg::Texture3D::MAG_FILTER, magFilter);
            bump_texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP_TO_EDGE);
            bump_texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP_TO_EDGE);
            bump_texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP_TO_EDGE);
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
            texture3D->setResizeNonPowerOfTwoHint(false);
            texture3D->setFilter(osg::Texture3D::MIN_FILTER,minFilter);
            texture3D->setFilter(osg::Texture3D::MAG_FILTER, magFilter);
            texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP_TO_EDGE);
            texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP_TO_EDGE);
            texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP_TO_EDGE);
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
            osg::Texture3D* bump_texture3D = new osg::Texture3D;
            bump_texture3D->setResizeNonPowerOfTwoHint(false);
            bump_texture3D->setFilter(osg::Texture3D::MIN_FILTER,minFilter);
            bump_texture3D->setFilter(osg::Texture3D::MAG_FILTER, magFilter);
            bump_texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP_TO_EDGE);
            bump_texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP_TO_EDGE);
            bump_texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP_TO_EDGE);
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
        // imbalanced dimensions of the 256x256x4 texture.
        osg::Texture3D* texture3D = new osg::Texture3D;
        texture3D->setResizeNonPowerOfTwoHint(false);
        texture3D->setFilter(osg::Texture3D::MIN_FILTER,minFilter);
        texture3D->setFilter(osg::Texture3D::MAG_FILTER, magFilter);
        texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP_TO_EDGE);
        texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP_TO_EDGE);
        texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP_TO_EDGE);
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


struct ApplyTransferFunctionOperator
{
    ApplyTransferFunctionOperator(osg::TransferFunction1D* tf, unsigned char* data):
        _tf(tf),
        _data(data) {}
        
    inline void luminance(float l) const
    {
        osg::Vec4 c = _tf->getInterpolatedValue(l);
        //std::cout<<"l = "<<l<<" c="<<c<<std::endl;
        *(_data++) = (unsigned char)(c[0]*255.0f + 0.5f);
        *(_data++) = (unsigned char)(c[1]*255.0f + 0.5f);
        *(_data++) = (unsigned char)(c[2]*255.0f + 0.5f);
        *(_data++) = (unsigned char)(c[3]*255.0f + 0.5f);
    }
     
    inline void alpha(float a) const
    {
        luminance(a);
    } 
    
    inline void luminance_alpha(float l,float a) const
    { 
        luminance(l);
    }
     
    inline void rgb(float r,float g,float b) const
    {
        luminance((r+g+b)*0.3333333);
    }
    
    inline void rgba(float r,float g,float b,float a) const
    {
        luminance(a);
    }
    
    mutable osg::ref_ptr<osg::TransferFunction1D> _tf;
    mutable unsigned char* _data;
};

osg::Image* applyTransferFunction(osg::Image* image, osg::TransferFunction1D* transferFunction)
{
    std::cout<<"Applying transfer function"<<std::endl;
    osg::Image* output_image = new osg::Image;
    output_image->allocateImage(image->s(),image->t(), image->r(), GL_RGBA, GL_UNSIGNED_BYTE);
    
    ApplyTransferFunctionOperator op(transferFunction, output_image->data());
    osg::readImage(image,op); 
    
    return output_image;
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

    osg::TransferFunction1D::ValueMap valueMap;
    osgDB::ifstream fin(foundFile.c_str());
    while(fin)
    {
        float value, red, green, blue, alpha;
        fin >> value >> red >> green >> blue >> alpha;
        if (fin) 
        {
            std::cout<<"value = "<<value<<" ("<<red<<", "<<green<<", "<<blue<<", "<<alpha<<")"<<std::endl;
            valueMap[value] = osg::Vec4(red,green,blue,alpha);
        }
    }
    
    if (valueMap.empty())
    {
        std::cout<<"Error: No values read from transfer function file: "<<filename<<std::endl;
        return 0;
    }
    
    osg::TransferFunction1D* tf = new osg::TransferFunction1D;
    tf->assign(valueMap, true);
    
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
    arguments.getApplicationUsage()->addCommandLineOption("-n","Create normal map for per voxel lighting.");
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

    unsigned int numSlices=500;
    while (arguments.read("-s",numSlices)) {}
    
    
    float sliceEnd=1.0f;
    while (arguments.read("--clip",sliceEnd)) {}

    float alphaFunc=0.02f;
    while (arguments.read("--alphaFunc",alphaFunc)) {}


    
    ShadingModel shadingModel = Standard;
    while(arguments.read("--mip")) shadingModel =  MaximumIntensityProjection;

    bool createNormalMap = false;
    while (arguments.read("-n")) 
    {
        shadingModel = Light;
        createNormalMap=true;
    }

    while (arguments.read("--isosurface")) 
    {
        shadingModel = Isosurface;
    }

    float xSize=1.0f, ySize=1.0f, zSize=1.0f;
    while (arguments.read("--xSize",xSize)) {}
    while (arguments.read("--ySize",ySize)) {}
    while (arguments.read("--zSize",zSize)) {}

    float xMultiplier=1.0f, yMultiplier=1.0f, zMultiplier=1.0f;
    while (arguments.read("--xMultiplier",xMultiplier)) {}
    while (arguments.read("--yMultiplier",yMultiplier)) {}
    while (arguments.read("--zMultiplier",zMultiplier)) {}

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

    bool useOsgVolume = true; 
    while(arguments.read("--osgVolume")) { useOsgVolume = true; }
    while(arguments.read("--no-osgVolume")) { useOsgVolume = false; }

    bool useShader = true; 
    while(arguments.read("--shader")) { useShader = true; }
    while(arguments.read("--no-shader")) { useShader = true; }

    bool gpuTransferFunction = true; 
    while(arguments.read("--gpu-tf")) { gpuTransferFunction = true; }
    while(arguments.read("--cpu-tf")) { gpuTransferFunction = false; }

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
                osg::TransferFunction1D::ValueMap valueMap;
                float value = 0.0;
                while(fin && value<=1.0)
                {
                    float red, green, blue, alpha;
                    fin >> red >> green >> blue >> alpha;
                    if (fin) 
                    {
                        valueMap[value] = osg::Vec4(red/255.0f,green/255.0f,blue/255.0f,alpha/255.0f);
                        std::cout<<"value = "<<value<<" ("<<red<<", "<<green<<", "<<blue<<", "<<alpha<<")";
                        std::cout<<"  ("<<valueMap[value]<<")"<<std::endl;
                    }
                    value += 1/255.0;
                }

                if (valueMap.empty())
                {
                    std::cout<<"Error: No values read from transfer function file: "<<transfer_filename<<std::endl;
                    return 0;
                }

                transferFunction = new osg::TransferFunction1D;
                transferFunction->assign(valueMap, true);
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
    xSize = (*sizeItr)->s();
    ySize = (*sizeItr)->t();
    zSize = (*sizeItr)->r();
    ++sizeItr;

    for(;sizeItr != images.end(); ++sizeItr)
    {
        if ((*sizeItr)->s() != xSize || 
            (*sizeItr)->t() != ySize ||
            (*sizeItr)->r() != zSize)
        {
            std::cout<<"Images in sequence are not of the same dimensions."<<std::endl;
            return 1;
        }
    }


    osg::RefMatrix* matrix = dynamic_cast<osg::RefMatrix*>(images.front()->getUserData());
#if 0
    if (matrix)
    {
        osg::notify(osg::NOTICE)<<"Image has Matrix = "<<*matrix<<std::endl;
        xSize = xSize * (*matrix)(0,0);
        ySize = ySize * (*matrix)(1,1);
        zSize = zSize * (*matrix)(2,2);
    }
#endif

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
            *itr = applyTransferFunction(itr->get(), transferFunction.get());
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
        imageSequence->setLength(10.0);
        image_3d = imageSequence.get();
        for(Images::iterator itr = images.begin();
            itr != images.end();
            ++itr)
        {        
            imageSequence->addImage(itr->get());
        }
        imageSequence->play();
    }
    
    osg::ref_ptr<osg::Image> normalmap_3d = 0;
    if (createNormalMap)
    {
        if (images.size()==1)
        {
            normalmap_3d = createNormalMapTexture(images.front().get());
        }
        else
        {
            osg::ref_ptr<osg::ImageSequence> normalmapSequence = new osg::ImageSequence;
            normalmap_3d = normalmapSequence.get();
            for(Images::iterator itr = images.begin();
                itr != images.end();
                ++itr)
            {        
                normalmapSequence->addImage(createNormalMapTexture(itr->get()));
            }
            normalmapSequence->play();
        }
    }

    // create a model from the images.
    osg::ref_ptr<osg::Node> rootNode = 0;
    
    if (useOsgVolume)
    {

        osg::ref_ptr<osgVolume::Volume> volume = new osgVolume::Volume;
        osg::ref_ptr<osgVolume::VolumeTile> tile = new osgVolume::VolumeTile;
        volume->addChild(tile);

        osg::ref_ptr<osgVolume::Layer> layer = new osgVolume::ImageLayer(image_3d);
        layer->setProperty(new osgVolume::TransferFunctionProperty(transferFunction.get()));
        
        if (matrix)
        {
            osgVolume::Locator* locator = new osgVolume::Locator(*matrix);
            layer->setLocator(locator);
            tile->setLocator(locator);
        }
        
        tile->setLayer(layer.get());
        
        if (useShader)
        {
            switch(shadingModel)
            {
                case(Standard):
                    break;
                case(Light):
                    layer->addProperty(new osgVolume::LightingProperty);
                    break;
                case(Isosurface):
                    layer->addProperty(new osgVolume::IsoSurfaceProperty(alphaFunc));
                    break;
                case(MaximumIntensityProjection):
                    layer->addProperty(new osgVolume::MaximumIntensityProjectionProperty);
                    break;
            }
            
            layer->addProperty(new osgVolume::AlphaFuncProperty(alphaFunc));
        
            tile->setVolumeTechnique(new osgVolume::ShaderTechnique);
        }
        else
        {
            tile->setVolumeTechnique(new osgVolume::FixedFunctionTechnique);
        }
        
        
        rootNode = volume.get();        

    }
    else
    {
        if (useShader)
        {
            rootNode = createShaderModel(shadingModel, 
                                   image_3d, normalmap_3d.get(), 
                                   (gpuTransferFunction ? transferFunction.get() : 0),
                                   internalFormatMode,
                                   xSize, ySize, zSize,
                                   xMultiplier, yMultiplier, zMultiplier,
                                   numSlices, sliceEnd, alphaFunc);
        }
        else
        {
            rootNode = createModel(shadingModel,
                                   image_3d, normalmap_3d, 
                                   internalFormatMode,
                                   xSize, ySize, zSize,
                                   xMultiplier, yMultiplier, zMultiplier,
                                   numSlices, sliceEnd, alphaFunc);
        }
        
        if (matrix && rootNode)
        {
            osg::MatrixTransform* mt = new osg::MatrixTransform;
            mt->setMatrix(*matrix);
            mt->addChild(rootNode);

            rootNode = mt;
        }
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
        viewer.setSceneData(rootNode.get());
        
        // the the viewers main frame loop
        viewer.run();
    }    
    
    return 0;

}
