/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/
#include <osg/AnimationPath>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>

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
            
            time = getFirstTime()+(fraction_part*2.0) * getPeriod();
            break;
        }
        case(LOOP):
        {
            double modulated_time = (time - getFirstTime())/getPeriod();
            double fraction_part = modulated_time - floor(modulated_time);
            time = getFirstTime()+fraction_part * getPeriod();
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


class AnimationPathCallbackVisitor : public NodeVisitor
{
    public:

        AnimationPathCallbackVisitor(const AnimationPath::ControlPoint& cp):
            _cp(cp) {}

        virtual void apply(MatrixTransform& mt)
        {
            Matrix matrix;
            _cp.getMatrix(matrix);
            mt.setMatrix(matrix);
        }
        
        virtual void apply(PositionAttitudeTransform& pat)
        {
            pat.setPosition(_cp._position);
            pat.setAttitude(_cp._rotation);
        }
        
        AnimationPath::ControlPoint _cp;
      
};

void AnimationPathCallback::operator()(Node* node, NodeVisitor* nv)
{
    if (_animationPath.valid() && 
        nv->getVisitorType()==NodeVisitor::UPDATE_VISITOR && 
        nv->getFrameStamp())
    {
        double time = nv->getFrameStamp()->getReferenceTime();
        if (_firstTime==0.0) _firstTime = time;
        
        _animationTime = ((time-_firstTime)-_timeOffset)*_timeMultiplier;
        
        AnimationPath::ControlPoint cp;
        if (_animationPath->getInterpolatedControlPoint(_animationTime,cp))
        {
            AnimationPathCallbackVisitor apcv(cp);
            node->accept(apcv);
        }

    }

    // must call any nested node callbacks and continue subgraph traversal.
    NodeCallback::traverse(node,nv);
}
