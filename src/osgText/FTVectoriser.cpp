#include    "FTVectoriser.h"
#include    "FTGL.h"


FTContour::FTContour()
:    kMAXPOINTS( 1000)
{    
    pointList.reserve( kMAXPOINTS);
}


FTContour::~FTContour()
{
    pointList.clear();
}


void FTContour::AddPoint( const float x, const float y)
{
    ftPoint point( x, y, 0.0); 
    
    // Eliminate duplicate points.
    if( pointList.empty() || ( pointList[pointList.size() - 1] != point && pointList[0] != point))
    {
        pointList.push_back( point);
    }
}


FTVectoriser::FTVectoriser( FT_Glyph glyph)
:    contour(0),
    contourFlag(0),
    kBSTEPSIZE( 0.2f)
{
    FT_OutlineGlyph outline = (FT_OutlineGlyph)glyph;
    ftOutline = outline->outline;
    
    contourList.reserve( ftOutline.n_contours);
}


FTVectoriser::~FTVectoriser()
{
    for( int c = 0; c < contours(); ++c)
    {
        osgDelete contourList[c];
    }

    contourList.clear();
}


int FTVectoriser::points()
{
    int s = 0;
    for( int c = 0; c < contours(); ++c)
    {
        s += contourList[c]->size();
    }
    
    return s;
}


bool FTVectoriser::Process()
{
    short first = 0;
    short last;
    const short cont = ftOutline.n_contours;
    
    for( short c = 0; c < cont; ++c)
    {
        contour = osgNew FTContour;
        contourFlag = ftOutline.flags;
        last = ftOutline.contours[c];

        for( short p = first; p <= last; ++p)
        {
            switch( ftOutline.tags[p])
            {
                case FT_Curve_Tag_Conic:
                    p += Conic( p, first, last);
                    break;
                case FT_Curve_Tag_Cubic:
                    p += Cubic( p, first, last);
                    break;
                case FT_Curve_Tag_On:
                default:
                     contour->AddPoint( ftOutline.points[p].x, ftOutline.points[p].y);
            }
        }
        
        contourList.push_back( contour);
        first = last + 1;
    }
    
    return true;
}


int FTVectoriser::Conic( const int index, const int first, const int last)
{
    int next = index + 1;
    int prev = index - 1;
    
    if( index == last)
        next = first; 
    
    if( index == first)
        prev = last; 
    
    if( ftOutline.tags[next] != FT_Curve_Tag_Conic)
    {
        ctrlPtArray[0][0] = ftOutline.points[prev].x;    ctrlPtArray[0][1] = ftOutline.points[prev].y;
        ctrlPtArray[1][0] = ftOutline.points[index].x;    ctrlPtArray[1][1] = ftOutline.points[index].y;
        ctrlPtArray[2][0] = ftOutline.points[next].x;    ctrlPtArray[2][1] = ftOutline.points[next].y;
        
        evaluateCurve( 2);
        return 1;
    }
    else
    {
        int next2 = next + 1;
        if( next == last)
            next2 = first;
        
        //create a phantom point
        float x = ( ftOutline.points[index].x + ftOutline.points[next].x) / 2;
        float y = ( ftOutline.points[index].y + ftOutline.points[next].y) / 2;
        
        // process first curve
        ctrlPtArray[0][0] = ftOutline.points[prev].x;    ctrlPtArray[0][1] = ftOutline.points[prev].y;
        ctrlPtArray[1][0] = ftOutline.points[index].x;    ctrlPtArray[1][1] = ftOutline.points[index].y;
        ctrlPtArray[2][0] = x;                            ctrlPtArray[2][1] = y;
        
        evaluateCurve( 2);
        
        // process second curve
        ctrlPtArray[0][0] = x;                            ctrlPtArray[0][1] = y;
        ctrlPtArray[1][0] = ftOutline.points[next].x;    ctrlPtArray[1][1] = ftOutline.points[next].y;
        ctrlPtArray[2][0] = ftOutline.points[next2].x;    ctrlPtArray[2][1] = ftOutline.points[next2].y;
        evaluateCurve( 2);
        
        return 2;
    }
}


int FTVectoriser::Cubic( const int index, const int first, const int last)
{
    int next = index + 1;
    int prev = index - 1;
    
    if( index == last)
        next = first; 
    
    int next2 = next + 1;
    
    if( next == last)
        next2 = first;
    
    if( index == first)
        prev = last; 

    ctrlPtArray[0][0] = ftOutline.points[prev].x;        ctrlPtArray[0][1] = ftOutline.points[prev].y;
    ctrlPtArray[1][0] = ftOutline.points[index].x;        ctrlPtArray[1][1] = ftOutline.points[index].y;
    ctrlPtArray[2][0] = ftOutline.points[next].x;        ctrlPtArray[2][1] = ftOutline.points[next].y;
    ctrlPtArray[3][0] = ftOutline.points[next2].x;        ctrlPtArray[3][1] = ftOutline.points[next2].y;

    evaluateCurve( 3);
    return 2;
}


// De Casteljau algorithm contributed by Jed Soane
void FTVectoriser::deCasteljau( const float t, const int n)
{
    //Calculating successive b(i)'s using de Casteljau algorithm.
    for( int i = 1; i <= n; i++)
        for( int k = 0; k <= (n - i); k++)
        {
            bValues[i][k][0] = (1 - t) * bValues[i - 1][k][0] + t * bValues[i - 1][k + 1][0];
            bValues[i][k][1] = (1 - t) * bValues[i - 1][k][1] + t * bValues[i - 1][k + 1][1];
        }
        
    //Specify next vertex to be included on curve
    contour->AddPoint( bValues[n][0][0], bValues[n][0][1]);
}


// De Casteljau algorithm contributed by Jed Soane
void FTVectoriser::evaluateCurve( const int n)
{
    // setting the b(0) equal to the control points
    for( int i = 0; i <= n; i++)
    {
        bValues[0][i][0] = ctrlPtArray[i][0];
        bValues[0][i][1] = ctrlPtArray[i][1];
    }

    float t;                                //parameter for curve point calc. [0.0, 1.0]

    for( int m = 0; m <= ( 1 / kBSTEPSIZE); m++)
    {
        t = m * kBSTEPSIZE;
        deCasteljau( t, n);  //calls to evaluate point on curve att.
    }
}


void FTVectoriser::MakeOutline( double* data)
{
    int i = 0;
    
    for( int c= 0; c < contours(); ++c)
    {
        const FTContour* contour = contourList[c];
        
        for( int p = 0; p < contour->size(); ++p)
        {
            data[i] = static_cast<double>(contour->pointList[p].x / 64.0f); // is 64 correct?
            data[i + 1] = static_cast<double>(contour->pointList[p].y / 64.0f);
            data[i + 2] = 0.0; // static_cast<double>(contour->pointList[p].z / 64.0f);
            i += 3;
        }
    }
}

