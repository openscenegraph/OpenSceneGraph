#include <osg/Shape>
#include <algorithm>

using namespace osg;

Grid::Grid()
{
}

Grid::Grid(const Grid& mesh,const CopyOp& copyop): 
    HeightField(mesh,copyop)
{
    _heights = mesh._heights;
}

Grid::~Grid()
{
}


void Grid::allocGrid(unsigned int numColumns,unsigned int numRows, float value)
{
    if (_columns!=numColumns || _rows!=numRows)
    {
    	_heights.resize(numColumns*numRows);
    }
    _columns=numColumns;
    _rows=numRows;
    //_heights.fill(value);
}

void Grid::populateGrid(float minValue,float maxValue)
{
    float offset=minValue;
    float gain=(maxValue-minValue)/(float)RAND_MAX;
    for(unsigned int row=0;row<_rows;++row)
    {
    	for(unsigned int col=0;col<_columns;++col)
	{
	    setHeight(col,row,rand()*gain+offset);
	}
    }
}

Vec3 HeightField::getNormal(unsigned int c,unsigned int r) const 
{
    // four point normal generation.

    float dz_dx;
    if (c==0)
    {
    	dz_dx = (getHeight(c+1,r)-getHeight(c,r))/getXInterval();
    }
    else if (c==getNumColumns()-1)
    {
    	dz_dx = (getHeight(c,r)-getHeight(c-1,r))/getXInterval();
    }
    else // assume 0<c<_numColumns-1
    {
    	dz_dx = 0.5f*(getHeight(c+1,r)-getHeight(c-1,r))/getXInterval();
    }
    
    float dz_dy;
    if (r==0)
    {
    	dz_dy = (getHeight(c,r+1)-getHeight(c,r))/getYInterval();
    }
    else if (r==getNumRows()-1)
    {
    	dz_dy = (getHeight(c,r)-getHeight(c,r-1))/getYInterval();
    }
    else // assume 0<r<_numRows-1
    {
    	dz_dy = 0.5f*(getHeight(c,r+1)-getHeight(c,r-1))/getYInterval();
    }
    
    Vec3 normal(-dz_dx,-dz_dy,1.0f);
    normal.normalize();
    
    return normal;
}
