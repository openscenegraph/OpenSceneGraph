#include <osgProducer/ViewerEventHandler>
#include <osgDB/WriteFile>
#include <osgText/Text>

using namespace osgProducer;

class DrawHelpCallback : public Producer::Camera::Callback
{
public:

    DrawHelpCallback(ViewerEventHandler* veh):
        _veh(veh),
        _initialized(false)
    {
    }
    
    virtual void operator()( const Producer::Camera & camera)
    {
    
        if (_veh->getDisplayHelp())
        {
            if (!_initialized) createText();
            
            OsgSceneHandler* osh = _veh->getOsgCameraGroup()->getSceneHandlerList()[0].get();

            int x,y;
            unsigned int width,height;
            camera.getProjectionRect(x,y,width,height);
            _viewport->setViewport(x,y,width,height);

            // should possibly update _viewport...
           
	    // Set up the Orthographic view
	    glMatrixMode( GL_PROJECTION );
	    glPushMatrix();
	    glLoadIdentity();
	    glOrtho( 0.0, 1280.0, 0.0, 1024, -1.0, 1.0 ); 

            glPushAttrib( GL_ENABLE_BIT );
	    glDisable( GL_LIGHTING );
	    glDisable( GL_DEPTH_TEST );
	    glEnable( GL_BLEND );

	    glMatrixMode( GL_MODELVIEW );
	    glPushMatrix();
	    glLoadIdentity();
            
            osh->getState()->pushStateSet(_stateset.get());

            for(TextList::iterator ditr=_descriptionList.begin();
                ditr!=_descriptionList.end();
                ++ditr)
            {
                (*ditr)->draw(*(osh->getState()));
            }

            for(TextList::iterator oitr=_optionList.begin();
                oitr!=_optionList.end();
                ++oitr)
            {
                (*oitr)->draw(*(osh->getState()));
            }
            
            for(TextList::iterator eitr=_explanationList.begin();
                eitr!=_explanationList.end();
                ++eitr)
            {
                (*eitr)->draw(*(osh->getState()));
            }

            osh->getState()->popStateSet();

	    glPopMatrix();
	    glMatrixMode( GL_PROJECTION );
	    glPopMatrix();
	    glMatrixMode( GL_MODELVIEW );

	    glPopAttrib();

        }
        
    }
    
    void createText()
    {
        _stateset = new osg::StateSet;
        _viewport = new osg::Viewport(0,0,1280,1024);
        _stateset->setAttribute(_viewport.get());
    
        OsgCameraGroup* ocg = _veh->getOsgCameraGroup();
        if (ocg->getApplicationUsage())
        {

            const osg::ApplicationUsage::UsageMap& um = ocg->getApplicationUsage()->getKeyboardMouseBindings();
            
            float maxWidthOfDisplayRegion = 1200.0f;
            float bottomOfDescription = 1000.0f;
            osg::Vec3 posDescription(0.0f,bottomOfDescription,0.0f);
            osg::Vec4 colorDescription(1.0f,1.0f,0.0f,1.0f);
            float characterSize = 20.0f;
            
            if (!(ocg->getApplicationUsage()->getDescription()).empty())
            {
                osgText::Text* text = new osgText::Text;
                text->setFont("fonts/arial.ttf");
                text->setColor(colorDescription);
                text->setCharacterSize(characterSize);
                text->setPosition(posDescription);
                text->setMaximumWidth(maxWidthOfDisplayRegion);
                text->setAlignment(osgText::Text::BASE_LINE);
                text->setText(ocg->getApplicationUsage()->getDescription());

                bottomOfDescription = text->getBound().yMin()-characterSize*2.0f;

                _descriptionList.push_back(text);
                
            }
 
            osg::Vec3 posOption(0.0f,bottomOfDescription,0.0f);
            osg::Vec4 colorOption(1.0f,1.0f,0.0f,1.0f);
            float maxX = 0.0f;

            // create option strings.
            osg::ApplicationUsage::UsageMap::const_iterator citr;
            for(citr=um.begin();
                citr!=um.end();
                ++citr)
            {
                osgText::Text* text = new osgText::Text;
                text->setFont("fonts/arial.ttf");
                text->setColor(colorOption);
                text->setCharacterSize(characterSize);
                text->setPosition(posOption);
                text->setAlignment(osgText::Text::BASE_LINE);
                text->setText(citr->first);

                if (text->getBound().xMax()>maxX) maxX=text->getBound().xMax();

                _optionList.push_back(text);
                
            }
                
            osg::Vec3 posExplanation(maxX+characterSize,bottomOfDescription,0.0f);
            osg::Vec4 colorExplanation(1.0f,1.0f,0.0f,1.0f);
            float maxWidth = maxWidthOfDisplayRegion-maxX;

            TextList::iterator oitr;
            TextList::iterator eitr;
            TextList::iterator ditr;
            
            for(citr=um.begin(), oitr=_optionList.begin();
                citr!=um.end();
                ++citr,++oitr)
            {
                osgText::Text* text = new osgText::Text;
                text->setFont("fonts/arial.ttf");
                text->setColor(colorExplanation);
                text->setCharacterSize(characterSize);
                text->setPosition(posExplanation);
                text->setMaximumWidth(maxWidth);
                text->setAlignment(osgText::Text::BASE_LINE);
                text->setText(citr->second);

                if (text->getBound().xMax()>maxX) maxX=text->getBound().xMax();
                
                // fix the position of option text to be the same height as the examplanation.
                osg::Vec3 pos((*oitr)->getPosition());
                (*oitr)->setPosition(osg::Vec3(pos.x(),posExplanation.y(),pos.z()));
                
                posExplanation.y() = text->getBound().yMin()-characterSize;

                _explanationList.push_back(text);
                
            }
            
            // compute the boundings of the all the text.
            osg::BoundingBox bb;
            for(ditr=_descriptionList.begin();
                ditr!=_descriptionList.end();
                ++ditr)
            {
                bb.expandBy((*ditr)->getBound());
            }

            for(oitr=_optionList.begin();
                oitr!=_optionList.end();
                ++oitr)
            {
                bb.expandBy((*oitr)->getBound());
            }
            
            for(eitr=_explanationList.begin();
                eitr!=_explanationList.end();
                ++eitr)
            {
                bb.expandBy((*eitr)->getBound());
            }
            
            float totalWidth = bb.xMax()-bb.xMin();
            float totalHeight = bb.yMax()-bb.yMin();
            float widthMargin = (1280.0f-totalWidth)*0.5f;
            float heightMargin = (1024.0f-totalHeight)*0.5f;
            
            osg::Vec3 delta(widthMargin-bb.xMin(),heightMargin-bb.yMin(),0.0f);
            
            // shift the text to center it.
            for(ditr=_descriptionList.begin();
                ditr!=_descriptionList.end();
                ++ditr)
            {
                (*ditr)->setPosition((*ditr)->getPosition()+delta);
            }

            for(oitr=_optionList.begin();
                oitr!=_optionList.end();
                ++oitr)
            {
                (*oitr)->setPosition((*oitr)->getPosition()+delta);
            }
            
            for(eitr=_explanationList.begin();
                eitr!=_explanationList.end();
                ++eitr)
            {
                (*eitr)->setPosition((*eitr)->getPosition()+delta);
            }
           

        }
        _initialized = true;
    }

    ViewerEventHandler* _veh;
    bool                _initialized;
    
    osg::ref_ptr<osg::StateSet> _stateset;
    osg::ref_ptr<osg::Viewport> _viewport;
    
    typedef std::vector< osg::ref_ptr<osgText::Text> > TextList;
    TextList _descriptionList;
    TextList _optionList;
    TextList _explanationList;
};


ViewerEventHandler::ViewerEventHandler(OsgCameraGroup* cg):
    _cg(cg),
    _writeNodeFileName("savedmodel.osg"),
    _displayHelp(false)
{
    Producer::CameraConfig* cfg = _cg->getCameraConfig();
    Producer::Camera *cam = cfg->getCamera(0);
    cam->addPostDrawCallback(new DrawHelpCallback(this));
}

bool ViewerEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{
    if(!_cg) return false;

    if(ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
    {

        switch( ea.getKey() )
        {
          case 'f' :
          {
            Producer::CameraConfig* cfg = _cg->getCameraConfig();
            for( unsigned int i = 0; i < cfg->getNumberOfCameras(); ++i )
            {
                Producer::Camera *cam = cfg->getCamera(i);
                Producer::RenderSurface* rs = cam->getRenderSurface();
                rs->fullScreen(!rs->isFullScreen());
            }

            return true;
          }
          case 'o' :
          {
            osg::Node* node = _cg->getSceneData();
            if (node)
            {
                std::cout<<"writing file "<<_writeNodeFileName<<std::endl;
                osgDB::writeNodeFile(*node,_writeNodeFileName.c_str());
            }
            
            return true;
          }

          case osgGA::GUIEventAdapter::KEY_Help :
          case '/' :
          case '?' :
          {
            setDisplayHelp(!getDisplayHelp());
            
            return true;
          }

        default:
            break;

        }
    }
    return false;

}

void ViewerEventHandler::accept(osgGA::GUIEventHandlerVisitor& gehv)
{
    gehv.visit(*this);
}

void ViewerEventHandler::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("f","Toggle fullscreen");
    usage.addKeyboardMouseBinding("?","Display help");
    usage.addKeyboardMouseBinding("o","Write scene graph to file");
}
