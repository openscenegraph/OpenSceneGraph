#include <osgFX/Technique>

#include <osg/GLExtensions>

using namespace osgFX;

Technique::Technique()
:    osg::Referenced(),
    passes_defined_(false),
    control_node_(new osg::Group)
{
}

void Technique::addPass(osg::StateSet *ss)
{
    osg::ref_ptr<osg::Group> pass = new osg::Group;
    control_node_->addChild(pass.get());

    if (ss) {
        ss->setRenderBinDetails(static_cast<int>(control_node_->getNumChildren()), "RenderBin");
        pass->setStateSet(ss);
    }    
}

void Technique::addPass(osg::Group *pass)
{
    control_node_->addChild(pass);
}

bool Technique::validate(osg::State &) const
{
    typedef std::vector<std::string> String_list;
    String_list extensions;

    getRequiredExtensions(extensions);

    for (String_list::const_iterator i=extensions.begin(); i!=extensions.end(); ++i) {
        if (!osg::isGLExtensionSupported(i->c_str())) return false;
    }

    return true;
}

void Technique::accept(osg::NodeVisitor &nv, osg::Node *child)
{
    // define passes if necessary
    if (!passes_defined_) {

        // clear existing pass nodes
        if (control_node_->getNumChildren() > 0) {
            control_node_->removeChild(0, control_node_->getNumChildren());
        }

        define_passes();
        passes_defined_ = true;
    }

    // update pass children if necessary
    if (child != prev_child_.get()) {
        for (unsigned i=0; i<control_node_->getNumChildren(); ++i) {
            osg::Group *pass = dynamic_cast<osg::Group *>(control_node_->getChild(i));
            if (pass) {
                if (pass->getNumChildren() > 0) {
                    pass->removeChild(0, pass->getNumChildren());
                }
                osg::Node *oc = getOverrideChild(i);
                if (oc) {
                    pass->addChild(oc); 
                } else {
                    pass->addChild(child);
                }
            }
        }
        prev_child_ = child;
    }

    // traverse the control node
    control_node_->accept(nv);
}
