#include <osgProducer/ReadCameraConfigFile>


using namespace osgProducer;


Producer::CameraConfig* buildConfig()
{
    Producer::RenderSurface *rs1 = new Producer::RenderSurface;
    rs1->setScreenNum(0);
//     rs1->useBorder(false);
//     rs1->setWindowRect(0,0,640,480);
    rs1->setWindowRect(10,10,620,480);

    Producer::Camera *camera1 = new Producer::Camera;
    camera1->setRenderSurface(rs1);
    camera1->setOffset( 1.0, 0.0 );


    Producer::RenderSurface *rs2 = new Producer::RenderSurface;
    rs2->setScreenNum(0);
//     rs2->useBorder(false);
//     rs2->setWindowRect(640,0,640,480);
    rs2->setWindowRect(650,10,620,480);

    Producer::Camera *camera2 = new Producer::Camera;
    camera2->setRenderSurface(rs2);
    camera2->setOffset( -1.0, 0.0 );

    Producer::CameraConfig *cfg = new Producer::CameraConfig;
    cfg->addCamera("Camera 1",camera1);
    cfg->addCamera("Camera 2", camera2);

    Producer::InputArea *ia = new Producer::InputArea;
    ia->addInputRectangle( rs1, Producer::InputRectangle(-1.0,0.0,-1.0,1.0));
    ia->addInputRectangle( rs2, Producer::InputRectangle(0.0,1.0,-1.0,1.0));

    cfg->setInputArea(ia);

    return cfg;
}

Producer::CameraConfig* osgProducer::readCameraConfigFile(const std::string& filename)
{
    return buildConfig();
}
