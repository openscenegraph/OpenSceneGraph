#include <osg/Object>

namespace osg
{

Object::Object(const Object& obj,const CopyOp& copyop): 
    Referenced(),
    _dataVariance(obj._dataVariance),
    _userData(copyop(obj._userData.get())) {}


}; // end of namespace osg
