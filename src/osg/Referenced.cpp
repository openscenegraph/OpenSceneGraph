#include <osg/Referenced>
#include <osg/Notify>
#include <typeinfo>

namespace osg
{

static std::auto_ptr<DeleteHandler> s_deleteHandler(0);

void Referenced::setDeleteHandler(DeleteHandler* handler)
{
    s_deleteHandler.reset(handler);
}

DeleteHandler* Referenced::getDeleteHandler()
{
    return s_deleteHandler.get();
}


#ifdef OSG_COMPILE_UNIT_TESTS
int Referenced::_createdCount = 0;
int Referenced::_deletedCount = 0;
#endif

Referenced::~Referenced()
{
    if (_refCount>0)
    {
        notify(WARN)<<"Warning: deleting still referenced object "<<this<<" of type '"<<typeid(this).name()<<"'"<<std::endl;
        notify(WARN)<<"         the final reference count was "<<_refCount<<", memory corruption possible."<<std::endl;
    }
#ifdef OSG_COMPILE_UNIT_TESTS
    _deletedCount ++;
#endif
}

}; // end of namespace osg
