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
#include <osgDB/WriteFile>

#include <iostream>
#include <osg/AudioStream>

#include <osgDB/WriteFile>
#include <osgDB/ReadFile>
static float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }
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
    "    v_color = gl_Color;\n"
    "    gl_Position = gl_ModelViewProjectionMatrix *(gl_Vertex);\n"
    "}\n"
};
static const char* fragSource =
{
    "#version 120\n"
    "#extension GL_EXT_geometry_shader4 : enable\n"
    "uniform float u_anim1;\n"
    "varying vec4 v_color;\n"
    "void main(void)\n"
    "{\n"
    "    gl_FragColor =  v_color;\n"
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
    "	v_color = gl_Color;\n"
    "}\n"
};



static const char* geomSource =
{
    "#version 120\n"
    "#extension GL_EXT_geometry_shader4 : enable\n"
    "uniform float u_anim1;\n"
    " varying in vec4 v_color[];\n"
    " varying  vec4 out1;\n"
    " varying  vec4 out2;\n"
    "void main(void)\n"
    "{\n"
    "    vec4 v =vec4( gl_PositionIn[0].xyz,1);\n"
    " out1 =  v + vec4(u_anim1,0.,0.,0.);//  gl_Position = v + vec4(u_anim1,0.,0.,0.); \n"
    " out2 =  v_color[0] + vec4(0,0.,u_anim1,0.);// addblue \n"
    "  EmitVertex();\n"
    "    EndPrimitive();\n"
    "   out1 =  v - vec4(u_anim1,0.,0.,0.); // gl_Position = v - vec4(u_anim1,0.,0.,0.);  \n"
    " out2 =  v_color[0] + vec4(0,0.,u_anim1,0.);// addblue \n"
    " EmitVertex();\n"
    "    EndPrimitive();\n"
    "\n"
    "   out1=  v + vec4(0.,1.0-u_anim1,0.,0.);// gl_Position = v + vec4(0.,1.0-u_anim1,0.,0.); \n"
    " out2 =  v_color[0] + vec4(0,0.,u_anim1,0.);// addblue \n"
    "  EmitVertex();\n"
    "    EndPrimitive();\n"
    "   out1 =  v - vec4(0.,1.0-u_anim1,0.,0.); //gl_Position = v - vec4(0.,1.0-u_anim1,0.,0.); \n"
    " out2 =  v_color[0] + vec4(0,0.,u_anim1,0.);// addblue \n"
    "  EmitVertex();\n"
    "    EndPrimitive();\n"
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
    pgm->addTransformFeedBackVarying(std::string("out2"));
    pgm->setTransformFeedBackMode(GL_SEPARATE_ATTRIBS);

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
    osg::Vec3Array* vAry;

    //  virtual void drawImplementation( osg::RenderInfo& renderInfo ) const;
};
/////////////////////////////////////////////////////////////////////////////////////

class SomePointsRenderer : public osg::Geometry
{
public:

    SomePointsRenderer(SomePointsGenerator*_generator)
    {

        setUseVertexBufferObjects(true);
        setUseDisplayList(false);

        osg::Vec3Array* vAry2 = new osg::Vec3Array;
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
        //getOrCreateVertexBufferObject();
        sset->setAttribute( createRenderShader() );


    }



};

///////////////////////////////////////////////////////////////////////////
GLuint SomePointsGenerator::getNumPrimitivesGenerated()const
{
    return vAry->size()*3;
}
/*
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
*/


SomePointsGenerator::SomePointsGenerator():osg::Geometry()
{

    setUseVertexBufferObjects(true);
    setUseDisplayList(false);

    osg::StateSet* sset = getOrCreateStateSet();
    sset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    vAry = new osg::Vec3Array;;
    vAry->push_back( osg::Vec3(0,0,0) );
    vAry->push_back( osg::Vec3(0,1,0) );
    vAry->push_back( osg::Vec3(1,0,0) );
    vAry->push_back( osg::Vec3(1,1,0 ));
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
    osg::TransformFeedBackDrawCallback *tr=new  osg::TransformFeedBackDrawCallback();
    //tr->addTransformFeedbackBufferBinding(TargetArray(renderer->getVertexArray());
    setDrawCallback(tr);
}



///////////////////////////////////////////////////////////////////////////

int main( int , char** )
{
    osg::Geode* root( new osg::Geode );
    osg::ref_ptr<osg::Geometry > somePointsGenerator = new osg::Geometry();
    {
        somePointsGenerator->setUseVertexBufferObjects(true);
        somePointsGenerator->setUseDisplayList(false);

        osg::StateSet* sset = somePointsGenerator->getOrCreateStateSet();
        sset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        osg::ref_ptr<osg::Vec4Array> vAry = new osg::Vec4Array;;
        vAry->push_back( osg::Vec4(0,0,0,1) );
        vAry->push_back( osg::Vec4(0,1,0,1) );
        vAry->push_back( osg::Vec4(1,0,0,1) );
        vAry->push_back( osg::Vec4(1,1,0,1 ));
        vAry->setVertexBufferObject(new osg::VertexBufferObject);
        somePointsGenerator->setVertexArray( vAry );


        osg::ref_ptr<osg::Vec4Array> NoBluevAry2 = new osg::Vec4Array;;
        NoBluevAry2->push_back( osg::Vec4(1,0,0,1) );
        NoBluevAry2->push_back( osg::Vec4(0,1,0,1) );
        NoBluevAry2->push_back( osg::Vec4(0,0,0,1) );
        NoBluevAry2->push_back( osg::Vec4(1,1,0,1 ));
        NoBluevAry2->setVertexBufferObject(new osg::VertexBufferObject);
        somePointsGenerator->setColorArray(NoBluevAry2);

   somePointsGenerator->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
        somePointsGenerator->addPrimitiveSet( new osg::DrawArrays( GL_POINTS, 0, vAry->size() ) );

        osg::ref_ptr<osg::Program> _program=createGeneratorShader() ;
        sset->setAttribute(_program );

        // a generic cyclic animation value
        osg::Uniform* u_anim1( new osg::Uniform( "u_anim1", 0.9f ) );
        u_anim1->setUpdateCallback( new SineAnimation( 4, 0.5, 0.5 ) );
        sset->addUniform( u_anim1 );
    }

    osg::ref_ptr<osg::Geometry > somePointsRenderer = new osg::Geometry();
    {
        int numprimgen=somePointsGenerator->getVertexArray()->getNumElements()*4;
        somePointsRenderer->setUseVertexBufferObjects(true);
        somePointsRenderer->setUseDisplayList(false);

        osg::ref_ptr<osg::Vec4Array> vAry2 = new osg::Vec4Array;
        vAry2->resize(numprimgen);//_generator->getNumPrimitivesGenerated());
        vAry2->setVertexBufferObject(new osg::VertexBufferObject);
        somePointsRenderer-> setVertexArray(vAry2);

        osg::ref_ptr<osg::Vec4Array> gencolorvAry = new osg::Vec4Array;;
        gencolorvAry->resize(numprimgen);
        gencolorvAry->setVertexBufferObject(new osg::VertexBufferObject);
        somePointsRenderer->setColorArray(gencolorvAry);
somePointsRenderer->setColorBinding(osg::Geometry::BIND_PER_VERTEX);


        somePointsRenderer-> addPrimitiveSet( new osg::DrawArrays( GL_LINES, 0,numprimgen));

        osg::StateSet* sset =   somePointsRenderer-> getOrCreateStateSet();
        ///hacking rendering order
        /*osg::BlendFunc* bf = new
        osg::BlendFunc(osg::BlendFunc::SRC_ALPHA,
        osg::BlendFunc::ONE_MINUS_SRC_ALPHA );
        sset->setAttributeAndModes(bf);*/

      //  sset->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
        //getOrCreateVertexBufferObject();
        sset->setAttribute( createRenderShader() );


    }

    osg::TransformFeedBackDrawCallback *tr=new  osg::TransformFeedBackDrawCallback();
    osg::TransformFeedbackBufferBinding *tfbb=new   osg::TransformFeedbackBufferBinding (0);
    tfbb->setBufferObject(somePointsRenderer->getVertexArray()->getVertexBufferObject());
    osg::TransformFeedbackBufferBinding *tfbb2=new   osg::TransformFeedbackBufferBinding (1);
    tfbb2->setBufferObject(somePointsRenderer->getColorArray()->getVertexBufferObject());

    tfbb->setSize(somePointsRenderer->getVertexArray()->getTotalDataSize());
    tfbb2->setSize(somePointsRenderer->getColorArray()->getTotalDataSize());
    // tr->setTargetArray(somePointsRenderer->getVertexArray());

    tr->addTransformFeedbackBufferBinding(tfbb);
    tr->addTransformFeedbackBufferBinding(tfbb2);

        somePointsGenerator->setDrawCallback(tr);

    somePointsGenerator->getStateSet()->setAttribute(tfbb);
    somePointsGenerator->getStateSet()->setAttribute(tfbb2);
   // somePointsRenderer->getVertexArray()->getVertexBufferObject()->setCopyDataAndReleaseGLBufferObject(true);
    //pate->setRenderer( pate2);
    /*osg::ref_ptr<osg::Geode> geo=new osg::Geode();
    geo->addDrawable( somePointsGenerator );
    geo->addDrawable( somePointsRenderer );
    root->addChild(geo);*/
    root->addChild(somePointsGenerator);

    root->addChild(somePointsRenderer);

    somePointsGenerator->setName("SomePointsGenerator");
    somePointsRenderer->setName("SomePointsRenderer");
    somePointsGenerator->getStateSet()->setRenderBinDetails(0,"RenderBin");
    somePointsRenderer->getStateSet()->setRenderBinDetails(1,"RenderBin");
 osgDB::writeNodeFile(*root,"transfeedback.osgt");

 /*osg::Image * im=new osg::Image();
 osg::Image * im2D=osgDB::readImageFile("random20130606210642.png");

if(im2D){
 im->setImage(im2D->s()*im2D->t(),1,1,im2D->getInternalTextureFormat(),im2D->getPixelFormat(), im2D->getDataType(),im2D->data(),osg::Image::NO_DELETE);
 osgDB::writeImageFile(*im,"random1D.png");
}*/

unsigned int imsize=65656;
float * dataf=new float [imsize*4];
float *ptr=dataf;
while (ptr<dataf+imsize*4)
*ptr++=random(-1.0f,1.0f);
osg::Image * im=new osg::Image();
//im->allocateImage( 3*cell->_trees.size(), 1, 1, GL_RGBA, GL_FLOAT );
im->setImage(imsize,1,1,GL_RGBA32F_ARB,GL_RGBA,GL_FLOAT,(unsigned char*)dataf,osg::Image::NO_DELETE);
 osgDB::writeImageFile(*im,"random1D.ive");

    osgViewer::Viewer viewer;
    viewer.setSceneData( root );
    return viewer.run();
}
