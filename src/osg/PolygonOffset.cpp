#include "osg/GL"
#include "osg/PolygonOffset"
#include "osg/Input"
#include "osg/Output"

using namespace osg;


PolygonOffset::PolygonOffset( void )
{
    _factor = 0.0f; // are these sensible defaut values?
    _units = 0.0f;
}


PolygonOffset::~PolygonOffset( void )
{
}


PolygonOffset* PolygonOffset::instance()
{
    static ref_ptr<PolygonOffset> s_PolygonOffset(new PolygonOffset);
    return s_PolygonOffset.get();
}

void PolygonOffset::setOffset(float factor,float units)
{
    _factor = factor;
    _units = units;
}

void PolygonOffset::enable( void )
{
    glEnable( GL_POLYGON_OFFSET_FILL);
    glEnable( GL_POLYGON_OFFSET_LINE);
    glEnable( GL_POLYGON_OFFSET_POINT);
}


void PolygonOffset::disable( void )
{
    glDisable( GL_POLYGON_OFFSET_FILL);
    glDisable( GL_POLYGON_OFFSET_LINE);
    glDisable( GL_POLYGON_OFFSET_POINT);
}

void PolygonOffset::apply( void )
{
    glPolygonOffset(_factor,_units);
}

bool PolygonOffset::readLocalData(Input& fr)
{
    bool iteratorAdvanced = false;

    float data;
    if (fr[0].matchWord("factor") && fr[1].getFloat(data))
    {

        _factor = data;
        fr+=2;
        iteratorAdvanced = true;
    }

    if (fr[0].matchWord("units") && fr[1].getFloat(data))
    {

        _units = data;
        fr+=2;
        iteratorAdvanced = true;
    }

    return iteratorAdvanced;
}


bool PolygonOffset::writeLocalData(Output& fw)
{
    fw.indent() << "factor " << _factor << endl;
    fw.indent() << "units  " << _units << endl;
    return true;
}


