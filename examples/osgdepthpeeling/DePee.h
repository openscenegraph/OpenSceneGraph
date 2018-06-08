/*
  Steffen Frey
  Fachpraktikum Graphik-Programmierung 2007
  Institut fuer Visualisierung und Interaktive Systeme
  Universitaet Stuttgart
 */

#ifndef _DEPEE_H_
#define _DEPEE_H_

#include <osg/Node>
#include <osg/Camera>
#include <osg/Group>
#include <osg/Texture2D>
#include <osgText/Text>
#include <string>
#include <stack>

#include "DePeePass.h"

/*!
  The DePee class is main class for setting up and managing depth peeling. 
  A DePee object can be seen as a virtual node, that has one parent and one child. The rendering of every child and subchil of this child is managed by the DePee node. Besides that, it handles a head up display.
 */
class DePee : public osg::Referenced
{
public:
  /*!
    The constructor is initialized by giving it a parent and child node (subgraph), as well as the width and height in pixels of the output window. Additionally a subgraph can be added whose children aren't depth peeled but combined with de depth peeled scene
   */
  DePee(osg::Group* parent, osg::Group* subgraph, unsigned width, unsigned height);
  /*!
    Takes care of clean removal of DePee
   */
  ~DePee();
  
  /*!
    The head up display shows information like internal status and current frames per second. This function needs to be called in the rendering loop to keep the information updated.
   */
  bool updateHUDText();

  /*!
    Sets whether sketchiness is activated or deactivated
   */
  void setSketchy(bool sketchy);
  
  /*!
    If sketchiness is enabled, sets whether a crayon should be used
   */
  void setCrayon(bool crayon);
  
  /*!
    Sets whether color display is activated or deactivated
   */
  void setColored(bool colored);
  
  /*!
    Sets whether edges are displayed or not
   */
  void setEdgy(bool edgy);

  /*!
    Sets how sketchy lines and colors should be displayed (standard is 1.0)
   */
  void setSketchiness(double sketchiness);
  
  /*!
    Set the pointer to the double variable containing the current fps for displaying it on the head up display
   */
  void setFPS(double* fps);

  /*!
    Add a depth peeling pass and adjust the render passes accordingly
   */
  bool addDePeePass();

  /*!
    Remove a depth peeling pass and adjust the render passes accordingly
   */
  bool remDePeePass();
  
private:
  /*!
    Create a map. This is a function for convenience and calls either 
    createNoiseMap(), createEdgeMap() or createNormalDepthColorMap().
    Apart from NOISE_MAP, for every texture generation 
    one rendering pass is needed.
    The boolean first is used to indicate whether this rendering pass
    belongs to the first depth peeling pass.
   */
  bool createMap(MapMode mapMode, bool first=false);

  /*!
    Creates a two dimensional noise map and initializes _noiseMap with it
   */
  bool createNoiseMap();

  /*!
    Depending on the chosen MapMode, it either creates a new rendering
    pass for creaeting a normal, depth or color map. The created rendering
    pass is added to the current depth peeling pass.
   */
  bool createNormalDepthColorMap(MapMode mapMode, bool first);
  
  /*!
    Create an edge map. A previous depth and normal rendering pass in this 
    depth peeling pass is required for that.
   */
  bool createEdgeMap(bool first);
  
  /*!
    Creates the final rendering pass for depth peeling. Color and edge map are
    added up here and sketchiness is applied.
   */
  bool createFinal();
  
  /*!
    Create the rendering pass for the head up display
   */
  bool createHUD();

  /*
    Returns the number of rendering passes of the depth peeling object
   */
  unsigned int getNumberOfRenderPasses();
  
  
  unsigned _texWidth;
  unsigned _texHeight;
  unsigned _width;
  unsigned _height;
  
  osg::ref_ptr<osg::Group> _parent;
  osg::ref_ptr<osg::Group> _subgraph;
  osg::ref_ptr<osg::Texture2D> _noiseMap;
  osg::ref_ptr<osg::Texture2D> _normalDepthMap0;
  osg::ref_ptr<osg::Texture2D> _normalDepthMap1;
  
  osg::ref_ptr<osg::Texture2D> _edgeMap;
  
  osg::ref_ptr<osg::Texture2D> _colorMap;
  
  osg::ref_ptr<osg::Geode> _quadGeode;

  osgText::Text* _hudText;
  double* _fps;
  
  std::vector<DePeePass*> _dePeePasses;
  
  osg::Uniform* _sketchy;
  osg::Uniform* _colored;
  osg::Uniform* _edgy;
  osg::Uniform* _sketchiness;
  
  bool _isSketchy;
  bool _isColored;
  bool _isEdgy;
  bool _isCrayon;

  osg::Camera* _colorCamera;

  //shader programs
  osg::ref_ptr<osg::Program> _normalDepthMapProgram;
  osg::ref_ptr<osg::Program> _colorMapProgram;
  osg::ref_ptr<osg::Program> _edgeMapProgram;

  bool _renderToFirst;
};

#endif
