#ifndef OSG_CONVERT_TO_INVENTOR_H
#define OSG_CONVERT_TO_INVENTOR_H
//
//  ConvertToInventor converts OSG scene graph to Inventor or VRML 1 scene graph
//
//  It requires OSG and Inventor compatible library, such as Coin,
//  SGI Inventor , or TGS Inventor.
//
//
//  Autor: PCJohn (peciva _at fit.vutbr.cz)
//
//  License: public domain
//
//
//  THIS SOFTWARE IS NOT COPYRIGHTED
//
//  This source code is offered for use in the public domain.
//  You may use, modify or distribute it freely.
//
//  This source code is distributed in the hope that it will be useful but
//  WITHOUT ANY WARRANTY.  ALL WARRANTIES, EXPRESS OR IMPLIED ARE HEREBY
//  DISCLAIMED.  This includes but is not limited to warranties of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//  If you find the source code useful, authors will kindly welcome
//  if you give them credit and keep their names with their source code,
//  but you are not forced to do so.
//

#include <stack>
#include <osg/CullFace>
#include <osg/FrontFace>

class SoSeparator;

namespace osg {
  class BlendFunc;
  class CullFace;
  class FrontFace;
  class Material;
  class ShapeDrawable;
  class TexEnv;
  class TexGen;
}



class ConvertToInventor : public osg::NodeVisitor
{
public:
  ConvertToInventor();
  virtual ~ConvertToInventor();

  SoNode* getIvSceneGraph() const;
  void setVRML1Conversion(bool useVRML1)  { vrml1Conversion = useVRML1; };

  virtual void apply(osg::Node &node);
  virtual void apply(osg::Geode &node);
  virtual void apply(osg::Group &node);

  virtual void apply(osg::Billboard& node);
  //virtual void apply(ProxyNode& node)                 { apply((Group&)node); }
  //virtual void apply(Projection& node)                { apply((Group&)node); }
  //virtual void apply(CoordinateSystemNode& node)      { apply((Group&)node); }

  //virtual void apply(ClipNode& node)                  { apply((Group&)node); }
  //virtual void apply(TexGenNode& node)                { apply((Group&)node); }
  //virtual void apply(LightSource& node)               { apply((Group&)node); }

  //virtual void apply(Transform& node);
  //virtual void apply(Camera& node)                    { apply((Transform&)node); }
  //virtual void apply(CameraView& node)                { apply((Transform&)node); }
  virtual void apply(osg::MatrixTransform& node);
  virtual void apply(osg::PositionAttitudeTransform& node);

  //virtual void apply(Switch& node)                    { apply((Group&)node); }
  //virtual void apply(Sequence& node)                  { apply((Group&)node); }
  virtual void apply(osg::LOD& node);
  //virtual void apply(PagedLOD& node)                  { apply((LOD&)node); }
  //virtual void apply(ClearNode& node)                 { apply((Group&)node); }
  //virtual void apply(OccluderNode& node)              { apply((Group&)node); }

protected:
  bool vrml1Conversion;
  bool useIvExtensions;
  SoSeparator *ivRoot;

  struct InventorState {
    class SoGroup      *ivHead;
    class SoTexture2   *ivTexture;
    class SoMaterial   *ivMaterial;
    const osg::Material *osgMaterial;
    bool osgTexture2Enabled;
    const osg::Texture  *osgTexture;
    const osg::TexEnv   *osgTexEnv;
    bool osgTexGenS, osgTexGenT;
    const osg::TexGen   *osgTexGen;
    bool osgLighting;
    bool osgTwoSided;
    osg::FrontFace::Mode osgFrontFace;
    bool osgCullFaceEnabled;
    osg::CullFace::Mode osgCullFace;
    bool osgBlendEnabled;
    const osg::BlendFunc *osgBlendFunc;

    InventorState()  {}
    InventorState(SoGroup *root) : ivHead(root), ivTexture(NULL),
        ivMaterial(NULL), osgMaterial(NULL),
        osgTexture2Enabled(false), osgTexture(NULL), osgTexEnv(NULL),
        osgTexGenS(false), osgTexGenT(false), osgTexGen(NULL),
        osgLighting(true), osgTwoSided(false), osgFrontFace(osg::FrontFace::COUNTER_CLOCKWISE),
        osgCullFaceEnabled(false), osgCullFace(osg::CullFace::BACK),
        osgBlendEnabled(false), osgBlendFunc(NULL)  {}
    InventorState(const InventorState &s) : ivHead(s.ivHead), ivTexture(s.ivTexture),
        ivMaterial(s.ivMaterial), osgMaterial(s.osgMaterial),
        osgTexture2Enabled(s.osgTexture2Enabled), osgTexture(s.osgTexture), osgTexEnv(s.osgTexEnv),
        osgTexGenS(s.osgTexGenS), osgTexGenT(s.osgTexGenT), osgTexGen(s.osgTexGen),
        osgLighting(s.osgLighting), osgTwoSided(s.osgTwoSided), osgFrontFace(s.osgFrontFace),
        osgCullFaceEnabled(s.osgCullFaceEnabled), osgCullFace(s.osgCullFace),
        osgBlendEnabled(s.osgBlendEnabled), osgBlendFunc(s.osgBlendFunc)  {}
    static InventorState createTopLevelState(SoSeparator *ivRoot) { return InventorState(ivRoot); }
  };

  std::stack<InventorState> ivStack;

  typedef std::map<const class osg::TexEnv*, class SoTexture2*> Env2ivTexMap;
  std::map<const osg::Texture*, Env2ivTexMap> ivTexturesMap;
  int uniqueIdGenerator;

  void processDrawable(osg::Drawable *d);
  void processGeometry(const deprecated_osg::Geometry *g, InventorState *ivState);
  void processShapeDrawable(const osg::ShapeDrawable *d, InventorState *ivState);

  virtual InventorState* createInventorState(const osg::StateSet *ss);
  virtual void popInventorState();
};


#endif /* OSG_CONVERT_TO_INVENTOR_H */
