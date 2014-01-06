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

        osg::Object* indexObject = inputParameters[0].get();
        OSG_NOTICE<<"SwitchGetValue "<<indexObject->className()<<std::endl;

        unsigned int index = 0;
        osg::DoubleValueObject* dvo = dynamic_cast<osg::DoubleValueObject*>(indexObject);
        if (dvo) index = static_cast<unsigned int>(dvo->getValue());
        else
        {
            osg::UIntValueObject* uivo = dynamic_cast<osg::UIntValueObject*>(indexObject);
            if (uivo) index = uivo->getValue();
        }

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

        osg::Object* indexObject = inputParameters[0].get();
        OSG_NOTICE<<"SwitchSetValue "<<indexObject->className()<<std::endl;

        unsigned int index = 0;
        osg::DoubleValueObject* dvo = dynamic_cast<osg::DoubleValueObject*>(indexObject);
        if (dvo) index = static_cast<unsigned int>(dvo->getValue());
        else
        {
            osg::UIntValueObject* uivo = dynamic_cast<osg::UIntValueObject*>(indexObject);
            if (uivo) index = uivo->getValue();
        }

        bool enabled = false;
        indexObject = inputParameters[1].get();
        dvo = dynamic_cast<osg::DoubleValueObject*>(indexObject);
        if (dvo) enabled = dvo->getValue()!=0.0;
        else
        {
            osg::UIntValueObject* uivo = dynamic_cast<osg::UIntValueObject*>(indexObject);
            if (uivo) enabled = uivo->getValue()!=0;
        }

        OSG_NOTICE<<"switch->setValue("<<index<<", "<<enabled<<")"<<std::endl;

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
