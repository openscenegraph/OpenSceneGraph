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
#include <list>

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


struct dereference_less
{
    template<class T, class U>
    inline bool operator() (const T& lhs,const U& rhs) const
    {
        return *lhs < *rhs;
    }
};

struct SortFunctor
{
    typedef std::vector< osg::Vec3 > VertexArray;
    
    SortFunctor(VertexArray& vertices):
        _vertices(vertices) {}

    bool operator() (unsigned int p1, unsigned int p2) const
    {
        return _vertices[p1]<_vertices[p2];
    }
    
    VertexArray& _vertices;
};


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

    class Triangle;

    struct Edge : public osg::Referenced
    {
        typedef std::vector<Triangle*> TriangleList;
        
        enum IntersectionType
        {
            NO_INTERSECTION,
            POINT_1,
            POINT_2,
            MID_POINT,
            BOTH_ENDS
        };

        Edge(unsigned int p1, unsigned int p2)
        {
            if (p1>p2)
            {
                _p1 = p2;
                _p2 = p1;
            }
            else
            {
                _p1 = p1;
                _p2 = p2;
            }
        }
    
        bool operator < (const Edge& edge) const
        {
            if (_p1<edge._p1) return true;
            else if (_p1>edge._p1) return false;
            else return _p2<edge._p2;
        }
    
        inline void addTriangle(Triangle* tri)
        {
            TriangleList::iterator itr = find(_triangles.begin(), _triangles.end(), tri);
            if (itr==_triangles.end()) _triangles.push_back(tri);
        }
    
        void removeFromToTraverseList(Triangle* tri)
        {
            TriangleList::iterator itr = find(_toTraverse.begin(), _toTraverse.end(), tri);
            if (itr!=_toTraverse.end()) _toTraverse.erase(itr);
        }

        unsigned int _p1;
        unsigned int _p2;
        
        TriangleList _triangles;

        // intersection information        
        IntersectionType    _intersectionType;
        osg::Vec3           _intersectionVertex;
        TriangleList        _toTraverse;
    };

    struct Triangle : public osg::Referenced
    {
    
        Triangle(unsigned int p1, unsigned int p2, unsigned int p3):
            _p1(p1), _p2(p2), _p3(p3),
            _e1(0), _e2(0), _e3(0)
        {
            sort();
        }

        bool operator < (const Triangle& rhs) const
        {
            if (_p1 < rhs._p1) return true;
            else if (_p1 > rhs._p1) return false;
            else if (_p2 < rhs._p2) return true;
            else if (_p2 > rhs._p2) return false;
            else return (_p3 < rhs._p3);
        }
        
        bool operator == (const Triangle& rhs) const
        {
            return (_p1 == rhs._p1) && (_p2 != rhs._p2) && (_p3 != rhs._p3);
        }

        bool operator != (const Triangle& rhs) const
        {
            return (_p1 != rhs._p1) || (_p2 != rhs._p2) || (_p3 != rhs._p3);
        }

        void sort()
        {
            if (_p1>_p2) std::swap(_p1,_p2);
            if (_p1>_p3) std::swap(_p1,_p3);
            if (_p2>_p3) std::swap(_p2,_p3);
        }
        
        Edge* oppositeActiveEdge(Edge* edge)
        {
            if (edge!=_e1 &&  edge!=_e2 &&  edge!=_e3)
            {
                osg::notify(osg::NOTICE)<<"Edge problem"<<std::endl;
                return 0;
            }
        
            if (edge!=_e1 && _e1 && _e1->_intersectionType!=Edge::NO_INTERSECTION) return _e1;
            if (edge!=_e2 && _e2 && _e2->_intersectionType!=Edge::NO_INTERSECTION) return _e2;
            if (edge!=_e3 && _e3 && _e3->_intersectionType!=Edge::NO_INTERSECTION) return _e3;
            return 0;
        }


        unsigned int _p1;
        unsigned int _p2;
        unsigned int _p3;

        Edge* _e1;
        Edge* _e2;
        Edge* _e3;
    };

    typedef std::vector< osg::Vec3 >                            VertexArray;
    typedef std::vector< int >                                  RegionArray;
    typedef std::vector< bool >                                 BoolArray;
    typedef std::vector< unsigned int >                         IndexArray;
    typedef std::vector< osg::ref_ptr<Triangle> >               TriangleArray;
    typedef std::set< osg::ref_ptr<Edge>, dereference_less >    EdgeSet;
    typedef std::list< osg::ref_ptr<Edge> >                     EdgeList;
    
    VertexArray     _originalVertices;
    RegionArray     _regions;
    BoolArray       _vertexInIntersectionSet;
    IndexArray      _candidateVertexIndices;
    IndexArray      _remapIndices;
    TriangleArray   _triangles;
    EdgeSet         _edges;
    
    osg::Vec3       _centre;
    float           _radius; 
    float           _azMin, _azMax, _elevMin, _elevMax;
    
    unsigned int    _numOutside;
    unsigned int    _numInside;
    unsigned int    _numIntersecting;
    
    SphereSegment::LineList _generatedLines;
    
    void computePositionAndRegions(const osg::Matrixd& matrix, osg::Vec3Array& array)
    {
        _originalVertices.resize(array.size());
        _regions.resize(array.size(), 1);
        _vertexInIntersectionSet.resize(array.size(), false);
        _candidateVertexIndices.clear();
        
        float radius2 = _radius*_radius;
        
        for(unsigned int i=0; i<array.size(); ++i)
        {
            osg::Vec3 vertex = array[i]*matrix - _centre;
            _originalVertices[i] = vertex;
            float rad2 = vertex.length2();
            if (rad2 > radius2) 
            {
                _regions[i] = 1;
            }
            else
            {
                _regions[i] = -1;
#if 0                
                float length_xy = sqrtf(vertex.x()*vertex.x() + vertex.y()*vertex.y());
                float elevation = atan2(vertex.z(),length_xy);
                if (elevation<_elevMin || elevation>_elevMax)
                {
                    _regions[i] = 1;
                }
                else
                {
                    float azim = atan2(vertex.x(),vertex.y());
                    if (azim<0.0) azim += 2.0f*osg::PI;
                    if (azim<_azMin || azim>_azMax)
                    {
                        _regions[i] = 1;
                    }
                    else
                    {
                        if (rad2==radius2 || elevation==_elevMin || elevation==_elevMax || azim==_azMin || azim==_azMax)
                        {
                            _regions[i] = 0;
                        }
                        else
                        {
                            _regions[i] = -1;
                        }
                    
                    }
                }
#endif                
                
            }
        }
        
    }

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
        
        _triangles.push_back(new Triangle(p1,p2,p3));
        
        if (!_vertexInIntersectionSet[p1])
        {
            _vertexInIntersectionSet[p1] = true;
            _candidateVertexIndices.push_back(p1);
        }
        
        if (!_vertexInIntersectionSet[p2])
        {
            _vertexInIntersectionSet[p2] = true;
            _candidateVertexIndices.push_back(p2);
        }
        
        if (!_vertexInIntersectionSet[p3])
        {
            _vertexInIntersectionSet[p3] = true;
            _candidateVertexIndices.push_back(p3);
        }
    
    }
    
    void removeDuplicateVertices()
    {
        osg::notify(osg::NOTICE)<<"Removing duplicates : num vertices in "<<_candidateVertexIndices.size()<<std::endl;
    
        if (_candidateVertexIndices.size()<2) return;

        std::sort(_candidateVertexIndices.begin(), _candidateVertexIndices.end(), SortFunctor(_originalVertices));

        _remapIndices.resize(_originalVertices.size());
        for(unsigned int i=0; i< _originalVertices.size(); ++i)
        {
            _remapIndices[i] = i;
        }
        
        bool verticesRemapped = false;
        IndexArray::iterator itr = _candidateVertexIndices.begin();
        unsigned int lastUniqueIndex = _candidateVertexIndices[*(itr++)];
        for(; itr != _candidateVertexIndices.end(); ++itr)
        {
            if (_originalVertices[*itr]==_originalVertices[lastUniqueIndex])
            {
                osg::notify(osg::NOTICE)<<"Combining vertex "<<*itr<<" with "<<lastUniqueIndex<<std::endl;
                _remapIndices[*itr] = lastUniqueIndex;
                verticesRemapped = true;
            }
            else
            {
                lastUniqueIndex = *itr;
            }
        }
        
        if (verticesRemapped)
        {
            osg::notify(osg::NOTICE)<<"Remapping triangle vertices "<<std::endl;
            for(TriangleArray::iterator titr = _triangles.begin();
                titr != _triangles.end();
                ++titr)
            {
                (*titr)->_p1 = _remapIndices[(*titr)->_p1];
                (*titr)->_p2 = _remapIndices[(*titr)->_p2];
                (*titr)->_p3 = _remapIndices[(*titr)->_p3];
                (*titr)->sort();
            }
        }
    }

    void removeDuplicateTriangles()
    {
        osg::notify(osg::NOTICE)<<"Removing duplicate triangles : num triangles in "<<_triangles.size()<<std::endl;
    
        if (_triangles.size()<2) return;
        
        std::sort(_triangles.begin(), _triangles.end(), dereference_less());

        unsigned int lastUniqueTriangle = 0;
        unsigned int numDuplicates = 0;
        for(unsigned int i=1; i<_triangles.size(); ++i)
        {
            if ( *(_triangles[lastUniqueTriangle]) != *(_triangles[i]) )
            {
                ++lastUniqueTriangle;
                if (lastUniqueTriangle!=i)
                {
                    _triangles[lastUniqueTriangle] = _triangles[i];
                }
            }
            else
            {
                ++numDuplicates;
            }
        }
        if (lastUniqueTriangle<_triangles.size()-1)
        {
            _triangles.erase(_triangles.begin()+lastUniqueTriangle+1, _triangles.end());
        }
        
        osg::notify(osg::NOTICE)<<"Removed duplicate triangles : num duplicates found "<<numDuplicates<<std::endl;
        osg::notify(osg::NOTICE)<<"Removed duplicate triangles : num triangles out "<<_triangles.size()<<std::endl;
    }
    
    void buildEdges()
    {
        _edges.clear();
        for(TriangleArray::iterator itr = _triangles.begin();
            itr != _triangles.end();
            ++itr)
        {
            Triangle* tri = itr->get();
            tri->_e1 = addEdge(tri->_p1, tri->_p2, tri);
            tri->_e2 = addEdge(tri->_p2, tri->_p3, tri);
            tri->_e3 = addEdge(tri->_p1, tri->_p3, tri);
        }
        osg::notify(osg::NOTICE)<<"Number of edges "<<_edges.size()<<std::endl;
        
        unsigned int numZeroConnections = 0;
        unsigned int numSingleConnections = 0;
        unsigned int numDoubleConnections = 0;
        unsigned int numMultiConnections = 0;
        osg::notify(osg::NOTICE)<<"Number of edges "<<_edges.size()<<std::endl;
        for(EdgeSet::iterator eitr = _edges.begin();
            eitr != _edges.end();
            ++eitr)
        {
            const Edge* edge = eitr->get();
            unsigned int numConnections = edge->_triangles.size();
            if (numConnections==0) ++numZeroConnections;
            else if (numConnections==1) ++numSingleConnections;
            else if (numConnections==2) ++numDoubleConnections;
            else ++numMultiConnections;
        }

        osg::notify(osg::NOTICE)<<"Number of numZeroConnections "<<numZeroConnections<<std::endl;
        osg::notify(osg::NOTICE)<<"Number of numSingleConnections "<<numSingleConnections<<std::endl;
        osg::notify(osg::NOTICE)<<"Number of numDoubleConnections "<<numDoubleConnections<<std::endl;
        osg::notify(osg::NOTICE)<<"Number of numMultiConnections "<<numMultiConnections<<std::endl;
    }
    
    Edge* addEdge(unsigned int p1, unsigned int p2, Triangle* tri)
    {
        if ((_regions[p1]==_regions[p2]) && _regions[p1]!=0) return 0;
    
        osg::ref_ptr<Edge> edge = new Edge(p1, p2);
        EdgeSet::iterator itr = _edges.find(edge);
        if (itr==_edges.end())
        {
            edge->addTriangle(tri);            
            _edges.insert(edge);
            return edge.get();
        }
        else
        {
            Edge* edge = const_cast<Edge*>(itr->get());
            edge->addTriangle(tri);
            return edge;
        }
    }
    
    template<class I>
    void computeIntersections(I intersector)
    {
        // collect all the intersecting edges
        EdgeList hitEdges;
        for(EdgeSet::iterator itr = _edges.begin();
            itr != _edges.end();
            ++itr)
        {
            Edge* edge = const_cast<Edge*>(itr->get());
            if (intersector(edge))
            {
                hitEdges.push_back(edge);
            }
        }
        osg::notify(osg::NOTICE)<<"Number of edge intersections "<<hitEdges.size()<<std::endl;
        
        if (hitEdges.empty()) return;
        
        // now need to build the toTraverse list for each hit edge,
        // but should only contain traingles that actually hit the intersection surface
        EdgeList::iterator hitr;
        for(hitr = hitEdges.begin();
            hitr != hitEdges.end();
            ++hitr)
        {
            Edge* edge = hitr->get();
            edge->_toTraverse.clear();
            osg::notify(osg::NOTICE)<<"edge= "<<edge<<std::endl;
            for(Edge::TriangleList::iterator titr = edge->_triangles.begin();
                titr != edge->_triangles.end();
                ++titr)
            {
                Triangle* tri = *titr;
                
                // count how many active edges there are on this triangle
                unsigned int numActiveEdges = 0;
                unsigned int numEdges = 0;
                if (tri->_e1 && tri->_e1->_intersectionType!=Edge::NO_INTERSECTION) ++numActiveEdges;
                if (tri->_e2 && tri->_e2->_intersectionType!=Edge::NO_INTERSECTION) ++numActiveEdges;
                if (tri->_e3 && tri->_e3->_intersectionType!=Edge::NO_INTERSECTION) ++numActiveEdges;

                if (tri->_e1) ++numEdges;
                if (tri->_e2) ++numEdges;
                if (tri->_e3) ++numEdges;
                
                // if we have one or more then add it into the edges to traverse list
                if (numActiveEdges>1)
                {
                    osg::notify(osg::NOTICE)<<"   adding tri="<<tri<<std::endl;
                    edge->_toTraverse.push_back(tri);
                }
                
                // osg::notify(osg::NOTICE)<<"Number active edges "<<numActiveEdges<<" num orignal edges "<<numEdges<<std::endl;
            }
        }
        
        for(hitr = hitEdges.begin();
            hitr != hitEdges.end();
            ++hitr)
        {
            Edge* edge = hitr->get();
            osg::notify(osg::NOTICE)<<"edge= "<<edge<<std::endl;
            for(Edge::TriangleList::iterator titr = edge->_toTraverse.begin();
                titr != edge->_toTraverse.end();
                ++titr)
            {
                Triangle* tri = *titr;
                osg::notify(osg::NOTICE)<<"   "<<tri<<std::endl;
            }
        }
        
        while(!hitEdges.empty())
        {
            // find the an open edge
            for(hitr = hitEdges.begin();
                hitr != hitEdges.end();
                ++hitr)
            {
                Edge* edge = hitr->get();
                if (edge->_toTraverse.size()==1) break;
            }

            if (hitr == hitEdges.end())
            {
                hitr = hitEdges.begin();
            }
        
            osg::notify(osg::NOTICE)<<"New line "<<std::endl;

            
            osg::Vec3Array* newLine = new osg::Vec3Array;
            _generatedLines.push_back(newLine);

            Edge* edge = hitr->get();
            while (edge)
            {
                osg::notify(osg::NOTICE)<<"   vertex "<<edge->_intersectionVertex<<std::endl;
                newLine->push_back(edge->_intersectionVertex+_centre/*+osg::Vec3(0.0f,0.0f,200.0f)*/);
            
                Edge* newEdge = 0;
                
                Triangle* tri = !(edge->_toTraverse.empty()) ? edge->_toTraverse.back() : 0;
                if (tri)
                {

                    newEdge = tri->oppositeActiveEdge(edge);
                    
                    edge->removeFromToTraverseList(tri);
                    newEdge->removeFromToTraverseList(tri);

                    osg::notify(osg::NOTICE)<<"   tri="<<tri<<" edge="<<edge<<" newEdge="<<newEdge<<std::endl;

                    if (edge==newEdge)
                    {
                        osg::notify(osg::NOTICE)<<"   edge returned to itself problem "<<std::endl;
                    }
                }
                else
                {
                    newEdge = 0;
                }
                
                if (edge->_toTraverse.empty())
                {
                    edge->_intersectionType = Edge::NO_INTERSECTION;
                    
                    // remove edge for the hitEdges.
                    hitr = find(hitEdges.begin(), hitEdges.end(), edge);
                    if (hitr!=hitEdges.end()) hitEdges.erase(hitr);
                }

                // move on to next edge in line.
                edge = newEdge;
                
            }
        
        }
    }
};

struct RadiusIntersector
{
    RadiusIntersector(TriangleIntersectOperator& tif):
        _tif(tif) {}
        
    TriangleIntersectOperator& _tif;

    inline bool operator() (TriangleIntersectOperator::Edge* edge)
    {
        edge->_intersectionType = TriangleIntersectOperator::Edge::NO_INTERSECTION;

        osg::Vec3& v1 = _tif._originalVertices[edge->_p1];
        osg::Vec3& v2 = _tif._originalVertices[edge->_p2];
        float radius1 = v1.length();
        float radius2 = v2.length();

        // if both points inside then disgard
        if (radius1<_tif._radius && radius2<_tif._radius) return false;
        
        // if both points outside then disgard
        if (radius1>_tif._radius && radius2>_tif._radius) return false;
        
        if (radius1==_tif._radius)
        {
            if (radius2==_tif._radius)
            {
                edge->_intersectionType = TriangleIntersectOperator::Edge::BOTH_ENDS;
            }
            else
            {
                edge->_intersectionType = TriangleIntersectOperator::Edge::POINT_1;
            }
        }
        else if (radius2==_tif._radius)
        {
            edge->_intersectionType = TriangleIntersectOperator::Edge::POINT_2;
        }
        else
        {
            edge->_intersectionType = TriangleIntersectOperator::Edge::MID_POINT;
            float r = (_tif._radius-radius1)/(radius2-radius1);
            float one_minus_r = 1.0f-r;
            edge->_intersectionVertex = v1*one_minus_r + v2*r;
        }
        
        return true;
    }
};

SphereSegment::LineList SphereSegment::computeIntersection(const osg::Matrixd& matrix, osg::Drawable* drawable)
{
    // cast to Geometry, return empty handed if Drawable not a Geometry.
    osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(drawable);
    if (!geometry) return LineList();
    
    // get vertices from geometry, return empty handed if a Vec3Array not present.
    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
    if (!vertices) return LineList();
    
    typedef osg::TriangleIndexFunctor<TriangleIntersectOperator> TriangleIntersectFunctor;
    TriangleIntersectFunctor tif;
 
    tif._centre = _centre;
    tif._radius = _radius;
    tif._azMin = _azMin;
    tif._azMax = _azMax;
    tif._elevMin = _elevMin;
    tif._elevMax = _elevMax;

    tif.computePositionAndRegions(matrix, *vertices);
 
    // traverse the triangles in the Geometry dedicating intersections
    geometry->accept(tif);
    
    osg::notify(osg::NOTICE)<<"_numOutside = "<<tif._numOutside<<std::endl;
    osg::notify(osg::NOTICE)<<"_numInside = "<<tif._numInside<<std::endl;
    osg::notify(osg::NOTICE)<<"_numIntersecting = "<<tif._numIntersecting<<std::endl;

    tif.removeDuplicateVertices();
    tif.removeDuplicateTriangles();
    tif.buildEdges();

    tif.computeIntersections(RadiusIntersector(tif));
 
    return tif._generatedLines;
}
