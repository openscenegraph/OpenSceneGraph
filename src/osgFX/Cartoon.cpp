#include <osgFX/Cartoon>
#include <osgFX/Registry>

#include <osg/PolygonOffset>
#include <osg/Texture1D>
#include <osg/VertexProgram>
#include <osg/PolygonMode>
#include <osg/CullFace>
#include <osg/Image>
#include <osg/TexEnv>
#include <osg/LineWidth>
#include <osg/Material>

#include <sstream>

using namespace osgFX;

namespace
{

    osg::Image *create_sharp_lighting_map(int levels = 4, int texture_size = 16)
    {
        osg::ref_ptr<osg::Image> image = new osg::Image;
        image->setImage(texture_size, 1, 1, 4, GL_RGBA, GL_UNSIGNED_BYTE, new unsigned char[4*texture_size], osg::Image::USE_NEW_DELETE);
        for (int i=0; i<texture_size; ++i) {
            float c = i/static_cast<float>(texture_size);
            c = (1+static_cast<int>(sqrtf(c) * (levels))) / static_cast<float>(levels+1);
            *(image->data(i, 0)+0) = static_cast<unsigned char>(c*255);
            *(image->data(i, 0)+1) = static_cast<unsigned char>(c*255);
            *(image->data(i, 0)+2) = static_cast<unsigned char>(c*255);
            *(image->data(i, 0)+3) = 255;
        }
        return image.take();
    }

}


namespace
{

    // register a prototype for this effect
    Registry::Proxy proxy(new Cartoon);


    // default technique class
    class DefaultTechnique: public Technique {
    public:
        DefaultTechnique(osg::Material *wf_mat, osg::LineWidth *wf_lw, int lightnum)
            : Technique(), wf_mat_(wf_mat), wf_lw_(wf_lw), lightnum_(lightnum) {}

        void getRequiredExtensions(std::vector<std::string> &extensions) const
        {
            extensions.push_back("GL_ARB_vertex_program");
        }

    protected:

        void define_passes()
        {
            // implement pass #1 (solid surfaces)
            {
                std::ostringstream vp_oss;
                vp_oss <<
                    "!!ARBvp1.0\n"
                    "OPTION ARB_position_invariant;"
                    "PARAM c0 = { 0, 0, 0, 0 };"
                    "TEMP R0, R1;"
                    "ATTRIB v18 = vertex.normal;"
                    "PARAM s18 = state.light[" << lightnum_ << "].position;"
                    "PARAM s16 = state.light[" << lightnum_ << "].diffuse;"
                    "PARAM s1 = state.material.diffuse;"
                    "PARAM s631[4] = { state.matrix.modelview[0].invtrans };"
                    "MOV R0, s1;"
                    "MUL result.color.front.primary, R0, s16;"
                    "DP4 R0.x, s18, s18;"
                    "RSQ R0.x, R0.x;"
                    "MUL R1, R0.x, s18;"
                    "DP4 R0.x, s631[0], v18;"
                    "DP4 R0.y, s631[1], v18;"
                    "DP4 R0.z, s631[2], v18;"
                    "DP4 R0.w, s631[3], v18;"
                    "DP4 R0.x, R1, R0;"
                    "MAX result.texcoord[0].x, c0.x, R0.x;"
                    "END";

                osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;

                osg::ref_ptr<osg::PolygonOffset> polyoffset = new osg::PolygonOffset;
                polyoffset->setFactor(1.0f);
                polyoffset->setUnits(1.0f);
                ss->setAttributeAndModes(polyoffset.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

                osg::ref_ptr<osg::VertexProgram> vp = new osg::VertexProgram;
                vp->setVertexProgram(vp_oss.str());
                ss->setAttributeAndModes(vp.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

                ss->setTextureMode(0, GL_TEXTURE_2D, osg::StateAttribute::OVERRIDE | osg::StateAttribute::OFF);

                osg::ref_ptr<osg::Texture1D> texture = new osg::Texture1D;
                texture->setImage(create_sharp_lighting_map());
                texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
                texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
                ss->setTextureAttributeAndModes(0, texture.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

                osg::ref_ptr<osg::TexEnv> texenv = new osg::TexEnv;
                texenv->setMode(osg::TexEnv::MODULATE);
                ss->setTextureAttributeAndModes(0, texenv.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

                addPass(ss.get());
            }

            // implement pass #2 (outlines)
            {
                osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;
                osg::ref_ptr<osg::PolygonMode> polymode = new osg::PolygonMode;
                polymode->setMode(osg::PolygonMode::FRONT_AND_BACK, osg::PolygonMode::LINE);
                ss->setAttributeAndModes(polymode.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

                osg::ref_ptr<osg::CullFace> cf = new osg::CullFace;
                cf->setMode(osg::CullFace::FRONT);
                ss->setAttributeAndModes(cf.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

                wf_lw_->setWidth(2);
                ss->setAttributeAndModes(wf_lw_.get(), osg::StateAttribute::OVERRIDE | osg::StateAttribute::ON);

                wf_mat_->setColorMode(osg::Material::OFF);
                wf_mat_->setDiffuse(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
                wf_mat_->setAmbient(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
                wf_mat_->setSpecular(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
                wf_mat_->setEmission(osg::Material::FRONT_AND_BACK, osg::Vec4(0, 0, 0, 1));
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
        int lightnum_;
    };

}

Cartoon::Cartoon()
:    Effect(),
    wf_mat_(new osg::Material),
    wf_lw_(new osg::LineWidth),
    lightnum_(0)
{
}

Cartoon::Cartoon(const Cartoon &copy, const osg::CopyOp &copyop)
:    Effect(copy, copyop),
    wf_mat_(static_cast<osg::Material *>(copyop(copy.wf_mat_.get()))),
    wf_lw_(static_cast<osg::LineWidth *>(copyop(copy.wf_lw_.get()))),
    lightnum_(copy.lightnum_)
{
}

bool Cartoon::define_techniques()
{
    addTechnique(new DefaultTechnique(wf_mat_.get(), wf_lw_.get(), lightnum_));
    return true;
}
