#include <osgFX/Scribe>
#include <osgFX/Registry>

#include <osg/GL>
#include <osg/PolygonOffset>
#include <osg/Material>
#include <osg/LineWidth>
#include <osg/PolygonMode>

#include <string.h>

using namespace osgFX;

namespace
{

    // register a prototype for this effect
    Registry::Proxy proxy(new Scribe);


    // default technique class
    class DefaultTechnique: public Technique {
    public:
        DefaultTechnique(osg::Material *wf_mat, osg::LineWidth *wf_lw)
            : Technique(), wf_mat_(wf_mat), wf_lw_(wf_lw) {}

            bool validate(osg::State &) const
            {
                return strncmp((const char*)glGetString(GL_VERSION), "1.1", 3) >= 0;
            }

    protected:

        void define_passes()
        {
            // implement pass #1
            {
                osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;

                osg::ref_ptr<osg::PolygonOffset> polyoffset = new osg::PolygonOffset;
                polyoffset->setFactor(1.0f);
                polyoffset->setUnits(1.0f);
                ss->setAttributeAndModes(polyoffset.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

                addPass(ss.get());
            }

            // implement pass #2
            {
                osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;

                osg::ref_ptr<osg::PolygonMode> polymode = new osg::PolygonMode;
                polymode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
                ss->setAttributeAndModes(polymode.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

                wf_lw_->setWidth(1);
                ss->setAttributeAndModes(wf_lw_.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

                wf_mat_->setColorMode(osg::Material::OFF);
                wf_mat_->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
                wf_mat_->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
                wf_mat_->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
                wf_mat_->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f, 1.0f, 1.0f, 1.0f));
                ss->setAttributeAndModes(wf_mat_.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

                ss->setMode(GL_LIGHTING, osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
                ss->setTextureMode(0, GL_TEXTURE_1D, osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);
                ss->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);

                   addPass(ss.get());
            }
        }

    private:
        osg::ref_ptr<osg::Material> wf_mat_;
        osg::ref_ptr<osg::LineWidth> wf_lw_;
    };

}

Scribe::Scribe()
:    Effect(),
    wf_mat_(new osg::Material),
    wf_lw_(new osg::LineWidth)
{
}

Scribe::Scribe(const Scribe &copy, const osg::CopyOp &copyop)
:    Effect(copy, copyop),
    wf_mat_(static_cast<osg::Material *>(copyop(copy.wf_mat_.get()))),
    wf_lw_(static_cast<osg::LineWidth *>(copyop(copy.wf_lw_.get())))
{
}

bool Scribe::define_techniques()
{
    addTechnique(new DefaultTechnique(wf_mat_.get(), wf_lw_.get()));
    return true;
}
