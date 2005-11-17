#include <osgFX/Validator>
#include <osgFX/Effect>

#include <osg/Notify>

using namespace osgFX;

Validator::Validator()
:    osg::StateAttribute(),
    _effect(0)
{
}

Validator::Validator(Effect* effect)
:    osg::StateAttribute(),
    _effect(effect)
{
}

Validator::Validator(const Validator& copy, const osg::CopyOp& copyop)
:    osg::StateAttribute(copy, copyop),
    _effect(static_cast<Effect*>(copyop(copy._effect)))
{
}

void Validator::compileGLObjects(osg::State& state) const
{
    apply(state);
}

void Validator::apply(osg::State& state) const
{
    if (!_effect) return;

    if (_effect->_tech_selected[state.getContextID()] == 0) {
        Effect::Technique_list::iterator i;
        int j = 0;
        for (i=_effect->_techs.begin(); i!=_effect->_techs.end(); ++i, ++j) {
            if ((*i)->validate(state)) {
                _effect->_sel_tech[state.getContextID()] = j;
                _effect->_tech_selected[state.getContextID()] = 1;
                return;
            }
        }
        osg::notify(osg::WARN) << "Warning: osgFX::Validator: could not find any techniques compatible with the current OpenGL context" << std::endl;
    }
}
