#include <osgFX/Effect>
#include <osgFX/Validator>

#include <osg/Group>
#include <osg/Notify>
#include <osg/StateAttribute>
#include <osg/Geometry>

#include <osgUtil/CullVisitor>

using namespace osgFX;

Effect::Effect()
:   osg::Group(),
    _enabled(true),
    _global_sel_tech(AUTO_DETECT),
    _techs_defined(false)
{
    build_dummy_node();
}

Effect::Effect(const Effect& copy, const osg::CopyOp& copyop)
:   osg::Group(copy, copyop),
    _enabled(copy._enabled),
    _global_sel_tech(copy._global_sel_tech),
    _techs_defined(false)
{
    build_dummy_node();
}

Effect::~Effect()
{
    // disable the validator for safety, so it won't try to access us
    // even if it stays alive for some reason
    if (_dummy_for_validation.valid()) {
        osg::StateSet* ss = _dummy_for_validation->getStateSet();
        if (ss) {
            Validator *validator = dynamic_cast<Validator *>(ss->getAttribute(Validator::VALIDATOR));
            if (validator) {
                validator->disable();
            }
        }
    }
}

void Effect::traverse(osg::NodeVisitor& nv)
{
    // if this effect is not enabled, then go for default traversal
    if (!_enabled) {
        inherited_traverse(nv);
        return;
    }

    // ensure that at least one technique is defined
    if (!_techs_defined) {

        // clear existing techniques
        _techs.clear();

        // clear technique selection indices
        _sel_tech.clear();

        // clear technique selection flags
        _tech_selected.clear();

        // define new techniques
        _techs_defined = define_techniques();

        // check for errors, return on failure
        if (!_techs_defined) {
            osg::notify(osg::WARN) << "Warning: osgFX::Effect: could not define techniques for effect " << className() << std::endl;
            return;
        }

        // ensure that at least one technique has been defined
        if (_techs.empty()) {
            osg::notify(osg::WARN) << "Warning: osgFX::Effect: no techniques defined for effect " << className() << std::endl;
            return;
        }
    }

    Technique *tech = 0;

    // if the selection mode is set to AUTO_DETECT then we have to
    // choose the active technique!
    if (_global_sel_tech == AUTO_DETECT) {

        // test whether at least one technique has been selected
        bool none_selected = true;
        for (unsigned i=0; i<_tech_selected.size(); ++i) {
            if (_tech_selected[i] != 0) {
                none_selected = false;
                break;
            }
        }

        // no techniques selected, traverse a dummy node that
        // contains the Validator (it will select a technique)
        if (none_selected) {
            _dummy_for_validation->accept(nv);
        }

        // find the highest priority technique that could be validated
        // in all active rendering contexts
        int max_index = -1;
        for (unsigned j=0; j<_sel_tech.size(); ++j) {
            if (_tech_selected[j] != 0) {
                if (_sel_tech[j] > max_index) {
                    max_index = _sel_tech[j];
                }
            }
        }

        // found a valid technique?
        if (max_index >= 0) {
            tech = _techs[max_index].get();
        }

    } else {

        // the active technique was selected manually
        tech = _techs[_global_sel_tech].get();
    }

    // if we could find an active technique, then continue with traversal,
    // else go for default traversal (no effect)
    if (tech) {
        tech->traverse(nv, this);
    } else {
        if (nv.getTraversalMode() == osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
            inherited_traverse(nv);
        }
    }

    // wow, we're finished! :)
}

void Effect::build_dummy_node()
{
    _dummy_for_validation = new osg::Geode;
    osg::ref_ptr<osg::Geometry> geo = new osg::Geometry;
    _dummy_for_validation->addDrawable(geo.get());
    _dummy_for_validation->getOrCreateStateSet()->setAttribute(new Validator(this));
}
