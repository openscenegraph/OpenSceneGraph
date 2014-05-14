#include <osgParticle/ParticleSystem>

#include <vector>

#include <osg/Drawable>
#include <osg/CopyOp>
#include <osg/State>
#include <osg/Matrix>
#include <osg/GL>
#include <osg/StateSet>
#include <osg/Texture2D>
#include <osg/BlendFunc>
#include <osg/TexEnv>
#include <osg/Material>
#include <osg/PointSprite>
#include <osg/Program>
#include <osg/Notify>
#include <osg/io_utils>

#include <osgDB/FileUtils>
#include <osgDB/ReadFile>
#include <osgUtil/CullVisitor>

#define USE_LOCAL_SHADERS

static double distance(const osg::Vec3& coord, const osg::Matrix& matrix)
{
    // copied from CullVisitor.cpp
    return -(coord[0]*matrix(0,2)+coord[1]*matrix(1,2)+coord[2]*matrix(2,2)+matrix(3,2));
}

osgParticle::ParticleSystem::ParticleSystem()
:    osg::Drawable(),
    _def_bbox(osg::Vec3(-10, -10, -10), osg::Vec3(10, 10, 10)),
    _alignment(BILLBOARD),
    _align_X_axis(1, 0, 0),
    _align_Y_axis(0, 1, 0),
    _particleScaleReferenceFrame(WORLD_COORDINATES),
    _useVertexArray(false),
    _useShaders(false),
    _dirty_uniforms(false),
    _doublepass(false),
    _frozen(false),
    _bmin(0, 0, 0),
    _bmax(0, 0, 0),
    _reset_bounds_flag(false),
    _bounds_computed(false),
    _def_ptemp(Particle()),
    _last_frame(0),
    _dirty_dt(true),
    _freeze_on_cull(false),
    _t0(0.0),
    _dt(0.0),
    _detail(1),
    _sortMode(NO_SORT),
    _visibilityDistance(-1.0),
    _draw_count(0)
{
    // we don't support display lists because particle systems
    // are dynamic, and they always changes between frames
    setSupportsDisplayList(false);
}

osgParticle::ParticleSystem::ParticleSystem(const ParticleSystem& copy, const osg::CopyOp& copyop)
:    osg::Drawable(copy, copyop),
    _def_bbox(copy._def_bbox),
    _alignment(copy._alignment),
    _align_X_axis(copy._align_X_axis),
    _align_Y_axis(copy._align_Y_axis),
    _particleScaleReferenceFrame(copy._particleScaleReferenceFrame),
    _useVertexArray(copy._useVertexArray),
    _useShaders(copy._useShaders),
    _dirty_uniforms(copy._dirty_uniforms),
    _doublepass(copy._doublepass),
    _frozen(copy._frozen),
    _bmin(copy._bmin),
    _bmax(copy._bmax),
    _reset_bounds_flag(copy._reset_bounds_flag),
    _bounds_computed(copy._bounds_computed),
    _def_ptemp(copy._def_ptemp),
    _last_frame(copy._last_frame),
    _dirty_dt(copy._dirty_dt),
    _freeze_on_cull(copy._freeze_on_cull),
    _t0(copy._t0),
    _dt(copy._dt),
    _detail(copy._detail),
    _sortMode(copy._sortMode),
    _visibilityDistance(copy._visibilityDistance),
    _draw_count(0)
{
}

osgParticle::ParticleSystem::~ParticleSystem()
{
}

void osgParticle::ParticleSystem::update(double dt, osg::NodeVisitor& nv)
{
    // reset bounds
    _reset_bounds_flag = true;

    if (_useShaders)
    {
        // Update shader uniforms
        // This slightly reduces the consumption of traversing the particle vector, because we
        // don't compute tile and angle attributes that are useleff for shaders.
        // At present, our lcoal shader implementation will ignore these particle props:
        //     _cur_tile, _s_coord, _t_coord, _prev_pos, _prev_angle and _angle
        osg::StateSet* stateset = getOrCreateStateSet();

        if (_dirty_uniforms)
        {
            osg::Uniform* u_vd = stateset->getUniform("visibilityDistance");
            if (u_vd) u_vd->set((float)_visibilityDistance);
            _dirty_uniforms = false;
        }
    }

    for(unsigned int i=0; i<_particles.size(); ++i)
    {
        Particle& particle = _particles[i];
        if (particle.isAlive())
        {
            if (particle.update(dt, _useShaders))
            {
                update_bounds(particle.getPosition(), particle.getCurrentSize());
            }
            else
            {
                reuseParticle(i);
            }
        }
    }

    if (_sortMode != NO_SORT)
    {
        // sort particles
        osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(&nv);
        if (cv)
        {
            osg::Matrixd modelview = *(cv->getModelViewMatrix());
            double scale = (_sortMode==SORT_FRONT_TO_BACK ? -1.0 : 1.0);
            double deadDistance = DBL_MAX;
            for (unsigned int i=0; i<_particles.size(); ++i)
            {
                Particle& particle = _particles[i];
                if (particle.isAlive())
                    particle.setDepth(distance(particle.getPosition(), modelview) * scale);
                else
                    particle.setDepth(deadDistance);
            }
            std::sort<Particle_vector::iterator>(_particles.begin(), _particles.end());

            // Repopulate the death stack as it will have been invalidated by the sort.
            unsigned int numDead = _deadparts.size();
            if (numDead>0)
            {
                 // clear the death stack
                _deadparts = Death_stack();

                // copy the tail of the _particles vector as this will contain all the dead Particle thanks to the depth sort against DBL_MAX
                Particle* first_dead_ptr  = &_particles[_particles.size()-numDead];
                Particle* last_dead_ptr  = &_particles[_particles.size()-1];
                for(Particle* dead_ptr  = first_dead_ptr; dead_ptr<=last_dead_ptr; ++dead_ptr)
                {
                    _deadparts.push(dead_ptr);
                }
            }
        }
    }

    // force recomputing of bounding box on next frame
    dirtyBound();
}

void osgParticle::ParticleSystem::drawImplementation(osg::RenderInfo& renderInfo) const
{
    osg::State& state = *renderInfo.getState();

    ScopedReadLock lock(_readWriteMutex);

    // update the frame count, so other objects can detect when
    // this particle system is culled
    _last_frame = state.getFrameStamp()->getFrameNumber();

    // update the dirty flag of delta time, so next time a new request for delta time
    // will automatically cause recomputing
    _dirty_dt = true;

    // get the current modelview matrix
    osg::Matrix modelview = state.getModelViewMatrix();

    // set up depth mask for first rendering pass
#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
#endif

    glDepthMask(GL_FALSE);

    // render, first pass
    if (_useVertexArray)
        render_vertex_array(renderInfo);
    else
        single_pass_render(renderInfo, modelview);

#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
    // restore depth mask settings
    glPopAttrib();
#endif

    // render, second pass
    if (_doublepass) {
        // set up color mask for second rendering pass
#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
        glPushAttrib(GL_COLOR_BUFFER_BIT);
#endif
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        // render the particles onto the depth buffer
        if (_useVertexArray)
            render_vertex_array(renderInfo);
        else
            single_pass_render(renderInfo, modelview);

#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
        // restore color mask settings
        glPopAttrib();
#endif
    }

#if defined(OSG_GLES1_AVAILABLE) || defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
    OSG_NOTICE<<"Warning: ParticleSystem::drawImplementation(..) not fully implemented."<<std::endl;
#endif

}

void osgParticle::ParticleSystem::setDefaultAttributes(const std::string& texturefile, bool emissive_particles, bool lighting, int texture_unit)
{
    osg::StateSet *stateset = new osg::StateSet;

    stateset->setMode(GL_LIGHTING, lighting? osg::StateAttribute::ON: osg::StateAttribute::OFF);
    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    osg::Material *material = new osg::Material;
    material->setSpecular(osg::Material::FRONT, osg::Vec4(0, 0, 0, 1));
    material->setEmission(osg::Material::FRONT, osg::Vec4(0, 0, 0, 1));
    material->setColorMode(lighting? osg::Material::AMBIENT_AND_DIFFUSE : osg::Material::OFF);
    stateset->setAttributeAndModes(material, osg::StateAttribute::ON);

    if (!texturefile.empty()) {
        osg::Texture2D *texture = new osg::Texture2D;
        texture->setImage(osgDB::readImageFile(texturefile));
        texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
        texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
        texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::MIRROR);
        texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::MIRROR);
        stateset->setTextureAttributeAndModes(texture_unit, texture, osg::StateAttribute::ON);

        osg::TexEnv *texenv = new osg::TexEnv;
        texenv->setMode(osg::TexEnv::MODULATE);
        stateset->setTextureAttribute(texture_unit, texenv);
    }

    osg::BlendFunc *blend = new osg::BlendFunc;
    if (emissive_particles) {
        blend->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE);
    } else {
        blend->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    }
    stateset->setAttributeAndModes(blend, osg::StateAttribute::ON);

    setStateSet(stateset);
    setUseVertexArray(false);
    setUseShaders(false);
}


void osgParticle::ParticleSystem::setDefaultAttributesUsingShaders(const std::string& texturefile, bool emissive_particles, int texture_unit)
{
    osg::StateSet *stateset = new osg::StateSet;
    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    osg::PointSprite *sprite = new osg::PointSprite;
    stateset->setTextureAttributeAndModes(texture_unit, sprite, osg::StateAttribute::ON);

    #if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE)
        stateset->setMode(GL_VERTEX_PROGRAM_POINT_SIZE, osg::StateAttribute::ON);
    #else
        OSG_NOTICE<<"Warning: ParticleSystem::setDefaultAttributesUsingShaders(..) not fully implemented."<<std::endl;
    #endif

    if (!texturefile.empty())
    {
        osg::Texture2D *texture = new osg::Texture2D;
        texture->setImage(osgDB::readImageFile(texturefile));
        texture->setFilter(osg::Texture2D::MIN_FILTER, osg::Texture2D::LINEAR);
        texture->setFilter(osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
        texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::MIRROR);
        texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::MIRROR);
        stateset->setTextureAttributeAndModes(texture_unit, texture, osg::StateAttribute::ON);
    }

    osg::BlendFunc *blend = new osg::BlendFunc;
    if (emissive_particles)
    {
        blend->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE);
    }
    else
    {
        blend->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    }
    stateset->setAttributeAndModes(blend, osg::StateAttribute::ON);

    osg::Program *program = new osg::Program;
#ifdef USE_LOCAL_SHADERS
    char vertexShaderSource[] =
        "uniform float visibilityDistance;\n"
        "varying vec3 basic_prop;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    basic_prop = gl_MultiTexCoord0.xyz;\n"
        "    \n"
        "    vec4 ecPos = gl_ModelViewMatrix * gl_Vertex;\n"
        "    float ecDepth = -ecPos.z;\n"
        "    \n"
        "    if (visibilityDistance > 0.0)\n"
        "    {\n"
        "        if (ecDepth <= 0.0 || ecDepth >= visibilityDistance)\n"
        "            basic_prop.x = -1.0;\n"
        "    }\n"
        "    \n"
        "    gl_Position = ftransform();\n"
        "    gl_ClipVertex = ecPos;\n"
        "    \n"
        "    vec4 color = gl_Color;\n"
        "    color.a *= basic_prop.z;\n"
        "    gl_FrontColor = color;\n"
        "    gl_BackColor = gl_FrontColor;\n"
        "}\n";
    char fragmentShaderSource[] =
        "uniform sampler2D baseTexture;\n"
        "varying vec3 basic_prop;\n"
        "\n"
        "void main(void)\n"
        "{\n"
        "    if (basic_prop.x < 0.0) discard;\n"
        "    gl_FragColor = gl_Color * texture2D(baseTexture, gl_TexCoord[0].xy);\n"
        "}\n";
    program->addShader(new osg::Shader(osg::Shader::VERTEX, vertexShaderSource));
    program->addShader(new osg::Shader(osg::Shader::FRAGMENT, fragmentShaderSource));
#else
    program->addShader(osg::Shader::readShaderFile(osg::Shader::VERTEX, osgDB::findDataFile("shaders/particle.vert")));
    program->addShader(osg::Shader::readShaderFile(osg::Shader::FRAGMENT, osgDB::findDataFile("shaders/particle.frag")));
#endif
    stateset->setAttributeAndModes(program, osg::StateAttribute::ON);

    stateset->addUniform(new osg::Uniform("visibilityDistance", (float)_visibilityDistance));
    stateset->addUniform(new osg::Uniform("baseTexture", texture_unit));
    setStateSet(stateset);

    setUseVertexArray(true);
    setUseShaders(true);
}


void osgParticle::ParticleSystem::single_pass_render(osg::RenderInfo& renderInfo, const osg::Matrix& modelview) const
{
    _draw_count = 0;
    if (_particles.size() <= 0) return;

    osg::GLBeginEndAdapter* gl = &(renderInfo.getState()->getGLBeginEndAdapter());

    float scale = sqrtf(static_cast<float>(_detail));

    osg::Vec3 xAxis = _align_X_axis;
    osg::Vec3 yAxis = _align_Y_axis;

    osg::Vec3 scaled_aligned_xAxis = _align_X_axis;
    osg::Vec3 scaled_aligned_yAxis = _align_Y_axis;

    float xScale = 1.0f;
    float yScale = 1.0f;

    if (_alignment==BILLBOARD)
    {
        xAxis = osg::Matrix::transform3x3(modelview,_align_X_axis);
        yAxis = osg::Matrix::transform3x3(modelview,_align_Y_axis);

        float lengthX2 = xAxis.length2();
        float lengthY2 = yAxis.length2();

        if (_particleScaleReferenceFrame==LOCAL_COORDINATES)
        {
            xScale = 1.0f/sqrtf(lengthX2);
            yScale = 1.0f/sqrtf(lengthY2);
        }
        else
        {
            xScale = 1.0f/lengthX2;
            yScale = 1.0f/lengthY2;
        }

        scaled_aligned_xAxis *= xScale;
        scaled_aligned_yAxis *= yScale;

        xAxis *= xScale;
        yAxis *= yScale;
    }

    bool requiresEndRender = false;
    const Particle* startParticle = &_particles[0];
    if (startParticle->getShape() != Particle::USER)
    {
        startParticle->beginRender(gl);
        requiresEndRender = true;
    }
    else
    {
        // Enable writing depth mask when drawing user-defined particles
        glDepthMask(GL_TRUE);
    }

    for(unsigned int i=0; i<_particles.size(); i+=_detail)
    {
        const Particle* currentParticle = &_particles[i];

        bool insideDistance = true;
        if (_sortMode != NO_SORT && _visibilityDistance>0.0)
            insideDistance = (currentParticle->getDepth()>=0.0 && currentParticle->getDepth()<=_visibilityDistance);

        if (currentParticle->isAlive() && insideDistance)
        {
            if (currentParticle->getShape() != startParticle->getShape())
            {
                startParticle->endRender(gl);
                startParticle = currentParticle;
                if (currentParticle->getShape() != Particle::USER)
                {
                    currentParticle->beginRender(gl);
                    requiresEndRender = true;
                    glDepthMask(GL_FALSE);
                }
                else
                    glDepthMask(GL_TRUE);
            }
            ++_draw_count;

            if (currentParticle->getShape() == Particle::USER)
            {
                if (requiresEndRender)
                {
                    startParticle->endRender(gl);
                    requiresEndRender = false;
                }
                currentParticle->render(renderInfo, currentParticle->getPosition(), currentParticle->getAngle());
                continue;
            }

            const osg::Vec3& angle = currentParticle->getAngle();
            bool requiresRotation = (angle.x()!=0.0f || angle.y()!=0.0f || angle.z()!=0.0f);
            if (requiresRotation)
            {
                osg::Matrix R;
                R.makeRotate(
                    angle.x(), osg::Vec3(1, 0, 0),
                    angle.y(), osg::Vec3(0, 1, 0),
                    angle.z(), osg::Vec3(0, 0, 1));

                if (_alignment==BILLBOARD)
                {
                    xAxis = osg::Matrix::transform3x3(R,scaled_aligned_xAxis);
                    xAxis = osg::Matrix::transform3x3(modelview,xAxis);

                    yAxis = osg::Matrix::transform3x3(R,scaled_aligned_yAxis);
                    yAxis = osg::Matrix::transform3x3(modelview,yAxis);

                    currentParticle->render(gl,currentParticle->getPosition(), xAxis, yAxis, scale);
                }
                else
                {
                    xAxis = osg::Matrix::transform3x3(R, scaled_aligned_xAxis);
                    yAxis = osg::Matrix::transform3x3(R, scaled_aligned_yAxis);

                    currentParticle->render(gl,currentParticle->getPosition(), xAxis, yAxis, scale);
                }
            }
            else
            {
                currentParticle->render(gl,currentParticle->getPosition(), xAxis, yAxis, scale);
            }
        }
    }

    if (requiresEndRender)
        startParticle->endRender(gl);
}

void osgParticle::ParticleSystem::render_vertex_array(osg::RenderInfo& renderInfo) const
{
    if (_particles.size() <= 0) return;

    // Compute the pointer and offsets
    Particle_vector::const_iterator itr = _particles.begin();
    float* ptr = (float*)(&(*itr));
    GLsizei stride = 0;
    if (_particles.size() > 1)
    {
        float* ptr1 = (float*)(&(*(itr+1)));
        stride = ptr1 - ptr;
    }
    GLsizei posOffset = (float*)(&(itr->_position)) - ptr;         // Position
    GLsizei colorOffset = (float*)(&(itr->_current_color)) - ptr;  // Color
    GLsizei velOffset = (float*)(&(itr->_velocity)) - ptr;         // Velocity
    GLsizei propOffset = (float*)(&(itr->_alive)) - ptr;       // Alive, size & alpha

    // Draw particles as arrays
    osg::State& state = *renderInfo.getState();
    state.lazyDisablingOfVertexAttributes();
    state.setColorPointer(4, GL_FLOAT, stride * sizeof(float), ptr + colorOffset);
    state.setVertexPointer(3, GL_FLOAT, stride * sizeof(float), ptr + posOffset);
    if (_useShaders)
    {
        state.setNormalPointer(GL_FLOAT, stride * sizeof(float), ptr + velOffset);
        state.setTexCoordPointer(0, 3, GL_FLOAT, stride * sizeof(float), ptr + propOffset);
    }
    state.applyDisablingOfVertexAttributes();
    glDrawArrays(GL_POINTS, 0, _particles.size());
}

osg::BoundingBox osgParticle::ParticleSystem::computeBoundingBox() const
{
    if (!_bounds_computed)
    {
        return _def_bbox;
    } else
    {
        return osg::BoundingBox(_bmin,_bmax);
    }
}

