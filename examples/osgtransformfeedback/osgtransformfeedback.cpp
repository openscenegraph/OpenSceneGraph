/* OpenSceneGraph example, osgtransformfeedback
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

/* file:        examples/osgtransformfeedback/osgtransformfeedback.cpp
* author:      Julien Valentin 2013-10-01
* copyright:   (C) 2013
* license:     OpenSceneGraph Public License (OSGPL)
*
* A demo of GLSL geometry shaders using OSG transform feedback
*
*/


#include <osg/GL2Extensions>
#include <osg/Notify>
#include <osg/ref_ptr>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Point>
#include <osg/Vec3>
#include <osg/Vec4>
#include <osg/Program>
#include <osg/Shader>
#include <osg/BlendFunc>

#include <osg/Uniform>
#include <osgViewer/Viewer>

#include <osg/BufferIndexBinding>

#include <iostream>

///////////////////////////////////////////////////////////////////////////

class SineAnimation: public osg::UniformCallback
{
public:
    SineAnimation( float rate = 1.0f, float scale = 1.0f, float offset = 0.0f ) :
        _rate(rate), _scale(scale), _offset(offset)
    {}

    void operator()( osg::Uniform* uniform, osg::NodeVisitor* nv )
    {
        float angle = _rate * nv->getFrameStamp()->getSimulationTime();
        float value = sinf( angle ) * _scale + _offset;
        uniform->set( value );
    }

private:
    const float _rate;
    const float _scale;
    const float _offset;
};

///////////////////////////////////////////////////////////////////////////
static const char* RendervertSource =
{
    "#version 120\n"
    "uniform float u_anim1;\n"
    "//in vec4 Vertex;\n"
    "varying vec4 v_color;\n"
    "void main(void)\n"
    "{\n"
    "    v_color = gl_Vertex;\n"
    "    gl_Position = gl_ModelViewProjectionMatrix *(gl_Vertex);\n"
    "}\n"
};
static const char* vertSource =
{
    "#version 120\n"
    "#extension GL_EXT_geometry_shader4 : enable\n"
    "uniform float u_anim1;\n"
    " varying out vec4 v_color;\n"
    "void main(void)\n"
    "{\n"
    "   gl_Position = (gl_Vertex);\n"
    "	v_color = gl_Vertex;\n"
    "}\n"
};



static const char* geomSource =
{
    "#version 120\n"
    "#extension GL_EXT_geometry_shader4 : enable\n"
    "uniform float u_anim1;\n"
    " varying in vec4 v_color[];\n"
    " varying  vec4 out1;\n"
    "void main(void)\n"
    "{\n"
    "    vec4 v =vec4( gl_PositionIn[0].xyz,1);\n"
    " out1 =  v + vec4(u_anim1,0.,0.,0.);//  gl_Position = v + vec4(u_anim1,0.,0.,0.); \n"
    "  EmitVertex();\n"
    "    EndPrimitive();\n"
    "   out1 =  v - vec4(u_anim1,0.,0.,0.); // gl_Position = v - vec4(u_anim1,0.,0.,0.);  \n"
    " EmitVertex();\n"
    "    EndPrimitive();\n"
    "\n"
    "   out1=  v + vec4(0.,1.0-u_anim1,0.,0.);// gl_Position = v + vec4(0.,1.0-u_anim1,0.,0.); \n"
    "  EmitVertex();\n"
    "    EndPrimitive();\n"
    "   out1 =  v - vec4(0.,1.0-u_anim1,0.,0.); //gl_Position = v - vec4(0.,1.0-u_anim1,0.,0.); \n"
    "  EmitVertex();\n"
    "    EndPrimitive();\n"
    "}\n"
};


static const char* fragSource =
{
    "#version 120\n"
    "#extension GL_EXT_geometry_shader4 : enable\n"
    "uniform float u_anim1;\n"
    "varying vec4 v_color_out;\n"
    "void main(void)\n"
    "{\n"
    "    gl_FragColor = vec4(1,0,0,1);//v_color_out;\n"
    "}\n"
};

osg::Program* createGeneratorShader()
{
    osg::Program* pgm = new osg::Program;
    pgm->setName( "osg transformfeedback demo" );
    pgm->addShader( new osg::Shader( osg::Shader::VERTEX,   vertSource ) );
    pgm->setParameter( GL_GEOMETRY_VERTICES_OUT_EXT, 4 );
    pgm->setParameter( GL_GEOMETRY_INPUT_TYPE_EXT, GL_POINTS );
    pgm->setParameter( GL_GEOMETRY_OUTPUT_TYPE_EXT, GL_POINTS );
    pgm->addShader( new osg::Shader( osg::Shader::GEOMETRY, geomSource ) );
    pgm->addTransformFeedBackVarying(std::string("out1"));
    pgm->setTransformFeedBackMode(GL_INTERLEAVED_ATTRIBS);
    return pgm;
}

osg::Program* createRenderShader()
{
    osg::Program* pgm = new osg::Program;
    pgm->setName( "osg transformfeedback renderer demo" );
    pgm->addShader( new osg::Shader( osg::Shader::VERTEX,   RendervertSource ) );
    pgm->addShader( new osg::Shader( osg::Shader::FRAGMENT, fragSource ) );
    return pgm;
}

//////////////////////////////////////////////////////////////////////////////////////



class SomePointsRenderer;
class SomePointsGenerator:public osg::Geometry
{
public:
    SomePointsGenerator();
    void setRenderer(osg::Geometry*renderer);
    GLuint getNumPrimitivesGenerated()const;

protected:
    osg::Program * _program;
    osg::ref_ptr<osg::VertexBufferObject> genbuffer;//Renderer buffer
    osg::Vec4Array* vAry;

    virtual void drawImplementation( osg::RenderInfo& renderInfo ) const;
};
/////////////////////////////////////////////////////////////////////////////////////

class SomePointsRenderer : public osg::Geometry
{
public:

    SomePointsRenderer(SomePointsGenerator*_generator)
    {

        setUseVertexBufferObjects(true);

        osg::Vec4Array* vAry2 = new osg::Vec4Array;
        vAry2->resize(_generator->getNumPrimitivesGenerated());
        setVertexArray(vAry2);
        addPrimitiveSet( new osg::DrawArrays( GL_LINES, 0,_generator->getNumPrimitivesGenerated()));

        osg::StateSet* sset = getOrCreateStateSet();
        ///hacking rendering order
        /*osg::BlendFunc* bf = new
        osg::BlendFunc(osg::BlendFunc::SRC_ALPHA,
        osg::BlendFunc::ONE_MINUS_SRC_ALPHA );
        sset->setAttributeAndModes(bf);*/

        sset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        getOrCreateVertexBufferObject();
        sset->setAttribute( createRenderShader() );

    }



};

///////////////////////////////////////////////////////////////////////////
GLuint SomePointsGenerator::getNumPrimitivesGenerated()const
{
    return vAry->size()*4;
}

void SomePointsGenerator::drawImplementation( osg::RenderInfo& renderInfo ) const
{

    //get output buffer
    unsigned int contextID = renderInfo.getState()->getContextID();

    GLuint ubuff= genbuffer->getOrCreateGLBufferObject(contextID)->getGLObjectID();

    osg::GLExtensions* ext = renderInfo.getState()->get<osg::GLExtensions>();

    ext->glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0,ubuff);

    glEnable(GL_RASTERIZER_DISCARD);

    ext->glBeginTransformFeedback(GL_POINTS);


    osg::Geometry::drawImplementation(  renderInfo );


    ext->glEndTransformFeedback();

    glDisable(GL_RASTERIZER_DISCARD);

    ext->glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, 0);
}



SomePointsGenerator::SomePointsGenerator():osg::Geometry()
{

    setUseVertexBufferObjects(true);

    osg::StateSet* sset = getOrCreateStateSet();
    sset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    vAry = new osg::Vec4Array;;
    vAry->push_back( osg::Vec4(0,0,0,1) );
    vAry->push_back( osg::Vec4(0,1,0,1) );
    vAry->push_back( osg::Vec4(1,0,0,1) );
    vAry->push_back( osg::Vec4(1,1,0,1 ));
    addPrimitiveSet( new osg::DrawArrays( GL_POINTS, 0, vAry->size() ) );
    setVertexArray( vAry );

    _program=createGeneratorShader() ;
    sset->setAttribute(_program );

    // a generic cyclic animation value
    osg::Uniform* u_anim1( new osg::Uniform( "u_anim1", 0.9f ) );
    u_anim1->setUpdateCallback( new SineAnimation( 4, 0.5, 0.5 ) );
    sset->addUniform( u_anim1 );

}

void SomePointsGenerator::setRenderer(osg::Geometry* renderer)
{
    genbuffer = renderer->getOrCreateVertexBufferObject();
}



///////////////////////////////////////////////////////////////////////////

int main( int , char** )
{
    osg::Geode* root( new osg::Geode );
    SomePointsGenerator * pate = new SomePointsGenerator();
    SomePointsRenderer* pate2 = new SomePointsRenderer(pate);
    pate->setRenderer( pate2);
    root->addDrawable( pate );
    root->addDrawable( pate2 );
    osgViewer::Viewer viewer;
    viewer.setSceneData( root );
    return viewer.run();
}
