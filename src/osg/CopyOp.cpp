#include <osg/CopyOp>
#include <osg/Node>
#include <osg/StateSet>
#include <osg/Texture>
#include <osg/Drawable>

using namespace osg;

Referenced* CopyOp::operator() (const Referenced* ref) const
{
    return const_cast<Referenced*>(ref);
}

Object* CopyOp::operator() (const Object* obj) const
{
    if (obj && _flags&DEEP_COPY_OBJECTS)
        return obj->clone(*this);
    else return const_cast<Object*>(obj);
}

Node* CopyOp::operator() (const Node* node) const
{
    if (node && _flags&DEEP_COPY_NODES)
        return dynamic_cast<Node*>(node->clone(*this));
    else
        return const_cast<Node*>(node);
}

Drawable* CopyOp::operator() (const Drawable* drawable) const
{
    if (drawable && _flags&DEEP_COPY_DRAWABLES)
        return dynamic_cast<Drawable*>(drawable->clone(*this));
    else
        return const_cast<Drawable*>(drawable);
}

StateSet* CopyOp::operator() (const StateSet* stateset) const
{
    if (stateset && _flags&DEEP_COPY_STATESETS)
        return dynamic_cast<StateSet*>(stateset->clone(*this));
    else
        return const_cast<StateSet*>(stateset);
}

StateAttribute* CopyOp::operator() (const StateAttribute* attr) const
{
    if (attr && _flags&DEEP_COPY_STATEATTRIBUTES)
    {
        const Texture* text = dynamic_cast<const Texture*>(attr);
        if (text)
        {
            return operator()(text);
        }
        else        
            return dynamic_cast<StateAttribute*>(attr->clone(*this));
    }
    else
        return const_cast<StateAttribute*>(attr);
}

Texture* CopyOp::operator() (const Texture* text) const
{
    if (text && _flags&DEEP_COPY_TEXTURES)
        return dynamic_cast<Texture*>(text->clone(*this));
    else
        return const_cast<Texture*>(text);
}

Image* CopyOp::operator() (const Image* image) const
{
    if (image && _flags&DEEP_COPY_IMAGES)
        return dynamic_cast<Image*>(image->clone(*this));
    else return const_cast<Image*>(image);
}
