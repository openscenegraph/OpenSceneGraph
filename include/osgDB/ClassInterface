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


#ifndef OSGDB_CLASSINTERFACE
#define OSGDB_CLASSINTERFACE 1

#include <osgDB/ObjectWrapper>
#include <osgDB/Registry>
#include <osg/ValueObject>

namespace osgDB
{

template<typename T>
static osgDB::BaseSerializer::Type getTypeEnum()
{
    return osgDB::BaseSerializer::RW_UNDEFINED;
}

template<typename T>
static osgDB::BaseSerializer::Type getTypeEnumFrom(T)
{
    return getTypeEnum<T>();
}

template<typename T>
static const char* getTypeString()
{
    return "UNDEFINED";
}

template<typename T>
static const char* getTypeStringFrom(T)
{
    return getTypeString<T>();
}

extern OSGDB_EXPORT osgDB::BaseSerializer::Type getTypeEnumFromPtr(const osg::Object*);
extern OSGDB_EXPORT const char* getTypeStringFromPtr(const osg::Object*);

extern OSGDB_EXPORT osgDB::BaseSerializer::Type getTypeEnumFromPtr(const osg::Image*);
extern OSGDB_EXPORT const char* getTypeStringFromPtr(const osg::Image*);


#define DECLARE_TYPE(A,B) \
    template<> inline osgDB::BaseSerializer::Type getTypeEnum<A>() { return osgDB::BaseSerializer::RW_##B; } \
    template<> inline const char* getTypeString<A>() { return #B; }

DECLARE_TYPE(osg::Image*, IMAGE)
DECLARE_TYPE(osg::Object*, OBJECT)

DECLARE_TYPE(bool, BOOL)
DECLARE_TYPE(char, CHAR)
DECLARE_TYPE(unsigned char, UCHAR)
DECLARE_TYPE(short, SHORT)
DECLARE_TYPE(unsigned short, USHORT)
DECLARE_TYPE(int, INT)
DECLARE_TYPE(unsigned int, UINT)
DECLARE_TYPE(float, FLOAT)
DECLARE_TYPE(double, DOUBLE)

DECLARE_TYPE(osg::Vec2f, VEC2F)
DECLARE_TYPE(osg::Vec2d, VEC2D)
DECLARE_TYPE(osg::Vec3f, VEC3F)
DECLARE_TYPE(osg::Vec3d, VEC3D)
DECLARE_TYPE(osg::Vec4f, VEC4F)
DECLARE_TYPE(osg::Vec4d, VEC4D)
DECLARE_TYPE(osg::Quat, QUAT)
DECLARE_TYPE(osg::Plane, PLANE)

DECLARE_TYPE(osg::Matrixf, MATRIXF)
DECLARE_TYPE(osg::Matrixd, MATRIXD)
DECLARE_TYPE(std::string, STRING)

DECLARE_TYPE(osg::Vec2b, VEC2B)
DECLARE_TYPE(osg::Vec2ub, VEC2UB)
DECLARE_TYPE(osg::Vec2s, VEC2S)
DECLARE_TYPE(osg::Vec2us, VEC2US)
DECLARE_TYPE(osg::Vec2i, VEC2I)
DECLARE_TYPE(osg::Vec2ui, VEC2UI)

DECLARE_TYPE(osg::Vec3b, VEC3B)
DECLARE_TYPE(osg::Vec3ub, VEC3UB)
DECLARE_TYPE(osg::Vec3s, VEC3S)
DECLARE_TYPE(osg::Vec3us, VEC3US)
DECLARE_TYPE(osg::Vec3i, VEC3I)
DECLARE_TYPE(osg::Vec3ui, VEC3UI)

DECLARE_TYPE(osg::Vec4b, VEC4B)
DECLARE_TYPE(osg::Vec4ub, VEC4UB)
DECLARE_TYPE(osg::Vec4s, VEC4S)
DECLARE_TYPE(osg::Vec4us, VEC4US)
DECLARE_TYPE(osg::Vec4i, VEC4I)
DECLARE_TYPE(osg::Vec4ui, VEC4UI)

DECLARE_TYPE(osg::BoundingBoxf, BOUNDINGBOXF)
DECLARE_TYPE(osg::BoundingBoxd, BOUNDINGBOXD)

DECLARE_TYPE(osg::BoundingSpheref, BOUNDINGSPHEREF)
DECLARE_TYPE(osg::BoundingSphered, BOUNDINGSPHERED)

// forward declare
class PropertyOutputIterator;
class PropertyInputIterator;


/** ClassInterface provides a general means of checking for supported properties of classes, and getting/setting those properties.
    Uses the osgDB serializers to do the actual object query/get/set.
*/
class OSGDB_EXPORT ClassInterface
{
public:
    ClassInterface();


    /// get the Type of the specified property, return true if property is supported, otherwise false.
    bool getPropertyType(const osg::Object* object, const std::string& propertyName, osgDB::BaseSerializer::Type& type) const;

    /// return type of two types are compatible
    bool areTypesCompatible(osgDB::BaseSerializer::Type lhs, osgDB::BaseSerializer::Type rhs) const;


    /** create an object of specified type for provided compound class name  in the form libraryName::className. */
    osg::Object* createObject(const std::string& compoundClassdName) const;

    /// template method for getting property data, return true if property available and the type is compatible, otherwise returns false.
    template<typename T>
    bool getProperty(const osg::Object* object, const std::string& propertyName, T& value);

    /// template method for setting property data, return true if property available and the type is compatible, otherwise returns false.
    template<typename T>
    bool setProperty(osg::Object* object, const std::string& propertyName, const T& value);


    /// get the human readable name of type.
    std::string getTypeName(osgDB::BaseSerializer::Type type) const;

    /// get the enum value of type given the human readable name.
    osgDB::BaseSerializer::Type getType(const std::string& typeName) const;


    /// Properties supported for a single class
    typedef std::map<std::string, osgDB::BaseSerializer::Type> PropertyMap;

    /// Get the list of of properties supported by object
    bool getSupportedProperties(const osg::Object* object, PropertyMap& properties, bool searchAssociates=true) const;

    /// return true if the object can be cast to the specified class specified by compoundClassName
    bool isObjectOfType(const osg::Object* object, const std::string& compoundClassName) const;


    /// run method of object
    bool run(void* objectPtr, const std::string& compoundClassName, const std::string& methodName, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const;

    /// run method of object
    bool run(osg::Object* object, const std::string& methodName, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const;

    /// checked for support of specified method
    bool hasMethod(const std::string& compoundClassName, const std::string& methodName) const;

    /// checked for support of specified method
    bool hasMethod(const osg::Object* object, const std::string& methodName) const;


    /// Properties supported for a range of classes, used for white and black lists
    typedef std::map<std::string, PropertyMap> ObjectPropertyMap;

    /// Get the list of properties that are explicitly defined as supported
    ObjectPropertyMap& getWhiteList() { return _whiteList; }

    /// Get the const list of properties that are explicitly defined as supported
    const ObjectPropertyMap& getWhiteList() const { return _whiteList; }

    /// Get the list of properties that are explicitly defined as not supported
    ObjectPropertyMap& getBlackList() { return _blackList; }

    /// Get the const list of properties that are explicitly defined as not supported
    const ObjectPropertyMap& getBlackList() const { return _blackList; }

    osgDB::ObjectWrapper* getObjectWrapper(const osg::Object* object) const;

    osgDB::BaseSerializer* getSerializer(const osg::Object* object, const std::string& propertyName, osgDB::BaseSerializer::Type& type) const;

protected:

    bool copyPropertyDataFromObject(const osg::Object* object, const std::string& propertyName, void* valuePtr, unsigned int valueSize, osgDB::BaseSerializer::Type valueType);

    bool copyPropertyDataToObject(osg::Object* object, const std::string& propertyName, const void* valuePtr, unsigned int valueSize, osgDB::BaseSerializer::Type valueType);

    bool copyPropertyObjectFromObject(const osg::Object* object, const std::string& propertyName, void* valuePtr, unsigned int valueSize, osgDB::BaseSerializer::Type valueType);

    bool copyPropertyObjectToObject(osg::Object* object, const std::string& propertyName, const void* valuePtr, unsigned int valueSize, osgDB::BaseSerializer::Type valueType);



    osgDB::OutputStream _outputStream;
    PropertyOutputIterator* _poi;

    osgDB::InputStream _inputStream;
    PropertyInputIterator* _pii;

    typedef std::map<std::string, osgDB::BaseSerializer::Type> TypeNameToTypeMap;
    typedef std::map<osgDB::BaseSerializer::Type, std::string> TypeToTypeNameMap;

    TypeNameToTypeMap _typeNameToTypeMap;
    TypeToTypeNameMap _typeToTypeNameMap;

    ObjectPropertyMap _whiteList;
    ObjectPropertyMap _blackList;
};


template<typename T>
bool ClassInterface::getProperty(const osg::Object* object, const std::string& propertyName, T& value)
{
    if (copyPropertyDataFromObject(object, propertyName, &value, sizeof(T), getTypeEnum<T>())) return true;
    else return object->getUserValue(propertyName, value); // fallback to check user data for property
}

template<typename T>
bool ClassInterface::setProperty(osg::Object* object, const std::string& propertyName, const T& value)
{
    if (copyPropertyDataToObject(object, propertyName, &value, sizeof(T), getTypeEnum<T>())) return true;
    else
    {
        // fallback to using user data to store property data
        object->setUserValue(propertyName, value);
        return false;
    }
}

typedef osg::Object* ObjectPtr;

template<>
inline bool ClassInterface::getProperty(const osg::Object* object, const std::string& propertyName, ObjectPtr& value)
{
    if (copyPropertyObjectFromObject(object, propertyName, &value, sizeof(ObjectPtr), getTypeEnum<ObjectPtr>())) return true;
    else
    {
        OSG_INFO<<"ClassInterface::getProperty("<<propertyName<<", Checking UserDataContainer for object ptr"<<std::endl;
        const osg::UserDataContainer* udc = object->getUserDataContainer();
        if (udc)
        {
            OSG_INFO<<"   Checking UserDataContainer for object ptr"<<std::endl;
            const osg::Object* ptr = udc->getUserObject(propertyName);
            if (ptr)
            {
                value = const_cast<ObjectPtr>(ptr);
                return true;
            }
        }
        return false;
    }
}

template<>
inline bool ClassInterface::setProperty(osg::Object* object, const std::string& propertyName, const ObjectPtr& value)
{
    osgDB::BaseSerializer::Type type = dynamic_cast<osg::Image*>(value) ? osgDB::BaseSerializer::RW_IMAGE : getTypeEnum<ObjectPtr>();
    // osgDB::BaseSerializer::Type type = getTypeEnum<ObjectPtr>();
    if (copyPropertyObjectToObject(object, propertyName, &value, sizeof(ObjectPtr), type)) return true;
    else
    {
        // fallback to using user data to store property data
        osg::UserDataContainer* udc = object->getOrCreateUserDataContainer();
        unsigned int objectIndex = udc->getUserObjectIndex(propertyName);
        if (objectIndex < udc->getNumUserObjects())
        {
            const osg::Object* outgoingObject = udc->getUserObject(objectIndex);
            if (outgoingObject==value) return true;

            OSG_INFO<<"ClassInterface::setProperty("<<propertyName<<", "<<value->className()<<") replace object on UserDataContainer"<<std::endl;
            value->setName(propertyName);
            udc->setUserObject(objectIndex, value);
        }
        else
        {
            OSG_INFO<<"ClassInterface::setProperty("<<propertyName<<", "<<value->className()<<") Adding object to UserDataContainer"<<std::endl;
            value->setName(propertyName);
            udc->addUserObject(value);
        }
        return true;
    }
}


}

#endif
