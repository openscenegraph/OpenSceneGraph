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

#include "MultiviewOVR.h"

#include <osgViewer/Renderer>
#include <osgViewer/View>
#include <osgViewer/GraphicsWindow>

#include <osg/io_utils>

#include <osg/Texture2DArray>
#include <osg/TextureRectangle>
#include <osg/Texture1D>
#include <osg/Texture2D>
#include <osg/TexMat>
#include <osg/Stencil>
#include <osg/PolygonStipple>
#include <osg/ValueObject>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

using namespace osgViewer;

osg::ref_ptr<osg::Node> MultiviewOVR::createStereoMesh(const osg::Vec3& origin, const osg::Vec3& widthVector, const osg::Vec3& heightVector) const
{
    osg::Vec3d center(0.0,0.0,0.0);
    osg::Vec3d eye(0.0,0.0,0.0);

    // create the quad to visualize.
    osg::Geometry* geometry = new osg::Geometry();

    geometry->setSupportsDisplayList(false);

    osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(osg::PrimitiveSet::TRIANGLES);
    osg::Vec3Array* vertices = new osg::Vec3Array;
    osg::Vec3Array* texcoords = new osg::Vec3Array;
    osg::Vec4Array* colors = new osg::Vec4Array;
    colors->push_back(osg::Vec4(1.0, 1.0, 0.0, 1.0));

    // left hand side
    vertices->push_back(origin); texcoords->push_back(osg::Vec3(0.0f, 0.0f, 0.0f));
    vertices->push_back(origin + widthVector*0.5f); texcoords->push_back(osg::Vec3(1.0f, 0.0f, 0.0f));
    vertices->push_back(origin + widthVector*0.5f +heightVector); texcoords->push_back(osg::Vec3(1.0f, 1.0f, 0.0f));
    vertices->push_back(origin + heightVector); texcoords->push_back(osg::Vec3(0.0f, 1.0f, 0.0f));

    elements->push_back(0);
    elements->push_back(1);
    elements->push_back(2);
    elements->push_back(2);
    elements->push_back(3);
    elements->push_back(0);

    // right hand side
    vertices->push_back(origin + widthVector*0.5f); texcoords->push_back(osg::Vec3(0.0f, 0.0f, 1.0f));
    vertices->push_back(origin + widthVector); texcoords->push_back(osg::Vec3(1.0f, 0.0f, 1.0f));
    vertices->push_back(origin + widthVector +heightVector); texcoords->push_back(osg::Vec3(1.0f, 1.0f, 1.0f));
    vertices->push_back(origin + widthVector*0.5f + heightVector); texcoords->push_back(osg::Vec3(0.0f, 1.0f, 1.0f));

    elements->push_back(4);
    elements->push_back(5);
    elements->push_back(6);
    elements->push_back(6);
    elements->push_back(7);
    elements->push_back(4);

    geometry->setVertexArray(vertices);
    geometry->setColorArray(colors, osg::Array::BIND_OVERALL);
    geometry->setTexCoordArray(0, texcoords);
    geometry->addPrimitiveSet(elements);

    return geometry;
}

void MultiviewOVR::configure(osgViewer::View& view) const
{
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
    {
        OSG_NOTICE<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }

    osg::DisplaySettings* displaySettings = getActiveDisplaySetting(view);


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

    osg::Texture2DArray* texture = new osg::Texture2DArray;

    texture->setTextureSize(tex_width, tex_height, 2);
    texture->setInternalFormat(GL_RGBA);
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_R,osg::Texture::CLAMP_TO_EDGE);

#if 0
    osg::Camera::RenderTargetImplementation renderTargetImplementation = osg::Camera::SEPERATE_WINDOW;
    GLenum buffer = GL_FRONT;
#else
    osg::Camera::RenderTargetImplementation renderTargetImplementation = osg::Camera::FRAME_BUFFER_OBJECT;
    GLenum buffer = GL_FRONT;
#endif

    // left eye
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName("Left eye camera");
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(0,0,camera_width, camera_height));
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setAllowEventFocus(false);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderTargetImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, 0);

        // set up the projection and view matrices
        osg::Matrixd projectionOffset = displaySettings->computeLeftEyeProjectionImplementation(osg::Matrixd());
        osg::Matrixd viewOffset = displaySettings->computeLeftEyeViewImplementation(osg::Matrixd());

        view.addSlave(camera.get(), projectionOffset, viewOffset);
    }

    // right eye
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName("Right eyecamera");
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(0,0,camera_width, camera_height));
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setAllowEventFocus(false);
        camera->setClearColor(osg::Vec4(0.2f, 0.2f, 0.5f, 1.0f));

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderTargetImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, 1);

        // set up the projection and view matrices
        osg::Matrixd projectionOffset = displaySettings->computeRightEyeProjectionImplementation(osg::Matrixd());
        osg::Matrixd viewOffset = displaySettings->computeRightEyeViewImplementation(osg::Matrixd());

        view.addSlave(camera.get(), projectionOffset, viewOffset);
    }

    view.getCamera()->setProjectionMatrixAsPerspective(90.0f, 1.0, 1, 1000.0);

    // distortion correction set up.
    {
        osg::ref_ptr<osg::Node> mesh = createStereoMesh(osg::Vec3(0.0f,0.0f,0.0f), osg::Vec3(width,0.0f,0.0f), osg::Vec3(0.0f,height,0.0f));

        // new we need to add the texture to the mesh, we do so by creating a
        // StateSet to contain the Texture StateAttribute.
        osg::StateSet* stateset = mesh->getOrCreateStateSet();
        stateset->setTextureAttribute(0, texture, osg::StateAttribute::ON);
        stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

        {
            osg::ref_ptr<osg::Program> program = new osg::Program();
            stateset->setAttribute(program.get(), osg::StateAttribute::ON);

            std::string vsFileName("multiviewOVR.vert");
            std::string fsFileName("multiviewOVR.frag");

            osg::ref_ptr<osg::Shader> vertexShader = osgDB::readRefShaderFile( osg::Shader::VERTEX, vsFileName) ;
            if (vertexShader.get()) program->addShader( vertexShader.get() );

            osg::ref_ptr<osg::Shader> fragmentShader = osgDB::readRefShaderFile( osg::Shader::FRAGMENT, fsFileName) ;
            if (fragmentShader.get()) program->addShader( fragmentShader.get() );
        }

        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(gc.get());
        camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
        camera->setClearColor( osg::Vec4(0.0,0.0,0.0,1.0) );
        camera->setViewport(new osg::Viewport(0, 0, width, height));

        GLenum window_buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(window_buffer);
        camera->setReadBuffer(window_buffer);
        camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        camera->setAllowEventFocus(true);
        camera->setInheritanceMask(camera->getInheritanceMask() & ~osg::CullSettings::CLEAR_COLOR & ~osg::CullSettings::COMPUTE_NEAR_FAR_MODE);
        //camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

        camera->setProjectionMatrixAsOrtho2D(0,width,0,height);
        camera->setViewMatrix(osg::Matrix::identity());

        // add subgraph to render
        camera->addChild(mesh.get());

        camera->setName("DistortionCorrectionCamera");

        osgDB::writeNodeFile(*mesh, "mesh.osgt");

        view.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd(), false);
    }

    view.getCamera()->setNearFarRatio(0.0001f);

    if (view.getLightingMode()==osg::View::HEADLIGHT)
    {
        // set a local light source for headlight to ensure that lighting is consistent across sides of cube.
        view.getLight()->setPosition(osg::Vec4(0.0f,0.0f,0.0f,1.0f));
    }
}
