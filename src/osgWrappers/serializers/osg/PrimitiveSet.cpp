#include <osg/PrimitiveSet>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

namespace PrimitiveSetWrapper {

REGISTER_OBJECT_WRAPPER( PrimitiveSet,
                         0,
                         osg::PrimitiveSet,
                         "osg::Object osg::PrimitiveSet" )
{

    BEGIN_ENUM_SERIALIZER_NO_SET( Type, PrimitiveType );
            ADD_ENUM_VALUE( PrimitiveType );
            ADD_ENUM_VALUE( DrawArraysPrimitiveType );
            ADD_ENUM_VALUE( DrawArrayLengthsPrimitiveType );
            ADD_ENUM_VALUE( DrawElementsUBytePrimitiveType );
            ADD_ENUM_VALUE( DrawElementsUShortPrimitiveType );
            ADD_ENUM_VALUE( DrawElementsUIntPrimitiveType );
    END_ENUM_SERIALIZER();

    ADD_GLENUM_SERIALIZER( Mode, GLenum, GL_NONE );

    ADD_UINT_SERIALIZER_NO_SET( TotalDataSize, 0);
    ADD_UINT_SERIALIZER_NO_SET( NumPrimitives, 0);

    wrapper->addSerializer(
        new osgDB::PropByValSerializer< osg::PrimitiveSet, bool > ("supportsBufferObject", false, &osg::PrimitiveSet::supportsBufferObject, 0, osgDB::BaseSerializer::RW_BOOL )
    );
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

#define DRAW_ELEMENTS_WRAPPER( DRAWELEMENTS ) \
    namespace Wrapper##DRAWELEMENTS { REGISTER_OBJECT_WRAPPER( DRAWELEMENTS, new osg::DRAWELEMENTS, osg::DRAWELEMENTS, "osg::Object osg::PrimitiveSet osg::DrawElements "#DRAWELEMENTS) {} }

DRAW_ELEMENTS_WRAPPER( DrawElementsUByte )
DRAW_ELEMENTS_WRAPPER( DrawElementsUShort )
DRAW_ELEMENTS_WRAPPER( DrawElementsUInt )
