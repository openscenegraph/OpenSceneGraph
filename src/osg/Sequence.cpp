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
    _last(-1.0f),
    _step(1),
    _loopMode(LOOP),
    _begin(0),
    _end(-1),
    _speed(0),
    _nreps(0),
    _nrepsremain(0),
    _mode(STOP)
{
    setNumChildrenRequiringUpdateTraversal(1);
}

Sequence::Sequence(const Sequence& seq, const CopyOp& copyop) :
    Group(seq, copyop),
    _value(seq._value),
    _last(seq._last),
    _frameTime(seq._frameTime),
    _step(seq._step),
    _loopMode(seq._loopMode),
    _begin(seq._begin),
    _end(seq._end),
    _speed(seq._speed),
    _nreps(seq._nreps),
    _nrepsremain(seq._nrepsremain),
    _mode(seq._mode)
{
    setNumChildrenRequiringUpdateTraversal(getNumChildrenRequiringUpdateTraversal()+1);            
}

void Sequence::setTime(int frame, float t)
{
    int sz = _frameTime.size();
    //cerr << "sz=" << sz << " frame=" << frame << endl;
    if (frame < sz)
        _frameTime[frame] = t;
    else
        for (int i = sz; i < (frame+1); i++) {
            _frameTime.push_back(t);
        }
}

float Sequence::getTime(int frame) const
{
    if (frame >= 0 && frame < (int) _frameTime.size())
        return _frameTime[frame];
    else
        return -1.0f;
}

void Sequence::setInterval(LoopMode mode, int begin, int end)
{
    _loopMode = mode;
    _begin = begin;
    _end = end;

    // switch to beginning of interval
    unsigned int nch = getNumChildren();
    begin = (_begin < 0 ? nch + _begin : _begin);
    end = (_end < 0 ? nch + _end : _end);

    setValue(begin);
    _step = (begin < end ? 1 : -1);
}

void Sequence::setDuration(float speed, int nreps)
{
    _speed = (speed <= 0.0f ? 0.0f : speed);
    _nreps = (nreps < 0 ? -1 : nreps);
    _nrepsremain = _nreps;
}

void Sequence::setMode(SequenceMode mode)
{
    switch (mode) {
    case START:
        // restarts sequence in 'traverse'
        setValue(-1);
        _mode = mode;
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
    // if app traversal update the frame count.
    if (nv.getVisitorType()==NodeVisitor::UPDATE_VISITOR && _mode == START && _nrepsremain)
    {
        const FrameStamp* framestamp = nv.getFrameStamp();
        if (framestamp)
        {
    
            double t = framestamp->getReferenceTime();
            if (_last == -1.0)
                _last = t;

            // first and last frame of interval
            unsigned int nch = getNumChildren();
            int begin = (_begin < 0 ? nch + _begin : _begin);
            int end = (_end < 0 ? nch + _end : _end);

            int sw = getValue();
            if (sw<0)
            {
                sw = begin;
                _step = (begin < end ? 1 : -1);
            }

            // default timeout for unset values
            if (sw >= (int) _frameTime.size())
            {
                setTime(sw, 1.0f);
            }

            // frame time-out?
            float dur = _frameTime[sw] * _speed;
            if ((t - _last) > dur) {

                sw += _step;

                // check interval
                int ibegin = (begin < end ? begin : end);
                int iend = (end > begin ? end : begin);
                //cerr << this << " interval " << ibegin << "," << iend << endl;

                if (sw < ibegin || sw > iend) {
                    // stop at last frame
                    if (sw < ibegin)
                        sw = ibegin;
                    else
                        sw = iend;

                    // repeat counter
                    if (_nrepsremain > 0)
                        _nrepsremain--;

                    if (_nrepsremain == 0) {
                        // stop
                        setMode(STOP);
                    }
                    else {
                        // wrap around
                        switch (_loopMode) {
                        case LOOP:
                            //cerr << this << " loop" << endl;
                            sw = begin;
                            break;
                        case SWING:
                            //cerr << this << " swing" << endl;
                            _step = -_step;
                            break;
                        }
                    }
                }

                _last = t;
            }
            setValue(sw);
        }
        else
        {
            osg::notify(osg::WARN) << "osg::Sequence::traverse(NodeVisitor&) requires a valid FrameStamp to function, sequence not updated.\n";
        }
    }
    
    // now do the traversal
    if (nv.getTraversalMode()==NodeVisitor::TRAVERSE_ACTIVE_CHILDREN)
    {
        if (getValue()>=0 && getValue()<(int)_children.size()) _children[getValue()]->accept(nv);
    }
    else
    {
        Group::traverse(nv);
    }
}
