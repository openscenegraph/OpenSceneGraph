#include <osg/Object>
#include <osg/Notify>
#include <typeinfo>

using namespace osg;

Referenced::~Referenced()
{
    if (_refCount>0)
    {
        notify(WARN)<<"Warning: deleting still referenced object "<<this<<" of type '"<<typeid(this).name()<<"'"<<std::endl;
        notify(WARN)<<"         the final reference count was "<<_refCount<<", memory corruption possible."<<std::endl;
    }
}


Object::Object(const Object& obj,const CopyOp& copyop): 
    Referenced(),
    _dataVariance(obj._dataVariance),
    _userData(copyop(obj._userData.get())) {}
