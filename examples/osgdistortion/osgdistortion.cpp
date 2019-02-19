/* OpenSceneGraph example, osgdistortion.
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
#include <osg/Stencil>
#include <osg/ColorMask>
#include <osg/Depth>
#include <osg/Billboard>
#include <osg/Material>
#include <osg/Projection>
#include <osg/TextureCubeMap>
#include <osg/io_utils>


#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>
#include <osgGA/KeySwitchMatrixManipulator>
#include <osgGA/StateSetManipulator>
#include <osgGA/AnimationPathManipulator>
#include <osgGA/TerrainManipulator>

#include <osgUtil/SmoothingVisitor>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <iostream>

using namespace osg;

struct CommandLineOptions
{
    unsigned int width;
    unsigned int height;
    unsigned int tex_width;
    unsigned int tex_height;
    double sphere_radius;
    double collar_radius;
    double distance;

    CommandLineOptions():
        width(1024),
        height(1024),
        tex_width(1024),
        tex_height(1024),
        sphere_radius(1.0),
        collar_radius(0.45),
        distance(1.0)
    {

        osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
        if (!wsi)
        {
            osg::notify(osg::NOTICE)<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
            return;
        }

        osg::GraphicsContext::ScreenIdentifier main_screen_id;

        main_screen_id.readDISPLAY();
        main_screen_id.setUndefinedScreenDetailsToDefaultScreen();
        wsi->getScreenResolution(main_screen_id, width, height);
        distance = sqrt(sphere_radius*sphere_radius - collar_radius*collar_radius);
    }

     void read(osg::ArgumentParser& arguments)
     {
        while (arguments.read("--width",width)) {}
        while (arguments.read("--height",height)) {}
        while (arguments.read("--texture-width",tex_width) || arguments.read("--tx",tex_width) ) {}
        while (arguments.read("--texture-height",tex_height) || arguments.read("--ty",tex_height) ) {}

        while (arguments.read("--radius", sphere_radius)) {}
        while (arguments.read("--collar", collar_radius)) {}

        distance = sqrt(sphere_radius*sphere_radius - collar_radius*collar_radius);
        while (arguments.read("--distance", distance)) {}
     }
};


class WindowResizedHandler : public osgGA::GUIEventHandler
{
public:
    WindowResizedHandler()
    {
    }

    virtual bool handle(const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& /*aa*/, osg::Object* object, osg::NodeVisitor* nv)
    {
        osg::Camera* camera = dynamic_cast<osg::Camera*>(object);
        if (!camera) return false;

        const osg::FrameStamp* fs = nv->getFrameStamp();

        if (ea.getEventType()==osgGA::GUIEventAdapter::RESIZE)
        {
            OSG_NOTICE<<"Window resized event context="<<ea.getGraphicsContext()<<" frameNumber = "<<fs->getFrameNumber()<<std::endl;
            OSG_NOTICE<<"   WindowX="<<ea.getWindowX()<<std::endl;
            OSG_NOTICE<<"   WindowY="<<ea.getWindowY()<<std::endl;
            OSG_NOTICE<<"   WindowWidth="<<ea.getWindowWidth()<<std::endl;
            OSG_NOTICE<<"   WindowHeight="<<ea.getWindowHeight()<<std::endl;

            // reset the Camera viewport and associated Texture's to make sure it tracks the new window size.
            camera->resize(ea.getWindowWidth(), ea.getWindowHeight());
        }
        return false;
    }

};

osg::Node* createDistortionSubgraph(CommandLineOptions& options, osg::Node* subgraph, const osg::Vec4& clearColour)
{
    osg::Group* distortionNode = new osg::Group;

    osg::Texture2D* texture = new osg::Texture2D;
    texture->setTextureSize(options.tex_width, options.tex_height);
    texture->setInternalFormat(GL_RGBA);
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE);

    // set up the render to texture camera.
    {
        osg::Camera* camera = new osg::Camera;

        // set clear the color and depth buffer
        camera->setClearColor(clearColour);
        camera->setClearMask(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

        // just inherit the main cameras view
        camera->setReferenceFrame(osg::Transform::RELATIVE_RF);
        camera->setProjectionMatrix(osg::Matrixd::identity());
        camera->setViewMatrix(osg::Matrixd::identity());

        // set viewport
        camera->setViewport(0,0,options.tex_width,options.tex_height);

        // set the camera to render before the main camera.
        camera->setRenderOrder(osg::Camera::PRE_RENDER);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture);

        // add subgraph to render
        camera->addChild(subgraph);

        distortionNode->addChild(camera);

        camera->addEventCallback(new WindowResizedHandler());
   }

    // set up the hud camera
    {
        // create the quad to visualize.
        osg::Geometry* polyGeom = new osg::Geometry();

        polyGeom->setSupportsDisplayList(false);

        osg::Vec3 origin(0.0f,0.0f,0.0f);
        osg::Vec3 xAxis(1.0f,0.0f,0.0f);
        osg::Vec3 yAxis(0.0f,1.0f,0.0f);
        float height = 1024.0f;
        float width = 1280.0f;
        int noSteps = 50;

        osg::Vec3Array* vertices = new osg::Vec3Array;
        osg::Vec2Array* texcoords = new osg::Vec2Array;
        osg::Vec4Array* colors = new osg::Vec4Array;

        osg::Vec3 bottom = origin;
        osg::Vec3 dx = xAxis*(width/((float)(noSteps-1)));
        osg::Vec3 dy = yAxis*(height/((float)(noSteps-1)));

        osg::Vec2 bottom_texcoord(0.0f,0.0f);
        osg::Vec2 dx_texcoord(1.0f/(float)(noSteps-1),0.0f);
        osg::Vec2 dy_texcoord(0.0f,1.0f/(float)(noSteps-1));

        int i,j;
        for(i=0;i<noSteps;++i)
        {
            osg::Vec3 cursor = bottom+dy*(float)i;
            osg::Vec2 texcoord = bottom_texcoord+dy_texcoord*(float)i;
            for(j=0;j<noSteps;++j)
            {
                vertices->push_back(cursor);
                texcoords->push_back(osg::Vec2((sin(texcoord.x()*osg::PI-osg::PI*0.5)+1.0f)*0.5f,(sin(texcoord.y()*osg::PI-osg::PI*0.5)+1.0f)*0.5f));
                colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

                cursor += dx;
                texcoord += dx_texcoord;
            }
        }

        // pass the created vertex array to the points geometry object.
        polyGeom->setVertexArray(vertices);

        polyGeom->setColorArray(colors, osg::Array::BIND_PER_VERTEX);

        polyGeom->setTexCoordArray(0,texcoords);


        for(i=0;i<noSteps-1;++i)
        {
            osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(osg::PrimitiveSet::QUAD_STRIP);
            for(j=0;j<noSteps;++j)
            {
                elements->push_back(j+(i+1)*noSteps);
                elements->push_back(j+(i)*noSteps);
            }
            polyGeom->addPrimitiveSet(elements);
        }


        // new we need to add the texture to the Drawable, we do so by creating a
        // StateSet to contain the Texture StateAttribute.
        osg::StateSet* stateset = polyGeom->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON);
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

        osg::Geode* geode = new osg::Geode();
        geode->addDrawable(polyGeom);

        // set up the camera to render the textured quad
        osg::Camera* camera = new osg::Camera;

        // just inherit the main cameras view
        camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
        camera->setViewMatrix(osg::Matrix::identity());
        camera->setProjectionMatrixAsOrtho2D(0,1280,0,1024);

        // set the camera to render before the main camera.
        camera->setRenderOrder(osg::Camera::NESTED_RENDER);

        // add subgraph to render
        camera->addChild(geode);

        distortionNode->addChild(camera);
    }
    return distortionNode;
}

void setDomeFaces(osgViewer::Viewer& viewer, CommandLineOptions& options)
{
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = 0;
    traits->y = 0;
    traits->width = options.width;
    traits->height = options.height;
    traits->windowDecoration = true;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    traits->readDISPLAY();
    traits->setUndefinedScreenDetailsToDefaultScreen();

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    if (!gc)
    {
        osg::notify(osg::NOTICE)<<"GraphicsWindow has not been created successfully."<<std::endl;
        return;
    }


    int center_x = options.width/2;
    int center_y = options.height/2;
    int camera_width = 256;
    int camera_height = 256;

    // front face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(center_x-camera_width/2, center_y, camera_width, camera_height));

        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);

        viewer.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd());
    }

    // top face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(center_x-camera_width/2, center_y+camera_height, camera_width, camera_height));
        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);

        viewer.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate(osg::inDegrees(-90.0f), 1.0,0.0,0.0));
    }

    // left face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(center_x-camera_width*3/2, center_y, camera_width, camera_height));
        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);

        viewer.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate(osg::inDegrees(-90.0f), 0.0,1.0,0.0));
    }

    // right face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(center_x+camera_width/2, center_y, camera_width, camera_height));
        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);

        viewer.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate(osg::inDegrees(90.0f), 0.0,1.0,0.0));
    }

    // bottom face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(center_x-camera_width/2, center_y-camera_height, camera_width, camera_height));
        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);

        viewer.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate(osg::inDegrees(90.0f), 1.0,0.0,0.0));
    }

    // back face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(center_x-camera_width/2, center_y-2*camera_height, camera_width, camera_height));
        GLenum buffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);

        viewer.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate(osg::inDegrees(-180.0f), 1.0,0.0,0.0));
    }

    viewer.getCamera()->setProjectionMatrixAsPerspective(90.0f, 1.0, 1, 1000.0);

    viewer.assignSceneDataToCameras();
}

osg::Geometry* createDomeDistortionMesh(const osg::Vec3& origin, const osg::Vec3& widthVector, const osg::Vec3& heightVector,
                                        CommandLineOptions& options)
{
    osg::Vec3d center(0.0,0.0,0.0);
    osg::Vec3d eye(0.0,0.0,0.0);

    bool centerProjection = false;

    osg::Vec3d projector = eye - osg::Vec3d(0.0,0.0, options.distance);


    osg::notify(osg::NOTICE)<<"Projector position = "<<projector<<std::endl;
    osg::notify(osg::NOTICE)<<"distance = "<<options.distance<<std::endl;


    // create the quad to visualize.
    osg::Geometry* geometry = new osg::Geometry();

    geometry->setSupportsDisplayList(false);

    osg::Vec3 xAxis(widthVector);
    float width = widthVector.length();
    xAxis /= width;

    osg::Vec3 yAxis(heightVector);
    float height = heightVector.length();
    yAxis /= height;

    int noSteps = 50;

    osg::Vec3Array* vertices = new osg::Vec3Array;
    osg::Vec3Array* texcoords = new osg::Vec3Array;
    osg::Vec4Array* colors = new osg::Vec4Array;

    osg::Vec3 bottom = origin;
    osg::Vec3 dx = xAxis*(width/((float)(noSteps-1)));
    osg::Vec3 dy = yAxis*(height/((float)(noSteps-1)));

    osg::Vec3d screenCenter = origin + widthVector*0.5f + heightVector*0.5f;
    float screenRadius = heightVector.length() * 0.5f;

    int i,j;
    if (centerProjection)
    {
        for(i=0;i<noSteps;++i)
        {
            osg::Vec3 cursor = bottom+dy*(float)i;
            for(j=0;j<noSteps;++j)
            {
                osg::Vec2 delta(cursor.x() - screenCenter.x(), cursor.y() - screenCenter.y());
                double theta = atan2(-delta.y(), delta.x());
                double phi = osg::PI_2 * delta.length() / screenRadius;
                if (phi > osg::PI_2) phi = osg::PI_2;

                phi *= 2.0;

                // osg::notify(osg::NOTICE)<<"theta = "<<theta<< "phi="<<phi<<std::endl;

                osg::Vec3 texcoord(sin(phi) * cos(theta),
                                   sin(phi) * sin(theta),
                                   cos(phi));

                vertices->push_back(cursor);
                colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
                texcoords->push_back(texcoord);

                cursor += dx;
            }
            // osg::notify(osg::NOTICE)<<std::endl;
        }
    }
    else
    {
        for(i=0;i<noSteps;++i)
        {
            osg::Vec3 cursor = bottom+dy*(float)i;
            for(j=0;j<noSteps;++j)
            {
                osg::Vec2 delta(cursor.x() - screenCenter.x(), cursor.y() - screenCenter.y());
                double theta = atan2(-delta.y(), delta.x());
                double phi = osg::PI_2 * delta.length() / screenRadius;
                if (phi > osg::PI_2) phi = osg::PI_2;

                // osg::notify(osg::NOTICE)<<"theta = "<<theta<< "phi="<<phi<<std::endl;

                double f = options.distance * sin(phi);
                double e = options.distance * cos(phi) + sqrt( options.sphere_radius*options.sphere_radius - f*f);
                double l = e * cos(phi);
                double h = e * sin(phi);
                double z = l - options.distance;

                osg::Vec3 texcoord(h * cos(theta) / options.sphere_radius,
                                   h * sin(theta) / options.sphere_radius,
                                   z / options.sphere_radius);

                vertices->push_back(cursor);
                colors->push_back(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
                texcoords->push_back(texcoord);

                cursor += dx;
            }
            // osg::notify(osg::NOTICE)<<std::endl;
        }
    }

    // pass the created vertex array to the points geometry object.
    geometry->setVertexArray(vertices);

    geometry->setColorArray(colors, osg::Array::BIND_PER_VERTEX);

    geometry->setTexCoordArray(0,texcoords);

    for(i=0;i<noSteps-1;++i)
    {
        osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(osg::PrimitiveSet::QUAD_STRIP);
        for(j=0;j<noSteps;++j)
        {
            elements->push_back(j+(i+1)*noSteps);
            elements->push_back(j+(i)*noSteps);
        }
        geometry->addPrimitiveSet(elements);
    }

    return geometry;
}

void setDomeCorrection(osgViewer::Viewer& viewer, CommandLineOptions& options)
{
    osg::ref_ptr<osg::GraphicsContext::Traits> traits = new osg::GraphicsContext::Traits;
    traits->x = 0;
    traits->y = 0;
    traits->width = options.width;
    traits->height = options.height;
    traits->windowDecoration = false;
    traits->doubleBuffer = true;
    traits->sharedContext = 0;
    traits->readDISPLAY();
    traits->setUndefinedScreenDetailsToDefaultScreen();

    osg::ref_ptr<osg::GraphicsContext> gc = osg::GraphicsContext::createGraphicsContext(traits.get());
    if (!gc)
    {
        osg::notify(osg::NOTICE)<<"GraphicsWindow has not been created successfully."<<std::endl;
        return;
    }

    int camera_width = options.tex_width;
    int camera_height = options.tex_height;

    osg::TextureCubeMap* texture = new osg::TextureCubeMap;

    texture->setTextureSize(options.tex_width, options.tex_height);
    texture->setInternalFormat(GL_RGB);
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
        camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, osg::TextureCubeMap::POSITIVE_Y);

        viewer.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd());
    }


    // top face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName("Top face camera");
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(0,0,camera_width, camera_height));
        GLenum cbuffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(cbuffer);
        camera->setReadBuffer(cbuffer);
        camera->setAllowEventFocus(false);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderTargetImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, osg::TextureCubeMap::POSITIVE_Z);

        viewer.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate(osg::inDegrees(-90.0f), 1.0,0.0,0.0));
    }

    // left face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName("Left face camera");
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(0,0,camera_width, camera_height));
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setAllowEventFocus(false);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderTargetImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, osg::TextureCubeMap::NEGATIVE_X);

        viewer.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate(osg::inDegrees(-90.0f), 0.0,1.0,0.0) * osg::Matrixd::rotate(osg::inDegrees(-90.0f), 0.0,0.0,1.0));
    }

    // right face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName("Right face camera");
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(0,0,camera_width, camera_height));
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setAllowEventFocus(false);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderTargetImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, osg::TextureCubeMap::POSITIVE_X);

        viewer.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate(osg::inDegrees(90.0f), 0.0,1.0,0.0 ) * osg::Matrixd::rotate(osg::inDegrees(90.0f), 0.0,0.0,1.0));
    }

    // bottom face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(gc.get());
        camera->setName("Bottom face camera");
        camera->setViewport(new osg::Viewport(0,0,camera_width, camera_height));
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setAllowEventFocus(false);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderTargetImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, osg::TextureCubeMap::NEGATIVE_Z);

        viewer.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate(osg::inDegrees(90.0f), 1.0,0.0,0.0) * osg::Matrixd::rotate(osg::inDegrees(180.0f), 0.0,0.0,1.0));
    }

    // back face
    {
        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName("Back face camera");
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(0,0,camera_width, camera_height));
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setAllowEventFocus(false);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderTargetImplementation);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture, 0, osg::TextureCubeMap::NEGATIVE_Y);

        viewer.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd::rotate(osg::inDegrees(180.0f), 1.0,0.0,0.0));
    }

    viewer.getCamera()->setProjectionMatrixAsPerspective(90.0f, 1.0, 1, 1000.0);



    // distortion correction set up.
    {
        osg::Geode* geode = new osg::Geode();
        geode->addDrawable(createDomeDistortionMesh(osg::Vec3(0.0f,0.0f,0.0f), osg::Vec3(options.width,0.0f,0.0f), osg::Vec3(0.0f,options.height,0.0f), options));

        // new we need to add the texture to the mesh, we do so by creating a
        // StateSet to contain the Texture StateAttribute.
        osg::StateSet* stateset = geode->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0, texture,osg::StateAttribute::ON);
        stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setGraphicsContext(gc.get());
        camera->setClearMask(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT );
        camera->setClearColor( osg::Vec4(0.1,0.1,1.0,1.0) );
        camera->setViewport(new osg::Viewport(0, 0, options.width, options.height));
        GLenum cbuffer = traits->doubleBuffer ? GL_BACK : GL_FRONT;
        camera->setDrawBuffer(cbuffer);
        camera->setReadBuffer(cbuffer);
        camera->setReferenceFrame(osg::Camera::ABSOLUTE_RF);
        camera->setAllowEventFocus(false);
        //camera->setInheritanceMask(camera->getInheritanceMask() & ~osg::CullSettings::CLEAR_COLOR & ~osg::CullSettings::COMPUTE_NEAR_FAR_MODE);
        //camera->setComputeNearFarMode(osg::CullSettings::DO_NOT_COMPUTE_NEAR_FAR);

        camera->setProjectionMatrixAsOrtho2D(0,options.width,0,options.height);
        camera->setViewMatrix(osg::Matrix::identity());

        // add subgraph to render
        camera->addChild(geode);

        camera->setName("DistortionCorrectionCamera");

        viewer.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd(), false);
    }

    viewer.getCamera()->setNearFarRatio(0.0001f);
}


int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    CommandLineOptions options;
    options.read(arguments);

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);

    // load the nodes from the commandline arguments.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFiles(arguments);

    // if not loaded assume no arguments passed in, try use default mode instead.
    if (!loadedModel) loadedModel = osgDB::readRefNodeFile("cow.osgt");

    if (!loadedModel)
    {
        std::cout << arguments.getApplicationName() <<": No data loaded" << std::endl;
        return 1;
    }


    if (arguments.read("--dome") || arguments.read("--puffer") )
    {

        setDomeCorrection(viewer, options);

        viewer.setSceneData( loadedModel );
    }
    else if (arguments.read("--faces"))
    {

        setDomeFaces(viewer, options);

        viewer.setSceneData( loadedModel );
    }
    else
    {
        osg::Node* distortionNode = createDistortionSubgraph( options, loadedModel.get(), viewer.getCamera()->getClearColor());
        viewer.setSceneData( distortionNode );
    }

    while (arguments.read("--sky-light"))
    {
        viewer.setLightingMode(osg::View::SKY_LIGHT);
    }

    if (viewer.getLightingMode()==osg::View::HEADLIGHT)
    {
        viewer.getLight()->setPosition(osg::Vec4(0.0f,0.0f,0.0f,1.0f));
    }


    // load the nodes from the commandline arguments.
    if (!viewer.getSceneData())
    {
        osg::notify(osg::NOTICE)<<"Please specify a model filename on the command line."<<std::endl;
        return 1;
    }


    // set up the camera manipulators.
    {
        osg::ref_ptr<osgGA::KeySwitchMatrixManipulator> keyswitchManipulator = new osgGA::KeySwitchMatrixManipulator;

        keyswitchManipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
        keyswitchManipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
        keyswitchManipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );
        keyswitchManipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );

        std::string pathfile;
        char keyForAnimationPath = '5';
        while (arguments.read("-p",pathfile))
        {
            osgGA::AnimationPathManipulator* apm = new osgGA::AnimationPathManipulator(pathfile);
            if (apm || !apm->valid())
            {
                unsigned int num = keyswitchManipulator->getNumMatrixManipulators();
                keyswitchManipulator->addMatrixManipulator( keyForAnimationPath, "Path", apm );
                keyswitchManipulator->selectMatrixManipulator(num);
                ++keyForAnimationPath;
            }
        }

        viewer.setCameraManipulator( keyswitchManipulator.get() );
    }

    viewer.setThreadingModel(osgViewer::Viewer::SingleThreaded);

    // add window resize handler
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);

    // add the state manipulator
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    // add the stats handler
    viewer.addEventHandler(new osgViewer::StatsHandler);

    return viewer.run();
}

