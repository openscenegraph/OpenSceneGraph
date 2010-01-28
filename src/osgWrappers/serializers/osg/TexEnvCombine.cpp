#include <osg/TexEnvCombine>
#include <osgDB/ObjectWrapper>
#include <osgDB/InputStream>
#include <osgDB/OutputStream>

REGISTER_OBJECT_WRAPPER( TexEnvCombine,
                         new osg::TexEnvCombine,
                         osg::TexEnvCombine,
                         "osg::Object osg::StateAttribute osg::TexEnvCombine" )
{
    ADD_GLENUM_SERIALIZER( Combine_RGB, GLint, GL_NONE );  // _combine_RGB
    ADD_GLENUM_SERIALIZER( Combine_Alpha, GLint, GL_NONE );  // _combine_Alpha
    ADD_GLENUM_SERIALIZER( Source0_RGB, GLint, GL_NONE );  // _source0_RGB
    ADD_GLENUM_SERIALIZER( Source1_RGB, GLint, GL_NONE );  // _source1_RGB
    ADD_GLENUM_SERIALIZER( Source2_RGB, GLint, GL_NONE );  // _source2_RGB
    ADD_GLENUM_SERIALIZER( Source0_Alpha, GLint, GL_NONE );  // _source0_Alpha
    ADD_GLENUM_SERIALIZER( Source1_Alpha, GLint, GL_NONE );  // _source1_Alpha
    ADD_GLENUM_SERIALIZER( Source2_Alpha, GLint, GL_NONE );  // _source2_Alpha
    ADD_GLENUM_SERIALIZER( Operand0_RGB, GLint, GL_NONE );  // _operand0_RGB
    ADD_GLENUM_SERIALIZER( Operand1_RGB, GLint, GL_NONE );  // _operand1_RGB
    ADD_GLENUM_SERIALIZER( Operand2_RGB, GLint, GL_NONE );  // _operand2_RGB
    ADD_GLENUM_SERIALIZER( Operand0_Alpha, GLint, GL_NONE );  // _operand0_Alpha
    ADD_GLENUM_SERIALIZER( Operand1_Alpha, GLint, GL_NONE );  // _operand1_Alpha
    ADD_GLENUM_SERIALIZER( Operand2_Alpha, GLint, GL_NONE );  // _operand2_Alpha
    ADD_FLOAT_SERIALIZER( Scale_RGB, 1.0f );  // _scale_RGB
    ADD_FLOAT_SERIALIZER( Scale_Alpha, 1.0f );  // _scale_Alpha
    ADD_VEC4_SERIALIZER( ConstantColor, osg::Vec4() );  // _constantColor
}
