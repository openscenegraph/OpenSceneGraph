#include <osgFX/Effect>
#include <osgFX/Validator>

#include <osg/Group>
#include <osg/Notify>
#include <osg/StateAttribute>
#include <osg/Geometry>

#include <osgUtil/CullVisitor>

using namespace osgFX;

Effect::Effect()
:	osg::Group(),
    enabled_(true),
    global_sel_tech_(AUTO_DETECT),
    techs_defined_(false)
{
    build_dummy_node();
}

Effect::Effect(const Effect &copy, const osg::CopyOp &copyop)
:	osg::Group(copy, copyop),
    enabled_(copy.enabled_),
    global_sel_tech_(copy.global_sel_tech_),
    techs_defined_(false)
{
    build_dummy_node();
}

Effect::~Effect()
{
	// disable the validator for safety, so it won't try to access us
	// even if it stays alive for some reason
	if (dummy_for_validation_.valid()) {
		osg::StateSet *ss = dummy_for_validation_->getStateSet();
		if (ss) {
			Validator *validator = dynamic_cast<Validator *>(ss->getAttribute(Validator::VALIDATOR));
			if (validator) {
				validator->disable();
			}
		}
	}
}

void Effect::traverse(osg::NodeVisitor &nv)
{
	// if this effect is not enabled, then go for default traversal
    if (!enabled_) {
        inherited_traverse(nv);
        return;
    }

	// ensure that at least one technique is defined
    if (!techs_defined_) {

        // clear existing techniques
        techs_.clear();

		// clear technique selection indices
        sel_tech_.clear();

		// clear technique selection flags
        tech_selected_.clear();

        // define new techniques
        techs_defined_ = define_techniques();

		// check for errors, return on failure
        if (!techs_defined_) {
            osg::notify(osg::WARN) << "Warning: osgFX::Effect: could not define techniques for effect " << className() << std::endl;
            return;
        }

		// ensure that at least one technique has been defined
        if (techs_.empty()) {
            osg::notify(osg::WARN) << "Warning: osgFX::Effect: no techniques defined for effect " << className() << std::endl;
            return;
        }
    }

    Technique *tech = 0;

    // if the selection mode is set to AUTO_DETECT then we have to
	// choose the active technique!
	if (global_sel_tech_ == AUTO_DETECT) {

		// test whether at least one technique has been selected
        bool none_selected = true;
        for (unsigned i=0; i<tech_selected_.size(); ++i) {
            if (tech_selected_[i] != 0) {
                none_selected = false;
                break;
            }
        }

		// no techniques selected, traverse a dummy node that
		// contains the Validator (it will select a technique)
        if (none_selected) {
            dummy_for_validation_->accept(nv);
        }

        // find the highest priority technique that could be validated
		// in all active rendering contexts
		int max_index = -1;
        for (unsigned j=0; j<sel_tech_.size(); ++j) {
            if (tech_selected_[j] != 0) {
                if (sel_tech_[j] > max_index) {
                    max_index = sel_tech_[j];
                }
            }
        }

        // found a valid technique?
		if (max_index >= 0) {
            tech = techs_[max_index].get();
        }

    } else {

		// the active technique was selected manually
        tech = techs_[global_sel_tech_].get();
    }

    // if we could find an active technique, then continue with traversal
	if (tech) {
		tech->traverse(nv, this);
	}

	// wow, we're finished! :)
}

void Effect::build_dummy_node()
{
    dummy_for_validation_ = new osg::Geode;
    osg::ref_ptr<osg::Geometry> geo = new osg::Geometry;
    dummy_for_validation_->addDrawable(geo.get());
    dummy_for_validation_->getOrCreateStateSet()->setAttribute(new Validator(this));
}
