#ifndef CREATESHADOWEDSCENE_H
#define CREATESHADOWEDSCENE_H

#include <osg/Node>
#include <osg/Vec3>

// function to create a lightsource which contain a shadower and showed subgraph,
// the showadowed subgrph has a cull callback to fire off a pre render to texture
// of the shadowed subgraph.
extern osg::Group* createShadowedScene(osg::Node* shadower,osg::Node* shadowed,const osg::Vec3& lightPosition,float radius,unsigned int textureUnit=1);

#endif
