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
