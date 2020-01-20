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
#include <osgDB/WriteFile>
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
    _estimatedMaxNumOfParticles(0)
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
    _estimatedMaxNumOfParticles(0)
{
}

osgParticle::ParticleSystem::~ParticleSystem()
{
//    OSG_NOTICE<<"ParticleSystem::~ParticleSystem() "<<std::dec<<this<<std::dec<<" _particles.size()="<<_particles.size()<<", _particles.capacity()="<<_particles.capacity()<<" _estimatedMaxNumOfParticles="<<_estimatedMaxNumOfParticles<<std::endl;
}

osgParticle::Particle* osgParticle::ParticleSystem::createParticle(const osgParticle::Particle* ptemplate)
{
    // is there any dead particle?
    if (!_deadparts.empty())
    {

        // retrieve a pointer to the last dead particle
        Particle* P = _deadparts.top();

        // create a new (alive) particle in the same place
        *P = ptemplate? *ptemplate: _def_ptemp;

        // remove the pointer from the death stack
        _deadparts.pop();
        return P;

    }
    else
    {

        if (_particles.size()==_particles.capacity())
        {
            if (_estimatedMaxNumOfParticles > static_cast<int>(_particles.capacity()))
            {
                _particles.reserve(_estimatedMaxNumOfParticles);
            }
        }

        // add a new particle to the vector
        _particles.push_back(ptemplate? *ptemplate: _def_ptemp);
        return &_particles.back();
    }
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
        osgUtil::CullVisitor* cv = nv.asCullVisitor();
        if (cv)
        {
            osg::Matrix modelview = *(cv->getModelViewMatrix());
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
    ScopedReadLock lock(_readWriteMutex);

    osg::State& state = *renderInfo.getState();

    // update the frame count, so other objects can detect when
    // this particle system is culled
    _last_frame = state.getFrameStamp()->getFrameNumber();

    if (_particles.size() <= 0) return;

    // update the dirty flag of delta time, so next time a new request for delta time
    // will automatically cause recomputing
    _dirty_dt = true;

    // get the current modelview matrix
    osg::Matrix modelview = state.getModelViewMatrix();

    ArrayData& ad = _bufferedArrayData[state.getContextID()];

    if (_useVertexArray)
    {
        // note from Robert Osfield, September 2016, this block implemented for backwards compatibility but is pretty way vertex array/shaders were hacked into osgParticle

        // set up arrays and primitives ready to fill in
        if (!ad.vertices.valid())
        {
            ad.init3();
            ad.reserve(_particles.capacity());
        }

        ad.clear();
        ad.dirty();

        osg::Vec3Array& vertices = *ad.vertices;
        osg::Vec3Array& normals = *ad.normals;
        osg::Vec4Array& colors = *ad.colors;
        osg::Vec3Array& texcoords = *ad.texcoords3;
        ArrayData::Primitives& primitives = ad.primitives;

        for(unsigned int i=0; i<_particles.size(); i+=_detail)
        {
            const Particle* particle = &_particles[i];
            const osg::Vec4& color = particle->getCurrentColor();
            const osg::Vec3& pos = particle->getPosition();
            const osg::Vec3& vel = particle->getVelocity();
            colors.push_back( color );
            texcoords.push_back( osg::Vec3(particle->_alive, particle->_current_size, particle->_current_alpha) );
            normals.push_back(vel);
            vertices.push_back(pos);
        }

        primitives.push_back(ArrayData::ModeCount(GL_POINTS, vertices.size()));

    }
    else
    {
        // set up arrays and primitives ready to fill in
        if (!ad.vertices.valid())
        {
            ad.init();
            ad.reserve(_particles.capacity()*4);
        }

        ad.clear();
        ad.dirty();

        osg::Vec3Array& vertices = *ad.vertices;
        osg::Vec4Array& colors = *ad.colors;
        osg::Vec2Array& texcoords = *ad.texcoords2;
        ArrayData::Primitives& primitives = ad.primitives;

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

        for(unsigned int i=0; i<_particles.size(); i+=_detail)
        {
            const Particle* currentParticle = &_particles[i];

            bool insideDistance = true;
            if (_sortMode != NO_SORT && _visibilityDistance>0.0)
            {
                insideDistance = (currentParticle->getDepth()>=0.0 && currentParticle->getDepth()<=_visibilityDistance);
            }

            if (currentParticle->isAlive() && insideDistance)
            {
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
                        xAxis = osg::Matrix::transform3x3(scaled_aligned_xAxis, R);
                        xAxis = osg::Matrix::transform3x3(modelview,xAxis);

                        yAxis = osg::Matrix::transform3x3(scaled_aligned_yAxis, R);
                        yAxis = osg::Matrix::transform3x3(modelview,yAxis);
                    }
                    else
                    {
                        xAxis = osg::Matrix::transform3x3(scaled_aligned_xAxis, R);
                        yAxis = osg::Matrix::transform3x3(scaled_aligned_yAxis, R);
                    }
                }

                osg::Vec4 color = currentParticle->getCurrentColor();
                color.a() *= currentParticle->getCurrentAlpha();

                float currentSize = currentParticle->getCurrentSize();

                const osg::Vec3& xpos = currentParticle->getPosition();
                const float s_coord = currentParticle->getSTexCoord();
                const float t_coord = currentParticle->getTTexCoord();
                const float s_tile = currentParticle->getSTexTile();
                const float t_tile = currentParticle->getTTexTile();

                osg::Vec3 p1(xAxis * currentSize * scale);
                osg::Vec3 p2(yAxis * currentSize * scale);

                switch (currentParticle->getShape())
                {
                    case osgParticle::Particle::POINT:
                    {
                        vertices.push_back(currentParticle->getPosition());
                        colors.push_back(color);
                        texcoords.push_back(osg::Vec2(0.5f,0.5f));

                        if (!primitives.empty() && primitives.back().first==GL_POINTS)
                        {
                            primitives.back().second++;
                        }
                        else
                        {
                            primitives.push_back(ArrayData::ModeCount(GL_POINTS,1));
                        }

                        break;
                    }
                    case osgParticle::Particle::USER:
                    case osgParticle::Particle::QUAD_TRIANGLESTRIP:
                    case osgParticle::Particle::HEXAGON:
                    case osgParticle::Particle::QUAD:
                    {
                        const osg::Vec3 c0(xpos-p1-p2);
                        const osg::Vec2 t0(s_coord, t_coord);
                        const osg::Vec3 c1(xpos+p1-p2);
                        const osg::Vec2 t1(s_coord+s_tile, t_coord);
                        const osg::Vec3 c2(xpos+p1+p2);
                        const osg::Vec2 t2(s_coord+s_tile, t_coord+t_tile);
                        const osg::Vec3 c3(xpos-p1+p2);
                        const osg::Vec2 t3(s_coord, t_coord+t_tile);

                         // First 3 points (and texcoords) of quad or triangle
                        vertices.push_back(c0);
                        vertices.push_back(c1);
                        vertices.push_back(c2);
                        texcoords.push_back(t0);
                        texcoords.push_back(t1);
                        texcoords.push_back(t2);

#if defined(OSG_GL1_AVAILABLE) || defined(OSG_GL2_AVAILABLE) || defined(OSG_GLES1_AVAILABLE)
                        const unsigned int count = 4;
                        const GLenum mode = GL_QUADS;

                        // Last point (and texcoord) of quad
                        vertices.push_back(c3);
                        texcoords.push_back(t3);
#else
                        // No GL_QUADS mode on OpenGL 3 and upper / GLES2 and upper
                        const unsigned int count = 6;
                        const GLenum mode = GL_TRIANGLES;

                        // Second triangle
                        vertices.push_back(c2);
                        vertices.push_back(c3);
                        vertices.push_back(c0);
                        texcoords.push_back(t2);
                        texcoords.push_back(t3);
                        texcoords.push_back(t0);
#endif
                        for (unsigned int j = 0; j < count; ++j)
                            colors.push_back(color);

                        if (!primitives.empty() && primitives.back().first == mode)
                        {
                            primitives.back().second += count;
                        }
                        else
                        {
                            primitives.push_back(ArrayData::ModeCount(mode, count));
                        }

                        break;
                    }
                    case osgParticle::Particle::LINE:
                    {
                        // Get the normalized direction of the particle, to be used in the
                        // calculation of one of the linesegment endpoints.
                        const osg::Vec3& velocity = currentParticle->getVelocity();
                        float vl = velocity.length();
                        if (vl != 0)
                        {
                            osg::Vec3 v = velocity * currentSize * scale / vl;

                            vertices.push_back(currentParticle->getPosition());
                            colors.push_back(color);
                            texcoords.push_back(osg::Vec2(0.0f,0.0f));

                            vertices.push_back(currentParticle->getPosition()+v);
                            colors.push_back(color);
                            texcoords.push_back(osg::Vec2(1.0f,1.0f));

                            if (!primitives.empty() && primitives.back().first==GL_LINES)
                            {
                                primitives.back().second+=2;
                            }
                            else
                            {
                                primitives.push_back(ArrayData::ModeCount(GL_LINES,2));
                            }
                        }
                        break;
                    }

                    default:
                        OSG_WARN << "Invalid shape for particles\n";
                }
            }
        }

    }

    // set up depth mask for first rendering pass
#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GLES3_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
    glPushAttrib(GL_DEPTH_BUFFER_BIT);
#endif

    glDepthMask(GL_FALSE);

    ad.dispatchArrays(state);
    ad.dispatchPrimitives();

#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GLES3_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
    // restore depth mask settings
    glPopAttrib();
#endif

    // render, second pass
    if (_doublepass)
    {
        // set up color mask for second rendering pass
#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GLES3_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
        glPushAttrib(GL_COLOR_BUFFER_BIT);
#endif
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        ad.dispatchPrimitives();

#if !defined(OSG_GLES1_AVAILABLE) && !defined(OSG_GLES2_AVAILABLE) && !defined(OSG_GLES3_AVAILABLE) && !defined(OSG_GL3_AVAILABLE)
        // restore color mask settings
        glPopAttrib();
#endif
    }
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
        texture->setImage(osgDB::readRefImageFile(texturefile));
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
        texture->setImage(osgDB::readRefImageFile(texturefile));
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


void osgParticle::ParticleSystem::resizeGLObjectBuffers(unsigned int maxSize)
{
    Drawable::resizeGLObjectBuffers(maxSize);

    _bufferedArrayData.resize(maxSize);
    for(unsigned int i=0; i<_bufferedArrayData.size(); ++i)
    {
        _bufferedArrayData[i].resizeGLObjectBuffers(maxSize);
    }
}

void osgParticle::ParticleSystem::releaseGLObjects(osg::State* state) const
{
    Drawable::releaseGLObjects(state);

    if (state)
    {
        _bufferedArrayData[state->getContextID()].releaseGLObjects(state);
    }
    else
    {
        for(unsigned int i=0; i<_bufferedArrayData.size(); ++i)
        {
            _bufferedArrayData[i].releaseGLObjects(0);
        }
    }
}

osg::VertexArrayState* osgParticle::ParticleSystem::createVertexArrayStateImplementation(osg::RenderInfo& renderInfo) const
{
    osg::State& state = *renderInfo.getState();

    osg::VertexArrayState* vas = new osg::VertexArrayState(&state);

    vas->assignVertexArrayDispatcher();
    vas->assignNormalArrayDispatcher();
    vas->assignColorArrayDispatcher();
    vas->assignTexCoordArrayDispatcher(1);

    if (state.useVertexArrayObject(_useVertexArrayObject))
    {
        vas->generateVertexArrayObject();
    }

    return vas;
}


/////////////////////////////////////////////////////////////////////////////////////
//
// ArrayData
//
osgParticle::ParticleSystem::ArrayData::ArrayData()
{
}

void osgParticle::ParticleSystem::ArrayData::init()
{
    vertexBufferObject = new osg::VertexBufferObject;
    vertexBufferObject->setUsage(GL_DYNAMIC_DRAW);

    vertices = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
    vertices->setBufferObject(vertexBufferObject.get());
    vertices->setDataVariance(osg::Object::DYNAMIC);

    colors = new osg::Vec4Array(osg::Array::BIND_PER_VERTEX);
    colors->setBufferObject(vertexBufferObject.get());
    colors->setDataVariance(osg::Object::DYNAMIC);

    texcoords2 = new osg::Vec2Array(osg::Array::BIND_PER_VERTEX);
    texcoords2->setBufferObject(vertexBufferObject.get());
    texcoords2->setDataVariance(osg::Object::DYNAMIC);
}

void osgParticle::ParticleSystem::ArrayData::init3()
{
    vertexBufferObject = new osg::VertexBufferObject;
    vertexBufferObject->setUsage(GL_DYNAMIC_DRAW);

    vertices = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
    vertices->setBufferObject(vertexBufferObject.get());
    vertices->setDataVariance(osg::Object::DYNAMIC);

    normals = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
    normals->setBufferObject(vertexBufferObject.get());
    normals->setDataVariance(osg::Object::DYNAMIC);

    colors = new osg::Vec4Array(osg::Array::BIND_PER_VERTEX);
    colors->setBufferObject(vertexBufferObject.get());
    colors->setDataVariance(osg::Object::DYNAMIC);

    texcoords3 = new osg::Vec3Array(osg::Array::BIND_PER_VERTEX);
    texcoords3->setBufferObject(vertexBufferObject.get());
    texcoords3->setDataVariance(osg::Object::DYNAMIC);
}

void osgParticle::ParticleSystem::ArrayData::reserve(unsigned int numVertices)
{
    unsigned int vertex_size = 0;

    if (vertices.valid()) { vertices->reserve(numVertices); vertex_size += 12; }
    if (normals.valid()) { normals->reserve(numVertices); vertex_size += 12; }
    if (colors.valid()) { colors->reserve(numVertices); vertex_size += 16; }
    if (texcoords2.valid()) { texcoords2->reserve(numVertices); vertex_size += 8; }
    if (texcoords3.valid()) { texcoords3->reserve(numVertices); vertex_size += 12; }

    vertexBufferObject->getProfile()._size = numVertices * vertex_size;
}

void osgParticle::ParticleSystem::ArrayData::resize(unsigned int numVertices)
{
    if (vertices.valid()) vertices->resize(numVertices);
    if (normals.valid()) normals->resize(numVertices);
    if (colors.valid()) colors->resize(numVertices);
    if (texcoords2.valid()) texcoords2->resize(numVertices);
    if (texcoords3.valid()) texcoords3->resize(numVertices);
}

void osgParticle::ParticleSystem::ArrayData::resizeGLObjectBuffers(unsigned int maxSize)
{
    // OSG_NOTICE<<"osgParticle::ParticleSystem::resizeGLObjectBuffers("<<maxSize<<") "<<this<<std::endl;

    if (vertexBufferObject.valid()) vertexBufferObject->resizeGLObjectBuffers(maxSize);

    if (vertices.valid()) vertices->resizeGLObjectBuffers(maxSize);
    if (normals.valid()) normals->resizeGLObjectBuffers(maxSize);
    if (colors.valid()) colors->resizeGLObjectBuffers(maxSize);
    if (texcoords2.valid()) texcoords2->resizeGLObjectBuffers(maxSize);
    if (texcoords3.valid()) texcoords3->resizeGLObjectBuffers(maxSize);
}

void osgParticle::ParticleSystem::ArrayData::releaseGLObjects(osg::State* state)
{
    // OSG_NOTICE<<"osgParticle::ParticleSystem::releaseGLObjects("<<state<<") "<<this<<std::endl;

    if (vertexBufferObject.valid()) vertexBufferObject->releaseGLObjects(state);

    if (vertices.valid()) vertices->releaseGLObjects(state);
    if (normals.valid()) normals->releaseGLObjects(state);
    if (colors.valid()) colors->releaseGLObjects(state);
    if (texcoords2.valid()) texcoords2->releaseGLObjects(state);
    if (texcoords3.valid()) texcoords3->releaseGLObjects(state);
}

void osgParticle::ParticleSystem::ArrayData::clear()
{
    if (vertices.valid()) vertices->clear();
    if (normals.valid()) normals->clear();
    if (colors.valid()) colors->clear();
    if (texcoords2.valid()) texcoords2->clear();
    if (texcoords3.valid()) texcoords3->clear();
    primitives.clear();
}

void osgParticle::ParticleSystem::ArrayData::dirty()
{
    if (vertices.valid()) vertices->dirty();
    if (normals.valid()) normals->dirty();
    if (colors.valid()) colors->dirty();
    if (texcoords2.valid()) texcoords2->dirty();
    if (texcoords3.valid()) texcoords3->dirty();
}

void osgParticle::ParticleSystem::ArrayData::dispatchArrays(osg::State& state)
{
    osg::VertexArrayState* vas = state.getCurrentVertexArrayState();

    vas->lazyDisablingOfVertexAttributes();

    if (vertices.valid()) vas->setVertexArray(state, vertices.get());
    if (normals.valid()) vas->setNormalArray(state, normals.get());
    if (colors.valid()) vas->setColorArray(state, colors.get());
    if (texcoords2.valid()) vas->setTexCoordArray(state, 0, texcoords2.get());
    if (texcoords3.valid()) vas->setTexCoordArray(state, 0, texcoords3.get());

    vas->applyDisablingOfVertexAttributes(state);
}

void osgParticle::ParticleSystem::ArrayData::dispatchPrimitives()
{
    unsigned int base = 0;
    for(ArrayData::Primitives::iterator itr = primitives.begin();
        itr != primitives.end();
        ++itr)
    {
        ArrayData::ModeCount& mc = *itr;
        glDrawArrays(mc.first, base, mc.second);
        base += mc.second;
    }
}
