#include <osg/AnimationPath>

using namespace osg;

void AnimationPath::insert(double time,const Key& key)
{
    _timeKeyMap[time] = key;
}

bool AnimationPath::getMatrix(double time,Matrix& matrix)
{
    if (_timeKeyMap.empty()) return false;

    TimeKeyMap::iterator second = _timeKeyMap.lower_bound(time);
    if (second==_timeKeyMap.begin())
    {
        second->second.getMatrix(matrix);
    }
    else if (second!=_timeKeyMap.end())
    {
        TimeKeyMap::iterator first = second;
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

bool AnimationPath::getInverse(double time,Matrix& matrix)
{
    if (_timeKeyMap.empty()) return false;

    TimeKeyMap::iterator second = _timeKeyMap.lower_bound(time);
    if (second==_timeKeyMap.begin())
    {
        second->second.getInverse(matrix);
    }
    else if (second!=_timeKeyMap.end())
    {
        TimeKeyMap::iterator first = second;
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
