#include <osgDB/ReadFile>

#include <osg/Switch>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osg/Quat>
#include <osg/Geometry>
#include <osg/Geode>

osg::MatrixTransform* createPositionedAndScaledModel(osg::Node* model,const osg::Vec3& pos, float radius, osg::Quat rotation)
{
    osg::MatrixTransform* transform = new osg::MatrixTransform;
    
    const osg::BoundingSphere& bs = model->getBound();
    
    transform->setDataVariance(osg::Object::STATIC);
    transform->setMatrix(osg::Matrix::translate(-bs.center())*
                         osg::Matrix::scale(radius/bs.radius(),radius/bs.radius(),radius/bs.radius())*
                         osg::Matrix::rotate(rotation)*
                         osg::Matrix::translate(pos));
                         
    transform->addChild(model);
    
    return transform;
}


osg::Node* createDefaultPresentation()
{
    osg::Switch* presentation = new osg::Switch;
    presentation->setName("Presentation default");
    
    float width = 1280.0f;
    float height = 1024.0f;
    
    osg::Geometry* backgroundQuad = osg::createTexturedQuadGeometry(osg::Vec3(0.0f,0.0f,0.0f),
                                                    osg::Vec3(width,0.0f,0.0f),
                                                    osg::Vec3(0.0f,0.0f,height));
                
    osg::Geode* background = new osg::Geode;

    background->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                new osg::Texture2D(osgDB::readImageFile("lz.rgb")),
                osg::StateAttribute::ON);

    background->addDrawable(backgroundQuad);

    osg::Geode* background2 = new osg::Geode;
    
    background2->getOrCreateStateSet()->setTextureAttributeAndModes(0,
                new osg::Texture2D(osgDB::readImageFile("reflect.rgb")),
                osg::StateAttribute::ON);
                
    background2->addDrawable(backgroundQuad);

    {
        osg::Switch* slide = new osg::Switch;
        slide->setName("Slide 0");
        
        {
            osg::Group* group = new osg::Group;
            group->setName("Layer 0");
            
            group->addChild(background);
            
            
            slide->addChild(group);
        }
       
        {
            osg::Group* group = new osg::Group;
            group->setName("Layer 1");
            
            group->addChild(background2);

            slide->addChild(group);
        }
        
        {
            osg::Group* group = new osg::Group;
            group->setName("Layer 2");
            
            group->addChild(background);
            
            group->addChild(createPositionedAndScaledModel(osgDB::readNodeFile("cow.osg"),
                                                           osg::Vec3(600.0f,0.0f,600.0f),500.0f,
                                                           osg::Quat()));

            slide->addChild(group);
        }
        
        presentation->addChild(slide);        
        
    }
    
    {
        osg::Switch* slide = new osg::Switch;
        slide->setName("Slide 1");
        
        {
            osg::Group* group = new osg::Group;
            group->setName("Layer 0");
            
            group->addChild(background);
            
            group->addChild(createPositionedAndScaledModel(osgDB::readNodeFile("glider.osg"),
                                                           osg::Vec3(600.0f,0.0f,600.0f),500.0f,
                                                           osg::Quat()));

            slide->addChild(group);
        }
        
        {
            osg::Group* group = new osg::Group;
            group->setName("Layer 1");
            
            group->addChild(background2);
            
            group->addChild(createPositionedAndScaledModel(osgDB::readNodeFile("Town.osg"),
                                                           osg::Vec3(600.0f,0.0f,600.0f),500.0f,
                                                           osg::Quat()));

            slide->addChild(group);
        }

        presentation->addChild(slide);        
        
    }

    return presentation;
}
