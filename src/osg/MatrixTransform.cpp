#include <osg/MatrixTransform>

using namespace osg;

MatrixTransform::MatrixTransform():
    _inverseDirty(false)
{
    _matrix = osgNew Matrix;
    _inverse = osgNew Matrix;
}

MatrixTransform::MatrixTransform(const MatrixTransform& transform,const CopyOp& copyop):
    Transform(transform,copyop),
    _matrix(osgNew Matrix(*transform._matrix)),
    _inverse(osgNew Matrix(*transform._inverse)),
    _inverseDirty(transform._inverseDirty),
    _animationPath(dynamic_cast<AnimationPath*>(copyop(transform._animationPath.get())))
{    
    if (_animationPath.valid()) setNumChildrenRequiringAppTraversal(getNumChildrenRequiringAppTraversal()+1);            
}

MatrixTransform::MatrixTransform(const Matrix& mat )
{
    _referenceFrame = RELATIVE_TO_PARENTS;

    _matrix = osgNew Matrix(mat);
    _inverse = osgNew Matrix();
    _inverseDirty = false;
}


MatrixTransform::~MatrixTransform()
{
}


void MatrixTransform::traverse(NodeVisitor& nv)
{
    // if app traversal update the frame count.
    if (_animationPath.valid() && 
        nv.getVisitorType()==NodeVisitor::APP_VISITOR && 
        nv.getFrameStamp())
    {
        double time = nv.getFrameStamp()->getReferenceTime();
        _animationPath->getMatrix(time,*_matrix);
    }
    
    // must call any nested node callbacks and continue subgraph traversal.
    MatrixTransform::traverse(nv);
}

void MatrixTransform::AnimationPathCallback::operator()(Node* node, NodeVisitor* nv)
{
    MatrixTransform* mt = dynamic_cast<MatrixTransform*>(node);
    if (mt &&
        _animationPath.valid() && 
        nv->getVisitorType()==NodeVisitor::APP_VISITOR && 
        nv->getFrameStamp())
    {
        double time = nv->getFrameStamp()->getReferenceTime();
        if (_firstTime==0.0) _firstTime = time;
        Matrix matrix;
        if (_animationPath->getMatrix(((time-_firstTime)-_timeOffset)*_timeMultiplier,matrix))
        {
            mt->setMatrix(matrix);
        }
    }
    // must call any nested node callbacks and continue subgraph traversal.
    traverse(node,nv);
}
