#include <osg/GL>
#include <osg/PolygonMode>
#include <osg/Notify>

using namespace osg;

PolygonMode::PolygonMode()
{
    _frontAndBack = true;
    _modeFront = FILL;
    _modeBack = FILL;
}


PolygonMode::~PolygonMode()
{
}

void PolygonMode::setMode(const Face face,const Mode mode)
{
    switch(face)
    {
        case(FRONT):
            _frontAndBack = false;
            _modeFront = mode;
            break;
        case(BACK):
            _frontAndBack = false;
            _modeBack = mode;
            break;
        case(FRONT_AND_BACK):
            _frontAndBack = true;
            _modeFront = mode;
            _modeBack = mode;
            break;
    }    
}

const PolygonMode::Mode PolygonMode::getMode(const Face face) const
{
    switch(face)
    {
        case(FRONT):
            return _modeFront;
        case(BACK):
            return _modeBack;
        case(FRONT_AND_BACK):
            return _modeFront;
    }    
    notify(WARN)<<"Warning : invalid Face passed to PolygonMode::getMode(Face face)"<<std::endl;
    return _modeFront;
}

void PolygonMode::apply(State&) const
{
    if (_frontAndBack)
    {
        glPolygonMode(GL_FRONT_AND_BACK,(GLenum)_modeFront);
    }
    else
    {
        glPolygonMode(GL_FRONT,(GLenum)_modeFront);
        glPolygonMode(GL_BACK,(GLenum)_modeBack);
    }
}

