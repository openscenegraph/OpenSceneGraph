#include "Leia.h"

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
#include <osg/DisplaySettings>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

using namespace osgViewer;

#if 0
// https://docs.leialoft.com/developer/android-sdk/rendering-for-leia

// https://docs.leialoft.com/developer/android-sdk/rendering-for-leia/leiainitializecameradata
void leiaInitializeCameraData(LeiaCameraData * data,
                              int num_horizontal_views,
                              int num_vertical_views,
                              float system_disparity_in_pixels,
                              float baseline_scaling,
                              float convergence_distance,
                              float vertical_field_of_view_degrees,
                              float near, float far,
                              int view_resolution_x_pixels,
                              int view_resolution_y_pixels);

// https://docs.leialoft.com/developer/android-sdk/rendering-for-leia/leiacalculateviews
int leiaCalculateViews(LeiaCameraData * data,
                       LeiaCameraView * out_views,
                       int len_views_in_x,
                       int len_views_in_y);

// https://docs.leialoft.com/developer/android-sdk/rendering-for-leia/leiainitializecameradatafrommatrix
void leiaInitializeCameraDataFromMatrix(float* matrix_4_4,
                                        LeiaCameraData* data,
                                        int num_horizontal_views,
                                        int num_vertical_views,
                                        float system_disparity_in_pixels,
                                        float baseline_scaling,
                                        float convergence_distance,
                                        int view_resolution_x_pixels,
                                        int view_resolution_y_pixels);

// https://docs.leialoft.com/developer/android-sdk/rendering-for-leia/updating-your-matrices
void leiaSetNumberOfViews(LeiaCameraData* data,
                          int num_horizontal_views,
                          int num_vertical_views);
void leiaSetSystemDisparityInPixels(LeiaCameraData* data,
                                    float disparity_in_pixels);
void leiaSetApplicationBaselineScale(LeiaCameraData* data,
                                     float baseline_scaling);
void leiaSetFieldOfView(LeiaCameraData* data,
                        float fov_in_degrees);
void leiaSetFrustumPlanes(LeiaCameraData* data,
                          float near,
                          float focal_distance,
                          float far);
void leiaSetViewSizeInPixels(LeiaCameraData* data,
                             int resolution_x,
                             int resolution_y);

// shaders
// https://docs.leialoft.com/developer/android-sdk/rendering-for-leia/shaders

// updating the pipeline
// https://docs.leialoft.com/developer/android-sdk/updating-the-gl-pipeline


// Example Shaders in SDK
~/3rdParty/Lumia/LeiaLoft_NativeAndroid_SDK_2018-07-19/Samples/TeapotsWithLeia/classic-teapot/src/main/assets/Shaders


#endif

struct LeiaIntialFrustumCallback : public osg::CullSettings::InitialFrustumCallback
{
    std::vector<osg::Matrixd> projectionMatrices;

    bool applyBB = true;
    osg::BoundingBoxd bb;

    void toggle()
    {
        applyBB = !applyBB;
    }

    void computeClipSpaceBound(osg::Camera& camera)
    {
        osg::Matrixd pmv = camera.getProjectionMatrix() * camera.getViewMatrix();

        size_t numOffsets = projectionMatrices.size();

        std::vector<osg::Vec3d> world_vertices;
        world_vertices.reserve(numOffsets*8);

        for(size_t i=0; i<numOffsets; ++i)
        {
            osg::Matrixd proj = projectionMatrices[i];
            osg::Matrixd view = camera.getViewMatrix();

            osg::Matrix clipToWorld;
            clipToWorld.invert(proj * view);

            world_vertices.push_back(osg::Vec3d(-1.0, -1.0, -1.0) * clipToWorld);
            world_vertices.push_back(osg::Vec3d(1.0, -1.0, -1.0) * clipToWorld);
            world_vertices.push_back(osg::Vec3d(1.0, 1.0, -1.0) * clipToWorld);
            world_vertices.push_back(osg::Vec3d(-1.0, 1.0, -1.0) * clipToWorld);

            world_vertices.push_back(osg::Vec3d(-1.0, -1.0, 1.0) * clipToWorld);
            world_vertices.push_back(osg::Vec3d(1.0, -1.0, 1.0) * clipToWorld);
            world_vertices.push_back(osg::Vec3d(1.0, 1.0, 1.0) * clipToWorld);
            world_vertices.push_back(osg::Vec3d(-1.0, 1.0, 1.0) * clipToWorld);

            // project local clip space into world coords
            // project world coords back into master clipspace
        }

        osg::Matrix worldToclip = camera.getProjectionMatrix() * camera.getViewMatrix();

        for(auto& v : world_vertices)
        {
            bb.expandBy(v * worldToclip);
        }
    }

    virtual void setInitialFrustum(osg::CullStack& cullStack, osg::Polytope& frustum) const
    {
        osg::CullSettings::CullingMode cullingMode = cullStack.getCullingMode();
        if (applyBB)
        {
            frustum.setToBoundingBox(bb, ((cullingMode&osg::CullSettings::NEAR_PLANE_CULLING)!=0),((cullingMode&osg::CullSettings::FAR_PLANE_CULLING)!=0));
        }
        else
        {
            frustum.setToUnitFrustum(((cullingMode&osg::CullSettings::NEAR_PLANE_CULLING)!=0),((cullingMode&osg::CullSettings::FAR_PLANE_CULLING)!=0));
        }
    }
};

class LeiaToggleFrustumHandler : public osgGA::GUIEventHandler
{
    public:

        LeiaToggleFrustumHandler(LeiaIntialFrustumCallback* callback) :
            cifc(callback) {}

        osg::ref_ptr<LeiaIntialFrustumCallback> cifc;

        bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
        {
            if (ea.getHandled()) return false;

            switch(ea.getEventType())
            {
                case(osgGA::GUIEventAdapter::KEYUP):
                {
                    if (ea.getKey()=='c')
                    {
                        cifc->toggle();
                        return true;
                    }
                    break;
                }

                default:
                    return false;
            }
            return false;
        }
};


osg::ref_ptr<osg::Node> Leia::createLeiaMesh(const osg::Vec3& origin, const osg::Vec3& widthVector, const osg::Vec3& heightVector) const
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

    for(uint32_t row = 0; row<4; ++row)
    {
        for(uint32_t column = 0; column<4; ++column)
        {
            uint16_t base = static_cast<uint16_t>(vertices->size());

            vertices->push_back(origin + widthVector*static_cast<float>(column)*0.25f + heightVector*static_cast<float>(row)*0.25f);
            texcoords->push_back(osg::Vec3(0.0f, 0.0f, static_cast<float>(column)));

            vertices->push_back(origin + widthVector*static_cast<float>(column+1)*0.25f + heightVector*static_cast<float>(row)*0.25f);
            texcoords->push_back(osg::Vec3(1.0f, 0.0f, static_cast<float>(column)));

            vertices->push_back(origin + widthVector*static_cast<float>(column+1)*0.25f + heightVector*static_cast<float>(row+1)*0.25f);
            texcoords->push_back(osg::Vec3(1.0f, 1.0f, static_cast<float>(column)));

            vertices->push_back(origin + widthVector*static_cast<float>(column)*0.25f + heightVector*static_cast<float>(row+1)*0.25f);
            texcoords->push_back(osg::Vec3(0.0f, 1.0f, static_cast<float>(column)));

            elements->push_back(base + 0);
            elements->push_back(base + 1);
            elements->push_back(base + 2);
            elements->push_back(base + 2);
            elements->push_back(base + 3);
            elements->push_back(base + 0);
        }
    }

    geometry->setVertexArray(vertices);
    geometry->setColorArray(colors, osg::Array::BIND_OVERALL);
    geometry->setTexCoordArray(0, texcoords);
    geometry->addPrimitiveSet(elements);

    return geometry;
}

osg::ref_ptr<osg::Texture2DArray> Leia::createTexture2DArray(unsigned int width, unsigned int height, unsigned int depth, GLenum format) const
{
    osg::ref_ptr<osg::Texture2DArray> texture = new osg::Texture2DArray;
    texture->setTextureSize(width, height, depth);
    texture->setInternalFormat(format);
    texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
    texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
    texture->setWrap(osg::Texture::WRAP_S,osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_T,osg::Texture::CLAMP_TO_EDGE);
    texture->setWrap(osg::Texture::WRAP_R,osg::Texture::CLAMP_TO_EDGE);
    return texture;
}

void Leia::configure(osgViewer::View& view) const
{
    osg::GraphicsContext::WindowingSystemInterface* wsi = osg::GraphicsContext::getWindowingSystemInterface();
    if (!wsi)
    {
        OSG_NOTICE<<"Error, no WindowSystemInterface available, cannot create windows."<<std::endl;
        return;
    }

    // osg::DisplaySettings* displaySettings = getActiveDisplaySetting(view);

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

    osg::ref_ptr<osg::Texture2DArray> color_texture = createTexture2DArray(tex_width, tex_height, 4, GL_RGBA);
    osg::ref_ptr<osg::Texture2DArray> depth_texture = createTexture2DArray(tex_width, tex_height, 4, GL_RGBA);

    osg::Camera::RenderTargetImplementation renderTargetImplementation = osg::Camera::FRAME_BUFFER_OBJECT;
    GLenum buffer = GL_FRONT;

    view.getCamera()->setProjectionMatrixAsPerspective(90.0f, 1.0, 1, 1000.0);

    // left/right eye multiviewOVR camera
    {
        // GL_OVR_multiview2 extensions requires modern versions of GLSL without fixed function fallback
        gc->getState()->setUseModelViewAndProjectionUniforms(true);
        gc->getState()->setUseVertexAttributeAliasing(true);
        //osg::DisplaySettings::instance()->setShaderHint(osg::DisplaySettings::SHADER_GL3);

        osg::ref_ptr<osg::Camera> camera = new osg::Camera;
        camera->setName("multview eye camera");
        camera->setGraphicsContext(gc.get());
        camera->setViewport(new osg::Viewport(0,0,camera_width, camera_height));
        camera->setDrawBuffer(buffer);
        camera->setReadBuffer(buffer);
        camera->setAllowEventFocus(false);

        osg::ref_ptr<LeiaIntialFrustumCallback> ifc = new LeiaIntialFrustumCallback;


        view.addEventHandler(new LeiaToggleFrustumHandler(ifc.get()));

        // assign custom frustum callback
        camera->setInitialFrustumCallback(ifc.get());


        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(renderTargetImplementation);

        // attach the texture and use it as the color buffer, specify that the face is controlled by the multiview extension
        camera->attach(osg::Camera::COLOR_BUFFER, color_texture, 0, osg::Camera::FACE_CONTROLLED_BY_MULTIVIEW_SHADER);
        camera->attach(osg::Camera::DEPTH_BUFFER, depth_texture, 0, osg::Camera::FACE_CONTROLLED_BY_MULTIVIEW_SHADER);


        view.addSlave(camera.get(), osg::Matrixd(), osg::Matrixd());

        osg::StateSet* stateset = camera->getOrCreateStateSet();
        {
            // set up the projection and view matrix uniforms
            ifc->projectionMatrices.push_back(camera->getProjectionMatrix()*osg::Matrixd::translate(-0.2, 0.0, 0.0));
            ifc->projectionMatrices.push_back(camera->getProjectionMatrix()*osg::Matrixd::translate(-0.1, 0.0, 0.0));
            ifc->projectionMatrices.push_back(camera->getProjectionMatrix()*osg::Matrixd::translate(0.1, 0.0, 0.0));
            ifc->projectionMatrices.push_back(camera->getProjectionMatrix()*osg::Matrixd::translate(0.2, 0.0, 0.0));

            ifc->computeClipSpaceBound(*camera);

            osg::ref_ptr<osg::Uniform> projectionMatrices_uniform = new osg::Uniform(osg::Uniform::FLOAT_MAT4, "osg_ProjectionMatrices", ifc->projectionMatrices.size());
            stateset->addUniform(projectionMatrices_uniform);

            for(size_t i=0; i<ifc->projectionMatrices.size(); ++i)
            {
                projectionMatrices_uniform->setElement(i, ifc->projectionMatrices[i]);
            }

            // set up the shaders
            osg::ref_ptr<osg::Program> program = new osg::Program();
            stateset->setAttribute(program.get(), osg::StateAttribute::ON);

            std::string vsFileName("leia.vert");
            std::string fsFileName("leia.frag");

            osg::ref_ptr<osg::Shader> vertexShader = osgDB::readRefShaderFile( osg::Shader::VERTEX, vsFileName) ;
            if (vertexShader.get()) program->addShader( vertexShader.get() );

            osg::ref_ptr<osg::Shader> fragmentShader = osgDB::readRefShaderFile( osg::Shader::FRAGMENT, fsFileName) ;
            if (fragmentShader.get()) program->addShader( fragmentShader.get() );
        }

    }


    // distortion correction set up.
    {
        osg::ref_ptr<osg::Node> mesh = createLeiaMesh(osg::Vec3(0.0f,0.0f,0.0f), osg::Vec3(width,0.0f,0.0f), osg::Vec3(0.0f,height,0.0f));

        // new we need to add the texture to the mesh, we do so by creating a
        // StateSet to contain the Texture StateAttribute.
        osg::StateSet* stateset = mesh->getOrCreateStateSet();
        stateset->setTextureAttribute(0, color_texture, osg::StateAttribute::ON);
        stateset->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

        {
            osg::ref_ptr<osg::Program> program = new osg::Program();
            stateset->setAttribute(program.get(), osg::StateAttribute::ON);

            std::string vsFileName("standard.vert");
            std::string fsFileName("standard.frag");

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
