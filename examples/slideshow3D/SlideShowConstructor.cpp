#include "SlideShowConstructor.h"

#include <osg/Geometry>
#include <osg/PolygonOffset>
#include <osg/Geode>
#include <osg/Texture2D>
#include <osg/MatrixTransform>

#include <osgUtil/TransformCallback>

#include <osgDB/ReadFile>
#include <osgDB/FileUtils>

#include <osgText/Text>


SlideShowConstructor::SlideShowConstructor()
{
    _slideDistance = osg::DisplaySettings::instance()->getScreenDistance();
    _slideHeight = osg::DisplaySettings::instance()->getScreenHeight();
    _slideWidth = _slideHeight*1280.0f/1024.f;

    _slideOrigin.set(-_slideWidth*0.5f,_slideDistance,-_slideHeight*0.5f);
    

    _backgroundColor.set(0.0f,0.0f,0.0f,1.0f);
    _textColor.set(1.0f,1.0f,1.0f,1.0f);
    _textFont = "fonts/arial.ttf";
    
    _titlePositionRatios = osg::Vec3(0.5f,0.0f,0.92f);
    _titleAlignment = osgText::Text::CENTER_BASE_LINE;

    _titlePositionRatios = osg::Vec3(0.1f,0.0f,0.92f);
    _titleAlignment = osgText::Text::LEFT_BASE_LINE;

    _presentationDuration = 0.0;

}

void SlideShowConstructor::setPresentationAspectRatio(float aspectRatio)
{
    _slideWidth = _slideHeight*aspectRatio;
}

void SlideShowConstructor::setPresentationAspectRatio(const std::string& str)
{
    if (str=="Reality Theatre") setPresentationAspectRatio(3.0f);
    else if (str=="Desktop") setPresentationAspectRatio(1280.0f/1024.0f);
    else
    {
        float ratio = (float)atof(str.c_str());
        if (ratio!=0.0) setPresentationAspectRatio(1280.0f/1024.0f);
        else
        {
            std::cout<<"Error: presentation aspect ratio incorrect type"<<std::endl;
            std::cout<<"       valid types are \"Reality Theatre\", \"Desktop\" or a numerical value."<<std::endl;
        }
    }
}

void SlideShowConstructor::createPresentation()
{
    _titleHeight = _slideHeight*0.06f;
    _titleWidth = _slideWidth*0.8f;
    _titleOrigin = _slideOrigin + osg::Vec3(_titlePositionRatios.x()*_slideWidth,_titlePositionRatios.y()*1.0f,_titlePositionRatios.z()*_slideHeight);

    _textHeight = _slideHeight*0.04f;
    _textWidth = _slideWidth*0.8f;
    
    _textOrigin = _slideOrigin + osg::Vec3(_slideWidth*0.1f,0.0f,_titleOrigin.z()-_slideOrigin.z()-2*_textHeight);
    _imageOrigin = _slideOrigin + osg::Vec3(_slideWidth*0.7f,0.0f,_slideHeight*0.45f);
    
    _root = new osg::ClearNode;
    _root->setClearColor(_backgroundColor);
    
    _presentationSwitch = new osg::Switch;
    _presentationSwitch->setName(std::string("Presentation_")+_presentationName);
    
    _root->addChild(_presentationSwitch.get());
    
    osg::Vec3 slideCenter = _slideOrigin + osg::Vec3(_slideWidth*0.5f,0.0f,_slideHeight*0.5f);
       
    HomePosition* hp = new HomePosition;
    hp->eye.set(0.0f,0.0f,0.0f);
    hp->center = slideCenter;
    hp->up.set(0.0f,0.0f,1.0f);
    
    if (_presentationDuration!=0.0)
    {
        _presentationSwitch->setUserData(new Duration(_presentationDuration));
    }
     
    _root->setUserData(hp);
    
}

void SlideShowConstructor::setBackgroundColor(const osg::Vec4& color)
{
    _backgroundColor = color;
    if (_root.valid()) _root->setClearColor(_backgroundColor);
}

void SlideShowConstructor::setTextColor(const osg::Vec4& color)
{
    _textColor = color;
}

void SlideShowConstructor::setPresentationName(const std::string& name)
{
    _presentationName = name;
    if (_presentationSwitch.valid()) _presentationSwitch->setName(std::string("Presentation_")+_presentationName);
}

void SlideShowConstructor::setPresentationDuration(double duration)
{
    _presentationDuration = duration;
    if (_presentationDuration!=0.0 && _presentationSwitch.valid())
    {
        _presentationSwitch->setUserData(new Duration(_presentationDuration));
    }
}

void SlideShowConstructor::addSlide()
{
    if (!_presentationSwitch) createPresentation();

    // reset cursors
    _textCursor = _textOrigin;
    _imageCursor = _imageOrigin;
    
    _slide = new osg::Switch;
    _slide->setName(std::string("Slide_")+_slideTitle);
    
    _presentationSwitch->addChild(_slide.get());

    _previousLayer = 0;
    _currentLayer = 0;    
}


void SlideShowConstructor::setSlideDuration(double duration)
{
    if (!_slide) addSlide();

    if (_slide.valid())
    {
        _slide->setUserData(new Duration(duration));
    }
}

void SlideShowConstructor::addLayer()
{
    if (!_slide) addSlide();

    _previousLayer = _currentLayer;
    
    _currentLayer = new osg::Group;
    
    _slide->addChild(_currentLayer.get());
    
    if (!_previousLayer)
    {
        // create the background and title..
        if (!_slideBackgroundImageFileName.empty())
        {
            osg::Geometry* backgroundQuad = osg::createTexturedQuadGeometry(_slideOrigin,
                                                            osg::Vec3(_slideWidth,0.0f,0.0f),
                                                            osg::Vec3(0.0f,0.0f,_slideHeight));

            osg::Geode* background = new osg::Geode;

            osg::StateSet* backgroundStateSet = background->getOrCreateStateSet();
            backgroundStateSet->setAttributeAndModes(
                        new osg::PolygonOffset(1.0f,2.0f),
                        osg::StateAttribute::ON);

            backgroundStateSet->setTextureAttributeAndModes(0,
                        new osg::Texture2D(osgDB::readImageFile(_slideBackgroundImageFileName)),
                        osg::StateAttribute::ON);

            background->addDrawable(backgroundQuad);

            _currentLayer->addChild(background);
        }
        
        if (!_slideTitle.empty())
        {
            osg::Geode* geode = new osg::Geode;

            osgText::Text* text = new osgText::Text;
            text->setFont(_textFont);
            text->setColor(_textColor);
            text->setCharacterSize(_titleHeight);
            text->setMaximumWidth(_titleWidth);
            text->setAxisAlignment(osgText::Text::XZ_PLANE);
            text->setAlignment(_titleAlignment);
            text->setPosition(_titleOrigin);

            text->setText(_slideTitle);

            geode->addDrawable(text);

            _currentLayer->addChild(geode);
        }
        
    }
    else
    {
        // copy previous layer's children across into new layer.
        for(unsigned int i=0;i<_previousLayer->getNumChildren();++i)
        {
            _currentLayer->addChild(_previousLayer->getChild(i));
        }
    }

}

void SlideShowConstructor::setLayerDuration(double duration)
{
    if (!_currentLayer) addLayer();

    if (_currentLayer.valid())
    {
        _currentLayer->setUserData(new Duration(duration));
    }
}


void SlideShowConstructor::addBullet(const std::string& bullet)
{
    if (!_currentLayer) addLayer();

    osg::Geode* geode = new osg::Geode;

    osgText::Text* text = new osgText::Text;

    text->setFont(_textFont);
    text->setColor(_textColor);
    text->setCharacterSize(_textHeight);
    text->setMaximumWidth(_textWidth);
    text->setAxisAlignment(osgText::Text::XZ_PLANE);
    text->setAlignment(osgText::Text::BASE_LINE);
    text->setPosition(_textCursor);
    
    text->setText(bullet);

    osg::BoundingBox bb = text->getBound();
    _textCursor.z() = bb.zMin()-_textHeight*1.5;

    geode->addDrawable(text);
    
    _currentLayer->addChild(geode);
    
}

void SlideShowConstructor::addParagraph(const std::string& paragraph)
{
    if (!_currentLayer) addLayer();

    osg::Geode* geode = new osg::Geode;

    osgText::Text* text = new osgText::Text;

    text->setFont(_textFont);
    text->setColor(_textColor);
    text->setCharacterSize(_textHeight);
    text->setMaximumWidth(_textWidth);
    text->setAxisAlignment(osgText::Text::XZ_PLANE);
    text->setAlignment(osgText::Text::BASE_LINE);
    text->setPosition(_textCursor);
    
    text->setText(paragraph);

    osg::BoundingBox bb = text->getBound();
    _textCursor.z() = bb.zMin()-_textHeight*1.5;

    geode->addDrawable(text);
    
    _currentLayer->addChild(geode);
}

void SlideShowConstructor::addImage(const std::string& filename,float height)
{
    if (!_currentLayer) addLayer();

    osg::Image* image = osgDB::readImageFile(filename);
    
    if (!image) return;
    
    float s = image->s();
    float t = image->t();
    
    float image_height = _slideHeight*0.6f;
    float image_width =  image_height*s/t;
    float offset = height*image_height*0.1f;
    
    osg::Vec3 pos = _imageCursor + osg::Vec3(-image_width*0.5f+offset,-offset,-image_height*0.5f-offset);

    osg::Geometry* pictureQuad = osg::createTexturedQuadGeometry(pos,
                                                    osg::Vec3(image_width,0.0f,0.0f),
                                                    osg::Vec3(0.0f,0.0f,image_height));

    osg::Geode* picture = new osg::Geode;

    osg::StateSet* pictureStateSet = picture->getOrCreateStateSet();

    pictureStateSet->setTextureAttributeAndModes(0,
                new osg::Texture2D(image),
                osg::StateAttribute::ON);

    picture->addDrawable(pictureQuad);

    _currentLayer->addChild(picture);
}

void SlideShowConstructor::addStereoImagePair(const std::string& filenameLeft,const std::string& filenameRight,float height)
{
    if (!_currentLayer) addLayer();

    osg::ref_ptr<osg::Image> imageLeft = osgDB::readImageFile(filenameLeft);
    osg::ref_ptr<osg::Image> imageRight = osgDB::readImageFile(filenameRight);
    
    if (!imageLeft && !imageRight) return;
    
    float s = imageLeft->s();
    float t = imageLeft->t();
    
    float image_height = _slideHeight*0.6f;
    float image_width =  image_height*s/t;
    float offset = height*image_height*0.1f;
    
    osg::Vec3 pos = _imageCursor + osg::Vec3(-image_width*0.5f+offset,-offset,-image_height*0.5f-offset);


    osg::Geode* pictureLeft = new osg::Geode;
    {
        pictureLeft->setNodeMask(0x01);

        osg::StateSet* pictureLeftStateSet = pictureLeft->getOrCreateStateSet();

        pictureLeftStateSet->setTextureAttributeAndModes(0,
                    new osg::Texture2D(imageLeft.get()),
                    osg::StateAttribute::ON);

        osg::Geometry* pictureLeftQuad = osg::createTexturedQuadGeometry(pos,
                                                        osg::Vec3(image_width,0.0f,0.0f),
                                                        osg::Vec3(0.0f,0.0f,image_height));
        pictureLeft->addDrawable(pictureLeftQuad);

    }
    
    osg::Geode* pictureRight = new osg::Geode;
    {
        pictureRight->setNodeMask(0x02);

        osg::StateSet* pictureRightStateSet = pictureRight->getOrCreateStateSet();

        pictureRightStateSet->setTextureAttributeAndModes(0,
                    new osg::Texture2D(imageRight.get()),
                    osg::StateAttribute::ON);

        osg::Geometry* pictureRightQuad = osg::createTexturedQuadGeometry(pos,
                                                        osg::Vec3(image_width,0.0f,0.0f),
                                                        osg::Vec3(0.0f,0.0f,image_height));

        pictureRight->addDrawable(pictureRightQuad);
    }
    
    osg::Group* stereopair = new osg::Group;
    stereopair->addChild(pictureLeft);
    stereopair->addChild(pictureRight);

    _currentLayer->addChild(stereopair);
}

void SlideShowConstructor::addModel(const std::string& filename, CoordinateFrame coordinate_frame, const osg::Vec3& position, float scale, const osg::Vec4& rotate, const osg::Vec4& rotation)
{
    if (!_currentLayer) addLayer();

    osg::Node* model = osgDB::readNodeFile(filename);

    if (!model) return;

    if (coordinate_frame==SLIDE)
    {
        osg::Vec3 pos(_slideWidth*position.x(),
                      _slideDistance*position.y(),
                      _slideHeight*position.z());
     
        const osg::BoundingSphere& bs = model->getBound();
        float model_scale = scale*_slideHeight*0.7f/bs.radius();

        osg::MatrixTransform* transform = new osg::MatrixTransform;
        transform->setDataVariance(osg::Object::STATIC);
        transform->setMatrix(osg::Matrix::translate(-bs.center())*
                             osg::Matrix::scale(model_scale,model_scale,model_scale)*
                             osg::Matrix::rotate(osg::DegreesToRadians(rotate[0]),rotate[1],rotate[2],rotate[3])*
                             osg::Matrix::translate(pos));

        transform->addChild(model);

        if (rotation[0]!=0.0)
        {
            osg::MatrixTransform* animation_transform = new osg::MatrixTransform;
            animation_transform->setDataVariance(osg::Object::DYNAMIC);
            animation_transform->setUpdateCallback(new osgUtil::TransformCallback(pos,osg::Vec3(rotation[1],rotation[2],rotation[3]),osg::DegreesToRadians(rotation[0])));
            animation_transform->addChild(transform);            

            _currentLayer->addChild(animation_transform);
        }
        else
        {
            _currentLayer->addChild(transform);
        }

    }
    else
    {
        osg::MatrixTransform* transform = new osg::MatrixTransform;
        transform->setDataVariance(osg::Object::STATIC);
        transform->setMatrix(osg::Matrix::translate(-position)*
                             osg::Matrix::scale(scale,scale,scale)*
                             osg::Matrix::rotate(osg::DegreesToRadians(rotate[0]),rotate[1],rotate[2],rotate[3]));

        transform->addChild(model);
        
        if (rotation[0]!=0.0)
        {
            osg::MatrixTransform* animation_transform = new osg::MatrixTransform;
            animation_transform->setDataVariance(osg::Object::DYNAMIC);
            animation_transform->setUpdateCallback(new osgUtil::TransformCallback(osg::Vec3(0.0f,0.0f,0.0f),osg::Vec3(rotation[1],rotation[2],rotation[3]),osg::DegreesToRadians(rotation[0])));
            animation_transform->addChild(transform);            

            _currentLayer->addChild(animation_transform);
        }   
        else
        {
            _currentLayer->addChild(transform);
        }

    }
}

void SlideShowConstructor::addModelWithPath(const std::string& filename, CoordinateFrame coordinate_frame, const osg::Vec3& position, float scale, const osg::Vec4& rotate, const std::string& animation_path)
{
    if (!_currentLayer) addLayer();

    osg::Node* model = osgDB::readNodeFile(filename);

    if (!model) return;

    osg::AnimationPath* animation = 0;
    if (!animation_path.empty()) 
    {
        std::string absolute_animation_file_path = osgDB::findDataFile(animation_path);
        if (!absolute_animation_file_path.empty())
        {        
            std::ifstream animation_filestream(absolute_animation_file_path.c_str());
            if (!animation_filestream.eof())
            {
                animation = new osg::AnimationPath;
                animation->read(animation_filestream);
            }
        }
    }


    if (coordinate_frame==SLIDE)
    {
        osg::Vec3 pos(_slideWidth*position.x(),
                      _slideDistance*position.y(),
                      _slideHeight*position.z());
     
        const osg::BoundingSphere& bs = model->getBound();
        float model_scale = scale*_slideHeight*0.7f/bs.radius();

        osg::MatrixTransform* transform = new osg::MatrixTransform;
        transform->setDataVariance(osg::Object::STATIC);
        transform->setMatrix(osg::Matrix::translate(-bs.center())*
                             osg::Matrix::scale(model_scale,model_scale,model_scale)*
                             osg::Matrix::rotate(osg::DegreesToRadians(rotate[0]),rotate[1],rotate[2],rotate[3])*
                             osg::Matrix::translate(pos));

        transform->addChild(model);

        _currentLayer->addChild(transform);

    }
    else
    {
        osg::MatrixTransform* transform = new osg::MatrixTransform;
        transform->setDataVariance(osg::Object::STATIC);
        transform->setMatrix(osg::Matrix::translate(-position)*
                             osg::Matrix::scale(scale,scale,scale)*
                             osg::Matrix::rotate(osg::DegreesToRadians(rotate[0]),rotate[1],rotate[2],rotate[3]));

        transform->addChild(model);
        
        if (animation)
        {
            osg::MatrixTransform* animation_transform = new osg::MatrixTransform;
            animation_transform->setDataVariance(osg::Object::DYNAMIC);
            
            osg::AnimationPathCallback* apc = new osg::AnimationPathCallback(animation);
            
            animation_transform->setUpdateCallback(apc);
            animation_transform->addChild(transform);    
            
            _currentLayer->addChild(animation_transform);

// 
//             osg::MatrixTransform* orientation_transform = new osg::MatrixTransform;
//             orientation_transform->setDataVariance(osg::Object::STATIC);
//             orientation_transform->addChild(animation_transform);
//             
//             //orientation_transform->setMatrix(osg::Matrix::rotate(osg::DegreesToRadians(-00.0f),0.0f,1.0f,0.0f));
//             //orientation_transform->setMatrix(osg::Matrix::inverse(osg::Matrix::lookAt(osg::Vec3(0.0,0.0,0.0),osg::Vec3(0.0,1.0,0.0),osg::Vec3(0.0,0.0,1.0))));
// 
//             _currentLayer->addChild(orientation_transform);

        }
        else
        {
            _currentLayer->addChild(transform);
        }

    }
}

void SlideShowConstructor::addModelWithCameraPath(const std::string& filename, CoordinateFrame coordinate_frame, const osg::Vec3& position, float scale, const osg::Vec4& rotate, const std::string& animation_path)
{
    if (!_currentLayer) addLayer();

    osg::Node* model = osgDB::readNodeFile(filename);

    if (!model) return;

    osg::AnimationPath* animation = 0;
    if (!animation_path.empty()) 
    {
        std::string absolute_animation_file_path = osgDB::findDataFile(animation_path);
        if (!absolute_animation_file_path.empty())
        {        
            std::ifstream animation_filestream(absolute_animation_file_path.c_str());
            if (!animation_filestream.eof())
            {
                animation = new osg::AnimationPath;
                animation->read(animation_filestream);
            }
        }
    }


    if (coordinate_frame==SLIDE)
    {
        osg::Vec3 pos(_slideWidth*position.x(),
                      _slideDistance*position.y(),
                      _slideHeight*position.z());
     
        const osg::BoundingSphere& bs = model->getBound();
        float model_scale = scale*_slideHeight*0.7f/bs.radius();

        osg::MatrixTransform* transform = new osg::MatrixTransform;
        transform->setDataVariance(osg::Object::STATIC);
        transform->setMatrix(osg::Matrix::translate(-bs.center())*
                             osg::Matrix::scale(model_scale,model_scale,model_scale)*
                             osg::Matrix::rotate(osg::DegreesToRadians(rotate[0]),rotate[1],rotate[2],rotate[3])*
                             osg::Matrix::translate(pos));

        transform->addChild(model);

        _currentLayer->addChild(transform);

    }
    else
    {
        osg::MatrixTransform* transform = new osg::MatrixTransform;
        transform->setDataVariance(osg::Object::STATIC);
        transform->setMatrix(osg::Matrix::translate(-position)*
                             osg::Matrix::scale(scale,scale,scale)*
                             osg::Matrix::rotate(osg::DegreesToRadians(rotate[0]),rotate[1],rotate[2],rotate[3]));

        transform->addChild(model);
        
        if (animation)
        {
            osg::MatrixTransform* animation_transform = new osg::MatrixTransform;
            animation_transform->setDataVariance(osg::Object::DYNAMIC);
            
            osg::AnimationPathCallback* apc = new osg::AnimationPathCallback(animation);
            apc->setUseInverseMatrix(true);
            
            animation_transform->setUpdateCallback(apc);
            animation_transform->addChild(transform);    

            osg::MatrixTransform* orientation_transform = new osg::MatrixTransform;
            orientation_transform->setDataVariance(osg::Object::STATIC);
            orientation_transform->addChild(animation_transform);
            
            //orientation_transform->setMatrix(osg::Matrix::rotate(osg::DegreesToRadians(-00.0f),0.0f,1.0f,0.0f));
            orientation_transform->setMatrix(osg::Matrix::inverse(osg::Matrix::lookAt(osg::Vec3(0.0,0.0,0.0),osg::Vec3(0.0,1.0,0.0),osg::Vec3(0.0,0.0,1.0))));

            _currentLayer->addChild(orientation_transform);

        }
        else
        {
            _currentLayer->addChild(transform);
        }

    }
}
