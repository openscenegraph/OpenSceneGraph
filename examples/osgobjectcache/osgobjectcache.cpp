/* OpenSceneGraph example, osgobjectcache.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

#include <osg/Group>
#include <osgDB/ReadFile>

#include <osgViewer/Viewer>

#include <assert.h>

osg::Group* createObjectCache()
{
    osg::Group* group = new osg::Group();

    if (osgDB::Registry::instance()->getOptions()==0)
        osgDB::Registry::instance()->setOptions(new osgDB::Options());

    osgDB::Registry::instance()->getOptions()->setObjectCacheHint(osgDB::Options::CACHE_ALL);

    osg::ref_ptr<osgDB::Options> options1 = new osgDB::Options("a=1 b=2 c=3");
    options1->setObjectCacheHint(osgDB::Options::CACHE_ALL);

    osg::ref_ptr<osgDB::Options> options2 = new osgDB::Options("a=6 b=7 c=8");
    options2->setObjectCacheHint(osgDB::Options::CACHE_ALL);

    osg::ref_ptr<osgDB::Options> options3 = new osgDB::Options("b=7 a=6 c=8");
    options3->setObjectCacheHint(osgDB::Options::CACHE_ALL);

    osg::ref_ptr<osg::Node> node1 = osgDB::readRefNodeFile("cessna.osg");
    osg::ref_ptr<osg::Node> node2 = osgDB::readRefNodeFile("cessna.osg");
    osg::ref_ptr<osg::Node> node3 = osgDB::readRefNodeFile("cessna.osg", options1.get());
    osg::ref_ptr<osg::Node> node4 = osgDB::readRefNodeFile("cessna.osg", options2.get());
    osg::ref_ptr<osg::Node> node5 = osgDB::readRefNodeFile("cessna.osg", options1.get());
    osg::ref_ptr<osg::Node> node6 = osgDB::readRefNodeFile("cessna.osg", options2.get());
    osg::ref_ptr<osg::Node> node7 = osgDB::readRefNodeFile("cessna.osg", options3.get());
    osg::ref_ptr<osg::Node> node8 = osgDB::readRefNodeFile("cessna.osg", options3.get());

    // check that we really get the correct nodes
    if (node1 != node2)
    {
        fprintf(stderr, "error reading node from object cache using default options\n");
        exit(1);
    }

    if (node3 != node5)
    {
        fprintf(stderr, "error reading node from object cache stored with options '%s'\n", options1.get()->getOptionString().c_str());
        exit(1);
    }

    if (node4 != node6)
    {
        fprintf(stderr, "error reading node from object cache stored with options '%s'\n", options2.get()->getOptionString().c_str());
        exit(1);
    }

    if (node7 != node8)
    {
        fprintf(stderr, "error reading node from object cache stored with options '%s'\n", options3.get()->getOptionString().c_str());
        exit(1);
    }

    group->addChild(node1);
    group->addChild(node2);
    group->addChild(node3);
    group->addChild(node4);
    group->addChild(node5);
    group->addChild(node6);
    return group;
}

int main(int , char **)
{
    // construct the viewer.
    osgViewer::Viewer viewer;

    // add model to viewer.
    viewer.setSceneData(createObjectCache());

    // create the windows and run the threads.
    return viewer.run();
}
