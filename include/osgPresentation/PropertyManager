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

#ifndef PROPERTYMANAGER
#define PROPERTYMANAGER 1

#include <osg/UserDataContainer>
#include <osg/ValueObject>
#include <osg/ImageSequence>
#include <osgGA/GUIEventHandler>

#include <osgPresentation/Export>

#include <sstream>

namespace osgPresentation
{

class PropertyManager : protected osg::Object
{
public:

    PropertyManager() {}
    PropertyManager(const PropertyManager& pm, const osg::CopyOp& copyop=osg::CopyOp::SHALLOW_COPY):
        osg::Object(pm,copyop) {}

    META_Object(osgPresentation, PropertyManager)

    /** Convenience method that casts the named UserObject to osg::TemplateValueObject<T> and gets the value.
        * To use this template method you need to include the osg/ValueObject header.*/
    template<typename T>
    bool getProperty(const std::string& name, T& value) const
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        return getUserValue(name, value);
    }

    /** Convenience method that creates the osg::TemplateValueObject<T> to store the
        * specified value and adds it as a named UserObject.
        * To use this template method you need to include the osg/ValueObject header. */
    template<typename T>
    void setProperty(const std::string& name, const T& value)
    {
        OpenThreads::ScopedLock<OpenThreads::Mutex> lock(_mutex);
        return setUserValue(name, value);
    }

    int ref() const { return osg::Referenced::ref(); }
    int unref() const { return osg::Referenced::unref(); }

protected:

    mutable OpenThreads::Mutex _mutex;

};

extern OSGPRESENTATION_EXPORT const osg::Object* getUserObject(const osg::NodePath& nodepath, const std::string& name);

template<typename T>
bool getUserValue(const osg::NodePath& nodepath, const std::string& name, T& value)
{
    typedef osg::TemplateValueObject<T> UserValueObject;
    const osg::Object* object = getUserObject(nodepath, name);
    const UserValueObject* uvo = dynamic_cast<const UserValueObject*>(object);

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

extern OSGPRESENTATION_EXPORT bool containsPropertyReference(const std::string& str);

struct PropertyReader
{
    PropertyReader(const osg::NodePath& nodePath, const std::string& str):
        _errorGenerated(false),
        _nodePath(nodePath),
        _sstream(str) {}

    template<typename T>
    bool read(T& value)
    {
        // skip white space.
        while(!_sstream.fail() && _sstream.peek()==' ') _sstream.ignore();

        // check to see if a &propertyName is used.
        if (_sstream.peek()=='$')
        {
            std::string propertyName;
            _sstream.ignore(1);
            _sstream >> propertyName;
            OSG_NOTICE<<"Reading propertyName="<<propertyName<<std::endl;
            if (!_sstream.fail() && !propertyName.empty()) return getUserValue(_nodePath, propertyName, value);
            else return false;
        }
        else
        {
            _sstream >> value;
            OSG_NOTICE<<"Reading value="<<value<<std::endl;
            return !_sstream.fail();
        }
    }

    template<typename T>
    PropertyReader& operator>>( T& value ) { if (!read(value)) _errorGenerated=true; return *this; }

    bool ok() { return !_sstream.fail() && !_errorGenerated; }
    bool fail() { return _sstream.fail() || _errorGenerated; }

    bool                _errorGenerated;
    osg::NodePath       _nodePath;
    std::istringstream  _sstream;
};


class OSGPRESENTATION_EXPORT PropertyAnimation : public osg::NodeCallback
{
public:
    PropertyAnimation():
            _firstTime(DBL_MAX),
            _latestTime(0.0),
            _pause(false),
            _pauseTime(0.0) {}

    void setPropertyManager(PropertyManager* pm) { _pm = pm; }
    PropertyManager* getPropertyManager() const { return _pm.get(); }

    typedef std::map<double, osg::ref_ptr<osg::UserDataContainer> > KeyFrameMap;

    KeyFrameMap& getKeyFrameMap() { return _keyFrameMap; }
    const KeyFrameMap& getKeyFrameMap() const { return _keyFrameMap; }

    void addKeyFrame(double time, osg::UserDataContainer* udc)
    {
        _keyFrameMap[time] = udc;
    }

    virtual void reset();

    void setPause(bool pause);
    bool getPause() const { return _pause; }

    double getAnimationTime() const;

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

    virtual void update(osg::Node& node);


protected:

    osg::ref_ptr<PropertyManager> _pm;

    void assign(osg::UserDataContainer* destination, osg::UserDataContainer* source);
    void assign(osg::UserDataContainer* udc, osg::Object* obj);

    KeyFrameMap _keyFrameMap;

    double      _firstTime;
    double      _latestTime;
    bool        _pause;
    double      _pauseTime;

};



struct OSGPRESENTATION_EXPORT ImageSequenceUpdateCallback : public osg::NodeCallback
{
    ImageSequenceUpdateCallback(osg::ImageSequence* is, PropertyManager* pm, const std::string& propertyName):
        _imageSequence(is),
        _propertyManager(pm),
        _propertyName(propertyName) {}

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv);

    osg::ref_ptr<osg::ImageSequence> _imageSequence;
    osg::ref_ptr<PropertyManager> _propertyManager;
    std::string _propertyName;
};

struct OSGPRESENTATION_EXPORT PropertyEventCallback : public osgGA::GUIEventHandler
{
    PropertyEventCallback(PropertyManager* pm):
        _propertyManager(pm) {}

    virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&);

    osg::ref_ptr<PropertyManager> _propertyManager;
};

}

#endif
