/* OpenSceneGraph example, osgblenddrawbuffers.
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

#include <osg/Geometry>
#include <osg/Geode>
#include <osg/Camera>
#include <osg/Texture2D>
#include <osg/BlendFunci>
#include <osg/ColorMaski>
#include <osg/Capability>
#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <iostream>

osg::Camera* createMRTCamera( std::vector<osg::Texture*>& attachedTextures )
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setClearColor( osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f) );
    camera->setClearMask( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );
    camera->setRenderTargetImplementation( osg::Camera::FRAME_BUFFER_OBJECT );
    camera->setRenderOrder( osg::Camera::PRE_RENDER );

    for ( int i=0; i<4; ++i )
    {
        osg::Texture2D* tex = new osg::Texture2D;
        tex->setTextureSize( 1024, 1024 );
        tex->setInternalFormat( GL_RGBA );
        tex->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
        tex->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );
        attachedTextures.push_back( tex );

        camera->setViewport( 0, 0, tex->getTextureWidth(), tex->getTextureHeight() );
        camera->attach( osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER0+i), tex );
    }
    return camera.release();
}

osg::Camera* createHUDCamera( double left, double right, double bottom, double top )
{
    osg::ref_ptr<osg::Camera> camera = new osg::Camera;
    camera->setReferenceFrame( osg::Transform::ABSOLUTE_RF );
    camera->setClearMask( GL_DEPTH_BUFFER_BIT );
    camera->setRenderOrder( osg::Camera::POST_RENDER );
    camera->setAllowEventFocus( false );
    camera->setProjectionMatrix( osg::Matrix::ortho2D(left, right, bottom, top) );
    camera->getOrCreateStateSet()->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
    return camera.release();
}

int main( int argc, char **argv )
{
    osg::ArgumentParser arguments(&argc, argv);
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName() + " is the example which demonstrates how to enable/disable blending on specified draw buffers in multi-rendering-target cases.");

    std::vector<osg::Texture*> textures;
    bool useGlobalBlending = false;
    if ( arguments.read("--no-draw-buffers") ) useGlobalBlending = true;


    osg::ref_ptr<osg::Node> cessna = osgDB::readRefNodeFile("cessna.osgt");
    if (!cessna)
    {
        OSG_NOTICE<<"Cannot not find model 'cessna.osg' to render"<<std::endl;
        return 1;
    }

    // Create a camera to output multi-rendering-targets (MRT)
    osg::ref_ptr<osg::Camera> mrtCam = createMRTCamera( textures );
    mrtCam->addChild( cessna );

    // Create shader program to be used
    const char* mrtFragmentCode = {
        "void main() {\n"
        "   gl_FragData[0] = gl_Color * vec4(1.0, 1.0, 1.0, 0.7);\n"
        "   gl_FragData[1] = vec4(0.0, 1.0, 1.0, 0.0);\n"
        "   gl_FragData[2] = vec4(1.0, 0.0, 1.0, 0.3);\n"
        "   gl_FragData[3] = vec4(1.0, 1.0, 0.0, 1.0);\n"
        "}\n"
    };
    osg::ref_ptr<osg::Program> program = new osg::Program;
    program->addShader( new osg::Shader(osg::Shader::FRAGMENT, mrtFragmentCode) );

    osg::ref_ptr<osg::StateSet> ss = mrtCam->getOrCreateStateSet();
    ss->setAttributeAndModes( program );

    // Apply blending to the original scene in MRT
    if ( !useGlobalBlending )
    {
        // Only enable blending on the first draw buffer so other three outputs are
        // totally opaque, which is important for MRT cases
        ss->setAttribute( new osg::Enablei(GL_BLEND, 0) );
        ss->setAttribute( new osg::Disablei(GL_BLEND, 1) );
        ss->setAttribute( new osg::Disablei(GL_BLEND, 2) );
        ss->setAttribute( new osg::Disablei(GL_BLEND, 3) );

        // Accept different blend/colormask attributes on multiple render targets
        osg::ref_ptr<osg::BlendFunci> blend0 = new osg::BlendFunci(0, GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        osg::ref_ptr<osg::ColorMaski> colormask3 = new osg::ColorMaski(3, false, true, false, true);
        ss->setAttribute( blend0 );
        ss->setAttributeAndModes( colormask3 );
    }
    else
    {
        // When separated blending is disabled, all rendering targets will be affected
        // by its alpha channel and you will see each output blended with the background.
        //
        // This causes a big program in situations like deferred shading because we may
        // have to save different scene data to MRT 'GBuffer', in which alpha channels are
        // used to store certain attributes rather than opacity. These attributes can be
        // reused in following post-processing steps.
        //
        // For such targets, alpha blending must be disabled; otherwise it will mess the
        // output. That is why this example exists!
        osg::ref_ptr<osg::BlendFunc> blend = new osg::BlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        ss->setAttributeAndModes( blend.get() );
    }

    // Create some quads to be shown on screen to contain the MRT result
    osg::ref_ptr<osg::Geode> quad = new osg::Geode;
    for ( unsigned int i=0; i<textures.size(); ++i )
    {
        osg::Geometry* geom = osg::createTexturedQuadGeometry(
            osg::Vec3((float)i/(float)textures.size(), 0.0f, 0.0f),
            osg::Vec3(1.0f/(float)textures.size()-0.01f,0.0f,0.0f), osg::Vec3(0.0f,1.0f,0.0f) );
        geom->getOrCreateStateSet()->setTextureAttributeAndModes( 0, textures[i] );
        quad->addDrawable( geom );
    }

    osg::Camera* hudCam = createHUDCamera( 0.0, 1.0, 0.0, 1.0 );
    hudCam->addChild( quad.get() );

    // Construct scene graph and viewer
    osg::ref_ptr<osg::Group> root = new osg::Group;
    root->addChild( mrtCam );
    root->addChild( hudCam );

    osgViewer::Viewer viewer;
    viewer.setSceneData( root.get() );
    return viewer.run();
}
