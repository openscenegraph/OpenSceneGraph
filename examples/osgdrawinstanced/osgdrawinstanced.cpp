/* OpenSceneGraph example, osgdrawinstanced.
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
//
// This code is copyright (c) 2008 Skew Matrix Software LLC. You may use
// the code under the licensing terms described above.
//

#include <osgDB/ReadFile>
#include <osgViewer/Viewer>
#include <osg/Geometry>
#include <osg/Texture2D>

#include <osgDB/WriteFile>

#include <iostream>

void
createDAIGeometry( osg::Geometry& geom, int nInstances=1 )
{
    const float halfDimX( .5 );
    const float halfDimZ( .5 );

    osg::Vec3Array* v = new osg::Vec3Array;
    v->resize( 4 );
    geom.setVertexArray( v );

    // Geometry for a single quad.
    (*v)[ 0 ] = osg::Vec3( -halfDimX, 0., -halfDimZ );
    (*v)[ 1 ] = osg::Vec3( halfDimX, 0., -halfDimZ );
    (*v)[ 2 ] = osg::Vec3( halfDimX, 0., halfDimZ );
    (*v)[ 3 ] = osg::Vec3( -halfDimX, 0., halfDimZ );

    // Use the DrawArraysInstanced PrimitiveSet and tell it to draw 1024 instances.
    geom.addPrimitiveSet( new osg::DrawArrays( GL_QUADS, 0, 4, nInstances ) );
}


osg::StateSet*
createStateSet()
{
    osg::ref_ptr< osg::StateSet > ss = new osg::StateSet;

    // Create a vertex program that references the gl_InstanceID to
    // render each instance uniquely. gl_InstanceID will be in the range
    // 0 to numInstances-1 (1023 in our case).
    std::string vertexSource =
        "#extension GL_EXT_gpu_shader4 : enable\n"
        "uniform sampler2D osgLogo; \n"
        "uniform float osg_SimulationTime; \n"

        "void main() \n"
        "{ \n"
            // Using the instance ID, generate "texture coords" for this instance.
            "vec2 tC; \n"
            "float r = float(gl_InstanceID) / 32.; \n"
            "tC.s = fract( r ); tC.t = floor( r ) / 32.; \n"
            // Get the color from the OSG logo.
            "gl_FrontColor = texture2D( osgLogo, tC ); \n"

            // Use the (scaled) tex coord to translate the position of the vertices.
            "vec4 pos = vec4( tC.s * 48., 0., tC.t * 48., 1. ); \n"

            // Compute a rotation angle from the instanceID and elapsed time.
            "float timeOffset = gl_InstanceID / (32. * 32.); \n"
            "float angle = ( osg_SimulationTime - timeOffset ) * 6.283; \n"
            "float sa = sin( angle ); \n"
            "float ca = cos( angle ); \n"
            // New orientation, rotate around z axis.
            "vec4 newX = vec4( ca, sa, 0., 0. ); \n"
            "vec4 newY = vec4( sa, ca, 0., 0. ); \n"
            "vec4 newZ = vec4( 0., 0., 1., 0. ); \n"
            "mat4 mV = mat4( newX, newY, newZ, pos ); \n"
            "gl_Position = ( gl_ModelViewProjectionMatrix * mV * gl_Vertex ); \n"
        "} \n";

    osg::ref_ptr< osg::Shader > vertexShader = new osg::Shader();
    vertexShader->setType( osg::Shader::VERTEX );
    vertexShader->setShaderSource( vertexSource );

    osg::ref_ptr< osg::Program > program = new osg::Program();
    program->addShader( vertexShader.get() );

    ss->setAttribute( program.get(),
        osg::StateAttribute::ON | osg::StateAttribute::PROTECTED );

    osg::ref_ptr< osg::Image> iLogo = osgDB::readRefImageFile( "Images/osg128.png" );
    if( !iLogo.valid() )
    {
        osg::notify( osg::ALWAYS ) << "Can't open image file osg128.png" << std::endl;
        return( NULL );
    }
    osg::Texture2D* texLogo = new osg::Texture2D( iLogo.get() );
    texLogo->setFilter( osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR );
    texLogo->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR );

    ss->setTextureAttribute( 0, texLogo );

    osg::ref_ptr< osg::Uniform > texLogoUniform =
        new osg::Uniform( "osgLogo", 0 );
    ss->addUniform( texLogoUniform.get() );

    return( ss.release() );
}


int main( int argc, char **argv )
{
    osg::ArgumentParser arguments(&argc, argv);

   // Make a scene graph consisting of a single Geode, containing
    // a single Geometry, and a single PrimitiveSet.
    osg::ref_ptr< osg::Geode > geode = new osg::Geode;

    osg::ref_ptr< osg::Geometry > geom = new osg::Geometry;
    // Configure the Geometry for use with EXT_draw_arrays:
    // DL off and buffer objects on.
    geom->setUseDisplayList( false );
    geom->setUseVertexBufferObjects( true );
    // OSG has no clue where out vertex shader will place the geometric data,
    // so specify an initial bound to allow proper culling and near/far computation.
    osg::BoundingBox bb( -1., -.1, -1., 49., 1., 49. );
    geom->setInitialBound( bb );
    // Add geometric data and the PrimitiveSet. Specify numInstances as 32*32 or 1024.
    createDAIGeometry( *geom, 32*32 );
    geode->addDrawable( geom.get() );

    // Create a StateSet to render the instanced Geometry.
    osg::ref_ptr< osg::StateSet > ss = createStateSet();
    geode->setStateSet( ss.get() );

    // osgDB::writeNodeFile(*geode, "instanced.osgt");

    osgViewer::Viewer viewer(arguments);
    viewer.setSceneData( geode.get() );
    return viewer.run();
}
