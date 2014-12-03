/* OpenSceneGraph example, osgmultiplerendertargets.
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

#include <osg/GLExtensions>
#include <osg/Node>
#include <osg/Geometry>
#include <osg/Notify>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/ColorMask>
#include <osg/Material>
#include <osg/Capability>

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgViewer/Viewer>

#include <iostream>
#include <stdio.h>

//
// Below is relatively straight forward example of using the OpenGL multiple render targets (MRT) extension
// to FrameBufferObjects/GLSL shaders.
//
// Another, more sophisticated MRT example can be found in the osgstereomatch example.
//


// The callback modifies an input image.
struct MyCameraPostDrawCallback : public osg::Camera::DrawCallback
{
    MyCameraPostDrawCallback(osg::Image* image):
        _image(image)
    {
    }

    virtual void operator () (const osg::Camera& /*camera*/) const
    {
        if (_image && _image->getPixelFormat()==GL_RGBA && _image->getDataType()==GL_UNSIGNED_BYTE)
        {
            // we'll pick out the center 1/2 of the whole image,
            int column_start = _image->s()/4;
            int column_end = 3*column_start;

            int row_start = _image->t()/4;
            int row_end = 3*row_start;

            // and then halve their contribution
            for(int r=row_start; r<row_end; ++r)
            {
                unsigned char* data = _image->data(column_start, r);
                for(int c=column_start; c<column_end; ++c)
                {
                    (*data) = (*data)/2; ++data;
                    (*data) = (*data)/2; ++data;
                    (*data) = (*data)/2; ++data;
                    (*data) = 255; ++data;
                }
            }

            _image->dirty();
        }
        else if (_image && _image->getPixelFormat()==GL_RGBA && _image->getDataType()==GL_FLOAT)
        {
            // we'll pick out the center 1/2 of the whole image,
            int column_start = _image->s()/4;
            int column_end = 3*column_start;

            int row_start = _image->t()/4;
            int row_end = 3*row_start;

            // and then halve their contribution
            for(int r=row_start; r<row_end; ++r)
            {
                float* data = (float*)_image->data(column_start, r);
                for(int c=column_start; c<column_end; ++c)
                {
                    (*data) = (*data)/2.0; ++data;
                    (*data) = (*data)/2.0; ++data;
                    (*data) = (*data)/2.0; ++data;
                    (*data) = 1.0f; ++data;
                }
            }

            _image->dirty();

            //print out the first three values
            float* data = (float*)_image->data(0, 0);
            fprintf(stderr,"Float pixel data: r %e g %e b %e\n", data[0], data[1], data[2]);
        }
    }

    osg::Image* _image;
};

#define NUM_TEXTURES 4

// The quad geometry is used by the render to texture camera to generate multiple textures.
osg::Group* createRTTQuad(unsigned int tex_width, unsigned int tex_height, bool useHDR)
{
    osg::Group *top_group = new osg::Group;

    osg::ref_ptr<osg::Geode> quad_geode = new osg::Geode;

    osg::ref_ptr<osg::Vec3Array> quad_coords = new osg::Vec3Array; // vertex coords
    // counter-clockwise
    quad_coords->push_back(osg::Vec3d(0, 0, -1));
    quad_coords->push_back(osg::Vec3d(1, 0, -1));
    quad_coords->push_back(osg::Vec3d(1, 1, -1));
    quad_coords->push_back(osg::Vec3d(0, 1, -1));

    osg::ref_ptr<osg::Vec2Array> quad_tcoords = new osg::Vec2Array; // texture coords
    quad_tcoords->push_back(osg::Vec2(0, 0));
    quad_tcoords->push_back(osg::Vec2(tex_width, 0));
    quad_tcoords->push_back(osg::Vec2(tex_width, tex_height));
    quad_tcoords->push_back(osg::Vec2(0, tex_height));

    osg::ref_ptr<osg::Geometry> quad_geom = new osg::Geometry;
    osg::ref_ptr<osg::DrawArrays> quad_da = new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4);

    osg::ref_ptr<osg::Vec4Array> quad_colors = new osg::Vec4Array;
    quad_colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

    quad_geom->setVertexArray(quad_coords.get());
    quad_geom->setTexCoordArray(0, quad_tcoords.get());
    quad_geom->addPrimitiveSet(quad_da.get());
    quad_geom->setColorArray(quad_colors.get(), osg::Array::BIND_OVERALL);

    osg::StateSet *stateset = quad_geom->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    stateset->addUniform(new osg::Uniform("width", (int)tex_width));

    // Attach shader, glFragData is used to create data for multiple render targets

    if (useHDR) {
        static const char *shaderSource = {
            "uniform int width;"
            "void main(void)\n"
            "{\n"
            "    gl_FragData[0] = vec4(-1e-12,0,0,1);\n"
            "    gl_FragData[1] = vec4(0,1e-12,0,1);\n"
            "    gl_FragData[2] = vec4(0,0,1e-12,1);\n"
            "    gl_FragData[3] = vec4(0,0,1e-12,1);\n"
            "}\n"
        };

        osg::ref_ptr<osg::Shader> fshader = new osg::Shader( osg::Shader::FRAGMENT , shaderSource);
        osg::ref_ptr<osg::Program> program = new osg::Program;
        program->addShader(fshader.get());
        stateset->setAttributeAndModes(program.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    } else {
        static const char *shaderSource = {
            "uniform int width;"
            "void main(void)\n"
            "{\n"
            "    gl_FragData[0] = vec4(1,0,0,1);\n"
            "    gl_FragData[1] = vec4(0,1,0,1);\n"
            "    gl_FragData[2] = vec4(0,0,1,1);\n"
            "    gl_FragData[3] = vec4(0,0,1,1);\n"
            "}\n"
        };

        osg::ref_ptr<osg::Shader> fshader = new osg::Shader( osg::Shader::FRAGMENT , shaderSource);
        osg::ref_ptr<osg::Program> program = new osg::Program;
        program->addShader(fshader.get());
        stateset->setAttributeAndModes(program.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
    }

    quad_geode->addDrawable(quad_geom.get());

    top_group->addChild(quad_geode.get());

    return top_group;
}

// Here a scene consisting of a single quad is created. This scene is viewed by the screen camera.
// The quad is textured using a shader and the multiple textures generated in the RTT stage.
osg::Node* createScene(osg::Node* cam_subgraph, unsigned int tex_width, unsigned int tex_height, bool useHDR, bool useImage, bool useMultiSample)
{
    if (!cam_subgraph) return 0;

    // create a group to contain the quad and the pre render camera.
    osg::Group* parent = new osg::Group;

    // textures to render to and to use for texturing of the final quad
    osg::TextureRectangle* textureRect[NUM_TEXTURES] = {0,0,0,0};

    for (int i=0;i<NUM_TEXTURES;i++) {
        textureRect[i] = new osg::TextureRectangle;
        textureRect[i]->setTextureSize(tex_width, tex_height);
        textureRect[i]->setInternalFormat(GL_RGBA);
        textureRect[i]->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
        textureRect[i]->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

        if (useHDR)
        {
            // Default HDR format
            textureRect[i]->setInternalFormat(GL_RGBA32F_ARB);

            // GL_FLOAT_RGBA32_NV might be supported on pre 8-series GPUs
            //textureRect[i]->setInternalFormat(GL_FLOAT_RGBA32_NV);

            // GL_RGBA16F_ARB can be used with this example,
            // but modify e-12 and e12 in the shaders accordingly
            //textureRect[i]->setInternalFormat(GL_RGBA16F_ARB);

            textureRect[i]->setSourceFormat(GL_RGBA);
            textureRect[i]->setSourceType(GL_FLOAT);
        }
    }

    // first create the geometry of the quad
    {
        osg::Geometry* polyGeom = new osg::Geometry();

        polyGeom->setSupportsDisplayList(false);

        osg::Vec3Array* vertices = new osg::Vec3Array;
        osg::Vec2Array* texcoords = new osg::Vec2Array;

        vertices->push_back(osg::Vec3d(0,0,0));
        texcoords->push_back(osg::Vec2(0,0));

        vertices->push_back(osg::Vec3d(1,0,0));
        texcoords->push_back(osg::Vec2(tex_width,0));

        vertices->push_back(osg::Vec3d(1,0,1));
        texcoords->push_back(osg::Vec2(tex_width,tex_height));

        vertices->push_back(osg::Vec3d(0,0,1));
        texcoords->push_back(osg::Vec2(0,tex_height));

        polyGeom->setVertexArray(vertices);
        polyGeom->setTexCoordArray(0,texcoords);

        osg::Vec4Array* colors = new osg::Vec4Array;
        colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
        polyGeom->setColorArray(colors, osg::Array::BIND_OVERALL);

        polyGeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,vertices->size()));

        // now we need to add the textures (generated by RTT) to the Drawable.
        osg::StateSet* stateset = new osg::StateSet;
        for (int i=0;i<NUM_TEXTURES;i++){
            stateset->setTextureAttributeAndModes(i, textureRect[i], osg::StateAttribute::ON);
        }

        polyGeom->setStateSet(stateset);

        // Attach a shader to the final quad to combine the input textures.
        if (useHDR) {
            static const char *shaderSource = {
                "uniform sampler2DRect textureID0;\n"
                "uniform sampler2DRect textureID1;\n"
                "uniform sampler2DRect textureID2;\n"
                "uniform sampler2DRect textureID3;\n"
                "uniform float width;\n"
                "uniform float height; \n"
                "void main(void)\n"
                "{\n"
                "    gl_FragData[0] = \n"
                "     vec4(  -1e12 * texture2DRect( textureID0, gl_TexCoord[0].st ).rgb, 1) + \n"
                "     vec4(   1e12 * texture2DRect( textureID1, gl_TexCoord[0].st ).rgb, 1) + \n"
                "     vec4(   1e12 * texture2DRect( textureID2, gl_TexCoord[0].st ).rgb, 1) + \n"
                "     vec4(-0.5e12 * texture2DRect( textureID3, gl_TexCoord[0].st ).rgb, 1);  \n"
                "}\n"
            };
            osg::ref_ptr<osg::Shader> fshader = new osg::Shader( osg::Shader::FRAGMENT , shaderSource);
            osg::ref_ptr<osg::Program> program = new osg::Program;
            program->addShader( fshader.get());
            stateset->setAttributeAndModes( program.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );
        } else {
            static const char *shaderSource = {
                "uniform sampler2DRect textureID0;\n"
                "uniform sampler2DRect textureID1;\n"
                "uniform sampler2DRect textureID2;\n"
                "uniform sampler2DRect textureID3;\n"
                "void main(void)\n"
                "{\n"
                "    gl_FragData[0] = \n"
                "          vec4(texture2DRect( textureID0, gl_TexCoord[0].st ).rgb, 1) + \n"
                "          vec4(texture2DRect( textureID1, gl_TexCoord[0].st ).rgb, 1) + \n"
                "          vec4(texture2DRect( textureID2, gl_TexCoord[0].st ).rgb, 1) + \n"
                "     -0.5*vec4(texture2DRect( textureID3, gl_TexCoord[0].st ).rgb, 1);  \n"
                "}\n"
            };
            osg::ref_ptr<osg::Shader> fshader = new osg::Shader( osg::Shader::FRAGMENT , shaderSource);
            osg::ref_ptr<osg::Program> program = new osg::Program;
            program->addShader( fshader.get());
            stateset->setAttributeAndModes( program.get(), osg::StateAttribute::ON | osg::StateAttribute::OVERRIDE );

        }

        stateset->addUniform(new osg::Uniform("textureID0", 0));
        stateset->addUniform(new osg::Uniform("textureID1", 1));
        stateset->addUniform(new osg::Uniform("textureID2", 2));
        stateset->addUniform(new osg::Uniform("textureID3", 3));

        //stateset->setDataVariance(osg::Object::DYNAMIC);

        osg::Geode* geode = new osg::Geode();
        geode->addDrawable(polyGeom);

        parent->addChild(geode);
    }

    // now create the camera to do the multiple render to texture
    {
        osg::Camera* camera = new osg::Camera;

        // set up the background color and clear mask.
        camera->setClearColor(osg::Vec4(0.1f,0.1f,0.3f,1.0f));
        camera->setClearMask(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // the camera is going to look at our input quad
        camera->setProjectionMatrix(osg::Matrix::ortho2D(0,1,0,1));
        camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
        camera->setViewMatrix(osg::Matrix::identity());

        // set viewport
        camera->setViewport(0, 0, tex_width, tex_height);

        // set the camera to render before the main camera.
        camera->setRenderOrder(osg::Camera::PRE_RENDER);

        // tell the camera to use OpenGL frame buffer objects
        camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

        // attach the textures to use
        for (int i=0; i<NUM_TEXTURES; i++) {
            if (useMultiSample)
                camera->attach(osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER0+i), textureRect[i], 0, 0, false, 4, 4);
            else
                camera->attach(osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER0+i), textureRect[i]);


        }

#if 0
        // test for new glEnablei/glDisablei functionality.
        camera->getOrCreateStateSet()->setAttribute(new osg::Enablei(GL_BLEND, 0));
        camera->getOrCreateStateSet()->setAttribute(new osg::Disablei(GL_BLEND, 1));
#endif

        // we can also read back any of the targets as an image, modify this image and push it back
        if (useImage) {
            // which texture to get the image from
            const int tex_to_get = 0;

            osg::Image* image = new osg::Image;
            if (useHDR) {
                image->allocateImage(tex_width, tex_height, 1, GL_RGBA, GL_FLOAT);
            } else {
                image->allocateImage(tex_width, tex_height, 1, GL_RGBA, GL_UNSIGNED_BYTE);
            }

            // attach the image so its copied on each frame.
            camera->attach(osg::Camera::BufferComponent(osg::Camera::COLOR_BUFFER0 + tex_to_get), image);

            camera->setPostDrawCallback(new MyCameraPostDrawCallback(image));

            // push back the image to the texture
            textureRect[tex_to_get]->setImage(0, image);
        }

        // add the subgraph to render
        camera->addChild(cam_subgraph);

        parent->addChild(camera);
    }

    return parent;
}

int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName() + " demonstrates the use of multiple render targets (MRT) with frame buffer objects (FBOs). A render to texture (RTT) camera is used to render to four textures using a single shader. The four textures are then combined to texture the viewed geometry.");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information.");
    arguments.getApplicationUsage()->addCommandLineOption("--width","Set the width of the render to texture.");
    arguments.getApplicationUsage()->addCommandLineOption("--height","Set the height of the render to texture.");
    arguments.getApplicationUsage()->addCommandLineOption("--image","Render one of the targets to an image, then apply a post draw callback to modify it and use this image to update the final texture. Print some texture values when using HDR.");
    arguments.getApplicationUsage()->addCommandLineOption("--hdr","Use high dynamic range (HDR). Create floating point textures to render to.");

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    unsigned tex_width = 512;
    unsigned tex_height = 512;
    while (arguments.read("--width", tex_width)) {}
    while (arguments.read("--height", tex_height)) {}

    bool useHDR = false;
    while (arguments.read("--hdr")) { useHDR = true; }

    bool useImage = false;
    while (arguments.read("--image")) { useImage = true; }

    bool useMultiSample = false;
    while (arguments.read("--ms")) { useMultiSample = true; }

    osg::Group* subGraph = createRTTQuad(tex_width, tex_height, useHDR);

    osg::Group* rootNode = new osg::Group();
    rootNode->addChild(createScene(subGraph, tex_width, tex_height, useHDR, useImage, useMultiSample));

    // add model to the viewer.
    viewer.setSceneData( rootNode );

    return viewer.run();
}
