#include <osg/DOFTransform>

using namespace osg;

struct DOFCullCallback : public Transform::ComputeTransformCallback
{
    /**/
    virtual const bool computeLocalToWorldMatrix(Matrix& matrix, const Transform* trans, NodeVisitor* nv) const
    {
        const DOFTransform* dof = dynamic_cast<const DOFTransform*>(trans);
        if(dof)
        {
            //here we are:
            //put the PUT matrix first:
            Matrix l2w(dof->getPutMatrix());

            //now the current matrix:
            Matrix current; 
            current.makeTranslate(dof->getCurrentTranslate());

            //now create the local rotation:
            current.preMult(Matrix::rotate(dof->getCurrentHPR()[1], 1.0, 0.0, 0.0));//pitch
            current.preMult(Matrix::rotate(dof->getCurrentHPR()[2], 0.0, 1.0, 0.0));//roll
            current.preMult(Matrix::rotate(dof->getCurrentHPR()[0], 0.0, 0.0, 1.0));//heading

            //and scale:
            current.preMult(Matrix::scale(dof->getCurrentScale()));

            l2w.postMult(current);

            //and impose inverse put:
            l2w.postMult(dof->getInversePutMatrix());

            //finally:
            matrix.preMult(l2w);
        }

        return true;
    }

    /**/
    virtual const bool computeWorldToLocalMatrix(Matrix& matrix, const Transform* trans, NodeVisitor* nv) const
    {
        const DOFTransform* dof = dynamic_cast<const DOFTransform*>(trans);
        if(dof)
        {
            //here we are:
            //put the PUT matrix first:
            Matrix w2l(dof->getInversePutMatrix());

            //now the current matrix:
            Matrix current; 
            current.makeTranslate(-dof->getCurrentTranslate());

            //now create the local rotation:


            current.postMult(Matrix::rotate(-dof->getCurrentHPR()[0], 0.0, 0.0, 1.0));//heading
            current.postMult(Matrix::rotate(-dof->getCurrentHPR()[2], 0.0, 1.0, 0.0));//roll
            current.postMult(Matrix::rotate(-dof->getCurrentHPR()[1], 1.0, 0.0, 0.0));//pitch

            //and scale:
            current.postMult(Matrix::scale(1./dof->getCurrentScale()[0], 1./dof->getCurrentScale()[1], 1./dof->getCurrentScale()[2]));

            w2l.postMult(current);

            //and impose inverse put:
            w2l.postMult(dof->getPutMatrix());

            //finally:
            matrix.postMult(w2l);
        }

        return true;
    }
};

class DOFAnimationAppCallback : public osg::NodeCallback
{
    /** Callback method call by the NodeVisitor when visiting a node.*/
    virtual void operator()(Node* node, NodeVisitor* nv)
    {
        if (nv && nv->getVisitorType()==NodeVisitor::APP_VISITOR)
        {

            //this is aspp visitor, see if we have DOFTransform:
            DOFTransform* dof = dynamic_cast<DOFTransform*>(node);
            if(dof)
            {
                //see if it is ina animation mode:
                dof->animate();
            }

        }
        
        // note, callback is repsonsible for scenegraph traversal so
        // should always include call the traverse(node,nv) to ensure 
        // that the rest of cullbacks and the scene graph are traversed.
        traverse(node,nv);
    }
};

DOFTransform::DOFTransform():
    _limitationFlags(0), 
    _animationOn(true), 
    _increasingFlags(0xffff)
{
    //default zero-ed Vec3-s are fine for all
    //default idenetiy inverse matrix is fine:

    _computeTransformCallback = new DOFCullCallback;

    DOFAnimationAppCallback* dof_animation = new DOFAnimationAppCallback;

    setAppCallback(dof_animation);
}

void DOFTransform::setCurrentHPR(const Vec3& hpr)
{
    //if there is no constrain on animation
    if(!(_limitationFlags & ((unsigned long)1<<26)))
    {
        //if we have min == max, it is efective constrain, so don't change 
        if(_minHPR[2] != _maxHPR[2])
        {
            _currentHPR[2] = hpr[2];
            unsigned short this_flag = (unsigned short)1<<4;//roll

            if(_currentHPR[2] < _minHPR[2])
            {
                _currentHPR[2] = _minHPR[2]; 
                //force increasing flag to 1
                _increasingFlags |= this_flag;
            }
            else if(_currentHPR[2] > _maxHPR[2])
            {
                _currentHPR[2] = _maxHPR[2];
                //force increasing flag to 0
                _increasingFlags &= ~this_flag;
            }
        }
    }


    if(!(_limitationFlags & ((unsigned long)1<<27)))
    {
        if(_minHPR[1] != _maxHPR[1])
        {
            _currentHPR[1] = hpr[1];
            unsigned short this_flag = (unsigned short)1<<3;//pitch

            if(_currentHPR[1] < _minHPR[1])
            {
                _currentHPR[1] = _minHPR[1];
                _increasingFlags |= this_flag;
            }
            else if(_currentHPR[1] > _maxHPR[1])
            {
                _currentHPR[1] = _maxHPR[1];
                _increasingFlags &= ~this_flag;
            }
        }
    }


    if(!(_limitationFlags & ((unsigned long)1<<28)))
    {
        if(_minHPR[0] != _maxHPR[0])
        {
            _currentHPR[0] = hpr[0];

            unsigned short this_flag = (unsigned short)1<<5;//heading

            if(_currentHPR[0] < _minHPR[0]) 
            {
                _currentHPR[0] = _minHPR[0];
                _increasingFlags |= this_flag;
            }
            else if(_currentHPR[0] > _maxHPR[0]) 
            {
                _currentHPR[0] = _maxHPR[0];
                _increasingFlags &= ~this_flag;
            }
        }
    }
}

void DOFTransform::setCurrentTranslate(const Vec3& translate)
{
    if(!(_limitationFlags & (unsigned long)1<<29))
    {
        if(_minTranslate[2] != _maxTranslate[2])
        {
            _currentTranslate[2] = translate[2];
            unsigned short this_flag = (unsigned short)1<<2;

            if(_currentTranslate[2] < _minTranslate[2])
            {
                _currentTranslate[2] = _minTranslate[2];
                _increasingFlags |= this_flag;
            }
            else if(_currentTranslate[2] > _maxTranslate[2]) 
            {
                _currentTranslate[2] = _maxTranslate[2];
                _increasingFlags &= ~this_flag;
            }
        }
    }

    if(!(_limitationFlags & (unsigned long)1<<30))
    {
        if(_minTranslate[1] != _maxTranslate[1])
        {
            _currentTranslate[1] = translate[1];
            unsigned short this_flag = (unsigned short)1<<1;

            if(_currentTranslate[1] < _minTranslate[1])
            {
                _currentTranslate[1] = _minTranslate[1];
                _increasingFlags |= this_flag;
            }
            else if(_currentTranslate[1] > _maxTranslate[1]) 
            {
                _currentTranslate[1] = _maxTranslate[1];
                _increasingFlags &= ~this_flag;
            }
        }
    }

    if(!(_limitationFlags & (unsigned long)1<<31))
    {
        if(_minTranslate[0] != _maxTranslate[0])
        {
            _currentTranslate[0] = translate[0];
            unsigned short this_flag = (unsigned short)1;

            if(_currentTranslate[0] < _minTranslate[0]) 
            {
                _currentTranslate[0] = _minTranslate[0];
                _increasingFlags |= this_flag;
            }
            else if(_currentTranslate[0] > _maxTranslate[0]) 
            {
                _currentTranslate[0] = _maxTranslate[0];
                _increasingFlags &= ~this_flag;
            }
        }
    }    
}

void DOFTransform::setCurrentScale(const Vec3& scale)
{

    if(!(_limitationFlags & ((unsigned long)1<<23)))
    {
        if(_minScale[2] != _maxScale[2])
        {            
            _currentScale[2] = scale[2];
            unsigned short this_flag = (unsigned short)1<<8;

            if(_currentScale[2] < _minScale[2]) 
            {
                _currentScale[2] = _minScale[2];
                _increasingFlags |= this_flag;
            }
            else if(_currentScale[2] > _maxScale[2]) 
            {
                _currentScale[2] = _maxScale[2];
                _increasingFlags &= ~this_flag;
            }
        }
    }

    if(!(_limitationFlags & ((unsigned long)1<<24)))
    {
        if(_minScale[1] != _maxScale[1])
        {
            _currentScale[1] = scale[1];
            unsigned short this_flag = (unsigned short)1<<7;

            if(_currentScale[1] < _minScale[1]) 
            {
                _currentScale[1] = _minScale[1];
                _increasingFlags |= this_flag;
            }
            else if(_currentScale[1] > _maxScale[1]) 
            {
                _currentScale[1] = _maxScale[1];
                _increasingFlags &= ~this_flag;
            }
        }
    }

    if(!(_limitationFlags & ((unsigned long)1<<25)))
    {
        if(_minScale[0] != _maxScale[0])
        {
            _currentScale[0] = scale[0];
            unsigned short this_flag = (unsigned short)1<<6;

            if(_currentScale[0] < _minScale[0]) 
            {
                _currentScale[0] = _minScale[0];
                _increasingFlags |= this_flag;
            }
            else if(_currentScale[0] > _maxScale[0]) 
            {
                _currentScale[0] = _maxScale[0];
                _increasingFlags &= ~this_flag;
            }
        }
    }    
}

void DOFTransform::animate()
{
    if(!_animationOn) return;
    //this will increment or decrement all allowed values

    Vec3 new_value = _currentTranslate;

    if(_increasingFlags & 1)
        new_value[0] += _incrementTranslate[0];
    else
        new_value[0] -= _incrementTranslate[0];

    if(_increasingFlags & 1<<1)
        new_value[1] += _incrementTranslate[1];
    else
        new_value[1] -= _incrementTranslate[1];

    if(_increasingFlags & 1<<2)
        new_value[2] += _incrementTranslate[2];
    else
        new_value[2] -= _incrementTranslate[2];

    setCurrentTranslate(new_value);

    new_value = _currentHPR;

    if(_increasingFlags & ((unsigned short)1<<3))
        new_value[1] += _incrementHPR[1];
    else
        new_value[1] -= _incrementHPR[1];

    if(_increasingFlags & ((unsigned short)1<<4))
        new_value[2] += _incrementHPR[2];
    else
        new_value[2] -= _incrementHPR[2];

    if(_increasingFlags & ((unsigned short)1<<5))
        new_value[0] += _incrementHPR[0];
    else
        new_value[0] -= _incrementHPR[0];

    setCurrentHPR(new_value);

    new_value = _currentScale;

    if(_increasingFlags & 1<<6)
        new_value[0] += _incrementScale[0];
    else
        new_value[0] -= _incrementScale[0];

    if(_increasingFlags & 1<<7)
        new_value[1] += _incrementScale[1];
    else
        new_value[1] -= _incrementScale[1];

    if(_increasingFlags & 1<<8)
        new_value[2] += _incrementScale[2];
    else
        new_value[2] -= _incrementScale[2];

    setCurrentScale(new_value);
    
    dirtyBound();
}
