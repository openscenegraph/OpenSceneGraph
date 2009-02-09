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
#include <osg/Notify>
#include <osg/io_utils>

#include <osgDB/ReadFile>

osgParticle::ParticleSystem::ParticleSystem()
:    osg::Drawable(), 
    _def_bbox(osg::Vec3(-10, -10, -10), osg::Vec3(10, 10, 10)),
    _alignment(BILLBOARD),
    _align_X_axis(1, 0, 0),
    _align_Y_axis(0, 1, 0),
    _particleScaleReferenceFrame(WORLD_COORDINATES),
    _doublepass(false),
    _frozen(false),
    _bmin(0, 0, 0), 
    _bmax(0, 0, 0), 
    _reset_bounds_flag(false),
    _bounds_computed(false),
    _def_ptemp(Particle()),
    _last_frame(0),
    _freeze_on_cull(false),
    _detail(1),
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
    _doublepass(copy._doublepass),
    _frozen(copy._frozen),
    _bmin(copy._bmin), 
    _bmax(copy._bmax), 
    _reset_bounds_flag(copy._reset_bounds_flag),
    _bounds_computed(copy._bounds_computed),
    _def_ptemp(copy._def_ptemp),
    _last_frame(copy._last_frame),
    _freeze_on_cull(copy._freeze_on_cull),
    _detail(copy._detail),
    _draw_count(0)
{
}

osgParticle::ParticleSystem::~ParticleSystem()
{
}

void osgParticle::ParticleSystem::update(double dt)
{
    // reset bounds
    _reset_bounds_flag = true;


    for(unsigned int i=0; i<_particles.size(); ++i)
    {
        Particle& particle = _particles[i];
        if (particle.isAlive())
        {
            if (particle.update(dt))
            {
                update_bounds(particle.getPosition(), particle.getCurrentSize());
            }
            else
            {
                reuseParticle(i);
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

    // get the current modelview matrix
    osg::Matrix modelview = state.getModelViewMatrix();

    // set up depth mask for first rendering pass
    glPushAttrib(GL_DEPTH_BUFFER_BIT); 
    glDepthMask(GL_FALSE);

    // render, first pass
    single_pass_render(state, modelview);

    // restore depth mask settings
    glPopAttrib();

    // render, second pass
    if (_doublepass) {    
        // set up color mask for second rendering pass
        glPushAttrib(GL_COLOR_BUFFER_BIT);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        // render the particles onto the depth buffer
        single_pass_render(state, modelview);

        // restore color mask settings
        glPopAttrib();
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
}


void osgParticle::ParticleSystem::single_pass_render(osg::State&  /*state*/, const osg::Matrix& modelview) const
{
    _draw_count = 0;
    if (_particles.size() <= 0) return;

    float scale = sqrtf(static_cast<float>(_detail));
    
    const Particle* startParticle = &_particles[0];
    startParticle->beginRender();

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
        if (currentParticle->isAlive())
        {
            if (currentParticle->getShape() != startParticle->getShape())
            {
                startParticle->endRender();
                currentParticle->beginRender();
                startParticle = currentParticle;
            }
            ++_draw_count;
            
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

                    currentParticle->render(currentParticle->getPosition(), xAxis, yAxis, scale);
                }
                else
                {
                    xAxis = osg::Matrix::transform3x3(R, scaled_aligned_xAxis);
                    yAxis = osg::Matrix::transform3x3(R, scaled_aligned_yAxis);

                    currentParticle->render(currentParticle->getPosition(), xAxis, yAxis, scale);
                }
            }
            else
            {
                currentParticle->render(currentParticle->getPosition(), xAxis, yAxis, scale);
            }
        } 
    }

    startParticle->endRender();
    
}

osg::BoundingBox osgParticle::ParticleSystem::computeBound() const
{ 
    if (!_bounds_computed)
    {
        return _def_bbox;
    } else
    {
        return osg::BoundingBox(_bmin,_bmax);
    }
}

