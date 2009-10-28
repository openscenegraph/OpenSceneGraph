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
        DefaultTechnique(osg::Material* wf_mat, osg::LineWidth* wf_lw)
            : Technique(), _wf_mat(wf_mat), _wf_lw(wf_lw) {}

            bool validate(osg::State& ) const
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

                ss->setAttributeAndModes(_wf_lw.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

                ss->setAttributeAndModes(_wf_mat.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

                ss->setMode(GL_LIGHTING, osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
                ss->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::OVERRIDE|osg::StateAttribute::OFF);

                addPass(ss.get());
            }
        }

    private:
        osg::ref_ptr<osg::Material> _wf_mat;
        osg::ref_ptr<osg::LineWidth> _wf_lw;
    };

}

Scribe::Scribe()
:    Effect(),
    _wf_mat(new osg::Material),
    _wf_lw(new osg::LineWidth)
{
    _wf_lw->setWidth(1.0f);

    _wf_mat->setColorMode(osg::Material::OFF);
    _wf_mat->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
    _wf_mat->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
    _wf_mat->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0.0f, 0.0f, 0.0f, 1.0f));
    _wf_mat->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(1.0f,1.0f,1.0f,1.0f));
}

Scribe::Scribe(const Scribe& copy, const osg::CopyOp& copyop)
:    Effect(copy, copyop),
    _wf_mat(static_cast<osg::Material*>(copyop(copy._wf_mat.get()))),
    _wf_lw(static_cast<osg::LineWidth*>(copyop(copy._wf_lw.get())))
{
}

bool Scribe::define_techniques()
{
    addTechnique(new DefaultTechnique(_wf_mat.get(), _wf_lw.get()));
    return true;
}
