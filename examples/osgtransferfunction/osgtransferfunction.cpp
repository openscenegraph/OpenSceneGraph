/* OpenSceneGraph example, osggeometry.
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

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/ImageUtils>
#include <osg/MatrixTransform>
#include <osg/PositionAttitudeTransform>
#include <osg/ComputeBoundsVisitor>
#include <osg/UserDataContainer>
#include <osg/ValueObject>
#include <osg/io_utils>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>

#include <osgVolume/Volume>
#include <osgVolume/VolumeTile>
#include <osgVolume/RayTracedTechnique>
#include <osgVolume/FixedFunctionTechnique>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>


#include "TransferFunctionWidget.h"

class Histogram
{
public:
    Histogram() {}

    void analyse(const osg::Image* image, double interval=0.0);

    void insertZeroBoundaryValues(float xMin=FLT_MAX, float xMax=-FLT_MAX);

    osg::Node* createGraphicalRepresentation();

    typedef std::map<float, float> ValueMap;

    ValueMap& getValueMap() { return _valueMap; }

protected:

    ValueMap _valueMap;
};

struct PopulateHistogram
{

    PopulateHistogram(Histogram::ValueMap& valueMap):
        _histogram(valueMap) {}

    float cast(char v) { return v; }
    float cast(unsigned char v) { return v; }
    float cast(short v) { return v; }
    float cast(unsigned short v) { return v; }
    float cast(int v) { return v; }
    float cast(unsigned int v) { return v; }
    float cast(float v) { return v; }
    float cast(double v) { return v; }

    Histogram::ValueMap& _histogram;

    void update(int v)
    {
        _histogram[v]+=1.0;
    }

    void normalize()
    {
        double maxValue = 0;
        for(Histogram::ValueMap::iterator itr = _histogram.begin();
            itr != _histogram.end();
            ++itr)
        {
            if (itr->second>maxValue) maxValue = itr->second;
        }

        for(Histogram::ValueMap::iterator itr = _histogram.begin();
            itr != _histogram.end();
            ++itr)
        {
            itr->second /= maxValue;
        }
    }

    void luminance(float l) { update(l); }
    void alpha(float a) { update(a); }
    void luminance_alpha(float l, float /*a*/) { update(l); }
    void rgb(float r, float /*g*/, float /*b*/) { update(r); }
    void rgba(float /*r*/, float /*g*/, float /*b*/, float a) { update(a); }
};

void Histogram::analyse(const osg::Image* image, double /*interval*/)
{
    PopulateHistogram populateHistogram(_valueMap);
    readImage(image, populateHistogram);
    populateHistogram.normalize();

    for(Histogram::ValueMap::iterator itr = populateHistogram._histogram.begin();
        itr != populateHistogram._histogram.end();
        ++itr)
    {
        OSG_NOTICE<<"  "<<itr->first<<", "<<itr->second<<std::endl;
    }
}

void Histogram::insertZeroBoundaryValues(float xMin, float xMax)
{
    if (_valueMap.empty())
    {
        if (xMin<xMax)
        {
            _valueMap[xMin] = 0.0;
            _valueMap[xMax] = 0.0;
        }
        return;
    }

    float interval = 1.0f;
    float min_gap_for_single_insertion = interval*1.5;
    float min_gap_for_double_insertion = interval*2.5;

    if (xMin<_valueMap.begin()->first)
    {
        _valueMap[xMin] = 0.0;
    }

    if (xMax>_valueMap.rbegin()->first)
    {
        _valueMap[xMax] = 0.0;
    }

    ValueMap::iterator itr = _valueMap.begin();
    float previous_x = itr->first;
    for(;
        itr != _valueMap.end();
        ++itr)
    {
        float current_x = itr->first;
        float gap = current_x-previous_x;
        if (gap>min_gap_for_double_insertion)
        {
            _valueMap[previous_x+interval] = 0.0f;
            _valueMap[current_x-interval] = 0.0f;
        }
        else if  (gap>min_gap_for_single_insertion)
        {
            _valueMap[(previous_x+current_x)*0.5]=0.0f;
        }
        previous_x = current_x;
    }
}

osg::Node* Histogram::createGraphicalRepresentation()
{
    if (_valueMap.empty()) return 0;

    osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform;

    float xMin = _valueMap.begin()->first;
    float xMax = _valueMap.rbegin()->first;

    float depth = 0.0f;
    float yMax = 0.0f;

    // find yMax
    for(ValueMap::iterator itr = _valueMap.begin();
        itr != _valueMap.end();
        ++itr)
    {
        float y = itr->second;
        if (y>yMax) yMax = y;
    }

    float xScale = 1.0f/(xMax-xMin);
    float yScale = 1.0f/yMax;

    {
        osg::ref_ptr<osg::Geode> geode = new osg::Geode;
        transform->addChild(geode.get());

        osg::ref_ptr<osg::Geometry> geometry = new osg::Geometry;
        geode->addDrawable(geometry.get());
        geode->getOrCreateStateSet()->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
        geode->getOrCreateStateSet()->setMode(GL_BLEND, osg::StateAttribute::ON);

        osg::ref_ptr<osg::Vec3Array> vertices = new osg::Vec3Array;
        geometry->setVertexArray(vertices.get());

        osg::ref_ptr<osg::Vec4Array> colours = new osg::Vec4Array;
        geometry->setColorArray(colours.get(), osg::Array::BIND_PER_PRIMITIVE_SET);
        colours->push_back(osg::Vec4(1.0,1.0,1.0,1.0));
        colours->push_back(osg::Vec4(1.0,1.0,1.0,1.0));
        colours->push_back(osg::Vec4(1.0,1.0,1.0,0.1));


        unsigned numColumnsRequired = _valueMap.size();
        vertices->reserve(numColumnsRequired*3);
        for(ValueMap::iterator itr = _valueMap.begin();
            itr != _valueMap.end();
            ++itr)
        {
            float x = itr->first;
            float y = itr->second;

            vertices->push_back(osg::Vec3(x*xScale, 0.0f, depth));
            vertices->push_back(osg::Vec3(x*xScale, y*yScale, depth));
            vertices->push_back(osg::Vec3(x*xScale, yMax*yScale, depth));
        }

        osg::ref_ptr<osg::DrawElementsUShort> background_primitives = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
        osg::ref_ptr<osg::DrawElementsUShort> historgram_primitives = new osg::DrawElementsUShort(GL_TRIANGLE_STRIP);
        osg::ref_ptr<osg::DrawElementsUShort> outline_primitives = new osg::DrawElementsUShort(GL_LINE_STRIP);
        for(unsigned int i=0; i<numColumnsRequired; ++i)
        {
            int iv = i*3;

            background_primitives->push_back(iv+2);
            background_primitives->push_back(iv+1);

            historgram_primitives->push_back(iv+1);
            historgram_primitives->push_back(iv+0);

            outline_primitives->push_back(iv+1);

        }

        geometry->addPrimitiveSet(outline_primitives.get());
        geometry->addPrimitiveSet(historgram_primitives.get());
        geometry->addPrimitiveSet(background_primitives.get());
    }

    //transform->setMatrix(osg::Matrix::scale(xScale/(maxX-minY), yScale/(yMax), 1.0f));

    transform->setMatrix(osg::Matrix::scale(2.0,1.0,1.0)*osg::Matrix::rotate(osg::DegreesToRadians(90.0), osg::Vec3d(1.0,0.0,0.0)));

    return transform.release();
}

osg::TransferFunction1D* readTransferFunctionFile(const std::string& filename, float colorScale=1.0f)
{
    std::string foundFile = osgDB::findDataFile(filename);
    if (foundFile.empty())
    {
        std::cout<<"Error: could not find transfer function file : "<<filename<<std::endl;
        return 0;
    }

    std::cout<<"Reading transfer function "<<filename<<std::endl;

    osg::TransferFunction1D::ColorMap colorMap;
    osgDB::ifstream fin(foundFile.c_str());
    while(fin)
    {
        float value, red, green, blue, alpha;
        fin >> value >> red >> green >> blue >> alpha;
        if (fin)
        {
            std::cout<<"value = "<<value<<" ("<<red<<", "<<green<<", "<<blue<<", "<<alpha<<")"<<std::endl;
            colorMap[value] = osg::Vec4(red*colorScale,green*colorScale,blue*colorScale,alpha*colorScale);
        }
    }

    if (colorMap.empty())
    {
        std::cout<<"Error: No values read from transfer function file: "<<filename<<std::endl;
        return 0;
    }

    osg::TransferFunction1D* tf = new osg::TransferFunction1D;
    tf->assign(colorMap);

    return tf;
}

class FindTransferFunctionPropertyVisitor : public osgVolume::PropertyVisitor
{
public:

    osg::ref_ptr<osgVolume::TransferFunctionProperty> _tfp;

#if 0
    virtual void apply(osgVolume::SwitchProperty& sp)
    {
        OSG_NOTICE<<"Found SwitchProperty"<<std::endl;
        apply(static_cast<osgVolume::CompositeProperty&>(sp));
    }

    virtual void apply(osgVolume::CompositeProperty& cp)
    {
        OSG_NOTICE<<"Found CompositeProperty"<<std::endl;
        for(unsigned int i=0; i<cp.getNumProperties(); ++i)
        {
            cp.getProperty(i)->accept(*this);
        }
    }
#endif
    virtual void apply(osgVolume::TransferFunctionProperty& tfp)
    {
        OSG_NOTICE<<"Found TransferFunctionProperty "<<&tfp<<std::endl;
        _tfp = &tfp;
    }
};


class InsertTransferFunctionPropertyVisitor : public osgVolume::PropertyVisitor
{
public:

    InsertTransferFunctionPropertyVisitor(osg::TransferFunction1D* tf)
    {
        _tfp = new osgVolume::TransferFunctionProperty(tf);
    }

    osg::ref_ptr<osgVolume::TransferFunctionProperty> _tfp;

    virtual void apply(osgVolume::SwitchProperty& sp)
    {
        OSG_NOTICE<<"Found SwitchProperty"<<std::endl;
        for(unsigned int i=0; i<sp.getNumProperties(); ++i)
        {
            sp.getProperty(i)->accept(*this);
        }
    }

    virtual void apply(osgVolume::CompositeProperty& cp)
    {
        OSG_NOTICE<<"Found CompositeProperty, inserting transfer function"<<std::endl;
        if (_tfp.valid()) cp.addProperty(_tfp.get());
    }
};

class FindVolumeTiles : public osg::NodeVisitor
{
public:
    FindVolumeTiles(): osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {}

    typedef std::vector< osg::ref_ptr<osgVolume::VolumeTile> > Tiles;
    Tiles _tiles;

    void apply(osg::Group& group)
    {
        osgVolume::VolumeTile* tile = dynamic_cast<osgVolume::VolumeTile*>(&group);
        if (tile) _tiles.push_back(tile);
        else traverse(group);
    }
};



class MyScriptCallback : public osg::CallbackObject
{
public:
    MyScriptCallback(osg::ScriptEngine* se, osg::Script* script, const std::string& entryPoint) : _scriptEngine(se), _script(script) { setName(entryPoint); }

    virtual bool run(osg::Object* object, osg::Parameters& inputParameters, osg::Parameters& outputParameters) const
    {
        inputParameters.insert(inputParameters.begin(), object);
        return _scriptEngine->run(_script.get(), getName(), inputParameters, outputParameters);
    }

    osg::ref_ptr<osg::ScriptEngine> _scriptEngine;
    osg::ref_ptr<osg::Script> _script;
};

class MyClass : public osg::Object
{
public:
    MyClass() {}
    MyClass(const MyClass& rhs, const osg::CopyOp copyop=osg::CopyOp::SHALLOW_COPY):osg::Object(rhs,copyop) {}
    META_Object(local,MyClass)

    void myMethod()
    {
        osg::CallbackObject* co = osg::getCallbackObject(this, "myMethod");
        if (co) co->run(this);
        else myMethodImplementation();
    }

    virtual void myMethodImplementation()
    {
        OSG_NOTICE<<"MyClass::myMethodImplementation()"<<std::endl;
    }
};


int main(int argc, char ** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

#if 0
    osg::ref_ptr<MyClass> myobject = new MyClass;
    myobject->getOrCreateUserDataContainer()->addUserObject(new osg::CallbackObject("myMethod"));
    myobject->myMethod();

    osg::ref_ptr<osg::ScriptEngine> se = osgDB::readRefFile<osg::ScriptEngine>("ScriptEngine.lua");
    osg::ref_ptr<osg::Script> script = osgDB::readRefFile<osg::Script>("script.lua");

    osg::ref_ptr<MyClass> copyobject = new MyClass;
    copyobject->getOrCreateUserDataContainer()->addUserObject(new MyScriptCallback(se.get(), script.get(), "myMethod"));
    copyobject->myMethod();
#endif

#if 0
    osg::ref_ptr<osg::Object> object = osgDB::readRefNodeFile("load.lua");
    if (object.valid())
    {
        osg::CallbackObject* co = osg::getCallbackObject(object.get(), "method");
        OSG_NOTICE<<"Have object = "<<object->className()<<std::endl;
        OSG_NOTICE<<"Have co = "<<co<<std::endl;
        if (co)
        {
            osg::Parameters inputParameters, outputParameters;
            inputParameters.push_back(new osg::StringValueObject("string",std::string("all there is to celeberate is here.")));
            co->run(object.get(), inputParameters, outputParameters);
            OSG_NOTICE<<"outputParameters.size()="<<outputParameters.size()<<std::endl;
            for(osg::Parameters::iterator itr = outputParameters.begin();
                itr != outputParameters.end();
                ++itr)
            {
                OSG_NOTICE<<"   returned "<<(*itr)->className()<<std::endl;
            }
        }
    }
    return 0;
#endif


    #if 1
    osgViewer::Viewer viewer(arguments);

    viewer.addEventHandler(new osgViewer::StatsHandler());

    osg::ref_ptr<osg::TransferFunction1D> tf;
    std::string filename;
    if (arguments.read("--tf",filename))
    {
        tf = readTransferFunctionFile(filename, 1.0f);
    }
    if (arguments.read("--tf-255",filename))
    {
        tf = readTransferFunctionFile(filename,1.0f/255.0f);
    }

    bool createHistorgram = arguments.read("--histogram");

    osg::ref_ptr<osg::Node> model = osgDB::readRefNodeFiles(arguments);

    typedef std::vector< osg::ref_ptr<osg::Node> > Nodes;
    Nodes nodes;

    if (!model && !tf)
    {
        OSG_NOTICE<<"Please specify dataset on command line."<<std::endl;
        return 1;
    }

    osgVolume::ImageLayer* imageLayer = 0;


    if (model.valid())
    {
        osg::ref_ptr<osgVolume::VolumeTile> volumeTile = dynamic_cast<osgVolume::VolumeTile*>(model.get());
        if (volumeTile.valid())
        {
            OSG_NOTICE<<"Inserting Volume above VolumeTile."<<std::endl;
            osg::ref_ptr<osgVolume::Volume> volume = new osgVolume::Volume;
            volume->addChild(model.get());
            model = volume.get();

//            volumeTile->setVolumeTechnique(new osgVolume::RayTracedTechnique);
//            volumeTile->setVolumeTechnique(new osgVolume::FixedFunctionTechnique);
        }

        nodes.push_back(model.get());

        FindVolumeTiles fvt;
        model->accept(fvt);

        if (!fvt._tiles.empty())
        {
            osgVolume::VolumeTile* tile = fvt._tiles[0].get();
            imageLayer = dynamic_cast<osgVolume::ImageLayer*>(tile->getLayer());
            tile->addEventCallback(new osgVolume::PropertyAdjustmentCallback());
        }
    }


    if (createHistorgram && imageLayer)
    {
        Histogram histogram;
        histogram.analyse(imageLayer->getImage());
        nodes.push_back(histogram.createGraphicalRepresentation());
    }

    if (imageLayer)
    {
        osgVolume::Property* property = imageLayer->getProperty();
        if (property)
        {
            FindTransferFunctionPropertyVisitor ftfpv;
            property->accept(ftfpv);

            if (ftfpv._tfp.valid())
            {
                if (tf.valid())
                {
                    OSG_NOTICE<<"Need to replace volumes transfer function"<<std::endl;
                    ftfpv._tfp->setTransferFunction(tf.get());
                }
                else
                {
                    OSG_NOTICE<<"Using volumes transfer function"<<std::endl;
                    tf = dynamic_cast<osg::TransferFunction1D*>(ftfpv._tfp->getTransferFunction());
                }
            }
            else if (tf.valid())
            {
                // No existing transfer function but need to assign one
                OSG_NOTICE<<"Need to assign transfer function to CompositeProperty"<<std::endl;
                InsertTransferFunctionPropertyVisitor itfpv(tf.get());
                property->accept(itfpv);
            }
        }
        else if (tf.valid())
        {
            OSG_NOTICE<<"Assign transfer function directly"<<std::endl;
            imageLayer->setProperty(new osgVolume::TransferFunctionProperty(tf.get()));
        }
    }

    if (tf.valid())
    {
        osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform;
        transform->setMatrix(osg::Matrix::scale(2.0,1.0,1.0)*osg::Matrix::rotate(osg::DegreesToRadians(90.0), osg::Vec3d(1.0,0.0,0.0)));
        transform->addChild(new osgUI::TransferFunctionWidget(tf.get()));
        nodes.push_back(transform.get());
    }

    if (nodes.empty())
    {
        OSG_NOTICE<<"Please specify dataset on command line."<<std::endl;
        return 1;
    }

    if (nodes.size()==1)
    {
        viewer.setSceneData(nodes[0].get());
    }
    else
    {
        osg::Vec3d position(0.0,0.0,0.0);

        osg::ref_ptr<osg::Group> group = new osg::Group;

        for(Nodes::iterator itr = nodes.begin();
            itr != nodes.end();
            ++itr)
        {
            osg::ref_ptr<osg::Node> child = *itr;
            if (!child) continue;
#if 0
            osg::ComputeBoundsVisitor cbv;
            child->accept(cbv);

            osg::BoundingBox bb = cbv.getBoundingBox();
            double scale = 1.0/(bb.xMax()-bb.xMin());
#endif
            osg::BoundingSphere bb = child->getBound();
            double scale = 0.7/bb.radius();

            osg::ref_ptr<osg::PositionAttitudeTransform> pat = new osg::PositionAttitudeTransform;
            pat->addChild(child.get());
            pat->setPosition(position);
            pat->setPivotPoint(bb.center());
            pat->setScale(osg::Vec3d(scale, scale, scale));
            position.x() += 1.1;

            group->addChild(pat.get());
        }

        viewer.setSceneData(group.get());
    }

    OSG_NOTICE<<"Reading to run viewer"<<std::endl;

    osgDB::writeNodeFile(*viewer.getSceneData(),"graph.osgt");

    return viewer.run();
#endif

}
