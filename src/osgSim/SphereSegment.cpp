#include <osgSim/SphereSegment>
#include <osg/Notify>
#include <osg/CullFace>
#include <osg/LineWidth>

using namespace osgSim;

// Define the collection of nested classes, all Drawables, which make
// up the parts of the sphere segment.

/**
SphereSegment::Surface is the Drawable which represents the specified area of the
sphere's surface.
 */
class SphereSegment::Surface: public osg::Drawable
{
public:
    Surface(SphereSegment* ss): osg::Drawable(), _ss(ss) {}

    Surface():_ss(0)
    {
        osg::notify(osg::WARN)<<
            "Warning: unexpected call to osgSim::SphereSegment::Surface() default constructor"<<std::endl;
    }

    Surface(const Surface& rhs, const osg::CopyOp& co=osg::CopyOp::SHALLOW_COPY):osg::Drawable(rhs,co), _ss(0)
    {
        osg::notify(osg::WARN)<<
            "Warning: unexpected call to osgSim::SphereSegment::Surface() copy constructor"<<std::endl;
    }

    META_Object(osgSim,Surface)

    void drawImplementation(osg::State& state) const;

protected:

    virtual bool computeBound() const;

private:

    SphereSegment* _ss;
};

void SphereSegment::Surface::drawImplementation(osg::State& state) const
{
    _ss->Surface_drawImplementation(state);
}

bool SphereSegment::Surface::computeBound() const
{
    _bbox_computed = _ss->Surface_computeBound(_bbox);
    return _bbox_computed;
}


/**
SphereSegment::EdgeLine is the Drawable which represents the line around the edge
of the specified area of the sphere's EdgeLine.
 */
class SphereSegment::EdgeLine: public osg::Drawable
{
public:
    EdgeLine(SphereSegment* ss): osg::Drawable(), _ss(ss) { init(); }

    EdgeLine():_ss(0)
    {
    	init();
        osg::notify(osg::WARN)<<
            "Warning: unexpected call to osgSim::SphereSegment::EdgeLine() default constructor"<<std::endl;
    }

    EdgeLine(const EdgeLine& rhs, const osg::CopyOp& co=osg::CopyOp::SHALLOW_COPY):osg::Drawable(rhs,co), _ss(0)
    {
        osg::notify(osg::WARN)<<
            "Warning: unexpected call to osgSim::SphereSegment::EdgeLine() copy constructor"<<std::endl;
    }

    META_Object(osgSim,EdgeLine)

    void drawImplementation(osg::State& state) const;

protected:

    void init()
    {
	// switch off lighting.
	getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);
	
	//getOrCreateStateSet()->setAttributeAndModes(new osg::LineWidth(2.0),osg::StateAttribute::OFF);
    }


    virtual bool computeBound() const;

private:

    SphereSegment* _ss;
};

void SphereSegment::EdgeLine::drawImplementation(osg::State& state) const
{
    _ss->EdgeLine_drawImplementation(state);
}

bool SphereSegment::EdgeLine::computeBound() const
{
    _bbox_computed = _ss->EdgeLine_computeBound(_bbox);
    return _bbox_computed;
}



/**
SphereSegment::Side is a Drawable which represents one of the
planar areas, at either the minimum or maxium azimuth.
 */
class SphereSegment::Side: public osg::Drawable
{
public:
    Side(SphereSegment* ss, SphereSegment::SideOrientation po, SphereSegment::BoundaryAngle pa):
            osg::Drawable(), _ss(ss), _planeOrientation(po), _BoundaryAngle(pa) {}

    Side():_ss(0)
    {
        osg::notify(osg::WARN)<<
            "Warning: unexpected call to osgSim::SphereSegment::Side() default constructor"<<std::endl;
    }

    Side(const Side& rhs, const osg::CopyOp& co=osg:: CopyOp::SHALLOW_COPY): osg::Drawable(rhs,co), _ss(0)
    {
        osg::notify(osg::WARN)<<
            "Warning: unexpected call to osgSim::SphereSegment::Side() copy constructor"<<std::endl;
    }

    META_Object(osgSim,Side)

    void drawImplementation(osg::State& state) const;

protected:

    virtual bool computeBound() const;

private:
    SphereSegment* _ss;
    SphereSegment::SideOrientation _planeOrientation;
    SphereSegment::BoundaryAngle _BoundaryAngle;
};


void SphereSegment::Side::drawImplementation(osg::State& state) const
{
    _ss->Side_drawImplementation(state, _planeOrientation, _BoundaryAngle);
}

bool SphereSegment::Side::computeBound() const
{
    _bbox_computed = _ss->Side_computeBound(_bbox, _planeOrientation, _BoundaryAngle);
    return _bbox_computed;
}



/**
SphereSegment::Spoke is a Drawable which represents a spoke.
 */
class SphereSegment::Spoke: public osg::Drawable
{
public:
    Spoke(SphereSegment* ss, SphereSegment::BoundaryAngle azAngle, SphereSegment::BoundaryAngle elevAngle):
            osg::Drawable(), _ss(ss), _azAngle(azAngle), _elevAngle(elevAngle) { init(); }

    Spoke():_ss(0)
    {
    	init();
        osg::notify(osg::WARN)<<
            "Warning: unexpected call to osgSim::SphereSegment::Spoke() default constructor"<<std::endl;
    }

    Spoke(const Spoke& rhs, const osg::CopyOp& co=osg:: CopyOp::SHALLOW_COPY): osg::Drawable(rhs,co), _ss(0)
    {
        osg::notify(osg::WARN)<<
            "Warning: unexpected call to osgSim::SphereSegment::Spoke() copy constructor"<<std::endl;
    }

    META_Object(osgSim,Spoke)

    void drawImplementation(osg::State& state) const;

protected:

    void init()
    {
	// switch off lighting.
	getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

	//getOrCreateStateSet()->setAttributeAndModes(new osg::LineWidth(2.0),osg::StateAttribute::OFF);
    }
    
    virtual bool computeBound() const;

private:
    SphereSegment* _ss;
    SphereSegment::BoundaryAngle _azAngle, _elevAngle;
};

void SphereSegment::Spoke::drawImplementation(osg::State& state) const
{
    _ss->Spoke_drawImplementation(state, _azAngle, _elevAngle);
}

bool SphereSegment::Spoke::computeBound() const
{
    _bbox_computed = _ss->Spoke_computeBound(_bbox, _azAngle, _elevAngle);
    return _bbox_computed;
}

SphereSegment::SphereSegment(const osg::Vec3& centre, float radius, const osg::Vec3& vec, float azRange,
                float elevRange, int density):
    osg::Geode(),
    _centre(centre), _radius(radius),
    _density(density),
    _drawMask(DrawMask(ALL))
{
    // Rather than store the vector, we'll work out the azimuth boundaries and elev
    // boundaries now, rather than at draw time.
    setArea(vec, azRange, elevRange);

    init();
}

void SphereSegment::setCentre(const osg::Vec3& c)
{
    _centre = c;
    dirtyAllDrawableDisplayLists();
    dirtyAllDrawableBounds();
    dirtyBound();
}

const osg::Vec3& SphereSegment::getCentre() const
{
    return _centre;
}

void SphereSegment::setRadius(float r)
{
    _radius = r;
    dirtyAllDrawableDisplayLists();
    dirtyAllDrawableBounds();
    dirtyBound();
}

float SphereSegment::getRadius() const
{
    return _radius;
}


void SphereSegment::setArea(const osg::Vec3& v, float azRange, float elevRange)
{
    osg::Vec3 vec(v);

    vec.normalize();    // Make sure we're unit length

    // Calculate the elevation range
    float elev = asin(vec.z());   // Elevation angle
    elevRange /= 2.0f;
    _elevMin = elev - elevRange;
    _elevMax = elev + elevRange;

    // Calculate the azimuth range, cater for trig ambiguities
    float xyLen = cos(elev);
    float az;
    if(vec.x() != 0.0f) az = asin(vec.x()/xyLen);
    else az = acos(vec.y()/xyLen);

    azRange /= 2.0f;
    _azMin = az - azRange;
    _azMax = az + azRange;

    dirtyAllDrawableDisplayLists();
    dirtyAllDrawableBounds();
    dirtyBound();
}

void SphereSegment::getArea(osg::Vec3& vec, float& azRange, float& elevRange) const
{
    azRange = _azMax - _azMin;
    elevRange = _elevMax - _elevMin;

    float az = azRange/2.0f;
    float elev = elevRange/2.0f;
    vec.set(cos(elev)*sin(az), cos(elev)*cos(az), sin(elev));
}

void SphereSegment::setArea(float azMin, float azMax,
    float elevMin, float elevMax)
{
    _azMin=azMin;
    _azMax=azMax;
    _elevMin=elevMin;
    _elevMax=elevMax;

    dirtyAllDrawableDisplayLists();
    dirtyAllDrawableBounds();
    dirtyBound();
}

void SphereSegment::getArea(float &azMin, float &azMax,
    float &elevMin, float &elevMax) const
{
    azMin=_azMin;
    azMax=_azMax;
    elevMin=_elevMin;
    elevMax=_elevMax;
}

void SphereSegment::setDensity(int density)
{
    _density = density;
    dirtyAllDrawableDisplayLists();
}

int SphereSegment::getDensity() const
{
    return _density;
}

void SphereSegment::init()
{
    addDrawable(new Surface(this));

    addDrawable(new EdgeLine(this));

    addDrawable(new Side(this,AZIM,MIN));
    addDrawable(new Side(this,AZIM,MAX));
    addDrawable(new Side(this,ELEV,MIN));
    addDrawable(new Side(this,ELEV,MAX));

    addDrawable(new Spoke(this,MIN,MIN));
    addDrawable(new Spoke(this,MIN,MAX));
    addDrawable(new Spoke(this,MAX,MIN));
    addDrawable(new Spoke(this,MAX,MAX));
}

namespace
{

struct DirtyDisplayList
{
    void operator()(osg::ref_ptr<osg::Drawable>& dptr)
    {
        dptr->dirtyDisplayList();
    }
};

}

void SphereSegment::dirtyAllDrawableDisplayLists()
{
    std::for_each(_drawables.begin(), _drawables.end(), DirtyDisplayList());
}

namespace
{

struct DirtyBound
{
    void operator()(osg::ref_ptr<osg::Drawable>& dptr)
    {
        dptr->dirtyBound();
    }
};

}

void SphereSegment::dirtyAllDrawableBounds()
{
    std::for_each(_drawables.begin(), _drawables.end(), DirtyBound());
}

void SphereSegment::Surface_drawImplementation(osg::State& /* state */) const
{
    const float azIncr = (_azMax - _azMin)/_density;
    const float elevIncr = (_elevMax - _elevMin)/_density;

    // Draw the area on the sphere surface if needed
    // ---------------------------------------------
    if(_drawMask & SURFACE)
    {
        glColor4fv(_surfaceColor.ptr());

    	bool drawBackSide = true;
    	bool drawFrontSide = true;

    	// draw back side.
	if (drawBackSide)
	{
            for(int i=0; i+1<=_density; i++)
            {
        	// Because we're drawing quad strips, we need to work out
        	// two azimuth values, to form each edge of the (z-vertical)
        	// strips
        	float az1 = _azMin + (i*azIncr);
        	float az2 = _azMin + ((i+1)*azIncr);

        	glBegin(GL_QUAD_STRIP);
        	for (int j=0; j<=_density; j++)
        	{
                    float elev = _elevMin + (j*elevIncr);

                    // QuadStrip Edge formed at az1
                    // ----------------------------

                    // Work out the sphere normal
                    float x = cos(elev)*sin(az1);
                    float y = cos(elev)*cos(az1);
                    float z = sin(elev);

                    glNormal3f(-x, -y, -z);
                    glVertex3f(_centre.x() + _radius*x,
                            _centre.y() + _radius*y,
                            _centre.z() + _radius*z);

                    // QuadStrip Edge formed at az2
                    // ----------------------------

                    // Work out the sphere normal
                    x = cos(elev)*sin(az2);
                    y = cos(elev)*cos(az2);
                    // z = sin(elev);   z doesn't change

                    glNormal3f(-x, -y, -z);
                    glVertex3f(_centre.x() + _radius*x,
                            _centre.y() + _radius*y,
                            _centre.z() + _radius*z);
        	}
        	glEnd();
            }
    	}
	
    	// draw front side
	if (drawFrontSide)
	{
            for(int i=0; i+1<=_density; i++)
            {
        	// Because we're drawing quad strips, we need to work out
        	// two azimuth values, to form each edge of the (z-vertical)
        	// strips
        	float az1 = _azMin + (i*azIncr);
        	float az2 = _azMin + ((i+1)*azIncr);

        	glBegin(GL_QUAD_STRIP);
        	for (int j=0; j<=_density; j++)
        	{
                    float elev = _elevMin + (j*elevIncr);

                    // QuadStrip Edge formed at az1
                    // ----------------------------

                    // Work out the sphere normal
                    float x = cos(elev)*sin(az2);
                    float y = cos(elev)*cos(az2);
                    float z = sin(elev);

                    glNormal3f(x, y, z);
                    glVertex3f(_centre.x() + _radius*x,
                            _centre.y() + _radius*y,
                            _centre.z() + _radius*z);

                    // QuadStrip Edge formed at az2
                    // ----------------------------

                    // Work out the sphere normal
                    // z = sin(elev);   z doesn't change

                    x = cos(elev)*sin(az1);
                    y = cos(elev)*cos(az1);

                    glNormal3f(x, y, z);
                    glVertex3f(_centre.x() + _radius*x,
                            _centre.y() + _radius*y,
                            _centre.z() + _radius*z);
        	}
        	glEnd();
            }
    	}
    }
}

bool SphereSegment::Surface_computeBound(osg::BoundingBox& bbox) const
{
    bbox.init();

    float azIncr = (_azMax - _azMin)/_density;
    float elevIncr = (_elevMax - _elevMin)/_density;

    for(int i=0; i<=_density; i++){

        float az = _azMin + (i*azIncr);

        for(int j=0; j<=_density; j++){

            float elev = _elevMin + (j*elevIncr);

            bbox.expandBy(
                osg::Vec3(_centre.x() + _radius*cos(elev)*sin(az),
                        _centre.y() + _radius*cos(elev)*cos(az),
                        _centre.z() + _radius*sin(elev))
            );
        }
    }
    return true;
}

void SphereSegment::EdgeLine_drawImplementation(osg::State& /* state */) const
{
    const float azIncr = (_azMax - _azMin)/_density;
    const float elevIncr = (_elevMax - _elevMin)/_density;

    // Draw the edgeline if necessary
    // ------------------------------
    if(_drawMask & EDGELINE)
    {
        glColor4fv(_edgeLineColor.ptr());

        // Top edge
        glBegin(GL_LINE_STRIP);
        int i;
        for(i=0; i<=_density; i++)
        {
            float az = _azMin + (i*azIncr);
            glVertex3f(
                _centre.x() + _radius*cos(_elevMax)*sin(az),
                _centre.y() + _radius*cos(_elevMax)*cos(az),
                _centre.z() + _radius*sin(_elevMax));
        }
        glEnd();

        // Bottom edge
        glBegin(GL_LINE_STRIP);
        for(i=0; i<=_density; i++)
        {
            float az = _azMin + (i*azIncr);
            glVertex3f(
                _centre.x() + _radius*cos(_elevMin)*sin(az),
                _centre.y() + _radius*cos(_elevMin)*cos(az),
                _centre.z() + _radius*sin(_elevMin));
        }
        glEnd();

        // Left edge
        glBegin(GL_LINE_STRIP);
        int j;
        for(j=0; j<=_density; j++)
        {
            float elev = _elevMin + (j*elevIncr);
            glVertex3f(
                _centre.x() + _radius*cos(elev)*sin(_azMin),
                _centre.y() + _radius*cos(elev)*cos(_azMin),
                _centre.z() + _radius*sin(elev));
        }
        glEnd();

        // Right edge
        glBegin(GL_LINE_STRIP);
        for(j=0; j<=_density; j++)
        {
            float elev = _elevMin + (j*elevIncr);
            glVertex3f(
                _centre.x() + _radius*cos(elev)*sin(_azMax),
                _centre.y() + _radius*cos(elev)*cos(_azMax),
                _centre.z() + _radius*sin(elev));
        }
        glEnd();
#if 0
        // Split right
        glBegin(GL_LINE_STRIP);
        glVertex3f(
                _centre.x() + _radius*cos(_elevMin)*sin(_azMax),
                _centre.y() + _radius*cos(_elevMin)*cos(_azMax),
                _centre.z() + _radius*sin(_elevMin));
        glVertex3f(_centre.x(), _centre.y(), _centre.z());
        glVertex3f(
                _centre.x() + _radius*cos(_elevMax)*sin(_azMax),
                _centre.y() + _radius*cos(_elevMax)*cos(_azMax),
                _centre.z() + _radius*sin(_elevMax));
        glEnd();
 
        // Split left
        glBegin(GL_LINE_STRIP);
        glVertex3f(
                _centre.x() + _radius*cos(_elevMin)*sin(_azMin),
                _centre.y() + _radius*cos(_elevMin)*cos(_azMin),
                _centre.z() + _radius*sin(_elevMin));
        glVertex3f(_centre.x(), _centre.y(), _centre.z());
        glVertex3f(
                _centre.x() + _radius*cos(_elevMax)*sin(_azMin),
                _centre.y() + _radius*cos(_elevMax)*cos(_azMin),
                _centre.z() + _radius*sin(_elevMax));
        glEnd();
#endif
    }
}

bool SphereSegment::EdgeLine_computeBound(osg::BoundingBox& bbox) const
{
    bbox.init();

    float azIncr = (_azMax - _azMin)/_density;
    float elevIncr = (_elevMax - _elevMin)/_density;

    // Top edge
    int i;
    for(i=0; i<=_density; i++)
    {
        float az = _azMin + (i*azIncr);
        bbox.expandBy(
            _centre.x() + _radius*cos(_elevMax)*sin(az),
            _centre.y() + _radius*cos(_elevMax)*cos(az),
            _centre.z() + _radius*sin(_elevMax));
    }

    // Bottom edge
    for(i=0; i<=_density; i++)
    {
        float az = _azMin + (i*azIncr);
        bbox.expandBy(
            _centre.x() + _radius*cos(_elevMin)*sin(az),
            _centre.y() + _radius*cos(_elevMin)*cos(az),
            _centre.z() + _radius*sin(_elevMin));
    }

    // Left edge
    int j;
    for(j=0; j<=_density; j++)
    {
        float elev = _elevMin + (j*elevIncr);
        bbox.expandBy(
            _centre.x() + _radius*cos(elev)*sin(_azMin),
            _centre.y() + _radius*cos(elev)*cos(_azMin),
            _centre.z() + _radius*sin(elev));
    }

    // Right edge
    for(j=0; j<=_density; j++)
    {
        float elev = _elevMin + (j*elevIncr);
        bbox.expandBy(
            _centre.x() + _radius*cos(elev)*sin(_azMax),
            _centre.y() + _radius*cos(elev)*cos(_azMax),
            _centre.z() + _radius*sin(elev));
    }

    return true;
}

void SphereSegment::Side_drawImplementation(osg::State& /* state */,
                                SphereSegment::SideOrientation orientation,
                                SphereSegment::BoundaryAngle boundaryAngle) const
{
    // Draw the planes if necessary
    // ----------------------------
    if(_drawMask & SIDES)
    {
    	bool drawBackSide = true;
    	bool drawFrontSide = true;
    	int start, end, delta;

        glColor4fv(_planeColor.ptr());

    	// draw back side.
	if (drawBackSide)
	{

            if(orientation == AZIM)      // This is a plane at a given azimuth
            {
        	const float az = (boundaryAngle==MIN?_azMin:_azMax);
        	const float elevIncr = (_elevMax - _elevMin)/_density;

        	// Normal
        	osg::Vec3 normal = osg::Vec3(cos(_elevMin)*sin(az), cos(_elevMin)*cos(az), sin(_elevMin))
                                    ^ osg::Vec3(cos(_elevMax)*sin(az), cos(_elevMax)*cos(az), sin(_elevMax));

    	    	if (boundaryAngle==MIN)
		{
		    start = _density;
		    end = 0;
		}
		else
		{
		    start = 0;
		    end = _density;
		    normal = -normal;   // Make sure normals orientationint 'outwards'
		}		
		delta = end>start?1:-1;

    	    	if (drawBackSide)
		{
        	    // Tri fan
        	    glNormal3f(-normal.x(),-normal.y(),-normal.z());
        	    glBegin(GL_TRIANGLE_FAN);
        	    glVertex3fv(_centre.ptr());
        	    for (int j=start; j!=end+delta; j+=delta)
        	    {
                	float elev = _elevMin + (j*elevIncr);
                	glVertex3f( _centre.x() + _radius*cos(elev)*sin(az),
                        	    _centre.y() + _radius*cos(elev)*cos(az),
                        	    _centre.z() + _radius*sin(elev));
        	    }
        	    glEnd();
    	    	}
		
    	    	if (boundaryAngle==MIN)
		{
		    start = 0;
		    end = _density;
		}
		else
		{
		    start = _density;
		    end = 0;
		}		
		delta = end>start?1:-1;

		if (drawFrontSide)
		{
        	    glNormal3fv(normal.ptr());
        	    glBegin(GL_TRIANGLE_FAN);
        	    glVertex3fv(_centre.ptr());
        	    for (int j=start; j!=end+delta; j+=delta)
        	    {
                	float elev = _elevMin + (j*elevIncr);
                	glVertex3f( _centre.x() + _radius*cos(elev)*sin(az),
                        	    _centre.y() + _radius*cos(elev)*cos(az),
                        	    _centre.z() + _radius*sin(elev));
        	    }
    	   	    glEnd();
		}

            }
            else if(orientation == ELEV) // This is a plane at a given elevation
            {
        	const float elev = (boundaryAngle==MIN?_elevMin:_elevMax);
        	const float azIncr = (_azMax - _azMin)/_density;

        	// Normal
        	osg::Vec3 normal = osg::Vec3(cos(elev)*sin(_azMax), cos(elev)*cos(_azMax), sin(elev))
                                    ^ osg::Vec3(cos(elev)*sin(_azMin), cos(elev)*cos(_azMin), sin(elev));


    	    	if (boundaryAngle==MIN)
		{
		    start = _density;
		    end = 0;
		    normal = -normal;   // Make sure normals orientationint 'outwards'
		}
		else
		{
		    start = 0;
		    end = _density;
		}		
		delta = end>start?1:-1;

    	    	if (drawBackSide)
		{
        	    glNormal3f(-normal.x(),-normal.y(),-normal.z());

        	    // Tri fan
        	    glBegin(GL_TRIANGLE_FAN);
        	    glVertex3fv(_centre.ptr());
        	    for (int j=start; j!=end+delta; j+=delta)
        	    {
                	float az = _azMin + (j*azIncr);
                	glVertex3f( _centre.x() + _radius*cos(elev)*sin(az),
                        	    _centre.y() + _radius*cos(elev)*cos(az),
                        	    _centre.z() + _radius*sin(elev));
        	    }
        	    glEnd();
		}
		
    	    	if (boundaryAngle==MIN)
		{
		    start = 0;
		    end = _density;
		}
		else
		{
		    start = _density;
		    end = 0;
		}		
		delta = end>start?1:-1;

    	    	if (drawFrontSide)
		{
        	    glNormal3fv(normal.ptr());

        	    // Tri fan
        	    glBegin(GL_TRIANGLE_FAN);
        	    glVertex3fv(_centre.ptr());
        	    for (int j=start; j!=end+delta; j+=delta)
        	    {
                	float az = _azMin + (j*azIncr);
                	glVertex3f( _centre.x() + _radius*cos(elev)*sin(az),
                        	    _centre.y() + _radius*cos(elev)*cos(az),
                        	    _centre.z() + _radius*sin(elev));
        	    }
        	    glEnd();
		}
		
            }
	}
	
    }
}

bool SphereSegment::Side_computeBound(osg::BoundingBox& bbox,
                                SphereSegment::SideOrientation orientation,
                                SphereSegment::BoundaryAngle boundaryAngle) const
{
    bbox.init();
    bbox.expandBy(_centre);

    if(orientation == AZIM)      // This is a plane at a given azimuth
    {
        const float az = (boundaryAngle==MIN?_azMin:_azMax);
        const float elevIncr = (_elevMax - _elevMin)/_density;

        for (int j=0; j<=_density; j++)
        {
            float elev = _elevMin + (j*elevIncr);
            bbox.expandBy(
                _centre.x() + _radius*cos(elev)*sin(az),
                _centre.y() + _radius*cos(elev)*cos(az),
                _centre.z() + _radius*sin(elev));
        }
    }
    else if(orientation == ELEV) // This is a plane at a given elevation
    {
        const float elev = (boundaryAngle==MIN?_elevMin:_elevMax);
        const float azIncr = (_azMax - _azMin)/_density;

        for(int i=0; i<=_density; i++)
        {
            float az = _azMin + (i*azIncr);
            bbox.expandBy(
                _centre.x() + _radius*cos(elev)*sin(az),
                _centre.y() + _radius*cos(elev)*cos(az),
                _centre.z() + _radius*sin(elev));
        }
    }

    return true;
}

void SphereSegment::Spoke_drawImplementation(osg::State&, BoundaryAngle azAngle, BoundaryAngle elevAngle) const
{
    if(_drawMask & SPOKES){

        glColor4fv(_spokeColor.ptr());

        const float az = (azAngle==MIN?_azMin:_azMax);
        const float elev = (elevAngle==MIN?_elevMin:_elevMax);

        glBegin(GL_LINES);
            glVertex3fv(_centre.ptr());
            glVertex3f( _centre.x() + _radius*cos(elev)*sin(az),
                        _centre.y() + _radius*cos(elev)*cos(az),
                        _centre.z() + _radius*sin(elev));
        glEnd();
    }
}

bool SphereSegment::Spoke_computeBound(osg::BoundingBox& bbox, BoundaryAngle azAngle, BoundaryAngle elevAngle) const
{
    const float az = (azAngle==MIN?_azMin:_azMax);
    const float elev = (elevAngle==MIN?_elevMin:_elevMax);

    bbox.expandBy(_centre);
    bbox.expandBy(  _centre.x() + _radius*cos(elev)*sin(az),
                    _centre.y() + _radius*cos(elev)*cos(az),
                    _centre.z() + _radius*sin(elev));

    return true;
}

void SphereSegment::setDrawMask(DrawMask dm)
{
    _drawMask=dm;
    dirtyAllDrawableDisplayLists();
    dirtyAllDrawableBounds();
    dirtyBound();
}

namespace{

struct ActivateTransparencyOnType
{
    ActivateTransparencyOnType(const std::type_info& t): _t(t) {}

    void operator()(osg::ref_ptr<osg::Drawable>& dptr) const
    {
        if(typeid(*dptr)==_t)
        {
            osg::StateSet* ss = dptr->getOrCreateStateSet();
            ss->setRenderingHint(osg::StateSet::TRANSPARENT_BIN);

            ss->setAttributeAndModes(new osg::CullFace(osg::CullFace::BACK),osg::StateAttribute::ON);
            ss->setMode(GL_BLEND,osg::StateAttribute::ON);

            dptr->dirtyDisplayList();
        }
    }

    const std::type_info&  _t;
};

struct DeactivateTransparencyOnType
{
    DeactivateTransparencyOnType(const std::type_info& t): _t(t) {}

    void operator()(osg::ref_ptr<osg::Drawable>& dptr) const
    {
        if(typeid(*dptr)==_t)
        {
            osg::StateSet* ss = dptr->getStateSet();
            if(ss) ss->setRenderingHint(osg::StateSet::OPAQUE_BIN);

            dptr->dirtyDisplayList();
        }
    }

    const std::type_info&  _t;
};

}

void SphereSegment::setSurfaceColor(const osg::Vec4& c)
{
    _surfaceColor=c;

    if(c.w() != 1.0) std::for_each(_drawables.begin(), _drawables.end(), ActivateTransparencyOnType(typeid(Surface)));
    else std::for_each(_drawables.begin(), _drawables.end(), DeactivateTransparencyOnType(typeid(Surface)));
}

void SphereSegment::setSpokeColor(const osg::Vec4& c)
{
    _spokeColor=c;

    if(c.w() != 1.0) std::for_each(_drawables.begin(), _drawables.end(), ActivateTransparencyOnType(typeid(Spoke)));
    else std::for_each(_drawables.begin(), _drawables.end(), DeactivateTransparencyOnType(typeid(Spoke)));
}

void SphereSegment::setEdgeLineColor(const osg::Vec4& c)
{
    _edgeLineColor=c;

    if(c.w() != 1.0) std::for_each(_drawables.begin(), _drawables.end(), ActivateTransparencyOnType(typeid(EdgeLine)));
    else std::for_each(_drawables.begin(), _drawables.end(), DeactivateTransparencyOnType(typeid(EdgeLine)));
}

void SphereSegment::setSideColor(const osg::Vec4& c)
{
    _planeColor=c;

    if(c.w() != 1.0) std::for_each(_drawables.begin(), _drawables.end(), ActivateTransparencyOnType(typeid(Side)));
    else std::for_each(_drawables.begin(), _drawables.end(), DeactivateTransparencyOnType(typeid(Side)));
}

void SphereSegment::setAllColors(const osg::Vec4& c)
{
    setSurfaceColor(c);
    setSpokeColor(c);
    setEdgeLineColor(c);
    setSideColor(c);
}
