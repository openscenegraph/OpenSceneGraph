#include <osgDB/ReadFile>

#include <osg/Switch>
#include <osg/Texture2D>
#include <osg/MatrixTransform>
#include <osg/Quat>
#include <osg/Geometry>
#include <osg/Geode>
#include <osg/PolygonOffset>

#include <osgText/Text>

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

osg::Vec4 layoutColor(1.0f,1.0f,1.0f,1.0f);
float layoutCharacterSize = 50.0f;
std::string fontStr("fonts/arial.ttf");

osg::Geode* createText(osg::Vec3& cursor,const std::string& str)
{
    osg::Geode* geode = new osg::Geode;

    osgText::Text* text = new osgText::Text;
    text->setFont(fontStr);
    text->setColor(layoutColor);
    text->setCharacterSize(layoutCharacterSize);
    text->setAxisAlignment(osgText::Text::XZ_PLANE);
    text->setPosition(cursor);
    
    text->setText(str);

    osg::BoundingBox bb = text->getBound();
    cursor.z() = bb.zMin()-layoutCharacterSize;

    geode->addDrawable(text);
    
    return geode;
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

    osg::StateSet* backgroundStateSet = background->getOrCreateStateSet();
    backgroundStateSet->setAttributeAndModes(
                new osg::PolygonOffset(1.0f,1.0f),
                osg::StateAttribute::ON);
                                        
    backgroundStateSet->setTextureAttributeAndModes(0,
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

        osg::Vec3 cursor(width*0.25f,0.0f,height*.75f);
                
        osg::Group* layer_0 = new osg::Group;
        {
        
            layer_0->setName("Layer 0");
            layer_0->addChild(background);
            layer_0->addChild(createText(cursor,"The is layer 0"));

            slide->addChild(layer_0);
        }
       
        osg::Group* layer_1 = new osg::Group;
        {
            layer_1->setName("Layer 1");
            layer_1->addChild(layer_0);
            layer_1->addChild(createText(cursor,"The is layer 1"));

            slide->addChild(layer_1);
        }
        
        osg::Group* layer_2 = new osg::Group;
        {
            layer_2->setName("Layer 2");
            layer_2->addChild(layer_1);
            layer_2->addChild(createText(cursor,"The is layer 2\nWe can have multiple lines of text..."));

            slide->addChild(layer_2);
        }

        {
            osg::Group* layer_3 = new osg::Group;
            layer_3->setName("Layer 3");
            
            layer_3->addChild(layer_2);

            layer_3->addChild(createText(cursor,"The is layer 3, with cow.osg"));
            
            layer_3->addChild(createPositionedAndScaledModel(osgDB::readNodeFile("cow.osg"),
                                                           osg::Vec3(600.0f,0.0f,300.0f),500.0f,
                                                           osg::Quat()));

            slide->addChild(layer_3);
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
