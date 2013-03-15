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

#include <osg/Notify>
#include <osg/io_utils>

#include <osgDB/ReadFile>
#include <osgGA/TrackballManipulator>
#include <osgViewer/Viewer>



int main( int argc, char **argv )
{
    osg::ArgumentParser arguments(&argc,argv);
    
    // initialize the viewer.
    osgViewer::Viewer viewer(arguments);

    osg::Vec2d translate(0.0,0.0);
    osg::Vec2d scale(1.0,1.0);
    osg::Vec2d taper(1.0,1.0);
    double angle = 0; // osg::inDegrees(45.0);

    if (arguments.read("-a",angle)) { OSG_NOTICE<<"angle = "<<angle<<std::endl; angle = osg::inDegrees(angle); }
    if (arguments.read("-t",translate.x(), translate.y())) { OSG_NOTICE<<"translate = "<<translate<<std::endl;}
    if (arguments.read("-s",scale.x(), scale.y())) { OSG_NOTICE<<"scale = "<<scale<<std::endl;}
    if (arguments.read("-k",taper.x(), taper.y())) { OSG_NOTICE<<"taper = "<<taper<<std::endl;}

    

    osg::ref_ptr<osg::Node> model = osgDB::readNodeFiles(arguments);

    if (!model)
    {
        OSG_NOTICE<<"No models loaded, please specify a model file on the command line"<<std::endl;
        return 1;
    }

    viewer.setSceneData(model.get());

    viewer.setCameraManipulator(new osgGA::TrackballManipulator());

    viewer.realize();

    viewer.getCamera()->setComputeNearFarMode(osg::Camera::DO_NOT_COMPUTE_NEAR_FAR);

    osg::Matrixd& pm = viewer.getCamera()->getProjectionMatrix();

    pm.postMultRotate(osg::Quat(angle, osg::Vec3d(0.0,0.0,1.0)));
    pm.postMultScale(osg::Vec3d(scale.x(),scale.y(),1.0));
    pm.postMultTranslate(osg::Vec3d(translate.x(),translate.y(),0.0));

    if (taper.x()!=1.0)
    {
        double x0 = (1.0+taper.x())/(1-taper.x());
        OSG_NOTICE<<"x0 = "<<x0<<std::endl;

        pm.postMult(osg::Matrixd(1.0-x0, 0.0,    0.0,    1.0,
                                 0.0,    1.0-x0, 0.0,    0.0,
                                 0.0,    0.0,    (1.0-x0)*0.5, 0.0,
                                 0.0,    0.0,    0.0,    -x0));
    }
    
    return viewer.run();
}
