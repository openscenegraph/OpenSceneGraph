#include <osgProducer/ViewerEventHandler>

#include <osgDB/WriteFile>
#include <osgText/Text>

#include <algorithm>

using namespace osgProducer;

class DrawCallback : public Producer::CameraGroup::StatsHandler, public Producer::Camera::Callback
{
public:

    DrawCallback(ViewerEventHandler* veh, unsigned int cameraNumber):
        _veh(veh),
        _cameraNumber(cameraNumber),
        _helpInitialized(false),
        _statsInitialized(false)
    {
	_fs.resize(6);
	_index = 0;
        
        _veh->getOsgCameraGroup()->setStatsHandler(this);
        
        _stateset = new osg::StateSet;
        _viewport = new osg::Viewport(0,0,1280,1024);
        _stateset->setAttribute(_viewport.get());
    
        //createHelpText();
        createStatsText();
        
        //_helpInitialized = false;

    }
    
    void setArraySize(unsigned int size) { _fs.resize(size); }
    unsigned int getArraySize() { return _fs.size(); }

    void operator() (const Producer::CameraGroup &cg )
    {
	_index = (_index + 1) % _fs.size();
        _fs[_index] = cg.getFrameStats();     
    }

    virtual void operator()( const Producer::Camera & camera);

protected:
    
    ViewerEventHandler* _veh;
    unsigned int _cameraNumber;

    osg::ref_ptr<osg::StateSet> _stateset;
    osg::ref_ptr<osg::Viewport> _viewport;
    
    // help related methods and data
    void displayHelp();
    void createHelpText();
    
    typedef std::vector< osg::ref_ptr<osgText::Text> > TextList;
    bool        _helpInitialized;
    TextList    _descriptionList;
    TextList    _optionList;
    TextList    _explanationList;


    // stats related methods and data.
    void displayStats();
    void createStatsText();
    
    typedef std::vector<double> CameraTimes;

    bool        _statsInitialized;
    osg::ref_ptr<osgText::Text> _frameRateLabelText;
    osg::ref_ptr<osgText::Text> _frameRateCounterText;
    TextList                    _statsLabelList;
    osg::ref_ptr<osgText::Text> _updateTimeText;
    CameraTimes                 _cullTimes;
    TextList                    _cullTimeText;
    CameraTimes                 _drawTimes;
    TextList                    _drawTimeText;

    std::vector <Producer::CameraGroup::FrameStats> _fs;
    unsigned int _index;

};


void DrawCallback::operator()( const Producer::Camera & camera)
{
    if (_veh->getDisplayHelp()==false && 
        _veh->getFrameStatsMode()==ViewerEventHandler::NO_STATS) return;

    int x,y;
    unsigned int width,height;
    camera.getProjectionRect(x,y,width,height);
    _viewport->setViewport(x,y,width,height);

    OsgSceneHandler* osh = _veh->getOsgCameraGroup()->getSceneHandlerList()[_cameraNumber].get();
    osh->getState()->pushStateSet(_stateset.get());

    if (_veh->getDisplayHelp()) displayHelp();
    
    if (_veh->getFrameStatsMode()!=ViewerEventHandler::NO_STATS && camera.getInstrumentationMode())
    {
        osh->getState()->apply();
        displayStats();
    }

    osh->getState()->popStateSet();

}

void DrawCallback::displayHelp()
{
    if (!_helpInitialized) createHelpText();

    OsgSceneHandler* osh = _veh->getOsgCameraGroup()->getSceneHandlerList()[_cameraNumber].get();

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


    glPopMatrix();
    glMatrixMode( GL_PROJECTION );
    glPopMatrix();
    glMatrixMode( GL_MODELVIEW );

    glPopAttrib();
}

void DrawCallback::createHelpText()
{

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
            text->setFontSize(characterSize,characterSize);
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
            text->setFontSize(characterSize,characterSize);
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
            text->setFontSize(characterSize,characterSize);
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
    _helpInitialized = true;
}

void DrawCallback::displayStats()
{
    if (!_statsInitialized) createStatsText();

    OsgSceneHandler* osh = _veh->getOsgCameraGroup()->getSceneHandlerList()[_cameraNumber].get();

    // render the text
    if (_veh->getFrameStatsMode()>=ViewerEventHandler::FRAME_RATE)
    {
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
        
        
        // update and draw the frame rate text.
        
        char tmpText[128];

        _frameRateLabelText->draw(*(osh->getState()));

        if (_fs.size()>1)
        {
            unsigned int lindex = (_index + 1) % _fs.size();
            double timeForFrames = (_fs[_index]._startOfFrame-_fs[lindex]._startOfFrame);
            double timePerFrame = timeForFrames/(double)(_fs.size()-1);
            sprintf(tmpText,"%4.2f",1.0/timePerFrame);
            _frameRateCounterText->setText(tmpText);
        }
        _frameRateCounterText->draw(*(osh->getState()));
        

        if (_veh->getFrameStatsMode()>=ViewerEventHandler::CAMERA_STATS)
        {

            TextList::iterator itr;
            for(itr=_statsLabelList.begin();
                itr!=_statsLabelList.end();
                ++itr)
            {
                (*itr)->draw(*(osh->getState()));
            }

            double updateTime = 0.0;
            std::fill(_cullTimes.begin(),_cullTimes.end(),0.0);
            std::fill(_drawTimes.begin(),_drawTimes.end(),0.0);

            for(unsigned int frame = 0; frame < _fs.size(); frame++ )
            {
	        Producer::CameraGroup::FrameStats fs = _fs[frame];
                updateTime += (fs._endOfUpdate-fs._startOfUpdate);

	        for(unsigned int i = 0; i < fs.getNumFrameTimeStampSets(); i++ )
                {
	            Producer::Camera::FrameTimeStampSet fts = fs.getFrameTimeStampSet(i);

	            _cullTimes[i] += fts[Producer::Camera::EndCull]-fts[Producer::Camera::BeginCull];
	            _drawTimes[i] += fts[Producer::Camera::EndDraw]-fts[Producer::Camera::BeginDraw];
                }
            }

            sprintf(tmpText,"%4.2f",1000.0*updateTime/(double)_fs.size());
            _updateTimeText->setText(tmpText);

            _updateTimeText->draw(*(osh->getState()));

            CameraTimes::iterator titr;
            for(itr=_cullTimeText.begin(),titr = _cullTimes.begin();
                itr!=_cullTimeText.end() && titr!=_cullTimes.end();
                ++itr,++titr)
            {
                sprintf(tmpText,"%4.2f",1000.0*(*titr)/(double)_fs.size());
                (*itr)->setText(tmpText);
                (*itr)->draw(*(osh->getState()));
            }
            for(itr=_drawTimeText.begin(),titr = _drawTimes.begin();
                itr!=_drawTimeText.end() && titr!=_cullTimes.end();
                ++itr,++titr)
            {
                sprintf(tmpText,"%4.2f",1000.0*(*titr)/(double)_fs.size());
                (*itr)->setText(tmpText);
                (*itr)->draw(*(osh->getState()));
            }
        }
        
        glPopMatrix();
        glMatrixMode( GL_PROJECTION );
        glPopMatrix();
        glMatrixMode( GL_MODELVIEW );

        glPopAttrib();
    }

    // render graphs
    if (_veh->getFrameStatsMode()>=ViewerEventHandler::CAMERA_STATS)
    {

        // Set up the Orthographic view
        glMatrixMode( GL_PROJECTION );
        glPushMatrix();
        glLoadIdentity();
        glOrtho( -.025, .128, 600.0, -10.0, -1.0, 1.0 ); 

        glPushAttrib( GL_ENABLE_BIT );
        glDisable( GL_LIGHTING );
        glDisable( GL_DEPTH_TEST );
        glEnable( GL_BLEND );

        glMatrixMode( GL_MODELVIEW );
        glPushMatrix();
        glLoadIdentity();

        unsigned int lindex = (_index + 1) % _fs.size();
        Producer::Camera::TimeStamp zero = _fs[lindex]._startOfFrame;
        unsigned int i;
        double x1=0.0, x2=0.0, y1=0.0, y2=0.0;
        for(unsigned int frame = 0; frame < _fs.size(); frame++ )
        {
	    Producer::CameraGroup::FrameStats fs = _fs[(lindex + frame) % _fs.size()];
	    y1 = 0.0;
   	    y2 = y1 + 10;
	    x1 = fs._startOfUpdate - zero;
	    x2 = fs._endOfUpdate   - zero;

	    glBegin( GL_QUADS );

	    // Draw Update length
 	    glColor4f( 0.0, 1.0, 0.0, 0.5 );
	    glVertex2d( x1, y1);
	    glVertex2d( x2, y1);
	    glVertex2d( x2, y2);
	    glVertex2d( x1, y2);

	    for( i = 0; i < fs.getNumFrameTimeStampSets(); i++ )
            {
	        Producer::Camera::FrameTimeStampSet fts = fs.getFrameTimeStampSet(i);
	        y1 += 13.0;
	        y2 = y1 + 10.0;
	        x1 = fts[Producer::Camera::BeginCull] - zero;
	        x2 = fts[Producer::Camera::EndCull]   - zero;

 	        glColor4f( 0.0, 1.0, 1.0, 0.5 );
	        glVertex2d( x1, y1);
	        glVertex2d( x2, y1);
	        glVertex2d( x2, y2);
	        glVertex2d( x1, y2);

	        x1 = fts[Producer::Camera::BeginDraw] - zero;
	        x2 = fts[Producer::Camera::EndDraw]   - zero;

 	        glColor4f( 1.0, 1.0, 0.0, 0.5 );
	        glVertex2d( x1, y1);
	        glVertex2d( x2, y1);
	        glVertex2d( x2, y2);
	        glVertex2d( x1, y2);

            }
	    glEnd();

	    glBegin( GL_LINES );
	    glColor4f( 1, 1, 1, 0.5 );
	    glVertex2d( fs._startOfFrame - zero , 0.0 );
            y1 = fs.getNumFrameTimeStampSets() * 13.0 + 10.0;
            glVertex2d( fs._startOfFrame - zero, y1 );

            y1 = 12.5; 
	    for( i = 0; i < fs.getNumFrameTimeStampSets(); i++ )
            {
	        y2 = y1 + 11; 
	        Producer::Camera::FrameTimeStampSet fts = fs.getFrameTimeStampSet(i);
	        Producer::Camera::TimeStamp vsync = fts[Producer::Camera::Vsync];
	        double x1 = vsync - zero;
                glColor4f( 1.0, 1.0, 0.0, 0.5 );
	        glVertex2d( x1, y1 );
                glVertex2d( x1, y2 );
 	        y1 += 13.0;
  	    }
	    glEnd();	
        }

        glBegin( GL_LINES );

        glColor4f( 1, 1, 1, 0.5 );
        for( i = 0; i < 128; i++ )
        {
	    glVertex2d((GLdouble)i*.001, y1);

           if( !(i%10) )
	       glVertex2d((GLdouble)i*.001, y1 - 5.0);
	    else if( !(i%5) )
	        glVertex2d((GLdouble)i*.001, y1 - 3.0);
	    else
	        glVertex2d((GLdouble)i*.001, y1 - 1.0);
        }

        glEnd();

        glPopMatrix();
        glMatrixMode( GL_PROJECTION );
        glPopMatrix();
        glMatrixMode( GL_MODELVIEW );

        glPopAttrib();
        
    }
}

void DrawCallback::createStatsText()
{
    _statsInitialized = true;

    float characterSize = 20.0f;

    osg::Vec4 colorFR(1.0f,1.0f,1.0f,1.0f);
    osg::Vec4 colorUpdate( 0.0f,1.0f,0.0f,1.0f);
    osg::Vec4 colorCull( 0.0f,1.0f,1.0f,1.0f);
    osg::Vec4 colorDraw( 1.0f,1.0f,0.0f,1.0f);
    
    float leftPos = 10.0f;
    
    osg::Vec3 pos(leftPos,1000.0f,0.0f);

    _frameRateLabelText = new osgText::Text;
    _frameRateLabelText->setFont("fonts/arial.ttf");
    _frameRateLabelText->setColor(colorFR);
    _frameRateLabelText->setCharacterSize(characterSize);
    _frameRateLabelText->setPosition(pos);
    _frameRateLabelText->setAlignment(osgText::Text::BASE_LINE);
    _frameRateLabelText->setText("Frame Rate: ");

    pos.x() = _frameRateLabelText->getBound().xMax();

    _frameRateCounterText = new osgText::Text;
    _frameRateCounterText->setFont("fonts/arial.ttf");
    _frameRateCounterText->setColor(colorFR);
    _frameRateCounterText->setCharacterSize(characterSize);
    _frameRateCounterText->setPosition(pos);
    _frameRateCounterText->setAlignment(osgText::Text::BASE_LINE);
    _frameRateCounterText->setText("0123456789.");


    pos.x() = leftPos;
    pos.y() -= characterSize;

    {
        osgText::Text* text = new osgText::Text;
        text->setFont("fonts/arial.ttf");
        text->setColor(colorUpdate);
        text->setFontSize(characterSize,characterSize);
        text->setCharacterSize(characterSize);
        text->setPosition(pos);
        text->setAlignment(osgText::Text::BASE_LINE);
        text->setText("Update: ");

        _statsLabelList.push_back(text);
        
        pos.x() = text->getBound().xMax();

        _updateTimeText = new osgText::Text;

        _updateTimeText->setFont("fonts/arial.ttf");
        _updateTimeText->setColor(colorUpdate);
        _updateTimeText->setFontSize(characterSize,characterSize);
        _updateTimeText->setCharacterSize(characterSize);
        _updateTimeText->setPosition(pos);
        _updateTimeText->setAlignment(osgText::Text::BASE_LINE);
        _updateTimeText->setText("0123456789.");
        
    }

    pos.x() = leftPos;
    pos.y() -= characterSize;

    _cullTimes.clear();
    _drawTimes.clear();

    OsgCameraGroup* ocg = _veh->getOsgCameraGroup();
    Producer::CameraConfig* cfg = ocg->getCameraConfig();
    for (unsigned int i=0;i<cfg->getNumberOfCameras(); ++i )
    {
        pos.x() = leftPos;

        osgText::Text* cullLabel = new osgText::Text;
        cullLabel->setFont("fonts/arial.ttf");
        cullLabel->setColor(colorCull);
        cullLabel->setFontSize(characterSize,characterSize);
        cullLabel->setCharacterSize(characterSize);
        cullLabel->setPosition(pos);
        cullLabel->setAlignment(osgText::Text::BASE_LINE);
        cullLabel->setText("Cull: ");

        _statsLabelList.push_back(cullLabel);
        
        pos.x() = cullLabel->getBound().xMax();

        osgText::Text* cullField = new osgText::Text;

        cullField->setFont("fonts/arial.ttf");
        cullField->setColor(colorCull);
        cullField->setFontSize(characterSize,characterSize);
        cullField->setCharacterSize(characterSize);
        cullField->setPosition(pos);
        cullField->setAlignment(osgText::Text::BASE_LINE);
        cullField->setText("1000.00");
        
        _cullTimes.push_back(0.0);
        
        _cullTimeText.push_back(cullField);

        pos.x() = cullField->getBound().xMax();


        osgText::Text* drawLabel = new osgText::Text;
        drawLabel->setFont("fonts/arial.ttf");
        drawLabel->setColor(colorDraw);
        drawLabel->setFontSize(characterSize,characterSize);
        drawLabel->setCharacterSize(characterSize);
        drawLabel->setPosition(pos);
        drawLabel->setAlignment(osgText::Text::BASE_LINE);
        drawLabel->setText("Draw: ");

        _statsLabelList.push_back(drawLabel);
        
        pos.x() = drawLabel->getBound().xMax();

        osgText::Text* drawField = new osgText::Text;

        drawField->setFont("fonts/arial.ttf");
        drawField->setColor(colorDraw);
        drawField->setFontSize(characterSize,characterSize);
        drawField->setCharacterSize(characterSize);
        drawField->setPosition(pos);
        drawField->setAlignment(osgText::Text::BASE_LINE);
        drawField->setText("1000.00");
        
        _drawTimeText.push_back(drawField);

        _drawTimes.push_back(0.0);

        pos.x() = cullField->getBound().xMax();
    }


}



ViewerEventHandler::ViewerEventHandler(OsgCameraGroup* cg):
    _cg(cg),
    _writeNodeFileName("savedmodel.osg"),
    _displayHelp(false),
    _frameStatsMode(NO_STATS)
{
    Producer::CameraConfig* cfg = _cg->getCameraConfig();
    Producer::Camera *cam = cfg->getCamera(0);
    cam->addPostDrawCallback(new DrawCallback(this,0));
}

bool ViewerEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
{
    if(!_cg) return false;

    if(ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
    {

        switch( ea.getKey() )
        {
            case 's' :
            {
                _frameStatsMode = (FrameStatsMode)((_frameStatsMode+1)%3);
                if (_frameStatsMode==NO_STATS)
                {
                    _cg->setInstrumentationMode(false);
                }
                else
                {
                    _cg->setInstrumentationMode(true);
                }
                return true;
            }
            case 'v' :
            {
                _cg->setBlockOnVsync(!_cg->getBlockOnVsync());
                return true;
            }

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
            case 'h' :
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
    usage.addKeyboardMouseBinding("h","Display help");
    usage.addKeyboardMouseBinding("o","Write scene graph to file");
    usage.addKeyboardMouseBinding("s","Toggle intrumention");
    usage.addKeyboardMouseBinding("v","Toggle block and vsync");
}
