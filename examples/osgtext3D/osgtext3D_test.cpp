
#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>
#include <osgDB/ReadFile>

#include <osgGA/TrackballManipulator>
#include <osgGA/StateSetManipulator>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/Shape>
#include <osg/ShapeDrawable>
#include <osgText/Text3D>

#include <iostream>
#include <sstream>



osg::Group* test_create3DText(const osg::Vec3& center,float radius)
{

    osg::Geode* geode  = new osg::Geode;

    float characterSize=radius*0.2f;
    float characterDepth=characterSize*0.2f;

    osg::Vec3 pos(center.x()-radius*.5f,center.y()-radius*.5f,center.z()-radius*.5f);
#define SHOW_INTESECTION_CEASH
#ifdef SHOW_INTESECTION_CEASH
    osgText::Text3D* text3 = new osgText::Text3D;
    text3->setFont("fonts/dirtydoz.ttf");
    text3->setCharacterSize(characterSize);
    text3->setCharacterDepth(characterDepth);
    text3->setPosition(pos);
    text3->setDrawMode(osgText::Text3D::TEXT | osgText::Text3D::BOUNDINGBOX);
    text3->setAxisAlignment(osgText::Text3D::XZ_PLANE);
    text3->setText("CRAS H"); //intersection crash
    geode->addDrawable(text3);
#else
    osgText::Text3D* text7 = new osgText::Text3D;
    text7->setFont("fonts/times.ttf");
    text7->setCharacterSize(characterSize);
    text7->setCharacterDepth(characterSize*2.2f);
    text7->setPosition(center - osg::Vec3(0.0, 0.0, 0.6));
    text7->setDrawMode(osgText::Text3D::TEXT | osgText::Text3D::BOUNDINGBOX);
    text7->setAxisAlignment(osgText::Text3D::SCREEN);
    text7->setCharacterSizeMode(osgText::Text3D::OBJECT_COORDS);
    text7->setText("ABCDE"); //wrong intersection
    geode->addDrawable(text7);
#endif

    osg::ShapeDrawable* shape = new osg::ShapeDrawable(new osg::Sphere(center,characterSize*0.2f));
    shape->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::ON);
    geode->addDrawable(shape);

    osg::Group* rootNode = new osg::Group;
    rootNode->addChild(geode);

#define SHOW_WRONG_NORMAL
#ifdef SHOW_WRONG_NORMAL
    osg::Material* front = new osg::Material; //
    front->setAlpha(osg::Material::FRONT_AND_BACK,1);
    front->setAmbient(osg::Material::FRONT_AND_BACK,osg::Vec4(0.2,0.2,0.2,1.0));
    front->setDiffuse(osg::Material::FRONT_AND_BACK,osg::Vec4(.0,.0,1.0,1.0));
    rootNode->getOrCreateStateSet()->setAttributeAndModes(front);
#else
    osg::StateSet* stateset = new osg::StateSet; //Show wireframe
    osg::PolygonMode* polymode = new osg::PolygonMode;
    polymode->setMode(osg::PolygonMode::FRONT_AND_BACK,osg::PolygonMode::LINE);
    stateset->setAttributeAndModes(polymode,osg::StateAttribute::OVERRIDE|osg::StateAttribute::ON);
    rootNode->setStateSet(stateset);
#endif


    return rootNode;
}

//////////////////////////////////////////////////////////////////////////
#include <osg/PositionAttitudeTransform>
#include <osg/ShapeDrawable>
class CInputHandler : public osgGA::GUIEventHandler
{
public:
  CInputHandler( osg::PositionAttitudeTransform* pPatSphere )
  {
    m_rPatSphere = pPatSphere;
  }
  virtual bool handle( const osgGA::GUIEventAdapter& ea, osgGA::GUIActionAdapter& aa, osg::Object* /*pObject*/, osg::NodeVisitor* /*pNodeVisitor*/ )
  {
    osgViewer::Viewer* pViewer = dynamic_cast<osgViewer::Viewer*>(&aa);
    if ( !pViewer )
    {
      return false;
    }

    if ( ea.getEventType()==osgGA::GUIEventAdapter::PUSH )
    {
      osgViewer::ViewerBase::Cameras cams;
      pViewer->getCameras( cams );

      float x = ea.getXnormalized();
      float y = ea.getYnormalized();

      osgUtil::LineSegmentIntersector* picker = new osgUtil::LineSegmentIntersector( osgUtil::Intersector::PROJECTION, x, y );
      osgUtil::IntersectionVisitor iv( picker );
      cams[0]->accept( iv );

      if ( picker->containsIntersections() )
      {
        osgUtil::LineSegmentIntersector::Intersection intersection = picker->getFirstIntersection();
        osg::Vec3 v = intersection.getWorldIntersectPoint();
        m_rPatSphere->setPosition( v );
      }

      return true; // return true, event handled
    }

    return false;
  }

private:
  osg::ref_ptr<osg::PositionAttitudeTransform> m_rPatSphere;
};
//////////////////////////////////////////////////////////////////////////
int main_test(int, char**)
{
    osgViewer::Viewer viewer;
    viewer.setUpViewInWindow(99,99,666,666, 0);
    osg::ref_ptr<osg::PositionAttitudeTransform> rPat = new osg::PositionAttitudeTransform;
    // add the handler to the viewer
    viewer.addEventHandler( new CInputHandler(rPat.get()) );
    // create a group to contain our scene and sphere
    osg::Group* pGroup = new osg::Group;
    // create sphere
    osg::Geode* pGeodeSphere = new osg::Geode;
    pGeodeSphere->addDrawable( new osg::ShapeDrawable( new osg::Sphere(osg::Vec3(0.0f,0.0f,0.0f),0.01f) ) );
    rPat->addChild( pGeodeSphere );
    pGroup->addChild( rPat.get() );

    osg::Vec3 center(0.0f,0.0f,0.0f);
    float radius = 1.0f;

    osg::Group* root = new osg::Group;
    root->addChild(test_create3DText(center, radius));

    //viewer.setSceneData(root);
    pGroup->addChild(root);
    viewer.setSceneData(pGroup);
    viewer.setCameraManipulator(new osgGA::TrackballManipulator());
    viewer.addEventHandler( new osgGA::StateSetManipulator(viewer.getCamera()->getOrCreateStateSet()) );

    viewer.addEventHandler(new osgViewer::ThreadingHandler);
    viewer.addEventHandler(new osgViewer::WindowSizeHandler);
    viewer.addEventHandler(new osgViewer::StatsHandler);

    return viewer.run();
}


