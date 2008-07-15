// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008
// $Id: Util.cpp 59 2008-05-15 20:55:31Z cubicool $

#include <osg/io_utils>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>
#include <osgDB/FileUtils>
#include <osgDB/WriteFile>
#include <osgViewer/ViewerEventHandlers>
#include <osgWidget/Util>
#include <osgWidget/ViewerEventHandlers>
#include <osgWidget/WindowManager>

namespace osgWidget {

std::string getFilePath(const std::string& filename) {
	osgDB::FilePathList path;

	char* fp = getenv("OSGWIDGET_FILE_PATH");
	
	osgDB::convertStringPathIntoFilePathList(fp ? fp : ".", path);

	return osgDB::findFileInPath(filename, path);
}

std::string generateRandomName(const std::string& base) {
	static unsigned int count = 0;
	
	std::stringstream ss;

	ss << base << "_" << count;
	
	count++;

	return ss.str();
}

osg::Matrix createInvertedYOrthoProjectionMatrix(matrix_type width, matrix_type height) {
	osg::Matrix m = osg::Matrix::ortho2D(0.0f, width, 0.0f, height);
	osg::Matrix s = osg::Matrix::scale(1.0f, -1.0f, 1.0f);
	osg::Matrix t = osg::Matrix::translate(0.0f, -height, 0.0f);

	return t * s * m;
}

osg::Camera* createOrthoCamera(matrix_type width, matrix_type height) {
	osg::Camera* camera = new osg::Camera();

	camera->getOrCreateStateSet()->setMode(
		GL_LIGHTING,
		osg::StateAttribute::PROTECTED | osg::StateAttribute::OFF
	);

	camera->setProjectionMatrix(osg::Matrix::ortho2D(0.0, width, 0.0f, height));
	camera->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
	camera->setViewMatrix(osg::Matrix::identity());
	camera->setClearMask(GL_DEPTH_BUFFER_BIT);
	camera->setRenderOrder(osg::Camera::POST_RENDER);
	
	return camera;
}

osg::Camera* createInvertedYOrthoCamera(matrix_type width, matrix_type height) {
	osg::Camera* camera = createOrthoCamera(width, height);

	camera->setProjectionMatrix(createInvertedYOrthoProjectionMatrix(width, height));
	
	return camera;
}

osg::Group* _createExampleCommon(osgViewer::View* view, WindowManager* wm, osg::Node* node) {
	if(!wm) return 0;

	view->setUpViewInWindow(
		0,
		0,
		static_cast<int>(wm->getWidth()),
		static_cast<int>(wm->getHeight())
	);

	osg::Group*  group  = new osg::Group();
	osg::Camera* camera = wm->createParentOrthoCamera();

	group->addChild(camera);

	if(node) group->addChild(node);

	view->addEventHandler(new osgWidget::MouseHandler(wm));
	view->addEventHandler(new osgWidget::KeyboardHandler(wm));
	view->addEventHandler(new osgWidget::ResizeHandler(wm, camera));
	view->addEventHandler(new osgViewer::StatsHandler());
	view->addEventHandler(new osgViewer::WindowSizeHandler());
	view->addEventHandler(new osgGA::StateSetManipulator(
		view->getCamera()->getOrCreateStateSet()
	));

	wm->resizeAllWindows();
	
	return group;
}

int createExample(osgViewer::Viewer& viewer, WindowManager* wm, osg::Node* node) {
	osg::Group* group = _createExampleCommon(&viewer, wm, node);

	viewer.setSceneData(group);

	return viewer.run();
}

// TODO: This function is totally broken; I don't really have any idea of how to do this.
// Incredibly frustrating stuff.
int createCompositeExample(
	osgViewer::CompositeViewer& viewer,
	osgViewer::View*            view,
	WindowManager*              wm,
	osg::Node*                  node
) {
	osg::Group*           group   = _createExampleCommon(view, wm, node);
	osg::MatrixTransform* watcher = new osg::MatrixTransform();

	watcher->addChild(wm);

	// Setup the main 2D view.
	viewer.addView(view);

	view->setSceneData(group);

	// The view that "watches" the main view.
	osgViewer::View* viewWatcher = new osgViewer::View();

	viewer.addView(viewWatcher);

	int w = static_cast<int>(wm->getWidth());
	int h = static_cast<int>(wm->getHeight());
	
	viewWatcher->setUpViewInWindow(0, 0, w, h);

	// Setup our parent MatrixTransform so things look right in perspective.
	watcher->setMatrix(
		osg::Matrix::scale(1.0f, -1.0f, 1000.0f) *
		osg::Matrix::rotate(osg::DegreesToRadians(90.0f), osg::Vec3d(1.0f, 0.0f, 0.0f))
	);

	watcher->getOrCreateStateSet()->setAttributeAndModes(
		new osg::Scissor(0, 0, w, h),
		osg::StateAttribute::OVERRIDE
	);

	osgGA::TrackballManipulator* tb = new osgGA::TrackballManipulator();

	warn() << watcher->getMatrix() << std::endl;

	/*
	const osg::BoundingSphere& bs = watcher->getBound();

	tb->setHomePosition(
		bs.center() + osg::Vec3(0.0f, -3.5f * bs.radius(), 0.0f),
		bs.center(),
		osg::Vec3(0.0f, 1.0f, 0.0f)
	);
	*/

	viewWatcher->setSceneData(watcher);
	viewWatcher->setCameraManipulator(tb);

	return viewer.run();
}

bool writeWindowManagerNode(WindowManager* wm) {
	osgDB::writeNodeFile(*wm->getParent(0), "osgWidget.osg");

	return true;
}

}
