#include <osg/AnimationPath>
#include <osg/NodeVisitor>

using namespace osg;

void AnimationPath::insert(double time,const Key& key)
{
    _timeKeyMap[time] = key;
}

bool AnimationPath::getMatrix(double time,Matrix& matrix) const
{
    if (_timeKeyMap.empty()) return false;

    TimeKeyMap::const_iterator second = _timeKeyMap.lower_bound(time);
    if (second==_timeKeyMap.begin())
    {
        second->second.getMatrix(matrix);
    }
    else if (second!=_timeKeyMap.end())
    {
        TimeKeyMap::const_iterator first = second;
        --first;        
        
        // we have both a lower bound and the next item.

        // deta_time = second.time - first.time
        double delta_time = second->first - first->first;

        if (delta_time==0.0)
            first->second.getMatrix(matrix);
        else
        {
            Key key;
            key.interpolate((time - first->first)/delta_time,
                            first->second,
                            second->second);
            key.getMatrix(matrix);
        }        
    }
    else // (second==_timeKeyMap.end())
    {
        _timeKeyMap.rbegin().base()->second.getMatrix(matrix);
    }
    return true;
}

bool AnimationPath::getInverse(double time,Matrix& matrix) const
{
    if (_timeKeyMap.empty()) return false;

    TimeKeyMap::const_iterator second = _timeKeyMap.lower_bound(time);
    if (second==_timeKeyMap.begin())
    {
        second->second.getInverse(matrix);
    }
    else if (second!=_timeKeyMap.end())
    {
        TimeKeyMap::const_iterator first = second;
        --first;        
        
        // we have both a lower bound and the next item.

        // deta_time = second.time - first.time
        double delta_time = second->first - first->first;

        if (delta_time==0.0)
            first->second.getInverse(matrix);
        else
        {
            Key key;
            key.interpolate((time - first->first)/delta_time,
                            first->second,
                            second->second);
            key.getInverse(matrix);
        }        
    }
    else // (second==_timeKeyMap.end())
    {
        _timeKeyMap.rbegin().base()->second.getInverse(matrix);
    }
    return true;
}


bool AnimationPath::getKeyFrame(double time,Key& key) const
{
    if (_timeKeyMap.empty()) return false;

    TimeKeyMap::const_iterator second = _timeKeyMap.lower_bound(time);
    if (second==_timeKeyMap.begin())
    {
        key = second->second;
    }
    else if (second!=_timeKeyMap.end())
    {
        TimeKeyMap::const_iterator first = second;
        --first;        
        
        // we have both a lower bound and the next item.

        // deta_time = second.time - first.time
        double delta_time = second->first - first->first;

        if (delta_time==0.0)
            key = first->second;
        else
        {
            key.interpolate((time - first->first)/delta_time,
                            first->second,
                            second->second);
        }        
    }
    else // (second==_timeKeyMap.end())
    {
        key = _timeKeyMap.rbegin().base()->second;
    }
    return true;
}
