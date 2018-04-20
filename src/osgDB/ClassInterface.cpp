/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2013 Robert Osfield
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


#include <osgDB/ClassInterface>

namespace osgDB  // start of osgDB namespace
{

osgDB::BaseSerializer::Type getTypeEnumFromPtr(const osg::Object*) { return osgDB::BaseSerializer::RW_OBJECT; }
const char* getTypeStringFromPtr(const osg::Object*) { return "OBJECT"; }

osgDB::BaseSerializer::Type getTypeEnumFromPtr(const osg::Image*) { return osgDB::BaseSerializer::RW_IMAGE; }
const char* getTypeStringFromPtr(const osg::Image*) { return "IMAGE"; }

///////////////////////////////////////////////////////////////////
//
// PropertyOutputIterator enables the get of class properties
//
class PropertyOutputIterator : public osgDB::OutputIterator
{
public:

    PropertyOutputIterator()
    {
    }

    virtual ~PropertyOutputIterator() {}

    virtual bool isBinary() const { return true; }


    template<typename T>
    inline void write(T t)
    {
        char* ptr = reinterpret_cast<char*>(&t);
        _str.insert(_str.size(), ptr, sizeof(T));
    }

    virtual void writeBool( bool b ) { _str.push_back(static_cast<char>(b?1:0)); }
    virtual void writeChar( char c ) { _str.push_back(c); }
    virtual void writeUChar( unsigned char c ) { _str.push_back(static_cast<char>(c)); }
    virtual void writeShort( short s ) { write(s); }
    virtual void writeUShort( unsigned short s ) { write(s); }
    virtual void writeInt( int i ) { write(i); }
    virtual void writeUInt( unsigned int i ) { write(i);  }
    virtual void writeLong( long l ) { write(l); }
    virtual void writeULong( unsigned long l ) { write(l); }
    virtual void writeFloat( float f ) { write(f); }
    virtual void writeDouble( double d ) { write(d); }
    virtual void writeInt64( GLint64 ll ) { write(ll); }
    virtual void writeUInt64( GLuint64 ull ) { write(ull); }
    virtual void writeString( const std::string& s ) { _str.insert(_str.end(), s.begin(), s.end()); }
    virtual void writeStream( std::ostream& (*)(std::ostream&) ) {}
    virtual void writeBase( std::ios_base& (*)(std::ios_base&) ) {}
    virtual void writeGLenum( const osgDB::ObjectGLenum& value ) { writeInt(value.get()); }
    virtual void writeProperty( const osgDB::ObjectProperty& prop ) { _propertyName = prop._name; }
    virtual void writeMark( const osgDB::ObjectMark& mark ) { _markName = mark._name; }
    virtual void writeCharArray( const char* s, unsigned int size) { _str.insert(std::string::npos, s, size); }
    virtual void writeWrappedString( const std::string& str ) { _str.insert(_str.end(), str.begin(), str.end()); }

    virtual void flush()
    {
        _str.clear();
        _propertyName.clear();
        _markName.clear();
    }

    std::string         _str;
    std::string         _propertyName;
    std::string         _markName;
};

///////////////////////////////////////////////////////////////////
//
// PropertyInputIterator enables the set of class properties
//
class OSGDB_EXPORT PropertyInputIterator : public osgDB::InputIterator
{
public:
    PropertyInputIterator():
        _sstream(std::stringstream::binary),
        _bufferData(0),
        _currentPtr(0),
        _bufferSize(0)
    {
        setStream(&_sstream);
    }
    virtual ~PropertyInputIterator()
    {
        if (_bufferData) delete [] _bufferData;
        setStream(0);
    }

    virtual bool isBinary() const { return true; }

    template<typename T>
    void read(T& value)
    {
        memcpy(reinterpret_cast<char*>(&value), _currentPtr, sizeof(T));
        _currentPtr += sizeof(T);
    }

    virtual void readBool( bool& b ) { char c; read(c); b = (c!=0); }
    virtual void readChar( char& c ) { read(c); }
    virtual void readSChar( signed char& c ) { read(c); }
    virtual void readUChar( unsigned char& c ) { read(c); }
    virtual void readShort( short& s ) { read(s); }
    virtual void readUShort( unsigned short& s ) { read(s); }
    virtual void readInt( int& i ) { read(i); }
    virtual void readUInt( unsigned int& i ) { read(i);}
    virtual void readLong( long& l ) { read(l); }
    virtual void readULong( unsigned long& l ) { read(l); }
    virtual void readFloat( float& f ) { read(f); }
    virtual void readDouble( double& d ) { read(d); }
    virtual void readString( std::string& s ) { s = std::string(_bufferData, _bufferSize); }

    virtual void readStream( std::istream& (*)(std::istream&) ) {}
    virtual void readBase( std::ios_base& (*)(std::ios_base&) ) {}

    virtual void readGLenum( ObjectGLenum& value ) { readUInt(value._value); }
    virtual void readProperty( ObjectProperty& ) {}
    virtual void readMark( ObjectMark&) {}
    virtual void readCharArray( char* s, unsigned int size ) { if ( size>0 ) _in->read( s, size ); }
    virtual void readWrappedString( std::string& str ) { readString(str); }

    virtual bool matchString( const std::string& /*str*/ ) { return false; }

    template<typename T>
    void set(const T& value)
    {
        if (_bufferData) delete [] _bufferData;
        _bufferData = new char[sizeof(T)];
        _bufferSize = sizeof(T);
        _currentPtr = _bufferData;
        memcpy(_bufferData, reinterpret_cast<const char*>(&value), sizeof(T));
    }

    void set(const void* ptr, unsigned int valueSize)
    {
        if (_bufferData) delete [] _bufferData;
        _bufferData = new char[valueSize];
        _currentPtr = _bufferData;
        _bufferSize = valueSize;
        memcpy(_bufferData, reinterpret_cast<const char*>(ptr), valueSize);
    }

    std::stringstream _sstream;
    char* _bufferData;
    char* _currentPtr;
    unsigned int _bufferSize;
};


////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// ClassInterface class provides a generic mechanism for get/setting class properties using the osgDB serializers
//
ClassInterface::ClassInterface():
    _outputStream(0),
    _inputStream(0)
{
    _poi = new PropertyOutputIterator;
    _outputStream.setOutputIterator(_poi);

    _pii = new PropertyInputIterator;
    _inputStream.setInputIterator(_pii);


    // initialize the type maps
    #define TYPENAME(A) \
        _typeToTypeNameMap[osgDB::BaseSerializer::RW_##A] = #A; \
        _typeNameToTypeMap[#A] = osgDB::BaseSerializer::RW_##A;

    TYPENAME(UNDEFINED)
    TYPENAME(USER)
    TYPENAME(OBJECT)
    TYPENAME(IMAGE)
    TYPENAME(LIST)

    TYPENAME(BOOL)
    TYPENAME(CHAR)
    TYPENAME(UCHAR)
    TYPENAME(SHORT)
    TYPENAME(USHORT)
    TYPENAME(INT)
    TYPENAME(UINT)
    TYPENAME(FLOAT)
    TYPENAME(DOUBLE)

    TYPENAME(VEC2F)
    TYPENAME(VEC2D)
    TYPENAME(VEC3F)
    TYPENAME(VEC3D)
    TYPENAME(VEC4F)
    TYPENAME(VEC4D)
    TYPENAME(QUAT)
    TYPENAME(PLANE)

    TYPENAME(MATRIXF)
    TYPENAME(MATRIXD)
    TYPENAME(MATRIX)

    TYPENAME(BOUNDINGBOXF)
    TYPENAME(BOUNDINGBOXD)

    TYPENAME(BOUNDINGSPHEREF)
    TYPENAME(BOUNDINGSPHERED)

    TYPENAME(GLENUM)
    TYPENAME(STRING)
    TYPENAME(ENUM)

    TYPENAME(VEC2B)
    TYPENAME(VEC2UB)
    TYPENAME(VEC2S)
    TYPENAME(VEC2US)
    TYPENAME(VEC2I)
    TYPENAME(VEC2UI)

    TYPENAME(VEC3B)
    TYPENAME(VEC3UB)
    TYPENAME(VEC3S)
    TYPENAME(VEC3US)
    TYPENAME(VEC3I)
    TYPENAME(VEC3UI)

    TYPENAME(VEC4B)
    TYPENAME(VEC4UB)
    TYPENAME(VEC4S)
    TYPENAME(VEC4US)
    TYPENAME(VEC4I)
    TYPENAME(VEC4UI)

    TYPENAME(LIST)
    TYPENAME(VECTOR)
    TYPENAME(MAP)
}


bool ClassInterface::areTypesCompatible(osgDB::BaseSerializer::Type lhs, osgDB::BaseSerializer::Type rhs) const
{
    if (lhs==rhs) return true;

#ifdef OSG_USE_FLOAT_MATRIX
    if (lhs==osgDB::BaseSerializer::RW_MATRIX) lhs = osgDB::BaseSerializer::RW_MATRIXF;
    if (rhs==osgDB::BaseSerializer::RW_MATRIX) rhs = osgDB::BaseSerializer::RW_MATRIXF;
#else
    if (lhs==osgDB::BaseSerializer::RW_MATRIX) lhs = osgDB::BaseSerializer::RW_MATRIXD;
    if (rhs==osgDB::BaseSerializer::RW_MATRIX) rhs = osgDB::BaseSerializer::RW_MATRIXD;
#endif

    if (lhs==osgDB::BaseSerializer::RW_GLENUM) lhs = osgDB::BaseSerializer::RW_UINT;
    if (rhs==osgDB::BaseSerializer::RW_GLENUM) rhs = osgDB::BaseSerializer::RW_UINT;

    if (lhs==osgDB::BaseSerializer::RW_ENUM) lhs = osgDB::BaseSerializer::RW_INT;
    if (rhs==osgDB::BaseSerializer::RW_ENUM) rhs = osgDB::BaseSerializer::RW_INT;

    if (lhs==osgDB::BaseSerializer::RW_IMAGE) lhs = osgDB::BaseSerializer::RW_OBJECT;

    return lhs==rhs;
}

std::string ClassInterface::getTypeName(osgDB::BaseSerializer::Type type) const
{
    TypeToTypeNameMap::const_iterator itr = _typeToTypeNameMap.find(type);
    if (itr != _typeToTypeNameMap.end()) return itr->second;
    else return std::string();
}

osgDB::BaseSerializer::Type ClassInterface::getType(const std::string& typeName) const
{
    TypeNameToTypeMap::const_iterator itr = _typeNameToTypeMap.find(typeName);
    if (itr != _typeNameToTypeMap.end()) return itr->second;
    else return osgDB::BaseSerializer::RW_UNDEFINED;
}


osgDB::ObjectWrapper* ClassInterface::getObjectWrapper(const osg::Object* object) const
{
    return osgDB::Registry::instance()->getObjectWrapperManager()->findWrapper(object->getCompoundClassName());
}

osgDB::BaseSerializer* ClassInterface::getSerializer(const osg::Object* object, const std::string& propertyName, osgDB::BaseSerializer::Type& type) const
{
    osgDB::ObjectWrapper* ow = getObjectWrapper(object);
    return (ow!=0) ? ow->getSerializer(propertyName, type) : 0;
}

osg::Object* ClassInterface::createObject(const std::string& compoundClassName) const
{
    osgDB::ObjectWrapper* ow = osgDB::Registry::instance()->getObjectWrapperManager()->findWrapper(compoundClassName);
    if (ow)
    {
        osg::Object* object = ow->createInstance();
        // OSG_NOTICE<<"ClassInterface::createObject("<<compoundClassName<<"), wrapper found, created object="<<object<<std::endl;
        return object;
    }
    else
    {
        OSG_NOTICE<<"ClassInterface::createObject("<<compoundClassName<<"), No object wrapper available."<<std::endl;
        return 0;
    }
    // return (ow!=0) ? ow->createInstance() : 0;
}

bool ClassInterface::copyPropertyDataFromObject(const osg::Object* object, const std::string& propertyName, void* valuePtr, unsigned int valueSize, osgDB::BaseSerializer::Type valueType)
{
    _poi->flush();

    osgDB::BaseSerializer::Type sourceType;
    osgDB::BaseSerializer* serializer = getSerializer(object, propertyName, sourceType);
    if (!serializer) return false;

    if (!areTypesCompatible(sourceType, valueType))
    {
        OSG_NOTICE<<"ClassInterface::copyPropertyDataFromObject() Types are not compatible, valueType = "<<valueType<<", sourceType="<<sourceType<<std::endl;
        return false;
    }

    if (serializer->write(_outputStream, *object))
    {
        unsigned int sourceSize = _poi->_str.size();

        if (valueType==osgDB::BaseSerializer::RW_STRING)
        {
            std::string* string_ptr = reinterpret_cast<std::string*>(valuePtr);
            (*string_ptr) = _poi->_str;
            return true;
        }
        else if (sourceSize==valueSize)
        {
            memcpy(valuePtr, &(_poi->_str[0]), valueSize);
            return true;
        }
        else
        {
            OSG_NOTICE<<"ClassInterface::copyPropertyDataFromObject() Sizes not compatible, sourceSize = "<<sourceSize<<" valueSize = "<<valueSize<<std::endl;
            return false;
        }
    }
    else
    {
        OSG_INFO<<"ClassInterface::copyPropertyDataFromObject() serializer write failed."<<std::endl;
        return false;
    }
}

bool ClassInterface::copyPropertyDataToObject(osg::Object* object, const std::string& propertyName, const void* valuePtr, unsigned int valueSize, osgDB::BaseSerializer::Type valueType)
{
    // copy data to PropertyInputIterator
    if (valueType==osgDB::BaseSerializer::RW_STRING)
    {
        const std::string* string_ptr = reinterpret_cast<const std::string*>(valuePtr);
        _pii->set(&((*string_ptr)[0]), string_ptr->size());
    }
    else
    {
        _pii->set(valuePtr, valueSize);
    }

    osgDB::BaseSerializer::Type destinationType;
    osgDB::BaseSerializer* serializer = getSerializer(object, propertyName, destinationType);
    if (serializer)
    {
        if (areTypesCompatible(valueType, destinationType))
        {
            return serializer->read(_inputStream, *object);
        }
        else
        {
            OSG_NOTICE<<"ClassInterface::copyPropertyDataToObject() Types are not compatible, valueType = "<<valueType<<" ["<<getTypeName(valueType)<<"] , destinationType="<<destinationType<<" ["<<getTypeName(destinationType)<<"]"<<std::endl;
            return false;
        }
    }
    else
    {
        OSG_INFO<<"ClassInterface::copyPropertyDataFromObject() no serializer available."<<std::endl;
        return false;
    }
}

bool ClassInterface::copyPropertyObjectFromObject(const osg::Object* object, const std::string& propertyName, void* valuePtr, unsigned int /*valueSize*/, osgDB::BaseSerializer::Type valueType)
{
    osgDB::BaseSerializer::Type sourceType;
    osgDB::BaseSerializer* serializer = getSerializer(object, propertyName, sourceType);
    if (serializer)
    {
        if (areTypesCompatible(sourceType, valueType))
        {
            return serializer->get(*object, valuePtr);
        }
        else
        {
            OSG_NOTICE<<"ClassInterface::copyPropertyObjectFromObject() Types are not compatible, valueType = "<<valueType<<" ["<<getTypeName(valueType)<<"] , sourceType="<<sourceType<<" ["<<getTypeName(sourceType)<<"]"<<std::endl;
            return false;
        }
    }
    else
    {
        OSG_INFO<<"ClassInterface::copyPropertyObjectFromObject() no serializer available."<<std::endl;
        return false;
    }
}

bool ClassInterface::copyPropertyObjectToObject(osg::Object* object, const std::string& propertyName, const void* valuePtr, unsigned int /*valueSize*/, osgDB::BaseSerializer::Type valueType)
{
    osgDB::BaseSerializer::Type destinationType;
    osgDB::BaseSerializer* serializer = getSerializer(object, propertyName, destinationType);
    if (serializer)
    {
        if (areTypesCompatible(valueType, destinationType))
        {
            return serializer->set(*object, const_cast<void*>(valuePtr));
        }
        else
        {
            OSG_NOTICE<<"ClassInterface::copyPropertyObjectToObject() Types are not compatible, valueType = "<<valueType<<", destinationType="<<destinationType<<std::endl;
            return false;
        }
    }
    else
    {
        OSG_INFO<<"ClassInterface::copyPropertyObjectToObject() no serializer available."<<std::endl;
        return false;
    }
}


class GetPropertyType : public osg::ValueObject::GetValueVisitor
{
public:

    GetPropertyType(): type(osgDB::BaseSerializer::RW_UNDEFINED) {}

    osgDB::BaseSerializer::Type type;

    virtual void apply(bool /*value*/) { type = osgDB::BaseSerializer::RW_BOOL; }
    virtual void apply(char /*value*/) { type = osgDB::BaseSerializer::RW_CHAR; }
    virtual void apply(unsigned char /*value*/) { type = osgDB::BaseSerializer::RW_UCHAR; }
    virtual void apply(short /*value*/) { type = osgDB::BaseSerializer::RW_SHORT; }
    virtual void apply(unsigned short /*value*/) { type = osgDB::BaseSerializer::RW_USHORT; }
    virtual void apply(int /*value*/) { type = osgDB::BaseSerializer::RW_INT; }
    virtual void apply(unsigned int /*value*/) { type = osgDB::BaseSerializer::RW_UINT; }
    virtual void apply(float /*value*/) { type = osgDB::BaseSerializer::RW_FLOAT; }
    virtual void apply(double /*value*/) { type = osgDB::BaseSerializer::RW_DOUBLE; }
    virtual void apply(const std::string& /*value*/) { type = osgDB::BaseSerializer::RW_STRING; }
    virtual void apply(const osg::Vec2f& /*value*/) { type = osgDB::BaseSerializer::RW_VEC2F; }
    virtual void apply(const osg::Vec3f& /*value*/) { type = osgDB::BaseSerializer::RW_VEC3F; }
    virtual void apply(const osg::Vec4f& /*value*/) { type = osgDB::BaseSerializer::RW_VEC4F; }
    virtual void apply(const osg::Vec2d& /*value*/) { type = osgDB::BaseSerializer::RW_VEC2D; }
    virtual void apply(const osg::Vec3d& /*value*/) { type = osgDB::BaseSerializer::RW_VEC3D; }
    virtual void apply(const osg::Vec4d& /*value*/) { type = osgDB::BaseSerializer::RW_VEC4D; }
    virtual void apply(const osg::Quat& /*value*/) { type = osgDB::BaseSerializer::RW_QUAT; }
    virtual void apply(const osg::Plane& /*value*/) { type = osgDB::BaseSerializer::RW_PLANE; }
    virtual void apply(const osg::Matrixf& /*value*/) { type = osgDB::BaseSerializer::RW_MATRIXF; }
    virtual void apply(const osg::Matrixd& /*value*/) { type = osgDB::BaseSerializer::RW_MATRIXD; }
    virtual void apply(const osg::BoundingBoxf& /*value*/) { type = osgDB::BaseSerializer::RW_BOUNDINGBOXF; }
    virtual void apply(const osg::BoundingBoxd& /*value*/) { type = osgDB::BaseSerializer::RW_BOUNDINGBOXD; }
    virtual void apply(const osg::BoundingSpheref& /*value*/) { type = osgDB::BaseSerializer::RW_BOUNDINGSPHEREF; }
    virtual void apply(const osg::BoundingSphered& /*value*/) { type = osgDB::BaseSerializer::RW_BOUNDINGSPHERED; }
};

bool ClassInterface::getPropertyType(const osg::Object* object, const std::string& propertyName, osgDB::BaseSerializer::Type& type) const
{
    if (getSerializer(object, propertyName, type)!=0) return true;

    const osg::UserDataContainer* udc = object->getUserDataContainer();
    const osg::Object* userObject = udc ? udc->getUserObject(propertyName) : 0;
    if (userObject)
    {
        const osg::ValueObject* valueObject = dynamic_cast<const osg::ValueObject*>(userObject);
        if (valueObject)
        {
            GetPropertyType gpt;
            valueObject->get(gpt);
            type = gpt.type;
            return gpt.type!=osgDB::BaseSerializer::RW_UNDEFINED;
        }
    }
    return false;
}


bool ClassInterface::getSupportedProperties(const osg::Object* object, PropertyMap& properties, bool searchAssociates) const
{
    osgDB::ObjectWrapper* ow = getObjectWrapper(object);
    if (!ow)
    {
        return false;
    }

    std::string compoundClassName = object->getCompoundClassName();
    ObjectPropertyMap::const_iterator wl_itr = _whiteList.find(compoundClassName);
    if (wl_itr != _whiteList.end())
    {
        properties = wl_itr->second;
    }

    ObjectPropertyMap::const_iterator bl_itr = _blackList.find(compoundClassName);

    if (searchAssociates)
    {
        const ObjectWrapper::RevisionAssociateList& associates = ow->getAssociates();
        for(ObjectWrapper::RevisionAssociateList::const_iterator aitr = associates.begin();
            aitr != associates.end();
            ++aitr)
        {
            osgDB::ObjectWrapper* associate_wrapper = osgDB::Registry::instance()->getObjectWrapperManager()->findWrapper(aitr->_name);
            if (associate_wrapper)
            {
                const osgDB::ObjectWrapper::SerializerList& associate_serializers = associate_wrapper->getSerializerList();
                unsigned int i=0;
                for(osgDB::ObjectWrapper::SerializerList::const_iterator sitr = associate_serializers.begin();
                    sitr != associate_serializers.end();
                    ++sitr, ++i)
                {
                    const std::string& propertyName = (*sitr)->getName();
                    bool notBlackListed = (bl_itr == _blackList.end()) || (bl_itr->second.count(propertyName)==0);
                    if (notBlackListed) properties[propertyName] = associate_wrapper->getTypeList()[i];
                }
            }
        }
    }
    else
    {
        const osgDB::ObjectWrapper::SerializerList& serializers = ow->getSerializerList();
        unsigned int i=0;
        for(osgDB::ObjectWrapper::SerializerList::const_iterator itr = serializers.begin();
            itr != serializers.end();
            ++itr, ++i)
        {
            const std::string& propertyName = (*itr)->getName();
            bool notBlackListed = (bl_itr == _blackList.end()) || (bl_itr->second.count(propertyName)==0);
            if (notBlackListed) properties[propertyName] = ow->getTypeList()[i];
        }
    }


    return true;
}

bool ClassInterface::isObjectOfType(const osg::Object* object, const std::string& compoundClassName) const
{
    if (!object) return false;

    if (object->getCompoundClassName()==compoundClassName) return true;

    osgDB::ObjectWrapper* ow = getObjectWrapper(object);
    if (!ow)
    {
        return false;
    }

    const ObjectWrapper::RevisionAssociateList& associates = ow->getAssociates();
    for(ObjectWrapper::RevisionAssociateList::const_iterator aitr = associates.begin();
        aitr != associates.end();
        ++aitr)
    {
        if ((aitr->_name)==compoundClassName) return true;
    }
    return false;
}

bool ClassInterface::run(void* objectPtr, const std::string& compoundClassName, const std::string& methodName, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
{
    ObjectWrapper* ow = osgDB::Registry::instance()->getObjectWrapperManager()->findWrapper(compoundClassName);
    if (!ow) return false;

    const ObjectWrapper::MethodObjectMap& ow_methodObjectMap = ow->getMethodObjectMap();
    for(ObjectWrapper::MethodObjectMap::const_iterator itr = ow_methodObjectMap.find(methodName);
        (itr!=ow_methodObjectMap.end()) && (itr->first==methodName);
        ++itr)
    {
        MethodObject* mo = itr->second.get();
        if (mo->run(objectPtr, inputParameters, outputParameters)) return true;
    }

    const ObjectWrapper::RevisionAssociateList& associates = ow->getAssociates();
    for(ObjectWrapper::RevisionAssociateList::const_iterator aitr = associates.begin();
        aitr != associates.end();
        ++aitr)
    {
        osgDB::ObjectWrapper* aow = osgDB::Registry::instance()->getObjectWrapperManager()->findWrapper(aitr->_name);
        if (aow)
        {
            const ObjectWrapper::MethodObjectMap& methodObjectMap = aow->getMethodObjectMap();
            for(ObjectWrapper::MethodObjectMap::const_iterator itr = methodObjectMap.find(methodName);
                (itr!=methodObjectMap.end()) && (itr->first==methodName);
                ++itr)
            {
                MethodObject* mo = itr->second.get();
                if (mo->run(objectPtr, inputParameters, outputParameters)) return true;
            }
        }
    }

    return false;
}

bool ClassInterface::run(osg::Object* object, const std::string& methodName, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
{
    return run(object, object->getCompoundClassName(), methodName, inputParameters, outputParameters);
}

bool ClassInterface::hasMethod(const std::string& compoundClassName, const std::string& methodName) const
{
    ObjectWrapper* ow = osgDB::Registry::instance()->getObjectWrapperManager()->findWrapper(compoundClassName);
    if (!ow) return false;

    const ObjectWrapper::MethodObjectMap& ow_methodObjectMap = ow->getMethodObjectMap();
    ObjectWrapper::MethodObjectMap::const_iterator oitr = ow_methodObjectMap.find(methodName);
    if (oitr!=ow_methodObjectMap.end()) return true;

    const ObjectWrapper::RevisionAssociateList& associates = ow->getAssociates();
    for(ObjectWrapper::RevisionAssociateList::const_iterator aitr = associates.begin();
        aitr != associates.end();
        ++aitr)
    {
        osgDB::ObjectWrapper* aow = osgDB::Registry::instance()->getObjectWrapperManager()->findWrapper(aitr->_name);
        if (aow)
        {
            const ObjectWrapper::MethodObjectMap& methodObjectMap = aow->getMethodObjectMap();
            ObjectWrapper::MethodObjectMap::const_iterator itr = methodObjectMap.find(methodName);
            if (itr!=methodObjectMap.end()) return true;
        }
    }

    return false;
}

bool ClassInterface::hasMethod(const osg::Object* object, const std::string& methodName) const
{
    return hasMethod(object->getCompoundClassName(), methodName);
}


} // end of osgDB namespace


