#ifndef		__FTVectoriser__
#define		__FTVectoriser__

#include <vector>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

#include "FTGlyph.h"

using namespace std;

class ftPoint
{
	public:
		ftPoint()
		: x(0), y(0), z(0)
		{}
		
		ftPoint( const float X, const float Y, const float Z)
		: x(X), y(Y), z(Z)
		{}
		
		friend bool operator == ( const ftPoint &a, const ftPoint &b) 
		{
			return((a.x == b.x) && (a.y == b.y) && (a.z == b.z));
		}

		friend bool operator != ( const ftPoint &a, const ftPoint &b) 
		{
			return((a.x != b.x) || (a.y != b.y) || (a.z != b.z));
		}
		
		float x, y, z; // FIXME make private
		
	private:
};


class FTContour
{
	public:
		// methods
		FTContour();
		~FTContour();
	
		void AddPoint( const float x, const float y);
		
		int size() const { return pointList.size();}

		// attributes
		vector< ftPoint> pointList;
		float ctrlPtArray[4][2];
		
	private:
		// methods

		// attributes
		const unsigned int kMAXPOINTS;
};


class FTVectoriser
{
	public:
		// methods
		FTVectoriser( FT_Glyph glyph);
		virtual ~FTVectoriser();
		
		bool Ingest();
		void Output( double* d);
		int points();
		int contours() const { return contourList.size();}
		int contourSize( int c) const { return contourList[c]->size();}
		int ContourFlag() const { return contourFlag;}
		
		// attributes
		
	private:
		// methods
		int Conic( const int index, const int first, const int last);
		int Cubic( const int index, const int first, const int last);
		void deCasteljau( const float t, const int n);
		void evaluateCurve( const int n);

		// attributes
		vector< const FTContour*> contourList;
			
		FTContour* contour;
		int contourFlag;

		FT_Outline ftOutline;
		
		 // Magic numbers -- #define MAX_DEG 4
		float bValues[4][4][2];	//3D array storing values of de Casteljau algorithm.
		float ctrlPtArray[4][2]; // Magic numbers
		
		const float kBSTEPSIZE;

};


#endif	//	__FTVectoriser__
