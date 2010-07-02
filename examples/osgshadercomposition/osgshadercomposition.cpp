/* OpenSceneGraph example, osganimate.
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

#include <osgViewer/Viewer>
#include <osgDB/ReadFile>
#include <osg/ShaderAttribute>

osg::Node* createSceneGraph(osg::ArgumentParser& arguments)
{
    osg::Node* node = osgDB::readNodeFiles(arguments);
    if (!node) return 0;

    osg::StateSet* stateset = node->getOrCreateStateSet();
    osg::ShaderAttribute* sa = new osg::ShaderAttribute;
    stateset->setAttribute(sa);

    return node;
}

int main( int argc, char **argv )
{
    osg::ArgumentParser arguments(&argc,argv);

    osgViewer::Viewer viewer(arguments);

    osg::ref_ptr<osg::Node> scenegraph = createSceneGraph(arguments);
    if (!scenegraph) return 1;

    viewer.setSceneData(scenegraph.get());

    return viewer.run();
}
