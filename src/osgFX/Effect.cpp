#include <osgFX/Effect>
#include <osgFX/Validator>

#include <osg/Group>
#include <osg/Notify>
#include <osg/StateAttribute>
#include <osg/Geometry>

using namespace osgFX;

Effect::Effect()
:    osg::Node(),
    enabled_(true),
    global_sel_tech_(AUTO_DETECT),
    techs_defined_(false)
{
    build_dummy_node();
}

Effect::Effect(const Effect &copy, const osg::CopyOp &copyop)
:    osg::Node(copy, copyop),
    enabled_(copy.enabled_),
    global_sel_tech_(copy.global_sel_tech_),
    techs_defined_(false),
    child_(static_cast<osg::Node *>(copyop(copy.child_.get())))
{
    build_dummy_node();
}

bool Effect::computeBound() const
{
    _bsphere.init();
    _bsphere_computed = true;

    if (child_.valid()) {
        _bsphere.expandBy(child_->getBound());
    }

    return _bsphere.valid();
}

void Effect::traverse(osg::NodeVisitor &nv)
{
    typedef osg::Node Inherited;

    if (!child_.valid()) return;

    // we are not a Group, so children will not notify us when their
    // bounding box has changed. We need to recompute it at each traversal... :(
    dirtyBound();

    if (!enabled_) {
        child_->accept(nv);
        Inherited::traverse(nv);
        return;
    }

    if (!techs_defined_) {

        // clear existing techniques if necessary
        techs_.clear();
        sel_tech_.clear();
        tech_selected_.clear();

        // define new techniques
        techs_defined_ = define_techniques();
        if (!techs_defined_) {
            osg::notify(osg::WARN) << "Warning: osgFX::Effect: could not define techniques for effect " << className() << std::endl;
            return;
        }
        if (techs_.empty()) {
            osg::notify(osg::WARN) << "Warning: osgFX::Effect: no techniques defined for effect " << className() << std::endl;
            return;
        }
    }

    Technique *tech = 0;

    if (global_sel_tech_ == AUTO_DETECT) {
        bool none_selected = true;
        for (unsigned i=0; i<tech_selected_.size(); ++i) {
            if (tech_selected_[i] != 0) {
                none_selected = false;
                break;
            }
        }

        if (none_selected) {
            dummy_for_validation_->accept(nv);
        }

        int max_index = -1;
        for (unsigned j=0; j<sel_tech_.size(); ++j) {
            if (tech_selected_[j] != 0) {
                if (sel_tech_[j] > max_index) {
                    max_index = sel_tech_[j];
                }
            }
        }

        if (max_index >= 0) {
            tech = techs_[max_index].get();
        }

    } else {
        tech = techs_[global_sel_tech_].get();
    }

    if (tech) {
        tech->accept(nv, child_.get());
    } else {
        child_->accept(nv);
    }

    Inherited::traverse(nv);
}

void Effect::build_dummy_node()
{
    dummy_for_validation_ = new osg::Geode;    
    osg::ref_ptr<osg::Geometry> geo = new osg::Geometry;
    dummy_for_validation_->addDrawable(geo.get());
    dummy_for_validation_->getOrCreateStateSet()->setAttribute(new Validator(this));
}
