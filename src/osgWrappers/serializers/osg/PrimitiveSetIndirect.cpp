#include <osg/PrimitiveSetIndirect>
#include <osg/ValueObject>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

namespace DACommandsArrays {
REGISTER_OBJECT_WRAPPER( IndirectCommandDrawArrays,
                         0,
                         osg::IndirectCommandDrawArrays,
                         "osg::Object osg::BufferData osg::IndirectCommandDrawArrays" )
{
    {
        UPDATE_TO_VERSION_SCOPED( 147 )
        ADDED_ASSOCIATE("osg::BufferData")
    }
}
}
namespace DECommandsArrays {
REGISTER_OBJECT_WRAPPER( IndirectCommandDrawElements,
                         0,
                         osg::IndirectCommandDrawElements,
                         "osg::Object osg::BufferData osg::IndirectCommandDrawElements" )
{
    {
        UPDATE_TO_VERSION_SCOPED( 147 )
        ADDED_ASSOCIATE("osg::BufferData")
    }
}
}
namespace DefaultDACommandsArrays {


static bool checkDACommands( const osg::DefaultIndirectCommandDrawArrays& node )
{
    return node.size()>0;
}

static bool readDACommands( osgDB::InputStream& is, osg::DefaultIndirectCommandDrawArrays& node )
{
    unsigned int elmt, size = 0; is >> size >> is.BEGIN_BRACKET;
    node.resize(size);
    for ( unsigned int i=0; i<size; ++i )
    {
        is >>elmt;    node.count(i) = elmt;
        is >>elmt;    node.instanceCount(i) = elmt;
        is >>elmt;    node.first(i) = elmt;
        is >>elmt;    node.baseInstance(i) = elmt;
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeDACommands( osgDB::OutputStream& os, const osg::DefaultIndirectCommandDrawArrays& node )
{
    unsigned int size = node.getNumElements();
    osg::DefaultIndirectCommandDrawArrays& nonconstnode =const_cast<osg::DefaultIndirectCommandDrawArrays&>(node);
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << nonconstnode.count(i);
        os << nonconstnode.instanceCount(i);
        os << nonconstnode.first(i);
        os << nonconstnode.baseInstance(i);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}


REGISTER_OBJECT_WRAPPER( osgDefaultIndirectCommandDrawArrays,
                         new osg::DefaultIndirectCommandDrawArrays,
                         osg::DefaultIndirectCommandDrawArrays,
                         "osg::Object osg::BufferData osg::IndirectCommandDrawArrays osg::DefaultIndirectCommandDrawArrays" )
{
    ADD_USER_SERIALIZER(DACommands);
}
}
namespace DefaultDECommandsArrays {

static bool checkDECommands( const osg::DefaultIndirectCommandDrawElements& node )
{
    return node.size()>0;
}

static bool readDECommands( osgDB::InputStream& is, osg::DefaultIndirectCommandDrawElements& node )
{
    unsigned int elmt, size = 0; is >> size >> is.BEGIN_BRACKET;
    node.resize(size);
    for ( unsigned int i=0; i<size; ++i )
    {
        is >>elmt;    node.count(i) = elmt;
        is >>elmt;    node.instanceCount(i) = elmt;
        is >>elmt;    node.firstIndex(i) = elmt;
        is >>elmt;    node.baseVertex(i) = elmt;
        is >>elmt;    node.baseInstance(i) = elmt;
    }
    is >> is.END_BRACKET;
    return true;
}

static bool writeDECommands( osgDB::OutputStream& os, const osg::DefaultIndirectCommandDrawElements& node )
{
    unsigned int size = node.getNumElements();
    osg::DefaultIndirectCommandDrawElements& nonconstnode =const_cast<osg::DefaultIndirectCommandDrawElements&>(node);
    os << size << os.BEGIN_BRACKET << std::endl;
    for ( unsigned int i=0; i<size; ++i )
    {
        os << nonconstnode.count(i);
        os << nonconstnode.instanceCount(i);
        os << nonconstnode.firstIndex(i);
        os << nonconstnode.baseVertex(i);
        os << nonconstnode.baseInstance(i);
    }
    os << os.END_BRACKET << std::endl;
    return true;
}

REGISTER_OBJECT_WRAPPER( osgDefaultIndirectCommandDrawElements,
                         new osg::DefaultIndirectCommandDrawElements,
                         osg::DefaultIndirectCommandDrawElements,
                         "osg::Object osg::BufferData osg::IndirectCommandDrawElements osg::DefaultIndirectCommandDrawElements" )
{
    {
        UPDATE_TO_VERSION_SCOPED( 147 )
        ADDED_ASSOCIATE("osg::BufferData")
    }
    ADD_USER_SERIALIZER(DECommands);
}
}
namespace DrawArraysIndirectWrapper {

REGISTER_OBJECT_WRAPPER( DrawArraysIndirect,
                         new osg::DrawArraysIndirect,
                         osg::DrawArraysIndirect,
                         "osg::Object osg::BufferData osg::PrimitiveSet osg::DrawArraysIndirect" )
{
    ADD_OBJECT_SERIALIZER( IndirectCommandArray, osg::IndirectCommandDrawArrays, new osg::DefaultIndirectCommandDrawArrays());
    ADD_UINT_SERIALIZER( FirstCommandToDraw, 0);
    ADD_INT_SERIALIZER( Stride, 0);
}

}
namespace MultiDrawArraysIndirectWrapper {

REGISTER_OBJECT_WRAPPER( MultiDrawArraysIndirect,
                         new osg::MultiDrawArraysIndirect,
                         osg::MultiDrawArraysIndirect,
                         "osg::Object osg::BufferData osg::PrimitiveSet osg::DrawArraysIndirect osg::MultiDrawArraysIndirect" )
{
    ADD_UINT_SERIALIZER( NumCommandsToDraw, 0);
}

}

namespace DrawElementsIndirectWrapper {

REGISTER_OBJECT_WRAPPER( DrawElementsIndirect,
                         0,
                         osg::DrawElementsIndirect,
                         "osg::Object osg::BufferData osg::PrimitiveSet osg::DrawElementsIndirect" )
{
    ADD_OBJECT_SERIALIZER( IndirectCommandArray, osg::IndirectCommandDrawElements, new osg::DefaultIndirectCommandDrawElements());
    ADD_UINT_SERIALIZER( FirstCommandToDraw, 0);
    ADD_INT_SERIALIZER( Stride, 0);
}

}
#define INDIRECTDRAW_ELEMENTS_WRAPPER( DRAWELEMENTS, ELEMENTTYPE ) \
    namespace Wrapper##DRAWELEMENTS { \
        REGISTER_OBJECT_WRAPPER( DRAWELEMENTS, new osg::DRAWELEMENTS, osg::DRAWELEMENTS, "osg::Object osg::BufferData osg::PrimitiveSet osg::DrawElementsIndirect osg::"#DRAWELEMENTS) \
        { \
                ADD_ISAVECTOR_SERIALIZER( vector, osgDB::BaseSerializer::ELEMENTTYPE, 4 ); \
        } \
    }\
    namespace WrapperMulti##DRAWELEMENTS { \
        REGISTER_OBJECT_WRAPPER( Multi##DRAWELEMENTS, new osg::Multi##DRAWELEMENTS, osg::Multi##DRAWELEMENTS, "osg::Object osg::BufferData osg::PrimitiveSet osg::DrawElementsIndirect osg::"#DRAWELEMENTS" osg::Multi"#DRAWELEMENTS) \
        { \
    ADD_UINT_SERIALIZER( NumCommandsToDraw, 0);\
        } \
    }\


INDIRECTDRAW_ELEMENTS_WRAPPER( DrawElementsIndirectUByte, RW_UCHAR )
INDIRECTDRAW_ELEMENTS_WRAPPER( DrawElementsIndirectUShort, RW_USHORT )
INDIRECTDRAW_ELEMENTS_WRAPPER( DrawElementsIndirectUInt, RW_UINT )


