#include <osg/PrimitiveSet>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

namespace PrimitiveSetWrapper {

#define CUSTOM_BEGIN_ENUM_SERIALIZER(PROP, DEF) \
{ typedef osgDB::EnumSerializer<MyClass, MyClass::PROP, void> MySerializer; \
osg::ref_ptr<MySerializer> serializer = new MySerializer( \
    #PROP, MyClass::DEF, &MyClass::get##PROP, &MyClass::set##PROP)



REGISTER_OBJECT_WRAPPER( PrimitiveSet,
                         0,
                         osg::PrimitiveSet,
                         "osg::Object osg::PrimitiveSet" )
{

    ADD_INT_SERIALIZER( NumInstances, 0);

#if 1

    {
        typedef osgDB::EnumSerializer<MyClass, MyClass::Mode, void> MySerializer;

        typedef osg::PrimitiveSet::Mode (osg::PrimitiveSet::*Getter)() const;
        typedef void (osg::PrimitiveSet::*Setter)( osg::PrimitiveSet::Mode );

        osg::ref_ptr<MySerializer> serializer = new MySerializer( "Mode", osg::PrimitiveSet::POINTS,
                                                                  reinterpret_cast<Getter>(&osg::PrimitiveSet::getMode),
                                                                  reinterpret_cast<Setter>(&osg::PrimitiveSet::setMode));
        ADD_ENUM_VALUE( POINTS );
        ADD_ENUM_VALUE( LINES );
        ADD_ENUM_VALUE( LINE_STRIP );
        ADD_ENUM_VALUE( LINE_LOOP );
        ADD_ENUM_VALUE( TRIANGLES );
        ADD_ENUM_VALUE( TRIANGLE_STRIP );
        ADD_ENUM_VALUE( TRIANGLE_FAN );
        ADD_ENUM_VALUE( QUADS );
        ADD_ENUM_VALUE( QUAD_STRIP );
        ADD_ENUM_VALUE( POLYGON );
        ADD_ENUM_VALUE( LINES_ADJACENCY );
        ADD_ENUM_VALUE( LINE_STRIP_ADJACENCY );
        ADD_ENUM_VALUE( TRIANGLES_ADJACENCY );
        ADD_ENUM_VALUE( TRIANGLE_STRIP_ADJACENCY );
        ADD_ENUM_VALUE( PATCHES  );

        wrapper->addSerializer(serializer.get(), osgDB::BaseSerializer::RW_ENUM);
    }
#else
    ADD_GLENUM_SERIALIZER( Mode, GLenum, GL_NONE );
#endif
#if 0
    BEGIN_ENUM_SERIALIZER_NO_SET( Type, PrimitiveType );
            ADD_ENUM_VALUE( PrimitiveType );
            ADD_ENUM_VALUE( DrawArraysPrimitiveType );
            ADD_ENUM_VALUE( DrawArrayLengthsPrimitiveType );
            ADD_ENUM_VALUE( DrawElementsUBytePrimitiveType );
            ADD_ENUM_VALUE( DrawElementsUShortPrimitiveType );
            ADD_ENUM_VALUE( DrawElementsUIntPrimitiveType );
    END_ENUM_SERIALIZER();

    ADD_UINT_SERIALIZER_NO_SET( TotalDataSize, 0);
    ADD_UINT_SERIALIZER_NO_SET( NumPrimitives, 0);
    ADD_UINT_SERIALIZER_NO_SET( NumIndices, 0);

    wrapper->addSerializer(
        new osgDB::PropByValSerializer< osg::PrimitiveSet, bool > ("supportsBufferObject", false, &osg::PrimitiveSet::supportsBufferObject, 0, osgDB::BaseSerializer::RW_BOOL )
    );
#endif
}

}

namespace DrawArraysWrapper {

REGISTER_OBJECT_WRAPPER( DrawArrays,
                         new osg::DrawArrays,
                         osg::DrawArrays,
                         "osg::Object osg::PrimitiveSet osg::DrawArrays" )
{
    ADD_GLINT_SERIALIZER( First, 0);
    ADD_GLINT_SERIALIZER( Count, 0);
}

}

namespace DrawArrayLengthsWrapper {

REGISTER_OBJECT_WRAPPER( DrawArrayLengths,
                         new osg::DrawArrayLengths,
                         osg::DrawArrayLengths,
                         "osg::Object osg::PrimitiveSet osg::DrawArrayLengths" )
{
    ADD_GLINT_SERIALIZER( First, 0);
    ADD_ISAVECTOR_SERIALIZER( vector, osgDB::BaseSerializer::RW_INT, 4 );
}

}

#if 0
namespace DrawElementsWrapper {

struct ResizeDrawElements : public osgDB::MethodObject
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
        osg::DrawElements* de = reinterpret_cast<osg::DrawElements*>(objectPtr);
        de->resizeElements(index);

        return true;
    }
};

REGISTER_OBJECT_WRAPPER( DrawElements,
                         0,
                         osg::DrawElements,
                         "osg::Object osg::PrimitiveSet osg::DrawElements" )
{
    ADD_METHOD_OBJECT( "resizeElements", ResizeDrawElements );
}

}
#endif

#define DRAW_ELEMENTS_WRAPPER( DRAWELEMENTS, ELEMENTTYPE ) \
    namespace Wrapper##DRAWELEMENTS { \
        REGISTER_OBJECT_WRAPPER( DRAWELEMENTS, new osg::DRAWELEMENTS, osg::DRAWELEMENTS, "osg::Object osg::PrimitiveSet osg::"#DRAWELEMENTS) \
        { \
                ADD_ISAVECTOR_SERIALIZER( vector, osgDB::BaseSerializer::ELEMENTTYPE, 4 ); \
        } \
    }

DRAW_ELEMENTS_WRAPPER( DrawElementsUByte, RW_UCHAR )
DRAW_ELEMENTS_WRAPPER( DrawElementsUShort, RW_USHORT )
DRAW_ELEMENTS_WRAPPER( DrawElementsUInt, RW_UINT )
