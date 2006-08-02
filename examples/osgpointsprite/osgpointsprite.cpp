#include <osg/PointSprite>
#include <osg/BlendFunc>
#include <osg/StateAttribute>
#include <osg/Point>
#include <osg/Geometry>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osgDB/ReadFile>
#include <osgProducer/Viewer>
#include <osg/GLExtensions>
#include <osg/TexEnv>

osg::Geode *makeGalaxy(unsigned nvertices)
{
    osg::Geode *geode = new osg::Geode();
    osg::Geometry *galaxy = new osg::Geometry();
    osg::Vec3Array *vertices = new osg::Vec3Array();
    osg::Vec4Array *colors = new osg::Vec4Array();
    osg::Vec4 ini(1,1,0,1);
    osg::Vec4 fin(0,0,1,1);

    /** Formula for the two spirals */
    for (unsigned i=0;i<nvertices/2;i++) {
        float val = (i*2/(float)nvertices * 2 * 3.14159265359);
        float modx1 = rand() / (float)RAND_MAX*2;
        float mody1 = rand() / (float)RAND_MAX*2;
        float modx2 = rand() / (float)RAND_MAX*2;
        float mody2 = rand() / (float)RAND_MAX*2;
        float modz1 = ((rand()-RAND_MAX/2) / (float)(RAND_MAX))*3/(val+1);
        float modz2 = ((rand()-RAND_MAX/2) / (float)(RAND_MAX))*3/(val+1);
        vertices->push_back(osg::Vec3(cos(val)*val+modx1, sin(val)*val+mody1, modz1));
        vertices->push_back(osg::Vec3(-cos(val)*val+modx2, -sin(val)*val+mody2, modz2));

        colors->push_back(ini+(fin-ini)*(i*2/(float)nvertices));
        colors->push_back(ini+(fin-ini)*(i*2/(float)nvertices));
    }
    galaxy->setVertexArray(vertices);
    galaxy->setColorArray(colors);
    galaxy->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
    galaxy->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS, 0, nvertices));
    geode->addDrawable(galaxy);
    return geode;
}

osg::StateSet* makeStateSet(float size)
{
    osg::StateSet *set = new osg::StateSet();

    /// Setup cool blending
    set->setMode(GL_BLEND, osg::StateAttribute::ON);
    osg::BlendFunc *fn = new osg::BlendFunc();
    fn->setFunction(osg::BlendFunc::SRC_ALPHA, osg::BlendFunc::DST_ALPHA);
    set->setAttributeAndModes(fn, osg::StateAttribute::ON);

    /// Setup the point sprites
    osg::PointSprite *sprite = new osg::PointSprite();
    set->setTextureAttributeAndModes(0, sprite, osg::StateAttribute::ON);

    /// Give some size to the points to be able to see the sprite
    osg::Point *point = new osg::Point();
    point->setSize(size);
    set->setAttribute(point);

    /// Disable depth test to avoid sort problems and Lighting
    set->setMode(GL_DEPTH_TEST, osg::StateAttribute::OFF);
    set->setMode(GL_LIGHTING, osg::StateAttribute::OFF);

    /// The texture for the sprites
    osg::Texture2D *tex = new osg::Texture2D();
    tex->setImage(osgDB::readImageFile("Images/particle.rgb"));
    set->setTextureAttributeAndModes(0, tex, osg::StateAttribute::ON);

    return set;
}

int main(int, char *[])
{
    osgProducer::Viewer viewer;

    /// Make the galaxy of points
    osg::Node *node = makeGalaxy(5000);

    node->setStateSet(makeStateSet(10.0f));

    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);
    viewer.setSceneData(node);

    viewer.realize();
    while (!viewer.done())
    {
        viewer.sync();
        viewer.update();
        viewer.frame();
    }

    // wait for all cull and draw threads to complete.
    viewer.sync();

    // run a clean up frame to delete all OpenGL objects.
    viewer.cleanup_frame();

    // wait for all the clean up frame to complete.
    viewer.sync();

    return(0);
}

