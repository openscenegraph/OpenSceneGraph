// clip region
// GEO version, Nov 2003
// may be replaced when clipRegion accepted into OSG proper.
// i) a clipregion is class derived from Group, with a Geode and any children of the group
// ii) a special draw is made that:
//    sets stencil bits by drawing the clip Geode
//    draws the children of group clipped by the stencil region.
// partly derived fromt he stencil code in osgreflect example.

#include "ClipRegion.h"
#include <osg/ColorMask>
#include <osg/Geode>
#include <osg/Depth>
#include <osg/BlendFunc>
#include <osgUtil/CullVisitor>

using namespace osg;

//=====
GeoClipRegion::GeoClipRegion(int bin)
{
    stencilbin=bin;
}

GeoClipRegion::~GeoClipRegion()
{
}

GeoClipRegion::GeoClipRegion(const GeoClipRegion& clr,const osg::CopyOp& copyop): osg::Group(clr,copyop)
{    //_clipNodes=clr._clipNodes;
}

void GeoClipRegion::addClipNode(osg::Node *gd) {
    
    osg::StateSet *state=gd->getOrCreateStateSet();
    // add clip node(s) to set stencil bit marking the clip area.
    // stencil op so that the stencil buffer get set at the clip pixels 
    osg::Stencil* stencil = new osg::Stencil;
    stencil->setFunction(osg::Stencil::ALWAYS,1,~0u);
    stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::REPLACE);
    state->setAttributeAndModes(stencil,osg::StateAttribute::ON);
    
    // switch off the writing to the color bit planes. (Dont show the clip area)
    osg::ColorMask* colorMask = new osg::ColorMask;
    colorMask->setMask(false,false,false,false);
    
    state->setRenderBinDetails(stencilbin,"RenderBin");
    state->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
    state->setAttribute(colorMask);
        // set up depth so all writing to depth goes to maximum depth. (dont want to z-clip the cull stencil)
    osg::Depth* depth = new osg::Depth;
    depth->setFunction(osg::Depth::ALWAYS);
    depth->setRange(1.0,1.0);
    state->setAttribute(depth);
    Group::addChild(gd);
}

bool GeoClipRegion::addChild( osg::Node *child )
{ 
    // bin the last - draw 'real' scenery last, using Z buffer to clip against any clip region... 

        osg::StateSet* statesetBin2 = child->getOrCreateStateSet();        
        statesetBin2->setRenderBinDetails(stencilbin+3,"RenderBin");
    /*   osg::Stencil* stencil = new osg::Stencil;
        stencil->setFunction(osg::Stencil::ALWAYS,0,~0);
        stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::REPLACE);
        statesetBin2->setAttributeAndModes(stencil,osg::StateAttribute::ON);*/
        return Group::addChild(child);
}

bool GeoClipRegion::addClippedChild( osg::Node *child )
{
// these children of this clipregion are drawn in stencilBin+2, clipped at the edges of the clip region
    osg::StateSet *state=child->getOrCreateStateSet();
    // state tests pixels against the set stencil.
    osg::Stencil* stenciltest = new osg::Stencil;
    stenciltest->setFunction(osg::Stencil::EQUAL,1,~0u);
    stenciltest->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
    state->setAttributeAndModes(stenciltest,osg::StateAttribute::ON);

    osg::ColorMask* cMask = new osg::ColorMask;
    cMask->setMask(true,true,true,true);
    state->setAttribute(cMask);

    state->setRenderBinDetails(stencilbin+1,"RenderBin");
    // use depth for rest of the scene unless overriden.
    osg::Depth* rootDepth = new osg::Depth;
    rootDepth->setFunction(osg::Depth::LESS);
    rootDepth->setRange(0.0,1.0);
    state->setAttribute(rootDepth);

    return   Group::addChild(child);
}
bool GeoClipRegion::addObscuredChild( osg::Node *child )
{
// other children of this node are drawn in stencilBin+2 outside the clip, hidden by the clip region
    osg::StateSet *state=child->getOrCreateStateSet();
    // state tests pixels against the set stencil.
    osg::Stencil* stenciltest = new osg::Stencil;
    stenciltest->setFunction(osg::Stencil::NOTEQUAL,1,~0u);
    stenciltest->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::KEEP);
    state->setAttributeAndModes(stenciltest,osg::StateAttribute::ON);

    osg::ColorMask* cMask = new osg::ColorMask;
    cMask->setMask(true,true,true,true);
    state->setAttribute(cMask);

    state->setRenderBinDetails(stencilbin+1,"RenderBin");
    // use depth for rest of the scene unless overriden.
    osg::Depth* rootDepth = new osg::Depth;
    rootDepth->setFunction(osg::Depth::LESS);
    rootDepth->setRange(0.0,1.0);
    state->setAttribute(rootDepth);
    return Group::addChild(child);
}

void GeoClipRegion::addDrawClipNode(osg::Node *ndclip)
{
    osg::StateSet *state=ndclip->getOrCreateStateSet();
    // last bin  - draw clip area and blend it with the clipped, visible geometry.
    
    // set up depth so all writing to depth goes to maximum depth.
    osg::Depth* depth = new osg::Depth;
    depth->setFunction(osg::Depth::ALWAYS);

    osg::Stencil* stencil = new osg::Stencil;
    stencil->setFunction(osg::Stencil::EQUAL,1,~0u);
    stencil->setOperation(osg::Stencil::KEEP, osg::Stencil::KEEP, osg::Stencil::ZERO);

    // set up additive blending.
    osg::BlendFunc* trans = new osg::BlendFunc;
    trans->setFunction(osg::BlendFunc::ONE,osg::BlendFunc::ONE);

    state->setRenderBinDetails(stencilbin+2,"RenderBin");
    state->setMode(GL_CULL_FACE,osg::StateAttribute::OFF);
    state->setAttributeAndModes(stencil,osg::StateAttribute::ON);
    state->setAttributeAndModes(trans,osg::StateAttribute::ON);
    state->setAttribute(depth);
    Group::addChild(ndclip);
}
