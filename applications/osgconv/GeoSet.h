/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2003 Robert Osfield 
 *
 * This library is open source and may be redistributed and/or modified under  
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or 
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the 
 * OpenSceneGraph Public License for more details.
*/


#ifndef OSG_GEOSET
#define OSG_GEOSET 1

#include <osg/Drawable>
#include <osg/Vec2>
#include <osg/Vec3>
#include <osg/Vec4>

namespace osg {


// forward declare so that we don't need to include the header.
class Geometry;

/** Note, osg::GeoSet is now deprecated, please use osg::Geometry instead.
    osg::GeoSet will be kept through to the beta release for 
    backwards compatability only.

    Encapsulates OpenGL drawing primitives, geometry and
    optional binding of normal, color and texture coordinates.  Used
    for representing the visible objects in the scene.  State attributes
    for a GeoSet are maintained in StateSet which the GeoSet maintains
    a referenced counted pointer to.  Both GeoSet's and StateSet's can 
    be shared for optimal memory usage and graphics performance.
*/
class GeoSet : public Drawable
{
    public:


        
        enum PrimitiveType {
            NO_TYPE,
            POINTS,
            LINES,
            LINE_STRIP,
            FLAT_LINE_STRIP,
            LINE_LOOP,
            TRIANGLES,
            TRIANGLE_STRIP,
            FLAT_TRIANGLE_STRIP,
            TRIANGLE_FAN,
            FLAT_TRIANGLE_FAN,
            QUADS,
            QUAD_STRIP,
            POLYGON
        };

        enum BindingType {
            BIND_OFF,
            BIND_OVERALL,
            BIND_PERPRIM,
            BIND_PERVERTEX,
            BIND_DEFAULT
        };

	enum InterleaveArrayType {
	    IA_OFF,
	    IA_V2F, 
	    IA_V3F,
	    IA_C4UB_V2F,
	    IA_C4UB_V3F,
	    IA_C3F_V3F,
	    IA_N3F_V3F,
	    IA_C4F_N3F_V3F,
	    IA_T2F_V3F,
	    IA_T4F_V4F,
	    IA_T2F_C4UB_V3F,
	    IA_T2F_C3F_V3F,
	    IA_T2F_N3F_V3F,
	    IA_T2F_C4F_N3F_V3F,
	    IA_T4F_C4F_N3F_V4F
	};


        struct IndexPointer
        {
        
            mutable unsigned int _size;
            bool _is_ushort;
            union _TPtr
            {
                GLushort* _ushort;
                GLuint*   _uint;
            } _ptr;
            
            IndexPointer() { _size=0;_is_ushort=true;_ptr._ushort = (GLushort*)0; }
            
            inline bool operator == (const IndexPointer& ip) const
            {
                return _size == ip._size &&
                       _is_ushort == ip._is_ushort &&
                       _ptr._ushort == ip._ptr._ushort;
            }
            
            inline bool valid() const
            {
                return _ptr._ushort != (GLushort*)0;
            }

            inline bool null() const
            {
                return _ptr._ushort == (GLushort*)0;
            }

            inline void setToNull()
            {
                _size = 0;
                _is_ushort = true;
                _ptr._ushort = (GLushort*)0;
            }

            inline void set(unsigned int size,GLushort* data)
            {
                _size = size;
                _is_ushort = true;
                _ptr._ushort = data;
            }
            

            void set(unsigned int size,GLuint* data)
            {
                _size = size;
                _is_ushort = false;
                _ptr._uint = data;
            }

            inline unsigned int maxIndex() const
            {
                unsigned int max = 0;            
                if (_is_ushort)
                {
                    for(unsigned int ai = 0; ai < _size; ai++ )
                        if( _ptr._ushort[ai] > max ) max = _ptr._ushort[ai];
                }
                else
                {
                    for(unsigned int ai = 0; ai < _size; ai++ )
                        if( _ptr._uint[ai] > max ) max = _ptr._uint[ai];
                }
                return max;
            }
            
            inline GLint operator [] (const GLuint pos) const
            {
                if (_is_ushort) return _ptr._ushort[pos];
                else return _ptr._uint[pos];
            }

        };

        GeoSet();

        /** Copy constructor using CopyOp to manage deep vs shallow copy.*/
        GeoSet(const GeoSet& geoset,const CopyOp& copyop=CopyOp::SHALLOW_COPY);
            
        virtual Object* cloneType() const { return new GeoSet(); }

        virtual Object* clone(const CopyOp& copyop) const { return new GeoSet(*this,copyop); }
        
        virtual bool isSameKindAs(const Object* obj) const { return dynamic_cast<const GeoSet*>(obj)!=NULL; }
        
        virtual const char* libraryName() const { return "osg"; }
        virtual const char* className() const { return "GeoSet"; }


        // data access methods.
        inline void setNumPrims( int n )                   { _numprims = n; _numcoords=0;}
        inline int getNumPrims() const                    { return _numprims; }

        void setPrimType( PrimitiveType type );
        inline PrimitiveType getPrimType() const           { return _primtype; }

        inline void setPrimLengths( int *lens )            { _primLengths = lens; }
        inline int  *getPrimLengths()                      { return _primLengths; }
        inline int *getPrimLengths() const           { return _primLengths; }

        void computeNumVerts() const;

        /** get the number of coords required by the defined primitives. */
        inline int getNumCoords()  const
	{ if( _numcoords == 0 ) computeNumVerts(); return _numcoords; }
        /** get a pointer to Vec3 coord array. */
        inline Vec3* getCoords()                           { return _coords; }
        /** get a const pointer to Vec3 coord array. */
        inline const Vec3* getCoords() const               { return _coords; }
        /** get the number of indices required by the defined primitives. */
        inline int getNumCoordIndices() const                     { return _cindex._size; }
        /** get the coord index array. */
        inline IndexPointer& getCoordIndices()                    { return _cindex; }
        /** get the const coord index array. */
        inline const IndexPointer& getCoordIndices() const        { return _cindex; }
        /** set the coords (i.e the geometry) of the geoset.*/
        void setCoords( Vec3 *cp );
        /** set the coords (i.e the geometry) and ushort indices of the geoset.
            To reduce memory footprint and bandwidth for small datasets it is
            recommended the ushort indices are used instead of unit indices.*/
        void setCoords( Vec3 *cp, GLushort *ci );
        /** set the coords (i.e the geometry) and unsigned int indices of the geoset.
            Unless your data set exceeds 65536 indices prefer ushort indices
            over unsigned int indices, only use this unit indices version if necessary.*/
        void setCoords( Vec3 *cp, GLuint *ci );
        /** set the coords (i.e the geometry) and indices of the geoset.*/
        void setCoords( Vec3 *cp, IndexPointer& ip );

        /** get the number of normals required by the defined primitives and normals binding.*/
        inline int getNumNormals() const                    { return _numnormals; }
        /** get a pointer to Vec3 normal array. */
        inline Vec3* getNormals()                           { return _normals; }
        /** get a const pointer to Vec3 normal array. */
        inline const Vec3* getNormals() const               { return _normals; }
        /** get the number of normal indices required by the defined primitives and normals binding.*/
        inline int getNumNormalIndices() const              { return _nindex._size; }
        /** get the normal index array. */
        inline IndexPointer& getNormalIndices()             { return _nindex; }
        /** get the const normal index array. */
        inline const IndexPointer& getNormalIndices() const { return _nindex; }
        /** set the normals of the geoset.*/
        void setNormals( Vec3 *np );
        /** set the normals and normal indices of the geoset.*/
        void setNormals( Vec3 *np, GLushort *ni );
        /** set the normals and normal indices of the geoset.*/
        void setNormals( Vec3 *np, GLuint *ni );
        /** set the normals and normal indices of the geoset.*/
        void setNormals( Vec3 *np, IndexPointer& ip );
        /** set the normals binding to the vertices/primitives/overall.*/
        void setNormalBinding( BindingType binding );
        inline BindingType getNormalBinding() const        { return _normal_binding; }

        /** get the number of colors required by the defined primitives and color binding.*/
        inline int getNumColors() const                    { return _numcolors; }
        /** get a pointer to Vec4 color array. */
        inline Vec4* getColors()                           { return _colors; }
        /** get a pointer to Vec4 color array. */
        inline const Vec4* getColors() const               { return _colors; }
        /** get the number of colors indices required by the defined primitives and color binding.*/
        inline int getNumColorIndices() const              { return _colindex._size; }
        /** get the color index array. */
        inline IndexPointer& getColorIndices()             { return _colindex; }
        /** get the const color index array. */
        inline const IndexPointer& getColorIndices() const { return _colindex; }
        /** set the colors of the geoset.*/
        void setColors( Vec4 *cp );
        /** set the colors and color indices of the geoset.*/
        void setColors( Vec4 *cp, GLushort *li );
        /** set the colors and color indices of the geoset.*/
        void setColors( Vec4 *cp, GLuint *li );
        /** set the colors and color indices of the geoset.*/
        void setColors( Vec4 *cp, IndexPointer& ip );
        /** set the color binding to the vertices/primitives/overall.*/
        void setColorBinding( BindingType binding );
        inline BindingType getColorBinding() const         { return _color_binding; }

        /** get the number of texture coords required by the defined primitives and textures binding.*/
        inline int getNumTextureCoords() const               { return _numtcoords; }
        /** get a pointer to Vec4 color array. */
        inline Vec2* getTextureCoords()                      { return _tcoords; }
        /** get a pointer to Vec4 color array. */
        inline const Vec2* getTextureCoords() const          { return _tcoords; }
        /** get the number of texture coord indices required by the defined primitives and texture binding.*/
        inline int getNumTextureIndices() const              { return _tindex._size; }
        /** get the texture index array. */
        inline IndexPointer& getTextureIndices()             { return _tindex; }
        /** get the texture index array. */
        inline const IndexPointer& getTextureIndices() const { return _tindex; }
        /** set the texture coords of the geoset.*/
        void setTextureCoords( Vec2 *tc );
        /** set the texture coords and texture coord indices of the geoset.*/
        void setTextureCoords( Vec2 *tc, GLushort *ti );
        /** set the texture coords and texture coord indices of the geoset.*/
        void setTextureCoords( Vec2 *tc, GLuint *ti );
        /** set the texture coords and texture indices of the geoset.*/
        void setTextureCoords( Vec2 *tc, IndexPointer& ip );
        /** set the texture coord binding to the vertices/primitives/overall.*/
        void setTextureBinding( BindingType binding );
        inline BindingType getTextureBinding() const         { return _texture_binding; }

        /** get the number of texture coords required by the defined primitives and textures binding.*/
        inline int getNumInterleavedCoords() const               { return _numcoords; }
        /** get a pointer to interleaved float array. */
        inline void* getInterleavedArray()                      { return _iarray; }
        /** get a const pointer to interleaved float array. */
        inline const void* getInterleavedArray() const          { return _iarray; }
        /** get the number of texture coord indices required by the defined primitives and texture binding.*/
        inline int getNumInterleavedIndices() const              { return _iaindex._size; }
        /** get the texture index array. */
        inline IndexPointer& getInterleavedIndices()             { return _iaindex; }
        /** get the interleaved index array. */
        inline const IndexPointer& getInterleavedIndices() const { return _iaindex; }
        /** get the interleaved array storage format. */
        inline InterleaveArrayType getInterleavedFormat() const  { return _iaformat; }
        
        /** set the interleaved arrays of the geoset.*/
	void setInterleavedArray( InterleaveArrayType format, float *ia ); 
	void setInterleavedArray( InterleaveArrayType format, float *ia, GLushort *iai ); 
	void setInterleavedArray( InterleaveArrayType format, float *ia, GLuint *iai ); 
	void setInterleavedArray( InterleaveArrayType format, float *ia, IndexPointer& iai ); 

        /** draw geoset directly ignoring an OpenGL display list which could be attached.
          * This is the internal draw method which does the drawing itself,
          * and is the method to override when deriving from GeoSet for user-drawn objects.
          */
        virtual void drawImplementation(State&) const {}

        bool check() const;


        /** function object which is used to handling the clean up of attribute arrays
          * associated with GeoSet's.  A default is provided which assumes that all
          * momory attached to the GeoSet is owned by this GeoSet and can be deleted
          * using delete [].  If this is not the cause derive your own AttributeDeleteFunctor
          * a specify your own memory deletion operation.*/
        struct AttributeDeleteFunctor : public osg::Referenced
        {
            // see GeoSet.cpp for implemention.
            virtual void operator() (GeoSet* gset);
        };

        /** set an alternative AttributeDeleteFunction to handle attribute arrays attached to this Geoset.*/
        void setAttributeDeleteFunctor(AttributeDeleteFunctor* adf) { _adf = adf; }
        
        /** get the current AttributeDeleteFunction to handle attribute arrays attached to this Geoset.*/
        AttributeDeleteFunctor* getAttributeDeleteFunctor() { return _adf.get(); }

        /** get the current AttributeDeleteFunction to handle attribute arrays attached to this Geoset.*/
        const AttributeDeleteFunctor* getAttributeDeleteFunctor() const { return _adf.get(); }

        /** return true, osg::GeoSet does support accept(AttributeFunctor&).*/
        virtual bool supports(AttributeFunctor&) const { return true; }

        /** accept an AttributeFunctor and call its methods to tell it about the interal attributes that this Drawable has.*/
        virtual void accept(AttributeFunctor& af);

        /** return true, osg::GeoSet does support accept(ConstAttributeFunctor&).*/
        virtual bool supports(ConstAttributeFunctor&) const { return true; }

        /** accept an ConstAttributeFunctor and call its methods to tell it about the interal attributes that this Drawable has.*/
        virtual void accept(ConstAttributeFunctor& af) const;

        /** return true, osg::GeoSet does support accept(PrimitiveFunctor&) .*/
        virtual bool supports(PrimitiveFunctor&) const { return true; }

        /** accept a PrimtiveFunctor and call its methods to tell it about the interal primtives that this Drawable has.*/
        virtual void accept(PrimitiveFunctor& pf) const;

        /** convinience function for converting GeoSet's to equivilant Geometry nodes.*/
        Geometry* convertToGeometry();

    protected:

        GeoSet& operator = (const GeoSet&) { return *this;}

        virtual ~GeoSet();

        virtual bool computeBound() const;
            
        ref_ptr<AttributeDeleteFunctor> _adf;

        int             _numprims;
        PrimitiveType   _primtype;
	int             _needprimlen;
	unsigned int    _oglprimtype;
        int             *_primLengths;
	mutable unsigned char   _primlength;
	unsigned char   _flat_shaded_skip;

        mutable int     _numcoords;
        Vec3            *_coords;
        mutable IndexPointer    _cindex;

        BindingType     _normal_binding;
        mutable int     _numnormals;
        Vec3            *_normals;
        IndexPointer    _nindex;

        BindingType     _color_binding;
        mutable int     _numcolors;
        Vec4            *_colors;
        IndexPointer    _colindex;

        BindingType     _texture_binding;
        mutable int     _numtcoords;
        Vec2            *_tcoords;
        IndexPointer    _tindex;

	void            *_iarray;
        IndexPointer    _iaindex;
	InterleaveArrayType _iaformat;
	unsigned int    _ogliaformat;


        int         _fast_path;
        
};



}

#endif
