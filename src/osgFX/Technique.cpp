#include <osgFX/Technique>
#include <osgFX/Effect>

#include <osg/GLExtensions>

#include <osgUtil/CullVisitor>

using namespace osgFX;

Technique::Technique()
:    osg::Referenced()
{
}

void Technique::addPass(osg::StateSet* ss)
{
    if (ss) {
        _passes.push_back(ss);
        ss->setRenderBinDetails(static_cast<int>(_passes.size()), "RenderBin");
    }
}

bool Technique::validate(osg::State& state) const
{
    typedef std::vector<std::string> String_list;
    String_list extensions;

    getRequiredExtensions(extensions);

    for (String_list::const_iterator i=extensions.begin(); i!=extensions.end(); ++i) {
        if (!osg::isGLExtensionSupported(state.getContextID(),i->c_str())) return false;
    }

    return true;
}

void Technique::traverse_implementation(osg::NodeVisitor& nv, Effect* fx)
{
    // define passes if necessary
    if (_passes.empty()) {
        define_passes();
    }

    // special actions must be taken if the node visitor is actually a CullVisitor
    osgUtil::CullVisitor *cv = dynamic_cast<osgUtil::CullVisitor *>(&nv);

    // traverse all passes
    for (int i=0; i<getNumPasses(); ++i) {

        // push the i-th pass' StateSet if necessary
        if (cv) {
            cv->pushStateSet(_passes[i].get());
        }

        // traverse the override node if defined, otherwise
        // traverse children as a Group would do
        osg::Node *override = getOverrideChild(i);
        if (override) {
            override->accept(nv);
        } else {
            fx->inherited_traverse(nv);
        }

        // pop the StateSet if necessary
        if (cv) {
            cv->popStateSet();
        }
    }
}
