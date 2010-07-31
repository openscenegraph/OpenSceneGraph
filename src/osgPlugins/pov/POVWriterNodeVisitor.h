#ifndef POV_WRITER_NODE_VISITOR_H
#define POV_WRITER_NODE_VISITOR_H
//
//  POVWriterNodeVisitor converts OSG scene graph to POV (povray)
//  and writes it to the stream.
//
//
//  Autor: PCJohn (peciva _at fit.vutbr.cz)
//                developed for research purposes of Cadwork (c) and
//                Brno University of Technology (Czech Rep., EU)
//
//  License: public domain
//
//
//  THIS SOFTWARE IS NOT COPYRIGHTED
//
//  This source code is offered for use in the public domain.
//  You may use, modify or distribute it freely.
//
//  This source code is distributed in the hope that it will be useful
//  but WITHOUT ANY WARRANTY. ALL WARRANTIES, EXPRESS OR IMPLIED
//  ARE HEREBY DISCLAIMED. This includes but is not limited to
//  warranties of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
//  If you find the source code useful, authors will kindly welcome
//  if you give them credit and keep their names with their source
//  code, but you are not forced to do so.
//

#include <osg/NodeVisitor>
#include <stack>

namespace osg {
   class Light;
   class Transform;
}


class POVWriterNodeVisitor : public osg::NodeVisitor
{
public:

   POVWriterNodeVisitor( std::ostream& fout, const osg::BoundingSphere& bound );
   virtual ~POVWriterNodeVisitor();

   void traverse( osg::Node& node );

   virtual void apply( osg::Node &node );
   virtual void apply( osg::Geode &node );
  //virtual void apply(osg::Group &node);

  //virtual void apply(osg::Billboard& node);
  //virtual void apply(ProxyNode& node)                 { apply((Group&)node); }
  //virtual void apply(Projection& node)                { apply((Group&)node); }
  //virtual void apply(CoordinateSystemNode& node)      { apply((Group&)node); }

  //virtual void apply(ClipNode& node)                  { apply((Group&)node); }
  //virtual void apply(TexGenNode& node)                { apply((Group&)node); }
  //virtual void apply(LightSource& node)               { apply((Group&)node); }

  virtual void apply( osg::Transform& node );
  //virtual void apply(Camera& node)                    { apply((Transform&)node); }
  //virtual void apply(CameraView& node)                { apply((Transform&)node); }
  //virtual void apply(osg::MatrixTransform& node);
  //virtual void apply(osg::PositionAttitudeTransform& node);

  //virtual void apply(Switch& node)                    { apply((Group&)node); }
  //virtual void apply(Sequence& node)                  { apply((Group&)node); }
  //virtual void apply(osg::LOD& node);
  //virtual void apply(PagedLOD& node)                  { apply((LOD&)node); }
  //virtual void apply(ClearNode& node)                 { apply((Group&)node); }
  //virtual void apply(OccluderNode& node)              { apply((Group&)node); }

   unsigned int getNumProducedTriangles() const  { return numProducedTriangles; }
protected:

   std::ostream& _fout;
   osg::BoundingSphere bound;
   std::stack< osg::ref_ptr< osg::StateSet > > _stateSetStack;
   std::stack< osg::Matrix > _transformationStack;
   unsigned int numProducedTriangles;
   std::map< osg::Light*, int > lights;

   void pushStateSet( const osg::StateSet *ss );
   void popStateSet( const osg::StateSet *ss );

   virtual void processGeometry( const osg::Geometry *g,
                                 const osg::StateSet *ss, const osg::Matrix &m );
   virtual void processLights( const osg::StateSet *ss, const osg::Matrix &m );
};


#endif /* POV_WRITER_NODE_VISITOR_H */
