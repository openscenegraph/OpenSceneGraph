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

#include <osgGA/TrackballManipulator>
#include <osgGA/FlightManipulator>
#include <osgGA/DriveManipulator>

#include <osgUtil/SmoothingVisitor>

#include <osgDB/Registry>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>

using namespace osg;

osg::Node* createDistortionSubgraph(osg::Node* subgraph, const osg::Vec4& clearColour)
{
    osg::Group* distortionNode = new osg::Group;
    
    unsigned int tex_width = 1024;
    unsigned int tex_height = 1024;
    
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setTextureSize(tex_width, tex_height);
    texture->setInternalFormat(GL_RGBA);
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
   
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
        camera->setViewport(0,0,tex_width,tex_height);

        // set the camera to render before the main camera.
        camera->setRenderOrder(osg::Camera::PRE_RENDER);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture);

        // add subgraph to render
        camera->addChild(subgraph);
        
        distortionNode->addChild(camera);
   }
    
    // set up the render to texture camera.
    {

        // create the quad to visualize.
        osg::Geometry* polyGeom = new osg::Geometry();

        polyGeom->setSupportsDisplayList(false);

        osg::Vec3 origin(0.0f,0.0f,0.0f);
        osg::Vec3 xAxis(1.0f,0.0f,0.0f);
        osg::Vec3 yAxis(0.0f,1.0f,0.0f);
        osg::Vec3 zAxis(0.0f,0.0f,1.0f);
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

        osg::Vec3 cursor = bottom;
        osg::Vec2 texcoord = bottom_texcoord;
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

        polyGeom->setColorArray(colors);
        polyGeom->setColorBinding(osg::Geometry::BIND_PER_VERTEX);

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

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(osg::Camera::FRAME_BUFFER_OBJECT);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::Camera::COLOR_BUFFER, texture);

        // add subgraph to render
        camera->addChild(geode);
        
        distortionNode->addChild(camera);
    }
    return distortionNode;
}


int main(int argc, char** argv)
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer;
    
    // load the nodes from the commandline arguments.
    osg::Node* loadedModel = osgDB::readNodeFiles(arguments);
    if (!loadedModel)
    {
        osg::notify(osg::NOTICE)<<"Please specify a model filename on the command line."<<std::endl;
        return 1;
    }
    
    osg::Node* distortionNode = createDistortionSubgraph(loadedModel, viewer.getCamera()->getClearColor());
    
    // add model to the viewer.
    viewer.setSceneData( distortionNode );

    return viewer.run();
}
;
