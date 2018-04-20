/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#ifndef OSG_CALLBACK
#define OSG_CALLBACK 1

#include <osg/Object>
#include <osg/UserDataContainer>

// forward declare
namespace osgGA { class EventHandler; }

namespace osg {

// forward declare
class CallbackObject;
class NodeCallback;
class StateAttributeCallback;
class UniformCallback;
class DrawableUpdateCallback;
class DrawableEventCallback;
class DrawableCullCallback;

class OSG_EXPORT Callback : public virtual Object {

    public :

        Callback(){}

        Callback(const Callback& cb,const CopyOp& copyop):
            osg::Object(cb, copyop),
            _nestedCallback(cb._nestedCallback) {}

        META_Object(osg, Callback);

        virtual Callback* asCallback() { return this; }
        virtual const Callback* asCallback() const { return this; }

        virtual CallbackObject* asCallbackObject() { return 0; }
        virtual const CallbackObject* asCallbackObject() const { return 0; }

        virtual NodeCallback* asNodeCallback() { return 0; }
        virtual const NodeCallback* asNodeCallback() const { return 0; }

        virtual StateAttributeCallback* asStateAttributeCallback() { return 0; }
        virtual const StateAttributeCallback* asStateAttributeCallback() const { return 0; }

        virtual UniformCallback* asUniformCallback() { return 0; }
        virtual const UniformCallback* asUniformCallback() const { return 0; }

        virtual DrawableUpdateCallback* asDrawableUpdateCallback() { return 0; }
        virtual const DrawableUpdateCallback* asDrawableUpdateCallback() const { return 0; }

        virtual DrawableEventCallback* asDrawableEventCallback() { return 0; }
        virtual const DrawableEventCallback* asDrawableEventCallback() const { return 0; }

        virtual DrawableCullCallback* asDrawableCullCallback() { return 0; }
        virtual const DrawableCullCallback* asDrawableCullCallback() const { return 0; }

        virtual osgGA::EventHandler* asEventHandler() { return 0; }
        virtual const osgGA::EventHandler* asEventHandler() const { return 0; }

        /** Invoke the callback, first parameter is the Object that the callback is attached to,
         *  the second parameter, the data, is typically the NodeVisitor that is invoking the callback.
         *  The run(..) method may be overridden by users directly, or if the user is using one of the old
         *  style callbacks such as NodeCallback or Drawable::UpdateCallback then you can just override
         *  the appropriate callback method on those callback subclasses.
         *  If you are implementing your own callback then one should call traverse() to make sure nested callbacks
         *  and visitor traversal() is completed. */
        virtual bool run(osg::Object* object, osg::Object* data)
        {
            return traverse(object, data);
        }

        /** traverse the nested callbacks or call NodeVisitor::traverse() if the object is Node, and data is NodeVisitor.*/
        bool traverse(osg::Object* object, osg::Object* data);

        void setNestedCallback(osg::Callback* cb) { _nestedCallback = cb; }
        osg::Callback* getNestedCallback() { return _nestedCallback.get(); }
        const osg::Callback* getNestedCallback() const { return _nestedCallback.get(); }

        inline void addNestedCallback(osg::Callback* nc)
        {
            if (nc)
            {
                if (_nestedCallback.valid())
                {
                    _nestedCallback->addNestedCallback(nc);
                }
                else
                {
                    _nestedCallback = nc;
                }
            }
        }

        inline void removeNestedCallback(osg::Callback* nc)
        {
            if (nc)
            {
                if (_nestedCallback==nc)
                {
                    ref_ptr<osg::Callback> new_nested_callback = _nestedCallback->getNestedCallback();
                    _nestedCallback->setNestedCallback(0);
                    _nestedCallback = new_nested_callback;
                }
                else if (_nestedCallback.valid())
                {
                    _nestedCallback->removeNestedCallback(nc);
                }
            }
        }

    protected:

        virtual ~Callback() {}
        ref_ptr<Callback> _nestedCallback;
};

typedef std::vector< osg::ref_ptr<osg::Object> > Parameters;

/** Callback for attaching a script to a Node's via there UserDataContainer for the purpose of overriding class methods within scripts.*/
class OSG_EXPORT CallbackObject : public virtual osg::Callback
{
public:
    CallbackObject() {}
    CallbackObject(const std::string& name) { setName(name); }
    CallbackObject(const CallbackObject& co, const osg::CopyOp copyop=osg::CopyOp::SHALLOW_COPY):
        osg::Object(co, copyop),
        osg::Callback(co,copyop) {}

    META_Object(osg, CallbackObject);

    virtual CallbackObject* asCallbackObject() { return this; }
    virtual const CallbackObject* asCallbackObject() const { return this; }

    /** override Callback::run() entry point to adapt to CallbackObject::run(..) method.*/
    bool run(osg::Object* object, osg::Object* data);

    inline bool run(osg::Object* object) const
    {
        osg::Parameters inputParameters;
        osg::Parameters outputParameters;
        return run(object, inputParameters, outputParameters);
    }

    virtual bool run(osg::Object* object, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const;

};

/** Convenience function for getting the CallbackObject associated with specified name from an Object's UserDataContainer.*/
inline CallbackObject* getCallbackObject(osg::Object* object, const std::string& name)
{
    osg::UserDataContainer* udc = object->getUserDataContainer();
    if (!udc) return 0;

    osg::Object* obj = udc->getUserObject(name);
    if (!obj) return 0;

    return obj->asCallbackObject();
}


/** Convenience function for getting the CallbackObject associated with specified name from an Object's UserDataContainer.*/
inline const CallbackObject* getCallbackObject(const osg::Object* object, const std::string& name)
{
    const osg::UserDataContainer* udc = object->getUserDataContainer();
    if (!udc) return 0;

    const osg::Object* obj = udc->getUserObject(name);
    if (!obj) return 0;

    return obj->asCallbackObject();
}

/** Call run(..) on named CallbackObjects attached to specified Object. Return true if at least one CallbackObject has been successfully invoked.*/
inline bool runNamedCallbackObjects(osg::Object* object, const std::string& name, osg::Parameters& inputParameters, osg::Parameters& outputParameters)
{
    bool result = false;
    osg::UserDataContainer* udc = object->getUserDataContainer();
    if (udc)
    {
        for(unsigned int i = 0; i<udc->getNumUserObjects(); ++i)
        {
            osg::Object* obj = udc->getUserObject(i);
            if (obj && obj->getName()==name)
            {
                osg::CallbackObject* co = obj->asCallbackObject();
                if (co) result = co->run(object, inputParameters, outputParameters) | result;
            }
        }
    }

    return result;
}



// forward declare
class Node;
class NodeVisitor;


/** Deprecated. */
class OSG_EXPORT NodeCallback : public virtual Callback {

    public :

        NodeCallback(){}

        NodeCallback(const NodeCallback& nc,const CopyOp& copyop):
            Object(nc, copyop),
            Callback(nc, copyop) {}

        META_Object(osg,NodeCallback);

        virtual NodeCallback* asNodeCallback() { return this; }
        virtual const NodeCallback* asNodeCallback() const { return this; }

        /** NodeCallback overrides the Callback::run() method to adapt it the old style NodeCallback::operator()(Node* node, NodeVisitor* nv) method.*/
        virtual bool run(osg::Object* object, osg::Object* data);

        /** Callback method called by the NodeVisitor when visiting a node.*/
        virtual void operator()(Node* node, NodeVisitor* nv);

    protected:

        virtual ~NodeCallback() {}
};

// forward declare
class StateAttribute;

/** Deprecated. */
class OSG_EXPORT StateAttributeCallback : public virtual osg::Callback
{
    public:
        StateAttributeCallback() {}

        StateAttributeCallback(const StateAttributeCallback& org,const CopyOp& copyop) :
            Object(org, copyop),
            Callback(org, copyop) {}

        META_Object(osg,StateAttributeCallback);

        virtual StateAttributeCallback* asStateAttributeCallback() { return this; }
        virtual const StateAttributeCallback* asStateAttributeCallback() const { return this; }

        /** override Callback::run() entry point to adapt to StateAttributeCallback::run(..) method.*/
        virtual bool run(osg::Object* object, osg::Object* data);

        /** do customized update code.*/
        virtual void operator () (StateAttribute*, NodeVisitor*) {}
};

// forward declare
class Uniform;

/** Deprecated. */
class OSG_EXPORT UniformCallback : public virtual osg::Callback
{
public:
    UniformCallback() {}

    UniformCallback(const UniformCallback& org, const CopyOp& copyop) :
        Object(org, copyop),
        Callback(org, copyop) {}

    META_Object(osg, UniformCallback);

    virtual UniformCallback* asUniformCallback() { return this; }
    virtual const UniformCallback* asUniformCallback() const { return this; }

    /** override Callback::run() entry point to adapt to UniformCallback::run(..) method.*/
    virtual bool run(osg::Object* object, osg::Object* data);

    /** do customized update code.*/
    virtual void operator () (Uniform*, NodeVisitor*) {}
};


// forward declare
class Drawable;
class State;
class RenderInfo;

class OSG_EXPORT DrawableUpdateCallback : public virtual Callback
{
public:
    DrawableUpdateCallback() {}

    DrawableUpdateCallback(const DrawableUpdateCallback& org,const CopyOp& copyop):
    Object(org, copyop),
    Callback(org, copyop) {}

    META_Object(osg,DrawableUpdateCallback);

    virtual DrawableUpdateCallback* asDrawableUpdateCallback() { return this; }
    virtual const DrawableUpdateCallback* asDrawableUpdateCallback() const { return this; }

    /** override Callback::run() entry point to adapt to StateAttributeCallback::run(..) method.*/
    virtual bool run(osg::Object* object, osg::Object* data);

    /** do customized update code.*/
    virtual void update(osg::NodeVisitor*, osg::Drawable*) {}
};


class OSG_EXPORT DrawableEventCallback : public virtual Callback
{
public:
    DrawableEventCallback() {}

    DrawableEventCallback(const DrawableEventCallback& org, const CopyOp& copyop) :
        Object(org, copyop),
        Callback(org, copyop) {}

    META_Object(osg,DrawableEventCallback);

    virtual DrawableEventCallback* asDrawableEventCallback() { return this; }
    virtual const DrawableEventCallback* asDrawableEventCallback() const { return this; }

    /** override Callback::run() entry point to adapt to StateAttributeCallback::run(..) method.*/
    virtual bool run(osg::Object* object, osg::Object* data);

    /** do customized Event code. */
    virtual void event(osg::NodeVisitor*, osg::Drawable*) {}
};

class OSG_EXPORT DrawableCullCallback : public virtual Callback
{
public:
    DrawableCullCallback() {}

    DrawableCullCallback(const DrawableCullCallback& org, const CopyOp& copyop) :
        Object(org, copyop),
        Callback(org, copyop) {}

    META_Object(osg,DrawableCullCallback);

    virtual DrawableCullCallback* asDrawableCullCallback() { return this; }
    virtual const DrawableCullCallback* asDrawableCullCallback() const { return this; }

    // just use the standard run implementation to passes run onto any nested callbacks.
    using Callback::run;

    /** deprecated.*/
    virtual bool cull(osg::NodeVisitor*, osg::Drawable*, osg::State*) const { return false; }

    /** do customized cull code, return true if drawable should be culled.*/
    virtual bool cull(osg::NodeVisitor* nv, osg::Drawable* drawable, osg::RenderInfo* renderInfo) const;
};


} // namespace

#endif

