#include <osgFX/AnisotropicLighting>
#include <osgFX/Registry>

#include <osg/VertexProgram>
#include <osg/Texture2D>
#include <osg/TexEnv>

#include <osgDB/ReadFile>

#include <sstream>

using namespace osgFX;

namespace
{

    // a state attribute class that grabs the initial inverse view matrix
    // and sends it to a VertexProgram.
    // NOTE: due to lack of support for per-context parameters in VertexProgram,
    // this class will send the matrix to the vp only while the first context
    // is being rendered. All subsequent contexts will use the first context's
    // matrix.
    class ViewMatrixExtractor: public osg::StateAttribute {
    public:
        ViewMatrixExtractor()
        :    osg::StateAttribute(),
            _vp(0),
            _param(0),
            _first_context(-1)
        {
        }

        ViewMatrixExtractor(const ViewMatrixExtractor& copy, const osg::CopyOp& copyop)
        :    osg::StateAttribute(copy, copyop),
            _vp(static_cast<osg::VertexProgram *>(copyop(copy._vp.get()))),
            _param(copy._param),
            _first_context(-1)
        {
        }

        ViewMatrixExtractor(osg::VertexProgram *vp, int param)
        :    osg::StateAttribute(),
            _vp(vp),
            _param(param),
            _first_context(-1)
        {
        }

        META_StateAttribute(osgFX, ViewMatrixExtractor, VIEWMATRIXEXTRACTOR);

        int compare(const osg::StateAttribute &sa) const
        {
            COMPARE_StateAttribute_Types(ViewMatrixExtractor, sa);
            if (_vp.get() != rhs._vp.get()) return -1;
            if (_param < rhs._param) return -1;
            if (_param > rhs._param) return 1;
            return 0;
        }

        void apply(osg::State& state) const
        {
            if (_first_context == -1) {
                _first_context = state.getContextID();
            }
            if (state.getContextID() == (unsigned int)_first_context) {
                if (_vp.valid()) {
                    osg::Matrix M = state.getInitialInverseViewMatrix();
                    for (int i=0; i<4; ++i) {
                        _vp->setProgramLocalParameter(_param+i, osg::Vec4(M(0, i), M(1, i), M(2, i), M(3, i)));
                    }                
                }
            }
        }

    private:
        mutable osg::ref_ptr<osg::VertexProgram> _vp;
        int _param;
        mutable int _first_context;
    };

}

namespace
{

    osg::Image* create_default_image()
    {
        const int _texturesize = 16;
        osg::ref_ptr<osg::Image> image = new osg::Image;
        image->setImage(_texturesize, _texturesize, 1, 3, GL_RGB, GL_UNSIGNED_BYTE, new unsigned char[3*_texturesize*_texturesize], osg::Image::USE_NEW_DELETE);
        for (int i=0; i<_texturesize; ++i) {
            for (int j=0; j<_texturesize; ++j) {
                float s = static_cast<float>(j) / (_texturesize-1);
                float t = static_cast<float>(i) / (_texturesize-1);
                float lum = t * 0.75f;
                float red = lum + 0.2f * powf(cosf(s*10), 3.0f);
                float green = lum;
                float blue = lum + 0.2f * powf(sinf(s*10), 3.0f);
                if (red > 1) red = 1;
                if (red < 0) red = 0;
                if (blue > 1) blue = 1;
                if (blue < 0) blue = 0;
                *(image->data(j, i)+0) = static_cast<unsigned char>(red * 255);
                *(image->data(j, i)+1) = static_cast<unsigned char>(green * 255);
                *(image->data(j, i)+2) = static_cast<unsigned char>(blue * 255);
            }
        }
        return image.release();
    }

}

namespace
{

    Registry::Proxy proxy(new AnisotropicLighting);

    class DefaultTechnique: public Technique {
    public:

        DefaultTechnique(int lightnum, osg::Texture2D *texture)
        :    Technique(),
            _lightnum(lightnum),
            _texture(texture)
        {
        }

        void getRequiredExtensions(std::vector<std::string>& extensions) const
        {
            extensions.push_back("GL_ARB_vertex_program");
        }

    protected:

        void define_passes()
        {
            std::ostringstream vp_oss;
            vp_oss <<
                "!!ARBvp1.0\n"
                "PARAM c5 = { 0, 0, 0, 1 };"
                "PARAM c4 = { 0, 0, 0, 0 };"
                "TEMP R0, R1, R2, R3, R4, R5, R6, R7, R8, R9;"
                "ATTRIB v18 = vertex.normal;"
                "ATTRIB v16 = vertex.position;"
                "PARAM s259[4] = { state.matrix.mvp };"
                "PARAM s18 = state.light[" << _lightnum << "].position;"
                "PARAM s223[4] = { state.matrix.modelview };"
                "PARAM c0[4] = { program.local[0..3] };"
                "    DP4 result.position.x, s259[0], v16;"
                "    DP4 result.position.y, s259[1], v16;"
                "    DP4 result.position.z, s259[2], v16;"
                "    DP4 result.position.w, s259[3], v16;"
                "    MOV R9, c0[0];"
                "    MUL R0, R9.y, s223[1];"
                "    MAD R0, R9.x, s223[0], R0;"
                "    MAD R0, R9.z, s223[2], R0;"
                "    MAD R8, R9.w, s223[3], R0;"
                "    DP4 R0.x, R8, v16;"
                "    MOV R7, c0[1];"
                "    MUL R1, R7.y, s223[1];"
                "    MAD R1, R7.x, s223[0], R1;"
                "    MAD R1, R7.z, s223[2], R1;"
                "    MAD R6, R7.w, s223[3], R1;"
                "    DP4 R0.y, R6, v16;"
                "    MOV R5, c0[2];"
                "    MUL R1, R5.y, s223[1];"
                "    MAD R1, R5.x, s223[0], R1;"
                "    MAD R1, R5.z, s223[2], R1;"
                "    MAD R4, R5.w, s223[3], R1;"
                "    DP4 R0.z, R4, v16;"
                "    MOV R3, c0[3];"
                "    MUL R1, R3.y, s223[1];"
                "    MAD R1, R3.x, s223[0], R1;"
                "    MAD R1, R3.z, s223[2], R1;"
                "    MAD R1, R3.w, s223[3], R1;"
                "    DP4 R0.w, R1, v16;"
                "    MOV R1.x, R9.w;"
                "    MOV R1.y, R7.w;"
                "    MOV R1.z, R5.w;"
                "    MOV R1.w, R3.w;"
                "    ADD R2, R1, -R0;"
                "    DP4 R0.x, R2, R2;"
                "    RSQ R1.x, R0.x;"
                "    DP4 R0.x, R9, s18;"
                "    DP4 R0.y, R7, s18;"
                "    DP4 R0.z, R5, s18;"
                "    DP4 R0.w, R3, s18;"
                "    DP4 R1.y, R0, R0;"
                "    RSQ R1.y, R1.y;"
                "    MUL R3, R1.y, R0;"
                "    MAD R2, R1.x, R2, R3;"
                "    DP4 R1.x, R2, R2;"
                "    RSQ R1.x, R1.x;"
                "    MUL R1, R1.x, R2;"
                "    DP3 R2.x, R8.xyzx, v18.xyzx;"
                "    DP3 R2.y, R6.xyzx, v18.xyzx;"
                "    DP3 R2.z, R4.xyzx, v18.xyzx;"
                "    MOV R2.w, c4.x;"
                "    DP4 R1.x, R1, R2;"
                "    MAX result.texcoord[0].x, R1.x, c4.x;"
                "    DP4 R0.x, R0, R2;"
                "    MAX result.texcoord[0].y, R0.x, c4.x;"
                "END\n";

            osg::ref_ptr<osg::StateSet> ss = new osg::StateSet;

            osg::ref_ptr<osg::VertexProgram> vp = new osg::VertexProgram;
            vp->setVertexProgram(vp_oss.str());
            ss->setAttributeAndModes(vp.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

            ss->setAttributeAndModes(new ViewMatrixExtractor(vp.get(), 0), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

            ss->setTextureAttributeAndModes(0, _texture.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

            osg::ref_ptr<osg::TexEnv> texenv = new osg::TexEnv;
            texenv->setMode(osg::TexEnv::DECAL);
            ss->setTextureAttributeAndModes(0, texenv.get(), osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);

            ss->setMode( GL_ALPHA_TEST, osg::StateAttribute::OFF );

            addPass(ss.get());
        }

    private:
        int _lightnum;
        osg::ref_ptr<osg::Texture2D> _texture;
    };

}


AnisotropicLighting::AnisotropicLighting()
:    Effect(),
    _lightnum(0),
    _texture(new osg::Texture2D)
{
    _texture->setImage(create_default_image());
    _texture->setWrap(osg::Texture::WRAP_S, osg::Texture::CLAMP);
    _texture->setWrap(osg::Texture::WRAP_T, osg::Texture::CLAMP);
}

AnisotropicLighting::AnisotropicLighting(const AnisotropicLighting& copy, const osg::CopyOp& copyop)
:    Effect(copy, copyop),
    _lightnum(copy._lightnum),
    _texture(static_cast<osg::Texture2D *>(copyop(copy._texture.get())))
{
}

bool AnisotropicLighting::define_techniques()
{
    addTechnique(new DefaultTechnique(_lightnum, _texture.get()));
    return true;
}
