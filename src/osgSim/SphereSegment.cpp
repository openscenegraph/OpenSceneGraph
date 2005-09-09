/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2005 Robert Osfield
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

#include <osgSim/SphereSegment>

#include <osg/Notify>
#include <osg/CullFace>
#include <osg/LineWidth>
#include <osg/Transform>
#include <osg/Geometry>
#include <osg/TriangleIndexFunctor>
#include <osg/io_utils>

#include <algorithm>

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

    virtual osg::BoundingBox computeBound() const;

protected:

private:

    SphereSegment* _ss;
};

void SphereSegment::Surface::drawImplementation(osg::State& state) const
{
    _ss->Surface_drawImplementation(state);
}

osg:: BoundingBox SphereSegment::Surface::computeBound() const
{
    osg:: BoundingBox bbox;
    _ss->Surface_computeBound(bbox);
    return bbox;
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


    virtual osg::BoundingBox computeBound() const;

private:

    SphereSegment* _ss;
};

void SphereSegment::EdgeLine::drawImplementation(osg::State& state) const
{
    _ss->EdgeLine_drawImplementation(state);
}

osg::BoundingBox SphereSegment::EdgeLine::computeBound() const
{
    osg::BoundingBox bbox;
    _ss->EdgeLine_computeBound(bbox);
    return bbox;
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

    virtual osg::BoundingBox computeBound() const;

private:
    SphereSegment* _ss;
    SphereSegment::SideOrientation _planeOrientation;
    SphereSegment::BoundaryAngle _BoundaryAngle;
};


void SphereSegment::Side::drawImplementation(osg::State& state) const
{
    _ss->Side_drawImplementation(state, _planeOrientation, _BoundaryAngle);
}

osg::BoundingBox SphereSegment::Side::computeBound() const
{
    osg::BoundingBox bbox;
    _ss->Side_computeBound(bbox, _planeOrientation, _BoundaryAngle);
    return bbox;
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
    
    virtual osg::BoundingBox computeBound() const;

private:
    SphereSegment* _ss;
    SphereSegment::BoundaryAngle _azAngle, _elevAngle;
};

void SphereSegment::Spoke::drawImplementation(osg::State& state) const
{
    _ss->Spoke_drawImplementation(state, _azAngle, _elevAngle);
}

osg::BoundingBox SphereSegment::Spoke::computeBound() const
{
    osg::BoundingBox bbox;
    _ss->Spoke_computeBound(bbox, _azAngle, _elevAngle);
    return bbox;
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

void SphereSegment::dirtyAllDrawableDisplayLists()
{
    for(DrawableList::iterator itr = _drawables.begin();
        itr != _drawables.end();
        ++itr)
    {
        (*itr)->dirtyDisplayList();
    }
}

void SphereSegment::dirtyAllDrawableBounds()
{
    for(DrawableList::iterator itr = _drawables.begin();
        itr != _drawables.end();
        ++itr)
    {
        (*itr)->dirtyBound();
    }
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
// SphereSegment interesection code.

class PolytopeVisitor : public osg::NodeVisitor
{
    public:
    
        typedef std::pair<osg::Matrix, osg::Polytope> MatrixPolytopePair;
        typedef std::vector<MatrixPolytopePair> PolytopeStack;
    
        struct Hit
        {
            Hit(const osg::Matrix& matrix, osg::NodePath& nodePath, osg::Drawable* drawable):
                _matrix(matrix),
                _nodePath(nodePath),
                _drawable(drawable) {}

            osg::Matrix                 _matrix;
            osg::NodePath               _nodePath;
            osg::ref_ptr<osg::Drawable> _drawable;
        };
    
        typedef std::vector<Hit> HitList;


        PolytopeVisitor(const osg::Matrix& matrix, const osg::Polytope& polytope):
            osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ACTIVE_CHILDREN)
        {
            _polytopeStack.push_back(MatrixPolytopePair());
            _polytopeStack.back().first = matrix;
            _polytopeStack.back().second.setAndTransformProvidingInverse(polytope, _polytopeStack.back().first);
        }
        
        void reset()
        {
            _polytopeStack.clear();
            _hits.clear();
        }
            
        void apply(osg::Node& node)
        {
            if (_polytopeStack.back().second.contains(node.getBound()))
            {
                traverse(node);
            }
        }

        void apply(osg::Transform& transform)
        {
            if (_polytopeStack.back().second.contains(transform.getBound()))
            {
                const osg::Polytope& polytope = _polytopeStack.front().second;

                osg::Matrix matrix = _polytopeStack.back().first;
                transform.computeLocalToWorldMatrix(matrix, this);
                
                _polytopeStack.push_back(MatrixPolytopePair());
                _polytopeStack.back().first = matrix;
                _polytopeStack.back().second.setAndTransformProvidingInverse(polytope, matrix);

                traverse(transform);
                
                _polytopeStack.back();
            }
        }

        void apply(osg::Geode& node)
        {
            if (_polytopeStack.back().second.contains(node.getBound()))
            {
                for(unsigned int i=0; i<node.getNumDrawables(); ++i)
                {
                    if (_polytopeStack.back().second.contains(node.getDrawable(i)->getBound()))
                    {
                        _hits.push_back(Hit(_polytopeStack.back().first,getNodePath(),node.getDrawable(i)));
                    }

                }
            
                traverse(node);
            }
        }

        HitList& getHits() { return _hits; }

    protected:
        
        PolytopeStack   _polytopeStack;
        HitList         _hits;

};


SphereSegment::LineList SphereSegment::computeIntersection(const osg::Matrixd& transform, osg::Node* subgraph)
{
    osg::notify(osg::NOTICE)<<"Creating line intersection between sphere segment and subgraph."<<std::endl;
    
    osg::BoundingBox bb = getBoundingBox();

    osg::Polytope polytope;
    polytope.add(osg::Plane(1.0,0.0,0.0,-bb.xMin())); 
    polytope.add(osg::Plane(-1.0,0.0,0.0,bb.xMax())); 
    polytope.add(osg::Plane(0.0,1.0,0.0,-bb.yMin())); 
    polytope.add(osg::Plane(0.0,-1.0,0.0,bb.yMax())); 
    polytope.add(osg::Plane(0.0,0.0,1.0,-bb.zMin())); 
    polytope.add(osg::Plane(0.0,0.0,-1.0,bb.zMax())); 
    
    osg::Plane pl;
    pl.set(osg::Vec3(-1.0,0.0,0.0), bb.corner(0));
    PolytopeVisitor polytopeVisitor(transform, polytope);
    
    subgraph->accept(polytopeVisitor);

    if (polytopeVisitor.getHits().empty())
    {
        osg::notify(osg::NOTICE)<<"No hits found."<<std::endl;
        return LineList();
    }

    // create a LineList to store all the compute line segments
    LineList all_lines;

    // compute the line intersections with each of the hit drawables
    osg::notify(osg::NOTICE)<<"Hits found. "<<polytopeVisitor.getHits().size()<<std::endl;
    PolytopeVisitor::HitList& hits = polytopeVisitor.getHits();
    for(PolytopeVisitor::HitList::iterator itr = hits.begin();
        itr != hits.end();
        ++itr)
    {
        SphereSegment::LineList lines = computeIntersection(itr->_matrix, itr->_drawable.get());
        all_lines.insert(all_lines.end(), lines.begin(), lines.end());
    }
    
    // join all the lines that have ends that are close together..

    return all_lines;
}

struct TriangleIntersectOperator
{

    TriangleIntersectOperator():
        _radius(-1.0f),
        _azMin(0.0f),
        _azMax(0.0f),
        _elevMin(0.0f),
        _elevMax(0.0f),
        _numOutside(0),
        _numInside(0),
        _numIntersecting(0) {}

    typedef osg::ref_ptr<osg::Vec3Array>    PositionArray;
    typedef std::vector<int>                RegionArray;
    
    PositionArray _positions;
    RegionArray   _regions;
    float         _radius; 
    float         _azMin, _azMax, _elevMin, _elevMax;
    
    unsigned int _numOutside;
    unsigned int _numInside;
    unsigned int _numIntersecting;

    inline void operator()(unsigned int p1, unsigned int p2, unsigned int p3)
    {
        // reject if outside.
        if (_regions[p1]==1 && _regions[p2]==1 && _regions[p3]==1)
        {
            ++_numOutside;
            return; 
        }
    
        if (_regions[p1]==-1 && _regions[p2]==-1 && _regions[p3]==-1)
        {
            ++_numInside;
            return; 
        }

        ++_numIntersecting;
    
    }

};


SphereSegment::LineList SphereSegment::computeIntersection(const osg::Matrixd& transform, osg::Drawable* drawable)
{
    // cast to Geometry, return empty handed if Drawable not a Geometry.
    osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(drawable);
    if (!geometry) return LineList();
    
    // get vertices from geometry, return empty handed if a Vec3Array not present.
    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
    if (!vertices) return LineList();
    
    typedef osg::TriangleIndexFunctor<TriangleIntersectOperator> TriangleIntersectFunctor;
    TriangleIntersectFunctor tif;
 
    tif._radius = _radius;
    tif._azMin = _azMin;
    tif._azMax = _azMax;
    tif._elevMin = _elevMin;
    tif._elevMax = _elevMax;

    tif._positions = vertices;
    tif._regions.resize(vertices->size(), 1);
 
    // traverse the triangles in the Geometry dedicating intersections
    geometry->accept(tif);
    
    osg::notify(osg::NOTICE)<<"_numOutside = "<<tif._numOutside<<std::endl;
    osg::notify(osg::NOTICE)<<"_numInside = "<<tif._numInside<<std::endl;
    osg::notify(osg::NOTICE)<<"_numIntersecting = "<<tif._numIntersecting<<std::endl;
 
    return LineList();
}
