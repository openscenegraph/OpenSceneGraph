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


void AnimationPath::read(std::istream& in)
{
    while (!in.eof())
    {
        double time;
        osg::Vec3 position;
        osg::Quat rotation;
        in >> time >> position.x() >> position.y() >> position.z() >> rotation.x() >> rotation.y() >> rotation.z() >> rotation.w();
        if(!in.eof())
            insert(time,osg::AnimationPath::ControlPoint(position,rotation));
    }
}

void AnimationPath::write(std::ostream& fout)
{
    const TimeControlPointMap& tcpm = getTimeControlPointMap();
    for(TimeControlPointMap::const_iterator tcpmitr=tcpm.begin();
        tcpmitr!=tcpm.end();
        ++tcpmitr)
    {
        const ControlPoint& cp = tcpmitr->second;
        fout<<tcpmitr->first<<" "<<cp._position<<" "<<cp._rotation<<std::endl;
    }
}

class AnimationPathCallbackVisitor : public NodeVisitor
{
    public:

        AnimationPathCallbackVisitor(const AnimationPath::ControlPoint& cp, bool useInverseMatrix):
            _cp(cp),
            _useInverseMatrix(useInverseMatrix) {}

        virtual void apply(MatrixTransform& mt)
        {
            Matrix matrix;
            if (_useInverseMatrix)
                _cp.getInverse(matrix);
            else
                _cp.getMatrix(matrix);
                
            mt.setMatrix(matrix);
        }
        
        virtual void apply(PositionAttitudeTransform& pat)
        {
            if (_useInverseMatrix)
            {
                Matrix matrix;
                _cp.getInverse(matrix);
                pat.setPosition(matrix.getTrans());
                pat.setAttitude(_cp._rotation.inverse());
                
            }
            else
            {
                pat.setPosition(_cp._position);
                pat.setAttitude(_cp._rotation);
            }
        }
        
        AnimationPath::ControlPoint _cp;
        bool _useInverseMatrix;      
};

void AnimationPathCallback::operator()(Node* node, NodeVisitor* nv)
{
    if (_animationPath.valid() && 
        nv->getVisitorType()==NodeVisitor::UPDATE_VISITOR && 
        nv->getFrameStamp())
    {
        double time = nv->getFrameStamp()->getReferenceTime();
        _latestTime = time;

        if (!_pause)
        {
            // Only update _firstTime the first time, when its value is still DBL_MAX
            if (_firstTime==DBL_MAX) _firstTime = time;
            update(*node);
        }
    }
    
    // must call any nested node callbacks and continue subgraph traversal.
    NodeCallback::traverse(node,nv);
}

void AnimationPathCallback::update(osg::Node& node)
{
    double animationTime = ((_latestTime-_firstTime)-_timeOffset)*_timeMultiplier;

    AnimationPath::ControlPoint cp;
    if (_animationPath->getInterpolatedControlPoint(animationTime,cp))
    {
        AnimationPathCallbackVisitor apcv(cp,_useInverseMatrix);
        node.accept(apcv);
    }
}


void AnimationPathCallback::reset()
{
    _firstTime = _latestTime;
    _pauseTime = _latestTime;
}

void AnimationPathCallback::setPause(bool pause)
{
    if (_pause==pause)
    {
        return;
    }
    
    _pause = pause;
    if (_pause)
    {
        _pauseTime = _latestTime;
    }
    else
    {
        _firstTime += (_latestTime-_pauseTime);
    }
}
