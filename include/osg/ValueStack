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

#ifndef OSG_VALUESTACK
#define OSG_VALUESTACK 1

#include <osg/ValueMap>

namespace osg {

#define OSG_HAS_VALUESTACK

class OSG_EXPORT ValueStack : public osg::Object
{
    public:

        ValueStack();

        ValueStack(const ValueStack& ps, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY);

        META_Object(osg, ValueStack);

        typedef std::vector< osg::ref_ptr<Object> > Values;
        typedef std::map< osg::ref_ptr<const osg::Referenced>, Values> ValueStackMap;

        void setValueStackMap(ValueStackMap& pm) { _valuesMap = pm; }

        ValueStackMap& getValueStackMap() { return _valuesMap; }

        const ValueStackMap& getValueStackMap() const { return _valuesMap; }

        inline void push(const Referenced* key, Object* value)
        {
            _valuesMap[key].push_back(value);
        }

        template<typename T>
        void push(const osg::Referenced* key, const T& value)
        {
            typedef TemplateValueObject<T> UserValueObject;
            _valuesMap[key].push_back(new UserValueObject(value));
        }

        inline void pop(const Referenced* key)
        {
            _valuesMap[key].pop_back();
        }

        inline void push(ValueMap* valueMap)
        {
            if (valueMap)
            {
                ValueMap::KeyValueMap& keyValueMap = valueMap->getKeyValueMap();
                for(ValueMap::KeyValueMap::iterator itr = keyValueMap.begin();
                    itr != keyValueMap.end();
                    ++itr)
                {
                    push(itr->first.get(), itr->second.get());
                }
            }
        }

        inline void pop(ValueMap* valueMap)
        {
            if (valueMap)
            {
                ValueMap::KeyValueMap& keyValueMap = valueMap->getKeyValueMap();
                for(ValueMap::KeyValueMap::iterator itr = keyValueMap.begin();
                    itr != keyValueMap.end();
                    ++itr)
                {
                    pop(itr->first.get());
                }
            }
        }

        inline osg::Object* getValue(const osg::Referenced* key)
        {
            ValueStackMap::iterator itr = _valuesMap.find(key);
            if (itr==_valuesMap.end()) return 0;

            Values& values = itr->second;
            if (values.empty()) return 0;

            return values.back().get();
        }

        inline const osg::Object* getValue(const osg::Referenced* key) const
        {
            ValueStackMap::const_iterator itr = _valuesMap.find(key);
            if (itr==_valuesMap.end()) return 0;

            const Values& values = itr->second;
            if (values.empty()) return 0;

            return values.back().get();
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

        virtual ~ValueStack();

        ValueStackMap _valuesMap;

};


}

#endif
