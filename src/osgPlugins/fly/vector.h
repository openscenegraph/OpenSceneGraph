#ifndef __VECTOR_H
#define __VECTOR_H


struct _vector {
	float x, y, z;
};

typedef struct _vector *Vector;
typedef struct _vector sgvector;

extern Vector 	NewVector( void );
extern void 	DeleteVector( Vector v );
extern void 	VectorSet( Vector v, float a, float b, float c );
extern void	VectorGet( Vector v, float *a, float *b, float *c );
extern float 	VectorDotProduct( Vector p, Vector q );
extern void 	VectorCrossProduct(Vector v1, Vector v2, Vector prod);
extern void 	VectorAdd( Vector v1, Vector v2, Vector diff );
extern void 	VectorDiff( Vector v1, Vector v2, Vector diff );
extern void 	VectorNormalize( Vector v );
extern float 	VectorAngle( Vector v1, Vector v2 );
extern char *	VectorToA( Vector v, char *cptr, int size );
extern float 	VectorMagnitude( Vector v );
extern void 	VectorMagnify( Vector v, float magnitude, Vector res );


#endif
