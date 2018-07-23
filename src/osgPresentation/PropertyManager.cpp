/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2018 Robert Osfield
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

#include <osgPresentation/PropertyManager>
#include <osg/io_utils>

using namespace osgPresentation;

const osg::Object* osgPresentation::getUserObject(const osg::NodePath& nodepath, const std::string& name)
{
    for(osg::NodePath::const_reverse_iterator itr = nodepath.rbegin();
        itr != nodepath.rend();
        ++itr)
    {
        const osg::UserDataContainer* udc = (*itr)->getUserDataContainer();
        const osg::Object* object = udc ? udc->getUserObject(name) : 0;
        if (object) return object;
    }
    return 0;
}

bool osgPresentation::containsPropertyReference(const std::string& str)
{
    return (str.find('$')!=std::string::npos);
}



void PropertyAnimation::reset()
{
    _firstTime = DBL_MAX;
    _pauseTime = DBL_MAX;

    OSG_NOTICE<<"PropertyAnimation::reset()"<<std::endl;
}

void PropertyAnimation::setPause(bool pause)
{
    OSG_NOTICE<<"PropertyAnimation::setPause("<<pause<<")"<<std::endl;

    if (_pause==pause)
    {
        return;
    }

    _pause = pause;

    if (_firstTime==DBL_MAX) return;

    if (_pause)
    {
        _pauseTime = _latestTime;
    }
    else
    {
        _firstTime += (_latestTime-_pauseTime);
    }
}

double PropertyAnimation::getAnimationTime() const
{
    return _latestTime-_firstTime;
}


void PropertyAnimation::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    if (nv->getVisitorType()==osg::NodeVisitor::UPDATE_VISITOR &&
        nv->getFrameStamp())
    {
        double time = nv->getFrameStamp()->getSimulationTime();
        _latestTime = time;

        if (!_pause)
        {
            // Only update _firstTime the first time, when its value is still DBL_MAX
            if (_firstTime==DBL_MAX) _firstTime = time;
            update(*node);
        }
    }

    traverse(node, nv);
}

class MySetValueVisitor : public osg::ValueObject::SetValueVisitor
{
public:

    MySetValueVisitor(double in_r1, double in_r2, osg::ValueObject* in_object2):
        _r1(in_r1), _r2(in_r2), _object2(in_object2)
    {
    }

    template<typename T>
    void combineRealUserValue(T& value) const
    {
        typedef osg::TemplateValueObject<T> UserValueObject;
        const UserValueObject* uvo = _object2 ? dynamic_cast<const UserValueObject*>(_object2) : 0;
        if (uvo)
        {
            value = value*_r1 + uvo->getValue()*_r2;
        }
        OSG_NOTICE<<"combineRealUserValue r1="<<_r1<<", r2="<<_r2<<", value="<<value<<std::endl;
    }

    template<typename T>
    void combineIntegerUserValue(T& value) const
    {
        typedef osg::TemplateValueObject<T> UserValueObject;
        const UserValueObject* uvo = _object2 ? dynamic_cast<const UserValueObject*>(_object2) : 0;
        if (uvo)
        {
            value = static_cast<T>(static_cast<double>(value)*_r1 + static_cast<double>(uvo->getValue())*_r2);
        }
        OSG_NOTICE<<"combineIntegerUserValue "<<value<<std::endl;
    }

    template<typename T>
    void combineDiscretUserValue(T& value) const
    {
        if (_r1<_r2) // choose value2 if possible
        {
            typedef osg::TemplateValueObject<T> UserValueObject;
            const UserValueObject* uvo = _object2 ? dynamic_cast<const UserValueObject*>(_object2) : 0;
            if (uvo)
            {
                value = uvo->getValue();
            }
        }
        OSG_NOTICE<<"combineDiscretUserValue "<<value<<std::endl;
    }

    template<typename T>
    void combineRotationUserValue(T& /*value*/) const
    {
        OSG_NOTICE<<"combineRotationUserValue TODO - do slerp"<<std::endl;
    }

    template<typename T>
    void combinePlaneUserValue(T& /*value*/) const
    {
        OSG_NOTICE<<"combinePlaneUserValue TODO"<<std::endl;
    }

    template<typename T>
    void combineMatrixUserValue(T& /*value*/) const
    {
        OSG_NOTICE<<"combineMatrixUserValue TODO - decomposs into translate, rotation and scale and then interpolate."<<std::endl;
    }


    virtual void apply(bool& value)             { combineDiscretUserValue(value); }
    virtual void apply(char& value)             { combineDiscretUserValue(value); }
    virtual void apply(unsigned char& value)    { combineDiscretUserValue(value); }
    virtual void apply(short& value)            { combineIntegerUserValue(value); }
    virtual void apply(unsigned short& value)   { combineIntegerUserValue(value); }
    virtual void apply(int& value)              { combineIntegerUserValue(value); }
    virtual void apply(unsigned int& value)     { combineIntegerUserValue(value); }
    virtual void apply(float& value)            { combineRealUserValue(value); }
    virtual void apply(double& value)           { combineRealUserValue(value); }
    virtual void apply(std::string& value)      { combineDiscretUserValue(value); }
    virtual void apply(osg::Vec2f& value)       { combineRealUserValue(value); }
    virtual void apply(osg::Vec3f& value)       { combineRealUserValue(value); }
    virtual void apply(osg::Vec4f& value)       { combineRealUserValue(value); }
    virtual void apply(osg::Vec2d& value)       { combineRealUserValue(value); }
    virtual void apply(osg::Vec3d& value)       { combineRealUserValue(value); }
    virtual void apply(osg::Vec4d& value)       { combineRealUserValue(value); }
    virtual void apply(osg::Quat& value)        { combineRotationUserValue(value); }
    virtual void apply(osg::Plane& value)       { combinePlaneUserValue(value); }
    virtual void apply(osg::Matrixf& value)     { combineMatrixUserValue(value); }
    virtual void apply(osg::Matrixd& value)     { combineMatrixUserValue(value); }

    virtual ~MySetValueVisitor() {}

    double _r1, _r2;
    osg::ValueObject* _object2;
};

void PropertyAnimation::update(osg::Node& node)
{
    OSG_NOTICE<<"PropertyAnimation::update()"<<this<<std::endl;

    double time = getAnimationTime();

    if (_keyFrameMap.empty()) return;

    KeyFrameMap::const_iterator itr = _keyFrameMap.lower_bound(time);
    if (itr==_keyFrameMap.begin())
    {
        // need to copy first UserDataContainer
        OSG_NOTICE<<"PropertyAnimation::update() : copy first UserDataContainer"<<std::endl;
        assign(node.getOrCreateUserDataContainer(), itr->second.get());
    }
    else if (itr!=_keyFrameMap.end())
    {
        KeyFrameMap::const_iterator itr_1 = itr; --itr_1;
        KeyFrameMap::const_iterator itr_2 = itr;

        // delta_time = second.time - first.time
        double delta_time = itr_2->first - itr_1->first;
        double r1, r2;
        if (delta_time==0.0)
        {
            r1 = 0.5;
            r2 = 0.5;
        }
        else
        {
            r2 = (time - itr_1->first)/delta_time;
            r1 = 1.0-r2;
        }

        osg::UserDataContainer* p1 = itr_1->second.get();
        osg::UserDataContainer* p2 = itr_2->second.get();

        // clone all the properties from p1;

        osg::ref_ptr<osg::UserDataContainer> destination = node.getOrCreateUserDataContainer();

        assign(destination.get(), p1);

        for(unsigned int i2=0; i2<p2->getNumUserObjects(); ++i2)
        {
            osg::Object* obj_2 = p2->getUserObject(i2);
            unsigned int i1 = p1->getUserObjectIndex(obj_2->getName());
            if (i1<p1->getNumUserObjects())
            {
                osg::Object* obj_1 = p1->getUserObject(i1);
                osg::ValueObject* valueobject_1 = dynamic_cast<osg::ValueObject*>(obj_1);
                osg::ValueObject* valueobject_2 = dynamic_cast<osg::ValueObject*>(obj_2);
                if (valueobject_1 && valueobject_2)
                {
                    osg::ref_ptr<osg::ValueObject> vo = osg::clone(valueobject_1);
                    MySetValueVisitor mySetValue(r1, r2, valueobject_2);
                    vo->set(mySetValue);
                    assign(destination.get(), vo.get());
                }
                else if (obj_1)
                {
                    assign(destination.get(), obj_1);
                }
                else if (obj_2)
                {
                    assign(destination.get(), obj_2);
                }
            }
            else
            {
                // need to insert property;
                assign(destination.get(), obj_2);
            }

        }

    }
    else // (itr==_keyFrameMap.end())
    {
        OSG_NOTICE<<"PropertyAnimation::update() : copy last UserDataContainer"<<std::endl;
        assign(node.getOrCreateUserDataContainer(), _keyFrameMap.rbegin()->second.get());
    }

}

void PropertyAnimation::assign(osg::UserDataContainer* destination, osg::UserDataContainer* source)
{
    if (!destination) return;
    if (!source) return;

    for(unsigned int i=0; i<source->getNumUserObjects(); ++i)
    {
        assign(destination, source->getUserObject(i));
    }
}

void PropertyAnimation::assign(osg::UserDataContainer* udc, osg::Object* obj)
{
    if (!obj) return;

    unsigned int index = udc->getUserObjectIndex(obj);
    if (index != udc->getNumUserObjects())
    {
        OSG_NOTICE<<"Object already assigned to UserDataContainer"<<std::endl;
        return;
    }

    index = udc->getUserObjectIndex(obj->getName());
    if (index != udc->getNumUserObjects())
    {
        OSG_NOTICE<<"Replacing object in UserDataContainer"<<std::endl;
        udc->setUserObject(index, obj);
        return;
    }

    OSG_NOTICE<<"Assigned object to UserDataContainer"<<std::endl;
    udc->addUserObject(obj);
}



void ImageSequenceUpdateCallback::operator()(osg::Node* node, osg::NodeVisitor* nv)
{
    float x;
    if (_propertyManager->getProperty(_propertyName,x))
    {
        double xMin = -1.0;
        double xMax = 1.0;
        double position = ((double)x-xMin)/(xMax-xMin)*_imageSequence->getLength();

        _imageSequence->seek(position);
    }
    else
    {
        OSG_INFO<<"ImageSequenceUpdateCallback::operator() Could not find property : "<<_propertyName<<std::endl;
    }

    // note, callback is responsible for scenegraph traversal so
    // they must call traverse(node,nv) to ensure that the
    // scene graph subtree (and associated callbacks) are traversed.
    traverse(node,nv);
}


bool PropertyEventCallback::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{

    bool mouseEvent =  (ea.getEventType()==osgGA::GUIEventAdapter::MOVE ||
                        ea.getEventType()==osgGA::GUIEventAdapter::DRAG ||
                        ea.getEventType()==osgGA::GUIEventAdapter::PUSH ||
                        ea.getEventType()==osgGA::GUIEventAdapter::RELEASE);
    if(mouseEvent)
    {
        _propertyManager->setProperty("mouse.x",ea.getX());
        _propertyManager->setProperty("mouse.x_normalized",ea.getXnormalized());
        _propertyManager->setProperty("mouse.y",ea.getX());
        _propertyManager->setProperty("mouse.y_normalized",ea.getYnormalized());
    }

    return false;
}
