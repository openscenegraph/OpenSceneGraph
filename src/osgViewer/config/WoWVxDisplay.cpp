/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/

#include <osgViewer/config/WoWVxDisplay>
#include <osgViewer/Renderer>
#include <osgViewer/View>
#include <osgViewer/GraphicsWindow>

#include <osg/io_utils>

#include <osg/TextureRectangle>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/TexMat>
#include <osg/Stencil>
#include <osg/PolygonStipple>
#include <osg/ValueObject>

using namespace osgViewer;

void WoWVxDisplay::configure(osgViewer::View& view) const
{
    OSG_INFO<<"WoWVxDisplay::configure(...)"<<std::endl;

    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
    {
        OSG_NOTICE<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }

    osg::GraphicsContext::ScreenIdentifier si;
    si.readDISPLAY();

    // displayNum has not been set so reset it to 0.
    if (si.displayNum<0) si.displayNum = 0;

    si.screenNum = _screenNum;

    unsigned int width, height;
    wsi->getScreenResolution(si, width, height);

    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->hostName = si.hostName;
    traits->displayNum = si.displayNum;
    traits->screenNum = si.screenNum;
    traits->x = 0;
    traits->y = 0;
    traits->width = width;
    traits->height = height;
    traits->windowDecoration = false;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;


    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    if (!gc)
    {
        OSG_NOTICE<<"GraphicsWindow has not been created successfully."<<std::endl;
        return;
    }

    int tex_width = width;
    int tex_height = height;

    int camera_width = tex_width;
    int camera_height = tex_height;

    osg::Texture2D* texture = new osg::Texture2D;
    texture->setTextureSize(tex_width, tex_height);
    texture->setInternalFormat(GL_RGB);
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

    osg::Texture2D* textureD = new osg::Texture2D;
    textureD->setTextureSize(tex_width, tex_height);
    textureD->setInternalFormat(GL_DEPTH_COMPONENT);
    textureD->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    textureD->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);

#if 0
    osg::Camera::RenderTargetImplementation renderTargetImplementation = osg::Camera::SEPERATE_WINDOW;
    GLenum buffer = GL_FRONT;
#else
    osg::Camera::RenderTargetImplementation renderTargetImplementation = osg::Camera::FRAME_BUFFER_OBJECT;
    GLenum buffer = GL_FRONT;
#endif

    // front face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName("Front face camera");
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(0,0,camera_width, camera_height));
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setAllowEventFocus(false);
        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderTargetImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture);
        camera->attach(osg::Camera::DEPTH_BUFFER, textureD);

        view.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd());
    }

    // WoW display set up.
    {
        osg::Texture1D *textureHeader = new osg::Texture1D();
        // Set up the header
        {
            unsigned char header[]= {0xF1,_wow_content,_wow_factor,_wow_offset,0x00,0x00,0x00,0x00,0x00,0x00};
            // Calc the CRC32
            {
                unsigned long _register = 0;
                for(int i = 0; i < 10; ++i) {
                    unsigned char mask = 0x80;
                    unsigned char byte = header[i];
                    for (int j = 0; j < 8; ++j)
                    {
                        bool topBit = (_register & 0x80000000) != 0;
                        _register <<= 1;
                        _register ^= ((byte & mask) != 0? 0x1: 0x0);
                        if (topBit)
                        {
                            _register ^= 0x04c11db7;
                        }
                        mask >>= 1;
                    }
                }
                unsigned char *p = (unsigned char*) &_register;
                for(size_t i = 0; i < 4; ++i)
                {
                    header[i+6] = p[3-i];
                }
            }

            osg::ref_ptr<osg::Image> imageheader = new osg::Image();
            imageheader->allocateImage(256,1,1,GL_LUMINANCE,GL_UNSIGNED_BYTE);
            {
                unsigned char *cheader = imageheader->data();
                for (int x=0; x<256; ++x){
                    cheader[x] = 0;
                }
                for (int x=0; x<=9; ++x){
                    for (int y=7; y>=0; --y){
                        int i = 2*(7-y)+16*x;
                        cheader[i] = (((1<<(y))&(header[x])) << (7-(y)));
                    }
                }
            }
            textureHeader->setImage(imageheader.get());
        }

        // Create the Screen Aligned Quad
        osg::Geode* geode = new osg::Geode();
        {
            osg::Geometry* geom = new osg::Geometry;

            osg::Vec3Array* vertices = new osg::Vec3Array;
            vertices->push_back(osg::Vec3(0,height,0));
            vertices->push_back(osg::Vec3(0,0,0));
            vertices->push_back(osg::Vec3(width,0,0));
            vertices->push_back(osg::Vec3(width,height,0));
            geom->setVertexArray(vertices);

            osg::Vec2Array* tex = new osg::Vec2Array;
            tex->push_back(osg::Vec2(0,1));
            tex->push_back(osg::Vec2(0,0));
            tex->push_back(osg::Vec2(1,0));
            tex->push_back(osg::Vec2(1,1));
            geom->setTexCoordArray(0,tex);

            geom->addPrimitiveSet(new osg::DrawArrays(GL_QUADS,0,4));
            geode->addDrawable(geom);

            // new we need to add the textures to the quad, and setting up the shader.
            osg::StateSet* stateset = geode->getOrCreateStateSet();
            stateset->setTextureAttributeAndModes(0, textureHeader,osg::StateAttribute::ON);
            stateset->setTextureAttributeAndModes(1, texture,osg::StateAttribute::ON);
            stateset->setTextureAttributeAndModes(2, textureD,osg::StateAttribute::ON);
            stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

            osg::ref_ptr<osg::Program> programShader = new osg::Program();
            stateset->setAttribute(programShader.get(), osg::StateAttribute::ON);
            stateset->addUniform( new osg::Uniform("wow_width", (int)width));
            stateset->addUniform( new osg::Uniform("wow_height", (int)height));
            stateset->addUniform( new osg::Uniform("wow_disparity_M", _wow_disparity_M));
            stateset->addUniform( new osg::Uniform("wow_disparity_Zd", _wow_disparity_Zd));
            stateset->addUniform( new osg::Uniform("wow_disparity_vz", _wow_disparity_vz));
            stateset->addUniform( new osg::Uniform("wow_disparity_C", _wow_disparity_C));

            stateset->addUniform(new osg::Uniform("wow_header", 0));
            stateset->addUniform(new osg::Uniform("wow_tcolor", 1));
            stateset->addUniform(new osg::Uniform("wow_tdepth", 2));

            osg::Shader *frag = new osg::Shader(osg::Shader::FRAGMENT);
            frag->setShaderSource(" "\
                    " uniform sampler1D wow_header;                                                                                   " \
                    " uniform sampler2D wow_tcolor;                                                                                   " \
                    " uniform sampler2D wow_tdepth;                                                                                   " \
                    "                                                                                                                 " \
                    " uniform int wow_width;                                                                                          " \
                    " uniform int wow_height;                                                                                         " \
                    " uniform float wow_disparity_M;                                                                                  " \
                    " uniform float wow_disparity_Zd;                                                                                 " \
                    " uniform float wow_disparity_vz;                                                                                 " \
                    " uniform float wow_disparity_C;                                                                                  " \
                    "                                                                                                                 " \
                    " float disparity(float Z)                                                                                        " \
                    " {                                                                                                               " \
                    "     return (wow_disparity_M*(1.0-(wow_disparity_vz/(Z-wow_disparity_Zd+wow_disparity_vz)))                        " \
                    "                   + wow_disparity_C) / 255.0;                                                                   " \
                    " }                                                                                                               " \
                    "                                                                                                                 " \
                    " void main()                                                                                                     " \
                    " {                                                                                                               " \
                    "       vec2 pos = (gl_FragCoord.xy / vec2(wow_width/2,wow_height) );                                             " \
                    "         if (gl_FragCoord.x > float(wow_width/2))                                                                  " \
                    "         {                                                                                                         " \
                    "             gl_FragColor = vec4(disparity(( texture2D(wow_tdepth, pos - vec2(1,0))).z));                          " \
                    "         }                                                                                                         " \
                    "         else{                                                                                                     " \
                    "             gl_FragColor = texture2D(wow_tcolor, pos);                                                            " \
                    "         }                                                                                                         " \
                    "     if ( (gl_FragCoord.y >= float(wow_height-1)) && (gl_FragCoord.x < 256.0) )                                    " \
                    "     {                                                                                                             " \
                    "         float pos = gl_FragCoord.x/256.0;                                                                         " \
                    "         float blue = texture1D(wow_header, pos).b;                                                                " \
                    "         if ( blue < 0.5)                                                                                          " \
                    "             gl_FragColor.b = 0.0;                                                                                 " \
                    "         else                                                                                                      " \
                    "             gl_FragColor.b = 1.0;                                                                                 " \
                    "     }                                                                                                             " \
                    " }                                                                                                               " );

            programShader->addShader(frag);
        }

        // Create the Camera
        {
            osg::ref_ptr<osg::Camera> camera = new osg::Camera;
            camera->setGraphicsContext(gc.get());
            camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
            camera->setClearColor( osg::Vec4(0.0,0.0,0.0,1.0) );
            camera->setViewport(new osg::Viewport(0, 0, width, height));
            GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
            camera->setDrawBuffer(buffer);
            camera->setReadBuffer(buffer);
            camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
            camera->setAllowEventFocus(false);
            camera->setInheritanceMask(camera->getInheritanceMask() & ~osg::CullSettings::CLEAR_COLOR & ~osg::CullSettings::COMPUTE_NEAR_FAR_MODE);
            //camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

            camera->setProjectionMatrixAsOrtho2D(0,width,0,height);
            camera->setViewMatrix(osg::Matrix::identity());

            // add subgraph to render
            camera->addChild(geode);

            camera->setName("WoWCamera");

            view.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd(), false);
        }
    }
}
