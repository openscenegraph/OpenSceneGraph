#if defined(_MSC_VER)
    #pragma warning( disable : 4786 )
#endif

#include <stdio.h>
#include <math.h>
#include <float.h>

#include <osg/Geometry>
#include <osg/ImpostorSprite>
#include <osg/Texture>
#include <osg/TexEnv>
#include <osg/AlphaFunc>
#include <osg/Notify>
#include <osg/Statistics>


using namespace osg;

ImpostorSprite::ImpostorSprite()
{
    // don't use display list since we will be updating the geometry.
    _useDisplayList = false;
    _parent = NULL;
    
    _ism = NULL;
    _previous = NULL;
    _next = NULL;

    _texture = NULL;
    _s = 0;
    _t = 0;        
}

ImpostorSprite::~ImpostorSprite()
{
    if (_ism)
    {
        _ism->remove(this);
    }
}

const float ImpostorSprite::calcPixelError(const Matrix& MVPW) const
{
    // find the maximum screen space pixel error between the control coords and the quad coners.
    float max_error_sqrd = 0.0f;

    for(int i=0;i<4;++i)
    {

        Vec3 projected_coord = _coords[i]*MVPW;
        Vec3 projected_control = _controlcoords[i]*MVPW;

        float dx = (projected_coord.x()-projected_control.x());
        float dy = (projected_coord.y()-projected_control.y());

        float error_sqrd = dx*dx+dy*dy;
        
        if (error_sqrd > max_error_sqrd) max_error_sqrd = error_sqrd;
        
    }

    return sqrtf(max_error_sqrd);
}
void ImpostorSprite::drawImmediateMode(State&)
{
    // when the tex env is set to REPLACE, and the 
    // texture is set up correctly the color has no effect.
    glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
    
    glBegin( GL_QUADS );
    
    glTexCoord2fv( (GLfloat *)&_texcoords[0] );
    glVertex3fv( (GLfloat *)&_coords[0] );

    glTexCoord2fv( (GLfloat *)&_texcoords[1] );
    glVertex3fv( (GLfloat *)&_coords[1] );

    glTexCoord2fv( (GLfloat *)&_texcoords[2] );
    glVertex3fv( (GLfloat *)&_coords[2] );

    glTexCoord2fv( (GLfloat *)&_texcoords[3] );
    glVertex3fv( (GLfloat *)&_coords[3] );

    glEnd();

}

const bool ImpostorSprite::computeBound() const
{
    _bbox.init();
    _bbox.expandBy(_coords[0]);
    _bbox.expandBy(_coords[1]);
    _bbox.expandBy(_coords[2]);
    _bbox.expandBy(_coords[3]);

    _bbox_computed=true;
    
    if (!_bbox.isValid())
    {
        notify(WARN) << "******* ImpostorSprite::computeBound() problem"<<std::endl;
        notify(WARN) << "*******  = "<<_coords[0]<<std::endl;
        notify(WARN) << "*******  = "<<_coords[1]<<std::endl;
        notify(WARN) << "*******  = "<<_coords[2]<<std::endl;
        notify(WARN) << "*******  = "<<_coords[3]<<std::endl;
    }

    return true;
}

void ImpostorSprite::setTexture(Texture* tex,int s,int t)
{
    _texture = tex;
    _s = s;
    _t = t;
}


///////////////////////////////////////////////////////////////////////////
// Helper class for managing the reuse of ImpostorSprite resources.
///////////////////////////////////////////////////////////////////////////

ImpostorSpriteManager::ImpostorSpriteManager()
{
    _texenv = osgNew TexEnv;
    _texenv->setMode(TexEnv::REPLACE);

    _alphafunc = osgNew osg::AlphaFunc;
    _alphafunc->setFunction( AlphaFunc::GREATER, 0.000f );

    _first = NULL;
    _last = NULL;
}


ImpostorSpriteManager::~ImpostorSpriteManager()
{
    while (_first)
    {
        ImpostorSprite* next = _first->_next;
        _first->_ism = NULL;
        _first->_previous = NULL;
        _first->_next = NULL;
        _first = next;
    }
    
}

void ImpostorSpriteManager::push_back(ImpostorSprite* is)
{
    if (is==NULL || is==_last) return;

    // remove entry for exisiting position in linked list
    // if it is already inserted.
    if (is->_previous)
    {
        (is->_previous)->_next = is->_next;
    }

    if (is->_next)
    {
        (is->_next)->_previous = is->_previous;
    }

    if (_first==is) _first = is->_next;

    if (empty())
    {
        _first = is;
        _last = is;
        is->_ism = this;
        is->_previous = NULL;
        is->_next = NULL;
    }
    else
    {

        // now add the element into the list.
        ImpostorSprite* previous_last = _last;
        previous_last->_next = is;
        _last = is;
        _last->_ism = this;
        _last->_previous = previous_last;
        _last->_next = NULL;
    }
}

void ImpostorSpriteManager::remove(ImpostorSprite* is)
{
    if (is==NULL) return;

    // remove entry for exisiting position in linked list
    // if it is already inserted.
    if (is->_previous)
    {
        (is->_previous)->_next = is->_next;
    }

    if (is->_next)
    {
        (is->_next)->_previous = is->_previous;
    }

    if (_first==is) _first = is->_next;
    if (_last==is) _last = is->_previous;
}

ImpostorSprite* ImpostorSpriteManager::createOrReuseImpostorSprite(int s,int t,int frameNumber)
{
    if (!empty())
    {

        // search for a valid impostor to reuse.        
        ImpostorSprite* curr = _first;
        while (curr)
        {
            if (curr->getLastFrameUsed()<=frameNumber &&
                curr->s()==s &&
                curr->t()==t)
            {
                push_back(curr);
                return curr;
            }
            else
            {
                curr = curr->_next;
            }
        }

    }

    // creating new impostor sprite.
    

    StateSet* stateset = osgNew StateSet;

    stateset->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
    stateset->setMode(GL_BLEND,osg::StateAttribute::ON);


    stateset->setAttributeAndModes( _alphafunc.get(), StateAttribute::ON );

    Texture* texture = osgNew Texture;

    stateset->setTextureAttributeAndModes(0,texture,StateAttribute::ON);
    stateset->setTextureAttribute(0,_texenv.get());

/*
    TexEnv* texenv = osgNew TexEnv;
    texenv->setMode(TexEnv::REPLACE);
    stateset->setAttribute(texenv);

    AlphaFunc* alphafunc = osgNew osg::AlphaFunc;
    alphafunc->setFunction( AlphaFunc::GREATER, 0.000f );
    stateset->setAttributeAndModes( alphafunc, StateAttribute::ON );
*/


    //    stateset->setMode( GL_ALPHA_TEST, StateAttribute::OFF );

    ImpostorSprite* is = osgNew ImpostorSprite;
    is->setStateSet(stateset);
    is->setTexture(texture,s,t);

    push_back(is);
    
    return is;

}


bool ImpostorSprite::getStats(Statistics &stat) 
{ // analyse the drawable Impostor Sprite
    stat.addNumPrims(1); // use new member functions of Statistics class to update
    stat.addNumPrims(Statistics::QUADS, 2, 1, 4);
    stat.addImpostor(1);
    return true;
}
