#include <osgPresentation/PresentationInterface>
#include <osgGA/Event>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

struct PresentationInterfaceGetSlideEventHandler : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        osgPresentation::PresentationInterface* pi = reinterpret_cast<osgPresentation::PresentationInterface*>(objectPtr);
        outputParameters.push_back(pi->getSlideEventHandler());
        return true;
    }
};

struct PresentationInterfaceSendEventToViewer : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        osgPresentation::PresentationInterface* pi = reinterpret_cast<osgPresentation::PresentationInterface*>(objectPtr);
        if (inputParameters.empty()) return false;

        for(osg::Parameters::iterator itr = inputParameters.begin();
            itr != inputParameters.end();
            ++itr)
        {
            osgGA::Event* event = dynamic_cast<osgGA::Event*>(itr->get());
            osgPresentation::KeyPosition* kp = dynamic_cast<osgPresentation::KeyPosition*>(itr->get());
            OSG_NOTICE<<"Dispatch event "<<pi<<", "<<kp<<", "<<event<<std::endl;
            if (kp) pi->sendEventToViewer(kp);
            else if (event) pi->sendEventToViewer(event);
        }

        return true;
    }
};

struct PresentationInterfaceSendEventToDevices : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        osgPresentation::PresentationInterface* pi = reinterpret_cast<osgPresentation::PresentationInterface*>(objectPtr);
        if (inputParameters.empty()) return false;

        for(osg::Parameters::iterator itr = inputParameters.begin();
            itr != inputParameters.end();
            ++itr)
        {
            osgGA::Event* event = dynamic_cast<osgGA::Event*>(itr->get());
            OSG_NOTICE<<"forward event "<<pi<<", "<<event<<std::endl;
            if (event) pi->sendEventToDevices(event);
        }

        return true;
    }
};

REGISTER_OBJECT_WRAPPER( osgPresentation_PresentationInterface,
                         new osgPresentation::PresentationInterface,
                         osgPresentation::PresentationInterface,
                         "osg::Object osgPresentation::PresentationInterface" )
{
    ADD_METHOD_OBJECT( "getSlideEventHandler", PresentationInterfaceGetSlideEventHandler );
    ADD_METHOD_OBJECT( "sendEventToViewer", PresentationInterfaceSendEventToViewer );
    ADD_METHOD_OBJECT( "sendEventToDevices", PresentationInterfaceSendEventToDevices );
}
