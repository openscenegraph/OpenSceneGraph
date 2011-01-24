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
#include <osgDB/WriteFile>
#include <osgDB/FileNameUtils>


#include <osgUtil/IncrementalCompileOperation>
#include <osgUtil/Simplifier>
#include <osgUtil/MeshOptimizers>

namespace osgUtil
{


struct CostFunction1D
{
    virtual double operator() (unsigned int input) = 0;
};

struct LinearCostFunction1D : public CostFunction1D
{
    LinearCostFunction1D(double cost0, double dcost_di):
        _cost0(cost0),
        _dcost_di(dcost_di) {}
    virtual double operator() (unsigned int input) { return _cost0 + _dcost_di * double(input); }
    double _cost0;
    double _dcost_di;
};

struct ClampedLinearCostFunction1D : public CostFunction1D
{
    ClampedLinearCostFunction1D(double cost0, double dcost_di, unsigned int min_input):
        _cost0(cost0),
        _dcost_di(dcost_di),
        _min_input(min_input) {}

    virtual double operator() (unsigned int input) { return _cost0 + _dcost_di * double(input<=_min_input ? 0u : input-_min_input); }
    double _cost0;
    double _dcost_di;
    unsigned int _min_input;
};

/** Pair of double representing CPU and GPU times in seconds as first and second elements in std::pair. */
typedef std::pair<double, double> CostPair;


class GeometryCostEstimator : public osg::Referenced
{
public:
    CostPair estimateCompileCost(const osg::Geometry* geometry) const;
    CostPair estimateDrawCost(const osg::Geometry* geometry) const;
};

class TextureCostEstimator : public osg::Referenced
{
public:
    CostPair estimateCompileCost(const osg::Texture* texture) const;
    CostPair estimateDrawCost(const osg::Texture* texture) const;
};


class ProgramCostEstimator : public osg::Referenced
{
public:
    CostPair estimateCompileCost(const osg::Program* program) const;
    CostPair estimateDrawCost(const osg::Program* program) const;
};

class GraphicsCostEstimator : public osg::Referenced
{
public:
    GraphicsCostEstimator();

    /** set defaults for computing the costs.*/
    void setDefaults();

    /** calibrate the costs of various compile and draw operations */
    void calibrate(osg::RenderInfo& renderInfo);

    CostPair estimateCompileCost(const osg::Geometry* geometry) const { return _geometryEstimator->estimateCompileCost(geometry); }
    CostPair estimateDrawCost(const osg::Geometry* geometry) const { return _geometryEstimator->estimateDrawCost(geometry); }

    CostPair estimateCompileCost(const osg::Texture* texture) const { return _textureEstimator->estimateDrawCost(texture); }
    CostPair estimateDrawCost(const osg::Texture* texture) const { return _textureEstimator->estimateDrawCost(texture); }

    CostPair estimateCompileCost(const osg::Program* program) const { return _programEstimator->estimateCompileCost(program); }
    CostPair estimateDrawCost(const osg::Program* program) const { return _programEstimator->estimateDrawCost(program); }

    CostPair estimateCompileCost(const osg::Node* node) const;
    CostPair estimateDrawCost(const osg::Node* node) const;

protected:

    virtual ~GraphicsCostEstimator();

    osg::ref_ptr<GeometryCostEstimator> _geometryEstimator;
    osg::ref_ptr<TextureCostEstimator> _textureEstimator;
    osg::ref_ptr<ProgramCostEstimator> _programEstimator;

};



CostPair GeometryCostEstimator::estimateCompileCost(const osg::Geometry* geometry) const
{
    return CostPair(1.0,2.0);
}

CostPair GeometryCostEstimator::estimateDrawCost(const osg::Geometry* geometry) const
{
    return CostPair(0.0,0.0);
}

CostPair TextureCostEstimator::estimateCompileCost(const osg::Texture* texture) const
{
    return CostPair(0.0,0.0);
}

CostPair TextureCostEstimator::estimateDrawCost(const osg::Texture* texture) const
{
    return CostPair(0.0,0.0);
}

CostPair ProgramCostEstimator::estimateCompileCost(const osg::Program* program) const
{
    return CostPair(0.0,0.0);
}

CostPair ProgramCostEstimator::estimateDrawCost(const osg::Program* program) const
{
    return CostPair(0.0,0.0);
}


GraphicsCostEstimator::GraphicsCostEstimator()
{
    setDefaults();
}

GraphicsCostEstimator::~GraphicsCostEstimator()
{
}

void GraphicsCostEstimator::setDefaults()
{
    OSG_NOTICE<<"GraphicsCostEstimator::setDefaults()"<<std::endl;
}


void GraphicsCostEstimator::calibrate(osg::RenderInfo& renderInfo)
{
    OSG_NOTICE<<"GraphicsCostEstimator::calibrate(..)"<<std::endl;
}

class CollectCompileCosts : public osg::NodeVisitor
{
public:
    CollectCompileCosts(const GraphicsCostEstimator* gce):
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN),
        _gce(gce),
        _costs(0.0,0.0)
        {}

    virtual void apply(osg::Node& node)
    {
        apply(node.getStateSet());
        traverse(node);
    }

    virtual void apply(osg::Geode& geode)
    {
        apply(geode.getStateSet());
        for(unsigned int i=0; i<geode.getNumDrawables(); ++i)
        {
            apply(geode.getDrawable(i)->getStateSet());
            osg::Geometry* geometry = geode.getDrawable(i)->asGeometry();
            if (geometry) apply(geometry);
        }
    }

    void apply(osg::StateSet* stateset)
    {
        if (!stateset) return;
        if (_statesets.count(stateset)) return;
        _statesets.insert(stateset);

        const osg::Program* program = dynamic_cast<const osg::Program*>(stateset->getAttribute(osg::StateAttribute::PROGRAM));
        if (program)
        {
            CostPair cost = _gce->estimateCompileCost(program);
            _costs.first += cost.first;
            _costs.second += cost.second;
        }

        for(unsigned int i=0; i<stateset->getNumTextureAttributeLists(); ++i)
        {
            const osg::Texture* texture = dynamic_cast<const osg::Texture*>(stateset->getTextureAttribute(i, osg::StateAttribute::TEXTURE));
            CostPair cost = _gce->estimateCompileCost(texture);
            _costs.first += cost.first;
            _costs.second += cost.second;
        }
    }

    void apply(osg::Geometry* geometry)
    {
        if (!geometry) return;
        if (_geometries.count(geometry)) return;
        _geometries.insert(geometry);

        CostPair cost = _gce->estimateCompileCost(geometry);
        _costs.first += cost.first;
        _costs.second += cost.second;
    }


    typedef std::set<osg::StateSet*> StateSets;
    typedef std::set<osg::Texture*> Textures;
    typedef std::set<osg::Geometry*> Geometries;

    const GraphicsCostEstimator* _gce;
    StateSets   _statesets;
    Textures    _textures;
    Geometries  _geometries;
    CostPair    _costs;
};

CostPair GraphicsCostEstimator::estimateCompileCost(const osg::Node* node) const
{
    if (!node) return CostPair(0.0,0.0);
    CollectCompileCosts ccc(this);
    const_cast<osg::Node*>(node)->accept(ccc);
    return ccc._costs;
}

CostPair GraphicsCostEstimator::estimateDrawCost(const osg::Node* node) const
{
    return CostPair(0.0,0.0);
}

}


class CalibrateCostEsimator : public osg::GraphicsOperation
{
public:

    CalibrateCostEsimator(osgUtil::GraphicsCostEstimator* gce):
        osg::GraphicsOperation("CalbirateCostEstimator",false),
        _gce(gce) {}

    virtual void operator () (osg::GraphicsContext* context)
    {
        osg::RenderInfo renderInfo(context->getState(), 0);
        _gce->calibrate(renderInfo);
    }

    osg::ref_ptr<osgUtil::GraphicsCostEstimator> _gce;

};


int main(int argc, char** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    // construct the viewer.
    osgViewer::Viewer viewer(arguments);


    osg::ref_ptr<osg::Node> node = osgDB::readNodeFiles(arguments);
    if (!node) return 0;

    osg::ref_ptr<osgUtil::GraphicsCostEstimator> gce = new osgUtil::GraphicsCostEstimator;

    viewer.setSceneData(node.get());

    viewer.realize();

    osgUtil::CostPair compileCost = gce->estimateCompileCost(node.get());
    osgUtil::CostPair drawCost = gce->estimateDrawCost(node.get());

    OSG_NOTICE<<"estimateCompileCost("<<node->getName()<<"), CPU="<<compileCost.first<<" GPU="<<compileCost.second<<std::endl;
    OSG_NOTICE<<"estimateDrawCost("<<node->getName()<<"), CPU="<<drawCost.first<<" GPU="<<drawCost.second<<std::endl;

    return viewer.run();
}
