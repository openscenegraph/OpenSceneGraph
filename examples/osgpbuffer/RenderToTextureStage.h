//C++ header - Open Scene Graph - Copyright (C) 1998-2002 Robert Osfield
//Distributed under the terms of the GNU Library General Public License (LGPL)
//as published by the Free Software Foundation.

#ifndef RENDERTOTEXTURESTAGE
#define RENDERTOTEXTURESTAGE 1

#include <osg/Texture2D>

#include <osgUtil/RenderStage>

#include "pbuffer.h"

// namespace osgUtil {

/**
 * RenderStage which copies the final image to an attached texture or image.
 * Generally used as a pre-rendering stage.
 */
class /*OSGUTIL_EXPORT*/ MyRenderToTextureStage : public osgUtil::RenderStage
{
    public:
    

        MyRenderToTextureStage();
        
        virtual osg::Object* cloneType() const { return new MyRenderToTextureStage(); }
        virtual osg::Object* clone(const osg::CopyOp&) const { return new MyRenderToTextureStage(); } // note only implements a clone of type.
        virtual bool isSameKindAs(const osg::Object* obj) const { return dynamic_cast<const MyRenderToTextureStage*>(obj)!=0L; }
        virtual const char* libraryName() const { return ""; }
        virtual const char* className() const { return "MyRenderToTextureStage"; }

        inline void setPBuffer(PBuffer* pbuffer) { _pbuffer = pbuffer; }

        virtual void reset();
        
        void setTexture(osg::Texture2D* texture) { _texture = texture; }
        osg::Texture2D* getTexture() { return _texture.get(); }
        
        void setImage(osg::Image* image) { _image = image; }
        osg::Image* getImage() { return _image.get(); }

        virtual void draw(osg::State& state,osgUtil::RenderLeaf*& previous);

    public:
        
        
    protected:
    
        virtual ~MyRenderToTextureStage();
        
        osg::ref_ptr<osg::Texture2D> _texture;
        osg::ref_ptr<osg::Image> _image;
        PBuffer* _pbuffer;
};

// }

#endif

