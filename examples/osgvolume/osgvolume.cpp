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
#include <osg/TexEnvCombine>
#include <osg/Material>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>

#include <osgUtil/CullVisitor>

#include <osgProducer/Viewer>


const int maximumTextureSize = 256;

typedef std::vector< osg::ref_ptr<osg::Image> > ImageList;

struct PassThroughTransformFunction
{
    unsigned char operator() (unsigned char c) const { return c; }
};


struct ProcessRow
{
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


osg::Image* createTexture3D(ImageList& imageList, ProcessRow& processRow, unsigned int numComponentsDesired=0)
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
    while(s_nearestPowerOfTwo<max_s && s_nearestPowerOfTwo<maximumTextureSize) s_nearestPowerOfTwo*=2;

    int t_nearestPowerOfTwo = 1;
    while(t_nearestPowerOfTwo<max_t && t_nearestPowerOfTwo<maximumTextureSize) t_nearestPowerOfTwo*=2;

    int r_nearestPowerOfTwo = 1;
    while(r_nearestPowerOfTwo<total_r && r_nearestPowerOfTwo<maximumTextureSize) r_nearestPowerOfTwo*=2;


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
    
    osg::ref_ptr<osg::Image> bumpmap_3d = new osg::Image;
    bumpmap_3d->allocateImage(image_3d->s(),image_3d->t(),image_3d->r(),
                            GL_RGBA,GL_UNSIGNED_BYTE);


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

            unsigned char* destination = (unsigned char*) bumpmap_3d->data(1,t,r);

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

                *destination++ = (unsigned char)(grad.x()); // scale and bias X.
                *destination++ = (unsigned char)(grad.y()); // scale and bias Y.
                *destination++ = (unsigned char)(grad.z()); // scale and bias Z.

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
    
    return bumpmap_3d.release();
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

osg::Node* createModel(osg::ref_ptr<osg::Image>& image_3d, osg::ref_ptr<osg::Image>& bumpmap_3d,
                       float xSize, float ySize, float zSize,
                       float xMultiplier, float yMultiplier, float zMultiplier,
                       unsigned int numSlices=500, float sliceEnd=1.0f, float alphaFuncValue=0.02f)
{
    bool two_pass = bumpmap_3d.valid() && (image_3d->getPixelFormat()==GL_RGB || image_3d->getPixelFormat()==GL_RGBA);

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

    if (bumpmap_3d.valid())
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
            bump_texture3D->setImage(bumpmap_3d.get());

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
            texture3D->setImage(image_3d.get());

            stateset->setTextureAttributeAndModes(1,texture3D,osg::StateAttribute::ON);

            stateset->setTextureMode(1,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
            stateset->setTextureMode(1,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
            stateset->setTextureMode(1,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);

            stateset->setTextureAttributeAndModes(1,new osg::TexEnv(),osg::StateAttribute::ON);

	}
        else
        {
            osg::ref_ptr<osg::Image> bumpmap_3d = createNormalMapTexture(image_3d.get());
            osg::Texture3D* bump_texture3D = new osg::Texture3D;
            bump_texture3D->setFilter(osg::Texture3D::MIN_FILTER,osg::Texture3D::LINEAR);
            bump_texture3D->setFilter(osg::Texture3D::MAG_FILTER,osg::Texture3D::LINEAR);
            bump_texture3D->setWrap(osg::Texture3D::WRAP_R,osg::Texture3D::CLAMP);
            bump_texture3D->setWrap(osg::Texture3D::WRAP_S,osg::Texture3D::CLAMP);
            bump_texture3D->setWrap(osg::Texture3D::WRAP_T,osg::Texture3D::CLAMP);
            bump_texture3D->setImage(bumpmap_3d.get());

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

            image_3d = bumpmap_3d;
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
        texture3D->setImage(image_3d.get());

        stateset->setTextureAttributeAndModes(0,texture3D,osg::StateAttribute::ON);

        stateset->setTextureMode(0,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
        stateset->setTextureMode(0,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
        stateset->setTextureMode(0,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);

        stateset->setTextureAttributeAndModes(0,new osg::TexEnv(),osg::StateAttribute::ON);
    }
 
    return group;
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
    arguments.getApplicationUsage()->addCommandLineOption("-s","Number of slices to create.");
    arguments.getApplicationUsage()->addCommandLineOption("--xSize","Relative width of rendered brick.");
    arguments.getApplicationUsage()->addCommandLineOption("--ySize","Relative length of rendered brick.");
    arguments.getApplicationUsage()->addCommandLineOption("--zSize","Relative height of rendered brick.");
    arguments.getApplicationUsage()->addCommandLineOption("--xMultiplier","Tex coord x mulitplier.");
    arguments.getApplicationUsage()->addCommandLineOption("--yMultiplier","Tex coord y mulitplier.");
    arguments.getApplicationUsage()->addCommandLineOption("--zMultiplier","Tex coord z mulitplier.");
    arguments.getApplicationUsage()->addCommandLineOption("--clip","clip volume as a ratio, 0.0 clip all, 1.0 clip none.");
    

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

    
    osg::ref_ptr<osg::Image> image_3d;
    
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
        image_3d = createTexture3D(imageList, processRow);
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
    
    osg::ref_ptr<osg::Image> bumpmap_3d = createNormalMap ? createNormalMapTexture(image_3d.get()) : 0;

    // create a model from the images.
    osg::Node* rootNode = createModel(image_3d, bumpmap_3d, 
                                      xSize, ySize, zSize,
                                      xMultiplier, yMultiplier, zMultiplier,
                                      numSlices, sliceEnd, alphaFunc);

    if (!outputFile.empty())
    {   
        std::string ext = osgDB::getFileExtension(outputFile);
        std::string name_no_ext = osgDB::getNameLessExtension(outputFile);
        if (ext=="osg")
        {
            image_3d->setFileName(name_no_ext + ".dds");
            osgDB::writeImageFile(*image_3d, image_3d->getFileName());
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
        

        // wait for all cull and draw threads to complete before exit.
        viewer.sync();
    }    
    
    return 0;
}
