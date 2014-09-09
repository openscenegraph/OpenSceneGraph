#include <osgUI/Widget>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>


struct Traverse : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters&) const
    {
        osgUI::Widget* widget = reinterpret_cast<osgUI::Widget*>(objectPtr);
        osg::NodeVisitor* nv = (inputParameters.size()>=1) ? dynamic_cast<osg::NodeVisitor*>(inputParameters[0].get()) : 0;
        if (!nv) return false;
        widget->traverse(*nv);
        return true;
    }
};

struct TraverseImplementation : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters&) const
    {
        osgUI::Widget* widget = reinterpret_cast<osgUI::Widget*>(objectPtr);
        osg::NodeVisitor* nv = (inputParameters.size()>=1) ? dynamic_cast<osg::NodeVisitor*>(inputParameters[0].get()) : 0;
        if (!nv) return false;
        widget->traverseImplementation(*nv);
        return true;
    }
};

struct Handle : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters&) const
    {
        osgUI::Widget* widget = reinterpret_cast<osgUI::Widget*>(objectPtr);
        osgGA::EventVisitor* ev = (inputParameters.size()>=1) ? dynamic_cast<osgGA::EventVisitor*>(inputParameters[0].get()) : 0;
        osgGA::Event* event = (inputParameters.size()>=2) ? dynamic_cast<osgGA::Event*>(inputParameters[1].get()) : 0;
        if (!widget || !ev || !event) return false;
        widget->handle(ev, event);
        return true;
    }
};

struct HandleImplementation : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters&) const
    {
        osgUI::Widget* widget = reinterpret_cast<osgUI::Widget*>(objectPtr);
        osgGA::EventVisitor* ev = (inputParameters.size()>=1) ? dynamic_cast<osgGA::EventVisitor*>(inputParameters[0].get()) : 0;
        osgGA::Event* event = (inputParameters.size()>=2) ? dynamic_cast<osgGA::Event*>(inputParameters[1].get()) : 0;
        if (!widget || !ev || !event) return false;
        widget->handleImplementation(ev, event);
        return true;
    }
};


REGISTER_OBJECT_WRAPPER( Widget,
                         new osgUI::Widget,
                         osgUI::Widget,
                         "osg::Object osg::Node osg::Group osgUI::Widget" )
{
    BEGIN_ENUM_SERIALIZER( FocusBehaviour, FOCUS_FOLLOWS_POINTER );
        ADD_ENUM_VALUE( CLICK_TO_FOCUS );
        ADD_ENUM_VALUE( FOCUS_FOLLOWS_POINTER );
        ADD_ENUM_VALUE( EVENT_DRIVEN_FOCUS_DISABLED );
    END_ENUM_SERIALIZER();

    ADD_BOOL_SERIALIZER(HasEventFocus, false);

    ADD_MAP_SERIALIZER(GraphicsSubgraphMap, osgUI::Widget::GraphicsSubgraphMap, osgDB::BaseSerializer::RW_INT, osgDB::BaseSerializer::RW_OBJECT);
    SET_USAGE( osgDB::BaseSerializer::GET_SET_PROPERTY);

    ADD_OBJECT_SERIALIZER( WidgetStateSet, osg::StateSet, NULL );
    SET_USAGE( osgDB::BaseSerializer::GET_SET_PROPERTY);

    ADD_BOUNDINGBOXF_SERIALIZER(Extents, osg::BoundingBoxf());

    ADD_OBJECT_SERIALIZER( FrameSettings, osgUI::FrameSettings, NULL );
    ADD_OBJECT_SERIALIZER( AlignmentSettings, osgUI::AlignmentSettings, NULL );
    ADD_OBJECT_SERIALIZER( TextSettings, osgUI::TextSettings, NULL );

    ADD_BOOL_SERIALIZER(Visible, true);
    ADD_BOOL_SERIALIZER(Enabled, true);

    ADD_METHOD( createGraphics );
    ADD_METHOD( createGraphicsImplementation );

    ADD_METHOD( enter );
    ADD_METHOD( enterImplementation );

    ADD_METHOD( leave );
    ADD_METHOD( leaveImplementation );

    ADD_METHOD_OBJECT( "traverse", Traverse );
    ADD_METHOD_OBJECT( "traverseImplementation", TraverseImplementation );

    ADD_METHOD_OBJECT( "handle", Handle );
    ADD_METHOD_OBJECT( "handleImplementation", HandleImplementation );

}
