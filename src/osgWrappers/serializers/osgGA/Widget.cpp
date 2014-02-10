#include <osgGA/Widget>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>


struct CreateGraphics : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters&, osg::Parameters&) const
    {
        osgGA::Widget* widget = reinterpret_cast<osgGA::Widget*>(objectPtr);
        widget->createGraphics();
        return true;
    }
};

struct CreateGraphicsImplementation : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters&, osg::Parameters&) const
    {
        osgGA::Widget* widget = reinterpret_cast<osgGA::Widget*>(objectPtr);
        widget->createGraphicsImplementation();
        return true;
    }
};

struct Enter : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters&, osg::Parameters&) const
    {
        osgGA::Widget* widget = reinterpret_cast<osgGA::Widget*>(objectPtr);
        widget->enter();
        return true;
    }
};

struct EnterImplementation : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters&, osg::Parameters&) const
    {
        osgGA::Widget* widget = reinterpret_cast<osgGA::Widget*>(objectPtr);
        widget->enterImplementation();
        return true;
    }
};

struct Leave : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters&, osg::Parameters&) const
    {
        osgGA::Widget* widget = reinterpret_cast<osgGA::Widget*>(objectPtr);
        widget->leave();
        return true;
    }
};

struct LeaveImplementation : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters&, osg::Parameters&) const
    {
        osgGA::Widget* widget = reinterpret_cast<osgGA::Widget*>(objectPtr);
        widget->leaveImplementation();
        return true;
    }
};


struct Traverse : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters&) const
    {
        osgGA::Widget* widget = reinterpret_cast<osgGA::Widget*>(objectPtr);
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
        osgGA::Widget* widget = reinterpret_cast<osgGA::Widget*>(objectPtr);
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
        osgGA::Widget* widget = reinterpret_cast<osgGA::Widget*>(objectPtr);
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
        osgGA::Widget* widget = reinterpret_cast<osgGA::Widget*>(objectPtr);
        osgGA::EventVisitor* ev = (inputParameters.size()>=1) ? dynamic_cast<osgGA::EventVisitor*>(inputParameters[0].get()) : 0;
        osgGA::Event* event = (inputParameters.size()>=2) ? dynamic_cast<osgGA::Event*>(inputParameters[1].get()) : 0;
        if (!widget || !ev || !event) return false;
        widget->handleImplementation(ev, event);
        return true;
    }
};


REGISTER_OBJECT_WRAPPER( Widget,
                         new osgGA::Widget,
                         osgGA::Widget,
                         "osg::Object osg::Node osg::Group osgGA::Widget" )
{
    BEGIN_ENUM_SERIALIZER( FocusBehaviour, FOCUS_FOLLOWS_POINTER );
        ADD_ENUM_VALUE( CLICK_TO_FOCUS );
        ADD_ENUM_VALUE( FOCUS_FOLLOWS_POINTER );
        ADD_ENUM_VALUE( EVENT_DRIVEN_FOCUS_DISABLED );
    END_ENUM_SERIALIZER();

    ADD_BOOL_SERIALIZER(HasEventFocus, false);

    ADD_BOUNDINGBOXF_SERIALIZER(Extents, osg::BoundingBoxf());

    ADD_METHOD_OBJECT( "createGraphics", CreateGraphics );
    ADD_METHOD_OBJECT( "createGraphicsImplementation", CreateGraphicsImplementation );

    ADD_METHOD_OBJECT( "enter", Enter );
    ADD_METHOD_OBJECT( "enterImplementation", EnterImplementation );

    ADD_METHOD_OBJECT( "leave", Leave );
    ADD_METHOD_OBJECT( "leaveImplementation", LeaveImplementation );

    ADD_METHOD_OBJECT( "traverse", Traverse );
    ADD_METHOD_OBJECT( "traverseImplementation", TraverseImplementation );

    ADD_METHOD_OBJECT( "handle", Handle );
    ADD_METHOD_OBJECT( "handleImplementation", HandleImplementation );

}
