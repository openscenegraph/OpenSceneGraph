#include <osgParticle/ParticleSystem>

#include <vector>

#include <osg/Drawable>
#include <osg/CopyOp>
#include <osg/State>
#include <osg/Matrix>
#include <osg/GL>
#include <osg/StateSet>
#include <osg/Texture>
#include <osg/BlendFunc>
#include <osg/TexEnv>
#include <osg/Material>

#include <osgDB/ReadFile>

osgParticle::ParticleSystem::ParticleSystem()
:    osg::Drawable(), 
    def_bbox_(osg::Vec3(-10, -10, -10), osg::Vec3(10, 10, 10)),
    doublepass_(false),
    frozen_(false),
    display_list_id_(-1), 
    bmin_(0, 0, 0), 
    bmax_(0, 0, 0), 
    reset_bounds_flag_(false),
    bounds_computed_(false),
    def_ptemp_(Particle()),
    last_frame_(0),
    freeze_on_cull_(true),
    detail_(1),
    draw_count_(0)
{
    // we don't support display lists because particle systems
    // are dynamic, and they always changes between frames
    setSupportsDisplayList(false);
}

osgParticle::ParticleSystem::ParticleSystem(const ParticleSystem &copy, const osg::CopyOp &copyop)
:    osg::Drawable(copy, copyop), 
    def_bbox_(copy.def_bbox_),
    doublepass_(copy.doublepass_),
    frozen_(copy.frozen_),
    display_list_id_(-1), 
    bmin_(copy.bmin_), 
    bmax_(copy.bmax_), 
    reset_bounds_flag_(copy.reset_bounds_flag_),
    bounds_computed_(copy.bounds_computed_),
    def_ptemp_(copy.def_ptemp_),
    last_frame_(copy.last_frame_),
    freeze_on_cull_(copy.freeze_on_cull_),
    detail_(copy.detail_),
    draw_count_(0)
{
}

osgParticle::ParticleSystem::~ParticleSystem()
{
}

void osgParticle::ParticleSystem::update(double dt)
{
    // reset bounds
    reset_bounds_flag_ = true;

    // set up iterators for particles
    Particle_vector::iterator i;
    Particle_vector::iterator end = particles_.end();

    // update particles
    for (i=particles_.begin(); i!=end; ++i) {
        if (i->isAlive()) {
            if (i->update(dt)) {
                update_bounds(i->getPosition(), i->getCurrentSize());
            } else {
                deadparts_.push(&(*i));
            }
        }
    }

    // force recomputing of bounding box on next frame
    dirtyBound();
}

void osgParticle::ParticleSystem::drawImmediateMode(osg::State &state)
{
    // update the frame count, so other objects can detect when
    // this particle system is culled
    last_frame_ = state.getFrameStamp()->getFrameNumber();

    // get the current modelview matrix
    const osg::Matrix &modelview = state.getModelViewMatrix();

    // set modelview = identity
    state.applyModelViewMatrix(0);

    // set up depth mask for first rendering pass
    glPushAttrib(GL_DEPTH_BUFFER_BIT); 
    glDepthMask(GL_FALSE);

    // render, first pass
    if (doublepass_) {

        // generate a display list ID if necessary
        if (display_list_id_ == -1) {
            display_list_id_ = glGenLists(1);
        }

        #ifdef USE_SEPERATE_COMPILE_AND_EXECUTE
            glNewList(display_list_id_, GL_COMPILE);
            single_pass_render(modelview);
            glEndList();
            glCallList(display_list_id_);
        #else
            glNewList(display_list_id_, GL_COMPILE_AND_EXECUTE);
            single_pass_render(modelview);
            glEndList();
        #endif

    } else {
        single_pass_render(modelview);
    }

    // restore depth mask settings
    glPopAttrib();

    // render, second pass
    if (doublepass_) {    
        // set up color mask for second rendering pass
        glPushAttrib(GL_COLOR_BUFFER_BIT);
        glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

        // call the display list to render the particles onto the depth buffer
        glCallList(display_list_id_);

        // restore color mask settings
        glPopAttrib();
    }
}

void osgParticle::ParticleSystem::setDefaultAttributes(const std::string &texturefile, bool emissive_particles, bool lighting, int texture_unit)
{
    osg::StateSet *stateset = osgNew osg::StateSet;

    stateset->setMode(GL_LIGHTING, lighting? osg::StateAttribute::ON: osg::StateAttribute::OFF);
    stateset->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

    osg::Material *material = osgNew osg::Material;
    material->setSpecular(osg::Material::FRONT, osg::Vec4(0, 0, 0, 1));
    material->setEmission(osg::Material::FRONT, osg::Vec4(0, 0, 0, 1));
    material->setColorMode(lighting? osg::Material::AMBIENT_AND_DIFFUSE : osg::Material::OFF);
    stateset->setAttributeAndModes(material, osg::StateAttribute::ON);

    if (!texturefile.empty()) {
        osg::Texture *texture = osgNew osg::Texture;
        texture->setImage(osgDB::readImageFile(texturefile));
        texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR);
        texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR);
        stateset->setTextureAttributeAndModes(texture_unit, texture, osg::StateAttribute::ON);

        osg::TexEnv *texenv = osgNew osg::TexEnv;
        texenv->setMode(osg::TexEnv::MODULATE);
        stateset->setTextureAttribute(texture_unit, texenv);
    }

    osg::BlendFunc *blend = osgNew osg::BlendFunc;
    if (emissive_particles) {    
        blend->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE);
    } else {
        blend->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::ONE_MINUS_SRC_ALPHA);
    }
    stateset->setAttributeAndModes(blend, osg::StateAttribute::ON);

    setStateSet(stateset);
}


void osgParticle::ParticleSystem::single_pass_render(const osg::Matrix &modelview)
{
    draw_count_ = 0;
    if (particles_.size() <= 0) return;

    Particle_vector::iterator i;
    Particle_vector::iterator i0 = particles_.begin();
    Particle_vector::iterator end = particles_.end();
    
    i0->beginRender();

    for (i=i0; i<end; i+=detail_) {
        if (i->isAlive()) {
            if (i->getShape() != i0->getShape()) {
                i0->endRender();
                i->beginRender();
                i0 = i;
            }
            ++draw_count_;            
            i->render(modelview, sqrtf(static_cast<float>(detail_)));
        }        
    }

    i0->endRender();
    
}
