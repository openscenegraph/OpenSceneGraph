/*
  Steffen Frey
  Fachpraktikum Graphik-Programmierung 2007
  Institut fuer Visualisierung und Interaktive Systeme
  Universitaet Stuttgart
 */

#ifndef _DEPEEPASS_H_
#define _DEPEEPASS_H_

#include <map>
#include <osg/Node>
#include <osg/Camera>
#include <osg/Group>

/*!
  MapMode specifies the kind of texture maps that can be generated for later
  usage
 */
enum MapMode {NORMAL_DEPTH_MAP, COLOR_MAP, EDGE_MAP, NOISE_MAP};

/*!
  DePeePass can be seen as a mera data structure and typically used by 
  the class DePee. It represents one depth peeling pass and is initialized
  by functions in the DePee class, but cleans itself up.
  Please note, that no texture generation mode is allowed to appear twice
*/
class DePeePass
{
 public:
  /*!
    Constructor
   */
  DePeePass();
  
  /*!
    Destructor cleans the whole depth peeling pass
   */
  ~DePeePass();
  
  /*!
    Make data structure ready for incorporating a new rendering pass
   */
  void newRenderPass(MapMode mapMode);
  
  /*!
    Clean up the specified rendering pass
   */
  void remRenderPass(MapMode mapMode);
  
  osg::ref_ptr<osg::Group> root;
  std::map<MapMode, osg::ref_ptr<osg::Camera> > Cameras;
  std::map<MapMode, osg::ref_ptr<osg::Group> > settingNodes;
};

#endif
