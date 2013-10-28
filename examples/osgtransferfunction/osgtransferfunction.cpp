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
#include <osg/io_utils>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/FileUtils>

#include <osgVolume/VolumeTile>

#include <osgViewer/Viewer>



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


    viewer.setSceneData( model.get() );

    return viewer.run();
}
