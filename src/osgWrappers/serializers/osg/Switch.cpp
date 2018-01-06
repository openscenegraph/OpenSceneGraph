#include <osg/Switch>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

struct SwitchGetValue : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        unsigned int index = 0;
        osg::ValueObject* indexObject = inputParameters[0]->asValueObject();
        if (indexObject) indexObject->getScalarValue(index);

        osg::Switch* sw = reinterpret_cast<osg::Switch*>(objectPtr);
        outputParameters.push_back(new osg::BoolValueObject("return", sw->getValue(index)));

        return true;
    }
};


struct SwitchSetValue : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.size()<2) return false;

        unsigned int index = 0;
        osg::ValueObject* indexObject = inputParameters[0]->asValueObject();
        if (indexObject) indexObject->getScalarValue(index);

        bool enabled = false;
        osg::ValueObject* valueObject = inputParameters[1]->asValueObject();
        if (valueObject) valueObject->getScalarValue(enabled);

        osg::Switch* sw = reinterpret_cast<osg::Switch*>(objectPtr);
        sw->setValue(index, enabled);

        return true;
    }
};

REGISTER_OBJECT_WRAPPER( Switch,
                         new osg::Switch,
                         osg::Switch,
                         "osg::Object osg::Node osg::Group osg::Switch" )
{
    ADD_BOOL_SERIALIZER( NewChildDefaultValue, true );  // _newChildDefaultValue
    ADD_LIST_SERIALIZER( ValueList, osg::Switch::ValueList );  // _values

    ADD_METHOD_OBJECT( "getValue", SwitchGetValue );
    ADD_METHOD_OBJECT( "setValue", SwitchSetValue );
}
