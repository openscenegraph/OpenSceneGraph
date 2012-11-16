/* -*-c++-*- Present3D - Copyright (C) 1999-2006 Robert Osfield
 *
 * This software is open source and may be redistributed and/or modified under
 * the terms of the GNU General Public License (GPL) version 2.0.
 * The full license is in LICENSE.txt file included with this distribution,.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * include LICENSE.txt for more details.
*/

#include <osgPresentation/PropertyManager>

using namespace osgPresentation;



void PropertyAnimation::reset()
{
    _firstTime = DBL_MAX;
    _pauseTime = DBL_MAX;
}

void PropertyAnimation::setPause(bool pause)
{
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
            update();
        }
    }

    traverse(node, nv);
}

class MySetValueVisitor : public osg::ValueObject::SetValueVisitor
{
public:

    MySetValueVisitor(double r1, double r2, osg::ValueObject* object2)
    {
    }

    template<typename T>
    void combineRealUserValue(T& value) const
    {
        typedef osg::TemplateValueObject<T> UserValueObject;
        const UserValueObject* uvo = object2 ? dynamic_cast<const UserValueObject*>(object2) : 0;
        if (uvo)
        {
            value = value*_r1 + uvo->getValue()*_r2;
        }
    }

    template<typename T>
    void combineIntegerUserValue(T& value) const
    {
        typedef osg::TemplateValueObject<T> UserValueObject;
        const UserValueObject* uvo = object2 ? dynamic_cast<const UserValueObject*>(object2) : 0;
        if (uvo)
        {
            value = static_cast<T>(static_cast<double>(value)*_r1 + static_cast<double>(uvo->getValue())*_r2);
        }
    }

    template<typename T>
    void combineDiscretUserValue(T& value) const
    {
        if (_r1<_r2) // choose value2 if possible
        {
            typedef osg::TemplateValueObject<T> UserValueObject;
            const UserValueObject* uvo = object2 ? dynamic_cast<const UserValueObject*>(object2) : 0;
            if (uvo)
            {
                value = uvo->getValue();
            }
        }
    }

    template<typename T>
    void combineRotationUserValue(T& value) const
    {
        OSG_NOTICE<<"combineRotationUserValue TODO - do slerp"<<std::endl;
    }

    template<typename T>
    void combinePlaneUserValue(T& value) const
    {
        OSG_NOTICE<<"combinePlaneUserValue TODO"<<std::endl;
    }

    template<typename T>
    void combineMatrixUserValue(T& value) const
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

    double _r1, _r2;
    osg::ValueObject* object2;
};

void PropertyAnimation::update()
{
    double time = getAnimationTime();

    osg::ref_ptr<osg::UserDataContainer> result;

    if (_keyFrameMap.empty()) return;
    
    KeyFrameMap::const_iterator itr = _keyFrameMap.lower_bound(time);
    if (itr==_keyFrameMap.begin())
    {
        // need to copy first UserDataContainer
        OSG_NOTICE<<"PropertyAnimation::update() : copy first UserDataContainer"<<std::endl;
        result = osg::clone(itr->second.get(), osg::CopyOp::DEEP_COPY_ALL);
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
            r1 = (time - itr_1->first)/delta_time;
            r2 = 1.0-r1;
        }

        osg::UserDataContainer* p1 = itr_1->second.get();
        osg::UserDataContainer* p2 = itr_2->second.get();

        // clone all the properties from p1;
        result = osg::clone(p1, osg::CopyOp::DEEP_COPY_ALL);
        
        for(unsigned int i2=0; i2<p2->getNumUserObjects(); ++i2)
        {
            osg::Object* obj_2 = p2->getUserObject(i2);
            unsigned int i1 = result->getUserObjectIndex(obj_2->getName());
            if (i1<result->getNumUserObjects())
            {
                osg::Object* obj_1 = result->getUserObject(i1);
                osg::ValueObject* valueobject_1 = dynamic_cast<osg::ValueObject*>(obj_1);
                osg::ValueObject* valueobject_2 = dynamic_cast<osg::ValueObject*>(obj_2);
                if (valueobject_1)
                {
                    MySetValueVisitor mySetValue(r1, r2, valueobject_2);
                    valueobject_1->set(mySetValue);
                }
            }
            else
            {
                // need to insert property;
                result->addUserObject(obj_2->clone(osg::CopyOp::DEEP_COPY_ALL));
            }
            
        }
        

        OSG_NOTICE<<"PropertyAnimation::update() : Need to interpolate between two UserDataContainer, r1="<<r1<<", r2="<<r2<<std::endl;

    }
    else // (itr==_keyFrameMap.end())
    {
        OSG_NOTICE<<"PropertyAnimation::update() : copy last UserDataContainer"<<std::endl;
        result = osg::clone(_keyFrameMap.rbegin()->second.get(), osg::CopyOp::DEEP_COPY_ALL);
    }
    
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
