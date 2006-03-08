#include <osgGA/AnimationPathManipulator>

#include <fstream>

using namespace osgGA;

AnimationPathManipulator::AnimationPathManipulator(osg::AnimationPath* animationPath) 
{
    _printOutTiminInfo = true;

    _animationPath = animationPath;
    _timeOffset = 0.0;
    _timeScale = 1.0;
    _isPaused = false;
    
    _realStartOfTimedPeriod = 0.0;
    _animStartOfTimedPeriod = 0.0;
    _numOfFramesSinceStartOfTimedPeriod = -1; // need to init.
}

AnimationPathManipulator::AnimationPathManipulator( const std::string& filename ) 
{
    _printOutTiminInfo = true;

    _animationPath = new osg::AnimationPath;
    _animationPath->setLoopMode(osg::AnimationPath::LOOP);
    _timeOffset = 0.0f;
    _timeScale = 1.0f;
    _isPaused = false;


    std::ifstream in(filename.c_str());

    if (!in)
    {
        osg::notify(osg::WARN) << "AnimationPathManipulator: Cannot open animation path file \"" << filename << "\".\n";
        _valid = false;
        return;
    }

    _animationPath->read(in);

    in.close();
    
}

void AnimationPathManipulator::home(double currentTime)
{
    if (_animationPath.valid())
    {
        _timeOffset = _animationPath->getFirstTime()-currentTime; 

    }
    // reset the timing of the animation.
    _numOfFramesSinceStartOfTimedPeriod=-1;
}

void AnimationPathManipulator::home(const GUIEventAdapter& ea,GUIActionAdapter&)
{
    home(ea.getTime());
}

void AnimationPathManipulator::init(const GUIEventAdapter& ea,GUIActionAdapter& aa)
{
    home(ea,aa);
}

bool AnimationPathManipulator::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us)
{
    if( !valid() ) return false;

    switch( ea.getEventType() )
    {
    case GUIEventAdapter::FRAME:
        if( _isPaused )
        {
            handleFrame( _pauseTime );
        }
        else
        {
            handleFrame( ea.getTime() );
        }
        return false;
    case GUIEventAdapter::KEYDOWN:
            if (ea.getKey()==' ')
            {
                _isPaused = false;
                home(ea,us);
                us.requestRedraw();
                us.requestContinuousUpdate(false);
                
                return true;
            } 
            else if(ea.getKey() == 'p')
            {
                if( _isPaused )
                {
                    _isPaused = false;
                    _timeOffset -= ea.getTime() - _pauseTime;
                }
                else
                {
                    _isPaused = true;
                    _pauseTime = ea.getTime();
                }
                us.requestRedraw();
                us.requestContinuousUpdate(false);
                return true;
            }
                
        break;
        default:
            break;
    }
    return false;
}

void AnimationPathManipulator::getUsage(osg::ApplicationUsage& usage) const
{
    usage.addKeyboardMouseBinding("AnimationPath: Space","Reset the viewing position to start of animation");
    usage.addKeyboardMouseBinding("AnimationPath: p","Pause/resume animation.");
}

void AnimationPathManipulator::handleFrame( double time )
{
    osg::AnimationPath::ControlPoint cp;
    
    double animTime = (time+_timeOffset)*_timeScale;
    _animationPath->getInterpolatedControlPoint( animTime, cp );

    if (_numOfFramesSinceStartOfTimedPeriod==-1)
    {    
        _realStartOfTimedPeriod = time;
        _animStartOfTimedPeriod = animTime;

    }
    
    ++_numOfFramesSinceStartOfTimedPeriod;
    
    if (_printOutTiminInfo)
    {
        double delta = (animTime-_animStartOfTimedPeriod);
        if (delta>=_animationPath->getPeriod())
        {
            double frameRate = (double)_numOfFramesSinceStartOfTimedPeriod/delta;
            osg::notify(osg::NOTICE) <<"AnimatonPath completed in "<<delta<<" seconds, completing "<<_numOfFramesSinceStartOfTimedPeriod<<" frames,"<<std::endl;
            osg::notify(osg::NOTICE) <<"             average frame rate = "<<frameRate<<std::endl;

            // reset counters for next loop.
            _realStartOfTimedPeriod = time;
            _animStartOfTimedPeriod = animTime;

            _numOfFramesSinceStartOfTimedPeriod = 0;  
        }
    }
    
    cp.getMatrix( _matrix );
}
