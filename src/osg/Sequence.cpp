#include <osg/Sequence>

using namespace osg;

/**
 * Sequence constructor.
 */
Sequence::Sequence() :
    Switch(),
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
    setNumChildrenRequiringAppTraversal(1);
}

Sequence::Sequence(const Sequence& seq, const CopyOp& copyop) :
    Switch(seq, copyop),
    _last(seq._last),
    _step(seq._step),
    _loopMode(seq._loopMode),
    _begin(seq._begin),
    _end(seq._end),
    _speed(seq._speed),
    _nreps(seq._nreps),
    _nrepsremain(seq._nrepsremain),
    _mode(seq._mode)
{
    setNumChildrenRequiringAppTraversal(getNumChildrenRequiringAppTraversal()+1);            
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
        setValue(ALL_CHILDREN_OFF);
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
    if (nv.getVisitorType()==NodeVisitor::APP_VISITOR && _mode == START && _nrepsremain)
    {
        double t = nv.getFrameStamp()->getReferenceTime();
        if (_last == -1.0)
            _last = t;

        // first and last frame of interval
        unsigned int nch = getNumChildren();
        int begin = (_begin < 0 ? nch + _begin : _begin);
        int end = (_end < 0 ? nch + _end : _end);

        int sw = getValue();
        if (sw == ALL_CHILDREN_OFF || 
            sw == ALL_CHILDREN_ON ||
            sw == MULTIPLE_CHILDREN_ON ) {
            sw = begin;
            _step = (begin < end ? 1 : -1);
        }

        // default timeout for unset values
        if (sw >= (int) _frameTime.size()) {
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
        //cerr << this << " child=" << sw << endl;
        setValue(sw);
    }
    Switch::traverse(nv);
}
