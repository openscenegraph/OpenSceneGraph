/* -*-c++-*- Present3D - Copyright (C) 1999-2006 Robert Osfield
 *
 * This software is open source and may be redistributed and/or modified under
 * the terms of the GNU General Public License (GPL) version 2.0.
 * The full license is in LICENSE.txt file included with this distribution,.
 *
 * This software is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * include LICENSE.txt for more details.
*/

#include <osgPresentation/SlideShowConstructor>

#include <osg/Geometry>
#include <osg/PolygonOffset>
#include <osg/Geode>
#include <osg/Texture2D>
#include <osg/TextureRectangle>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/TexMat>
#include <osg/ShapeDrawable>
#include <osg/ImageSequence>
#include <osg/ImageUtils>
#include <osg/ClipNode>
#include <osg/ComputeBoundsVisitor>
#include <osg/Notify>
#include <osg/io_utils>

#include <osgUtil/TransformCallback>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>
#include <osgDB/Input>
#include <osgDB/FileNameUtils>

#include <osgWidget/PdfReader>

#include <osgViewer/ViewerEventHandlers>

#include <osgText/Text>

#include <osgFX/SpecularHighlights>

#include <osgVolume/Volume>
#include <osgVolume/RayTracedTechnique>
#include <osgVolume/FixedFunctionTechnique>

#include <sstream>
#include <algorithm>

#include <osgPresentation/AnimationMaterial>
#include <osgPresentation/PickEventHandler>
#include <osgPresentation/KeyEventHandler>

#include <osgManipulator/TabBoxDragger>
#include <osgManipulator/TabBoxTrackballDragger>
#include <osgManipulator/TrackballDragger>

using namespace osgPresentation;

#define USE_CLIENT_STORAGE_HINT 0


class SetToTransparentBin : public osg::NodeVisitor
{
public:

    SetToTransparentBin():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

    virtual void appply(osg::Node& node)
    {
        if (node.getStateSet())
        {
            node.getStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
            node.getStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        }
    }

    virtual void apply(osg::Geode& geode)
    {
        if (geode.getStateSet())
        {
            geode.getStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
            geode.getStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
        }
        for(unsigned int i=0;i<geode.getNumDrawables();++i)
        {
            if (geode.getDrawable(i)->getStateSet())
            {
                geode.getDrawable(i)->getStateSet()->setMode(GL_BLEND,osg::StateAttribute::ON);
                geode.getDrawable(i)->getStateSet()->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);
            }
        }
    }
};


HUDTransform::HUDTransform(HUDSettings* hudSettings):
    _hudSettings(hudSettings)
{
    setDataVariance(osg::Object::DYNAMIC);
    setReferenceFrame(osg::Transform::ABSOLUTE_RF);
}

HUDTransform::~HUDTransform() {}

bool HUDTransform::computeLocalToWorldMatrix(osg::Matrix& matrix,osg::NodeVisitor* nv) const
{
    return _hudSettings->getModelViewMatrix(matrix,nv);
}

bool HUDTransform::computeWorldToLocalMatrix(osg::Matrix& matrix,osg::NodeVisitor* nv) const
{
    return _hudSettings->getInverseModelViewMatrix(matrix,nv);
}

SlideShowConstructor::SlideShowConstructor(osgDB::Options* options):
    _options(options)
{
    const osg::DisplaySettings* ds = osg::DisplaySettings::instance().get();

    _propertyManager = new osgPresentation::PropertyManager;
    _propertyEventCallback = new osgPresentation::PropertyEventCallback(_propertyManager.get());

    _slideHeight = ds->getScreenHeight();
    _slideWidth = ds->getScreenWidth();
    _slideDistance = ds->getScreenDistance();
    _leftEyeMask = 0x01;
    _rightEyeMask = 0x02;

    _hudSettings = new HUDSettings(_slideDistance, ds->getEyeSeparation()*0.5, _leftEyeMask, _rightEyeMask);

    _backgroundColor.set(0.0f,0.0f,0.0f,0.0f);

    _presentationDuration = -1.0;

    // set up title defaults
    _titleFontDataDefault.font = "fonts/arial.ttf";
    _titleFontDataDefault.color.set(1.0f,1.0f,1.0f,1.0f);
    _titleFontDataDefault.layout =osgText::Text::LEFT_TO_RIGHT;
    _titleFontDataDefault.alignment = osgText::Text::CENTER_BASE_LINE;
    _titleFontDataDefault.axisAlignment = osgText::Text::XZ_PLANE;
    _titleFontDataDefault.characterSize = 0.06f;
    _titleFontDataDefault.maximumWidth = 0.9f;

    _titlePositionDataDefault.position.set(0.5f,0.92f,0.0f);

    // set up text defaults
    _textFontDataDefault.font = "fonts/arial.ttf";
    _textFontDataDefault.color.set(1.0f,1.0f,1.0f,1.0f);
    _textFontDataDefault.layout = osgText::Text::LEFT_TO_RIGHT;
    _textFontDataDefault.alignment = osgText::Text::LEFT_BASE_LINE;
    _textFontDataDefault.axisAlignment = osgText::Text::XZ_PLANE;
    _textFontDataDefault.characterSize = 0.04f;
    _textFontDataDefault.maximumWidth = 0.8f;

    _textPositionDataDefault.position.set(0.1f,0.85f,0.0f);

    _loopPresentation = false;
    _autoSteppingActive = false;

    _slideBackgroundAsHUD = false;

    _layerToApplyEventCallbackTo = 0;
    _currentEventCallbacksToApply.clear();
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
            OSG_WARN<<"Error: presentation aspect ratio incorrect type"<<std::endl;
            OSG_WARN<<"       valid types are \"Reality Theatre\", \"Desktop\" or a numerical value."<<std::endl;
        }
    }
}

void SlideShowConstructor::createPresentation()
{
    _slideOrigin.set(-_slideWidth*0.5f,_slideDistance,-_slideHeight*0.5f);

#if 0
    _titleFontDataDefault.characterSize = 0.06f;
    _titleFontDataDefault.maximumWidth = 0.9f;

    _textFontDataDefault.characterSize = 0.04f;
    _textFontDataDefault.maximumWidth = 0.8f;
#endif

    OSG_INFO<<"_titlePositionDataDefault.position="<<_titlePositionDataDefault.position<<std::endl;

    _textPositionDataDefault.position.set(0.1f,_titlePositionDataDefault.position.y()-_titleFontDataDefault.characterSize,0.0f);
    _imagePositionDataDefault.position.set(0.5f,0.5f,0.0f);
    _modelPositionDataDefault.position.set(0.5f,0.5f,0.0f);

    _root = new osg::Group;

    _presentationSwitch = new osg::Switch;
    _presentationSwitch->setName(std::string("Presentation_")+_presentationName);

    _root->addChild(_presentationSwitch.get());
    _root->setName(std::string("Presentation_")+_presentationName);

    osg::Vec3 slideCenter = _slideOrigin + osg::Vec3(_slideWidth*0.5f,0.0f,_slideHeight*0.5f);

    HomePosition* hp = new HomePosition;
    hp->eye.set(0.0f,0.0f,0.0f);
    hp->center = slideCenter;
    hp->up.set(0.0f,0.0f,1.0f);

    OSG_INFO<<" slideCenter "<<slideCenter<<std::endl;

    if (_presentationDuration>=0.0)
    {
        setDuration(_presentationSwitch.get(),_presentationDuration);
    }

    _root->setUserData(hp);

    if (_loopPresentation) _root->addDescription("loop");
    if (_autoSteppingActive) _root->addDescription("auto");

    //_root->addEventCallback(_propertyEventCallback.get());

    _presentationSwitch->setEventCallback(_propertyEventCallback.get());
}

LayerAttributes* SlideShowConstructor::getOrCreateLayerAttributes(osg::Node* node)
{
    LayerAttributes* la = dynamic_cast<LayerAttributes*>(node->getUserData());
    if (!la)
    {
        if (node->getUserData())
        {
            OSG_NOTICE<<"UserData already assigned, overriding to set LayerAttributes."<<std::endl;
        }

        la = new LayerAttributes;
        node->setUserData(la);
    }

    return la;
}

void SlideShowConstructor::setBackgroundColor(const osg::Vec4& color, bool updateClearNode)
{
    _backgroundColor = color;
    if (updateClearNode && _slideClearNode.valid()) _slideClearNode->setClearColor(_backgroundColor);
}

void SlideShowConstructor::setTextColor(const osg::Vec4& color)
{
    _titleFontDataDefault.color = color;
    _textFontDataDefault.color = color;

    _titleFontData.color = _titleFontDataDefault.color;
    _textFontData.color = _textFontDataDefault.color;

}

void SlideShowConstructor::setPresentationName(const std::string& name)
{
    _presentationName = name;
    if (_presentationSwitch.valid()) _presentationSwitch->setName(std::string("Presentation_")+_presentationName);
}

void SlideShowConstructor::setPresentationDuration(double duration)
{
    _presentationDuration = duration;
    if (_presentationDuration>=0.0 && _presentationSwitch.valid())
    {
        setDuration(_presentationSwitch.get(),_presentationDuration);
    }
}

void SlideShowConstructor::addSlide()
{
    if (!_presentationSwitch) createPresentation();

    // reset fonts
    _titleFontData = _titleFontDataDefault;
    _textFontData = _textFontDataDefault;

    // reset cursors
    _titlePositionData = _titlePositionDataDefault;
    _textPositionData = _textPositionDataDefault;
    _imagePositionData =  _imagePositionDataDefault;
    _modelPositionData =  _modelPositionDataDefault;

    _slide = new osg::Switch;
    _slide->setName(std::string("Slide_")+_slideTitle);

    _slideClearNode = new osg::ClearNode;
    _slideClearNode->setClearColor(_backgroundColor);
    _slideClearNode->addChild(_slide.get());

    _presentationSwitch->addChild(_slideClearNode.get());

    _previousLayer = 0;
    _currentLayer = 0;


    _filePathData = new FilePathData(osgDB::getDataFilePathList());

    _slideClearNode->setUserData(_filePathData.get());
}

void SlideShowConstructor::selectSlide(int slideNum)
{
    if (slideNum<0)
    {
        addSlide();
    }
    else if (slideNum>=static_cast<int>(_presentationSwitch->getNumChildren()))
    {
        addSlide();
    }
    else
    {
        _slideClearNode = dynamic_cast<osg::ClearNode*>(_presentationSwitch->getChild(slideNum));
        if (!_slideClearNode || _slideClearNode->getNumChildren()==0 || _slideClearNode->getChild(0)->asSwitch()==0)
        {
            addSlide();
        }
        else
        {
            _slide = _slideClearNode->getChild(0)->asSwitch();
            _previousLayer = _slide->getChild(_slide->getNumChildren()-1)->asGroup();
            _currentLayer = 0;
        }
    }
}

void SlideShowConstructor::setSlideDuration(double duration)
{
    if (!_slide) addSlide();

    if (_slide.valid())
    {
        setDuration(_slide.get(),duration);
    }
}


Timeout* SlideShowConstructor::addTimeout()
{
    osg::ref_ptr<osgPresentation::Timeout> timeout = new osgPresentation::Timeout(_hudSettings.get());
    if (_currentLayer.valid()) _currentLayer->addChild(timeout.get());
    _currentLayer = timeout.get();
    return timeout.release();
}

void SlideShowConstructor::pushCurrentLayer()
{
    _layerStack.push_back(_currentLayer.get());
}

void SlideShowConstructor::popCurrentLayer()
{
    if (!_layerStack.empty())
    {
        _currentLayer = _layerStack.back();
        _layerStack.pop_back();
    }
}

void SlideShowConstructor::addLayer(bool inheritPreviousLayers, bool defineAsBaseLayer)
{
    if (!_slide) addSlide();

    _currentLayer = new osg::Group;
    _currentLayer->setName("Layer");

    // OSG_NOTICE<<"addLayer"<<std::endl;

    if (!_previousLayer || !inheritPreviousLayers)
    {
        _textPositionData = _textPositionDataDefault;
        _imagePositionData =  _imagePositionDataDefault;
        _modelPositionData =  _modelPositionDataDefault;

        // OSG_NOTICE<<"   new layer background = "<<_slideBackgroundImageFileName<<std::endl;

        osg::ref_ptr<osg::Image> image = !_slideBackgroundImageFileName.empty() ?
            osgDB::readImageFile(_slideBackgroundImageFileName, _options.get()) :
            0;

        // create the background and title..
        if (image.valid())
        {
            osg::Geode* background = new osg::Geode;

            osg::StateSet* backgroundStateSet = background->getOrCreateStateSet();
            backgroundStateSet->setAttributeAndModes(
                        new osg::PolygonOffset(1.0f,2.0f),
                        osg::StateAttribute::ON);


            bool useTextureRectangle = true;
            float s = useTextureRectangle ? image->s() : 1.0;
            float t = useTextureRectangle ? image->t() : 1.0;
            osg::Geometry* backgroundQuad = osg::createTexturedQuadGeometry(_slideOrigin,
                                                            osg::Vec3(_slideWidth,0.0f,0.0f),
                                                            osg::Vec3(0.0f,0.0f,_slideHeight),
                                                            s, t);
            // OSG_NOTICE<<"Image loaded "<<image.get()<<"  "<<_slideBackgroundImageFileName<<std::endl;

            if (useTextureRectangle)
            {
                osg::TextureRectangle* texture = new osg::TextureRectangle(image.get());
                backgroundStateSet->setTextureAttributeAndModes(0,
                            texture,
                            osg::StateAttribute::ON);
            }
            else
            {
                osg::Texture2D* texture = new osg::Texture2D(image.get());
                texture->setResizeNonPowerOfTwoHint(false);
                texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
                texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
#if USE_CLIENT_STORAGE_HINT
                texture->setClientStorageHint(true);
#endif
                backgroundStateSet->setTextureAttributeAndModes(0,
                            texture,
                            osg::StateAttribute::ON);
            }

            background->addDrawable(backgroundQuad);

            if (_slideBackgroundAsHUD)
            {
                HUDTransform* hudTransform = new HUDTransform(_hudSettings.get());
                hudTransform->addChild(background);
                addToCurrentLayer(hudTransform);
            }
            else
            {
                addToCurrentLayer(background);
            }
        }

        if (!_slideTitle.empty())
        {
            osg::Geode* geode = new osg::Geode;

            osg::Vec3 localPosition = computePositionInModelCoords(_titlePositionData);

            osgText::Text* text = new osgText::Text;
            text->setFont(osgText::readFontFile(_titleFontData.font, _options.get()));
            text->setColor(_titleFontData.color);
            text->setCharacterSize(_titleFontData.characterSize*_slideHeight);
            text->setFontResolution(110,120);
            text->setMaximumWidth(_titleFontData.maximumWidth*_slideWidth);
            text->setLayout(_titleFontData.layout);
            text->setAlignment(_titleFontData.alignment);
            text->setAxisAlignment(_titleFontData.axisAlignment);
            //text->setPosition(_titlePositionData.position);
            text->setPosition(localPosition);

            text->setText(_slideTitle);

            geode->addDrawable(text);

            addToCurrentLayer(decorateSubgraphForPosition(geode, _titlePositionData));
        }

    }
    else
    {
        // copy previous layer's children across into new layer.
        for(unsigned int i=0;i<_previousLayer->getNumChildren();++i)
        {
            addToCurrentLayer(_previousLayer->getChild(i));
        }
    }

    if (!defineAsBaseLayer)
    {
        _slide->addChild(_currentLayer.get());
    }

    _previousLayer = _currentLayer;
}

void SlideShowConstructor::selectLayer(int layerNum)
{
    if (!_slide)
    {
        addSlide();
        addLayer();
    }
    else if (layerNum>=0 && layerNum<static_cast<int>(_slide->getNumChildren()) && _slide->getChild(layerNum)->asGroup())
    {
        _currentLayer = _slide->getChild(layerNum)->asGroup();
        _previousLayer = _currentLayer;
    }
    else
    {
        addLayer();
    }

}


void SlideShowConstructor::setLayerDuration(double duration)
{
    if (!_currentLayer) addLayer();

    if (_currentLayer.valid())
    {
        setDuration(_currentLayer.get(),duration);
    }
}

void SlideShowConstructor::addToCurrentLayer(osg::Node* subgraph)
{
    if (!subgraph) return;

    if (!_currentLayer) addLayer();

    if (!_currentEventCallbacksToApply.empty())
    {
        if (_layerToApplyEventCallbackTo==0 || _currentLayer==_layerToApplyEventCallbackTo)
        {
            OSG_NOTICE<<"Assigning event callbacks."<<std::endl;

            for(EventHandlerList::iterator itr = _currentEventCallbacksToApply.begin();
                itr != _currentEventCallbacksToApply.end();
                ++itr)
            {
                subgraph->addEventCallback(itr->get());
            }
        }
        else
        {
            OSG_INFO<<"Ignoring event callback from previous layer."<<std::endl;
        }

        _currentEventCallbacksToApply.clear();
    }
    _currentLayer->addChild(subgraph);
}

void SlideShowConstructor::addEventHandler(PresentationContext presentationContext, osg::ref_ptr<osgGA::GUIEventHandler> handler)
{
    switch(presentationContext)
    {
        case(CURRENT_PRESENTATION):
            OSG_NOTICE<<"Need to add event handler to presentation."<<std::endl;
            break;
        case(CURRENT_SLIDE):
            OSG_NOTICE<<"Need to add event handler to slide."<<std::endl;
            break;
        case(CURRENT_LAYER):
            OSG_INFO<<"Add event handler to layer."<<std::endl;
            _layerToApplyEventCallbackTo = _currentLayer;
            _currentEventCallbacksToApply.push_back(handler);
            break;
    }
}

void SlideShowConstructor::keyToDoOperation(PresentationContext presentationContext, int key, Operation operation, const JumpData& jumpData)
{
    OSG_INFO<<"keyToDoOperation(key="<<key<<", operation="<<operation<<")"<<std::endl;
    addEventHandler(presentationContext, new KeyEventHandler(key, operation, jumpData));
}


void SlideShowConstructor::keyToDoOperation(PresentationContext presentationContext, int key, const std::string& command, Operation operation, const JumpData& jumpData)
{
    OSG_INFO<<"keyToDoOperation(key="<<key<<",command="<<command<<")"<<std::endl;
    addEventHandler(presentationContext, new KeyEventHandler(key, command, operation, jumpData));
}


void SlideShowConstructor::keyEventOperation(PresentationContext presentationContext, int key, const KeyPosition& keyPos,  const JumpData& jumpData)
{
    OSG_INFO<<"keyEventOperation(key="<<key<<")"<<std::endl;
    addEventHandler(presentationContext, new KeyEventHandler(key, keyPos, jumpData));
}


void SlideShowConstructor::layerClickToDoOperation(Operation operation, const JumpData& jumpData)
{
    addEventHandler(CURRENT_LAYER, new PickEventHandler(operation, jumpData));
}


void SlideShowConstructor::layerClickToDoOperation(const std::string& command, Operation operation, const JumpData& jumpData)
{
    addEventHandler(CURRENT_LAYER, new PickEventHandler(command, operation, jumpData));
}


void SlideShowConstructor::layerClickEventOperation(const KeyPosition& keyPos, const JumpData& jumpData)
{
    addEventHandler(CURRENT_LAYER, new PickEventHandler(keyPos, jumpData));
}



void SlideShowConstructor::addPropertyAnimation(PresentationContext presentationContext, PropertyAnimation* propertyAnimation)
{
    switch(presentationContext)
    {
        case(CURRENT_PRESENTATION):
            OSG_NOTICE<<"Need to add PropertyAnimation to presentation."<<std::endl;
            if (!_presentationSwitch) createPresentation();
            if (_presentationSwitch.valid()) _presentationSwitch->addUpdateCallback(propertyAnimation);
            break;
        case(CURRENT_SLIDE):
            OSG_NOTICE<<"Need to add PropertyAnimation to slide."<<std::endl;
            if (!_slide) addSlide();
            if (_slide.valid()) _slide->addUpdateCallback(propertyAnimation);
            break;
        case(CURRENT_LAYER):
            OSG_NOTICE<<"Need to add PropertyAnimation to layer."<<std::endl;
            if (!_currentLayer) addLayer();
            if (_currentLayer.valid())
            {
                _currentLayer->addUpdateCallback(propertyAnimation);
            }
            break;
    }
}


osg::Node* SlideShowConstructor::decorateSubgraphForPosition(osg::Node* node, PositionData& positionData)
{
    osg::Node* subgraph = node;

    if (positionData.requiresMaterialAnimation())
    {
        subgraph = attachMaterialAnimation(subgraph,positionData);
    }

    if (positionData.rotation[0]!=0.0)
    {
        osg::MatrixTransform* animation_transform = new osg::MatrixTransform;
        animation_transform->setDataVariance(osg::Object::DYNAMIC);
        animation_transform->setUpdateCallback(
            new osgUtil::TransformCallback(subgraph->getBound().center(),
                                           osg::Vec3(positionData.rotation[1],positionData.rotation[2],positionData.rotation[3]),
                                           osg::DegreesToRadians(positionData.rotation[0])));
        animation_transform->addChild(subgraph);

        subgraph = animation_transform;
    }

    if (positionData.hud)
    {
        HUDTransform* hudTransform = new HUDTransform(_hudSettings.get());
        hudTransform->addChild(subgraph);

        subgraph = hudTransform;
    }
    return subgraph;
}

void SlideShowConstructor::addBullet(const std::string& bullet, PositionData& positionData, FontData& fontData)
{
    osg::Geode* geode = new osg::Geode;

    osgText::Text* text = new osgText::Text;

    osg::Vec3 localPosition = computePositionInModelCoords(positionData);

    text->setFont(osgText::readFontFile(fontData.font, _options.get()));
    text->setColor(fontData.color);
    text->setCharacterSize(fontData.characterSize*_slideHeight);
    text->setCharacterSizeMode(fontData.characterSizeMode);
    text->setFontResolution(110,120);
    text->setMaximumWidth(fontData.maximumWidth*_slideWidth);
    text->setLayout(fontData.layout);
    text->setAlignment(fontData.alignment);
    text->setAxisAlignment(fontData.axisAlignment);
    text->setPosition(localPosition);

    if (positionData.autoRotate)
    {
        text->setAxisAlignment(osgText::Text::SCREEN);
    }

    if (positionData.autoScale)
    {
        text->setCharacterSizeMode(osgText::Text::SCREEN_COORDS);
    }

    text->setText(bullet);

    osg::BoundingBox bb = text->getBound();

    // note, this increment is only "correct" when text is on the plane of the slide..
    // will need to make this more general later.
    localPosition.z() = bb.zMin()-fontData.characterSize*_slideHeight*1.5;

    geode->addDrawable(text);

    addToCurrentLayer( decorateSubgraphForPosition(geode, positionData) );

    bool needToApplyPosition = (_textPositionData.position == positionData.position);
    if (needToApplyPosition)
    {
        updatePositionFromInModelCoords(localPosition, _textPositionData);
    }
}

void SlideShowConstructor::addParagraph(const std::string& paragraph, PositionData& positionData, FontData& fontData)
{
    osg::Geode* geode = new osg::Geode;

    osg::Vec3 localPosition = computePositionInModelCoords(positionData);

    osgText::Text* text = new osgText::Text;

    text->setFont(osgText::readFontFile(fontData.font, _options.get()));
    text->setColor(fontData.color);
    text->setCharacterSize(fontData.characterSize*_slideHeight);
    text->setCharacterSizeMode(fontData.characterSizeMode);
    text->setFontResolution(110,120);
    text->setMaximumWidth(fontData.maximumWidth*_slideWidth);
    text->setLayout(fontData.layout);
    text->setAlignment(fontData.alignment);
    text->setAxisAlignment(fontData.axisAlignment);
    text->setPosition(localPosition);

    if (positionData.autoRotate)
    {
        text->setAxisAlignment(osgText::Text::SCREEN);
    }

    if (positionData.autoScale)
    {
        text->setCharacterSizeMode(osgText::Text::SCREEN_COORDS);
    }
    text->setText(paragraph);

    osg::BoundingBox bb = text->getBound();

    // note, this increment is only "correct" when text is on the plane of the slide..
    // will need to make this more general later.
    localPosition.z() = bb.zMin()-fontData.characterSize*_slideHeight*1.5;

    geode->addDrawable(text);

    addToCurrentLayer( decorateSubgraphForPosition(geode, positionData) );

    bool needToApplyPosition = (_textPositionData.position == positionData.position);
    if (needToApplyPosition)
    {
        updatePositionFromInModelCoords(localPosition, _textPositionData);
    }
}

class FindImageStreamsVisitor : public osg::NodeVisitor
{
public:
    FindImageStreamsVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

    virtual void apply(osg::Node& node)
    {
        if (node.getStateSet())
        {
                process(node.getStateSet());
        }
        traverse(node);
    }

    virtual void apply(osg::Geode& node)
    {
        if (node.getStateSet())
        {
                process(node.getStateSet());
        }

        for(unsigned int i=0;i<node.getNumDrawables();++i)
        {
            osg::Drawable* drawable = node.getDrawable(i);
            if (drawable && drawable->getStateSet())
            {
                process(drawable->getStateSet());
            }
        }
    }

    void process(osg::StateSet* ss)
    {
        for(unsigned int i=0;i<ss->getTextureAttributeList().size();++i)
        {
            osg::Texture* texture = dynamic_cast<osg::Texture*>(ss->getTextureAttribute(i,osg::StateAttribute::TEXTURE));
            osg::Image* image = texture ? texture->getImage(0) : 0;
            osg::ImageStream* imageStream = image ? dynamic_cast<osg::ImageStream*>(image) : 0;
            if (imageStream)
            {
                texture->setDataVariance(osg::Object::DYNAMIC);
                texture->setUnRefImageDataAfterApply(false);
                texture->setResizeNonPowerOfTwoHint(false);
                texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
                texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
#if USE_CLIENT_STORAGE_HINT
                texture->setClientStorageHint(true);
#endif
            }
        }
    }

};

void SlideShowConstructor::findImageStreamsAndAddCallbacks(osg::Node* node)
{
    FindImageStreamsVisitor fisv;
    node->accept(fisv);
}


osg::Geometry* SlideShowConstructor::createTexturedQuadGeometry(const osg::Vec3& pos, const osg::Vec4& rotation, float width, float height, osg::Image* image, bool& usedTextureRectangle)
{
    osg::Geometry* pictureQuad = 0;
    osg::ref_ptr<osg::Texture> texture = 0;
    osg::StateSet* stateset = 0;

    osg::Vec3 positionVec = pos;
    osg::Vec3 widthVec(width,0.0f,0.0f);
    osg::Vec3 heightVec(0.0f,0.0f,height);

    osg::Matrixd rotationMatrix = osg::Matrixd::rotate(osg::DegreesToRadians(rotation[0]),rotation[1],rotation[2],rotation[3]);
    widthVec = widthVec*rotationMatrix;
    heightVec = heightVec*rotationMatrix;

    osg::ImageStream* imageStream = dynamic_cast<osg::ImageStream*>(image);

    // let the video-plugin create a texture for us, if supported
    if(imageStream && getenv("P3D_ENABLE_CORE_VIDEO"))
    {
        texture = imageStream->createSuitableTexture();
    }

    bool flipYAxis = image->getOrigin()==osg::Image::TOP_LEFT;

#if 1
    bool useTextureRectangle = false;
#else
    #ifdef __sgi
        bool useTextureRectangle = false;
    #else
        bool useTextureRectangle = true;
    #endif
#endif

    // pass back info on wether texture 2D is used.
    usedTextureRectangle = useTextureRectangle;

    if (!texture)
    {
        if (useTextureRectangle)
        {
            texture = new osg::TextureRectangle(image);
        }
        else
        {
            texture = new osg::Texture2D(image);

            texture->setResizeNonPowerOfTwoHint(false);
            texture->setFilter(osg::Texture::MIN_FILTER,osg::Texture::LINEAR);
            texture->setFilter(osg::Texture::MAG_FILTER,osg::Texture::LINEAR);
    #if USE_CLIENT_STORAGE_HINT
            texture->setClientStorageHint(true);
    #endif

        }
    }
    if (texture)
    {
        float t(0), l(0);
        float r = (texture->getTextureTarget() == GL_TEXTURE_RECTANGLE) ? image->s() : 1;
        float b = (texture->getTextureTarget() == GL_TEXTURE_RECTANGLE) ? image->t() : 1;

        if (flipYAxis)
            std::swap(t,b);

        pictureQuad = osg::createTexturedQuadGeometry(positionVec,
                                               widthVec,
                                               heightVec,
                                               l, t, r, b);

        stateset = pictureQuad->getOrCreateStateSet();
        stateset->setTextureAttributeAndModes(0,
                        texture.get(),
                        osg::StateAttribute::ON);
    }

    if (!pictureQuad) return 0;

    if (imageStream)
    {
        imageStream->pause();

        OSG_INFO<<"Reading video "<<imageStream->getFileName()<<std::endl;
#if USE_CLIENT_STORAGE_HINT
        // make sure that OSX uses the client storage extension to accelerate peformance where possible.
        texture->setClientStorageHint(true);
#endif
    }


    return pictureQuad;
}



osg::Image* SlideShowConstructor::readImage(const std::string& filename, const ImageData& imageData)
{
    osg::ref_ptr<osgDB::Options> options = _options;
    if (!imageData.options.empty())
    {
        options = _options->cloneOptions();
        options->setOptionString(imageData.options);
    }

    osg::ref_ptr<osg::Image> image;
    osgDB::DirectoryContents filenames;

    std::string foundFile = filename;

    if (imageData.imageSequence)
    {
        // check for wild cards
        if (filename.find('*')!=std::string::npos)
        {
            OSG_INFO<<"Expanding wildcard "<<std::endl;
            filenames = osgDB::expandWildcardsInFilename(filename);
        }
        else
        {
            std::string foundFile = filename;
            osgDB::FileType fileType = osgDB::fileType(foundFile);
            if (fileType == osgDB::FILE_NOT_FOUND)
            {
                foundFile = findFileAndRecordPath(foundFile);
                fileType = osgDB::fileType(foundFile);
            }

            if (fileType == osgDB::DIRECTORY)
            {
                OSG_INFO<<"Reading directory "<<foundFile<<std::endl;

                filenames = osgDB::getDirectoryContents(foundFile);

                // need to insert the directory path in front of the filenames so it's relative to the appropriate directory.
                for(osgDB::DirectoryContents::iterator itr = filenames.begin();
                    itr != filenames.end();
                    ++itr)
                {
                    *itr = foundFile + osgDB::getNativePathSeparator() + *itr;
                }

                // prune any directory entries from the list.
                for(osgDB::DirectoryContents::iterator itr = filenames.begin();
                    itr != filenames.end();
                    )
                {
                    if (osgDB::fileType(*itr)!=osgDB::REGULAR_FILE)
                    {
                        itr = filenames.erase(itr);
                    }
                    else
                    {
                        ++itr;
                    }
                }
            }
            else
            {
                filenames.push_back(foundFile);
            }
        }
    }
    else
    {
        std::string foundFile = filename;
        osgDB::FileType fileType = osgDB::fileType(foundFile);
        if (fileType == osgDB::FILE_NOT_FOUND)
        {
            foundFile = findFileAndRecordPath(foundFile);
            fileType = osgDB::fileType(foundFile);
        }
        filenames.push_back(foundFile);
    }

    if (filenames.empty()) return 0;

    if (filenames.size()==1)
    {
        image = osgDB::readImageFile(filenames[0], options.get());
        if (image.valid()) recordOptionsFilePath(options.get() );
    }
    else
    {
        // make sure images are in alphabetical order.
        std::sort(filenames.begin(), filenames.end(), osgDB::FileNameComparator());

        osg::ref_ptr<osg::ImageSequence> imageSequence = new osg::ImageSequence;

        imageSequence->setMode(imageData.imageSequencePagingMode);

        bool firstLoad = true;

        for(osgDB::DirectoryContents::iterator itr = filenames.begin();
            itr != filenames.end();
            ++itr)
        {
            if (imageSequence->getMode()==osg::ImageSequence::PRE_LOAD_ALL_IMAGES)
            {
                OSG_INFO<<"Attempting to read "<<*itr<<std::endl;
                osg::ref_ptr<osg::Image> loadedImage = osgDB::readImageFile(*itr, options.get());
                if (loadedImage.valid())
                {
                    OSG_INFO<<"Loaded image "<<*itr<<std::endl;
                    imageSequence->addImage(loadedImage.get());
                }
            }
            else
            {
                OSG_INFO<<"Adding filename for load image on demand "<<*itr<<std::endl;
                imageSequence->addImageFile(*itr);
                if (firstLoad)
                {
                    osg::ref_ptr<osg::Image> loadedImage = osgDB::readImageFile(*itr, options.get());
                    if (loadedImage.valid())
                    {
                        imageSequence->addImage(loadedImage.get());
                        firstLoad = false;
                    }
                }
            }
        }

#if 0
        if (imageSequence->getMode()==osg::ImageSequence::PAGE_AND_DISCARD_USED_IMAGES)
        {

            if (_options.valid())
            {
                OSG_NOTICE<<"Object cache usage _options "<<options->getObjectCacheHint()<<std::endl;
            }
            else
            {
                OSG_NOTICE<<"No Object _options assigned"<<std::endl;
            }


            osg::ref_ptr<osgDB::Options> options = _options.valid() ? _options->cloneOptions() : (new osgDB::Options);
            if (!imageData.options.empty())
            {
                options->setOptionString(imageData.options);
            }
            OSG_NOTICE<<"Disabling object cache usage"<<std::endl;
            options->setObjectCacheHint(osgDB::Options::CACHE_NONE);
            imageSequence->setReadOptions(options);
        }
#endif
        if (imageData.duration>0.0)
        {
            imageSequence->setLength(imageData.duration);
        }
        else
        {
            unsigned int maxNum = imageSequence->getNumImageData();
            imageSequence->setLength(double(maxNum)*(1.0/imageData.fps));
        }

        if (imageData.imageSequenceInteractionMode==ImageData::USE_MOUSE_X_POSITION)
        {
            imageSequence->setName("USE_MOUSE_X_POSITION");
        }
        else if (imageData.imageSequenceInteractionMode==ImageData::USE_MOUSE_Y_POSITION)
        {
            imageSequence->setName("USE_MOUSE_Y_POSITION");
        }


        imageSequence->play();

        image = imageSequence;
    }
    if (image.valid())
    {
        if (imageData.delayTime>0.0) image->setUserValue("delay",imageData.delayTime);
        if (imageData.startTime>0.0) image->setUserValue("start",imageData.startTime);
        if (imageData.stopTime>0.0) image->setUserValue("stop",imageData.stopTime);
    }
    return image.release();
}

struct VolumeCallback : public osg::NodeCallback
{
public:
    VolumeCallback(osg::ImageStream* movie, const std::string& str):
        _movie(movie),
        _source(str) {}

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        PropertyReader pr(nv->getNodePath(), _source);

        float volume=0.0f;
        pr>>volume;

        if (pr.ok())
        {
            OSG_NOTICE<<"VolumeCallback : volume="<<volume<<", from "<<_source<<std::endl;
            _movie->setVolume(volume);
        }
        else
        {
            OSG_NOTICE<<"Problem in reading, VolumeCallback : volume="<<volume<<std::endl;
        }


        // note, callback is responsible for scenegraph traversal so
        // they must call traverse(node,nv) to ensure that the
        // scene graph subtree (and associated callbacks) are traversed.
        traverse(node, nv);
    }

protected:

    osg::ref_ptr<osg::ImageStream> _movie;
    std::string  _source;
};

void SlideShowConstructor::setUpMovieVolume(osg::Node* subgraph, osg::ImageStream* imageStream, const ImageData& imageData)
{
    if (containsPropertyReference(imageData.volume))
    {
        subgraph->addUpdateCallback(new VolumeCallback(imageStream, imageData.volume));
    }
    else
    {
        float volume;
        std::istringstream sstream(imageData.volume);
        sstream>>volume;

        if (!sstream.fail())
        {
            OSG_NOTICE<<"Setting volume "<<volume<<std::endl;
            imageStream->setVolume( volume );
        }
        else
        {
            OSG_NOTICE<<"Invalid volume setting: "<<imageData.volume<<std::endl;
        }
    }
}

void SlideShowConstructor::addImage(const std::string& filename, const PositionData& positionData, const ImageData& imageData)
{

    osg::ref_ptr<osgVolume::Volume> volume;
    osg::ref_ptr<osgVolume::VolumeTile> tile;
    osg::ref_ptr<osgVolume::ImageLayer> layer;

    osg::ref_ptr<osg::Image> image = readImage(filename, imageData);
    if (!image) return;

    bool isImageTranslucent = false;

    osg::ImageStream* imageStream = dynamic_cast<osg::ImageStream*>(image.get());
    if (imageStream)
    {
        imageStream->setLoopingMode(imageData.loopingMode);

        isImageTranslucent = imageStream->getPixelFormat()==GL_RGBA ||
                             imageStream->getPixelFormat()==GL_BGRA;

    }
    else
    {
        isImageTranslucent = image->isImageTranslucent();
    }

    if (imageData.blendingHint==ImageData::ON)
    {
        isImageTranslucent = true;
    }
    else if (imageData.blendingHint==ImageData::OFF)
    {
        isImageTranslucent = false;
    }


    float s = image->s();
    float t = image->t();

    float sx = imageData.region_in_pixel_coords ? 1.0f : s;
    float sy = imageData.region_in_pixel_coords ? 1.0f : t;

    float x1 = imageData.region[0]*sx;
    float y1 = imageData.region[1]*sy;
    float x2 = imageData.region[2]*sx;
    float y2 = imageData.region[3]*sy;

    float aspectRatio = (y2-y1)/(x2-x1);

    float image_width = _slideWidth*positionData.scale.x();
    float image_height = image_width*aspectRatio*positionData.scale.y()/positionData.scale.x();
    float offset = 0.0f;

    osg::Vec3 pos = computePositionInModelCoords(positionData);
    osg::Vec3 image_local_pos = osg::Vec3(-image_width*0.5f+offset,-offset,-image_height*0.5f-offset);
    osg::Vec3 image_pos = positionData.autoRotate ? image_local_pos : (pos+image_local_pos);


    bool usedTextureRectangle = false;
    osg::Geometry* pictureQuad = createTexturedQuadGeometry(image_pos, positionData.rotate, image_width, image_height, image.get(), usedTextureRectangle);
    osg::StateSet* pictureStateSet = pictureQuad->getOrCreateStateSet();

    attachTexMat(pictureStateSet, imageData, s, t, usedTextureRectangle);

    osg::Node* subgraph = 0;

    if (positionData.autoRotate)
    {
        osg::Billboard* picture = new osg::Billboard;
        picture->setMode(osg::Billboard::POINT_ROT_EYE);
        picture->setNormal(osg::Vec3(0.0f,-1.0f,0.0f));
        picture->setAxis(osg::Vec3(0.0f,0.0f,1.0f));
        picture->addDrawable(pictureQuad,pos);
        subgraph = picture;
    }
    else
    {
        osg::Geode* picture = new osg::Geode;
        picture->addDrawable(pictureQuad);
        subgraph = picture;
    }

    // attach any meterial animation.
    if (positionData.requiresMaterialAnimation())
        subgraph = attachMaterialAnimation(subgraph,positionData);


    if (isImageTranslucent)
    {
        SetToTransparentBin sttb;
        subgraph->accept(sttb);
        pictureStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
    }


    if (imageStream && !imageData.volume.empty())
    {
        setUpMovieVolume(subgraph, imageStream, imageData);
    }

    osg::ImageSequence* imageSequence = dynamic_cast<osg::ImageSequence*>(image.get());
    if (imageSequence)
    {
        if (imageData.imageSequenceInteractionMode==ImageData::USE_MOUSE_X_POSITION)
        {
            subgraph->setUpdateCallback(new osgPresentation::ImageSequenceUpdateCallback(imageSequence, _propertyManager.get(), "mouse.x_normalized"));
        }
        else if (imageData.imageSequenceInteractionMode==ImageData::USE_MOUSE_Y_POSITION)
        {
            subgraph->setUpdateCallback(new osgPresentation::ImageSequenceUpdateCallback(imageSequence, _propertyManager.get(), "mouse.y_normalized"));
        }
    }

    // attached any rotation
    if (positionData.rotation[0]!=0.0)
    {
        osg::MatrixTransform* animation_transform = new osg::MatrixTransform;
        animation_transform->setDataVariance(osg::Object::DYNAMIC);
        animation_transform->setUpdateCallback(
            new osgUtil::TransformCallback(subgraph->getBound().center(),
                                           osg::Vec3(positionData.rotation[1],positionData.rotation[2],positionData.rotation[3]),
                                           osg::DegreesToRadians(positionData.rotation[0])));

        animation_transform->addChild(subgraph);

        subgraph = animation_transform;
    }

    // attached any animation
    osg::AnimationPathCallback* animation = getAnimationPathCallback(positionData);
    if (animation)
    {
        OSG_INFO<<"Have animation path for image"<<std::endl;

        osg::BoundingSphere::vec_type pivot = positionData.absolute_path ?
                osg::BoundingSphere::vec_type(0.0f,0.0f,0.0f) :
                subgraph->getBound().center();

        osg::PositionAttitudeTransform* animation_transform = new osg::PositionAttitudeTransform;
        animation_transform->setDataVariance(osg::Object::DYNAMIC);
        animation_transform->setPivotPoint(pivot);
        animation->setPivotPoint(pivot);

        animation_transform->setUpdateCallback(animation);

        animation_transform->setReferenceFrame(positionData.absolute_path ?
                                                    osg::Transform::ABSOLUTE_RF:
                                                    osg::Transform::RELATIVE_RF);

        animation_transform->addChild(subgraph);

        subgraph = animation_transform;
    }

    if (positionData.hud)
    {
        HUDTransform* hudTransform = new HUDTransform(_hudSettings.get());
        hudTransform->addChild(subgraph);

        subgraph = hudTransform;
    }

    addToCurrentLayer(subgraph);
}

void SlideShowConstructor::addStereoImagePair(const std::string& filenameLeft, const ImageData& imageDataLeft, const std::string& filenameRight, const ImageData& imageDataRight,const PositionData& positionData)
{
    osg::ref_ptr<osg::Image> imageLeft = readImage(filenameLeft, imageDataLeft);
    osg::ref_ptr<osg::Image> imageRight = (filenameRight==filenameLeft) ? imageLeft.get() : readImage(filenameRight, imageDataRight);

    if (!imageLeft && !imageRight) return;

    bool isImageTranslucent = false;

    osg::ImageStream* imageStreamLeft = dynamic_cast<osg::ImageStream*>(imageLeft.get());
    if (imageStreamLeft)
    {
        imageStreamLeft->setLoopingMode(imageDataLeft.loopingMode);
        isImageTranslucent = imageStreamLeft->getPixelFormat()==GL_RGBA ||
                             imageStreamLeft->getPixelFormat()==GL_BGRA;
    }
    else
    {
        isImageTranslucent = imageLeft->isImageTranslucent();
    }

    osg::ImageStream* imageStreamRight = dynamic_cast<osg::ImageStream*>(imageRight.get());
    if (imageStreamRight)
    {
        imageStreamRight->setLoopingMode(imageDataRight.loopingMode);
        if (!isImageTranslucent)
        {
            isImageTranslucent = imageStreamRight->getPixelFormat()==GL_RGBA ||
                                imageStreamRight->getPixelFormat()==GL_BGRA;
        }
    }
    else if (!isImageTranslucent)
    {
        isImageTranslucent = imageRight->isImageTranslucent();
    }

    if (imageDataLeft.blendingHint==ImageData::ON || imageDataRight.blendingHint==ImageData::ON)
    {
        isImageTranslucent = true;
    }
    else if (imageDataLeft.blendingHint==ImageData::OFF || imageDataRight.blendingHint==ImageData::OFF)
    {
        isImageTranslucent = false;
    }


    float s = imageLeft->s();
    float t = imageLeft->t();


    float sx = imageDataLeft.region_in_pixel_coords ? 1.0f : s;
    float sy = imageDataLeft.region_in_pixel_coords ? 1.0f : t;

    float x1 = imageDataLeft.region[0]*sx;
    float y1 = imageDataLeft.region[1]*sy;
    float x2 = imageDataLeft.region[2]*sx;
    float y2 = imageDataLeft.region[3]*sy;

    float aspectRatio = (y2-y1)/(x2-x1);

    float image_width = _slideWidth*positionData.scale.x();
    float image_height = image_width*aspectRatio*positionData.scale.y()/positionData.scale.x();

    float offset = 0.0f;

    bool usedTextureRectangle = false;

    osg::Vec3 pos = computePositionInModelCoords(positionData);
    osg::Vec3 image_local_pos = osg::Vec3(-image_width*0.5f+offset,-offset,-image_height*0.5f-offset);
    osg::Vec3 image_pos = positionData.autoRotate ? image_local_pos : (pos+image_local_pos);


    osg::Node* pictureLeft = 0;
    {
        osg::Geometry* pictureLeftQuad = createTexturedQuadGeometry(image_pos, positionData.rotate, image_width,image_height,imageLeft.get(),usedTextureRectangle);
        osg::StateSet* pictureLeftStateSet = pictureLeftQuad->getOrCreateStateSet();

        if (isImageTranslucent)
        {
            pictureLeftStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
        }

        attachTexMat(pictureLeftStateSet, imageDataLeft, s, t, usedTextureRectangle);

        if (positionData.autoRotate)
        {
            osg::Billboard* billboard = new osg::Billboard;
            billboard->setMode(osg::Billboard::POINT_ROT_EYE);
            billboard->setNormal(osg::Vec3(0.0f,-1.0f,0.0f));
            billboard->setAxis(osg::Vec3(0.0f,0.0f,1.0f));
            billboard->addDrawable(pictureLeftQuad,pos);
            pictureLeft = billboard;
        }
        else
        {
            osg::Geode* geode = new osg::Geode;
            geode->addDrawable(pictureLeftQuad);
            pictureLeft = geode;
        }

        pictureLeft->setNodeMask(_leftEyeMask);
    }

    osg::Node* pictureRight = 0;
    {
        osg::Geometry* pictureRightQuad = createTexturedQuadGeometry(image_pos, positionData.rotate, image_width,image_height,imageRight.get(),usedTextureRectangle);
        osg::StateSet* pictureRightStateSet = pictureRightQuad->getOrCreateStateSet();

        if (isImageTranslucent)
        {
            pictureRightStateSet->setMode(GL_BLEND, osg::StateAttribute::ON);
        }

        attachTexMat(pictureRightStateSet, imageDataRight, s, t, usedTextureRectangle);

        if (positionData.autoRotate)
        {
            osg::Billboard* billboard = new osg::Billboard;
            billboard->setMode(osg::Billboard::POINT_ROT_EYE);
            billboard->setNormal(osg::Vec3(0.0f,-1.0f,0.0f));
            billboard->setAxis(osg::Vec3(0.0f,0.0f,1.0f));
            billboard->addDrawable(pictureRightQuad,pos);
            pictureRight = billboard;
        }
        else
        {
            osg::Geode* geode = new osg::Geode;
            geode->addDrawable(pictureRightQuad);
            pictureRight = geode;
        }

        pictureRight->setNodeMask(_rightEyeMask);
    }

    osg::Group* subgraph = new osg::Group;
    subgraph->addChild(pictureLeft);
    subgraph->addChild(pictureRight);

    if (imageStreamLeft && !imageDataLeft.volume.empty())
    {
        setUpMovieVolume(subgraph, imageStreamLeft, imageDataLeft);
    }

    if (imageStreamRight && !imageDataRight.volume.empty())
    {
        setUpMovieVolume(subgraph, imageStreamRight, imageDataRight);
    }

    osg::ImageSequence* imageSequence = dynamic_cast<osg::ImageSequence*>(imageLeft.get());
    if (imageSequence)
    {
        if (imageDataLeft.imageSequenceInteractionMode==ImageData::USE_MOUSE_X_POSITION)
        {
            subgraph->setUpdateCallback(new osgPresentation::ImageSequenceUpdateCallback(imageSequence, _propertyManager.get(), "mouse.x_normalized"));
        }
        else if (imageDataLeft.imageSequenceInteractionMode==ImageData::USE_MOUSE_Y_POSITION)
        {
            subgraph->setUpdateCallback(new osgPresentation::ImageSequenceUpdateCallback(imageSequence, _propertyManager.get(), "mouse.y_normalized"));
        }
    }

    // attach any meterial animation.
    if (positionData.requiresMaterialAnimation())
        subgraph = attachMaterialAnimation(subgraph,positionData)->asGroup();

    if (isImageTranslucent)
    {
        SetToTransparentBin sttb;
        subgraph->accept(sttb);
    }

    // attached any rotation
    if (positionData.rotation[0]!=0.0)
    {
        osg::MatrixTransform* animation_transform = new osg::MatrixTransform;
        animation_transform->setDataVariance(osg::Object::DYNAMIC);
        animation_transform->setUpdateCallback(
            new osgUtil::TransformCallback(subgraph->getBound().center(),
                                           osg::Vec3(positionData.rotation[1],positionData.rotation[2],positionData.rotation[3]),
                                           osg::DegreesToRadians(positionData.rotation[0])));

        animation_transform->addChild(subgraph);

        subgraph = animation_transform;
    }

    // attached any animation
    osg::AnimationPathCallback* animation = getAnimationPathCallback(positionData);
    if (animation)
    {
        OSG_INFO<<"Have animation path for image"<<std::endl;

        osg::BoundingSphere::vec_type pivot = positionData.absolute_path ?
                osg::BoundingSphere::vec_type(0.0f,0.0f,0.0f) :
                subgraph->getBound().center();

        osg::PositionAttitudeTransform* animation_transform = new osg::PositionAttitudeTransform;
        animation_transform->setDataVariance(osg::Object::DYNAMIC);
        animation_transform->setPivotPoint(pivot);
        animation->setPivotPoint(pivot);

        animation_transform->setUpdateCallback(animation);
        animation_transform->setReferenceFrame(positionData.absolute_path ?
                                                    osg::Transform::ABSOLUTE_RF:
                                                    osg::Transform::RELATIVE_RF);

        animation_transform->addChild(subgraph);

        subgraph = animation_transform;
    }

    if (positionData.hud)
    {
        HUDTransform* hudTransform = new HUDTransform(_hudSettings.get());
        hudTransform->addChild(subgraph);

        subgraph = hudTransform;
    }

    addToCurrentLayer(subgraph);
}

void SlideShowConstructor::addGraph(const std::string& contents, const PositionData& positionData, const ImageData& imageData)
{
    static int s_count=0;

    if (contents.empty()) return;

    std::string tmpDirectory("/tmp/");

    std::string filename = contents;
    std::string ext = osgDB::getFileExtension(contents);
    if (ext.empty())
    {
        std::stringstream dotFileNameStream;
        dotFileNameStream << tmpDirectory<<"graph_"<<s_count<<std::string(".dot");
        filename = dotFileNameStream.str();

        // write out the string to the temporary file.
        std::ofstream fout(filename.c_str());
        fout<<contents.c_str();
    }

    std::stringstream svgFileNameStream;
    svgFileNameStream << tmpDirectory<<osgDB::getStrippedName(filename)<<s_count<<std::string(".svg");
    std::string tmpSvgFileName(svgFileNameStream.str());
    std::string dotFileName = filename;

    if (osgDB::getFileExtension(filename)=="dot")
    {
        dotFileName = filename;
    }
    else
    {
        osg::ref_ptr<osg::Node> model = osgDB::readNodeFile(filename, _options.get());
        if (!model) return;

        dotFileName = tmpDirectory+osgDB::getStrippedName(filename)+std::string(".dot");

        osg::ref_ptr<osgDB::Options> opts = _options.valid() ? _options->cloneOptions() : (new osgDB::Options);
        if (!imageData.options.empty())
        {
            opts->setOptionString(imageData.options);
        }
        opts->setObjectCacheHint(osgDB::Options::CACHE_NONE);

        osgDB::writeNodeFile(*model, dotFileName, opts.get());
    }

    std::stringstream command;
    command<<"dot -Tsvg "<<dotFileName<<" -o "<<tmpSvgFileName;
    int result = system(command.str().c_str());
    if (result==0)
    {
        osg::ref_ptr<osgDB::Options> previousOptions = _options;

        // switch off cache so we make sure that we re-read the generated svg each time.
        _options = _options.valid() ? _options->cloneOptions() : (new osgDB::Options);
        _options->setObjectCacheHint(osgDB::Options::CACHE_NONE);

        addImage(tmpSvgFileName, positionData, imageData);

        _options = previousOptions;

        ++s_count;
    }
    else OSG_NOTICE<<"Error: SlideShowConstructor::addGraph() system("<<command.str()<<") failed with return "<<result<<std::endl;
}


void SlideShowConstructor::addVNC(const std::string& hostname, const PositionData& positionData, const ImageData& imageData, const std::string& password)
{
    if (!password.empty())
    {
        OSG_NOTICE<<"Setting password"<<std::endl;
        if (!osgDB::Registry::instance()->getAuthenticationMap()) osgDB::Registry::instance()->setAuthenticationMap(new osgDB::AuthenticationMap);
        osgDB::Registry::instance()->getAuthenticationMap()->addAuthenticationDetails(hostname, new osgDB::AuthenticationDetails("", password));
    }

    addInteractiveImage(hostname+".vnc", positionData, imageData);
}

void SlideShowConstructor::addBrowser(const std::string& url, const PositionData& positionData, const ImageData& imageData)
{
    addInteractiveImage(url+".gecko", positionData, imageData);
}

void SlideShowConstructor::addPDF(const std::string& filename, const PositionData& positionData, const ImageData& imageData)
{
    addInteractiveImage(filename, positionData, imageData);
}

class SetPageCallback: public LayerCallback
{
public:
    SetPageCallback(osgWidget::PdfImage* pdfImage, int pageNum):
        _pdfImage(pdfImage),
        _pageNum(pageNum)
    {
    }

    virtual void operator() (osg::Node*) const
    {
        OSG_INFO<<"PDF Page to be updated "<<_pageNum<<std::endl;

        if (_pdfImage.valid() && _pdfImage->getPageNum()!=_pageNum)
        {
            _pdfImage->page(_pageNum);
        }
    }

    osg::observer_ptr<osgWidget::PdfImage> _pdfImage;
    int _pageNum;
};


osg::Image* SlideShowConstructor::addInteractiveImage(const std::string& filename, const PositionData& positionData, const ImageData& imageData)
{
    osg::ref_ptr<osgDB::Options> options = _options;
    if (!imageData.options.empty())
    {
        options = _options->cloneOptions();
        options->setOptionString(imageData.options);
    }

    osg::Image* image = osgDB::readImageFile(filename, options.get());

    OSG_INFO<<"addInteractiveImage("<<filename<<") "<<image<<std::endl;


    if (!image) return 0;

    float s = image->s();
    float t = image->t();

    float sx = imageData.region_in_pixel_coords ? 1.0f : s;
    float sy = imageData.region_in_pixel_coords ? 1.0f : t;

    float x1 = imageData.region[0]*sx;
    float y1 = imageData.region[1]*sy;
    float x2 = imageData.region[2]*sx;
    float y2 = imageData.region[3]*sy;

    float aspectRatio = (y2-y1)/(x2-x1);

    float image_width = _slideWidth*positionData.scale.x();
    float image_height = image_width*aspectRatio*positionData.scale.y()/positionData.scale.x();
    float offset = 0.0f;

    osg::Vec3 pos = computePositionInModelCoords(positionData);
    osg::Vec3 image_local_pos = osg::Vec3(-image_width*0.5f+offset,-offset,-image_height*0.5f-offset);
    osg::Vec3 image_pos = positionData.autoRotate ? image_local_pos : (pos+image_local_pos);

    bool usedTextureRectangle = false;
    osg::Geometry* pictureQuad = createTexturedQuadGeometry(image_pos, positionData.rotate, image_width, image_height, image, usedTextureRectangle);

    osg::ref_ptr<osgViewer::InteractiveImageHandler> handler = new osgViewer::InteractiveImageHandler(image);
    pictureQuad->setEventCallback(handler.get());
    pictureQuad->setCullCallback(handler.get());

    osg::StateSet* pictureStateSet = pictureQuad->getOrCreateStateSet();

    attachTexMat(pictureStateSet, imageData, s, t, usedTextureRectangle);

    pictureStateSet->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    osg::Node* subgraph = 0;

    if (positionData.autoRotate)
    {
        osg::Billboard* picture = new osg::Billboard;
        picture->setMode(osg::Billboard::POINT_ROT_EYE);
        picture->setNormal(osg::Vec3(0.0f,-1.0f,0.0f));
        picture->setAxis(osg::Vec3(0.0f,0.0f,1.0f));
        picture->addDrawable(pictureQuad,pos);
        subgraph = picture;
    }
    else
    {
        osg::Geode* picture = new osg::Geode;
        picture->addDrawable(pictureQuad);
        subgraph = picture;
    }

    // attach any meterial animation.
    if (positionData.requiresMaterialAnimation())
        subgraph = attachMaterialAnimation(subgraph,positionData);


    // attached any rotation
    if (positionData.rotation[0]!=0.0)
    {
        osg::MatrixTransform* animation_transform = new osg::MatrixTransform;
        animation_transform->setDataVariance(osg::Object::DYNAMIC);
        animation_transform->setUpdateCallback(
            new osgUtil::TransformCallback(subgraph->getBound().center(),
                                           osg::Vec3(positionData.rotation[1],positionData.rotation[2],positionData.rotation[3]),
                                           osg::DegreesToRadians(positionData.rotation[0])));

        animation_transform->addChild(subgraph);

        subgraph = animation_transform;
    }


    // attached any animation
    osg::AnimationPathCallback* animation = getAnimationPathCallback(positionData);
    if (animation)
    {
        OSG_INFO<<"Have animation path for image"<<std::endl;

        osg::BoundingSphere::vec_type pivot = positionData.absolute_path ?
                osg::BoundingSphere::vec_type(0.0f,0.0f,0.0f) :
                subgraph->getBound().center();

        osg::PositionAttitudeTransform* animation_transform = new osg::PositionAttitudeTransform;
        animation_transform->setDataVariance(osg::Object::DYNAMIC);
        animation_transform->setPivotPoint(pivot);
        animation->setPivotPoint(pivot);

        animation_transform->setUpdateCallback(animation);

        animation_transform->setReferenceFrame(positionData.absolute_path ?
                                                    osg::Transform::ABSOLUTE_RF:
                                                    osg::Transform::RELATIVE_RF);

        animation_transform->addChild(subgraph);

        subgraph = animation_transform;
    }

    if (positionData.hud)
    {
        HUDTransform* hudTransform = new HUDTransform(_hudSettings.get());
        hudTransform->addChild(subgraph);

        subgraph = hudTransform;
    }

    addToCurrentLayer(subgraph);

    osgWidget::PdfImage* pdfImage = dynamic_cast<osgWidget::PdfImage*>(image);
    if (pdfImage && imageData.page>=0)
    {
        getOrCreateLayerAttributes(_currentLayer.get())->addEnterCallback(new SetPageCallback(pdfImage, imageData.page));

        OSG_INFO<<"Setting pdf page num "<<imageData.page<<std::endl;
        pdfImage->setBackgroundColor(imageData.backgroundColor);
        pdfImage->page(imageData.page);

        if (imageData.backgroundColor.a()<1.0f)
        {
            SetToTransparentBin sttb;
            subgraph->accept(sttb);
        }


    }


    return image;
}

std::string SlideShowConstructor::findFileAndRecordPath(const std::string& filename)
{
    std::string foundFile = osgDB::findDataFile(filename, _options.get());
    if (foundFile.empty()) return filename;

    OSG_INFO<<"foundFile "<<foundFile<<std::endl;

    std::string path = osgDB::getFilePath(foundFile);
    if (!path.empty() && _filePathData.valid())
    {
        osgDB::FilePathList::iterator itr = std::find(_filePathData->filePathList.begin(),_filePathData->filePathList.end(),path);
        if (itr==_filePathData->filePathList.end())
        {
            OSG_INFO<<"New path to record "<<path<<std::endl;
            _filePathData->filePathList.push_front(path);
        }
    }

    return foundFile;

}


struct ClipRegionCallback : public osg::NodeCallback
{
public:
    ClipRegionCallback(const osg::Matrixd& originalMatrix, const std::string& str):
        _matrix(originalMatrix),
        _source(str) {}

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        osg::MatrixTransform* transform = dynamic_cast<osg::MatrixTransform*>(node);
        if (transform)
        {
            PropertyReader pr(nv->getNodePath(), _source);

            float xMin=0.0;
            float yMin=0.0;
            float zMin=0.0;
            float xMax=1.0;
            float yMax=1.0;
            float zMax=1.0;

            pr>>xMin>>yMin>>zMin>>xMax>>yMax>>zMax;

            if (pr.ok())
            {
                OSG_NOTICE<<"ClipRegionCallback : xMin="<<xMin<<", yMin="<<yMin<<", zMin="<<zMin<<", xMax="<<xMax<<", yMax="<<yMax<<", zMax="<<zMax<<std::endl;
            }
            else
            {
                OSG_NOTICE<<"Problem in reading, ClipRegionCallback : xMin="<<xMin<<", yMin="<<yMin<<", zMin="<<zMin<<", xMax="<<xMax<<", yMax="<<yMax<<", zMax="<<zMax<<std::endl;
            }

            osg::Matrixd tm = osg::Matrix::scale(xMax-xMin, yMax-yMin, zMax-zMin) *
                              osg::Matrix::translate(xMin,yMin,zMin);

            transform->setMatrix(tm * _matrix);

        }
        else
        {
            OSG_NOTICE<<"ClipRegionCallback not attached to MatrixTransform, unable to update any values."<<std::endl;
        }

        // note, callback is responsible for scenegraph traversal so
        // they must call traverse(node,nv) to ensure that the
        // scene graph subtree (and associated callbacks) are traversed.
        traverse(node, nv);
    }

protected:

    osg::Matrixd _matrix;
    std::string  _source;
};


void SlideShowConstructor::addModel(const std::string& filename, const PositionData& positionData, const ModelData& modelData)
{
    OSG_INFO<<"SlideShowConstructor::addModel("<<filename<<")"<<std::endl;

    osg::ref_ptr<osgDB::Options> options = _options;
    if (!modelData.options.empty())
    {
        options = _options->cloneOptions();
        options->setOptionString(modelData.options);
    }

    osg::ref_ptr<osg::Node> subgraph;

    if (filename=="sphere")
    {
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(new osg::ShapeDrawable(new osg::Sphere));

        subgraph = geode;
    }
    else if (filename=="box")
    {
        osg::Geode* geode = new osg::Geode;
        geode->addDrawable(new osg::ShapeDrawable(new osg::Box));

        subgraph = geode;
    }
    else
    {
        subgraph = osgDB::readNodeFile(filename, options.get());
        if (subgraph) recordOptionsFilePath(options.get());
    }

    if (!modelData.region.empty())
    {
        osg::ref_ptr<osg::ClipNode> clipnode = new osg::ClipNode;
        clipnode->createClipBox(osg::BoundingBox(0.0,0.0,0.0,1.0,1.0,1.0),0);
        clipnode->setCullingActive(false);

        osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform;
        transform->addChild(clipnode.get());

        osg::ref_ptr<osg::Group> group = new osg::Group;
        group->addChild(subgraph.get());
        group->addChild(transform.get());

        //clipnode->setStateSetModes(*(group->getOrCreateStateSet()), osg::StateAttribute::ON);
        group->setStateSet(clipnode->getStateSet());

        osg::ComputeBoundsVisitor cbbv(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN);
        subgraph->accept(cbbv);
        osg::BoundingBox bb = cbbv.getBoundingBox();
        double width = bb.xMax()-bb.xMin();
        double length = bb.yMax()-bb.yMin();
        double height = bb.zMax()-bb.zMin();

        osg::Matrixd matrix = osg::Matrixd::translate(-0.5,-0.5,-0.5)*osg::Matrixd::scale(width,length,height)*osg::Matrixd::translate(bb.center());
        transform->setMatrix(matrix);

        if (containsPropertyReference(modelData.region))
        {
            transform->addUpdateCallback(new ClipRegionCallback(matrix, modelData.region));
        }
        else
        {
            double region[6];
            std::istringstream sstream(modelData.region);
            sstream>>region[0]>>region[1]>>region[2]>>region[3]>>region[4]>>region[5];

            osg::Matrix tm = osg::Matrix::scale(region[3]-region[0], region[4]-region[1], region[5]-region[2]) *
                             osg::Matrix::translate(region[0],region[1],region[2]);

            transform->setMatrix( tm * matrix );
        }

        subgraph = group;

        osgDB::writeNodeFile(*subgraph, "output.osgt");

    }


    if (subgraph.valid())
    {
        addModel(subgraph.get(), positionData, modelData);
    }

    OSG_INFO<<"end of SlideShowConstructor::addModel("<<filename<<")"<<std::endl<<std::endl;

}

void SlideShowConstructor::addModel(osg::Node* subgraph, const PositionData& positionData, const ModelData& modelData)
{
    osg::Object::DataVariance defaultMatrixDataVariance = osg::Object::DYNAMIC; // STATIC

    if (!modelData.effect.empty())
    {
        if (modelData.effect=="SpecularHighlights" || modelData.effect=="glossy")
        {
            osgFX::SpecularHighlights* specularHighlights = new osgFX::SpecularHighlights;
            specularHighlights->setTextureUnit(1);
            specularHighlights->addChild(subgraph);
            subgraph = specularHighlights;
        }
    }


    if (positionData.frame==SLIDE)
    {
        osg::Vec3 pos = convertSlideToModel(positionData.position);

        const osg::BoundingSphere& bs = subgraph->getBound();
        float slide_scale = _slideHeight*(1.0f-positionData.position.z())*0.7f/bs.radius();

        osg::MatrixTransform* transform = new osg::MatrixTransform;
        transform->setDataVariance(defaultMatrixDataVariance);
        transform->setMatrix(osg::Matrix::translate(-bs.center())*
                             osg::Matrix::scale(positionData.scale.x()*slide_scale, positionData.scale.y()*slide_scale ,positionData.scale.z()*slide_scale)*
                             osg::Matrix::rotate(osg::DegreesToRadians(positionData.rotate[0]),positionData.rotate[1],positionData.rotate[2],positionData.rotate[3])*
                             osg::Matrix::translate(pos));


        transform->setStateSet(createTransformStateSet());
        transform->addChild(subgraph);

        subgraph = transform;

    }
    else
    {
        osg::Matrix matrix(osg::Matrix::scale(1.0f/positionData.scale.x(),1.0f/positionData.scale.y(),1.0f/positionData.scale.z())*
                           osg::Matrix::rotate(osg::DegreesToRadians(positionData.rotate[0]),positionData.rotate[1],positionData.rotate[2],positionData.rotate[3])*
                           osg::Matrix::translate(positionData.position));

        osg::MatrixTransform* transform = new osg::MatrixTransform;
        transform->setDataVariance(defaultMatrixDataVariance);
        transform->setMatrix(osg::Matrix::inverse(matrix));

        OSG_INFO<<"Position Matrix "<<transform->getMatrix()<<std::endl;

        transform->addChild(subgraph);

        subgraph = transform;
    }

    float referenceSizeRatio = 0.707;
    float referenceSize = subgraph->getBound().radius() * referenceSizeRatio;


    // attach any meterial animation.
    if (positionData.requiresMaterialAnimation())
        subgraph = attachMaterialAnimation(subgraph,positionData);

    // attached any rotation
    if (positionData.rotation[0]!=0.0)
    {
        osg::MatrixTransform* animation_transform = new osg::MatrixTransform;
        animation_transform->setDataVariance(osg::Object::DYNAMIC);
        animation_transform->setUpdateCallback(
            new osgUtil::TransformCallback(subgraph->getBound().center(),
                                           osg::Vec3(positionData.rotation[1],positionData.rotation[2],positionData.rotation[3]),
                                           osg::DegreesToRadians(positionData.rotation[0])));

        animation_transform->addChild(subgraph);

        OSG_INFO<<"Rotation Matrix "<<animation_transform->getMatrix()<<std::endl;

        subgraph = animation_transform;
    }


    // attached any animation
    osg::AnimationPathCallback* animation = getAnimationPathCallback(positionData);
    if (animation)
    {
        OSG_INFO<<"Have animation path for model"<<std::endl;

        osg::BoundingSphere::vec_type pivot = positionData.absolute_path ?
            osg::BoundingSphere::vec_type(0.0f,0.0f,0.0f) :
            subgraph->getBound().center();

        osg::AnimationPath* path = animation->getAnimationPath();
        if (positionData.animation_name=="wheel" && (path->getTimeControlPointMap()).size()>=2)
        {
            OSG_INFO<<"****  Need to handle special wheel animation"<<std::endl;

            osg::AnimationPath::TimeControlPointMap& controlPoints = path->getTimeControlPointMap();

            osg::AnimationPath::TimeControlPointMap::iterator curr_itr = controlPoints.begin();
            osg::AnimationPath::TimeControlPointMap::iterator prev_itr=curr_itr;
            ++curr_itr;

            osg::AnimationPath::ControlPoint* prev_cp = &(prev_itr->second);
            osg::AnimationPath::ControlPoint* curr_cp = &(curr_itr->second);

            float totalLength = 0;
            float rotation_y_axis = 0;
            osg::Vec3 delta_position = curr_cp->getPosition() - prev_cp->getPosition();
            float rotation_z_axis = atan2f(delta_position.y(),delta_position.x());

            osg::Quat quat_y_axis,quat_z_axis,quat_combined;

            quat_y_axis.makeRotate(rotation_y_axis,0.0f,1.0f,0.0f);
            quat_z_axis.makeRotate(rotation_z_axis,0.0f,0.0f,1.0f);
            quat_combined = quat_y_axis*quat_z_axis;

            // set first rotation.
            prev_cp->setRotation(quat_combined);

            for(;
                curr_itr!=controlPoints.end();
                ++curr_itr)
            {
                prev_cp = &(prev_itr->second);
                curr_cp = &(curr_itr->second);

                delta_position = curr_cp->getPosition() - prev_cp->getPosition();

                totalLength += delta_position.length();

                // rolling - rotation about the y axis.
                rotation_y_axis = totalLength/referenceSize;

                // direction - rotation about the z axis.
                rotation_z_axis = atan2f(delta_position.y(),delta_position.x());

                OSG_INFO<<" rotation_y_axis="<<rotation_y_axis<<" rotation_z_axis="<<rotation_z_axis<<std::endl;

                quat_y_axis.makeRotate(rotation_y_axis,0.0f,1.0f,0.0f);
                quat_z_axis.makeRotate(rotation_z_axis,0.0f,0.0f,1.0f);
                quat_combined = quat_y_axis*quat_z_axis;

                curr_cp->setRotation(quat_combined);

                prev_itr = curr_itr;

            }

        }


        osg::PositionAttitudeTransform* animation_transform = new osg::PositionAttitudeTransform;
        animation_transform->setDataVariance(osg::Object::DYNAMIC);
        animation_transform->setPivotPoint(pivot);
        animation->setPivotPoint(pivot);
        animation_transform->setUpdateCallback(animation);

        animation_transform->setReferenceFrame(positionData.absolute_path ?
                                                    osg::Transform::ABSOLUTE_RF:
                                                    osg::Transform::RELATIVE_RF);

        animation_transform->addChild(subgraph);

        subgraph = animation_transform;
    }

    findImageStreamsAndAddCallbacks(subgraph);

    addToCurrentLayer(subgraph);
}

class DraggerVolumeTileCallback : public osgManipulator::DraggerCallback
{
public:

    DraggerVolumeTileCallback(osgVolume::VolumeTile* volume, osgVolume::Locator* locator):
        _volume(volume),
        _locator(locator) {}


    virtual bool receive(const osgManipulator::MotionCommand& command);


    osg::observer_ptr<osgVolume::VolumeTile>    _volume;
    osg::ref_ptr<osgVolume::Locator>            _locator;

    osg::Matrix _startMotionMatrix;

    osg::Matrix _localToWorld;
    osg::Matrix _worldToLocal;

};

bool DraggerVolumeTileCallback::receive(const osgManipulator::MotionCommand& command)
{
    if (!_locator) return false;

    switch (command.getStage())
    {
        case osgManipulator::MotionCommand::START:
        {
            // Save the current matrix
            _startMotionMatrix = _locator->getTransform();

            // Get the LocalToWorld and WorldToLocal matrix for this node.
            osg::NodePath nodePathToRoot;
            osgManipulator::computeNodePathToRoot(*_volume,nodePathToRoot);
            _localToWorld = _startMotionMatrix * osg::computeLocalToWorld(nodePathToRoot);
            _worldToLocal = osg::Matrix::inverse(_localToWorld);

            return true;
        }
        case osgManipulator::MotionCommand::MOVE:
        {
            // Transform the command's motion matrix into local motion matrix.
            osg::Matrix localMotionMatrix = _localToWorld * command.getWorldToLocal()
                                            * command.getMotionMatrix()
                                            * command.getLocalToWorld() * _worldToLocal;

            // Transform by the localMotionMatrix
            _locator->setTransform(localMotionMatrix * _startMotionMatrix);

            // OSG_NOTICE<<"New locator matrix "<<_locator->getTransform()<<std::endl;

            return true;
        }
        case osgManipulator::MotionCommand::FINISH:
        {
            return true;
        }
        case osgManipulator::MotionCommand::NONE:
        default:
            return false;
    }
}

class VolumeTileCallback : public osg::NodeCallback
{
    public:

        VolumeTileCallback()
        {
        }

        VolumeTileCallback(const VolumeTileCallback& vtc,const osg::CopyOp& copyop):
            osg::NodeCallback(vtc,copyop) {}

        META_Object(osgPresentation, VolumeTileCallback);

        void reset() {}
        void update(osg::Node* /*node*/) {}
        void setPause(bool /*pause*/) {}

        virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
        {
            osgVolume::VolumeTile* tile = dynamic_cast<osgVolume::VolumeTile*>(node);
            osgVolume::Locator* locator = tile ? tile->getLocator() : 0;
            if (tile)
            {
                OSG_NOTICE<<"VolumeTileCallback : Have locator matrix "<<locator->getTransform()<<std::endl;
            }

            // note, callback is responsible for scenegraph traversal so
            // they must call traverse(node,nv) to ensure that the
            // scene graph subtree (and associated callbacks) are traversed.
            traverse(node,nv);
        }

};


struct VolumeRegionCallback : public osg::NodeCallback
{
public:
    VolumeRegionCallback(const osg::Matrixd& originalMatrix, const std::string& str):
        _matrix(originalMatrix),
        _source(str) {}

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        osgVolume::VolumeTile* tile = dynamic_cast<osgVolume::VolumeTile*>(node);
        osgVolume::Locator* locator = tile ? tile->getLocator() : 0;
        if (locator)
        {
            PropertyReader pr(nv->getNodePath(), _source);

            float xMin=0.0;
            float yMin=0.0;
            float zMin=0.0;
            float xMax=1.0;
            float yMax=1.0;
            float zMax=1.0;

            pr>>xMin>>yMin>>zMin>>xMax>>yMax>>zMax;

            if (pr.ok())
            {
                OSG_NOTICE<<"VolumeRegionCallback : xMin="<<xMin<<", yMin="<<yMin<<", zMin="<<zMin<<", xMax="<<xMax<<", yMax="<<yMax<<", zMax="<<zMax<<std::endl;
            }
            else
            {
                OSG_NOTICE<<"Problem in reading, VolumeRegionCallback : xMin="<<xMin<<", yMin="<<yMin<<", zMin="<<zMin<<", xMax="<<xMax<<", yMax="<<yMax<<", zMax="<<zMax<<std::endl;
            }

            osg::Matrixd tm = osg::Matrix::scale(xMax-xMin, yMax-yMin, zMax-zMin) *
                              osg::Matrix::translate(xMin,yMin,zMin);

            locator->setTransform(tm * _matrix);

        }
        else
        {
            OSG_NOTICE<<"VolumeRegionCallback not attached to VolumeTile, unable to update any values."<<std::endl;
        }

        // note, callback is responsible for scenegraph traversal so
        // they must call traverse(node,nv) to ensure that the
        // scene graph subtree (and associated callbacks) are traversed.
        traverse(node, nv);
    }

protected:

    osg::Matrixd _matrix;
    std::string  _source;
};

struct ScalarPropertyCallback : public osg::NodeCallback
{
public:
    ScalarPropertyCallback(osgVolume::ScalarProperty* sp, const std::string& str):
        _sp(sp),
        _source(str) {}

    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    {
        PropertyReader pr(nv->getNodePath(), _source);

        float value=0.0;
        pr>>value;

        if (pr.ok())
        {
            OSG_NOTICE<<"ScalarPropertyCallback : value ["<<_source<<"]="<<value<<std::endl;
            _sp->setValue(value);
        }
        else
        {
            OSG_NOTICE<<"Problem in reading, ScalarPropertyCallback : value="<<value<<std::endl;
        }

        // note, callback is responsible for scenegraph traversal so
        // they must call traverse(node,nv) to ensure that the
        // scene graph subtree (and associated callbacks) are traversed.
        traverse(node, nv);
    }

protected:

    osgVolume::ScalarProperty* _sp;
    std::string  _source;
};

void SlideShowConstructor::setUpVolumeScalarProperty(osgVolume::VolumeTile* tile, osgVolume::ScalarProperty* property, const std::string& source)
{
    if (!source.empty())
    {
        if (containsPropertyReference(source))
        {
            tile->addUpdateCallback(new ScalarPropertyCallback( property, source));
        }
        else
        {
            float value;
            std::istringstream sstream(source);
            sstream>>value;
            property->setValue(value);
        }
    }
}


void SlideShowConstructor::addVolume(const std::string& filename, const PositionData& in_positionData, const VolumeData& volumeData)
{
    // osg::Object::DataVariance defaultMatrixDataVariance = osg::Object::DYNAMIC; // STATIC

    PositionData positionData(in_positionData);

    osg::ref_ptr<osgDB::Options> options = _options;
    if (!volumeData.options.empty())
    {
        options = _options->cloneOptions();
        options->setOptionString(volumeData.options);
    }

    std::string foundFile = filename;
    osg::ref_ptr<osg::Image> image;
    osg::ref_ptr<osgVolume::Volume> volume;
    osg::ref_ptr<osgVolume::VolumeTile> tile;
    osg::ref_ptr<osgVolume::ImageLayer> layer;

    // check for wild cards
    if (filename.find('*')!=std::string::npos)
    {
        osgDB::DirectoryContents filenames = osgDB::expandWildcardsInFilename(filename);
        if (filenames.empty()) return;

        // make sure images are in alphabetical order.
        std::sort(filenames.begin(), filenames.end(), osgDB::FileNameComparator());

        typedef std::vector< osg::ref_ptr<osg::Image> > Images;
        Images images;
        for(osgDB::DirectoryContents::iterator itr = filenames.begin();
            itr != filenames.end();
            ++itr)
        {
            osg::ref_ptr<osg::Image> loadedImage = osgDB::readImageFile(*itr, options.get());
            if (loadedImage.valid())
            {
                images.push_back(loadedImage.get());
            }
        }

        image = osg::createImage3DWithAlpha(images);
    }
    else
    {
        osgDB::FileType fileType = osgDB::fileType(foundFile);
        if (fileType == osgDB::FILE_NOT_FOUND)
        {
            foundFile = findFileAndRecordPath(foundFile);
            fileType = osgDB::fileType(foundFile);
        }

        if (fileType == osgDB::DIRECTORY)
        {
            image = osgDB::readImageFile(foundFile+".dicom", options.get());
        }
        else if (fileType == osgDB::REGULAR_FILE)
        {
            std::string ext = osgDB::getFileExtension(foundFile);
            if (ext=="osg" || ext=="ive" || ext=="osgx" || ext=="osgb" || ext=="osgt")
            {
                osg::ref_ptr<osg::Object> obj = osgDB::readObjectFile(foundFile);
                image = dynamic_cast<osg::Image*>(obj.get());
                volume = dynamic_cast<osgVolume::Volume*>(obj.get());
            }
            else
            {
                image = osgDB::readImageFile( foundFile, options.get() );
            }
        }
        else
        {
            // not found image, so fallback to plugins/callbacks to find the model.
            image = osgDB::readImageFile( filename, options.get() );
            if (image) recordOptionsFilePath(options.get() );
        }
    }

    if (!image && !volume) return;


    if (volumeData.colorSpaceOperation!=osg::NO_COLOR_SPACE_OPERATION)
    {
        OSG_NOTICE<<"Doing colour space conversion"<<std::endl;
        osg::ref_ptr<osg::Image> converted_image = osg::colorSpaceConversion(volumeData.colorSpaceOperation, image.get(), volumeData.colorModulate);
        if (converted_image!=image)
        {
            image->swap(*converted_image);
        }
    }

    if (positionData.scale.x()<0.0)
    {
        image->flipHorizontal();
        positionData.scale.x() = fabs(positionData.scale.x());

        OSG_INFO<<"addVolume(..) image->flipHorizontal();"<<std::endl;
    }

    if (positionData.scale.y()<0.0)
    {
        image->flipVertical();
        positionData.scale.y() = fabs(positionData.scale.y());

        OSG_INFO<<"addVolume(..) image->flipVertical();"<<std::endl;
    }

    if (positionData.scale.z()<0.0)
    {
        image->flipDepth();
        positionData.scale.z() = fabs(positionData.scale.z());

        OSG_INFO<<"addVolume(..) image->flipDepth();"<<std::endl;
    }

    if (volume.valid())
    {
        if (!tile)
        {
            if (volume->getNumChildren()>0)
            {
                tile = dynamic_cast<osgVolume::VolumeTile*>(volume->getChild(0));
            }
        }
    }
    else
    {
        volume = new osgVolume::Volume;
    }

    if (tile.valid())
    {
        layer = dynamic_cast<osgVolume::ImageLayer*>(tile->getLayer());
        image = layer.valid() ? layer->getImage() : 0;
    }
    else
    {
        if (!image) return;

        tile = new osgVolume::VolumeTile;
        volume->addChild(tile.get());
    }

    if (!layer)
    {
        if (!image) return;

        osg::ref_ptr<osgVolume::ImageDetails> details = dynamic_cast<osgVolume::ImageDetails*>(image->getUserData());
        osg::ref_ptr<osg::RefMatrix> matrix = details ? details->getMatrix() : dynamic_cast<osg::RefMatrix*>(image->getUserData());

        osg::ref_ptr<osgVolume::ImageLayer> layer = new osgVolume::ImageLayer(image.get());
        if (details)
        {
            layer->setTexelOffset(details->getTexelOffset());
            layer->setTexelScale(details->getTexelScale());
        }
        layer->rescaleToZeroToOneRange();

        if (matrix.valid())
        {
            layer->setLocator(new osgVolume::Locator(*matrix));
            tile->setLocator(new osgVolume::Locator(*matrix));
        }
        else
        {
            layer->setLocator(new osgVolume::Locator());
            tile->setLocator(new osgVolume::Locator());
        }

        if (!volumeData.region.empty())
        {
            if (containsPropertyReference(volumeData.region))
            {
                tile->addUpdateCallback(new VolumeRegionCallback((matrix.valid() ? *matrix : osg::Matrix::identity()), volumeData.region));
            }
            else
            {
                float region[6];
                std::istringstream sstream(volumeData.region);
                sstream>>region[0]>>region[1]>>region[2]>>region[3]>>region[4]>>region[5];

                osg::Matrix tm = osg::Matrix::scale(region[3]-region[0], region[4]-region[1], region[5]-region[2]) *
                                osg::Matrix::translate(region[0],region[1],region[2]);

                if (matrix.valid())
                {
                    tile->setLocator(new osgVolume::Locator(tm * (*matrix)));
                }
                else
                {
                    tile->setLocator(new osgVolume::Locator(tm));
                }
            }
        }

        tile->setLayer(layer.get());

        osgVolume::SwitchProperty* sp = new osgVolume::SwitchProperty;
        sp->setActiveProperty(0);



        osgVolume::AlphaFuncProperty* ap = new osgVolume::AlphaFuncProperty(0.1f);
        setUpVolumeScalarProperty(tile.get(), ap, volumeData.cutoffValue);

        osgVolume::TransparencyProperty* tp = new osgVolume::TransparencyProperty(1.0f);
        setUpVolumeScalarProperty(tile.get(), tp, volumeData.alphaValue);

        osgVolume::SampleDensityProperty* sd = new osgVolume::SampleDensityProperty(0.005);
        setUpVolumeScalarProperty(tile.get(), sd, volumeData.sampleDensityValue);

        osgVolume::SampleDensityWhenMovingProperty* sdm = 0;
        if (!volumeData.sampleDensityWhenMovingValue.empty())
        {
            sdm = new osgVolume::SampleDensityWhenMovingProperty(0.005);
            setUpVolumeScalarProperty(tile.get(), sdm, volumeData.sampleDensityWhenMovingValue);
        }

        osgVolume::TransferFunctionProperty* tfp = volumeData.transferFunction.valid() ? new osgVolume::TransferFunctionProperty(volumeData.transferFunction.get()) : 0;

        {
            // Standard
            osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
            cp->addProperty(ap);
            cp->addProperty(sd);
            cp->addProperty(tp);
            if (sdm) cp->addProperty(sdm);
            if (tfp) cp->addProperty(tfp);

            sp->addProperty(cp);
        }

        {
            // Light
            osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
            cp->addProperty(ap);
            cp->addProperty(sd);
            cp->addProperty(tp);
            cp->addProperty(new osgVolume::LightingProperty);
            if (sdm) cp->addProperty(sdm);
            if (tfp) cp->addProperty(tfp);

            sp->addProperty(cp);
        }

        {
            // Isosurface
            osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
            cp->addProperty(sd);
            cp->addProperty(tp);


            osgVolume::IsoSurfaceProperty* isp = new osgVolume::IsoSurfaceProperty(0.1);
            setUpVolumeScalarProperty(tile.get(), isp, volumeData.alphaValue);
            cp->addProperty(isp);

            if (sdm) cp->addProperty(sdm);
            if (tfp) cp->addProperty(tfp);

            sp->addProperty(cp);
        }

        {
            // MaximumIntensityProjection
            osgVolume::CompositeProperty* cp = new osgVolume::CompositeProperty;
            cp->addProperty(ap);
            cp->addProperty(sd);
            cp->addProperty(tp);
            cp->addProperty(new osgVolume::MaximumIntensityProjectionProperty);
            if (sdm) cp->addProperty(sdm);
            if (tfp) cp->addProperty(tfp);

            sp->addProperty(cp);
        }

        switch(volumeData.shadingModel)
        {
            case(VolumeData::Standard):                     sp->setActiveProperty(0); break;
            case(VolumeData::Light):                        sp->setActiveProperty(1); break;
            case(VolumeData::Isosurface):                   sp->setActiveProperty(2); break;
            case(VolumeData::MaximumIntensityProjection):   sp->setActiveProperty(3); break;
        }

        layer->addProperty(sp);
        tile->setVolumeTechnique(new osgVolume::RayTracedTechnique);
        tile->addEventCallback(new osgVolume::PropertyAdjustmentCallback());
    }




    osg::ref_ptr<osg::Node> model = volume.get();

    if (volumeData.useTabbedDragger || volumeData.useTrackballDragger)
    {
        osg::ref_ptr<osg::Group> group = new osg::Group;

        osg::ref_ptr<osgManipulator::Dragger> dragger;
        if (volumeData.useTabbedDragger)
        {
            if (volumeData.useTrackballDragger)
                dragger = new osgManipulator::TabBoxTrackballDragger;
            else
                dragger = new osgManipulator::TabBoxDragger;
        }
        else
            dragger = new osgManipulator::TrackballDragger();

        dragger->setupDefaultGeometry();
        dragger->setHandleEvents(true);
        dragger->setActivationModKeyMask(osgGA::GUIEventAdapter::MODKEY_SHIFT);
        dragger->addDraggerCallback(new DraggerVolumeTileCallback(tile.get(), tile->getLocator()));
        dragger->setMatrix(osg::Matrix::translate(0.5,0.5,0.5)*tile->getLocator()->getTransform());


        group->addChild(dragger.get());

        //dragger->addChild(volume.get());

        group->addChild(volume.get());

        model = group.get();
    }

    ModelData modelData;
    addModel(model.get(), positionData, modelData);
}

bool SlideShowConstructor::attachTexMat(osg::StateSet* stateset, const ImageData& imageData, float s, float t, bool textureRectangle)
{
    float xScale = textureRectangle ? s : 1.0f;
    float yScale = textureRectangle ? t : 1.0f;

    float sx = (textureRectangle ? s : 1.0f) / (imageData.region_in_pixel_coords ? s : 1.0f);
    float sy = (textureRectangle ? t : 1.0f) / (imageData.region_in_pixel_coords ? t : 1.0f);

    float x1 = imageData.region[0]*sx;
    float y1 = imageData.region[1]*sy;
    float x2 = imageData.region[2]*sx;
    float y2 = imageData.region[3]*sy;

    if (x1!=0.0f || y1!=0.0f || x2!=xScale || y2 != yScale ||
        imageData.texcoord_rotate != 0.0f)
    {
        osg::TexMat* texmat = new osg::TexMat;
        texmat->setMatrix(osg::Matrix::translate(-0.5f*xScale,-0.5f*yScale,0.0f)*
                          osg::Matrix::rotate(osg::DegreesToRadians(imageData.texcoord_rotate),0.0f,0.0f,1.0f)*
                          osg::Matrix::translate(0.5f*xScale,0.5f*yScale,0.0f)*
                          osg::Matrix::scale((x2-x1)/xScale,(y2-y1)/yScale,1.0f)*
                          osg::Matrix::translate(x1,
                                                 y1,
                                                 0.0f));

        stateset->setTextureAttribute(0,texmat);
        return true;
    }
    return false;
}

osg::Node* SlideShowConstructor::attachMaterialAnimation(osg::Node* model, const PositionData& positionData)
{
    AnimationMaterial* animationMaterial = 0;

    if (!positionData.animation_material_filename.empty())
    {
#if 0
        std::string absolute_animation_file_path = osgDB::findDataFile(positionData.animation_material_filename, _options.get());
        if (!absolute_animation_file_path.empty())
        {
            std::ifstream animation_filestream(absolute_animation_file_path.c_str());
            if (!animation_filestream.eof())
            {
                animationMaterial = new AnimationMaterial;
                animationMaterial->read(animation_filestream);
            }
        }
#else
        osg::ref_ptr<osg::Object> object = osgDB::readObjectFile(positionData.animation_material_filename, _options.get());
        animationMaterial = dynamic_cast<AnimationMaterial*>(object.get());
#endif

    }
    else if (!positionData.fade.empty())
    {
        std::istringstream iss(positionData.fade);

        animationMaterial = new AnimationMaterial;
        while (!iss.fail() && !iss.eof())
        {
            float time=1.0f, alpha=1.0f;
            iss >> time >> alpha;
            if (!iss.fail())
            {
                osg::Material* material = new osg::Material;
                material->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(1.0f,1.0f,1.0f,alpha));
                material->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(1.0f,1.0f,1.0f,alpha));
                animationMaterial->insert(time,material);
            }
        }
    }

    if (animationMaterial)
    {
        animationMaterial->setLoopMode(positionData.animation_material_loop_mode);

        AnimationMaterialCallback* animationMaterialCallback = new AnimationMaterialCallback(animationMaterial);
        animationMaterialCallback->setTimeOffset(positionData.animation_material_time_offset);
        animationMaterialCallback->setTimeMultiplier(positionData.animation_material_time_multiplier);

        osg::Group* decorator = new osg::Group;
        decorator->addChild(model);

        decorator->setUpdateCallback(animationMaterialCallback);

        if (animationMaterial->requiresBlending())
        {
            SetToTransparentBin sttb;
            decorator->accept(sttb);
        }

        return decorator;
    }

    return model;
}

osg::AnimationPathCallback* SlideShowConstructor::getAnimationPathCallback(const PositionData& positionData)
{
    if (!positionData.path.empty())
    {
        // need to create an Options object with cache off to prevent sharing of animation paths
        osg::ref_ptr<osgDB::Options> options = _options.valid() ? _options->cloneOptions() : new osgDB::Options;
        options->setObjectCacheHint(osgDB::Options::CACHE_NONE);

        osg::ref_ptr<osg::Object> object = osgDB::readObjectFile(positionData.path, options.get());
        osg::AnimationPath* animation = dynamic_cast<osg::AnimationPath*>(object.get());
        if (animation)
        {
            if (positionData.frame==SlideShowConstructor::SLIDE)
            {
                osg::AnimationPath::TimeControlPointMap& controlPoints = animation->getTimeControlPointMap();
                for(osg::AnimationPath::TimeControlPointMap::iterator itr=controlPoints.begin();
                    itr!=controlPoints.end();
                    ++itr)
                {
                    osg::AnimationPath::ControlPoint& cp = itr->second;
                    cp.setPosition(convertSlideToModel(cp.getPosition()+positionData.position));
                }
            }

            animation->setLoopMode(positionData.path_loop_mode);

            osg::AnimationPathCallback* apc = new osg::AnimationPathCallback(animation);
            apc->setTimeOffset(positionData.path_time_offset);
            apc->setTimeMultiplier(positionData.path_time_multiplier);
            apc->setUseInverseMatrix(positionData.inverse_path);

            OSG_INFO<<"UseInverseMatrix "<<positionData.inverse_path<<std::endl;

            return apc;

        }

    }
    return 0;
}

osg::Vec3 SlideShowConstructor::computePositionInModelCoords(const PositionData& positionData) const
{
    if (positionData.frame==SLIDE)
    {
        OSG_INFO<<"********* Scaling from slide coords to model coords"<<std::endl;
        return convertSlideToModel(positionData.position);
    }
    else
    {
        OSG_INFO<<"keeping original model coords"<<std::endl;
        return positionData.position;
    }
}

osg::Vec3 SlideShowConstructor::convertSlideToModel(const osg::Vec3& position) const
{
    return osg::Vec3(_slideOrigin+osg::Vec3(_slideWidth*position.x(),0.0f,_slideHeight*position.y()))*(1.0f-position.z());
}

osg::Vec3 SlideShowConstructor::convertModelToSlide(const osg::Vec3& position) const
{
    return osg::Vec3((position.x()*(_slideOrigin.y()/position.y())-_slideOrigin.x())/_slideWidth,
                     (position.z()*(_slideOrigin.y()/position.y())-_slideOrigin.z())/_slideHeight,
                     1.0f-position.y()/_slideOrigin.y());
}

void SlideShowConstructor::updatePositionFromInModelCoords(const osg::Vec3& vertex, PositionData& positionData) const
{
    if (positionData.frame==SLIDE)
    {
        positionData.position = convertModelToSlide(vertex);
    }
    else
    {
        positionData.position = vertex;
    }
}

void SlideShowConstructor::recordOptionsFilePath(const osgDB::Options* options)
{
    if (options)
    {
        std::string filename_used = _options->getPluginStringData("filename");
        std::string path = osgDB::getFilePath(filename_used);
        if (!path.empty() && _filePathData.valid())
        {
            osgDB::FilePathList::iterator itr = std::find(_filePathData->filePathList.begin(),_filePathData->filePathList.end(),path);
            if (itr==_filePathData->filePathList.end())
            {
                OSG_INFO<<"SlideShowConstructor::recordOptionsFilePath(..) - new path to record path="<<path<<" filename_used="<<filename_used<<std::endl;
                _filePathData->filePathList.push_front(path);
            }
        }
    }
}

