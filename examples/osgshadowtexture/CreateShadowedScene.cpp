#include <osg/Texture2D>
#include <osg/Material>
#include <osg/LightSource>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ShapeDrawable>
#include <osg/CameraNode>
#include <osg/TexGenNode>
#include <osg/Notify>
#include <osg/io_utils>

#include "CreateShadowedScene.h"

using namespace osg;

class UpdateCameraAndTexGenCallback : public osg::NodeCallback
{
    public:
    
        UpdateCameraAndTexGenCallback(const osg::Vec3& position, osg::CameraNode* cameraNode, osg::TexGenNode* texgenNode):
            _position(position),
            _cameraNode(cameraNode),
            _texgenNode(texgenNode)
        {
        }
       
        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            // first update subgraph to make sure objects are all moved into postion
            traverse(node,nv);
            
            // now compute the camera's view and projection matrix to point at the shadower (the camera's children)
            osg::BoundingSphere bs;
            for(unsigned int i=0; i<_cameraNode->getNumChildren(); ++i)
            {
                bs.expandBy(_cameraNode->getChild(i)->getBound());
            }
            
            if (!bs.valid())
            {
                osg::notify(osg::WARN) << "bb invalid"<<_cameraNode.get()<<std::endl;
                return;
            }

            float centerDistance = (_position-bs.center()).length();

            float znear = centerDistance-bs.radius();
            float zfar  = centerDistance+bs.radius();
            float zNearRatio = 0.001f;
            if (znear<zfar*zNearRatio) znear = zfar*zNearRatio;

            float top   = (bs.radius()/centerDistance)*znear;
            float right = top;

            _cameraNode->setReferenceFrame(osg::CameraNode::ABSOLUTE_RF);
            _cameraNode->setProjectionMatrixAsFrustum(-right,right,-top,top,znear,zfar);
            _cameraNode->setViewMatrixAsLookAt(_position,bs.center(),osg::Vec3(0.0f,1.0f,0.0f));

            // compute the matrix which takes a vertex from local coords into tex coords
            // will use this later to specify osg::TexGen..
            osg::Matrix MVPT = _cameraNode->getViewMatrix() * 
                               _cameraNode->getProjectionMatrix() *
                               osg::Matrix::translate(1.0,1.0,1.0) *
                               osg::Matrix::scale(0.5f,0.5f,0.5f);
                               
            _texgenNode->getTexGen()->setMode(osg::TexGen::EYE_LINEAR);
            _texgenNode->getTexGen()->setPlanesFromMatrix(MVPT);

        }
        
    protected:
    
        virtual ~UpdateCameraAndTexGenCallback() {}
        
        osg::Vec3                     _position;
        osg::ref_ptr<osg::CameraNode> _cameraNode;
        osg::ref_ptr<osg::TexGenNode> _texgenNode;

};


osg::Group* createShadowedScene(osg::Node* shadower,osg::Node* shadowed,const osg::Vec3& lightPosition,float radius,unsigned int unit)
{
    osg::Group* group = new osg::Group;
    
    // add light source
    {
        osg::LightSource* lightSource = new osg::LightSource;
        lightSource->getLight()->setPosition(osg::Vec4(lightPosition,1.0f));
        lightSource->getLight()->setLightNum(0);

        group->addChild(lightSource);

        osg::Geode* lightgeode = new osg::Geode;
        lightgeode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
        lightgeode->addDrawable(new osg::ShapeDrawable(new osg::Sphere(lightPosition,radius)));
        group->addChild(lightgeode);
    }
        
    osg::Vec4 ambientLightColor(0.2,0.2f,0.2f,1.0f);

    // add the shadower and shadowed.
    group->addChild(shadower);
    group->addChild(shadowed);

        
    unsigned int tex_width = 512;
    unsigned int tex_height = 512;
    
    osg::Texture2D* texture = new osg::Texture2D;
    texture->setTextureSize(tex_width, tex_height);
    texture->setInternalFormat(GL_RGB);
    texture->setFilter(osg::Texture2D::MIN_FILTER,osg::Texture2D::LINEAR);
    texture->setFilter(osg::Texture2D::MAG_FILTER,osg::Texture2D::LINEAR);
    texture->setWrap(osg::Texture2D::WRAP_S,osg::Texture2D::CLAMP_TO_BORDER);
    texture->setWrap(osg::Texture2D::WRAP_T,osg::Texture2D::CLAMP_TO_BORDER);
    texture->setBorderColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));
   
    // set up the render to texture camera.
    {

        // create the camera
        osg::CameraNode* camera = new osg::CameraNode;

        camera->setClearColor(osg::Vec4(1.0f,1.0f,1.0f,1.0f));

        // set viewport
        camera->setViewport(0,0,tex_width,tex_height);

        // set the camera to render before the main camera.
        camera->setRenderOrder(osg::CameraNode::PRE_RENDER);

        // tell the camera to use OpenGL frame buffer object where supported.
        camera->setRenderTargetImplementation(osg::CameraNode::FRAME_BUFFER_OBJECT);

        // attach the texture and use it as the color buffer.
        camera->attach(osg::CameraNode::COLOR_BUFFER, texture);

        // add subgraph to render
        camera->addChild(shadower);
        
        osg::StateSet* stateset = camera->getOrCreateStateSet();

        // make the material black for a shadow.
        osg::Material* material = new osg::Material;
        material->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(0.0f,0.0f,0.0f,1.0f));
        material->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(0.0f,0.0f,0.0f,1.0f));
        material->setEmission(osg::Material::FRONT_AND_BACK,ambientLightColor);
        material->setShininess(osg::Material::FRONT_AND_BACK,0.0f);
        stateset->setAttribute(material,osg::StateAttribute::OVERRIDE);

        group->addChild(camera);
        
        // create the texgen node to project the tex coords onto the subgraph
        osg::TexGenNode* texgenNode = new osg::TexGenNode;
        texgenNode->setTextureUnit(unit);
        group->addChild(texgenNode);

        // set an update callback to keep moving the camera and tex gen in the right direction.
        group->setUpdateCallback(new UpdateCameraAndTexGenCallback(lightPosition, camera, texgenNode));
    }

    // set the shadowed subgraph so that it uses the texture and tex gen settings.    
    {
        osg::StateSet* stateset = shadowed->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(unit,texture,osg::StateAttribute::ON);
        stateset->setTextureMode(unit,GL_TEXTURE_GEN_S,osg::StateAttribute::ON);
        stateset->setTextureMode(unit,GL_TEXTURE_GEN_T,osg::StateAttribute::ON);
        stateset->setTextureMode(unit,GL_TEXTURE_GEN_R,osg::StateAttribute::ON);
        stateset->setTextureMode(unit,GL_TEXTURE_GEN_Q,osg::StateAttribute::ON);
    }
    

    // set hud to render shadow texture, just for interest
    {
        osg::Geode* geode = new osg::Geode;
        osg::Geometry* geom = osg::createTexturedQuadGeometry(osg::Vec3(0,0,0),osg::Vec3(100.0,0.0,0.0),osg::Vec3(0.0,100.0,0.0));
        geom->getOrCreateStateSet()->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
        geom->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
        geode->addDrawable(geom);
        
        osg::CameraNode* camera = new osg::CameraNode;

        // set the projection matrix
        camera->setProjectionMatrix(osg::Matrix::ortho2D(0,100,0,100));

        // set the view matrix    
        camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
        camera->setViewMatrix(osg::Matrix::identity());
        
        camera->setViewport(50,50,100,100);

        // only clear the depth buffer
        camera->setClearMask(GL_DEPTH_BUFFER_BIT);

        // draw subgraph after main camera view.
        camera->setRenderOrder(osg::CameraNode::POST_RENDER);

        camera->addChild(geode);
        
        group->addChild(camera);
        
    }
    
    return group;
}
