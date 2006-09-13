#include <osgProducer/ViewerEventHandler>
#include <osgGA/AnimationPathManipulator>
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>
#include <osgText/Text>
#include <osg/BlendFunc>
#include <osg/PolygonOffset>
#include <osgUtil/Statistics>
#include <OpenThreads/Barrier>


#include <algorithm>
#include <fstream>

using namespace osgProducer;


class ViewerEventHandler::SnapImageDrawCallback : public Producer::Camera::Callback
{
public:

    SnapImageDrawCallback():
        _snapImageOnNextFrame(false)
    {
    }

    void setFileName(const std::string& filename) { _filename = filename; }
    const std::string& getFileName() const { return _filename; }

    void setSnapImageOnNextFrame(bool flag) { _snapImageOnNextFrame = flag; }
    bool getSnapImageOnNextFrame() const { return _snapImageOnNextFrame; }
    
    virtual void operator()( const Producer::Camera & camera)
    {
        if (!_snapImageOnNextFrame) return;
        
        int x,y;
        unsigned int width,height;
        camera.getProjectionRectangle(x,y,width,height);

        osg::ref_ptr<osg::Image> image = new osg::Image;
        image->readPixels(x,y,width,height,
                          GL_RGB,GL_UNSIGNED_BYTE);

        if (osgDB::writeImageFile(*image,_filename))
        {
            osg::notify(osg::NOTICE) << "Saved screen image to `"<<_filename<<"`"<< std::endl;
        }
        
        _snapImageOnNextFrame = false;
    }

protected:
    
    std::string _filename;
    bool        _snapImageOnNextFrame;

    
};

class ViewerEventHandler::StatsAndHelpDrawCallback : public Producer::CameraGroup::StatsHandler, public Producer::Camera::Callback
{
public:

    StatsAndHelpDrawCallback(ViewerEventHandler* veh, unsigned int cameraNumber):
        _veh(veh),
        _cameraNumber(cameraNumber),
        _helpInitialized(false),
        _statsInitialized(false),
        _infoInitialized(false)
    {
        _fs.resize(10);
        _index = 0;
        
        Producer::PipeTimer::instance()->setReturnType( Producer::PipeTimer::milliseconds );

        _veh->getOsgCameraGroup()->setStatsHandler(this);
        
        _stateset = new osg::StateSet;
        _viewport = new osg::Viewport(0,0,1280,1024);
        _stateset->setAttribute(_viewport.get());
        _stateset->setAttribute(new osg::BlendFunc());
        _stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
        _stateset->setMode(GL_BLEND,osg::StateAttribute::ON);
        
        _projection = new osg::RefMatrix(osg::Matrix::ortho2D(0.0,1280,0.0,1024));
        _modelview = new osg::RefMatrix();
    
        //createHelpText();
        //createStatsText();
        
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
    osg::ref_ptr<osg::RefMatrix> _projection;
    osg::ref_ptr<osg::RefMatrix> _modelview;
    
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

    bool                        _statsInitialized;
    osg::ref_ptr<osgText::Text> _frameRateLabelText;
    osg::ref_ptr<osgText::Text> _frameRateCounterText;
    TextList                    _statsLabelList;
    osg::ref_ptr<osgText::Text> _updateTimeText;
    CameraTimes                 _cullTimes;
    TextList                    _cullTimeText;
    CameraTimes                 _drawTimes;
    TextList                    _drawTimeText;
    TextList                    _gpuTimeText;
    
    // info related methods and data.
    void displayInfo();
    void createInfoText();
    
    bool                        _infoInitialized;
    TextList                    _infoLabelList;
    osg::ref_ptr<osgText::Text> _positionText;
    osg::ref_ptr<osgText::Text> _orientationText;
    osg::ref_ptr<osgText::Text> _speedText;
    
    TextList                    _sceneStatsLabelList;
    osg::ref_ptr<osgText::Text> _numVerticesText;
    osg::ref_ptr<osgText::Text> _numPrimitivesText;
    osg::ref_ptr<osgText::Text> _numDrawablesText;
    

    std::vector <Producer::CameraGroup::FrameStats> _fs;
    unsigned int _index;

};


void ViewerEventHandler::StatsAndHelpDrawCallback::operator()( const Producer::Camera & camera)
{
    if (_veh->getDisplayHelp()==false && 
        _veh->getFrameStatsMode()==ViewerEventHandler::NO_STATS) return;

    int x,y;
    unsigned int width,height;
    camera.getProjectionRectangle(x,y,width,height);
    _viewport->setViewport(x,y,width,height);

    OsgSceneHandler* osh = _veh->getOsgCameraGroup()->getSceneHandlerList()[_cameraNumber].get();
    osgUtil::SceneView* sv = osh->getSceneView();
    osg::State& state = *(sv->getState());
    

    if (!_projection) _projection = new osg::RefMatrix(osg::Matrix::ortho2D(0.0,width/height*1024.0f,0.0,1024.0));
    else 
    {
        _projection->makeOrtho2D(0.0,width/height*1024.0f,0.0,1024.0);
    }


    state.applyProjectionMatrix(_projection.get());
    state.applyModelViewMatrix(_modelview.get());
    
    state.pushStateSet(_stateset.get());
    state.apply();

    if (_veh->getFrameStatsMode()!=ViewerEventHandler::NO_STATS && camera.getInstrumentationMode())
    {
        displayStats();
    }
    
    if (_veh->getDisplayHelp())
    {
        displayHelp();
    }
       
    if (_veh->getDisplayHelp())
    {
        displayInfo();
    }

    state.popStateSet();
    
    //state.apply();

}

void ViewerEventHandler::StatsAndHelpDrawCallback::displayHelp()
{
    if (!_helpInitialized) createHelpText();

    OsgSceneHandler* osh = _veh->getOsgCameraGroup()->getSceneHandlerList()[_cameraNumber].get();
    osgUtil::SceneView* sv = osh->getSceneView();


    for(TextList::iterator ditr=_descriptionList.begin();
        ditr!=_descriptionList.end();
        ++ditr)
    {
        (*ditr)->draw(*(sv->getState()));
    }

    for(TextList::iterator oitr=_optionList.begin();
        oitr!=_optionList.end();
        ++oitr)
    {
        (*oitr)->draw(*(sv->getState()));
    }

    for(TextList::iterator eitr=_explanationList.begin();
        eitr!=_explanationList.end();
        ++eitr)
    {
        (*eitr)->draw(*(sv->getState()));
    }
}

void ViewerEventHandler::StatsAndHelpDrawCallback::createHelpText()
{

    OsgCameraGroup* ocg = _veh->getOsgCameraGroup();
    if (ocg->getApplicationUsage())
    {

        const osg::ApplicationUsage::UsageMap& um = ocg->getApplicationUsage()->getKeyboardMouseBindings();

        float maxWidthOfDisplayRegion = 1200.0f;
        float bottomOfDescription = 1000.0f;
        osg::Vec3 posDescription(0.0f,bottomOfDescription,0.0f);
        osg::Vec4 colorDescription(1.0f,1.0f,0.0f,1.0f);
        osg::Vec4 backdropColorDescription(0.0f,0.0f,0.0f,1.0f);
        float characterSize = 20.0f;

        if (!(ocg->getApplicationUsage()->getDescription()).empty())
        {
            osgText::Text* text = new osgText::Text;
            text->setFont("fonts/arial.ttf");
            text->setColor(colorDescription);
            text->setFontResolution((unsigned int)characterSize,(unsigned int)characterSize);
            text->setCharacterSize(characterSize);
            text->setPosition(posDescription);
            text->setMaximumWidth(maxWidthOfDisplayRegion);
            text->setAlignment(osgText::Text::BASE_LINE);
            text->setBackdropType(osgText::Text::OUTLINE);
            // Probably could use NO_DEPTH_BUFFER and not have any problems
            text->setBackdropImplementation(osgText::Text::DEPTH_RANGE);
            text->setBackdropColor(backdropColorDescription);
            text->setText(ocg->getApplicationUsage()->getDescription());

            bottomOfDescription = text->getBound().yMin()-characterSize*2.0f;

            _descriptionList.push_back(text);

        }

        osg::Vec3 posOption(0.0f,bottomOfDescription,0.0f);
        osg::Vec4 colorOption(1.0f,1.0f,0.0f,1.0f);
        osg::Vec4 backdropColorOption(0.0f,0.0f,0.0f,1.0f);
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
            text->setFontResolution((unsigned int)characterSize,(unsigned int)characterSize);
            text->setCharacterSize(characterSize);
            text->setPosition(posOption);
            text->setAlignment(osgText::Text::BASE_LINE);
            text->setBackdropType(osgText::Text::OUTLINE);
            // Probably could use NO_DEPTH_BUFFER and not have any problems
            text->setBackdropImplementation(osgText::Text::DEPTH_RANGE);
            text->setBackdropColor(backdropColorOption);
            text->setText(citr->first);

            if (text->getBound().xMax()>maxX) maxX=text->getBound().xMax();

            _optionList.push_back(text);

        }

        osg::Vec3 posExplanation(maxX+characterSize,bottomOfDescription,0.0f);
        osg::Vec4 colorExplanation(1.0f,1.0f,0.0f,1.0f);
        osg::Vec4 backdropColorExplanation(0.0f,0.0f,0.0f,1.0f);
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
            text->setFontResolution((unsigned int)characterSize,(unsigned int)characterSize);
            text->setCharacterSize(characterSize);
            text->setPosition(posExplanation);
            text->setMaximumWidth(maxWidth);
            text->setAlignment(osgText::Text::BASE_LINE);
            text->setBackdropType(osgText::Text::OUTLINE);
            // Probably could use NO_DEPTH_BUFFER and not have any problems
            text->setBackdropImplementation(osgText::Text::DEPTH_RANGE);
            text->setBackdropColor(backdropColorExplanation);
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

void ViewerEventHandler::StatsAndHelpDrawCallback::displayStats()
{
    if (!_statsInitialized) createStatsText();

    OsgCameraGroup* ocg = _veh->getOsgCameraGroup();
    OsgSceneHandler* osh = ocg->getSceneHandlerList()[_cameraNumber].get();
    osgUtil::SceneView* sv = osh->getSceneView();

    bool gpuTimingsValid = false;

    char tmpText[128];

    // render the text
    if (_veh->getFrameStatsMode()>=ViewerEventHandler::FRAME_RATE)
    {
        // update and draw the frame rate text.
        

        _frameRateLabelText->draw(*(sv->getState()));
        if (_fs.size()>1)
        {
            unsigned int lindex = (_index + 1) % _fs.size();
            double timeForFrames = (_fs[_index]._startOfFrame-_fs[lindex]._startOfFrame);
            double timePerFrame = timeForFrames/(double)(_fs.size()-1);
            sprintf(tmpText,"%4.2f",1.0/timePerFrame);
            _frameRateCounterText->setText(tmpText);
        }
        _frameRateCounterText->draw(*(sv->getState()));
        

        if (_veh->getFrameStatsMode()>=ViewerEventHandler::CAMERA_STATS)
        {


            double updateTime = 0.0;
            std::fill(_cullTimes.begin(),_cullTimes.end(),0.0);
            std::fill(_drawTimes.begin(),_drawTimes.end(),0.0);

            for(unsigned int frame = 0; frame < _fs.size(); ++frame )
            {
                Producer::CameraGroup::FrameStats fs = _fs[frame];
                updateTime += (fs._endOfUpdate-fs._startOfUpdate);

                for(unsigned int i = 0; i < fs.getNumFrameTimeStampSets(); ++i )
                {
                    Producer::Camera::FrameTimeStampSet fts = fs.getFrameTimeStampSet(i);

                    _cullTimes[i] += fts[Producer::Camera::EndCull]-fts[Producer::Camera::BeginCull];
                    _drawTimes[i] += fts[Producer::Camera::EndDraw]-fts[Producer::Camera::BeginDraw];
                }
            }

            sprintf(tmpText,"%4.2f",1000.0*updateTime/(double)_fs.size());
            _updateTimeText->setText(tmpText);

            _updateTimeText->draw(*(sv->getState()));

            TextList::iterator itr;
            CameraTimes::iterator titr;
            for(itr=_cullTimeText.begin(),titr = _cullTimes.begin();
                itr!=_cullTimeText.end() && titr!=_cullTimes.end();
                ++itr,++titr)
            {
                sprintf(tmpText,"%4.2f",1000.0*(*titr)/(double)_fs.size());
                (*itr)->setText(tmpText);
                (*itr)->draw(*(sv->getState()));
            }
            
            for(itr=_drawTimeText.begin(),titr = _drawTimes.begin();
                itr!=_drawTimeText.end() && titr!=_drawTimes.end();
                ++itr,++titr)
            {
                sprintf(tmpText,"%4.2f",1000.0*(*titr)/(double)_fs.size());
                (*itr)->setText(tmpText);
                (*itr)->draw(*(sv->getState()));
            }
            
            unsigned int i;
            for(itr=_gpuTimeText.begin(), i = 0;
                itr!=_gpuTimeText.end() && i < ocg->getNumberOfCameras();
                ++itr, ++i)
            {
                double gpuTime = ocg->getCamera(i)->getFrameStats().getPipeStats(Producer::Camera::DrawTime);
                if (gpuTime!=0.0)
                {
                    gpuTimingsValid = true;
                    sprintf(tmpText,"%4.2f", gpuTime);
                    (*itr)->setText(tmpText);
                    (*itr)->draw(*(sv->getState()));
                }
            }

            for(itr=_statsLabelList.begin();
                itr!=_statsLabelList.end();
                ++itr)
            {
                if (gpuTimingsValid || ((*itr)->getName()!="GPU"))
                {
                    (*itr)->draw(*(sv->getState()));
                }
            }
        }
        
        if (_veh->getFrameStatsMode()>=ViewerEventHandler::SCENE_STATS)
        {
            osgUtil::Statistics stats;

            for(osgProducer::OsgCameraGroup::SceneHandlerList::iterator shitr = _veh->getOsgCameraGroup()->getSceneHandlerList().begin();
                shitr != _veh->getOsgCameraGroup()->getSceneHandlerList().end();
                ++shitr)
            {
                (*shitr)->getStats(stats); 
            }

            unsigned int primitives = 0;
            for(osgUtil::Statistics::PrimitiveCountMap::iterator pcmitr = stats.GetPrimitivesBegin();
                pcmitr != stats.GetPrimitivesEnd();
                ++pcmitr)
            {
                primitives += pcmitr->second;
            }
            
            for(TextList::iterator itr = _sceneStatsLabelList.begin();
                itr != _sceneStatsLabelList.end();
                ++itr)
            {
                (*itr)->draw(*(sv->getState()));
            }
            
            sprintf(tmpText,"%d",stats._vertexCount);
            _numVerticesText->setText(tmpText);
            _numVerticesText->draw(*(sv->getState()));

            sprintf(tmpText,"%d",primitives);
            _numPrimitivesText->setText(tmpText);
            _numPrimitivesText->draw(*(sv->getState()));

            sprintf(tmpText,"%d",stats.numDrawables);
            _numDrawablesText->setText(tmpText);
            _numDrawablesText->draw(*(sv->getState()));
        } 
    }

    // render graphs
    if (_veh->getFrameStatsMode()>=ViewerEventHandler::CAMERA_STATS)
    {
    
        sv->getState()->applyTextureMode(0,GL_TEXTURE_2D,false);

        // Set up the Orthographic view
        glMatrixMode( GL_PROJECTION );
        glPushMatrix();
        glLoadIdentity();

        if (gpuTimingsValid)
        {
            glOrtho( -0.075, .128, 600.0, -20.0, -1.0, 1.0 ); 
        }
        else
        {
            glOrtho( -0.04, .128, 600.0, -20.0, -1.0, 1.0 ); 
        }


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

        glMatrixMode( GL_PROJECTION );
        glPopMatrix();
        glMatrixMode( GL_MODELVIEW );
        
    }
}

void ViewerEventHandler::StatsAndHelpDrawCallback::createStatsText()
{
    _statsInitialized = true;

    float characterSize = 20.0f;

    osg::Vec4 colorFR(1.0f,1.0f,1.0f,1.0f);
    osg::Vec4 colorUpdate( 0.0f,1.0f,0.0f,1.0f);
    osg::Vec4 colorCull( 0.0f,1.0f,1.0f,1.0f);
    osg::Vec4 colorDraw( 1.0f,1.0f,0.0f,1.0f);
    osg::Vec4 colorGPU( 1.0f,0.5f,0.0f,1.0f);
    
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
    pos.y() -= characterSize*1.5f;

    {
        osgText::Text* text = new osgText::Text;
        text->setFont("fonts/arial.ttf");
        text->setColor(colorUpdate);
        text->setFontResolution((unsigned int)characterSize,(unsigned int)characterSize);
        text->setCharacterSize(characterSize);
        text->setPosition(pos);
        text->setAlignment(osgText::Text::BASE_LINE);
        text->setText("Update: ");

        _statsLabelList.push_back(text);
        
        pos.x() = text->getBound().xMax();

        _updateTimeText = new osgText::Text;

        _updateTimeText->setFont("fonts/arial.ttf");
        _updateTimeText->setColor(colorUpdate);
        _updateTimeText->setFontResolution((unsigned int)characterSize,(unsigned int)characterSize);
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
        cullLabel->setFontResolution((unsigned int)characterSize,(unsigned int)characterSize);
        cullLabel->setCharacterSize(characterSize);
        cullLabel->setPosition(pos);
        cullLabel->setAlignment(osgText::Text::BASE_LINE);
        cullLabel->setText("Cull: ");

        _statsLabelList.push_back(cullLabel);
        
        pos.x() = cullLabel->getBound().xMax();

        osgText::Text* cullField = new osgText::Text;

        cullField->setFont("fonts/arial.ttf");
        cullField->setColor(colorCull);
        cullField->setFontResolution((unsigned int)characterSize,(unsigned int)characterSize);
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
        drawLabel->setFontResolution((unsigned int)characterSize,(unsigned int)characterSize);
        drawLabel->setCharacterSize(characterSize);
        drawLabel->setPosition(pos);
        drawLabel->setAlignment(osgText::Text::BASE_LINE);
        drawLabel->setText("Draw: ");

        _statsLabelList.push_back(drawLabel);
        
        pos.x() = drawLabel->getBound().xMax();

        osgText::Text* drawField = new osgText::Text;

        drawField->setFont("fonts/arial.ttf");
        drawField->setColor(colorDraw);
        drawField->setFontResolution((unsigned int)characterSize,(unsigned int)characterSize);
        drawField->setCharacterSize(characterSize);
        drawField->setPosition(pos);
        drawField->setAlignment(osgText::Text::BASE_LINE);
        drawField->setText("1000.00");
        
        _drawTimeText.push_back(drawField);

        _drawTimes.push_back(0.0);

        pos.x() = drawField->getBound().xMax();


        osgText::Text* gpuLabel = new osgText::Text;
        gpuLabel->setFont("fonts/arial.ttf");
        gpuLabel->setName("GPU");
        gpuLabel->setColor(colorGPU);
        gpuLabel->setFontResolution((unsigned int)characterSize,(unsigned int)characterSize);
        gpuLabel->setCharacterSize(characterSize);
        gpuLabel->setPosition(pos);
        gpuLabel->setAlignment(osgText::Text::BASE_LINE);
        gpuLabel->setText("GPU: ");

        _statsLabelList.push_back(gpuLabel);
        
        pos.x() = gpuLabel->getBound().xMax();

        osgText::Text* gpuField = new osgText::Text;

        gpuField->setFont("fonts/arial.ttf");
        gpuField->setColor(colorGPU);
        gpuField->setFontResolution((unsigned int)characterSize,(unsigned int)characterSize);
        gpuField->setCharacterSize(characterSize);
        gpuField->setPosition(pos);
        gpuField->setAlignment(osgText::Text::BASE_LINE);
        gpuField->setText("1000.00");
        
        _gpuTimeText.push_back(gpuField);

        pos.y() -= characterSize;
    }

    pos.y() -= characterSize*0.5f;

    // scene stats section
    {
        pos.x() = leftPos;

        // num drawables
        osgText::Text* drawLabel = new osgText::Text;
        drawLabel->setFont("fonts/arial.ttf");
        drawLabel->setColor(colorDraw);
        drawLabel->setFontResolution((unsigned int)characterSize,(unsigned int)characterSize);
        drawLabel->setCharacterSize(characterSize);
        drawLabel->setPosition(pos);
        drawLabel->setAlignment(osgText::Text::BASE_LINE);
        drawLabel->setText("Number of Drawables: ");

        _sceneStatsLabelList.push_back(drawLabel);
        
        pos.x() = drawLabel->getBound().xMax();

        _numDrawablesText = new osgText::Text;
        _numDrawablesText->setFont("fonts/arial.ttf");
        _numDrawablesText->setColor(colorDraw);
        _numDrawablesText->setFontResolution((unsigned int)characterSize,(unsigned int)characterSize);
        _numDrawablesText->setCharacterSize(characterSize);
        _numDrawablesText->setPosition(pos);
        _numDrawablesText->setAlignment(osgText::Text::BASE_LINE);
        _numDrawablesText->setText("100000  ");
        
        pos.x() = _numDrawablesText->getBound().xMax();

        // num vertices
        drawLabel = new osgText::Text;
        drawLabel->setFont("fonts/arial.ttf");
        drawLabel->setColor(colorDraw);
        drawLabel->setFontResolution((unsigned int)characterSize,(unsigned int)characterSize);
        drawLabel->setCharacterSize(characterSize);
        drawLabel->setPosition(pos);
        drawLabel->setAlignment(osgText::Text::BASE_LINE);
        drawLabel->setText("Number of Vertices: ");

        _sceneStatsLabelList.push_back(drawLabel);
        
        pos.x() = drawLabel->getBound().xMax();

        _numVerticesText = new osgText::Text;
        _numVerticesText->setFont("fonts/arial.ttf");
        _numVerticesText->setColor(colorDraw);
        _numVerticesText->setFontResolution((unsigned int)characterSize,(unsigned int)characterSize);
        _numVerticesText->setCharacterSize(characterSize);
        _numVerticesText->setPosition(pos);
        _numVerticesText->setAlignment(osgText::Text::BASE_LINE);
        _numVerticesText->setText("1000000  ");
        
        pos.x() = _numVerticesText->getBound().xMax();

    
        // num primitives
        drawLabel = new osgText::Text;
        drawLabel->setFont("fonts/arial.ttf");
        drawLabel->setColor(colorDraw);
        drawLabel->setFontResolution((unsigned int)characterSize,(unsigned int)characterSize);
        drawLabel->setCharacterSize(characterSize);
        drawLabel->setPosition(pos);
        drawLabel->setAlignment(osgText::Text::BASE_LINE);
        drawLabel->setText("Number of Primitives: ");

        _sceneStatsLabelList.push_back(drawLabel);
        
        pos.x() = drawLabel->getBound().xMax();

        _numPrimitivesText = new osgText::Text;
        _numPrimitivesText->setFont("fonts/arial.ttf");
        _numPrimitivesText->setColor(colorDraw);
        _numPrimitivesText->setFontResolution((unsigned int)characterSize,(unsigned int)characterSize);
        _numPrimitivesText->setCharacterSize(characterSize);
        _numPrimitivesText->setPosition(pos);
        _numPrimitivesText->setAlignment(osgText::Text::BASE_LINE);
        _numPrimitivesText->setText("1000000  ");
        
        pos.x() = _numPrimitivesText->getBound().xMax();
        


    }
}

void ViewerEventHandler::StatsAndHelpDrawCallback::displayInfo()
{
}

void ViewerEventHandler::StatsAndHelpDrawCallback::createInfoText()
{
}



ViewerEventHandler::ViewerEventHandler(OsgCameraGroup* cg):
    _cg(cg),
    _writeNodeFileName("saved_model.osg"),
    _displayHelp(false),
    _frameStatsMode(NO_STATS),
    _firstTimeTogglingFullScreen(true)
{
    Producer::CameraConfig* cfg = _cg->getCameraConfig();

    Producer::Camera *cam = cfg->getCamera(0);
    
    _statsAndHelpDrawCallback = new StatsAndHelpDrawCallback(this,0);
    cam->addPostDrawCallback(_statsAndHelpDrawCallback);

    for(unsigned int i=0;i<cfg->getNumberOfCameras();++i)
    {
        SnapImageDrawCallback* snapImageDrawCallback = new SnapImageDrawCallback();
        cfg->getCamera(i)->addPostDrawCallback(snapImageDrawCallback);
        _snapImageDrawCallbackList.push_back(snapImageDrawCallback);
    }
    
    
    Viewer* viewer = dynamic_cast<Viewer*>(cg);
    if (viewer) setWriteImageFileName(viewer->getWriteImageFileName());
    else setWriteImageFileName( Viewer::getDefaultImageFileName() );
}

void ViewerEventHandler::setWriteImageOnNextFrame(bool writeImageOnNextFrame)
{
    for(SnapImageDrawCallbackList::iterator itr=_snapImageDrawCallbackList.begin();
        itr!=_snapImageDrawCallbackList.end();
        ++itr)
    {
        (*itr)->setSnapImageOnNextFrame(writeImageOnNextFrame);
    }
}

void ViewerEventHandler::setWriteImageFileName(const std::string& filename)
{
    std::string basename = osgDB::getNameLessExtension(filename);
    std::string ext = osgDB::getFileExtension(filename);
    
    unsigned int cameraNum = 0;
    for(SnapImageDrawCallbackList::iterator itr=_snapImageDrawCallbackList.begin();
        itr!=_snapImageDrawCallbackList.end();
        ++itr, ++cameraNum)
    {
        if (cameraNum==0)
        {
            (*itr)->setFileName(filename);
        }
        else
        {
            std::string name(basename+"_");
            name += ('0'+cameraNum);
            name += '.';
            name += ext;
            (*itr)->setFileName(name);
        }
    }
}

void ViewerEventHandler::setFrameStatsMode(FrameStatsMode mode)
{ 
    _frameStatsMode = mode; 
    _cg->setInstrumentationMode(_frameStatsMode!=NO_STATS);

    for(osgProducer::OsgCameraGroup::SceneHandlerList::iterator shitr = _cg->getSceneHandlerList().begin();
        shitr != _cg->getSceneHandlerList().end();
        ++shitr)
    {
        (*shitr)->setCollectStats(_frameStatsMode==SCENE_STATS); 
    }
}

bool ViewerEventHandler::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& aa)
{
    if(!_cg) return false;

    if(ea.getEventType()==osgGA::GUIEventAdapter::KEYDOWN)
    {

        switch( ea.getKey() )
        {
            case 's' :
            {
                FrameStatsMode newFrameStatsMode = (FrameStatsMode)((_frameStatsMode+1)%4);
                setFrameStatsMode(newFrameStatsMode);
                return true;
            }
            case 'v' :
            {
                bool block = !_cg->getBlockOnVsync();
                _cg->setBlockOnVsync(block);
                for(unsigned int i=0;i<_cg->getNumberOfCameras();++i)
                {
                    _cg->getCamera(i)->setBlockOnVsync(block);
                }
                return true;
            }

            case 'f' :
            {
                Producer::CameraConfig* cfg = _cg->getCameraConfig();

                bool shouldBeFullScreen = false;
                bool flag = true;
                for( unsigned int i = 0; i < cfg->getNumberOfCameras(); ++i )
                {
                    Producer::Camera *cam = cfg->getCamera(i);
                    Producer::RenderSurface* rs = cam->getRenderSurface();
                    if( rs->getDrawableType() != Producer::RenderSurface::DrawableType_Window )
                        continue;
                  
                    if( flag )
                    {
                        shouldBeFullScreen =! rs->isFullScreen();    // Remember the initial state of the first render surface
                        flag = false;
                    }

                    if ( shouldBeFullScreen!=rs->isFullScreen() )        // If the current render surface hasn't been modified already
                    {
                        if (_firstTimeTogglingFullScreen && rs->isFullScreen())
                        {
                            unsigned int screenWidth;
                            unsigned int screenHeight;
                            unsigned int windowWidth;
                            unsigned int windowHeight;
                            rs->getScreenSize( screenWidth, screenHeight );
                            if( screenHeight > screenWidth )
                            {
                                windowWidth  = (unsigned int)((float)screenWidth * 0.625);
                                windowHeight = (unsigned int)((float)windowWidth * 0.75);
                            }
                            else
                            {
                                windowHeight = (unsigned int)((float)screenHeight * 0.625);
                                windowWidth  = (unsigned int)((float)windowHeight * 1.334);
                            }
                            int x = (screenWidth - windowWidth) >> 1;
                            int y = (screenHeight - windowHeight) >> 1;
                        #ifndef WIN32                    
                            rs->useBorder(true);
                        #else                        
                            rs->fullScreen(false);
                        #endif
                            rs->setWindowRectangle( x, y, windowWidth, windowHeight );
                        }
                        else
                        {
                            rs->fullScreen(!rs->isFullScreen());
                        }
                    }
                }
                _firstTimeTogglingFullScreen = false;

                return true;
            }

            case 'o' :
            {
                osg::Node* node = _cg->getSceneData();
                if (node)
                {
                    if (osgDB::writeNodeFile(*node,_writeNodeFileName.c_str()))
                    {
                        std::cout<<"written nodes to file "<<_writeNodeFileName<<std::endl;
                    }
                }

                return true;
            }

            case osgGA::GUIEventAdapter::KEY_Print :
            case 'O' :
            {
                setWriteImageOnNextFrame(true);                
                return true;
            }
#if 0
// Adjust osg::PolygonOffset
            case '+' :
            {
                osg::PolygonOffset::setFactorMultiplier(osg::PolygonOffset::getFactorMultiplier()*1.1f);
                osg::notify(osg::NOTICE)<<"osg::PolygonOffset::getFactorMultiplier()="<<osg::PolygonOffset::getFactorMultiplier()<<std::endl;
                return true;
            }
            case '-' :
            {
                osg::PolygonOffset::setFactorMultiplier(osg::PolygonOffset::getFactorMultiplier()/1.1f);
                osg::notify(osg::NOTICE)<<"osg::PolygonOffset::getFactorMultiplier()="<<osg::PolygonOffset::getFactorMultiplier()<<std::endl;
                return true;
            }

            case '*' :
            {
                osg::PolygonOffset::setUnitsMultiplier(osg::PolygonOffset::getUnitsMultiplier()*1.1f);
                osg::notify(osg::NOTICE)<<"osg::PolygonOffset::getUnitsMultiplier()="<<osg::PolygonOffset::getUnitsMultiplier()<<std::endl;
                return true;
            }
            case '/' :
            {
                osg::PolygonOffset::setUnitsMultiplier(osg::PolygonOffset::getUnitsMultiplier()/1.1f);
                osg::notify(osg::NOTICE)<<"osg::PolygonOffset::getUnitsMultiplier()="<<osg::PolygonOffset::getUnitsMultiplier()<<std::endl;
                return true;
            }
#else
// Adjust osg::LODScale

            case '*' :
            case '/' :
            {
                if (ea.getKey()=='*') _cg->setLODScale( _cg->getLODScale() * 1.1f);
                else _cg->setLODScale( _cg->getLODScale() / 1.1f);
                std::cout<<"New LOD Scale = "<<_cg->getLODScale()<<std::endl;
                return true;
            }
#endif
            case osgGA::GUIEventAdapter::KEY_Help :
            case 'h' :
            {
                setDisplayHelp(!getDisplayHelp());
                return true;
            }
            case 'Z' :
            case 'z' :
            {
                osgProducer::Viewer* viewer = dynamic_cast<osgProducer::Viewer*>(_cg);
                if (viewer)
                {
                    if (viewer->getRecordingAnimationPath())
                    {
                        // have already been recording so switch of recording.
                        viewer->setRecordingAnimationPath(false);

                        osg::notify(osg::NOTICE) << "finished recording camera animation, press 'Z' to replay."<< std::endl;

                        if (viewer->getAnimationPath())
                        {
                            std::ofstream fout("saved_animation.path");
                            viewer->getAnimationPath()->write(fout);
                            fout.close();

                            osg::notify(osg::NOTICE) << "Saved camera animation to 'saved_animation.path'"<< std::endl;

                        }

                    }
                    else if (ea.getKey()=='z') 
                    {
                        viewer->setRecordingAnimationPath(true);
                        viewer->setAnimationPath(new osg::AnimationPath());
                        viewer->getAnimationPath()->setLoopMode(osg::AnimationPath::LOOP);
                        osg::notify(osg::NOTICE) << "Recording camera animation, press 'z' to finish recording."<< std::endl;
                    }

                    if (ea.getKey()=='Z')
                    {
                        osgGA::AnimationPathManipulator* apm = 0;
                        unsigned int apmNo = 0;
                        
                        osgGA::KeySwitchMatrixManipulator* kscm = viewer->getKeySwitchMatrixManipulator();
                        if (kscm)
                        {
                            for(apmNo=0;apmNo<kscm->getNumMatrixManipulators();++apmNo)
                            {
                                apm = dynamic_cast<osgGA::AnimationPathManipulator*>(kscm->getMatrixManipulatorWithIndex(apmNo));
                                if (apm) break;
                            }
                        }

                        if (!apm)
                        {
                            apm = new osgGA::AnimationPathManipulator();
                            apmNo = viewer->addCameraManipulator(apm);
                        }

                        apm->setAnimationPath(viewer->getAnimationPath());
                        apm->home(ea,aa);

                        viewer->selectCameraManipulator(apmNo);
                    }

                    break;
                }
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
    usage.addKeyboardMouseBinding("o","Write scene graph to \"saved_model.osg\"");
    usage.addKeyboardMouseBinding("O PrtSrn","Write camera images to \"saved_image*.jpg\"");
    usage.addKeyboardMouseBinding("s","Toggle instrumention");
    usage.addKeyboardMouseBinding("v","Toggle block and vsync");
    usage.addKeyboardMouseBinding("z","Start recording camera path.");
    usage.addKeyboardMouseBinding("Z","If recording camera path stop recording camera path, save to \"saved_animation.path\"\nThen restart camera from beginning on animation path");
}
