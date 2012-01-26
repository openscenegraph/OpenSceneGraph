/* OpenSceneGraph example, osgterrain.
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
#include <osgViewer/ViewerEventHandlers>

#include <osgDB/ReadFile>

#include <osg/GraphicsCostEstimator>

class CalibrateCostEsimator : public osg::GraphicsOperation
{
public:

    CalibrateCostEsimator(osg::GraphicsCostEstimator* gce):
        osg::GraphicsOperation("CalbirateCostEstimator",false),
        _gce(gce) {}

    virtual void operator () (osg::GraphicsContext* context)
    {
        osg::RenderInfo renderInfo(context->getState(), 0);
        _gce->calibrate(renderInfo);
    }

    osg::ref_ptr<osg::GraphicsCostEstimator> _gce;

};


int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);


    osg::ref_ptr<osg::Node> node = osgDB::readNodeFiles(arguments);
    if (!node) return 0;

    osg::ref_ptr<osg::GraphicsCostEstimator> gce = new osg::GraphicsCostEstimator;

    viewer.setSceneData(node.get());

    viewer.realize();

    osg::CostPair compileCost = gce->estimateCompileCost(node.get());
    osg::CostPair drawCost = gce->estimateDrawCost(node.get());

    OSG_NOTICE<<"estimateCompileCost("<<node->getName()<<"), CPU="<<compileCost.first<<" GPU="<<compileCost.second<<std::endl;
    OSG_NOTICE<<"estimateDrawCost("<<node->getName()<<"), CPU="<<drawCost.first<<" GPU="<<drawCost.second<<std::endl;

    return viewer.run();
}
