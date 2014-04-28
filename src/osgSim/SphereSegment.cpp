/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
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
#include <osg/ShapeDrawable>
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
        OSG_WARN<<
            "Warning: unexpected call to osgSim::SphereSegment::Surface() default constructor"<<std::endl;
    }

    Surface(const Surface& rhs, const osg::CopyOp& co=osg::CopyOp::SHALLOW_COPY):osg::Drawable(rhs,co), _ss(0)
    {
        OSG_WARN<<
            "Warning: unexpected call to osgSim::SphereSegment::Surface() copy constructor"<<std::endl;
    }

    META_Object(osgSim,Surface)

    void drawImplementation(osg::RenderInfo& renderInfo) const;

    virtual osg::BoundingBox computeBound() const;

protected:

private:

    SphereSegment* _ss;
};

void SphereSegment::Surface::drawImplementation(osg::RenderInfo& renderInfo) const
{
    _ss->Surface_drawImplementation(*renderInfo.getState());
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
        OSG_WARN<<
            "Warning: unexpected call to osgSim::SphereSegment::EdgeLine() default constructor"<<std::endl;
    }

    EdgeLine(const EdgeLine& rhs, const osg::CopyOp& co=osg::CopyOp::SHALLOW_COPY):osg::Drawable(rhs,co), _ss(0)
    {
        OSG_WARN<<
            "Warning: unexpected call to osgSim::SphereSegment::EdgeLine() copy constructor"<<std::endl;
    }

    META_Object(osgSim,EdgeLine)

    void drawImplementation(osg::RenderInfo& renderInfo) const;

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

void SphereSegment::EdgeLine::drawImplementation(osg::RenderInfo& renderInfo) const
{
    _ss->EdgeLine_drawImplementation(*renderInfo.getState());
}

osg::BoundingBox SphereSegment::EdgeLine::computeBound() const
{
    osg::BoundingBox bbox;
    _ss->EdgeLine_computeBound(bbox);
    return bbox;
}



/**
SphereSegment::Side is a Drawable which represents one of the
planar areas, at either the minimum or maximum azimuth.
 */
class SphereSegment::Side: public osg::Drawable
{
public:
    Side(SphereSegment* ss, SphereSegment::SideOrientation po, SphereSegment::BoundaryAngle pa):
            osg::Drawable(), _ss(ss), _planeOrientation(po), _BoundaryAngle(pa) {}

    META_Object(osgSim,Side)

    void drawImplementation(osg::RenderInfo& renderInfo) const;

protected:

    Side():_ss(0), _planeOrientation(SphereSegment::AZIM), _BoundaryAngle(SphereSegment::MIN)
    {
        OSG_WARN<<
            "Warning: unexpected call to osgSim::SphereSegment::Side() default constructor"<<std::endl;
    }

    Side(const Side& rhs, const osg::CopyOp& co=osg:: CopyOp::SHALLOW_COPY):
        osg::Drawable(rhs,co),
        _ss(0),
        _planeOrientation(rhs._planeOrientation),
        _BoundaryAngle(rhs._BoundaryAngle)
    {
        OSG_WARN<<
            "Warning: unexpected call to osgSim::SphereSegment::Side() copy constructor"<<std::endl;
    }

    virtual osg::BoundingBox computeBound() const;

private:
    SphereSegment* _ss;
    SphereSegment::SideOrientation _planeOrientation;
    SphereSegment::BoundaryAngle _BoundaryAngle;
};


void SphereSegment::Side::drawImplementation(osg::RenderInfo& renderInfo) const
{
    _ss->Side_drawImplementation(*renderInfo.getState(), _planeOrientation, _BoundaryAngle);
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

    META_Object(osgSim,Spoke)

    void drawImplementation(osg::RenderInfo& renderInfo) const;

protected:

    Spoke():_ss(0)
    {
        init();
        OSG_WARN<<
            "Warning: unexpected call to osgSim::SphereSegment::Spoke() default constructor"<<std::endl;
    }

    Spoke(const Spoke& rhs, const osg::CopyOp& co=osg:: CopyOp::SHALLOW_COPY):
        osg::Drawable(rhs,co),
        _ss(0),
        _azAngle(rhs._azAngle), _elevAngle(rhs._elevAngle)
    {
        OSG_WARN<<
            "Warning: unexpected call to osgSim::SphereSegment::Spoke() copy constructor"<<std::endl;
    }


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

void SphereSegment::Spoke::drawImplementation(osg::RenderInfo& renderInfo) const
{
    _ss->Spoke_drawImplementation(*renderInfo.getState(), _azAngle, _elevAngle);
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
    float xyLen = sqrtf(vec.x()*vec.x() + vec.y()*vec.y());
    float elev = atan2(vec.z(), xyLen);   // Elevation angle

    elevRange /= 2.0f;
    _elevMin = elev - elevRange;
    _elevMax = elev + elevRange;

    // Calculate the azimuth range, cater for trig ambiguities
    float az = atan2(vec.x(), vec.y());

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

void SphereSegment::Surface_drawImplementation(osg::State& state) const
{
    const float azIncr = (_azMax - _azMin)/_density;
    const float elevIncr = (_elevMax - _elevMin)/_density;

    // Draw the area on the sphere surface if needed
    // ---------------------------------------------
    if(_drawMask & SURFACE)
    {
        osg::GLBeginEndAdapter& gl = (state.getGLBeginEndAdapter());

        gl.Color4fv(_surfaceColor.ptr());

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

                gl.Begin(GL_QUAD_STRIP);
                for (int j=0; j<=_density; j++)
                {
                    float elev = _elevMin + (j*elevIncr);

                    // QuadStrip Edge formed at az1
                    // ----------------------------

                    // Work out the sphere normal
                    float x = cos(elev)*sin(az1);
                    float y = cos(elev)*cos(az1);
                    float z = sin(elev);

                    gl.Normal3f(-x, -y, -z);
                    gl.Vertex3f(_centre.x() + _radius*x,
                            _centre.y() + _radius*y,
                            _centre.z() + _radius*z);

                    // QuadStrip Edge formed at az2
                    // ----------------------------

                    // Work out the sphere normal
                    x = cos(elev)*sin(az2);
                    y = cos(elev)*cos(az2);
                    // z = sin(elev);   z doesn't change

                    gl.Normal3f(-x, -y, -z);
                    gl.Vertex3f(_centre.x() + _radius*x,
                            _centre.y() + _radius*y,
                            _centre.z() + _radius*z);
                }
                gl.End();
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

                gl.Begin(GL_QUAD_STRIP);
                for (int j=0; j<=_density; j++)
                {
                    float elev = _elevMin + (j*elevIncr);

                    // QuadStrip Edge formed at az1
                    // ----------------------------

                    // Work out the sphere normal
                    float x = cos(elev)*sin(az2);
                    float y = cos(elev)*cos(az2);
                    float z = sin(elev);

                    gl.Normal3f(x, y, z);
                    gl.Vertex3f(_centre.x() + _radius*x,
                            _centre.y() + _radius*y,
                            _centre.z() + _radius*z);

                    // QuadStrip Edge formed at az2
                    // ----------------------------

                    // Work out the sphere normal
                    // z = sin(elev);   z doesn't change

                    x = cos(elev)*sin(az1);
                    y = cos(elev)*cos(az1);

                    gl.Normal3f(x, y, z);
                    gl.Vertex3f(_centre.x() + _radius*x,
                            _centre.y() + _radius*y,
                            _centre.z() + _radius*z);
                }
                gl.End();
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

void SphereSegment::EdgeLine_drawImplementation(osg::State& state) const
{
    const float azIncr = (_azMax - _azMin)/_density;
    const float elevIncr = (_elevMax - _elevMin)/_density;

    // Draw the edgeline if necessary
    // ------------------------------
    if(_drawMask & EDGELINE)
    {
        osg::GLBeginEndAdapter& gl = (state.getGLBeginEndAdapter());

        gl.Color4fv(_edgeLineColor.ptr());

        // Top edge
        gl.Begin(GL_LINE_STRIP);
        int i;
        for(i=0; i<=_density; i++)
        {
            float az = _azMin + (i*azIncr);
            gl.Vertex3f(
                _centre.x() + _radius*cos(_elevMax)*sin(az),
                _centre.y() + _radius*cos(_elevMax)*cos(az),
                _centre.z() + _radius*sin(_elevMax));
        }
        gl.End();

        // Bottom edge
        gl.Begin(GL_LINE_STRIP);
        for(i=0; i<=_density; i++)
        {
            float az = _azMin + (i*azIncr);
            gl.Vertex3f(
                _centre.x() + _radius*cos(_elevMin)*sin(az),
                _centre.y() + _radius*cos(_elevMin)*cos(az),
                _centre.z() + _radius*sin(_elevMin));
        }
        gl.End();

        // Left edge
        gl.Begin(GL_LINE_STRIP);
        int j;
        for(j=0; j<=_density; j++)
        {
            float elev = _elevMin + (j*elevIncr);
            gl.Vertex3f(
                _centre.x() + _radius*cos(elev)*sin(_azMin),
                _centre.y() + _radius*cos(elev)*cos(_azMin),
                _centre.z() + _radius*sin(elev));
        }
        gl.End();

        // Right edge
        gl.Begin(GL_LINE_STRIP);
        for(j=0; j<=_density; j++)
        {
            float elev = _elevMin + (j*elevIncr);
            gl.Vertex3f(
                _centre.x() + _radius*cos(elev)*sin(_azMax),
                _centre.y() + _radius*cos(elev)*cos(_azMax),
                _centre.z() + _radius*sin(elev));
        }
        gl.End();
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

void SphereSegment::Side_drawImplementation(osg::State& state,
                                SphereSegment::SideOrientation orientation,
                                SphereSegment::BoundaryAngle boundaryAngle) const
{
    // Draw the planes if necessary
    // ----------------------------
    if(_drawMask & SIDES)
    {
        osg::GLBeginEndAdapter& gl = (state.getGLBeginEndAdapter());

        bool drawBackSide = true;
        bool drawFrontSide = true;
        int start, end, delta;

        gl.Color4fv(_planeColor.ptr());

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
                    normal = -normal;   // Make sure normals oriented 'outwards'
                }
                delta = end>start?1:-1;

                if (drawBackSide)
                {
                    // Tri fan
                    gl.Normal3f(-normal.x(),-normal.y(),-normal.z());
                    gl.Begin(GL_TRIANGLE_FAN);
                    gl.Vertex3fv(_centre.ptr());
                    for (int j=start; j!=end+delta; j+=delta)
                    {
                        float elev = _elevMin + (j*elevIncr);
                        gl.Vertex3f( _centre.x() + _radius*cos(elev)*sin(az),
                                    _centre.y() + _radius*cos(elev)*cos(az),
                                    _centre.z() + _radius*sin(elev));
                    }
                    gl.End();
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
                    gl.Normal3fv(normal.ptr());
                    gl.Begin(GL_TRIANGLE_FAN);
                    gl.Vertex3fv(_centre.ptr());
                    for (int j=start; j!=end+delta; j+=delta)
                    {
                        float elev = _elevMin + (j*elevIncr);
                        gl.Vertex3f( _centre.x() + _radius*cos(elev)*sin(az),
                                    _centre.y() + _radius*cos(elev)*cos(az),
                                    _centre.z() + _radius*sin(elev));
                    }
                    gl.End();
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
                    normal = -normal;   // Make sure normals orientated 'outwards'
                }
                else
                {
                    start = 0;
                    end = _density;
                }
                delta = end>start?1:-1;

                if (drawBackSide)
                {
                    gl.Normal3f(-normal.x(),-normal.y(),-normal.z());

                    // Tri fan
                    gl.Begin(GL_TRIANGLE_FAN);
                    gl.Vertex3fv(_centre.ptr());
                    for (int j=start; j!=end+delta; j+=delta)
                    {
                        float az = _azMin + (j*azIncr);
                        gl.Vertex3f( _centre.x() + _radius*cos(elev)*sin(az),
                                    _centre.y() + _radius*cos(elev)*cos(az),
                                    _centre.z() + _radius*sin(elev));
                    }
                    gl.End();
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
                    gl.Normal3fv(normal.ptr());

                    // Tri fan
                    gl.Begin(GL_TRIANGLE_FAN);
                    gl.Vertex3fv(_centre.ptr());
                    for (int j=start; j!=end+delta; j+=delta)
                    {
                        float az = _azMin + (j*azIncr);
                        gl.Vertex3f( _centre.x() + _radius*cos(elev)*sin(az),
                                    _centre.y() + _radius*cos(elev)*cos(az),
                                    _centre.z() + _radius*sin(elev));
                    }
                    gl.End();
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

void SphereSegment::Spoke_drawImplementation(osg::State& state, BoundaryAngle azAngle, BoundaryAngle elevAngle) const
{
    if(_drawMask & SPOKES)
    {
        osg::GLBeginEndAdapter& gl = (state.getGLBeginEndAdapter());

        gl.Color4fv(_spokeColor.ptr());

        const float az = (azAngle==MIN?_azMin:_azMax);
        const float elev = (elevAngle==MIN?_elevMin:_elevMax);

        gl.Begin(GL_LINES);
            gl.Vertex3fv(_centre.ptr());
            gl.Vertex3f( _centre.x() + _radius*cos(elev)*sin(az),
                        _centre.y() + _radius*cos(elev)*cos(az),
                        _centre.z() + _radius*sin(elev));
        gl.End();
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

void SphereSegment::setDrawMask(int dm)
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

protected:

    ActivateTransparencyOnType& operator = (const ActivateTransparencyOnType&) { return *this; }
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

protected:

    DeactivateTransparencyOnType& operator = (const DeactivateTransparencyOnType&) { return *this; }
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
// SphereSegment intersection code.

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

        META_NodeVisitor("osgSim","PolytopeVisitor")

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
                osg::Matrix matrix = _polytopeStack.back().first;
                transform.computeLocalToWorldMatrix(matrix, this);

                _polytopeStack.push_back(MatrixPolytopePair());
                _polytopeStack.back().first = matrix;
                _polytopeStack.back().second.setAndTransformProvidingInverse(_polytopeStack.front().second, matrix);

                traverse(transform);

                _polytopeStack.pop_back();
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
    OSG_INFO<<"Creating line intersection between sphere segment and subgraph."<<std::endl;

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
        OSG_INFO<<"No hits found."<<std::endl;
        return LineList();
    }

    // create a LineList to store all the compute line segments
    LineList all_lines;

    // compute the line intersections with each of the hit drawables
    OSG_INFO<<"Hits found. "<<polytopeVisitor.getHits().size()<<std::endl;
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

osg::Node* SphereSegment::computeIntersectionSubgraph(const osg::Matrixd& transform, osg::Node* subgraph)
{
    OSG_INFO<<"Creating line intersection between sphere segment and subgraph."<<std::endl;

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
        OSG_INFO<<"No hits found."<<std::endl;
        return 0;
    }

    // create a LineList to store all the compute line segments
    osg::Group* group = new osg::Group;

    // compute the line intersections with each of the hit drawables
    OSG_INFO<<"Hits found. "<<polytopeVisitor.getHits().size()<<std::endl;
    PolytopeVisitor::HitList& hits = polytopeVisitor.getHits();
    for(PolytopeVisitor::HitList::iterator itr = hits.begin();
        itr != hits.end();
        ++itr)
    {
        group->addChild(computeIntersectionSubgraph(itr->_matrix, itr->_drawable.get()));
    }

    // join all the lines that have ends that are close together..

    return group;
}

namespace SphereSegmentIntersector
{

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

    protected:

        SortFunctor& operator = (const SortFunctor&) { return *this; }
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

        enum SurfaceType
        {
            NO_SURFACE = 0,
            RADIUS_SURFACE,
            LEFT_SURFACE,
            RIGHT_SURFACE,
            BOTTOM_SURFACE,
            TOP_SURFACE
        };

        struct Triangle;

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

            Edge(unsigned int p1, unsigned int p2, SurfaceType intersectionEdge=NO_SURFACE):
                _intersectionType(NO_INTERSECTION),
                _intersectionVertexIndex(0),
                _p1Outside(false),
                _p2Outside(false),
                _intersectionEdge(intersectionEdge)
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
                TriangleList::iterator itr = std::find(_triangles.begin(), _triangles.end(), tri);
                if (itr==_triangles.end()) _triangles.push_back(tri);
            }

            void removeFromToTraverseList(Triangle* tri)
            {
                TriangleList::iterator itr = std::find(_toTraverse.begin(), _toTraverse.end(), tri);
                if (itr!=_toTraverse.end()) _toTraverse.erase(itr);
            }


            bool completlyOutside() const { return _p1Outside && _p2Outside; }

            unsigned int _p1;
            unsigned int _p2;

            TriangleList _triangles;

            // intersection information
            IntersectionType    _intersectionType;
            osg::Vec3           _intersectionVertex;
            unsigned int        _intersectionVertexIndex;
            bool                _p1Outside;
            bool                _p2Outside;
            TriangleList        _toTraverse;
            SurfaceType         _intersectionEdge;
        };

        typedef std::list< osg::ref_ptr<Edge> >                     EdgeList;

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
                    OSG_INFO<<"Edge problem"<<std::endl;
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


        struct Region
        {
            enum Classification
            {
                INSIDE = -1,
                INTERSECTS = 0,
                OUTSIDE = 1
            };

            Region():
                _radiusSurface(OUTSIDE),
                _leftRightSurfaces(OUTSIDE),
                _leftSurface(OUTSIDE),
                _rightSurface(OUTSIDE),
                _bottomSurface(OUTSIDE),
                _topSurface(OUTSIDE) {}



            void classify(const osg::Vec3& vertex, double radius2, double azimMin, double azimMax, double elevMin, double elevMax)
            {
                double azimCenter = (azimMax+azimMin)*0.5;
                double azimRange = (azimMax-azimMin)*0.5;

                double rad2 = vertex.length2();
                double length_xy = sqrtf(vertex.x()*vertex.x() + vertex.y()*vertex.y());
                double elevation = atan2((double)vertex.z(),length_xy);

                // radius surface
                if      (rad2 > radius2) _radiusSurface = OUTSIDE;
                else if (rad2 < radius2) _radiusSurface = INSIDE;
                else                     _radiusSurface = INTERSECTS;

                // bottom surface
                if      (elevation<elevMin) _bottomSurface = OUTSIDE;
                else if (elevation>elevMin) _bottomSurface = INSIDE;
                else                        _bottomSurface = INTERSECTS;

                // top surface
                if      (elevation>elevMax) _topSurface = OUTSIDE;
                else if (elevation<elevMax) _topSurface = INSIDE;
                else                        _topSurface = INTERSECTS;


                double dot_alphaMin = cos(azimMin)*vertex.x() - sin(azimMin)*vertex.y();
                if (dot_alphaMin<0.0) _leftSurface = OUTSIDE;
                else if (dot_alphaMin>0.0) _leftSurface = INSIDE;
                else _leftSurface = INTERSECTS;

                double dot_alphaMax = cos(azimMax)*vertex.x() - sin(azimMax)*vertex.y();
                if (dot_alphaMax>0.0) _rightSurface = OUTSIDE;
                else if (dot_alphaMax<0.0) _rightSurface = INSIDE;
                else _rightSurface = INTERSECTS;

                double azim = atan2(vertex.x(),vertex.y());
                double azimDelta1 = azim-azimCenter;
                double azimDelta2 = 2.0*osg::PI + azim-azimCenter;
                double azimDelta = std::min(fabs(azimDelta1), fabs(azimDelta2));
                if (azimDelta>azimRange)
                {
                    _leftRightSurfaces = OUTSIDE;
                }
                else if (azimDelta<azimRange)
                {
                    _leftRightSurfaces = INSIDE;
                }
                else if (azimDelta==azimRange)
                {
                    _leftRightSurfaces = INTERSECTS;
                }
            }

            Classification _radiusSurface;
            Classification _leftRightSurfaces;
            Classification _leftSurface;
            Classification _rightSurface;
            Classification _bottomSurface;
            Classification _topSurface;
        };

        struct RegionCounter
        {
            RegionCounter():
                _numVertices(0),
                _outside_radiusSurface(0),
                _inside_radiusSurface(0),
                _intersects_radiusSurface(0),
                _outside_leftRightSurfaces(0),
                _inside_leftRightSurfaces(0),
                _intersects_leftRightSurfaces(0),
                _outside_leftSurface(0),
                _inside_leftSurface(0),
                _intersects_leftSurface(0),
                _outside_rightSurface(0),
                _inside_rightSurface(0),
                _intersects_rightSurface(0),
                _outside_bottomSurface(0),
                _inside_bottomSurface(0),
                _intersects_bottomSurface(0),
                _outside_topSurface(0),
                _inside_topSurface(0),
                _intersects_topSurface(0) {}


            void add(const Region& region)
            {
                ++_numVertices;

                if (region._radiusSurface == Region::OUTSIDE)       ++_outside_radiusSurface;
                if (region._radiusSurface == Region::INSIDE)        ++_inside_radiusSurface;
                if (region._radiusSurface == Region::INTERSECTS)    ++_intersects_radiusSurface;

                if (region._leftRightSurfaces == Region::OUTSIDE)    ++_outside_leftRightSurfaces;
                if (region._leftRightSurfaces == Region::INSIDE)     ++_inside_leftRightSurfaces;
                if (region._leftRightSurfaces == Region::INTERSECTS) ++_intersects_leftRightSurfaces;

                if (region._leftSurface == Region::OUTSIDE)         ++_outside_leftSurface;
                if (region._leftSurface == Region::INSIDE)          ++_inside_leftSurface;
                if (region._leftSurface == Region::INTERSECTS)      ++_intersects_leftSurface;

                if (region._rightSurface == Region::OUTSIDE)        ++_outside_rightSurface;
                if (region._rightSurface == Region::INSIDE)         ++_inside_rightSurface;
                if (region._rightSurface == Region::INTERSECTS)     ++_intersects_rightSurface;

                if (region._bottomSurface == Region::OUTSIDE)       ++_outside_bottomSurface;
                if (region._bottomSurface == Region::INSIDE)        ++_inside_bottomSurface;
                if (region._bottomSurface == Region::INTERSECTS)    ++_intersects_bottomSurface;

                if (region._topSurface == Region::OUTSIDE)          ++_outside_topSurface;
                if (region._topSurface == Region::INSIDE)           ++_inside_topSurface;
                if (region._topSurface == Region::INTERSECTS)       ++_intersects_topSurface;
            }

            Region::Classification overallClassification() const
            {
                // if all vertices are outside any of the surfaces then we are completely outside
                if (_outside_radiusSurface==_numVertices ||
                    _outside_leftRightSurfaces==_numVertices ||
                    _outside_topSurface==_numVertices ||
                    _outside_bottomSurface==_numVertices) return Region::OUTSIDE;

                // if all the vertices on all the sides and inside then we are completely inside
                if (_inside_radiusSurface==_numVertices &&
                    _inside_leftRightSurfaces==_numVertices &&
                    _inside_topSurface==_numVertices &&
                    _inside_bottomSurface==_numVertices) return Region::INSIDE;

                return Region::INTERSECTS;
            }

            bool intersecting(SurfaceType surfaceType) const
            {
                // if all vertices are outside any of the surfaces then we are completely outside
                if ((surfaceType!=RADIUS_SURFACE && _outside_radiusSurface!=0) ||
                    (surfaceType!=LEFT_SURFACE && _outside_leftSurface!=0) ||
                    (surfaceType!=RIGHT_SURFACE && _outside_rightSurface!=0) ||
                    (surfaceType!=TOP_SURFACE && _outside_topSurface!=0) ||
                    (surfaceType!=BOTTOM_SURFACE && _outside_bottomSurface!=0)) return false;

                // if all the vertices on all the sides and inside then we are completely inside
                if ((surfaceType!=RADIUS_SURFACE && _inside_radiusSurface!=0) &&
                    (surfaceType!=LEFT_SURFACE && _inside_leftSurface!=0) &&
                    (surfaceType!=RIGHT_SURFACE && _inside_rightSurface!=0) &&
                    (surfaceType!=TOP_SURFACE && _inside_topSurface!=0) &&
                    (surfaceType!=BOTTOM_SURFACE && _inside_bottomSurface!=0)) return false;

                return true;
            }

            int numberOfIntersectingSurfaces() const
            {
                int sidesThatIntersect = 0;
                if (_outside_radiusSurface!=_numVertices && _inside_radiusSurface!=_numVertices) ++sidesThatIntersect;
                if (_outside_leftSurface!=_numVertices && _inside_leftSurface!=_numVertices) ++sidesThatIntersect;
                if (_outside_rightSurface!=_numVertices && _inside_rightSurface!=_numVertices) ++sidesThatIntersect;
                if (_outside_topSurface!=_numVertices && _inside_topSurface!=_numVertices) ++sidesThatIntersect;
                if (_outside_bottomSurface!=_numVertices && _inside_bottomSurface!=_numVertices) ++sidesThatIntersect;
                return sidesThatIntersect;
            }

            unsigned int _numVertices;
            unsigned int _outside_radiusSurface;
            unsigned int _inside_radiusSurface;
            unsigned int _intersects_radiusSurface;

            unsigned int _outside_leftRightSurfaces;
            unsigned int _inside_leftRightSurfaces;
            unsigned int _intersects_leftRightSurfaces;

            unsigned int _outside_leftSurface;
            unsigned int _inside_leftSurface;
            unsigned int _intersects_leftSurface;

            unsigned int _outside_rightSurface;
            unsigned int _inside_rightSurface;
            unsigned int _intersects_rightSurface;

            unsigned int _outside_bottomSurface;
            unsigned int _inside_bottomSurface;
            unsigned int _intersects_bottomSurface;

            unsigned int _outside_topSurface;
            unsigned int _inside_topSurface;
            unsigned int _intersects_topSurface;

        };


        typedef std::vector< osg::Vec3 >                            VertexArray;
        typedef std::vector< Region >                               RegionArray;
        typedef std::vector< bool >                                 BoolArray;
        typedef std::vector< unsigned int >                         IndexArray;
        typedef std::vector< osg::ref_ptr<Triangle> >               TriangleArray;
        typedef std::set< osg::ref_ptr<Edge>, dereference_less >    EdgeSet;

        VertexArray     _originalVertices;
        RegionArray     _regions;
        BoolArray       _vertexInIntersectionSet;
        IndexArray      _candidateVertexIndices;
        IndexArray      _remapIndices;
        TriangleArray   _triangles;
        EdgeSet         _edges;

        osg::Vec3       _centre;
        double          _radius;
        double          _azMin, _azMax, _elevMin, _elevMax;

        unsigned int    _numOutside;
        unsigned int    _numInside;
        unsigned int    _numIntersecting;

        SphereSegment::LineList _generatedLines;

        void computePositionAndRegions(const osg::Matrixd& matrix, osg::Vec3Array& array)
        {
            _originalVertices.resize(array.size());
            _regions.resize(array.size());
            _vertexInIntersectionSet.resize(array.size(), false);
            _candidateVertexIndices.clear();

            double radius2 = _radius*_radius;

            for(unsigned int i=0; i<array.size(); ++i)
            {
                osg::Vec3 vertex = array[i]*matrix - _centre;
                _originalVertices[i] = vertex;
                _regions[i].classify(vertex, radius2, _azMin, _azMax, _elevMin, _elevMax);
            }

        }

        inline void operator()(unsigned int p1, unsigned int p2, unsigned int p3)
        {
            RegionCounter rc;
            rc.add(_regions[p1]);
            rc.add(_regions[p2]);
            rc.add(_regions[p3]);

            Region::Classification classification = rc.overallClassification();

            // reject if outside.
            if (classification==Region::OUTSIDE)
            {
                ++_numOutside;
                return;
            }

            if (rc.numberOfIntersectingSurfaces()==0)
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
            OSG_INFO<<"Removing duplicates : num vertices in "<<_candidateVertexIndices.size()<<std::endl;

            if (_candidateVertexIndices.size()<2) return;

            std::sort(_candidateVertexIndices.begin(), _candidateVertexIndices.end(), SortFunctor(_originalVertices));

            _remapIndices.resize(_originalVertices.size());
            for(unsigned int i=0; i< _originalVertices.size(); ++i)
            {
                _remapIndices[i] = i;
            }

            bool verticesRemapped = false;
            IndexArray::iterator itr = _candidateVertexIndices.begin();
            unsigned int lastUniqueIndex = *(itr++);
            for(; itr != _candidateVertexIndices.end(); ++itr)
            {
                //unsigned int i = *itr;
                // OSG_INFO<<"  i="<<i<<" lastUniqueIndex="<<lastUniqueIndex<<std::endl;
                if (_originalVertices[*itr]==_originalVertices[lastUniqueIndex])
                {
                    OSG_INFO<<"Combining vertex "<<*itr<<" with "<<lastUniqueIndex<<std::endl;
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
                OSG_INFO<<"Remapping triangle vertices "<<std::endl;
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
            OSG_INFO<<"Removing duplicate triangles : num triangles in "<<_triangles.size()<<std::endl;

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

            OSG_INFO<<"Removed duplicate triangles : num duplicates found "<<numDuplicates<<std::endl;
            OSG_INFO<<"Removed duplicate triangles : num triangles out "<<_triangles.size()<<std::endl;
        }

        void buildEdges(Triangle* tri)
        {
            tri->_e1 = addEdge(tri->_p1, tri->_p2, tri);
            tri->_e2 = addEdge(tri->_p2, tri->_p3, tri);
            tri->_e3 = addEdge(tri->_p1, tri->_p3, tri);
        }

        void buildEdges()
        {
            _edges.clear();
            for(TriangleArray::iterator itr = _triangles.begin();
                itr != _triangles.end();
                ++itr)
            {
                Triangle* tri = itr->get();

                RegionCounter rc;
                rc.add(_regions[tri->_p1]);
                rc.add(_regions[tri->_p2]);
                rc.add(_regions[tri->_p3]);
                int numIntersections = rc.numberOfIntersectingSurfaces();

                if (numIntersections>=1)
                {
                    buildEdges(tri);
                }

            }
            OSG_INFO<<"Number of edges "<<_edges.size()<<std::endl;

            unsigned int numZeroConnections = 0;
            unsigned int numSingleConnections = 0;
            unsigned int numDoubleConnections = 0;
            unsigned int numMultiConnections = 0;
            OSG_INFO<<"Number of edges "<<_edges.size()<<std::endl;
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

            OSG_INFO<<"Number of numZeroConnections "<<numZeroConnections<<std::endl;
            OSG_INFO<<"Number of numSingleConnections "<<numSingleConnections<<std::endl;
            OSG_INFO<<"Number of numDoubleConnections "<<numDoubleConnections<<std::endl;
            OSG_INFO<<"Number of numMultiConnections "<<numMultiConnections<<std::endl;
        }

        Edge* addEdge(unsigned int p1, unsigned int p2, Triangle* tri)
        {

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

        SphereSegment::LineList connectIntersections(EdgeList& hitEdges)
        {
            SphereSegment::LineList lineList;

            OSG_INFO<<"Number of edge intersections "<<hitEdges.size()<<std::endl;

            if (hitEdges.empty()) return lineList;

            // now need to build the toTraverse list for each hit edge,
            // but should only contain triangles that actually hit the intersection surface
            EdgeList::iterator hitr;
            for(hitr = hitEdges.begin();
                hitr != hitEdges.end();
                ++hitr)
            {
                Edge* edge = hitr->get();
                edge->_toTraverse.clear();
                //OSG_INFO<<"edge= "<<edge<<std::endl;
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
                        //OSG_INFO<<"   adding tri="<<tri<<std::endl;
                        edge->_toTraverse.push_back(tri);
                    }

                    // OSG_INFO<<"Number active edges "<<numActiveEdges<<" num original edges "<<numEdges<<std::endl;
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

                // OSG_INFO<<"New line "<<std::endl;


                osg::Vec3Array* newLine = new osg::Vec3Array;
                lineList.push_back(newLine);

                Edge* edge = hitr->get();
                while (edge)
                {
                    // OSG_INFO<<"   vertex "<<edge->_intersectionVertex<<std::endl;
                    newLine->push_back(edge->_intersectionVertex+_centre/*+osg::Vec3(0.0f,0.0f,200.0f)*/);

                    Edge* newEdge = 0;

                    Triangle* tri = !(edge->_toTraverse.empty()) ? edge->_toTraverse.back() : 0;
                    if (tri)
                    {

                        newEdge = tri->oppositeActiveEdge(edge);

                        edge->removeFromToTraverseList(tri);
                        newEdge->removeFromToTraverseList(tri);

                        // OSG_INFO<<"   tri="<<tri<<" edge="<<edge<<" newEdge="<<newEdge<<std::endl;

                        if (edge==newEdge)
                        {
                            OSG_INFO<<"   edge returned to itself problem "<<std::endl;
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
                        hitr = std::find(hitEdges.begin(), hitEdges.end(), edge);
                        if (hitr!=hitEdges.end()) hitEdges.erase(hitr);
                    }

                    // move on to next edge in line.
                    edge = newEdge;

                }
            }
            return lineList;
        }

        template<class I>
        SphereSegment::LineList computeIntersections(I intersector)
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

            return connectIntersections(hitEdges);
        }

        template<class I>
        void trim(SphereSegment::LineList& lineList, osg::Vec3Array* sourceLine, I intersector)
        {
            if (sourceLine->empty()) return;

            // OSG_INFO<<"Testing line of "<<sourceLine->size()<<std::endl;

            unsigned int first=0;
            while (first<sourceLine->size())
            {
                // find first valid vertex.
                for(; first<sourceLine->size(); ++first)
                {
                    if (intersector.distance((*sourceLine)[first]-_centre)>=0.0) break;
                }

                if (first==sourceLine->size())
                {
                    // OSG_INFO<<"No valid points found"<<std::endl;
                    return;
                }

                // find last valid vertex.
                unsigned int last = first+1;
                for(; last<sourceLine->size(); ++last)
                {
                    if (intersector.distance((*sourceLine)[last]-_centre)<0.0) break;
                }

                if (first==0 && last==sourceLine->size())
                {
                    // OSG_INFO<<"Copying complete line"<<std::endl;
                    lineList.push_back(sourceLine);
                }
                else
                {
                    // OSG_INFO<<"Copying partial line line"<<first<<" "<<last<<std::endl;

                    osg::Vec3Array* newLine = new osg::Vec3Array;

                    if (first>0)
                    {
                        newLine->push_back(intersector.intersectionPoint((*sourceLine)[first-1]-_centre, (*sourceLine)[first]-_centre)+_centre);
                    }

                    for(unsigned int i=first; i<last; ++i)
                    {
                        newLine->push_back((*sourceLine)[i]);
                    }
                    if (last<sourceLine->size())
                    {
                        newLine->push_back(intersector.intersectionPoint((*sourceLine)[last-1]-_centre, (*sourceLine)[last]-_centre)+_centre);
                    }

                    lineList.push_back(newLine);
                }

                first = last;
            }
        }


        // handle a paired of surfaces that work to enclose a convex region, which means that
        // points can be inside either surface to be valid, and be outside both surfaces to be invalid.
        template<class I>
        void trim(SphereSegment::LineList& lineList, osg::Vec3Array* sourceLine, I intersector1, I intersector2)
        {
            if (sourceLine->empty()) return;

            // OSG_INFO<<"Testing line of "<<sourceLine->size()<<std::endl;

            unsigned int first=0;
            while (first<sourceLine->size())
            {
                // find first valid vertex.
                for(; first<sourceLine->size(); ++first)
                {
                    osg::Vec3 v = (*sourceLine)[first]-_centre;
                    if (intersector1.distance(v)>=0.0 || intersector2.distance(v)>=0.0 ) break;
                }

                if (first==sourceLine->size())
                {
                    // OSG_INFO<<"No valid points found"<<std::endl;
                    return;
                }

                // find last valid vertex.
                unsigned int last = first+1;
                for(; last<sourceLine->size(); ++last)
                {
                    osg::Vec3 v = (*sourceLine)[last]-_centre;
                    if (intersector1.distance(v)<0.0 && intersector2.distance(v)<0.0 ) break;
                }

                if (first==0 && last==sourceLine->size())
                {
                    // OSG_INFO<<"Copying complete line"<<std::endl;
                    lineList.push_back(sourceLine);
                }
                else
                {
                    OSG_INFO<<"Copying partial line line"<<first<<" "<<last<<std::endl;

                    osg::Vec3Array* newLine = new osg::Vec3Array;

                    if (first>0)
                    {
                        osg::Vec3 start = (*sourceLine)[first-1]-_centre;
                        osg::Vec3 end = (*sourceLine)[first]-_centre;

                        float end1 = intersector1.distance(end);
                        float end2 = intersector2.distance(end);


                        // work out which intersector to use by discounting the one that
                        // isn't a plausible candidate.
                        bool possible1 = end1>=0.0;
                        bool possible2 = end2>=0.0;
                        if (possible1 && possible2)
                        {

                            double start1 = intersector1.distance(start);
                            double start2 = intersector2.distance(start);

                            // need to check which intersection is latest.
                            double end1 = intersector1.distance(end);
                            double delta1 = (end1-start1);

                            double end2 = intersector2.distance(end);
                            double delta2 = (end2-start2);

                            double r1 = fabs(delta1)>0.0 ? start1/delta1 : 0.0;
                            double r2 = fabs(delta2)>0.0 ? start2/delta2 : 0.0;

                            // choose intersection which is nearest the end point.
                            if (r1<r2)
                            {
                                OSG_INFO<<"start point, 1 near to end than 2"<<r1<<" "<<r2<<std::endl;
                                possible1 = true;
                                possible2 = false;
                            }
                            else
                            {
                                OSG_INFO<<"start point, 2 near to end than 1"<<std::endl;
                                possible1 = false;
                                possible2 = true;
                            }
                        }

                        if (possible1)
                        {
                            newLine->push_back(intersector1.intersectionPoint(start, end)+_centre);
                        }
                        else
                        {
                            newLine->push_back(intersector2.intersectionPoint(start, end)+_centre);
                        }

                    }

                    for(unsigned int i=first; i<last; ++i)
                    {
                        newLine->push_back((*sourceLine)[i]);
                    }

                    if (last<sourceLine->size())
                    {
                        osg::Vec3 start = (*sourceLine)[last-1]-_centre;
                        osg::Vec3 end = (*sourceLine)[last]-_centre;

                        double start1 = intersector1.distance(start);
                        double start2 = intersector2.distance(start);

                        // work out which intersector to use by discounting the one that
                        // isn't a plausible candidate.
                        bool possible1 = start1>=0.0;
                        bool possible2 = start2>=0.0;
                        if (possible1 && possible2)
                        {
                            double end1 = intersector1.distance(end);
                            double end2 = intersector2.distance(end);

                            possible1 = end1<0.0;
                            possible2 = end2<0.0;

                            if (possible1 && possible2)
                            {
                                // need to check which intersection is latest.
                                double end1 = intersector1.distance(end);
                                double delta1 = (end1-start1);

                                double end2 = intersector2.distance(end);
                                double delta2 = (end2-start2);

                                double r1 = fabs(delta1)>0.0 ? start1/delta1 : 0.0;
                                double r2 = fabs(delta2)>0.0 ? start2/delta2 : 0.0;

                                // choose intersection which is nearest the end point.
                                if (r1>r2)
                                {
                                    OSG_INFO<<"end point, 1 near to end than 2"<<r1<<" "<<r2<<std::endl;
                                    possible1 = true;
                                    possible2 = false;
                                }
                                else
                                {
                                    OSG_INFO<<"end point, 2 near to end than 1"<<std::endl;
                                    possible1 = false;
                                    possible2 = true;
                                }
                            }
                        }

                        if (possible1)
                        {
                            newLine->push_back(intersector1.intersectionPoint(start, end)+_centre);
                        }
                        else
                        {
                            newLine->push_back(intersector2.intersectionPoint(start, end)+_centre);
                        }

                    }

                    lineList.push_back(newLine);
                }

                first = last;
            }
        }

        template<class I>
        void trim(SphereSegment::LineList& lineList, I intersector)
        {
            SphereSegment::LineList newLines;

            // collect all the intersecting edges
            for(SphereSegment::LineList::iterator itr = lineList.begin();
                itr != lineList.end();
                ++itr)
            {
                osg::Vec3Array* line = itr->get();
                trim(newLines, line, intersector);
            }
            lineList.swap(newLines);
        }


        template<class I>
        void trim(SphereSegment::LineList& lineList, I intersector1, I intersector2)
        {
            SphereSegment::LineList newLines;

            // collect all the intersecting edges
            for(SphereSegment::LineList::iterator itr = lineList.begin();
                itr != lineList.end();
                ++itr)
            {
                osg::Vec3Array* line = itr->get();
                trim(newLines, line, intersector1, intersector2);
            }
            lineList.swap(newLines);
        }

        struct LinePair
        {
            LinePair(osg::Vec3Array* line):
                _line(line),
                _lineEnd(0),
                _neighbourLine(0),
                _neighbourLineEnd(0),
                _distance(FLT_MAX) {}

            bool operator < (const LinePair& linePair) const
            {
                return _distance < linePair._distance;
            }

            void consider(osg::Vec3Array* testline)
            {
                if (_neighbourLine.valid())
                {
                    float distance = ((*_line)[0]-(*testline)[0]).length();
                    if (distance<_distance)
                    {
                        _lineEnd = 0;
                        _neighbourLine = testline;
                        _neighbourLineEnd = 0;
                        _distance = distance;
                    }

                    distance = ((*_line)[0]-(*testline)[testline->size()-1]).length();
                    if (distance<_distance)
                    {
                        _lineEnd = 0;
                        _neighbourLine = testline;
                        _neighbourLineEnd = testline->size()-1;
                        _distance = distance;
                    }

                    distance = ((*_line)[_line->size()-1]-(*testline)[0]).length();
                    if (distance<_distance)
                    {
                        _lineEnd = _line->size()-1;
                        _neighbourLine = testline;
                        _neighbourLineEnd = 0;
                        _distance = distance;
                    }

                    distance = ((*_line)[_line->size()-1]-(*testline)[testline->size()-1]).length();
                    if (distance<_distance)
                    {
                        _lineEnd = _line->size()-1;
                        _neighbourLine = testline;
                        _neighbourLineEnd = testline->size()-1;
                        _distance = distance;
                    }

                }
                else
                {
                    _neighbourLine = testline;
                    if (_neighbourLine==_line)
                    {
                        _lineEnd = 0;
                        _neighbourLineEnd = _neighbourLine->size()-1;
                        _distance = ((*_line)[_lineEnd]-(*_neighbourLine)[_neighbourLineEnd]).length();
                    }
                    else
                    {
                        _distance = ((*_line)[0]-(*_neighbourLine)[0]).length();
                        _lineEnd = 0;
                        _neighbourLineEnd = 0;

                        float distance = ((*_line)[0]-(*_neighbourLine)[_neighbourLine->size()-1]).length();
                        if (distance<_distance)
                        {
                            _lineEnd = 0;
                            _neighbourLineEnd = _neighbourLine->size()-1;
                            _distance = distance;
                        }

                        distance = ((*_line)[_line->size()-1]-(*_neighbourLine)[0]).length();
                        if (distance<_distance)
                        {
                            _lineEnd = _line->size()-1;
                            _neighbourLineEnd = 0;
                            _distance = distance;
                        }

                        distance = ((*_line)[_line->size()-1]-(*_neighbourLine)[_neighbourLine->size()-1]).length();
                        if (distance<_distance)
                        {
                            _lineEnd = _line->size()-1;
                            _neighbourLineEnd = _neighbourLine->size()-1;
                            _distance = distance;
                        }
                    }
                }
            };

            bool contains(osg::Vec3Array* line) const
            {
                return _line==line || _neighbourLine==line;
            }


            osg::ref_ptr<osg::Vec3Array>    _line;
            unsigned int                    _lineEnd;
            osg::ref_ptr<osg::Vec3Array>    _neighbourLine;
            unsigned int                    _neighbourLineEnd;
            float                           _distance;
        };

        void joinEnds(float fuseDistance, bool doFuse, bool allowJoinToSelf)
        {
            SphereSegment::LineList fusedLines;
            SphereSegment::LineList unfusedLines;

            // first separate the already fused lines from the unfused ones.
            for(SphereSegment::LineList::iterator itr = _generatedLines.begin();
                itr != _generatedLines.end();
                ++itr)
            {
                osg::Vec3Array* line = itr->get();
                if (line->size()>=2)
                {
                    if ((*line)[0]==(*line)[line->size()-1])
                    {
                        fusedLines.push_back(line);
                    }
                    else
                    {
                        unfusedLines.push_back(line);
                    }
                }
            }

            while (unfusedLines.size()>=1)
            {
                // generate a set of line pairs to establish which
                // line pair has the minimum distance.
                typedef std::multiset<LinePair> LinePairSet;
                LinePairSet linePairs;
                for(unsigned int i=0; i<unfusedLines.size(); ++i)
                {
                    unsigned int j = allowJoinToSelf ? i : i+1;
                    if (j<unfusedLines.size())
                    {
                        LinePair linePair(unfusedLines[i].get());
                        for(; j<unfusedLines.size(); ++j)
                        {
                            linePair.consider(unfusedLines[j].get());
                        }
                        linePairs.insert(linePair);
                    }
                }

                if (linePairs.empty())
                {
                    OSG_INFO<<"Line Pairs empty"<<std::endl;
                    break;
                }

                for(LinePairSet::iterator itr = linePairs.begin();
                    itr != linePairs.end();
                    ++itr)
                {
                    OSG_INFO<<"Line "<<itr->_line.get()<<" "<<itr->_lineEnd<<"  neighbour "<<itr->_neighbourLine.get()<<" "<<itr->_neighbourLineEnd<<" distance="<<itr->_distance<<std::endl;
                }

                LinePair linePair = *linePairs.begin();
                if (linePair._distance > fuseDistance)
                {
                    OSG_INFO<<"Completed work, shortest distance left is "<<linePair._distance<<std::endl;
                    break;
                }

                if (linePair._line == linePair._neighbourLine)
                {
                    OSG_INFO<<"Fusing line to itself"<<std::endl;
                    osg::Vec3Array* line = linePair._line.get();
                    osg::Vec3 average = ((*line)[0]+(*line)[line->size()-1])*0.5f;

                    if (doFuse)
                    {
                        (*line)[0] = average;
                        (*line)[line->size()-1] = average;
                    }
                    else
                    {
                        // add start of line to end.
                        line->push_back((*line)[0]);
                    }

                    fusedLines.push_back(line);

                    SphereSegment::LineList::iterator fitr = std::find(unfusedLines.begin(), unfusedLines.end(), line);
                    if (fitr != unfusedLines.end())
                    {
                        unfusedLines.erase(fitr);
                    }
                    else
                    {
                        OSG_INFO<<"Error couldn't find line in unfused list, exiting fusing loop."<<std::endl;
                        break;
                    }
                }
                else
                {

                    osg::Vec3Array* line1 = linePair._line.get();
                    int fuseEnd1 =  linePair._lineEnd;
                    int openEnd1 = fuseEnd1==0 ? line1->size()-1 : 0;
                    int direction1 = openEnd1<fuseEnd1 ? 1 : -1;

                    osg::Vec3Array* line2 = linePair._neighbourLine.get();
                    int fuseEnd2 =  linePair._neighbourLineEnd;
                    int openEnd2 = fuseEnd2==0 ? line2->size()-1 : 0;
                    int direction2 = fuseEnd2<openEnd2 ? 1 : -1;

                    osg::Vec3Array* newline = new osg::Vec3Array;

                    // copy across all but fuse end of line1
                    for(int i=openEnd1;
                        i != fuseEnd1;
                        i += direction1)
                    {
                        newline->push_back((*line1)[i]);
                    }

                    // add the average of the two fused ends

                    if (doFuse)
                    {
                        osg::Vec3 average = ((*line1)[fuseEnd1] + (*line2)[fuseEnd2])*0.5f;
                        newline->push_back(average);
                    }
                    else
                    {
                        newline->push_back((*line1)[fuseEnd1]);
                        newline->push_back((*line2)[fuseEnd2]);
                    }

                    // copy across from the next point in from fuseEnd2 to the openEnd2.
                    for(int j=fuseEnd2 + direction2;
                        j != openEnd2 + direction2;
                        j += direction2)
                    {
                        newline->push_back((*line2)[j]);
                    }

                    // remove line1 from unfused list.
                    SphereSegment::LineList::iterator fitr = std::find(unfusedLines.begin(), unfusedLines.end(), line1);
                    if (fitr != unfusedLines.end())
                    {
                        unfusedLines.erase(fitr);
                    }

                    // remove line2 from unfused list.
                    fitr = std::find(unfusedLines.begin(), unfusedLines.end(), line2);
                    if (fitr != unfusedLines.end())
                    {
                        unfusedLines.erase(fitr);
                    }

                    // add the newline into the unfused for further processing.
                    unfusedLines.push_back(newline);

                    OSG_INFO<<"Fusing two separate lines "<<newline<<std::endl;
                }

                _generatedLines = fusedLines;
                _generatedLines.insert(_generatedLines.end(), unfusedLines.begin(), unfusedLines.end());

            }
        }

    };

    bool computeQuadraticSolution(double a, double b, double c, double& s1, double& s2)
    {
        // avoid division by zero.
        if (a==0.0)
        {
            s1 = 0.0;
            s2 = 0.0;
            return false;
        }

        double inside_sqrt = b*b - 4.0*a*c;

        // avoid sqrt of negative number
        if (inside_sqrt<0.0)
        {
            s1 = 0.0;
            s2 = 0.0;
            return false;
        }

        double rhs = sqrt(inside_sqrt);
        s1 = (-b + rhs)/(2.0*a);
        s2 = (-b - rhs)/(2.0*a);

        return true;
    }

    struct AzimPlaneIntersector
    {
        AzimPlaneIntersector(TriangleIntersectOperator& tif, double azim, bool lowerOutside):
            _tif(tif),
            _lowerOutside(lowerOutside)
        {
            _plane.set(cos(azim),-sin(azim),0.0,0.0);
            _endPlane.set(sin(azim),cos(azim),0.0,0.0);
        }

        TriangleIntersectOperator& _tif;
        osg::Plane _plane;
        osg::Plane _endPlane;
        bool _lowerOutside;

        inline bool operator() (TriangleIntersectOperator::Edge* edge)
        {
            edge->_intersectionType = TriangleIntersectOperator::Edge::NO_INTERSECTION;

            osg::Vec3& v1 = _tif._originalVertices[edge->_p1];
            osg::Vec3& v2 = _tif._originalVertices[edge->_p2];

            double d1 = _plane.distance(v1);
            double d2 = _plane.distance(v2);

            edge->_p1Outside = _lowerOutside ? (d1<0.0) : (d1>0.0);
            edge->_p2Outside = _lowerOutside ? (d2<0.0) : (d2>0.0);

            // if both points inside then discard
            if (d1<0.0 && d2<0.0) return false;

            // if both points outside then discard
            if (d1>0.0 && d2>0.0) return false;

            if (d1==0.0)
            {
                if (d2==0.0)
                {
                    edge->_intersectionType = TriangleIntersectOperator::Edge::BOTH_ENDS;
                }
                else
                {
                    edge->_intersectionType = TriangleIntersectOperator::Edge::POINT_1;
                }
            }
            else if (d2==0.0)
            {
                edge->_intersectionType = TriangleIntersectOperator::Edge::POINT_2;
            }
            else
            {

                double div = d2-d1;
                if (div==0.0)
                {
                    edge->_intersectionType = TriangleIntersectOperator::Edge::NO_INTERSECTION;
                    return false;
                }

                double r =  -d1 / div;
                if (r<0.0 || r>1.0)
                {
                    edge->_intersectionType = TriangleIntersectOperator::Edge::NO_INTERSECTION;
                    return false;
                }

                // OSG_INFO<<"r = "<<r<<std::endl;

                double one_minus_r = 1.0-r;

                edge->_intersectionType = TriangleIntersectOperator::Edge::MID_POINT;
                edge->_intersectionVertex = v1*one_minus_r + v2*r;
            }

            return true;
        }

        // compute the intersection between line segment and surface
        osg::Vec3 intersectionPoint(const osg::Vec3& v1, const osg::Vec3& v2)
        {
            double d1 = _plane.distance(v1);
            double d2 = _plane.distance(v2);

            double div = d2-d1;
            if (div==0.0)
            {
                return v1;
            }

            double r =  -d1 / div;
            double one_minus_r = 1.0-r;

            return v1*one_minus_r + v2*r;
        }

        // positive distance to the inside.
        double distance(const osg::Vec3& v)
        {
            return _lowerOutside ? _plane.distance(v) : -_plane.distance(v) ;
        }

    protected:

        AzimPlaneIntersector& operator = (const AzimPlaneIntersector&) { return *this; }
    };

    struct ElevationIntersector
    {
        ElevationIntersector(TriangleIntersectOperator& tif, double elev, bool lowerOutside):
            _tif(tif),
            _elev(elev),
            _lowerOutside(lowerOutside) {}

        TriangleIntersectOperator& _tif;
        double _elev;
        bool _lowerOutside;

        inline bool operator() (TriangleIntersectOperator::Edge* edge)
        {
            edge->_intersectionType = TriangleIntersectOperator::Edge::NO_INTERSECTION;

            osg::Vec3& v1 = _tif._originalVertices[edge->_p1];
            osg::Vec3& v2 = _tif._originalVertices[edge->_p2];

            double length_xy1 = sqrt(v1.x()*v1.x() + v1.y()*v1.y());
            double elev1 = atan2((double)v1.z(),length_xy1);

            double length_xy2 = sqrt(v2.x()*v2.x() + v2.y()*v2.y());
            double elev2 = atan2((double)v2.z(),length_xy2);

            edge->_p1Outside = _lowerOutside ? (elev1<_elev) : (elev1>_elev);
            edge->_p2Outside = _lowerOutside ? (elev2<_elev) : (elev2>_elev);

            // if both points inside then discard
            if (elev1<_elev && elev2<_elev) return false;

            // if both points outside then discard
            if (elev1>_elev && elev2>_elev) return false;

            if (elev1==_elev)
            {
                if (elev2==_elev)
                {
                    edge->_intersectionType = TriangleIntersectOperator::Edge::BOTH_ENDS;
                }
                else
                {
                    edge->_intersectionType = TriangleIntersectOperator::Edge::POINT_1;
                }
            }
            else if (elev2==_elev)
            {
                edge->_intersectionType = TriangleIntersectOperator::Edge::POINT_2;
            }
            else
            {
                double dx = v2.x()-v1.x();
                double dy = v2.y()-v1.y();
                double dz = v2.z()-v1.z();
                double t = tan(_elev);
                double tt = t*t;
                double a = dz*dz-tt*(dx*dx + dy*dy);
                double b = 2.0*(v1.z()*dz - tt*(v1.x()*dx + v1.y()*dy));
                double c = v1.z()*v1.z() - tt*(v1.x()*v1.x() + v1.y()*v1.y());

                double s1, s2;
                if (!computeQuadraticSolution(a,b,c,s1,s2))
                {
                    edge->_intersectionType = TriangleIntersectOperator::Edge::NO_INTERSECTION;
                    return false;
                }
                double r = 0.0;
                if (s1>=0.0 && s1<=1.0)
                {
                    r = s1;
                }
                else if (s2>=0.0 && s2<=1.0)
                {
                    r = s2;
                }
                else
                {
                    OSG_INFO<<"neither segment intersects s1="<<s1<<" s2="<<s2<<std::endl;

                    edge->_intersectionType = TriangleIntersectOperator::Edge::NO_INTERSECTION;
                    return false;
                }

                double one_minus_r = 1.0-r;
                edge->_intersectionType = TriangleIntersectOperator::Edge::MID_POINT;
                edge->_intersectionVertex = v1*one_minus_r + v2*r;
            }

            return true;
        }

        // compute the intersection between line segment and surface
        osg::Vec3 intersectionPoint(const osg::Vec3& v1, const osg::Vec3& v2)
        {
            double dx = v2.x()-v1.x();
            double dy = v2.y()-v1.y();
            double dz = v2.z()-v1.z();
            double t = tan(_elev);
            double tt = t*t;
            double a = dz*dz-tt*(dx*dx + dy*dy);
            double b = 2.0*(v1.z()*dz - tt*(v1.x()*dx + v1.y()*dy));
            double c = v1.z()*v1.z() - tt*(v1.x()*v1.x() + v1.y()*v1.y());

            double s1, s2;
            if (!computeQuadraticSolution(a,b,c,s1,s2))
            {
                OSG_INFO<<"Warning::neither segment intersects s1="<<s1<<" s2="<<s2<<std::endl;
                return v1;
            }
            double r = 0.0;
            if (s1>=0.0 && s1<=1.0)
            {
                r = s1;
            }
            else if (s2>=0.0 && s2<=1.0)
            {
                r = s2;
            }
            else
            {
                OSG_INFO<<"Warning::neither segment intersects s1="<<s1<<" s2="<<s2<<std::endl;
                return v1;
            }

            double one_minus_r = 1.0-r;
            return v1*one_minus_r + v2*r;
        }

        // positive distance to the inside.
        double distance(const osg::Vec3& v)
        {
            double length_xy = sqrt(v.x()*v.x() + v.y()*v.y());
            double computedElev = atan2((double)v.z(),length_xy);

            return _lowerOutside ? computedElev-_elev : _elev-computedElev ;
        }

    protected:

        ElevationIntersector& operator = (const ElevationIntersector&) { return *this; }

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
            double radius1 = v1.length();
            double radius2 = v2.length();

            edge->_p1Outside = radius1>_tif._radius;
            edge->_p2Outside = radius2>_tif._radius;

            // if both points inside then discard
            if (radius1<_tif._radius && radius2<_tif._radius) return false;

            // if both points outside then discard
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
                double dx = v2.x()-v1.x();
                double dy = v2.y()-v1.y();
                double dz = v2.z()-v1.z();
                double a = dx*dx + dy*dy + dz*dz;
                double b = 2.0*(v1.x()*dx + v1.y()*dy + v1.z()*dz);
                double c = v1.x()*v1.x() +  v1.y()*v1.y() + v1.z()*v1.z() - _tif._radius*_tif._radius;

                double s1, s2;
                if (!computeQuadraticSolution(a,b,c,s1,s2))
                {
                    edge->_intersectionType = TriangleIntersectOperator::Edge::NO_INTERSECTION;
                    return false;
                }
                double r = 0.0;
                if (s1>=0.0 && s1<=1.0)
                {
                    r = s1;
                }
                else if (s2>=0.0 && s2<=1.0)
                {
                    r = s2;
                }
                else
                {
                    OSG_INFO<<"neither segment intersects s1="<<s1<<" s2="<<s2<<std::endl;

                    edge->_intersectionType = TriangleIntersectOperator::Edge::NO_INTERSECTION;
                    return false;
                }

                double one_minus_r = 1.0-r;
                edge->_intersectionType = TriangleIntersectOperator::Edge::MID_POINT;
                edge->_intersectionVertex = v1*one_minus_r + v2*r;

            }

            return true;
        }

        // compute the intersection between line segment and surface
        osg::Vec3 intersectionPoint(const osg::Vec3& v1, const osg::Vec3& v2)
        {
            double dx = v2.x()-v1.x();
            double dy = v2.y()-v1.y();
            double dz = v2.z()-v1.z();
            double a = dx*dx + dy*dy + dz*dz;
            double b = 2.0*(v1.x()*dx + v1.y()*dy + v1.z()*dz);
            double c = v1.x()*v1.x() +  v1.y()*v1.y() + v1.z()*v1.z() - _tif._radius*_tif._radius;

            double s1, s2;
            if (!computeQuadraticSolution(a,b,c,s1,s2))
            {
                OSG_INFO<<"Warning: neither segment intersects s1="<<s1<<" s2="<<s2<<std::endl;
                return v1;
            }
            double r = 0.0;
            if (s1>=0.0 && s1<=1.0)
            {
                r = s1;
            }
            else if (s2>=0.0 && s2<=1.0)
            {
                r = s2;
            }
            else
            {
                OSG_INFO<<"Warning: neither segment intersects s1="<<s1<<" s2="<<s2<<std::endl;
                return v1;
            }

            double one_minus_r = 1.0-r;
            return v1*one_minus_r + v2*r;

        }

        // positive distance to the inside.
        double distance(const osg::Vec3& v)
        {
            return _tif._radius-v.length();
        }


    protected:

        RadiusIntersector& operator = (const RadiusIntersector&) { return *this; }

    };
}

using namespace SphereSegmentIntersector;

SphereSegment::LineList SphereSegment::computeIntersection(const osg::Matrixd& matrix, osg::Drawable* drawable)
{
    // cast to Geometry, return empty handed if Drawable not a Geometry.
    osg::Geometry* geometry = dynamic_cast<osg::Geometry*>(drawable);
    if (!geometry) return SphereSegment::LineList();

    // get vertices from geometry, return empty handed if a Vec3Array not present.
    osg::Vec3Array* vertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
    if (!vertices) return SphereSegment::LineList();

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

    OSG_INFO<<"_numOutside = "<<tif._numOutside<<std::endl;
    OSG_INFO<<"_numInside = "<<tif._numInside<<std::endl;
    OSG_INFO<<"_numIntersecting = "<<tif._numIntersecting<<std::endl;

    tif.removeDuplicateVertices();
    tif.removeDuplicateTriangles();
    tif.buildEdges();


    RadiusIntersector radiusIntersector(tif);
    AzimPlaneIntersector azMinIntersector(tif,_azMin, true);
    AzimPlaneIntersector azMinEndIntersector(tif,_azMin-osg::PI*0.5, true);
    AzimPlaneIntersector azMaxIntersector(tif,_azMax, false);
    AzimPlaneIntersector azMaxEndIntersector(tif,_azMax-osg::PI*0.5, true);
    ElevationIntersector elevMinIntersector(tif,_elevMin, true);
    ElevationIntersector elevMaxIntersector(tif,_elevMax, false);

    // create the line intersections with the terrain
    SphereSegment::LineList radiusLines = tif.computeIntersections(radiusIntersector);
    SphereSegment::LineList elevMinLines = tif.computeIntersections(elevMinIntersector);
    SphereSegment::LineList elevMaxLines = tif.computeIntersections(elevMaxIntersector);
    SphereSegment::LineList azMinLines;
    SphereSegment::LineList azMaxLines;

    double azimRange = _azMax-_azMin;
    if (azimRange<2.0*osg::PI)
    {
        azMinLines = tif.computeIntersections(azMinIntersector);
        azMaxLines = tif.computeIntersections(azMaxIntersector);

        // trim the azimuth intersection lines by the radius
        tif.trim(azMinLines,radiusIntersector);
        tif.trim(azMaxLines,radiusIntersector);

        // trim the azim intersection lines by the elevation
        tif.trim(azMinLines, elevMinIntersector);
        tif.trim(azMaxLines, elevMinIntersector);

        // trim the azim intersection lines by the elevation
        tif.trim(azMinLines, elevMaxIntersector);
        tif.trim(azMaxLines, elevMaxIntersector);

        // trim the centeral ends of the azim lines
        tif.trim(azMinLines,azMinEndIntersector);
        tif.trim(azMaxLines,azMaxEndIntersector);

        if (azimRange<=osg::PI)
        {
            // trim the radius and elevation intersection lines by the azimMin
            tif.trim(radiusLines, azMinIntersector);
            tif.trim(elevMinLines, azMinIntersector);
            tif.trim(elevMaxLines, azMinIntersector);

            // trim the radius and elevation intersection lines by the azimMax
            tif.trim(radiusLines, azMaxIntersector);
            tif.trim(elevMinLines, azMaxIntersector);
            tif.trim(elevMaxLines, azMaxIntersector);
        }
        else
        {
            // need to have new intersector which handles convex azim planes
            tif.trim(radiusLines, azMinIntersector, azMaxIntersector);
            tif.trim(elevMinLines, azMinIntersector, azMaxIntersector);
            tif.trim(elevMaxLines, azMinIntersector, azMaxIntersector);
        }

    }

    // trim elevation intersection lines by radius
    tif.trim(elevMinLines,radiusIntersector);
    tif.trim(elevMaxLines,radiusIntersector);

    // trim the radius and elevation intersection lines by the elevMin
    tif.trim(radiusLines, elevMinIntersector);

    // trim the radius and elevation intersection lines by the elevMax
    tif.trim(radiusLines, elevMaxIntersector);

    // collect all lines together.
    tif._generatedLines.insert(tif._generatedLines.end(), radiusLines.begin(), radiusLines.end());
    tif._generatedLines.insert(tif._generatedLines.end(), azMinLines.begin(), azMinLines.end());
    tif._generatedLines.insert(tif._generatedLines.end(), azMaxLines.begin(), azMaxLines.end());
    tif._generatedLines.insert(tif._generatedLines.end(), elevMinLines.begin(), elevMinLines.end());
    tif._generatedLines.insert(tif._generatedLines.end(), elevMaxLines.begin(), elevMaxLines.end());

    OSG_INFO<<"number of separate lines = "<<tif._generatedLines.size()<<std::endl;

    float fuseDistance = 1.0;
    tif.joinEnds(fuseDistance, true, true);

    OSG_INFO<<"number of separate lines after fuse = "<<tif._generatedLines.size()<<std::endl;

    float joinDistance = 1e8;
    tif.joinEnds(joinDistance, false, false);
    OSG_INFO<<"number of separate lines after join = "<<tif._generatedLines.size()<<std::endl;

    tif.joinEnds(joinDistance, false, true);
    OSG_INFO<<"number of separate lines after second join = "<<tif._generatedLines.size()<<std::endl;

    return tif._generatedLines;
}

osg::Node* SphereSegment::computeIntersectionSubgraph(const osg::Matrixd& matrix, osg::Drawable* drawable)
{
    SphereSegment::LineList generatedLines = computeIntersection(matrix, drawable);

    osg::Geode* geode = new osg::Geode;

    geode->getOrCreateStateSet()->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    for(osgSim::SphereSegment::LineList::iterator itr = generatedLines.begin();
       itr != generatedLines.end();
       ++itr)
    {
        osg::Geometry* geom = new osg::Geometry;
        geode->addDrawable(geom);

        osg::Vec3Array* vertices = itr->get();
        geom->setVertexArray(vertices);
        geom->addPrimitiveSet(new osg::DrawArrays(GL_LINE_STRIP, 0, vertices->getNumElements()));
    }

#if 0
    float radius = 0.1f;
    for(unsigned int i=0; i<tif._originalVertices.size(); ++i)
    {
        osg::ShapeDrawable* sd = new osg::ShapeDrawable(new osg::Sphere(tif._originalVertices[i]+tif._centre,radius));

        TriangleIntersectFunctor::RegionCounter rc;
        rc.add(tif._regions[i]);

        TriangleIntersectFunctor::Region::Classification region = rc.overallClassification();
        if (region==TriangleIntersectFunctor::Region::OUTSIDE)
        {
            sd->setColor(osg::Vec4(1.0,0.0,0.0,1.0));
        }
        else if (region==TriangleIntersectFunctor::Region::INSIDE)
        {
            sd->setColor(osg::Vec4(1.0,1.0,0.0,1.0));
        }
        else if (region==TriangleIntersectFunctor::Region::INTERSECTS)
        {
            sd->setColor(osg::Vec4(1.0,1.0,1.0,1.0));
        }

        geode->addDrawable(sd);
    }
#endif

    return geode;
}
