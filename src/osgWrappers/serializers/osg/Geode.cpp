#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

static bool checkDrawables( const osg::Geode& node )
{
    return node.getNumDrawables()>0;
}

static bool readDrawables( osgDB::InputStream& is, osg::Geode& node )
{
    unsigned int size = 0; is >> size >> is.BEGIN_BRACKET;
    for ( unsigned int i=0; i<size; ++i )
    {
        osg::Drawable* drawable = dynamic_cast<osg::Drawable*>( is.readObject() );
        if ( drawable )
        {
            node.addDrawable( drawable );
        }
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeDrawables( osgDB::OutputStream& os, const osg::Geode& node )
{
    unsigned int size = node.getNumDrawables();
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << node.getDrawable(i);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

struct GeodeGetNumDrawables : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        osg::Geode* geode = reinterpret_cast<osg::Geode*>(objectPtr);
        outputParameters.push_back(new osg::UIntValueObject("return", geode->getNumDrawables()));
        return true;
    }
};


struct GeodeGetDrawable : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osg::Object* indexObject = inputParameters[0].get();
        osg::UIntValueObject* uivo = dynamic_cast<osg::UIntValueObject*>(indexObject);
        if (!uivo) return false;

        osg::Geode* geode = reinterpret_cast<osg::Geode*>(objectPtr);
        outputParameters.push_back(geode->getDrawable(uivo->getValue()));

        return true;
    }
};


struct GeodeSetDrawable : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.size()<2) return false;

        osg::Object* indexObject = inputParameters[0].get();
        OSG_NOTICE<<"GeodeSetChild "<<indexObject->className()<<std::endl;

        unsigned int index = 0;
        osg::DoubleValueObject* dvo = dynamic_cast<osg::DoubleValueObject*>(indexObject);
        if (dvo) index = static_cast<unsigned int>(dvo->getValue());
        else
        {
            osg::UIntValueObject* uivo = dynamic_cast<osg::UIntValueObject*>(indexObject);
            if (uivo) index = uivo->getValue();
        }

        osg::Drawable* child = dynamic_cast<osg::Drawable*>(inputParameters[1].get());
        if (!child) return false;

        osg::Geode* geode = reinterpret_cast<osg::Geode*>(objectPtr);
        geode->setDrawable(index, child);

        return true;
    }
};

struct GeodeAddDrawable : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osg::Drawable* child = dynamic_cast<osg::Drawable*>(inputParameters[0].get());
        if (!child) return false;

        osg::Geode* geode = reinterpret_cast<osg::Geode*>(objectPtr);
        geode->addDrawable(child);

        return true;
    }
};


struct GeodeRemoveDrawable : public osgDB::MethodObject
{
    virtual bool run(void* objectPtr, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        if (inputParameters.empty()) return false;

        osg::Drawable* child = dynamic_cast<osg::Drawable*>(inputParameters[0].get());
        if (!child) return false;

        osg::Geode* geode = reinterpret_cast<osg::Geode*>(objectPtr);
        geode->removeDrawable(child);

        return true;
    }
};

REGISTER_OBJECT_WRAPPER( Geode,
                         new osg::Geode,
                         osg::Geode,
                         "osg::Object osg::Node osg::Geode" )
{
    ADD_USER_SERIALIZER( Drawables );  // _drawables

    ADD_METHOD_OBJECT( "getNumDrawables", GeodeGetNumDrawables );
    ADD_METHOD_OBJECT( "getDrawable", GeodeGetDrawable );
    ADD_METHOD_OBJECT( "setDrawable", GeodeSetDrawable );
    ADD_METHOD_OBJECT( "addDrawable", GeodeAddDrawable );
    ADD_METHOD_OBJECT( "removeDrawable", GeodeRemoveDrawable );
}
