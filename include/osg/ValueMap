/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2016 Robert Osfield
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

#ifndef OSG_VALUEMAP
#define OSG_VALUEMAP 1

#include <osg/ValueObject>
#include <osg/Notify>
#include <map>

namespace osg {

#define OSG_HAS_VALUEMAP

class OSG_EXPORT ValueMap : public osg::Object
{
    public:

        ValueMap();

        ValueMap(const ValueMap& vm, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(osg, ValueMap);

        typedef std::map< osg::ref_ptr<const osg::Referenced>, osg::ref_ptr<osg::Object>  > KeyValueMap;

        void setKeyValueMap(KeyValueMap& properties) { _keyValueMap = properties; }

        KeyValueMap& getKeyValueMap() { return _keyValueMap; }

        const KeyValueMap& getKeyValueMap() const { return _keyValueMap; }

        osg::Object* setValue(const osg::Referenced* key, osg::Object* object)
        {
            return (_keyValueMap[key] = object).get();
        }

        template<typename T>
        osg::Object* setValue(const osg::Referenced* key, const T& value)
        {
            typedef TemplateValueObject<T> UserValueObject;
            KeyValueMap::iterator itr = _keyValueMap.find(key);
            if (itr!=_keyValueMap.end())
            {
                osg::Object* obj = itr->second.get();
                if (typeid(*(obj))==typeid(UserValueObject))
                {
                    UserValueObject* uvo = static_cast<UserValueObject*>(itr->second.get());
                    uvo->setValue(value);
                    return uvo;
                }
            }

            return (_keyValueMap[key] = new UserValueObject(value)).get();
        }


        inline osg::Object* getValue(const osg::Referenced* key)
        {
            KeyValueMap::iterator itr = _keyValueMap.find(key);
            return (itr!=_keyValueMap.end()) ? itr->second.get() : 0;
        }

        inline const osg::Object* getValue(const osg::Referenced* key) const
        {
            KeyValueMap::const_iterator itr = _keyValueMap.find(key);
            return (itr!=_keyValueMap.end()) ? itr->second.get() : 0;
        }


        template<typename T>
        T* getValueOfType(const osg::Referenced* key)
        {
            Object* object = getValue(key);
            return (object && typeid(*object)==typeid(T)) ? static_cast<T*>(object) : 0;
        }


        template<typename T>
        const T* getValueOfType(const osg::Referenced* key) const
        {
            const Object* object = getValue(key);
            return (object && typeid(*object)==typeid(T)) ? static_cast<const T*>(object) : 0;
        }


        template<typename T>
        bool getValue(const osg::Referenced* key, T& value)
        {
            typedef TemplateValueObject<T> UserValueObject;
            UserValueObject* uvo = getValueOfType<UserValueObject>(key);
            if (uvo)
            {
                value = uvo->getValue();
                return true;
            }
            else
            {
                return false;
            }
        }

        template<typename T>
        bool getValue(const osg::Referenced* key, T& value) const
        {
            typedef TemplateValueObject<T> UserValueObject;
            const UserValueObject* uvo = getValueOfType<UserValueObject>(key);
            if (uvo)
            {
                value = uvo->getValue();
                return true;
            }
            else
            {
                return false;
            }
        }


    protected:

        virtual ~ValueMap();

        KeyValueMap _keyValueMap;

};

}

#endif
