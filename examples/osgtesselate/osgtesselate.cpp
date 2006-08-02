/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield 
 *
 * This application is open source and may be redistributed and/or modified   
 * freely and without restriction, both in commericial and non commericial applications,
 * as long as this copyright notice is maintained.
 * 
 * This application is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/* osgtesselator
 * - this tesselator is an extension of the basic one - rather than tesselating
 * individual polygons, we tesselate the entire geode with multiple contours.
 * allows for overlapping contours etc.
 * the tesselator has new member fuinctions
 setTesselationType(osgUtil::Tesselator::TESS_TYPE_xxx);
    tscx->setBoundaryOnly(bool);
    tscx->setWindingType( osgUtil::Tesselator::TESS_WINDING_xxx); 
 * for winding rules: See the red book chap 13.
 */

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgProducer/Viewer>
#include <osg/Projection>
#include <osg/MatrixTransform>

#include <osgText/Text>

#include <osgUtil/Tesselator> // to tesselate multiple contours


class tesselateDemoGeometry : public osg::Geometry, public osgUtil::Tesselator {
    // We add the Tesselator to the geometry because we want to access the
    // tesselatable contours again;  you can apply a tesselator to a Geometry 
    // to produce exactly a required tesselation once only, and then 
    // the contours could be discarded since the geometry does not need to be retesselated.
public:
    tesselateDemoGeometry() {};

protected:
    virtual ~tesselateDemoGeometry() {};
};

osg::Geometry *makePolsTwo (void) 
{
    // an example of using current geometry contours to create next tesselation
    // this polygon disappears once the contour rules make no polygons.
    tesselateDemoGeometry *gtess= new tesselateDemoGeometry;
    int i;
    osg::Vec3Array *coords = new osg::Vec3Array;
    osg::Vec3Array *nrms = new osg::Vec3Array;
    osg::Vec2Array *tcs = new osg::Vec2Array;
    osg::Vec3 nrm(0,-1,0);
    static GLdouble quadstrip[8][3] =
    { { 1900.0, 1130.0, 0.0 },
      { 2100.0, 1130.0, 0.0 }, 
      { 1900.0, 1350.0, 0.0 },
      { 1950.0, 1350.0, 0.0 }, 
      { 1900.0, 1550.0, 0.0 }, 
      { 2000.0, 1550.0, 0.0 }, 
      { 1900.0, 1750.0, 0.0 }, 
      { 2400.0, 1750.0, 0.0 } };
    static GLdouble innerquadstrip[8][3] =
    { { 2000.0, 1230.0, 0.0 },
      { 2050.0, 1230.0, 0.0 },
      { 1920.0, 1350.0, 0.0 },
      { 1940.0, 1350.0, 0.0 },
      { 1920.0, 1550.0, 0.0 },
      { 1980.0, 1550.0, 0.0 }, 
      { 2000.0, 1650.0, 0.0 },
      { 2400.0, 1650.0, 0.0 } };
    
    // add one large quadstrip 
    for (i = 0; i < 8; i++)
    {
        coords->push_back(osg::Vec3(quadstrip[i][0],quadstrip[i][2],quadstrip[i][1]));
        tcs->push_back(osg::Vec2(quadstrip[i][0],quadstrip[i][1])/200.0);
        nrms->push_back(nrm);
    }
    for (i = 0; i < 8; i++) {
        coords->push_back(osg::Vec3(innerquadstrip[i][0],innerquadstrip[i][2],innerquadstrip[i][1]));
        tcs->push_back(osg::Vec2(innerquadstrip[i][0],innerquadstrip[i][1])/200.0);
        nrms->push_back(nrm);
    }
    gtess->setVertexArray(coords);
    gtess->setNormalArray(nrms);
    gtess->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    gtess->setTexCoordArray(0,tcs);
    
    // demonstrate that the tesselator makes textured tesselations
    osg::StateSet* stateset = new osg::StateSet();
    
    osg::Image* image = osgDB::readImageFile("Cubemap_snow/posy.jpg");
    if (image)
    {
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);
        texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
        texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
        stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    }
    gtess->setStateSet( stateset );
    
    int nstart=0;
    // The derived class tesselateDemoGeometry retains the original contours for re-use.
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP,nstart,8));nstart+=8;
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP,nstart,8));nstart+=8;

    gtess->setTesselationType(osgUtil::Tesselator::TESS_TYPE_GEOMETRY);
    gtess->setBoundaryOnly(true);
    gtess->setWindingType( osgUtil::Tesselator::TESS_WINDING_ABS_GEQ_TWO); // so that first change in wind type makes the commonest tesselation - ODD.
    
    return gtess;
}

osg::Geometry *makeSideWall (const float xpos)
{
    // demonstrate making a rectangular 'wall' with 2 holes in it.
    osg::Geometry *gtess= new osg::Geometry;
    int i;
    osg::Vec3Array *coords = new osg::Vec3Array;
    osg::Vec3Array *nrms = new osg::Vec3Array;
    osg::Vec2Array *tcs = new osg::Vec2Array;
    osg::Vec3 nrm(-1,0,0);
    // front wall
    static GLdouble wall[4][2] =
    { { 1130.0, 0.0 },
      { 1130.0, 300.0 } , 
      { 1340.0,300.0 },
      { 1340.0,0.0 } };

    gtess->setVertexArray(coords);
    gtess->setNormalArray(nrms);
    gtess->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    gtess->setTexCoordArray(0,tcs);

    for (i = 0; i < 4; i++) {
        coords->push_back(osg::Vec3(xpos,wall[i][1],wall[i][0]));
        tcs->push_back(osg::Vec2(wall[i][1],wall[i][0])/100.0);
        nrms->push_back(nrm);
    }
    int nstart=0;

    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,nstart,4));nstart+=4;
    for (i = 0; i < 24; i++) { // make an ellipse hole
        float y=150+50*cos(i*2*osg::PI/24.0);
        float z=1300+30* sin(i*2*osg::PI/24.0);
        coords->push_back(osg::Vec3(xpos,y,z));
        tcs->push_back(osg::Vec2(y,z)/100.0);
        nrms->push_back(nrm);
    }
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,nstart,24));nstart+=24;
    for (i = 0; i < 5; i++) { // make a pentagonal hole
        float y=150+50*cos(i*2*osg::PI/5.0);
        float z=1200+40* sin(i*2*osg::PI/5.0);
        coords->push_back(osg::Vec3(xpos,y,z));
        tcs->push_back(osg::Vec2(y,z)/100.0);
        nrms->push_back(nrm);
    }
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,nstart,5));nstart+=5;
    
    // demonstrate that the tesselator makes textured tesselations
    osg::StateSet* stateset = new osg::StateSet();
    
    osg::Image* image = osgDB::readImageFile("Cubemap_snow/posx.jpg");
    if (image)
    {
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);
        texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
        texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
        stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    }
    gtess->setStateSet( stateset );
    

    osg::ref_ptr<osgUtil::Tesselator> tscx=new osgUtil::Tesselator; // the v1.2 multi-contour tesselator.
    // we use the geometry primitives to describe the contours which are tesselated.
    // Winding odd means leave hole in surface where there are 2,4,6... contours circling the point.
    tscx->setTesselationType(osgUtil::Tesselator::TESS_TYPE_GEOMETRY);
    tscx->setBoundaryOnly(false);
    tscx->setWindingType( osgUtil::Tesselator::TESS_WINDING_ODD); // so that first change in wind type makes the commonest tesselation - ODD.
    
    tscx->retesselatePolygons(*gtess);
    
    return gtess;
}

osg::Geometry *makeFrontWall (const float zpos) {
    // an example of using one tesselation to make a 'house' wall
    // describe the wall as a pentagon, then door & 4 windows are further contours
    // tesselate the set of contours to make a 'house wall' from the Boolean-like operations.
    int nstart=0; // counts vertices used for the geometry primitives

    osg::Geometry *gtess= new osg::Geometry;
    int i;
    osg::Vec3Array *coords = new osg::Vec3Array;
    osg::Vec3Array *nrms = new osg::Vec3Array;
    osg::Vec2Array *tcs = new osg::Vec2Array;
    osg::Vec3 nrm(0,-1,0);
    // front wall
    static GLdouble wall[5][2] =
    { { 2200.0, 1130.0 },
      { 2600.0, 1130.0 },  
      { 2600.0, 1340.0 }, 
      { 2400.0, 1440.0 },
      { 2200.0, 1340.0 } };

    static GLdouble door[4][2] =
    { { 2360.0, 1130.0 },
      { 2440.0, 1130.0 }, 
      { 2440.0, 1230.0 },
      { 2360.0, 1230.0 } };

    static GLdouble windows[16][2] =
    { { 2240.0, 1180.0 },
      { 2330.0, 1180.0 }, 
      { 2330.0, 1220.0 },
      { 2240.0, 1220.0 }, 
      { 2460.0, 1180.0 },
      { 2560.0, 1180.0 }, 
      { 2560.0, 1220.0 },
      { 2460.0, 1220.0 },
      { 2240.0, 1280.0 },
      { 2330.0, 1280.0 }, 
      { 2330.0, 1320.0 },
      { 2240.0, 1320.0 }, 
      { 2460.0, 1280.0 },
      { 2560.0, 1280.0 }, 
      { 2560.0, 1320.0 },
      { 2460.0, 1320.0 } };

    gtess->setVertexArray(coords);
    gtess->setNormalArray(nrms);
    gtess->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    gtess->setTexCoordArray(0,tcs);

    // add one large pentagon -the wall 
    for (i = 0; i < 5; i++) {
        coords->push_back(osg::Vec3(wall[i][0],zpos,wall[i][1]));
        tcs->push_back(osg::Vec2(wall[i][0],wall[i][1])/100.0);
        nrms->push_back(nrm);
    }
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,nstart,5));nstart+=5;
    // add first hole, a door 
    for (i = 0; i < 4; i++) {
        coords->push_back(osg::Vec3(door[i][0],zpos,door[i][1]));
        tcs->push_back(osg::Vec2(door[i][0],door[i][1])/100.0);
        nrms->push_back(nrm);
    }
    // and windows
    for (i = 0; i < 16; i++) {
        coords->push_back(osg::Vec3(windows[i][0],zpos,windows[i][1]));
        tcs->push_back(osg::Vec2(windows[i][0],windows[i][1])/100.0);
        nrms->push_back(nrm);
    }
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,nstart,20));nstart+=20;

    // demonstrate that the tesselator makes textured tesselations
    osg::StateSet* stateset = new osg::StateSet();
    
    osg::Image* image = osgDB::readImageFile("Cubemap_snow/posy.jpg");
    if (image)
    {
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);
        texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
        texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
        stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    }
    gtess->setStateSet( stateset );
    
    // We use a Tesselator to produce the tesselation required once only 
    // and the contours are discarded.
    osg::ref_ptr<osgUtil::Tesselator> tscx=new osgUtil::Tesselator; // the v1.2 multi-contour tesselator.
    tscx->setTesselationType(osgUtil::Tesselator::TESS_TYPE_GEOMETRY);
    tscx->setBoundaryOnly(false);
    tscx->setWindingType( osgUtil::Tesselator::TESS_WINDING_ODD); // so that first change in wind type makes the commonest tesselation - ODD.
    
    tscx->retesselatePolygons(*gtess);
    
    return gtess;
}
osg::Geode *makeHouse (void) {
    osg::Geode *gd = new osg::Geode;
    gd->addDrawable(makeFrontWall(0.0));
    gd->addDrawable(makeFrontWall(300.0));
    gd->addDrawable(makeSideWall(2200.0));
    gd->addDrawable(makeSideWall(2600.0));
    return gd;
}
osg::Geometry *makePols (void) {
    tesselateDemoGeometry *gtess= new tesselateDemoGeometry;
    int i;
    osg::Vec3Array *coords = new osg::Vec3Array;
    osg::Vec3Array *nrms = new osg::Vec3Array;
    osg::Vec2Array *tcs = new osg::Vec2Array;
    osg::Vec3 nrm(0,-1,0);
    // coordinates from red book code but shifted by 1000 & 2000 for alternate tesselatory things.
    static GLdouble rects[12][3] = 
    { { 50.0, 50.0, 0.0 },
      { 300.0, 50.0, 0.0 }, 
      { 300.0, 300.0, 0.0 },
      { 50.0, 300.0, 0.0 },
      { 100.0, 100.0, 0.0 },
      { 250.0, 100.0, 0.0 }, 
      { 250.0, 250.0, 0.0 },
      { 100.0, 250.0, 0.0 },
      { 150.0, 150.0, 0.0 },
      { 200.0, 150.0, 0.0 }, 
      { 200.0, 200.0, 0.0 },
      { 150.0, 200.0, 0.0 } };

    static GLdouble rectsMidanti[12][3] = // the centre 2 contours are traversed opposite order to outer contour.
    { { 1050.0, 50.0, 0.0 },
      { 1300.0, 50.0, 0.0 }, 
      { 1300.0, 300.0, 0.0 },
      { 1050.0, 300.0, 0.0 },
      { 1250.0, 100.0, 0.0 },
      { 1100.0, 100.0, 0.0 }, 
      { 1100.0, 250.0, 0.0 },
      { 1250.0, 250.0, 0.0 }, 
      { 1200.0, 150.0, 0.0 },
      { 1150.0, 150.0, 0.0 }, 
      { 1150.0, 200.0, 0.0 },
      { 1200.0, 200.0, 0.0 } };
    static GLdouble spiral[16][3] = // shift by 1000; nb the order of vertices is reversed from that of the red book
    { { 3400.0, 250.0, 0.0 },
      { 3400.0, 50.0, 0.0 }, 
      { 3050.0, 50.0, 0.0 },  
      { 3050.0, 400.0, 0.0 }, 
      { 3350.0, 400.0, 0.0 }, 
      { 3350.0, 100.0, 0.0 }, 
      { 3100.0, 100.0, 0.0 }, 
      { 3100.0, 350.0, 0.0 }, 
      { 3300.0, 350.0, 0.0 }, 
      { 3300.0, 150.0, 0.0 }, 
      { 3150.0, 150.0, 0.0 }, 
      { 3150.0, 300.0, 0.0 }, 
      { 3250.0, 300.0, 0.0 }, 
      { 3250.0, 200.0, 0.0 }, 
      { 3200.0, 200.0, 0.0 }, 
      { 3200.0, 250.0, 0.0 }
    };
    static GLdouble quad1[4][3] = // shift by 2000 for next 3 things
    { { 2050.0, 150.0, 0.0 }, 
      { 2350.0, 150.0, 0.0 }, 
      { 2350.0, 200.0, 0.0 }, 
      { 2050.0, 200.0, 0.0 }
    };
    static GLdouble quad2[4][3] =
    { { 2100.0, 100.0, 0.0 },
      { 2300.0, 100.0, 0.0 }, 
      { 2300.0, 350.0, 0.0 }, 
      { 2100.0, 350.0, 0.0 }
    };
    static GLdouble tri[3][3] =
    { { 2200.0, 50.0, 0.0 }, 
      { 2250.0, 300.0, 0.0 },
      { 2150.0, 300.0, 0.0 }
    };
    static GLdouble quad3[4][3] =
    { { 100.0, 1100.0, 0.0 }, 
      { 1300.0, 1100.0, 0.0 }, 
      { 1300.0, 2350.0, 0.0 }, 
      { 100.0, 2350.0, 0.0}
    };
    static GLdouble quadstrip[8][3] =
    { { 900.0, 1130.0, 0.0 },
      { 1100.0, 1130.0, 0.0 }, 
      { 900.0, 1350.0, 0.0 },
      { 950.0, 1350.0, 0.0 }, 
      { 900.0, 1550.0, 0.0 },
      { 1000.0, 1550.0, 0.0 },
      { 900.0, 1750.0, 0.0 },
      { 1400.0, 1750.0, 0.0 }
    };
    
    for (i = 0; i < 12; i++) {
        coords->push_back(osg::Vec3(rects[i][0],rects[i][2],rects[i][1]));
        tcs->push_back(osg::Vec2(rects[i][0],rects[i][1])/200.0);
        nrms->push_back(nrm);
    }
    for (i = 0; i < 12; i++) {
        coords->push_back(osg::Vec3(rectsMidanti[i][0],rectsMidanti[i][2],rectsMidanti[i][1]));
        tcs->push_back(osg::Vec2(rectsMidanti[i][0],rectsMidanti[i][1])/200.0);
        nrms->push_back(nrm);
    }
    for (i = 0; i < 16; i++) { // and reverse spiral to make same as that of red book ch 11
        coords->push_back(osg::Vec3(spiral[15-i][0],spiral[15-i][2],spiral[15-i][1]));
        tcs->push_back(osg::Vec2(spiral[15-i][0],spiral[15-i][1])/200.0);
        nrms->push_back(nrm);
    }
    for (i = 0; i < 4; i++) {
        coords->push_back(osg::Vec3(quad1[i][0],quad1[i][2],quad1[i][1]));
        tcs->push_back(osg::Vec2(quad1[i][0],quad1[i][1])/200.0);
        nrms->push_back(nrm);
    }
    for (i = 0; i < 4; i++) {
        coords->push_back(osg::Vec3(quad2[i][0],quad2[i][2],quad2[i][1]));
        tcs->push_back(osg::Vec2(quad2[i][0],quad2[i][1])/200.0);
        nrms->push_back(nrm);
    }
    for (i = 0; i < 3; i++) {
        coords->push_back(osg::Vec3(tri[i][0],tri[i][2],tri[i][1]));
        tcs->push_back(osg::Vec2(tri[i][0],tri[i][1])/200.0);
        nrms->push_back(nrm);
    }
    // add one large quad with multiple holes
    for (i = 0; i < 4; i++) {
        coords->push_back(osg::Vec3(quad3[i][0],quad3[i][2],quad3[i][1]));
        tcs->push_back(osg::Vec2(quad3[i][0],quad3[i][1])/200.0);
        nrms->push_back(nrm);
    }
    {
        osg::Vec3 centre(300,0,1500);
        for (i = 0; i < 18; i++) {
            osg::Vec3 rim=centre+osg::Vec3(-cos(osg::DegreesToRadians((float)i*20.0)),0.0,sin(osg::DegreesToRadians((float)i*20.0)))*150.0;
            coords->push_back(rim);
            tcs->push_back(osg::Vec2(rim.x(),rim.z())/200.0);
            nrms->push_back(nrm);
        }
    }
    {
        osg::Vec3 centre(400,0,1800);
        for (i = 0; i < 18; i++) {
            osg::Vec3 rim=centre+osg::Vec3(-cos(osg::DegreesToRadians((float)i*15.0)),0.0,sin(osg::DegreesToRadians((float)i*15.0)))*250.0;
            coords->push_back(rim);
            tcs->push_back(osg::Vec2(rim.x(),rim.z())/200.0);
            nrms->push_back(nrm);
        }
    }
    {
        osg::Vec3 centre(600,0,1400);
        for (i = 0; i < 18; i++) {
            osg::Vec3 rim=centre+osg::Vec3(-cos(osg::DegreesToRadians((float)i*12.0)),0.0,sin(osg::DegreesToRadians((float)i*12.0)))*250.0;
            coords->push_back(rim);
            tcs->push_back(osg::Vec2(rim.x(),rim.z())/200.0);
            nrms->push_back(nrm);
        }
    }
    // add one large quadstrip 
    for (i = 0; i < 8; i++) {
        coords->push_back(osg::Vec3(quadstrip[i][0],quadstrip[i][2],quadstrip[i][1]));
        tcs->push_back(osg::Vec2(quadstrip[i][0],quadstrip[i][1])/200.0);
        nrms->push_back(nrm);
    }
    gtess->setVertexArray(coords);
    gtess->setNormalArray(nrms);
    gtess->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
    gtess->setTexCoordArray(0,tcs);
    
    // demonstrate that the tesselator makes textured tesselations
    osg::StateSet* stateset = new osg::StateSet();
    
    osg::Image* image = osgDB::readImageFile("Cubemap_snow/posz.jpg");
    if (image)
    {
        osg::Texture2D* texture = new osg::Texture2D;
        texture->setImage(image);
        texture->setWrap(osg::Texture2D::WRAP_S, osg::Texture2D::REPEAT);
        texture->setWrap(osg::Texture2D::WRAP_T, osg::Texture2D::REPEAT);
        stateset->setTextureAttributeAndModes(0,texture,osg::StateAttribute::ON);
    }
    gtess->setStateSet( stateset );
    
    int nstart=0;
    // the contours accepoted are polygons; quads & tris. Trifans can bve added later.
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,nstart,12));nstart+=12;
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,nstart,12));nstart+=12;
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,nstart,16));nstart+=16;
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,nstart,4));nstart+=4;
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,nstart,4));nstart+=4;
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLES,nstart,3));nstart+=3;
    // A rectabngle with multiple holes
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP,nstart,4));nstart+=4;
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_FAN,nstart,18));nstart+=18;
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,nstart,18));nstart+=18;
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,nstart,18));nstart+=18;
    // test for quad strip
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP,nstart,8));nstart+=8;
    
    // We need to access the tesselatable contours again to demonstrate all types of tesselation.
    // I could add the Tesselator to the geometry as userdata, but here
    // I use the derived tesselateDemoGeometry to hold both the drawable geode and the original contours.
    
    gtess->setTesselationType(osgUtil::Tesselator::TESS_TYPE_GEOMETRY);
    gtess->setBoundaryOnly(true);
    gtess->setWindingType( osgUtil::Tesselator::TESS_WINDING_ABS_GEQ_TWO); // so that first change in wind type makes the commonest tesselation - ODD.
    
    return gtess;
}
osg::Node* createHUD()
{ // add a string reporting the type of winding rule tesselation applied
    osg::Geode* geode = new osg::Geode();
    
    std::string timesFont("fonts/arial.ttf");

    // turn lighting off for the text and disable depth test to ensure its always ontop.
    osg::StateSet* stateset = geode->getOrCreateStateSet();
    stateset->setMode(GL_LIGHTING,osg::StateAttribute::OFF);

    // Disable depth test, and make sure that the hud is drawn after everything 
    // else so that it always appears ontop.
    stateset->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
    stateset->setRenderBinDetails(11,"RenderBin");

    osg::Vec3 position(150.0f,900.0f,0.0f);
    osg::Vec3 delta(0.0f,-30.0f,0.0f);

    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setPosition(position);
        text->setText("Tesselation example - no tesselation (use 'W' wireframe to visualise)");
        text->setColor(osg::Vec4(1.0,1.0,0.8,1.0));
        position += delta;
        
    }    
    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setPosition(position);
        text->setText("Press 'n' to use an alternative tesselation.");
        
    }    

    // create the hud.
    osg::MatrixTransform* modelview_abs = new osg::MatrixTransform;
    modelview_abs->setReferenceFrame(osg::Transform::ABSOLUTE_RF);
    modelview_abs->setMatrix(osg::Matrix::identity());
    modelview_abs->addChild(geode);

    osg::Projection* projection = new osg::Projection;
    projection->setMatrix(osg::Matrix::ortho2D(0,1280,0,1024));
    projection->addChild(modelview_abs);

    return projection;

}


osg::Group *makeTesselateExample (void) {
    osg::Group *grp=new osg::Group;
    osg::Geode *gd=new osg::Geode;
    gd->addDrawable(makePols());
    gd->addDrawable(makePolsTwo());
    grp->addChild(gd);

    grp->addChild(makeHouse());

    return grp;
}

class setTesselateVisitor : public osg::NodeVisitor
{ // searches a loaded model tree for tesselatable geometries.
    // used with any database model which has a renderGroup (Geode) named 'tesselate'
    // or you can force a type of tess with special names or a sub-class of Geode could have extra information
    // of course you can use any name to detect what is to be tesselated!
    // all the polygons within the specific node are deemed to be contours, so
    // any tesselation can be requested.
public:
    
    setTesselateVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
    }
    virtual void apply(osg::Geode& geode) {
        if (geode.getName().compare(0,9,"tesselate")==0) {
            for(unsigned int i=0;i<geode.getNumDrawables();++i)
            {
                osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
                if (geom) {
                    osg::ref_ptr<osgUtil::Tesselator> tscx=new osgUtil::Tesselator();
                    if (tscx.valid()) {
                        tscx->setTesselationType(osgUtil::Tesselator::TESS_TYPE_GEOMETRY);
                        if (geode.getName()== "tesselate") {
                            // add a tesselator so that this geom is retesselated when N is pressed
                            tscx->setBoundaryOnly(true);
                            tscx->setWindingType( osgUtil::Tesselator::TESS_WINDING_ABS_GEQ_TWO); // so that first change in wind type makes the commonest tesselation - ODD.
                            geom->setUserData(tscx.get());
                        } else if (geode.getName()== "tesselate odd") {
                            // OR you can just apply the tesselator once only, using these different types
                            tscx->setWindingType( osgUtil::Tesselator::TESS_WINDING_ODD); // commonest tesselation - ODD.
                            tscx->retesselatePolygons(*geom);
                        } else if (geode.getName()== "tesselate odd bound") {
                            tscx->setBoundaryOnly(true);
                            tscx->setWindingType( osgUtil::Tesselator::TESS_WINDING_ODD); // tesselation - ODD, only show boundary.
                            tscx->retesselatePolygons(*geom);
                        } else if (geode.getName()== "tesselate positive") {
                            tscx->setWindingType( osgUtil::Tesselator::TESS_WINDING_POSITIVE); // tesselation - pos.
                            tscx->retesselatePolygons(*geom);
                        } else if (geode.getName()== "tesselate positive bound") {
                            tscx->setBoundaryOnly(true);
                            tscx->setWindingType( osgUtil::Tesselator::TESS_WINDING_POSITIVE);
                            tscx->retesselatePolygons(*geom);
                        } else if (geode.getName()== "tesselate negative") {
                            tscx->setWindingType( osgUtil::Tesselator::TESS_WINDING_NEGATIVE);
                            tscx->retesselatePolygons(*geom);
                        } else if (geode.getName()== "tesselate negative bound") {
                            tscx->setBoundaryOnly(true);
                            tscx->setWindingType( osgUtil::Tesselator::TESS_WINDING_NEGATIVE);
                            tscx->retesselatePolygons(*geom);
                        } else if (geode.getName()== "tesselate nonzero") {
                            tscx->setWindingType( osgUtil::Tesselator::TESS_WINDING_NONZERO);
                            tscx->retesselatePolygons(*geom);
                        } else if (geode.getName()== "tesselate nonzero bound") {
                            tscx->setBoundaryOnly(true);
                            tscx->setWindingType( osgUtil::Tesselator::TESS_WINDING_NONZERO);
                            tscx->retesselatePolygons(*geom);
                        } else if (geode.getName()== "tesselate geq2") {
                            tscx->setWindingType( osgUtil::Tesselator::TESS_WINDING_ABS_GEQ_TWO);
                            tscx->retesselatePolygons(*geom);
                        } else if (geode.getName()== "tesselate geq2 bound") {
                            tscx->setBoundaryOnly(true);
                            tscx->setWindingType( osgUtil::Tesselator::TESS_WINDING_ABS_GEQ_TWO);
                            tscx->retesselatePolygons(*geom);
                        }
                    }
                }
            }
        }
    }    
};

class cxTesselateVisitor : public osg::NodeVisitor
{ // special to this demo, traverses SG and finds nodes which have been tesselated
    // for test/demo purposes these nodes are of type tesselateDemoGeometry
    // but you could store the Tesselator as UserData or however you like.
    // the tesselator holds copies of the original contours used in the tesselation
    // In this visitor, I reuse the contours to make a different type of tesselation.
public:
    
    cxTesselateVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
    }
    virtual void apply(osg::Geode& geode) {

        for(unsigned int i=0;i<geode.getNumDrawables();++i)
        {
            tesselateDemoGeometry *geom=dynamic_cast<tesselateDemoGeometry*>(geode.getDrawable(i));
            if (geom) {
                if (!geom->getBoundaryOnly()) { // turn on bounds only
                    // NB this shows only the true boundary of the curves, no internal edges                    
                    geom->setBoundaryOnly(true);
                    
                } else { // change to next type of tesselation...
                    geom->setBoundaryOnly(false);
                    switch (geom->getWindingType()) {
                    case         osgUtil::Tesselator::TESS_WINDING_ODD:
                        geom->setWindingType(osgUtil::Tesselator::TESS_WINDING_NONZERO);
                        break;
                    case    osgUtil::Tesselator::TESS_WINDING_NONZERO:
                        geom->setWindingType( osgUtil::Tesselator::TESS_WINDING_POSITIVE);
                        break;
                    case    osgUtil::Tesselator::TESS_WINDING_POSITIVE:
                        geom->setWindingType( osgUtil::Tesselator::TESS_WINDING_NEGATIVE);
                        break;
                    case    osgUtil::Tesselator::TESS_WINDING_NEGATIVE:
                        geom->setWindingType( osgUtil::Tesselator::TESS_WINDING_ABS_GEQ_TWO);
                        break;
                    case    osgUtil::Tesselator::TESS_WINDING_ABS_GEQ_TWO:
                        geom->setWindingType( osgUtil::Tesselator::TESS_WINDING_ODD);
                        break;
                    }
                }
                
                switch (geom->getWindingType()) { // a text to be added to the scene.
                case         osgUtil::Tesselator::TESS_WINDING_ODD:
                    str="TESS_WINDING_ODD";
                    break;
                case    osgUtil::Tesselator::TESS_WINDING_NONZERO:
                    str="TESS_WINDING_NONZERO";
                    break;
                case    osgUtil::Tesselator::TESS_WINDING_POSITIVE:
                    str="TESS_WINDING_POSITIVE";
                    break;
                case    osgUtil::Tesselator::TESS_WINDING_NEGATIVE:
                    str="TESS_WINDING_NEGATIVE";
                    break;
                case    osgUtil::Tesselator::TESS_WINDING_ABS_GEQ_TWO:
                    str="TESS_WINDING_ABS_GEQ_TWO";
                    break;
                }
                if (geom->getBoundaryOnly()) str += " Boundary";
                
                geom->retesselatePolygons(*geom);
            }
            osgText::Text* txt = dynamic_cast<osgText::Text*>(geode.getDrawable(i));
            if (txt) {
                const osg::Vec4& ct=txt->getColor(); // pick the text to be changed by its color
                if (ct.z()<0.9) {
                    txt->setText(str.c_str());
                }
            }
        }
        traverse(geode);
    }
    
    std::string str; // a label for on screen display
};

class KeyboardEventHandler : public osgGA::GUIEventHandler
{ // extra event handler traps 'n' key to re-tesselate any tesselated geodes.
public:
    
    KeyboardEventHandler(osg::Node *nd):
        _scene(nd) {}
    
        virtual bool handle(const osgGA::GUIEventAdapter& ea,osgGA::GUIActionAdapter&)
        {
            switch(ea.getEventType())
            {
                case(osgGA::GUIEventAdapter::KEYDOWN):
                {
                    if (_scene && ea.getKey()=='n')
                    {
                        // re-tesselate the scene graph. 
                        // the same contours are re-tesselated using a new method. Old contours 
                        // & tesselation type are held internally in the derived Geode class tesselateDemoGeometry.
                        cxTesselateVisitor tsv;
                        _scene->accept(tsv);
                       return true;
                    }
                    break;
                }
                default:
                    break;
            }
            return false;
        }

        virtual void accept(osgGA::GUIEventHandlerVisitor& v)
        {
            v.visit(*this);
        }
        
        osg::Node *_scene;
        
};


int main( int argc, char **argv )
{

    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);
    
    // set up the usage document, in case we need to print out how to use this program.
    arguments.getApplicationUsage()->setApplicationName(arguments.getApplicationName());
    arguments.getApplicationUsage()->setDescription(arguments.getApplicationName()+" is provides an example of doing tesseleation..");
    arguments.getApplicationUsage()->setCommandLineUsage(arguments.getApplicationName()+" [options] filename ...");
    arguments.getApplicationUsage()->addCommandLineOption("-h or --help","Display this information");
    

    // construct the viewer.
    osgProducer::Viewer viewer(arguments);

    // set up the value with sensible default event handlers.
    viewer.setUpViewer(osgProducer::Viewer::STANDARD_SETTINGS);

    // get details on keyboard and mouse bindings used by the viewer.
    viewer.getUsage(*arguments.getApplicationUsage());

    // if user request help write it out to cout.
    if (arguments.read("-h") || arguments.read("--help"))
    {
        arguments.getApplicationUsage()->write(std::cout);
        return 1;
    }

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
        return 1;
    }
    
    osg::Timer timer;
    osg::Timer_t start_tick = timer.tick();

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

    // if no model has been successfully loaded report failure.
    if (!loadedModel) 
    {
        loadedModel=makeTesselateExample();
    } else { // if there is a loaded model:
        // tesselate by searching for geode called tesselate & tesselate it
        setTesselateVisitor tsv;
        loadedModel->accept(tsv);
    }

    // create the hud.
    osg::Group *gload= dynamic_cast<osg::Group *> (loadedModel.get());
    gload->addChild(createHUD());

    
    // any option left unread are converted into errors to write out later.
    arguments.reportRemainingOptionsAsUnrecognized();

    // report any errors if they have occured when parsing the program aguments.
    if (arguments.errors())
    {
        arguments.writeErrorMessages(std::cout);
    }

    osg::Timer_t end_tick = timer.tick();

    std::cout << "Time to load = "<<timer.delta_s(start_tick,end_tick)<<std::endl;


    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get() );

    // set the scene to render
    viewer.setSceneData(loadedModel.get());

    // add event handler for keyboard 'n' to retesselate
    viewer.getEventHandlerList().push_front(new KeyboardEventHandler(loadedModel.get()));

    // create the windows and run the threads.
    viewer.realize();

    while( !viewer.done() )
    {
        // wait for all cull and draw threads to complete.
        viewer.sync();

        // update the scene by traversing it with the the update visitor which will
        // call all node update callbacks and animations.
        viewer.update();
         
        // fire off the cull and draw traversals of the scene.
        viewer.frame();
        
    }
    
    // wait for all cull and draw threads to complete.
    viewer.sync();

    // do OpenGL clean up
    viewer.cleanup_frame();

    // wait for all cull and draw threads to complete before exit.
    viewer.sync();

    return 0;
}

