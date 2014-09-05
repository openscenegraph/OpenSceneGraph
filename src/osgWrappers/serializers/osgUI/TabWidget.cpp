#include <osgUI/TabWidget>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

struct TabWidgetCurrentIndexChangedImplementation : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osg::Object* indexObject = inputParameters[0].get();

        unsigned int index = 0;
        osg::DoubleValueObject* dvo = dynamic_cast<osg::DoubleValueObject*>(indexObject);
        if (dvo) index = static_cast<unsigned int>(dvo->getValue());
        else
        {
            osg::UIntValueObject* uivo = dynamic_cast<osg::UIntValueObject*>(indexObject);
            if (uivo) index = uivo->getValue();
        }
        osgUI::TabWidget* cb = reinterpret_cast<osgUI::TabWidget*>(objectPtr);
        cb->currentIndexChangedImplementation(index);

        return true;
    }
};

REGISTER_OBJECT_WRAPPER( TabWidget,
                         new osgUI::TabWidget,
                         osgUI::TabWidget,
                         "osg::Object osg::Node osg::Group osgUI::Widget osgUI::TabWidget" )
{
    ADD_UINT_SERIALIZER(CurrentIndex, 0);
    ADD_VECTOR_SERIALIZER( Tabs, osgUI::TabWidget::Tabs, osgDB::BaseSerializer::RW_OBJECT, 0 );
    ADD_METHOD_OBJECT( "currentIndexChangedImplementation", TabWidgetCurrentIndexChangedImplementation );
}
