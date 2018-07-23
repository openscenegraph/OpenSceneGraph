/* -*-c++-*- OpenSceneGraph - Copyright (C) 2011 Robert Osfield
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

#ifndef OSG_VALUEOBJECT
#define OSG_VALUEOBJECT 1

#include <osg/Object>
#include <osg/UserDataContainer>
#include <osg/BoundingBox>
#include <osg/BoundingSphere>

#include <typeinfo>

namespace osg {

// forward declare core OSG math classes
class Vec2b;
class Vec3b;
class Vec4b;
class Vec2ub;
class Vec3ub;
class Vec4ub;

class Vec2s;
class Vec3s;
class Vec4s;
class Vec2us;
class Vec3us;
class Vec4us;

class Vec2i;
class Vec3i;
class Vec4i;
class Vec2ui;
class Vec3ui;
class Vec4ui;

class Vec2f;
class Vec3f;
class Vec4f;

class Vec2d;
class Vec3d;
class Vec4d;

class Quat;
class Plane;
class Matrixf;
class Matrixd;

template<typename T>
class GetScalarValue;
template<typename T>
class SetScalarValue;

class ValueObject : public Object
{
    public:

        ValueObject() : Object(true) {}
        ValueObject(const std::string& name) : Object(true) { setName(name); }
        ValueObject(const ValueObject& rhs, const osg::CopyOp copyop=osg::CopyOp::SHALLOW_COPY): Object(rhs,copyop) {}

        META_Object(osg, ValueObject)

        /** Convert 'this' into a ValueObject pointer if Object is a ValueObject, otherwise return 0.
          * Equivalent to dynamic_cast<ValueObject*>(this).*/
        virtual ValueObject* asValueObject() { return this; }

        /** Convert 'this' into a ValueObject pointer if Object is a ValueObject, otherwise return 0.
          * Equivalent to dynamic_cast<ValueObject*>(this).*/
        virtual const ValueObject* asValueObject() const { return this; }

        class GetValueVisitor
        {
        public:
            virtual ~GetValueVisitor() {}
            virtual void apply(bool /*in_value*/) {}
            virtual void apply(char /*in_value*/) {}
            virtual void apply(unsigned char /*in_value*/) {}
            virtual void apply(short /*in_value*/) {}
            virtual void apply(unsigned short /*in_value*/) {}
            virtual void apply(int /*in_value*/) {}
            virtual void apply(unsigned int /*in_value*/) {}
            virtual void apply(float /*in_value*/) {}
            virtual void apply(double /*in_value*/) {}
            virtual void apply(const std::string& /*in_value*/) {}

            virtual void apply(const osg::Vec2b& /*in_value*/) {}
            virtual void apply(const osg::Vec3b& /*in_value*/) {}
            virtual void apply(const osg::Vec4b& /*in_value*/) {}

            virtual void apply(const osg::Vec2ub& /*in_value*/) {}
            virtual void apply(const osg::Vec3ub& /*in_value*/) {}
            virtual void apply(const osg::Vec4ub& /*in_value*/) {}

            virtual void apply(const osg::Vec2s& /*in_value*/) {}
            virtual void apply(const osg::Vec3s& /*in_value*/) {}
            virtual void apply(const osg::Vec4s& /*in_value*/) {}

            virtual void apply(const osg::Vec2us& /*in_value*/) {}
            virtual void apply(const osg::Vec3us& /*in_value*/) {}
            virtual void apply(const osg::Vec4us& /*in_value*/) {}

            virtual void apply(const osg::Vec2i& /*in_value*/) {}
            virtual void apply(const osg::Vec3i& /*in_value*/) {}
            virtual void apply(const osg::Vec4i& /*in_value*/) {}

            virtual void apply(const osg::Vec2ui& /*in_value*/) {}
            virtual void apply(const osg::Vec3ui& /*in_value*/) {}
            virtual void apply(const osg::Vec4ui& /*in_value*/) {}

            virtual void apply(const osg::Vec2f& /*in_value*/) {}
            virtual void apply(const osg::Vec3f& /*in_value*/) {}
            virtual void apply(const osg::Vec4f& /*in_value*/) {}

            virtual void apply(const osg::Vec2d& /*in_value*/) {}
            virtual void apply(const osg::Vec3d& /*in_value*/) {}
            virtual void apply(const osg::Vec4d& /*in_value*/) {}

            virtual void apply(const osg::Quat& /*in_value*/) {}
            virtual void apply(const osg::Plane& /*in_value*/) {}
            virtual void apply(const osg::Matrixf& /*in_value*/) {}
            virtual void apply(const osg::Matrixd& /*in_value*/) {}
            virtual void apply(const osg::BoundingBoxf& /*in_value*/) {}
            virtual void apply(const osg::BoundingBoxd& /*in_value*/) {}
            virtual void apply(const osg::BoundingSpheref& /*in_value*/) {}
            virtual void apply(const osg::BoundingSphered& /*in_value*/) {}
        };


        class SetValueVisitor
        {
        public:
            virtual ~SetValueVisitor() {}
            virtual void apply(bool& /*in_value*/) {}
            virtual void apply(char& /*in_value*/) {}
            virtual void apply(unsigned char& /*in_value*/) {}
            virtual void apply(short& /*in_value*/) {}
            virtual void apply(unsigned short& /*in_value*/) {}
            virtual void apply(int& /*in_value*/) {}
            virtual void apply(unsigned int& /*in_value*/) {}
            virtual void apply(float& /*in_value*/) {}
            virtual void apply(double& /*in_value*/) {}
            virtual void apply(std::string& /*in_value*/) {}

            virtual void apply(osg::Vec2b& /*in_value*/) {}
            virtual void apply(osg::Vec3b& /*in_value*/) {}
            virtual void apply(osg::Vec4b& /*in_value*/) {}

            virtual void apply(osg::Vec2ub& /*in_value*/) {}
            virtual void apply(osg::Vec3ub& /*in_value*/) {}
            virtual void apply(osg::Vec4ub& /*in_value*/) {}

            virtual void apply(osg::Vec2s& /*in_value*/) {}
            virtual void apply(osg::Vec3s& /*in_value*/) {}
            virtual void apply(osg::Vec4s& /*in_value*/) {}

            virtual void apply(osg::Vec2us& /*in_value*/) {}
            virtual void apply(osg::Vec3us& /*in_value*/) {}
            virtual void apply(osg::Vec4us& /*in_value*/) {}

            virtual void apply(osg::Vec2i& /*in_value*/) {}
            virtual void apply(osg::Vec3i& /*in_value*/) {}
            virtual void apply(osg::Vec4i& /*in_value*/) {}

            virtual void apply(osg::Vec2ui& /*in_value*/) {}
            virtual void apply(osg::Vec3ui& /*in_value*/) {}
            virtual void apply(osg::Vec4ui& /*in_value*/) {}

            virtual void apply(osg::Vec2f& /*in_value*/) {}
            virtual void apply(osg::Vec3f& /*in_value*/) {}
            virtual void apply(osg::Vec4f& /*in_value*/) {}

            virtual void apply(osg::Vec2d& /*in_value*/) {}
            virtual void apply(osg::Vec3d& /*in_value*/) {}
            virtual void apply(osg::Vec4d& /*in_value*/) {}

            virtual void apply(osg::Quat& /*in_value*/) {}
            virtual void apply(osg::Plane& /*in_value*/) {}
            virtual void apply(osg::Matrixf& /*in_value*/) {}
            virtual void apply(osg::Matrixd& /*in_value*/) {}
            virtual void apply(osg::BoundingBoxf& /*in_value*/) {}
            virtual void apply(osg::BoundingBoxd& /*in_value*/) {}
            virtual void apply(osg::BoundingSpheref& /*in_value*/) {}
            virtual void apply(osg::BoundingSphered& /*in_value*/) {}
        };

        virtual bool get(GetValueVisitor& /*gvv*/) const { return false; }
        virtual bool set(SetValueVisitor& /*gvv*/) { return false; }

        template<typename T>
        bool getScalarValue(T& value) { GetScalarValue<T> gsv; if (get(gsv) &&  gsv.set) { value = gsv.value; return true; } else return false; }

        template<typename T>
        bool setScalarValue(T value) { SetScalarValue<T> ssv(value); return set(ssv) && ssv.set; }

protected:
        virtual ~ValueObject() {}
};

template<typename T>
class GetScalarValue : public ValueObject::GetValueVisitor
{
public:

    GetScalarValue() : set(false), value(0) {}

    bool set;
    T value;

    virtual void apply(bool in_value) { value = in_value ? 1 : 0; set = true; }
    virtual void apply(char in_value) { value = in_value; set = true; }
    virtual void apply(unsigned char in_value) { value = in_value; set = true; }
    virtual void apply(short in_value) { value = in_value; set = true; }
    virtual void apply(unsigned short in_value) { value = in_value; set = true; }
    virtual void apply(int in_value) { value = in_value; set = true; }
    virtual void apply(unsigned int in_value) { value = in_value; set = true; }
    virtual void apply(float in_value) { value = in_value; set = true; }
    virtual void apply(double in_value) { value = in_value; set = true; }
};
template<>
class GetScalarValue <bool> : public ValueObject::GetValueVisitor
{
public:

    GetScalarValue() : set(false), value(0) {}

    bool set;
    bool value;

    virtual void apply(bool in_value) { value = in_value; set = true; }
    virtual void apply(char in_value) { value = in_value != 0; set = true; }
    virtual void apply(unsigned char in_value) { value = in_value != 0; set = true; }
    virtual void apply(short in_value) { value = in_value != 0; set = true; }
    virtual void apply(unsigned short in_value) { value = in_value != 0; set = true; }
    virtual void apply(int in_value) { value = in_value != 0; set = true; }
    virtual void apply(unsigned int in_value) { value = in_value != 0; set = true; }
    virtual void apply(float in_value) { value = in_value != 0.0f; set = true; }
    virtual void apply(double in_value) { value = in_value != 0.0; set = true; }
};

template<typename T>
class SetScalarValue : public ValueObject::SetValueVisitor
{
public:

    SetScalarValue(T in_value) : set(false), value(in_value) {}

    bool set;
    T value;

    virtual void apply(bool& in_value) { in_value=(value!=0); set = true;}
    virtual void apply(char& in_value) { in_value=value; set = true;}
    virtual void apply(unsigned char& in_value) { in_value=value; set = true;}
    virtual void apply(short& in_value) { in_value=value; set = true;}
    virtual void apply(unsigned short& in_value) { in_value=value; set = true;}
    virtual void apply(int& in_value) { in_value=value; set = true;}
    virtual void apply(unsigned int& in_value) { in_value=value; set = true;}
    virtual void apply(float& in_value) { in_value=value; set = true;}
    virtual void apply(double& in_value) { in_value=value; set = true;}
};

template< typename T >
struct ValueObjectClassNameTrait
{
    static const char* className() { return "TemplateValueObject"; }
};


template< typename T >
class TemplateValueObject : public ValueObject
{
    public:

        TemplateValueObject():
            ValueObject(),
            _value() {}

        TemplateValueObject(const T& value) :
            ValueObject(),
            _value(value) {}

        TemplateValueObject(const std::string& name, const T& value) :
            ValueObject(name),
            _value(value) {}

        TemplateValueObject(const TemplateValueObject& rhs, const osg::CopyOp copyop=osg::CopyOp::SHALLOW_COPY) :
            ValueObject(rhs,copyop),
            _value(rhs._value) {}

        virtual Object* cloneType() const { return new TemplateValueObject(); }
        virtual Object* clone(const CopyOp& copyop) const { return new TemplateValueObject(*this, copyop); }
        virtual bool isSameKindAs(const Object* obj) const { return dynamic_cast<const TemplateValueObject*>(obj)!=NULL; }
        virtual const char* libraryName() const { return "osg"; }
        virtual const char* className() const { return ValueObjectClassNameTrait<T>::className(); }

        void setValue(const T& value) { _value = value; }
        const T& getValue() const { return _value; }

        virtual bool get(GetValueVisitor& gvv) const { gvv.apply(_value); return true; }
        virtual bool set(SetValueVisitor& svv) { svv.apply(_value); return true; }

protected:

        virtual ~TemplateValueObject() {}
        static const char* s_TemplateValueObject_className;

        T _value;
};

#define META_ValueObject(TYPE,NAME) \
    template<> struct ValueObjectClassNameTrait<TYPE> { static const char* className() { return #NAME; } }; \
    typedef TemplateValueObject<TYPE> NAME;

META_ValueObject(std::string, StringValueObject)
META_ValueObject(bool, BoolValueObject)
META_ValueObject(char, CharValueObject)
META_ValueObject(unsigned char, UCharValueObject)
META_ValueObject(short, ShortValueObject)
META_ValueObject(unsigned short, UShortValueObject)
META_ValueObject(int, IntValueObject)
META_ValueObject(unsigned int, UIntValueObject)
META_ValueObject(float, FloatValueObject)
META_ValueObject(double, DoubleValueObject)
META_ValueObject(Vec2f, Vec2fValueObject)
META_ValueObject(Vec3f, Vec3fValueObject)
META_ValueObject(Vec4f, Vec4fValueObject)
META_ValueObject(Vec2d, Vec2dValueObject)
META_ValueObject(Vec3d, Vec3dValueObject)
META_ValueObject(Vec4d, Vec4dValueObject)
META_ValueObject(Quat, QuatValueObject)
META_ValueObject(Plane, PlaneValueObject)
META_ValueObject(Matrixf, MatrixfValueObject)
META_ValueObject(Matrixd, MatrixdValueObject)
META_ValueObject(BoundingBoxf, BoundingBoxfValueObject)
META_ValueObject(BoundingBoxd, BoundingBoxdValueObject)
META_ValueObject(BoundingSpheref, BoundingSpherefValueObject)
META_ValueObject(BoundingSphered, BoundingSpheredValueObject)

/** provide implementation of osg::Object::getUserValue(..) template*/
template<typename T>
bool osg::Object::getUserValue(const std::string& name, T& value) const
{
    typedef TemplateValueObject<T> UserValueObject;

    const osg::UserDataContainer* udc = asUserDataContainer();
    if (!udc) udc = _userDataContainer;

    if (!udc) return false;

    const Object* obj = udc->getUserObject(name);
    if (obj && typeid(*obj)==typeid(UserValueObject))
    {
        const UserValueObject* uvo = static_cast<const UserValueObject*>(obj);
        value = uvo->getValue();
        return true;
    }
    else
    {
        return false;
    }
}

/** provide implementation of osg::Object::setUserValue(..) template.*/
template<typename T>
void osg::Object::setUserValue(const std::string& name, const T& value)
{
    typedef TemplateValueObject<T> UserValueObject;

    osg::UserDataContainer* udc = asUserDataContainer();
    if (!udc)
    {
        getOrCreateUserDataContainer();
        udc = _userDataContainer;
    }

    unsigned int i = udc->getUserObjectIndex(name);
    if (i<udc->getNumUserObjects())
    {
        Object* obj = udc->getUserObject(i);
        if (typeid(*obj)==typeid(UserValueObject))
        {
            UserValueObject* uvo = static_cast<UserValueObject*>(obj);
            uvo->setValue(value);
        }
        else
        {
            udc->setUserObject(i, new UserValueObject(name, value));
        }
    }
    else
    {
        udc->addUserObject(new UserValueObject(name,value));
    }
}


template<class P, class T>
T* getOrCreateUserObjectOfType(P* parent)
{
    T* object=0;
    const char* name = typeid(T).name();
    osg::UserDataContainer* udc = parent->getOrCreateUserDataContainer();
    unsigned int index = udc->getUserObjectIndex(name);
    if (index<udc->getNumUserObjects())
    {
        osg::Object* userObject = udc->getUserObject(index);
        if (typeid(*userObject)==typeid(T))
        {
            object = static_cast<T*>(userObject);
            // OSG_NOTICE<<"Reusing "<<name<<std::endl;
        }
        else
        {
            // OSG_NOTICE<<"Replacing "<<name<<", original object "<<userObject->className()<<std::endl;

            object = new T;
            object->setName(name);
            udc->setUserObject(index, object);
        }
    }
    else
    {
        object = new T;
        object->setName(name);
        udc->addUserObject(object);
        // OSG_NOTICE<<"Creating new "<<name<<std::endl;
    }
    return object;
}


}
#endif

