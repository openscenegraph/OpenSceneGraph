#include    "FTOutlineGlyph.h"
#include    "FTVectoriser.h"
#include    "FTGL.h"



FTOutlineGlyph::FTOutlineGlyph( FT_Glyph glyph)
:    FTGlyph(),
    vectoriser(0),
    numPoints(0),
    numContours(0),
    contourLength(0),
    data(0),
    glList(0)
{
    if( ft_glyph_format_outline != glyph->format)
    {
        return;
    }

    advance = glyph->advance.x >> 16;

    vectoriser = osgNew FTVectoriser( glyph);
    
    vectoriser->Process();
    numContours = vectoriser->contours();
    
    if (numContours==0)
    {
        return;
    }
    
    contourLength = osgNew int[ numContours];
    memset(contourLength,0,sizeof(int)*numContours);
    for( int cn = 0; cn < numContours; ++cn)
    {
        contourLength[cn] = vectoriser->contourSize( cn);
    }
    
    numPoints = vectoriser->points();
    data = osgNew double[ numPoints * 3];
    for( int cp = 0; cp < numPoints * 3; ++cp)
    {
        data[cp]=0.0;
    }
    
    vectoriser->MakeOutline( data);
    

    vectoriser=0;
    
    if ( ( numContours < 1) || ( numPoints < 3))
        return;
        
    glList = glGenLists(1);
    int d = 0;

    glNewList( glList, GL_COMPILE);
        for( int c = 0; c < numContours; ++c)
        {
            glBegin( GL_LINE_LOOP);
            for( int p = 0; p < ( contourLength[c]); ++p)
            {
                glVertex2dv( data + d);
                d += 3;
            }
            glEnd();
        }
    glEndList();

    // discard glyph image (bitmap or not)
    FT_Done_Glyph( glyph); // Why does this have to be HERE
}


FTOutlineGlyph::~FTOutlineGlyph()
{
    osgDelete [] data;
    osgDelete [] contourLength;
}


float FTOutlineGlyph::Render( const FT_Vector& pen)
{
    if( glList)
    {
        glTranslatef( pen.x, pen.y, 0);
            glCallList( glList);
        glTranslatef( -pen.x, -pen.y, 0);
    }
    
    return advance;
}

