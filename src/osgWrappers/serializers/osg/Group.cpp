#include <osg/Group>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkChildren( const osg::Group& node )
{
    return node.getNumChildren()>0;
}

static bool readChildren( osgDB::InputStream& is, osg::Group& node )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osg::ref_ptr<osg::Object> obj = is.readObject();
        osg::Node* child = dynamic_cast<osg::Node*>( obj.get() );
        if ( child ) node.addChild( child );
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeChildren( osgDB::OutputStream& os, const osg::Group& node )
{
    unsigned int size = node.getNumChildren();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << node.getChild(i);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

struct GroupGetNumChildren : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        osg::Group* group = reinterpret_cast<osg::Group*>(objectPtr);
        outputParameters.push_back(new osg::UIntValueObject("return", group->getNumChildren()));
        return true;
    }
};

struct GroupGetChild : public osgDB::MethodObject
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
        osg::Group* group = reinterpret_cast<osg::Group*>(objectPtr);
        outputParameters.push_back(group->getChild(index));

        return true;
    }
};

struct GroupSetChild : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.size()<2) return false;

        osg::Object* indexObject = inputParameters[0].get();
        OSG_NOTICE<<"GroupSetChild "<<indexObject->className()<<std::endl;

        unsigned int index = 0;
        osg::DoubleValueObject* dvo = dynamic_cast<osg::DoubleValueObject*>(indexObject);
        if (dvo) index = static_cast<unsigned int>(dvo->getValue());
        else
        {
            osg::UIntValueObject* uivo = dynamic_cast<osg::UIntValueObject*>(indexObject);
            if (uivo) index = uivo->getValue();
        }

        osg::Node* child = dynamic_cast<osg::Node*>(inputParameters[1].get());
        if (!child) return false;

        osg::Group* group = reinterpret_cast<osg::Group*>(objectPtr);
        group->setChild(index, child);

        return true;
    }
};

struct GroupAddChild : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osg::Node* child = dynamic_cast<osg::Node*>(inputParameters[0].get());
        if (!child) return false;

        osg::Group* group = reinterpret_cast<osg::Group*>(objectPtr);
        group->addChild(child);

        return true;
    }
};


struct GroupRemoveChild : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osg::Node* child = dynamic_cast<osg::Node*>(inputParameters[0].get());
        if (!child) return false;

        osg::Group* group = reinterpret_cast<osg::Group*>(objectPtr);
        group->removeChild(child);

        return true;
    }
};

REGISTER_OBJECT_WRAPPER( Group,
                         new osg::Group,
                         osg::Group,
                         "osg::Object osg::Node osg::Group" )
{
    ADD_USER_SERIALIZER( Children );  // _children

    ADD_METHOD_OBJECT( "getNumChildren", GroupGetNumChildren );
    ADD_METHOD_OBJECT( "getChild", GroupGetChild );
    ADD_METHOD_OBJECT( "setChild", GroupSetChild );
    ADD_METHOD_OBJECT( "addChild", GroupAddChild );
    ADD_METHOD_OBJECT( "removeChild", GroupRemoveChild );
}
