#include <osgFX/Validator>
#include <osgFX/Effect>

#include <osg/Notify>

using namespace osgFX;

Validator::Validator()
:    osg::StateAttribute(),
	effect_(0)
{
}

Validator::Validator(Effect *effect)
:    osg::StateAttribute(),
    effect_(effect)
{
}

Validator::Validator(const Validator &copy, const osg::CopyOp &copyop)
:    osg::StateAttribute(copy, copyop),
    effect_(static_cast<Effect *>(copyop(copy.effect_)))
{
}

void Validator::apply(osg::State &state) const
{
    if (!effect_) return;

    if (effect_->tech_selected_[state.getContextID()] == 0) {
        Effect::Technique_list::iterator i;
        int j = 0;
        for (i=effect_->techs_.begin(); i!=effect_->techs_.end(); ++i, ++j) {
            if ((*i)->validate(state)) {
                effect_->sel_tech_[state.getContextID()] = j;
                effect_->tech_selected_[state.getContextID()] = 1;
                return;
            }
        }
        osg::notify(osg::WARN) << "Warning: osgFX::Validator: could not find any techniques compatible with the current OpenGL context" << std::endl;
    }
}
