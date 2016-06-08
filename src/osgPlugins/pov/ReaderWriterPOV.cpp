#include <osg/ComputeBoundsVisitor>
#include <osg/io_utils>
#include <osg/Notify>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include "ReaderWriterPOV.h"
#include "POVWriterNodeVisitor.h"

using namespace std;
using namespace osg;


// Register with Registry to instantiate the Povray writer.
REGISTER_OSGPLUGIN( Povray, ReaderWriterPOV )


/**
 * Constructor.
 * Initializes the ReaderWriterPOV.
 */
ReaderWriterPOV::ReaderWriterPOV()
{
    // Set supported extensions and options
    supportsExtension( "pov", "POV-Ray format" );
}


static osgDB::ReaderWriter::WriteResult
writeNodeImplementation( const Node& node, ostream& fout,
                         const osgDB::ReaderWriter::Options* /*options*/ )
{
   // get camera on the top of scene graph
   const Camera *camera = node.asCamera();

   Vec3d eye, center, up, right;
   double fovy, aspectRatio, tmp;
   if( camera ) {

      // view matrix
      camera->getViewMatrixAsLookAt( eye, center, up );
      up = Vec3d( 0.,0.,1. );
      right = Vec3d( 1.,0.,0. );
      //up.normalize();
      //right = (center-eye) ^ up;
      //right.normalize();

      // projection matrix
      camera->getProjectionMatrixAsPerspective( fovy, aspectRatio, tmp, tmp );
      right *= aspectRatio;

   } else {

      // get POV-Ray camera setup from scene bounding sphere
      // constructed from bounding box
      // (bounding box computes model center more precisely than bounding sphere)
      ComputeBoundsVisitor cbVisitor;
      const_cast< Node& >( node ).accept( cbVisitor );
      BoundingBox &bb = cbVisitor.getBoundingBox();
      BoundingSphere bs( bb );

      // set
      eye = bs.center() + Vec3( 0., -3.0 * bs.radius(), 0. );
      center = bs.center();
      up = Vec3d( 0.,1.,0. );
      right = Vec3d( 4./3.,0.,0. );

   }

   // camera
   fout << "camera { // following POV-Ray, x is right, y is up, and z is to the scene" << endl
        << "   location <" << eye[0] << ", " << eye[2] << ", " << eye[1] << ">" << endl
        << "   up <" << up[0] << ", " << up[2] << ", " << up[1] << ">" << endl
        << "   right <" << right[0] << ", " << right[2] << ", " << right[1] << ">" << endl
        << "   look_at <" << center[0] << ", " << center[2] << ", " << center[1] << ">" << endl
        << "}" << endl
        << endl;

   POVWriterNodeVisitor povWriter( fout, node.getBound() );
   if( camera )

      for( int i=0, c=camera->getNumChildren(); i<c; i++ )
         const_cast< Camera* >( camera )->getChild( i )->accept( povWriter );

   else

      const_cast< Node* >( &node )->accept( povWriter );

   notify( NOTICE ) << "ReaderWriterPOV::writeNode() Done. ("
                    << povWriter.getNumProducedTriangles()
                    << " triangles written)" << endl;

   return osgDB::ReaderWriter::WriteResult::FILE_SAVED;
}


osgDB::ReaderWriter::WriteResult
ReaderWriterPOV::writeNode( const Node& node, const string& fileName,
                            const osgDB::ReaderWriter::Options* options ) const
{
   // accept extension
   string ext = osgDB::getLowerCaseFileExtension( fileName );
   if( !acceptsExtension( ext ) )  return WriteResult::FILE_NOT_HANDLED;

   notify( NOTICE ) << "ReaderWriterPOV::writeNode() Writing file " << fileName << endl;

   osgDB::ofstream fout( fileName.c_str(), ios::out | ios::trunc );
   if( !fout )
      return WriteResult::ERROR_IN_WRITING_FILE;
   else
      return writeNodeImplementation( node, fout, options );
}


osgDB::ReaderWriter::WriteResult
ReaderWriterPOV::writeNode( const Node& node, ostream& fout,
                            const osgDB::ReaderWriter::Options* options ) const
{
   notify( osg::NOTICE ) << "ReaderWriterPOV::writeNode() Writing to "
                         << "stream" << endl;

   return writeNodeImplementation( node, fout, options );
}
