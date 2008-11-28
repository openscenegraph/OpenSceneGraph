/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
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

#include <osg/Sequence>
#include <osg/Notify>

using namespace osg;

/**
 * Sequence constructor.
 */
Sequence::Sequence() :
    Group(),
    _value(-1),
    _now(0.0),
    _start(-1.0),
    _totalTime(0.),
    _resetTotalTime(true),
    _loopMode(LOOP),
    _begin(0),
    _end(-1),
    _speed(0),
    _nreps(-1),
    _nrepsRemain(0),
    _step(0),
    _defaultTime(1.),
    _lastFrameTime(0.),
    _saveRealLastFrameTime(-1.),
    _mode(STOP),
    _sync(false),
    _clearOnStop(false)
    
{
    setNumChildrenRequiringUpdateTraversal(1);
}

Sequence::Sequence(const Sequence& seq, const CopyOp& copyop) :
    Group(seq, copyop),
    _value(seq._value),
    _now(seq._now),
    _start(seq._start),
    _frameTime(seq._frameTime),
    _totalTime(seq._totalTime),
    _resetTotalTime(seq._resetTotalTime),
    _loopMode(seq._loopMode),
    _begin(seq._begin),
    _end(seq._end),
    _speed(seq._speed),
    _nreps(seq._nreps),
    _nrepsRemain(seq._nrepsRemain),
    _step(seq._step),
    _defaultTime(seq._defaultTime),
    _lastFrameTime(seq._lastFrameTime),
    _saveRealLastFrameTime(seq._saveRealLastFrameTime),
    _mode(seq._mode),
    _sync(seq._sync),
    _clearOnStop(seq._clearOnStop)
{
    setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);            
}

bool Sequence::addChild( Node *child)
{
    return Sequence::insertChild( _children.size(), child, _defaultTime);
}

bool Sequence::addChild( Node *child, double t)
{
    return Sequence::insertChild( _children.size(), child, t);
}

bool Sequence::insertChild( unsigned int index, Node *child)
{
    return Sequence::insertChild(index, child, _defaultTime);
}

bool Sequence::insertChild( unsigned int index, Node *child, double t)
{
    if (Group::insertChild(index,child))
    {
        if (index>=_frameTime.size())
        {
            Sequence::setTime(index, t);
        }
        _resetTotalTime = true;
        return true;
    }
    return false;
}

bool Sequence::removeChild( Node *child )
{
    if (Group::removeChild(child ))
    {
        unsigned int pos = getChildIndex(child);
        if (pos < _children.size())
            return Sequence::removeChildren(pos,1);
        else
            return false;
    }
    else
        return false;
}

bool Sequence::removeChildren(unsigned int pos,unsigned int numChildrenToRemove)
{
    if (pos<_frameTime.size())
        _frameTime.erase(_frameTime.begin()+pos, 
                         osg::minimum(_frameTime.begin()+(pos+numChildrenToRemove), 
                                      _frameTime.end()) );
    _resetTotalTime = true;
    return Group::removeChildren(pos,numChildrenToRemove);
}


// if frame >= _frameTime.size() then extend _frameTime to have frame-1 items
// a time <0 will get set to 0
void Sequence::setTime(unsigned int frame, double t)
{
    if (t<0.) t = 0.0;
    unsigned int sz = _frameTime.size();
    if (frame < sz)
    {
        _frameTime[frame] = t;
    }
    else
    {
        for (unsigned int i = sz; i <= frame; i++)
        {
            _frameTime.push_back(t);
        }
    }
  
}

// returns a frame time of -1 if frame is out of range
double Sequence::getTime(unsigned int frame) const
{
    if (frame < _frameTime.size())
        return _frameTime[frame];
    else
        return -1.0f;
}

void Sequence::setInterval(LoopMode mode, int begin, int end)
{
    _loopMode = mode;
    _end = end;
    _begin = begin;

    // _value based on _begin & _end
    _value = -1;

    _resetTotalTime = true;

}

void Sequence::setDuration(float speed, int nreps)
{
    _speed = speed;
    // -1 means loop forever
    _nreps = (nreps < 0 ? -1:nreps);
    // countdown of laps around the track
    _nrepsRemain = _nreps;
}

void Sequence::setMode(SequenceMode mode)
{
    int ubegin, uend;

    switch (mode) 
    {
    case START:
        // restarts sequence from beginning
        _value = -1;

        // Figure out which direction to start stepping the sequence
        ubegin = (_begin < 0 ?  (int)_frameTime.size()-1: _begin);
        uend = (_end < 0 ? (int)_frameTime.size()-1: _end);
        _step = (ubegin > uend ? -1 : 1);

        _start = -1.0;
        _mode = mode;
        if (_saveRealLastFrameTime>=0.)
        {
            _frameTime[_saveRealLastFrameValue] = _saveRealLastFrameTime;
            _saveRealLastFrameTime = -1.;
        }
        break;
    case STOP:
        _mode = mode;
        break;
    case PAUSE:
        if (_mode == START)
            _mode = PAUSE;
        break;
    case RESUME:
        if (_mode == PAUSE)
            _mode = START;
        break;
    }
}

void Sequence::traverse(NodeVisitor& nv)
{
    if (getNumChildren()==0) return;

    const FrameStamp* framestamp = nv.getFrameStamp();
    if (framestamp)
    {
        _now = framestamp->getSimulationTime();
    }


    if (nv.getVisitorType()==NodeVisitor::UPDATE_VISITOR && 
        _mode == START &&
        !_frameTime.empty() && getNumChildren()!=0)
    {
      
        // if begin or end < 0, make it last frame
        int _ubegin = (_begin < 0 ?  (int)_frameTime.size()-1: _begin);
        int _uend = (_end < 0 ? (int)_frameTime.size()-1: _end);

        int _sbegin = osg::minimum(_ubegin,_uend);
        int _send = osg::maximum(_ubegin,_uend);

        if (framestamp)
        {
            // hack for last frame time
            if (_lastFrameTime>0. && _nrepsRemain==1 && _saveRealLastFrameTime<0.)
            {
                if ( _loopMode == LOOP)
                {
                    if ((_step>0 && _value!=_send) || (_step<0 && _value!=_sbegin))
                    {
                        _saveRealLastFrameTime=_frameTime[_uend];
                        _saveRealLastFrameValue = _uend;
                        _frameTime[_uend] = _lastFrameTime;
                        _resetTotalTime = true;
                    }
                }
                else
                {
                    if (_step>0 && _value!=_sbegin)
                    {
                        _saveRealLastFrameTime=_frameTime[_send];
                        _saveRealLastFrameValue = _send;
                        _frameTime[_send] = _lastFrameTime;
                        _resetTotalTime = true;
                    }
                    else if (_step<0 && _value!=_send)
                    {
                        _saveRealLastFrameTime=_frameTime[_sbegin];
                        _saveRealLastFrameValue = _sbegin;
                        _frameTime[_sbegin] = _lastFrameTime;
                        _resetTotalTime = true;
                    }
                }
            }

            // I never know when to stop!
            // more fun for last frame time
            if (_nrepsRemain==0)
            {
                if (!_clearOnStop)
                {
                    _mode = STOP;
                }
                else
                {
                    if ( (_loopMode == LOOP) && 
                         ( (_step>0 && _value!=_send) || 
                           (_step<0 && _value!=_sbegin)))
                    {
                        _mode = STOP;
                    }
                    else if ( (_loopMode == SWING) &&
                              ( (_step<0 && _value!=_send) ||
                                (_step>0 && _value!=_sbegin)))
                    {
                        _mode = STOP;
                    }

                }
            }
          
            // update local variables
            _update();


            // now for the heavy lifting! three options
            // 1) still in the same frame, so have nothing to do
            // 2) just in the next frame
            // 3) need to calculate everything based on elapsed time
            if ((_now - _start) > _frameTime[_value]*osg::absolute(_speed))
            { // case 2 or case 3
                // most of the time it's just the next frame in the sequence
                int nextValue = _getNextValue();
                if (!_sync || 
                    ((_now - _start) <= (_frameTime[_value]+_frameTime[nextValue])*osg::absolute(_speed)) )
                {
                    _start += _frameTime[_value]*osg::absolute(_speed);
                    // repeat or change directions?
                    if ( (_step>0 && nextValue==_send) || 
                         (_step<0 && nextValue==_sbegin))
                    {
                        if (_nreps>0)
                            _nrepsRemain--;

                        // change direction
                        if  (_loopMode == SWING)
                            _step = -_step;

                    }
                    _value = nextValue;
                }
                else // case 3
        {
            // recalculate everything based on elapsed time

            // elapsed time from start of the frame
            double deltaT = _now - _start;
            
            // factors _speed into account
            double adjTotalTime = _totalTime*osg::absolute(_speed);
            
            // how many laps?
            int loops = (int)(deltaT/adjTotalTime);
            
            
            // adjust reps & quick check to see if done becuase reps used up
            
            if (_nreps>0)
            {
            if (_loopMode == LOOP)
                _nrepsRemain -= loops;
            else
                _nrepsRemain -= 2*loops;
            
            if (_nrepsRemain<=0)
            {
                _nrepsRemain = 0;
                _mode = STOP;
                osg::notify(osg::WARN) << "stopping because elapsed time greater or equal to time remaining to repeat the sequence\n";
            }
            }
            
            // deduct off time for laps- _value shouldn't change as it's modulo the total time
            double jumpStart = ((double)loops * adjTotalTime);
            
            // step through frames one at a time until caught up
            while (deltaT-jumpStart > _frameTime[_value]*osg::absolute(_speed))
            {
            jumpStart +=  _frameTime[_value]*osg::absolute(_speed );
            _value = _getNextValue();
            }
            
            // set start time
            _start += jumpStart;
        }
            }
        }
        else
            osg::notify(osg::WARN) << "osg::Sequence::traverse(NodeVisitor&) requires a valid FrameStamp to function, sequence not updated.\n";

    }

    // now do the traversal
    if (nv.getTraversalMode()==NodeVisitor::TRAVERSE_ACTIVE_CHILDREN)
    {
        if ( !((_mode == STOP) && _clearOnStop) &&
             (getValue()>=0 && getValue()<(int)_children.size()) )
        {
            _children[getValue()]->accept(nv);
        }
    }
    else
    {
        Group::traverse(nv);
    }

}

int Sequence::_getNextValue()
{
    if (_frameTime.empty() || getNumChildren()==0) return 0;

    // if begin or end < 0, make it last frame
    int _ubegin = (_begin < 0 ?  (int)_frameTime.size()-1: _begin);
    int _uend = (_end < 0 ? (int)_frameTime.size()-1: _end);
    
    int _sbegin = osg::minimum(_ubegin,_uend);
    int _send = osg::maximum(_ubegin,_uend);

    int v = _value + _step;

    if (_sbegin==_send)
    {
        return _sbegin;
    }
    else if (v<=_send && v>=_sbegin)
    {
        return v;
    }
    else
    {
        int vs = _send - _sbegin + 1;
        if (_loopMode == LOOP)
        {
            v = ((v-_sbegin)%vs) + _sbegin;
            if (v<_sbegin)
            {
                v+=vs;
            }
            
            return v;
        }
        else // SWING
        {
            if (v>_send)
                return (2*_send-v);
            else
                return (2*_sbegin-v);

        }
    }

}

void Sequence::_update()
{
    if (_frameTime.empty()) return;

    // if begin or end < 0, make it last frame
    int _ubegin = (_begin < 0 ?  (int)_frameTime.size()-1: _begin);
    int _uend = (_end < 0 ? (int)_frameTime.size()-1: _end);
    
    int _sbegin = osg::minimum(_ubegin,_uend);
    int _send = osg::maximum(_ubegin,_uend);

    // if _value<0, new or restarted
    if (_value<0)
    {
        _value = (_begin < 0 ?  (int)_frameTime.size()-1: _begin);
        _resetTotalTime = true;
    }
  
    // if _start<0, new or restarted
    if (_start<0)
    {
        _start = _now;
        _resetTotalTime = true;
    }
  
    // need to calculate time of a complete sequence?
    // time is different depending on loop mode
    if (_resetTotalTime)
    {
        if (_loopMode == LOOP)
        {
            _totalTime = 0.0;
            for (int i=_sbegin; i<=_send; i++)
            {
                _totalTime += _frameTime[i];
            }
        }
        else //SWING
        {
            _totalTime = _frameTime[_sbegin];
            // ones in the middle get counted twice: 0 1 2 3 4 3 2 1 0
            for (int i=_sbegin+1; i<_send; i++)
            {
                _totalTime += 2*_frameTime[i];
            }
            if (_sbegin != _send)
            {
                _totalTime += _frameTime[_send];
            }
        }

        _resetTotalTime = false;
    }
  
}
