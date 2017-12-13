#include "OsgMainApp.hpp"


OsgMainApp::OsgMainApp(){

    _lodScale = 1.0f;
    _prevFrame = 0;

    _initialized = false;
    _clean_scene = false;

}
OsgMainApp::~OsgMainApp(){

}
void OsgMainApp::loadModels(){
    if(_vModelsToLoad.size()==0) return;

    osg::notify(osg::ALWAYS)<<"There are "<<_vModelsToLoad.size()<<" models to load"<<std::endl;

    Model newModel;
    for(unsigned int i=0; i<_vModelsToLoad.size(); i++){
        newModel = _vModelsToLoad[i];
        osg::notify(osg::ALWAYS)<<"Loading: "<<newModel.filename<<std::endl;

        osg::ref_ptr<osg::Node> loadedModel = osgDB::readRefNodeFile(newModel.filename);
        if (loadedModel == 0) {
            osg::notify(osg::ALWAYS)<<"Model not loaded"<<std::endl;
        } else {
            osg::notify(osg::ALWAYS)<<"Model loaded"<<std::endl;
            _vModels.push_back(newModel);

            loadedModel->setName(newModel.name);
            _root->addChild(loadedModel);
        }
    }

    _viewer->setSceneData(NULL);
    _viewer->setSceneData(_root.get());
    _manipulator->getNode();
    _viewer->home();

    _viewer->getDatabasePager()->clear();
    _viewer->getDatabasePager()->registerPagedLODs(_root.get());
    _viewer->getDatabasePager()->setUpThreads(3, 1);
    _viewer->getDatabasePager()->setTargetMaximumNumberOfPageLOD(2);
    _viewer->getDatabasePager()->setUnrefImageDataAfterApplyPolicy(true, true);

    _vModelsToLoad.clear();

}
void OsgMainApp::deleteModels(){
    if(_vModelsToDelete.size()==0) return;

    osg::notify(osg::ALWAYS)<<"There are "<<_vModelsToDelete.size()<<" models to delete"<<std::endl;

    Model modelToDelete;
    for(unsigned int i=0; i<_vModelsToDelete.size(); i++){
        modelToDelete = _vModelsToDelete[i];
        osg::notify(osg::ALWAYS)<<"Deleting: "<<modelToDelete.name<<std::endl;

        for(unsigned int j=_root->getNumChildren(); j>0; j--){
            osg::ref_ptr<osg::Node> children = _root->getChild(j-1);
            if(children->getName() == modelToDelete.name){
                _root->removeChild(children);
            }
        }

    }

    _vModelsToDelete.clear();
    osg::notify(osg::ALWAYS)<<"finished"<<std::endl;
}
//Initialization function
void OsgMainApp::initOsgWindow(int x,int y,int width,int height){

    __android_log_write(ANDROID_LOG_ERROR, "OSGANDROID",
            "Initializing geometry");

    //Pending
    _notifyHandler = new OsgAndroidNotifyHandler();
    _notifyHandler->setTag("Osg Viewer");
    osg::setNotifyHandler(_notifyHandler);

    osg::notify(osg::ALWAYS)<<"Testing"<<std::endl;

    _viewer = new osgViewer::Viewer();
    _viewer->setUpViewerAsEmbeddedInWindow(x, y, width, height);

    _root = new osg::Group();

    _viewer->realize();

    _state = _root->getOrCreateStateSet();
    _state->setMode(GL_LIGHTING, osg::StateAttribute::ON);
    _state->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);
    _state->setMode(GL_CULL_FACE, osg::StateAttribute::ON);

    _viewer->setSceneData(_root.get());

    _viewer->addEventHandler(new osgViewer::StatsHandler);
    _viewer->addEventHandler(new osgGA::StateSetManipulator(_viewer->getCamera()->getOrCreateStateSet()));
    _viewer->addEventHandler(new osgViewer::ThreadingHandler);
    _viewer->addEventHandler(new osgViewer::LODScaleHandler);

    _manipulator = new osgGA::KeySwitchMatrixManipulator;

    _manipulator->addMatrixManipulator( '1', "Trackball", new osgGA::TrackballManipulator() );
    _manipulator->addMatrixManipulator( '2', "Flight", new osgGA::FlightManipulator() );
    _manipulator->addMatrixManipulator( '3', "Drive", new osgGA::DriveManipulator() );
    _manipulator->addMatrixManipulator( '4', "Terrain", new osgGA::TerrainManipulator() );
    _manipulator->addMatrixManipulator( '5', "Orbit", new osgGA::OrbitManipulator() );
    _manipulator->addMatrixManipulator( '6', "FirstPerson", new osgGA::FirstPersonManipulator() );
    _manipulator->addMatrixManipulator( '7', "Spherical", new osgGA::SphericalManipulator() );

    _viewer->setCameraManipulator( _manipulator.get() );

    _viewer->getViewerStats()->collectStats("scene", true);

    _initialized = true;

}
//Draw
void OsgMainApp::draw(){
    //Every load o remove has to be done before any drawing
    loadModels();
    deleteModels();

    _viewer->frame();
}
//Events
void OsgMainApp::mouseButtonPressEvent(float x,float y,int button){
    _viewer->getEventQueue()->mouseButtonPress(x, y, button);
}
void OsgMainApp::mouseButtonReleaseEvent(float x,float y,int button){
    _viewer->getEventQueue()->mouseButtonRelease(x, y, button);
}
void OsgMainApp::mouseMoveEvent(float x,float y){
    _viewer->getEventQueue()->mouseMotion(x, y);
}
void OsgMainApp::keyboardDown(int key){
    _viewer->getEventQueue()->keyPress(key);
}
void OsgMainApp::keyboardUp(int key){
    _viewer->getEventQueue()->keyRelease(key);
}
//Loading and unloading
void OsgMainApp::loadObject(std::string filePath){
    Model newModel;
    newModel.filename = filePath;
    newModel.name = filePath;

    int num = 0;
    for(unsigned int i=0;i<_vModels.size();i++){
        if(_vModels[i].name==newModel.name)
            return;
    }

    _vModelsToLoad.push_back(newModel);

}
void OsgMainApp::loadObject(std::string name,std::string filePath){

    Model newModel;
    newModel.filename = filePath;
    newModel.name = name;

    for(unsigned int i=0;i<_vModels.size();i++){
        if(_vModels[i].name==newModel.name){
            osg::notify(osg::ALWAYS)<<"Name already used"<<std::endl;
            return;
        }
    }

    _vModelsToLoad.push_back(newModel);
}
void OsgMainApp::unLoadObject(int number){
    if(_vModels.size() <= number){
        osg::notify(osg::FATAL)<<"Index number error"<<std::endl;
        return;
    }

    Model modelToDelete = _vModels[number];
    _vModels.erase(_vModels.begin()+number);
    _vModelsToDelete.push_back(modelToDelete);
}
void OsgMainApp::clearScene(){
    _vModelsToDelete = _vModels;
    _vModels.clear();
}
//Other Functions
int OsgMainApp::getNumberObjects(){
    return _vModels.size();
}
std::string OsgMainApp::getObjectName(int number){
    return _vModels[number].name;
}
void OsgMainApp::setClearColor(osg::Vec4f color){
    osg::notify(osg::ALWAYS)<<"Setting Clear Color"<<std::endl;
    _viewer->getCamera()->setClearColor(color);
}
osg::Vec4f OsgMainApp::getClearColor(){
    osg::notify(osg::ALWAYS)<<"Getting Clear Color"<<std::endl;
    return _viewer->getCamera()->getClearColor();
}
