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


}
