#include <osgGA/AnimationPathManipulator>
#include <stdio.h>

using namespace osgGA;

AnimationPathManipulator::AnimationPathManipulator(osg::AnimationPath* animationPath) 
{
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
    _animationPath = new osg::AnimationPath;
    _animationPath->setLoopMode(osg::AnimationPath::LOOP);
    _timeOffset = 0.0f;
    _timeScale = 1.0f;
    _isPaused = false;

    FILE *fp = fopen( filename.c_str(), "r" );
    if( fp == NULL )
    {
        osg::notify(osg::WARN) << "AnimationPathManipulator: Cannot open animation path file \"" << filename << "\".\n";
        _valid = false;
        return;
    }
    while( !feof( fp ))
    {
        double time;
            osg::Vec3 position;
            osg::Quat rotation;
        fscanf( fp, "%lf %f %f %f %f %f %f %f\n",
            &time, &position[0], &position[1], &position[2],
            &rotation[0], &rotation[1], &rotation[2], &rotation[3] );

        if( !feof(fp))
                _animationPath->insert(time,osg::AnimationPath::ControlPoint(position,rotation));
    }
    fclose(fp);
}

void AnimationPathManipulator::home(const GUIEventAdapter& ea,GUIActionAdapter&)
{
    if (_animationPath.valid())
    {
        _timeOffset = _animationPath->getFirstTime()-ea.time();

    }

    // reset the timing of the animation.
    _numOfFramesSinceStartOfTimedPeriod=-1;
                
}

void AnimationPathManipulator::init(const GUIEventAdapter& ea,GUIActionAdapter& aa)
{
    home(ea,aa);
}

bool AnimationPathManipulator::handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter& us)
{
    if( !valid() ) return false;

    us = us;

    bool retval = false;
    switch( ea.getEventType() )
    {
    case GUIEventAdapter::FRAME:
        if( _isPaused )
        {
            handleFrame( _pauseTime );
        }
        else
        {
            handleFrame( ea.time() );
        }
            retval =  true;
        break;
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
                    _timeOffset -= ea.time() - _pauseTime;
                }
                else
                {
                    _isPaused = true;
                    _pauseTime = ea.time();
                }
                us.requestRedraw();
                us.requestContinuousUpdate(false);
                return true;
            }
                
        retval =  false;
        break;
        default:
            break;
    }
    return retval;
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
    
    double delta = (animTime-_animStartOfTimedPeriod);
    if (delta>=_animationPath->getPeriod())
    {
        double frameRate = (double)_numOfFramesSinceStartOfTimedPeriod/delta;
        osg::notify(osg::INFO) <<"AnimatonPath completed in "<<delta<<" seconds, completing "<<_numOfFramesSinceStartOfTimedPeriod<<" frames,"<<std::endl;
        osg::notify(osg::INFO) <<"             average frame rate = "<<frameRate<<std::endl;
        
        // reset counters for next loop.
        _realStartOfTimedPeriod = time;
        _animStartOfTimedPeriod = animTime;
        
        _numOfFramesSinceStartOfTimedPeriod = 0;  
    }

    cp.getMatrix( _matrix );
}
