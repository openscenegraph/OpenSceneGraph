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
#include <osg/io_utils>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>

#include <osgVolume/VolumeTile>

#include <osgViewer/Viewer>


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
    void luminance_alpha(float l, float a) { update(l); }
    void rgb(float r, float g, float b) { update(r); }
    void rgba(float r, float g, float b, float a) { update(a); }
};

void Histogram::analyse(const osg::Image* image, double interval)
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

osg::Node* createGraphicalRepresentation(osg::TransferFunction1D* tf, unsigned int channel)
{
    typedef osg::TransferFunction1D::ColorMap ColorMap;
    ColorMap& colorMap = tf->getColorMap();
    if (colorMap.empty()) return 0;

    osg::ref_ptr<osg::MatrixTransform> transform = new osg::MatrixTransform;

    float xMin = colorMap.begin()->first;
    float xMax = colorMap.rbegin()->first;

    float depth = 0.0f;
    float yMax = 0.0f;

    // find yMax
    for(ColorMap::iterator itr = colorMap.begin();
        itr != colorMap.end();
        ++itr)
    {
        float y = itr->second[channel];
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


        unsigned numColumnsRequired = colorMap.size();
        vertices->reserve(numColumnsRequired*3);
        for(ColorMap::iterator itr = colorMap.begin();
            itr != colorMap.end();
            ++itr)
        {
            float x = itr->first;
            float y = itr->second[channel];

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


int main(int argc, char ** argv)
{
    osg::ArgumentParser arguments(&argc, argv);

    osgViewer::Viewer viewer(arguments);

    osg::ref_ptr<osg::TransferFunction1D> tf;
    std::string filename;
    if (arguments.read("--tf",filename))
    {
        tf = readTransferFunctionFile(filename);
    }

    osg::ref_ptr<osgVolume::Layer> layer;

    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles(arguments);

    osgVolume::VolumeTile* volumeTile = dynamic_cast<osgVolume::VolumeTile*>(model.get());
    if (!volumeTile)
    {
        OSG_NOTICE<<"Please specify volume dataset on command line."<<std::endl;
        return 1;
    }

    layer = volumeTile->getLayer();

    if (!layer)
    {
        OSG_NOTICE<<"No layer loaded."<<std::endl;
        return 1;
    }

    osg::ref_ptr<osgVolume::ImageLayer> imageLayer = dynamic_cast<osgVolume::ImageLayer*>(layer.get());

    osg::ref_ptr<osg::Image> image = imageLayer.valid() ? imageLayer->getImage() : 0;

    if (!image)
    {
        OSG_NOTICE<<"No image found."<<std::endl;
    }

    Histogram histogram;
    histogram.analyse(image.get());

    osg::ref_ptr<osg::Group> group = new osg::Group;
    group->addChild(histogram.createGraphicalRepresentation());
    //if (tf.valid()) group->addChild(createGraphicalRepresentation(tf.get(),1));

    viewer.setSceneData(group.get());

    osgDB::writeNodeFile(*viewer.getSceneData(),"graph.osgt");

    return viewer.run();
}
