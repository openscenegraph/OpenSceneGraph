#include <osg/AnimationPath>
#include <osg/NodeVisitor>

using namespace osg;

void AnimationPath::insert(double time,const ControlPoint& controlPoint)
{
    _timeControlPointMap[time] = controlPoint;
}

bool AnimationPath::getInterpolatedControlPoint(double time,ControlPoint& controlPoint) const
{
    if (_timeControlPointMap.empty()) return false;
    
    switch(_loopMode)
    {
        case(SWING):
        {
            double modulated_time = (time - getFirstTime())/(getPeriod()*2.0);
            double fraction_part = modulated_time - floor(modulated_time);
            if (fraction_part>0.5) fraction_part = 1.0-fraction_part;
            
            time = (fraction_part*2.0) * getPeriod();
            break;
        }
        case(LOOP):
        {
            double modulated_time = (time - getFirstTime())/getPeriod();
            double fraction_part = modulated_time - floor(modulated_time);
            time = fraction_part * getPeriod();
            break;
        }
        case(NO_LOOPING):
            // no need to modulate the time.
            break;
    }
    
    

    TimeControlPointMap::const_iterator second = _timeControlPointMap.lower_bound(time);
    if (second==_timeControlPointMap.begin())
    {
        controlPoint = second->second;
    }
    else if (second!=_timeControlPointMap.end())
    {
        TimeControlPointMap::const_iterator first = second;
        --first;        
        
        // we have both a lower bound and the next item.

        // deta_time = second.time - first.time
        double delta_time = second->first - first->first;

        if (delta_time==0.0)
            controlPoint = first->second;
        else
        {
            controlPoint.interpolate((time - first->first)/delta_time,
                            first->second,
                            second->second);
        }        
    }
    else // (second==_timeControlPointMap.end())
    {
        controlPoint = _timeControlPointMap.rbegin()->second;
    }
    return true;
}
