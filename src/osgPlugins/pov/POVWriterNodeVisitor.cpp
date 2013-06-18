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


#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/TexEnv>
#include <sstream>
#include <cassert>
#include "POVWriterNodeVisitor.h"

using namespace std;
using namespace osg;



void POVWriterNodeVisitor::apply( Node& node )
{
   POVWriterNodeVisitor::traverse( node );
}


void POVWriterNodeVisitor::traverse( Node& node )
{
   pushStateSet( node.getStateSet() );

   NodeVisitor::traverse( node );

   popStateSet( node.getStateSet() );
}


void POVWriterNodeVisitor::pushStateSet( const StateSet *ss )
{
   if( ss ) {

      // merge state set
      StateSet *mergedSS = new StateSet(
            *_stateSetStack.top().get(), CopyOp::SHALLOW_COPY );
      mergedSS->merge( *ss );

      // push state set
      _stateSetStack.push( mergedSS );

   }
}


void POVWriterNodeVisitor::popStateSet( const StateSet *ss )
{
   if( ss ) {

      // restore the previous stateset
      assert( _stateSetStack.size() > 0 && "_stateSetStack underflow" );
      _stateSetStack.pop();

   }
}


void POVWriterNodeVisitor::apply( Geode& node )
{
   pushStateSet( node.getStateSet() );

   // iterate through drawables
   const Geode::DrawableList& dl = node.getDrawableList();
   for( Geode::DrawableList::const_iterator itr = dl.begin();
        itr != dl.end(); ++itr)
   {
      // get drawable
      const Drawable *d = itr->get();

      // push state set
      const StateSet *ss = d->getStateSet();
      if( ss )  pushStateSet( ss );

      // transformation matrix
      Matrix m = _transformationStack.top();

      // process lights
      processLights( _stateSetStack.top().get(), m );

      // process geometry
      const Geometry *g = d->asGeometry();
      if( g )
         processGeometry( g, _stateSetStack.top().get(), m );

      // pop state set
      if( ss )  popStateSet( ss );
   }

   popStateSet( node.getStateSet() );
}




/**
 * ValueVisitor writes all values of an array out to a stream,
 * applies a matrix beforehand if necessary.
 */
// The code was adapted from the OBJWriterNodeVisitor.cpp.
// See the file for the list of authors and contributors.
class PovVec3WriterVisitor : public ConstValueVisitor
{
public:

   PovVec3WriterVisitor( ostream& fout, const Matrix& m =
         Matrix::identity(), bool isNormal = false)
   : ConstValueVisitor(),
     _fout( fout ),
     _m( m ),
     _isNormal( isNormal )
   {
      _applyMatrix = ( _m != Matrix::identity() );
      if ( _isNormal )
         _origin = Vec3( 0, 0, 0 ) * _m;
   }

   virtual void apply( const Vec2& v )
   {
      apply( Vec3( v[0], v[1], 0. ) );
   }

   virtual void apply( const Vec3& v )
   {
      Vec3 a;
      if (_applyMatrix)
         a = (_isNormal) ? (v * _m) - _origin : v * _m;
      else
         a = v;
      _fout << "      < " << a[0] << ", " << a[1] << ", " << a[2]
            << " >" << endl;
   }

   virtual void apply( const Vec2b& v )
   {
      apply( Vec3b( v[0], v[1], 0 ) );
   }

   virtual void apply( const Vec3b & v )
   {
      apply( Vec3s( v[0], v[1], v[2] ) );
   }

   virtual void apply( const Vec2s& v )
   {
      apply( Vec3s( v[0], v[1], 0 ) );
   }

   virtual void apply( const Vec3s& v )
   {
      apply( Vec3( v[0], v[1], v[2] ) );
   }

private:

   PovVec3WriterVisitor& operator = ( const PovVec3WriterVisitor& ) { return *this; }

   ostream& _fout;
   Matrix   _m;
   bool     _applyMatrix;
   bool     _isNormal;
   Vec3     _origin;
};


class PovVec2WriterVisitor : public ConstValueVisitor
{
public:

   PovVec2WriterVisitor( ostream& fout, const Matrix& m =
         Matrix::identity(), bool isNormal = false)
   : ConstValueVisitor(),
     _fout( fout ),
     _m( m ),
     _isNormal( isNormal )
   {
      _applyMatrix = ( _m != Matrix::identity() );
      if ( _isNormal )
         _origin = Vec3( 0, 0, 0 ) * _m;
   }

   virtual void apply( const Vec2& v )
   {
      Vec2 a;
      if (_applyMatrix) {
         Vec3 b( v[0], v[1], 0. );
         b = (_isNormal) ? (b * _m) - _origin : b * _m;
         a = Vec2( b[0], b[1] );
      } else
         a = v;
      _fout << "      < " << a[0] << ", " << a[1] << " >" << endl;
   }

   virtual void apply( const Vec3& v )
   {
      apply( Vec2( v[0], v[1] ) );
   }

   virtual void apply( const Vec2b& v )
   {
      apply( Vec2( v[0], v[1] ) );
   }

   virtual void apply( const Vec3b & v )
   {
      apply( Vec2b( v[0], v[1] ) );
   }

   virtual void apply( const Vec2s& v )
   {
      apply( Vec2( v[0], v[1] ) );
   }

   virtual void apply( const Vec3s& v )
   {
      apply( Vec2s( v[0], v[1] ) );
   }

private:

   PovVec2WriterVisitor& operator = ( const PovVec2WriterVisitor& ) { return *this; }

   ostream& _fout;
   Matrix   _m;
   bool     _applyMatrix;
   bool     _isNormal;
   Vec3     _origin;
};


class ArrayValueFunctor : public ConstArrayVisitor
{
public:

   ArrayValueFunctor( ConstValueVisitor &vv )
      : valueVisitor( &vv )  {}

   template< class ArrayType, class ValueType >
         void visitAll( const ArrayType& a ) {
            const ValueType *v = static_cast< const ValueType* >( a.getDataPointer() );
            unsigned int size = a.getNumElements();
            for( unsigned int i=0; i<size; i++ )
               valueVisitor->apply( v[i] );
         }
   virtual void apply( const ByteArray& a )   { visitAll< ByteArray, GLbyte >( a ); }
   virtual void apply( const ShortArray& a )  { visitAll< ShortArray, GLshort >( a ); }
   virtual void apply( const IntArray& a )    { visitAll< IntArray, GLint >( a ); }
   virtual void apply( const UByteArray& a )  { visitAll< UByteArray, GLubyte >( a ); }
   virtual void apply( const UShortArray& a ) { visitAll< UShortArray, GLushort >( a ); }
   virtual void apply( const UIntArray& a )   { visitAll< UIntArray, GLuint >( a ); }
   virtual void apply( const FloatArray& a )  { visitAll< FloatArray, GLfloat >( a ); }
   virtual void apply( const DoubleArray& a ) { visitAll< DoubleArray, GLdouble >( a ); }
   virtual void apply( const Vec2Array& a )   { visitAll< Vec2Array, Vec2 >( a ); }
   virtual void apply( const Vec3Array& a )   { visitAll< Vec3Array, Vec3 >( a ); }
   virtual void apply( const Vec4Array& a )   { visitAll< Vec4Array, Vec4 >( a ); }
   virtual void apply( const Vec4ubArray& a ) { visitAll< Vec4ubArray, Vec4ub >( a ); }
   virtual void apply( const Vec2bArray& a )  { visitAll< Vec2bArray, Vec2b >( a ); }
   virtual void apply( const Vec3bArray& a )  { visitAll< Vec3bArray, Vec3b >( a ); }
   virtual void apply( const Vec4bArray& a )  { visitAll< Vec4bArray, Vec4b >( a ); }
   virtual void apply( const Vec2sArray& a )  { visitAll< Vec2sArray, Vec2s >( a ); }
   virtual void apply( const Vec3sArray& a )  { visitAll< Vec3sArray, Vec3s >( a ); }
   virtual void apply( const Vec4sArray& a )  { visitAll< Vec4sArray, Vec4s >( a ); }
   virtual void apply( const Vec2dArray& a )  { visitAll< Vec2dArray, Vec2d >( a ); }
   virtual void apply( const Vec3dArray& a )  { visitAll< Vec3dArray, Vec3d >( a ); }
   virtual void apply( const Vec4dArray& a )  { visitAll< Vec4dArray, Vec4d >( a ); }

   virtual void apply( const Array& ) {
      assert( false && "Not using overloaded methods." );
   }

protected:
   ConstValueVisitor *valueVisitor;
};


template < class Type >
class PovArrayWriterFunctor : public ArrayValueFunctor
{
public:
   PovArrayWriterFunctor( ostream& fout, const Matrix& m =
         Matrix::identity(), bool isNormal = false)
      : ArrayValueFunctor( povValueVisitor ),
        povValueVisitor( fout, m, isNormal )  {}

protected:
   Type povValueVisitor;
};


static void writeIndex( ostream& _fout, unsigned int &numTriangles,
                        int i1, int i2, int i3, int &numOnLine )
{
   // produce comma, except the first record
   if( numTriangles != 0 )
      _fout << ",";

   // new line each four triangles
   if( numOnLine == 3 ) {
      _fout << endl;
      _fout << "   ";
      numOnLine = 0;
   }

   _fout << "   < " << i1 << ", " << i2 << ", " << i3 << " >";
   numOnLine++;
}


static void processDrawArrays( ostream& _fout, unsigned int &numTriangles,
                               GLenum mode, int startIndex, int stopIndex )
{
   int numOnLine = 0;

   int i = startIndex;
   switch( mode ) {

      case GL_TRIANGLES:
         for( i += 2; i < stopIndex; i += 3, numTriangles++ )
            writeIndex( _fout, numTriangles, i-2, i-1, i, numOnLine );
         break;

      case GL_TRIANGLE_STRIP:
         for( i += 2; i < stopIndex; i += 1, numTriangles++ )
            writeIndex( _fout, numTriangles, i-2, i-1, i, numOnLine );
         break;

      case GL_TRIANGLE_FAN:
         for( i += 2; i < stopIndex; i += 1, numTriangles++ )
            writeIndex( _fout, numTriangles, startIndex, i-1, i, numOnLine );
         break;

      case GL_QUADS:
      case GL_QUAD_STRIP:
      case GL_POLYGON:
         assert( 0 && "Not implemented yet." );

      default:
         assert( false );
   }

   // put new line when primitive set is over
   _fout << endl;
}


class DrawElementsWriter : public  ConstValueVisitor {
public:

   DrawElementsWriter( ostream& fout ) : _fout(fout),
                        numIndices( 0 ), numOnLine( 0 ),
                        numTriangles( 0 )  {}

   virtual void apply( const GLubyte& b )  { processIndex( b ); }
   virtual void apply( const GLushort& s ) { processIndex( s ); }
   virtual void apply( const GLuint& i )   { processIndex( i ); }

   virtual void processIndex( const GLuint i ) = 0;

   inline unsigned int getNumTriangles()  { return numTriangles; }

protected:
   ostream& _fout;
   GLuint indices[3];
   int numIndices;
   int numOnLine;
   unsigned int numTriangles;

   virtual bool processTriangle()
   {
      // dont produce trinagle until we have got three vertices
      if( numIndices < 3 )
         return false;

      // produce comma, except the first record
      if( numTriangles != 0 )
         _fout << ",";

      // new line each four triangles
      if( numOnLine == 3 ) {
         _fout << endl;
         _fout << "   ";
         numOnLine = 0;
      }

      // print out indices
      _fout << "   <" << indices[0] << "," << indices[1]
            << "," << indices[2] << ">";

      // increment counters
      numTriangles++;
      numOnLine++;

      // triangle produced => return true
      return true;
   }
};


class TriangleWriter : public DrawElementsWriter {
public:
   TriangleWriter( ostream& fout ) : DrawElementsWriter( fout ) {}

   virtual void processIndex( const GLuint i )
   {
      indices[numIndices++] = i;

      if( processTriangle() )
         numIndices = 0;
   }
};


class TriangleStripWriter : public DrawElementsWriter {
public:
   TriangleStripWriter( ostream& fout ) : DrawElementsWriter( fout ) {}

   virtual void processIndex( const GLuint i )
   {
      indices[0] = indices[1];
      indices[1] = indices[2];
      indices[2] = i;
      numIndices++;

      processTriangle();
   }
};


class TriangleFanWriter : public DrawElementsWriter {
public:
   TriangleFanWriter( ostream& fout ) : DrawElementsWriter( fout ) {}

   virtual void processIndex( const GLuint i )
   {
      if( numIndices == 0 ) {
         indices[0] = i;
         numIndices++;
         return;
      }

      indices[1] = indices[2];
      indices[2] = i;
      numIndices++;

      processTriangle();
   }
};


template < class Type, class IterType >
static void processDrawElements( const PrimitiveSet *pset, DrawElementsWriter *w )
{
   const Type *drawElements =
         dynamic_cast< const Type* >( pset );
   for( IterType primItr = drawElements->begin();
         primItr != drawElements->end();
         ++primItr )
      w->apply( *primItr );
}


class Vec4ConvertVisitor : public ConstValueVisitor
{
public:
   //virtual void apply( const GLbyte& v )
   //virtual void apply( const GLshort& v )
   //virtual void apply( const GLint& v )
   //virtual void apply( const GLushort& v )
   //virtual void apply( const GLubyte& v )
   //virtual void apply( const GLuint& v )
   //virtual void apply( const GLfloat& v )
   //virtual void apply( const GLdouble& v )
   virtual void apply( const Vec4ub& v ) { r = Vec4( v[0], v[1], v[2], v[3] ); }
   //virtual void apply( const Vec2& v )
   virtual void apply( const Vec3& v )   { r = Vec4( v[0], v[1], v[2], 1. ); }
   virtual void apply( const Vec4& v )   { r = v; }
   //virtual void apply( const Vec2b& v )
   virtual void apply( const Vec3b& v )  { r = Vec4( v[0], v[1], v[2], 1. ); }
   virtual void apply( const Vec4b& v )  { r = Vec4( v[0], v[1], v[2], v[3] ); }
   //virtual void apply( const Vec2s& v )
   virtual void apply( const Vec3s& v )  { r = Vec4( v[0], v[1], v[2], 1. ); }
   virtual void apply( const Vec4s& v )  { r = Vec4( v[0], v[1], v[2], v[3] ); }
   //virtual void apply( const Vec2d& v )
   virtual void apply( const Vec3d& v )  { r = Vec4( v[0], v[1], v[2], 1. ); }
   virtual void apply( const Vec4d& v )  { r = Vec4( v[0], v[1], v[2], v[3] ); }

   const Vec4& getResult() const  { return r; }

protected:
   Vec4 r;
};


void POVWriterNodeVisitor::processGeometry( const Geometry *g,
                                            const StateSet *ss,
                                            const Matrix &m )
{
   // ignore empty geometries because they cause povray to fail loading the model
   // (seen on POV-Ray 3.6.1)
   if( g->getVertexArray() == NULL || g->getVertexArray()->getNumElements() == 0 )
      return;

   // mesh2
   _fout << "mesh2" << endl;
   _fout << "{" << endl;

   // Convert coordinates
   // OSG represents coordinates by: Vec2, Vec3, Vec4
   if( g->getVertexArray() ) {

      _fout << "   vertex_vectors" << endl;
      _fout << "   {" << endl;
      _fout << "      " << g->getVertexArray()->getNumElements() << "," << endl;

      PovArrayWriterFunctor< PovVec3WriterVisitor > povArrayWriter( _fout, m, false );
      g->getVertexArray()->accept( povArrayWriter );

      _fout << "   }" << endl;
   }

   // reuse coord indices for normal and texCoord
   bool needNormalIndices = true;
   bool needTexCoordIndices = true;

   // Convert normals
   // OSG represents normals by: Vec3,Vec3s,Vec3b
   // and can handle: Vec4s,Vec4b by truncating them to three components
   if( g->getNormalArray() ) {

      _fout << "   normal_vectors" << endl;
      _fout << "   {" << endl;
      _fout << "      " << g->getNormalArray()->getNumElements() << "," << endl;

      PovArrayWriterFunctor< PovVec3WriterVisitor > povArrayWriter( _fout, m, true );
      g->getNormalArray()->accept( povArrayWriter );

      _fout << "   }" << endl;

      // need normal indices?
      if( g->getVertexArray() )
         if( g->getVertexArray()->getNumElements() == g->getNormalArray()->getNumElements() )
            needNormalIndices = false;
   }

   // Convert texture coordinates
   if( g->getTexCoordArray( 0 ) ) {

      _fout << "   uv_vectors" << endl;
      _fout << "   {" << endl;
      _fout << "      " << g->getTexCoordArray( 0 )->getNumElements() << "," << endl;

      // texture coordinates
      PovArrayWriterFunctor< PovVec2WriterVisitor > povArrayWriter( _fout );
      g->getTexCoordArray( 0 )->accept( povArrayWriter );

      _fout << "   }" << endl;

      // need texCoord indices?
      if( g->getVertexArray() )
         if( g->getVertexArray()->getNumElements() == g->getTexCoordArray( 0 )->getNumElements() )
            needTexCoordIndices = false;
   }

   // indices string stream
   stringstream indicesStream;
   indicesStream << "   ";
   unsigned int numTriangles = 0;

   // process primitive sets
   int psetIndex, numPsets = g->getNumPrimitiveSets();
   for( psetIndex = 0; psetIndex < numPsets; psetIndex++ ) {

      // get primitive set
      const PrimitiveSet *pset = g->getPrimitiveSet( psetIndex );
      PrimitiveSet::Type type = pset->getType();
      GLenum mode = pset->getMode();

      // skip not supported primitives
      switch( mode ) {
         case GL_TRIANGLES:
         case GL_TRIANGLE_STRIP:
         case GL_TRIANGLE_FAN:
         case GL_QUADS:
         case GL_QUAD_STRIP:
         case GL_POLYGON:
            break; // only supported types
         default:
            continue; // skip primitive set
                      // Povray does not support lines and points
      };

      switch( type ) {
         case PrimitiveSet::DrawArraysPrimitiveType:
         {
            const DrawArrays *drawArrays = dynamic_cast< const DrawArrays* >( pset );

            int startIndex = drawArrays->getFirst();
            int stopIndex = startIndex + drawArrays->getCount();

            // FIXME: Am I using startIndex for all bundles that are PER_VERTEX?

            processDrawArrays( indicesStream, numTriangles,
                               mode, startIndex, stopIndex );

            break;
         }

         case PrimitiveSet::DrawArrayLengthsPrimitiveType:
         {
            const DrawArrayLengths *drawArrayLengths =
                  dynamic_cast< const DrawArrayLengths* >( pset );

            int startIndex = drawArrayLengths->getFirst();
            DrawArrayLengths::vector_type::const_iterator itr = drawArrayLengths->begin();

            for( ; itr != drawArrayLengths->end(); itr++ ) {
               processDrawArrays( indicesStream, numTriangles,
                                  mode, startIndex, *itr );
               startIndex += *itr;
            }

            break;
         }

         case PrimitiveSet::DrawElementsUBytePrimitiveType:
         case PrimitiveSet::DrawElementsUShortPrimitiveType:
         case PrimitiveSet::DrawElementsUIntPrimitiveType:
         {
            DrawElementsWriter *w;
            switch( mode ) {
               case GL_TRIANGLES:
                  w = new TriangleWriter( indicesStream );
                  break;
               case GL_TRIANGLE_STRIP:
                  w = new TriangleStripWriter( indicesStream );
                  break;
               case GL_TRIANGLE_FAN:
                  w = new TriangleFanWriter( indicesStream );
                  break;
               default:
                  assert( false && "Not implemented yet." );
                  goto skip;
            }

            switch( type ) {

               case PrimitiveSet::DrawElementsUBytePrimitiveType:
                  processDrawElements< DrawElementsUByte,
                        DrawElementsUByte::const_iterator >( pset, w );
                  break;

               case PrimitiveSet::DrawElementsUShortPrimitiveType:
                  processDrawElements< DrawElementsUShort,
                        DrawElementsUShort::const_iterator >( pset, w );
                  break;

               case PrimitiveSet::DrawElementsUIntPrimitiveType:
                  processDrawElements< DrawElementsUInt,
                        DrawElementsUInt::const_iterator >( pset, w );
                  break;

               default:
                  assert( false );
            }

            numTriangles += w->getNumTriangles();

            delete w;
         }
         skip:
         break;

         default:
            assert( false && "Primitive set not handled." );
      }
   }

   const Texture *texture = dynamic_cast< const Texture* >(
         ss->getTextureAttribute( 0, StateAttribute::TEXTURE ));
   const Image *image = ( texture ? texture->getImage( 0 ) : NULL );
   // TexEnv not used yet
   //const TexEnv *texEnv =  dynamic_cast< const TexEnv* >(
   //      ss->getTextureAttribute( 0, StateAttribute::TEXENV ));
   bool texturing2D = ss->getTextureMode( 0, GL_TEXTURE_2D ) != 0;

   _fout << "   face_indices" << endl
         << "   {" << endl
         << "      " << numTriangles << "," << endl
         << indicesStream.str()
         << "   }" << endl;

   if( needNormalIndices )
      _fout << "   normal_indices" << endl
            << "   {" << endl
            << "      " << numTriangles << "," << endl
            << indicesStream.str()
            << "   }" << endl;

   if( needTexCoordIndices )
      _fout << "   uv_indices" << endl
            << "   {" << endl
            << "      " << numTriangles << "," << endl
            << indicesStream.str()
            << "   }" << endl;

   // POV-Ray's surface properties
   _fout << "   texture {" << endl;

#if 1

   // color variables
   Vec4f ambient( 0.2f, 0.2f, 0.2f, 1.f );
   Vec4f diffuse( 0.8f, 0.8f, 0.8f, 1.f );
   Vec4f specular( 0.f, 0.f, 0.f, 1.f );
   Vec4f pigmentColor;

   // get ambient and diffuse
   const Material *material = dynamic_cast< const Material* >(
         ss->getAttribute( StateAttribute::MATERIAL ) );
   if( !material ) {
      const Array *a = g->getColorArray();
      if( a && a->getNumElements() > 0 ) {
         Vec4ConvertVisitor cv;
         g->getColorArray()->accept( 0, cv );
         diffuse = cv.getResult();
      }
   } else {
      diffuse = material->getDiffuse( Material::FRONT );
      ambient = material->getAmbient( Material::FRONT );
      specular = material->getSpecular( Material::FRONT );
   }

   // compute intensities
   float diffuseIntensity  = diffuse.r() + diffuse.g() + diffuse.b();
   float ambientIntensity  = ambient.r() + ambient.g() + ambient.b();
   float specularIntensity = specular.r() + specular.g() + specular.b();

   // pigment color defaults to diffuse
   pigmentColor = diffuse;
   float pigmentIntensity = diffuseIntensity;

   // if diffuse is too dark and ambient is strong, use it instead
   if( diffuseIntensity < 2 * ambientIntensity ) {
      pigmentColor = ambient;
      pigmentIntensity = ambientIntensity;
   }

#else
   Vec4ConvertVisitor cv;
   g->getColorArray()->accept( 0, cv );
   Vec4 pigmentColor = cv.getResult();
#endif

   // has 2D texture => produce image_map
   if( texturing2D && texture && image ) {
      string fileName = image->getFileName();

      // replace '\' by '/'
      // ('\' does not work on Linux POV-Ray)
      // FIXME: is '/' working on Windows?
      string::size_type i=0;
      while( ( i = fileName.find( '\\', i ) ) != string::npos )
         fileName[i] = '/';

      _fout << "      uv_mapping pigment {" << endl
            << "         image_map { png \"" << fileName << "\" }" << endl
            << "      }" << endl;

   // no 2D texture => produce single color
   } else {

      // rgb and f as filter (for transparency, 0 - opaque, 1 - transparent)
      _fout << "      pigment {" << endl
            << "         color rgbf < " << pigmentColor[0] << ", "
                                        << pigmentColor[1] << ", "
                                        << pigmentColor[2] << ", 0 >" << endl
            << "      }" << endl;
   }

   // POV-Ray's finish
   float ai = (pigmentIntensity != 0) ? ambientIntensity/pigmentIntensity : 0.f;
   float di = (pigmentIntensity != 0) ? diffuseIntensity/pigmentIntensity : 0.f;
   _fout << "      finish { ambient " << ai << endl
         << "               diffuse " << di << endl
         << "               specular " << specularIntensity/3. << endl
         << "               reflection " << specularIntensity/3. << endl
         << "             }" << endl
         << "   }" << endl;

   _fout << "}" << endl;

   numProducedTriangles += numTriangles;
}


void POVWriterNodeVisitor::processLights( const StateSet *ss, const Matrix &m )
{
   const StateSet::AttributeList &attributeList = ss->getAttributeList();
   StateSet::AttributeList::const_iterator it = attributeList.begin();
   for( ; it!=attributeList.end(); ++it )
      if( it->first.first == StateAttribute::LIGHT ) {
         Light *l = dynamic_cast< Light* >( it->second.first.get() );
         if( l &&
             ss->getMode( GL_LIGHT0 + l->getLightNum() ) & StateAttribute::ON &&
             lights.find( l ) == lights.end() ) {

            // append the light into the map
            lights[ l ] = 1; // use any number (the number is not used)

            // directional light requires special treatment
            Vec4 pos4 = l->getPosition();
            Vec3 pos3( pos4.x(), pos4.y(), pos4.z() );
            bool directional = (pos4.w() == 0.);
            bool spot = false;
            if( directional ) {
               pos3.normalize();
               pos3 = bound.center() + ( pos3 * bound.radius() * 1.01f );
            } else {
               pos3 /= pos4.w();
               spot = !equivalent( l->getSpotCutoff(), 180.f );
            }

            // create the light code
            _fout << "light_source {" << endl;

            // position
            PovVec3WriterVisitor povVec3Writer( _fout, m );
            povVec3Writer.apply( pos3 );

            // color
            // note: transmit and filter color values are ignored
            // by POV-Ray for light sources
            // (see POV-Ray's (version 3.6.1, chapter 2.4.7) doc:
            // http://www.povray.org/documentation/view/3.6.1/308/)
            _fout << "   color rgb";
            PovVec3WriterVisitor povColorWriter( _fout );
            const Vec3::value_type *f = l->getDiffuse().ptr();
            povColorWriter.apply( Vec3( f[0], f[1], f[2] ) );

            // directional light
            if( directional ) {
               _fout << "   parallel" << endl
                     << "   point_at";
               povVec3Writer.apply( bound.center() );
            }

            // spot light
            if( spot ) {
               _fout << "   spotlight" << endl
                     << "   point_at";
               povVec3Writer.apply( pos3 + l->getDirection() );
               // FIXME: radius and tightness models the light distribution
               // differently than OpenGL's shininess. So, different
               // visual results are produced. The difference can be lowered
               // much by computing radius.
               _fout << "   falloff " << l->getSpotCutoff() << endl
                     << "   radius 0" << endl
                     << "   tightness " << l->getSpotExponent() / 128.f * 100.f << endl;
            }

            // light source end
            _fout << "}" << endl;
         }
      }
}


void POVWriterNodeVisitor::apply( Transform& node )
{
   // push new transformation on transformation stack
   Matrix m = _transformationStack.top();
   node.computeLocalToWorldMatrix( m, this );
   _transformationStack.push( m );

   // traverse the node
   apply( (Group&)node );

   // pop transformation
   _transformationStack.pop();
}


POVWriterNodeVisitor::POVWriterNodeVisitor( ostream& fout, const BoundingSphere& b )
   : NodeVisitor(TRAVERSE_ALL_CHILDREN),
     _fout( fout ),
     bound( b ),
     numProducedTriangles( 0 )
{
   _stateSetStack.push( new StateSet() );
   _transformationStack.push( Matrix( 1., 0.,0.,0.,
                                      0., 0.,1.,0.,
                                      0., 1.,0.,0.,
                                      0., 0.,0.,1. ) );
}


POVWriterNodeVisitor::~POVWriterNodeVisitor()
{
   assert( _stateSetStack.size() >= 1 && "_stateSetStack underflow." );
   assert( _stateSetStack.size() <= 1 && "_stateSetStack overflow." );
   assert( _transformationStack.size() >= 1 && "_transformationStack underflow." );
   assert( _transformationStack.size() <= 1 && "_transformationStack overflow." );
   _stateSetStack.pop();
   _transformationStack.pop();
}
