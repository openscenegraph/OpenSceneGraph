#include <osgFX/SpecularHighlights>
#include <osgFX/Registry>

#include <osg/TextureCubeMap>
#include <osg/TexGen>
#include <osg/TexEnv>
#include <osg/ColorMatrix>

#include <osgUtil/HighlightMapGenerator>

using namespace osgFX;

namespace
{

    class AutoTextureMatrix: public osg::StateAttribute {
    public:
        AutoTextureMatrix()
        :    osg::StateAttribute(),
            lightnum_(0),
            active_(false)
        {
        }

        AutoTextureMatrix(const AutoTextureMatrix &copy, const osg::CopyOp &copyop)
        :    osg::StateAttribute(copy, copyop),
            lightnum_(copy.lightnum_),
            active_(copy.active_)
        {
        }

        AutoTextureMatrix(int lightnum, bool active = true)
        :    osg::StateAttribute(),
            lightnum_(lightnum),
            active_(active)
        {
        }

        META_StateAttribute(osgFX, AutoTextureMatrix, osg::StateAttribute::TEXMAT);

        virtual bool isTextureAttribute() const { return true; }

        int compare(const osg::StateAttribute &sa) const
        {
            COMPARE_StateAttribute_Types(AutoTextureMatrix, sa);
            if (lightnum_ < rhs.lightnum_) return -1;
            if (lightnum_ > rhs.lightnum_) return 1;
            return 0;
        }

        void apply(osg::State &state) const
        {
            glMatrixMode(GL_TEXTURE);

            if (active_) {
                osg::Matrix M = state.getInitialViewMatrix();
                M(3, 0) = 0; M(3, 1) = 0; M(3, 2) = 0;
                M(3, 3) = 1; M(0, 3) = 0; M(1, 3) = 0;
                M(2, 3) = 0;

                osg::Vec4 lightvec;
                glGetLightfv(GL_LIGHT0+lightnum_, GL_POSITION, lightvec._v);

                osg::Vec3 eye_light_ref = osg::Vec3(0, 0, 1) * M;

                osg::Matrix LM = osg::Matrix::rotate(
                    osg::Vec3(lightvec.x(), lightvec.y(), lightvec.z()),
                    eye_light_ref);
                
                glLoadMatrixf((LM * osg::Matrix::inverse(M)).ptr());

            } else {
                glLoadIdentity();
            }

            glMatrixMode(GL_MODELVIEW);
        }

    private:
        int lightnum_;
        bool active_;
    };

}

namespace
{

    Registry::Proxy proxy(new SpecularHighlights);

    class DefaultTechnique: public Technique {
    public:

        DefaultTechnique(int lightnum, int unit, const osg::Vec4 &color, float sexp)
        :    Technique(),
            lightnum_(lightnum),
            unit_(unit),
            color_(color),
            sexp_(sexp)
        {
        }

        void getRequiredExtensions(std::vector<std::string> &extensions)
        {
            extensions.push_back("GL_ARB_texture_env_add");
        }

        bool validate(osg::State &state) const
        {
            if (!Technique::validate(state)) return false;

            osg::TextureCubeMap::Extensions *ext = 
                osg::TextureCubeMap::getExtensions(state.getContextID(), true);
            if (ext) {
                return ext->isCubeMapSupported();
            }
            return false;
        }

    protected:

        void define_passes()
        {
            osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;

            ss->setTextureAttributeAndModes(unit_, new AutoTextureMatrix(lightnum_), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

            osg::ref_ptr<osgUtil::HighlightMapGenerator> hmg = new osgUtil::HighlightMapGenerator(osg::Vec3(0, 0, -1), color_, sexp_);
            hmg->generateMap(false);

            osg::ref_ptr<osg::TextureCubeMap> texture = new osg::TextureCubeMap;
            texture->setImage(osg::TextureCubeMap::POSITIVE_X, hmg->getImage(osg::TextureCubeMap::POSITIVE_X));
            texture->setImage(osg::TextureCubeMap::POSITIVE_Y, hmg->getImage(osg::TextureCubeMap::POSITIVE_Y));
            texture->setImage(osg::TextureCubeMap::POSITIVE_Z, hmg->getImage(osg::TextureCubeMap::POSITIVE_Z));
            texture->setImage(osg::TextureCubeMap::NEGATIVE_X, hmg->getImage(osg::TextureCubeMap::NEGATIVE_X));
            texture->setImage(osg::TextureCubeMap::NEGATIVE_Y, hmg->getImage(osg::TextureCubeMap::NEGATIVE_Y));
            texture->setImage(osg::TextureCubeMap::NEGATIVE_Z, hmg->getImage(osg::TextureCubeMap::NEGATIVE_Z));
            texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP_TO_EDGE);
            texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP_TO_EDGE);
            texture->setWrap(osg::Texture::WRAP_R, osg::Texture::CLAMP_TO_EDGE);
            ss->setTextureAttributeAndModes(unit_, texture.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

            osg::ref_ptr<osg::TexGen> texgen = new osg::TexGen;
            texgen->setMode(osg::TexGen::REFLECTION_MAP);
            ss->setTextureAttributeAndModes(unit_, texgen.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

            osg::ref_ptr<osg::TexEnv> texenv = new osg::TexEnv;
            texenv->setMode(osg::TexEnv::ADD);
            ss->setTextureAttributeAndModes(unit_, texenv.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

            addPass(ss.get());
        }

    private:
        int lightnum_;
        int unit_;
        osg::Vec4 color_;
        float sexp_;
    };

}


SpecularHighlights::SpecularHighlights()
:    Effect(),
    lightnum_(0),
    unit_(0),
    color_(1, 1, 1, 1),
    sexp_(16)
{
}

SpecularHighlights::SpecularHighlights(const SpecularHighlights &copy, const osg::CopyOp &copyop)
:    Effect(copy, copyop),
    lightnum_(copy.lightnum_),
    unit_(copy.unit_),
    color_(copy.color_),
    sexp_(copy.sexp_)
{
}

bool SpecularHighlights::define_techniques()
{
    addTechnique(new DefaultTechnique(lightnum_, unit_, color_, sexp_));
    return true;
}
