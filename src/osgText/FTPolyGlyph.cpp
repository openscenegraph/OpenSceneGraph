
#include    "FTPolyGlyph.h"
#include    "FTVectoriser.h"

#ifndef CALLBACK
#define CALLBACK
#endif


void CALLBACK ftglError( GLenum /*errCode*/)
{
//    const GLubyte* estring;
//    estring = gluErrorString( errCode);
//    fprintf( stderr, "ERROR : %s\n", estring);
//    exit(1);
}

void CALLBACK ftglVertex( void* data)
{
    glVertex3dv( (double*)data);
}


void CALLBACK ftglBegin( GLenum type)
{
    glBegin( type);
}


void CALLBACK ftglEnd()
{
    glEnd();
}


// this static vector is to keep track of memory allocated by the combine
// callback below, so that it can be later deleted.  This approach does
// assume that the Tesselate method is single threaded.
typedef std::vector<double*> CreatedVertices;
static CreatedVertices s_createdVertices;

void CALLBACK ftglCombine( GLdouble coords[3], void* /*vertex_data*/[4], GLfloat /*weight*/[4], void** outData)
{
    double* vertex = osgNew double[3]; // FIXME MEM LEAK

    s_createdVertices.push_back(vertex);

    vertex[0] = coords[0];
    vertex[1] = coords[1];
    vertex[2] = coords[2];

    *outData = vertex;
}


FTPolyGlyph::FTPolyGlyph( FT_Glyph glyph)
:    FTGlyph(),
    vectoriser(0),
    numPoints(0),
    numContours(0),
    contourFlag(0),
    contourLength(0),
    data(0),
    glList(0)
{
    if( ft_glyph_format_outline != glyph->format)
    { return;}

    vectoriser = osgNew FTVectoriser( glyph);
    
    vectoriser->Process();
    numContours = vectoriser->contours();
    
    if (numContours==0) return;
    
    contourLength = osgNew int[ numContours];
    memset(contourLength,0,sizeof(int)*numContours);
    
    for( int c = 0; c < numContours; ++c)
    {
        contourLength[c] = vectoriser->contourSize( c);
    }
    
    numPoints = vectoriser->points();
    data = osgNew double[ numPoints * 3];
    // initalize memory.
    for( int pc=0;pc<numPoints*3;++pc) data[pc]=0.0;

    // FIXME MakeMesh
    vectoriser->MakeOutline( data);
    
    contourFlag = vectoriser->ContourFlag();
    advance = glyph->advance.x >> 16;

    vectoriser=0; // delete it, using ref_ptr.

    if ( ( numContours < 1) || ( numPoints < 3))
        return;

    Tesselate();

    // discard glyph image (bitmap or not)
    FT_Done_Glyph( glyph); // Why does this have to be HERE
}


void FTPolyGlyph::Tesselate()
{
    glList = glGenLists(1);
    GLUtesselator* tobj = gluNewTess();
    int d = 0;
    
    gluTessCallback( tobj, GLU_TESS_BEGIN, (void (CALLBACK*)())ftglBegin);
    gluTessCallback( tobj, GLU_TESS_VERTEX, (void (CALLBACK*)())ftglVertex);
    gluTessCallback( tobj, GLU_TESS_COMBINE, (void (CALLBACK*)())ftglCombine);
    gluTessCallback( tobj, GLU_TESS_END, ftglEnd);
    gluTessCallback( tobj, GLU_TESS_ERROR, (void (CALLBACK*)())ftglError);
    
    glNewList( glList, GL_COMPILE);
    
        if( contourFlag & ft_outline_even_odd_fill) // ft_outline_reverse_fill
        {
            gluTessProperty( tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
        }
        else
        {
            gluTessProperty( tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);
        }
        
        gluTessProperty( tobj, GLU_TESS_TOLERANCE, 0);
        gluTessBeginPolygon( tobj, NULL);
        
            for( int c = 0; c < numContours; ++c)
            {
                gluTessBeginContour( tobj);
                    for( int p = 0; p < ( contourLength[c]); ++p)
                    {
                        gluTessVertex( tobj, data + d, data + d);
                        d += 3;
                    }
                gluTessEndContour( tobj);
            }
            
        gluTessEndPolygon( tobj);
        
    glEndList();

    gluDeleteTess( tobj);

    // clean up the vertices create in the combine callback.
    for(CreatedVertices::iterator itr=s_createdVertices.begin();
        itr!=s_createdVertices.end();
        ++itr)
    {
        osgDelete [] (*itr);
    }
    s_createdVertices.clear();
}


FTPolyGlyph::~FTPolyGlyph()
{
    osgDelete [] data;
    osgDelete [] contourLength;
}


float FTPolyGlyph::Render( const FT_Vector& pen)
{
    if( glList)
    {
        glTranslatef( pen.x, pen.y, 0);
            glCallList( glList);    
        glTranslatef( -pen.x, -pen.y, 0);
    }
    
    return advance;
}
