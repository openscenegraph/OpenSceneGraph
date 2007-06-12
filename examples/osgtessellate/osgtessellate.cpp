/* OpenSceneGraph example, osgtessellate.
*
*  Permission is hereby granted, free of charge, to any person obtaining a copy
*  of this software and associated documentation files (the "Software"), to deal
*  in the Software without restriction, including without limitation the rights
*  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
*  copies of the Software, and to permit persons to whom the Software is
*  furnished to do so, subject to the following conditions:
*
*  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
*  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
*  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
*  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
*  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
*  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
*  THE SOFTWARE.
*/

/* osgTessellator
 * - this Tessellator is an extension of the basic one - rather than tessellating
 * individual polygons, we tessellate the entire geode with multiple contours.
 * allows for overlapping contours etc.
 * the Tessellator has new member fuinctions
 setTessellationType(osgUtil::Tessellator::TESS_TYPE_xxx);
    tscx->setBoundaryOnly(bool);
    tscx->setWindingType( osgUtil::Tessellator::TESS_WINDING_xxx); 
 * for winding rules: See the red book chap 13.
 */

#include <osgDB/ReadFile>
#include <osgUtil/Optimizer>
#include <osgViewer/Viewer>
#include <osg/Projection>
#include <osg/MatrixTransform>

#include <osgText/Text>

#include <osgUtil/Tessellator> // to tessellate multiple contours


class tessellateDemoGeometry : public osg::Geometry, public osgUtil::Tessellator {
    // We add the Tessellator to the geometry because we want to access the
    // tessellatable contours again;  you can apply a Tessellator to a Geometry 
    // to produce exactly a required tessellation once only, and then 
    // the contours could be discarded since the geometry does not need to be retessellated.
public:
    tessellateDemoGeometry() {};

protected:
    virtual ~tessellateDemoGeometry() {};
};

osg::Geometry *makePolsTwo (void) 
{
    // an example of using current geometry contours to create next tessellation
    // this polygon disappears once the contour rules make no polygons.
    tessellateDemoGeometry *gtess= new tessellateDemoGeometry;
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
    
    // demonstrate that the Tessellator makes textured tessellations
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
    // The derived class tessellateDemoGeometry retains the original contours for re-use.
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP,nstart,8));nstart+=8;
    gtess->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::TRIANGLE_STRIP,nstart,8));nstart+=8;

    gtess->setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
    gtess->setBoundaryOnly(true);
    gtess->setWindingType( osgUtil::Tessellator::TESS_WINDING_ABS_GEQ_TWO); // so that first change in wind type makes the commonest tessellation - ODD.
    
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
    
    // demonstrate that the Tessellator makes textured tessellations
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
    

    osg::ref_ptr<osgUtil::Tessellator> tscx=new osgUtil::Tessellator; // the v1.2 multi-contour Tessellator.
    // we use the geometry primitives to describe the contours which are tessellated.
    // Winding odd means leave hole in surface where there are 2,4,6... contours circling the point.
    tscx->setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
    tscx->setBoundaryOnly(false);
    tscx->setWindingType( osgUtil::Tessellator::TESS_WINDING_ODD); // so that first change in wind type makes the commonest tessellation - ODD.
    
    tscx->retessellatePolygons(*gtess);
    
    return gtess;
}

osg::Geometry *makeFrontWall (const float zpos) {
    // an example of using one tessellation to make a 'house' wall
    // describe the wall as a pentagon, then door & 4 windows are further contours
    // tessellate the set of contours to make a 'house wall' from the Boolean-like operations.
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

    // demonstrate that the Tessellator makes textured tessellations
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
    
    // We use a Tessellator to produce the tessellation required once only 
    // and the contours are discarded.
    osg::ref_ptr<osgUtil::Tessellator> tscx=new osgUtil::Tessellator; // the v1.2 multi-contour Tessellator.
    tscx->setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
    tscx->setBoundaryOnly(false);
    tscx->setWindingType( osgUtil::Tessellator::TESS_WINDING_ODD); // so that first change in wind type makes the commonest tessellation - ODD.
    
    tscx->retessellatePolygons(*gtess);
    
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
    tessellateDemoGeometry *gtess= new tessellateDemoGeometry;
    int i;
    osg::Vec3Array *coords = new osg::Vec3Array;
    osg::Vec3Array *nrms = new osg::Vec3Array;
    osg::Vec2Array *tcs = new osg::Vec2Array;
    osg::Vec3 nrm(0,-1,0);
    // coordinates from red book code but shifted by 1000 & 2000 for alternate Tessellatory things.
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
    
    // demonstrate that the Tessellator makes textured tessellations
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
    
    // We need to access the tessellatable contours again to demonstrate all types of tessellation.
    // I could add the Tessellator to the geometry as userdata, but here
    // I use the derived tessellateDemoGeometry to hold both the drawable geode and the original contours.
    
    gtess->setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
    gtess->setBoundaryOnly(true);
    gtess->setWindingType( osgUtil::Tessellator::TESS_WINDING_ABS_GEQ_TWO); // so that first change in wind type makes the commonest tessellation - ODD.
    
    return gtess;
}
osg::Node* createHUD()
{ // add a string reporting the type of winding rule tessellation applied
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
        text->setText("Tessellation example - no tessellation (use 'W' wireframe to visualise)");
        text->setColor(osg::Vec4(1.0,1.0,0.8,1.0));
        position += delta;
        
    }    
    {
        osgText::Text* text = new  osgText::Text;
        geode->addDrawable( text );

        text->setFont(timesFont);
        text->setPosition(position);
        text->setText("Press 'n' to use an alternative tessellation.");
        
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


osg::Group *makeTessellateExample (void) {
    osg::Group *grp=new osg::Group;
    osg::Geode *gd=new osg::Geode;
    gd->addDrawable(makePols());
    gd->addDrawable(makePolsTwo());
    grp->addChild(gd);

    grp->addChild(makeHouse());

    return grp;
}

class setTessellateVisitor : public osg::NodeVisitor
{ // searches a loaded model tree for tessellatable geometries.
    // used with any database model which has a renderGroup (Geode) named 'tessellate'
    // or you can force a type of tess with special names or a sub-class of Geode could have extra information
    // of course you can use any name to detect what is to be tessellated!
    // all the polygons within the specific node are deemed to be contours, so
    // any tessellation can be requested.
public:
    
    setTessellateVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
    }
    virtual void apply(osg::Geode& geode) {
        if (geode.getName().compare(0,9,"tessellate")==0) {
            for(unsigned int i=0;i<geode.getNumDrawables();++i)
            {
                osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode.getDrawable(i));
                if (geom) {
                    osg::ref_ptr<osgUtil::Tessellator> tscx=new osgUtil::Tessellator();
                    if (tscx.valid()) {
                        tscx->setTessellationType(osgUtil::Tessellator::TESS_TYPE_GEOMETRY);
                        if (geode.getName()== "tessellate") {
                            // add a Tessellator so that this geom is retessellated when N is pressed
                            tscx->setBoundaryOnly(true);
                            tscx->setWindingType( osgUtil::Tessellator::TESS_WINDING_ABS_GEQ_TWO); // so that first change in wind type makes the commonest tessellation - ODD.
                            geom->setUserData(tscx.get());
                        } else if (geode.getName()== "tessellate odd") {
                            // OR you can just apply the Tessellator once only, using these different types
                            tscx->setWindingType( osgUtil::Tessellator::TESS_WINDING_ODD); // commonest tessellation - ODD.
                            tscx->retessellatePolygons(*geom);
                        } else if (geode.getName()== "tessellate odd bound") {
                            tscx->setBoundaryOnly(true);
                            tscx->setWindingType( osgUtil::Tessellator::TESS_WINDING_ODD); // tessellation - ODD, only show boundary.
                            tscx->retessellatePolygons(*geom);
                        } else if (geode.getName()== "tessellate positive") {
                            tscx->setWindingType( osgUtil::Tessellator::TESS_WINDING_POSITIVE); // tessellation - pos.
                            tscx->retessellatePolygons(*geom);
                        } else if (geode.getName()== "tessellate positive bound") {
                            tscx->setBoundaryOnly(true);
                            tscx->setWindingType( osgUtil::Tessellator::TESS_WINDING_POSITIVE);
                            tscx->retessellatePolygons(*geom);
                        } else if (geode.getName()== "tessellate negative") {
                            tscx->setWindingType( osgUtil::Tessellator::TESS_WINDING_NEGATIVE);
                            tscx->retessellatePolygons(*geom);
                        } else if (geode.getName()== "tessellate negative bound") {
                            tscx->setBoundaryOnly(true);
                            tscx->setWindingType( osgUtil::Tessellator::TESS_WINDING_NEGATIVE);
                            tscx->retessellatePolygons(*geom);
                        } else if (geode.getName()== "tessellate nonzero") {
                            tscx->setWindingType( osgUtil::Tessellator::TESS_WINDING_NONZERO);
                            tscx->retessellatePolygons(*geom);
                        } else if (geode.getName()== "tessellate nonzero bound") {
                            tscx->setBoundaryOnly(true);
                            tscx->setWindingType( osgUtil::Tessellator::TESS_WINDING_NONZERO);
                            tscx->retessellatePolygons(*geom);
                        } else if (geode.getName()== "tessellate geq2") {
                            tscx->setWindingType( osgUtil::Tessellator::TESS_WINDING_ABS_GEQ_TWO);
                            tscx->retessellatePolygons(*geom);
                        } else if (geode.getName()== "tessellate geq2 bound") {
                            tscx->setBoundaryOnly(true);
                            tscx->setWindingType( osgUtil::Tessellator::TESS_WINDING_ABS_GEQ_TWO);
                            tscx->retessellatePolygons(*geom);
                        }
                    }
                }
            }
        }
    }    
};

class cxTessellateVisitor : public osg::NodeVisitor
{ // special to this demo, traverses SG and finds nodes which have been tessellated
    // for test/demo purposes these nodes are of type tessellateDemoGeometry
    // but you could store the Tessellator as UserData or however you like.
    // the Tessellator holds copies of the original contours used in the tessellation
    // In this visitor, I reuse the contours to make a different type of tessellation.
public:
    
    cxTessellateVisitor():osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN) {
    }
    virtual void apply(osg::Geode& geode) {

        for(unsigned int i=0;i<geode.getNumDrawables();++i)
        {
            tessellateDemoGeometry *geom=dynamic_cast<tessellateDemoGeometry*>(geode.getDrawable(i));
            if (geom) {
                if (!geom->getBoundaryOnly()) { // turn on bounds only
                    // NB this shows only the true boundary of the curves, no internal edges                    
                    geom->setBoundaryOnly(true);
                    
                } else { // change to next type of tessellation...
                    geom->setBoundaryOnly(false);
                    switch (geom->getWindingType()) {
                    case         osgUtil::Tessellator::TESS_WINDING_ODD:
                        geom->setWindingType(osgUtil::Tessellator::TESS_WINDING_NONZERO);
                        break;
                    case    osgUtil::Tessellator::TESS_WINDING_NONZERO:
                        geom->setWindingType( osgUtil::Tessellator::TESS_WINDING_POSITIVE);
                        break;
                    case    osgUtil::Tessellator::TESS_WINDING_POSITIVE:
                        geom->setWindingType( osgUtil::Tessellator::TESS_WINDING_NEGATIVE);
                        break;
                    case    osgUtil::Tessellator::TESS_WINDING_NEGATIVE:
                        geom->setWindingType( osgUtil::Tessellator::TESS_WINDING_ABS_GEQ_TWO);
                        break;
                    case    osgUtil::Tessellator::TESS_WINDING_ABS_GEQ_TWO:
                        geom->setWindingType( osgUtil::Tessellator::TESS_WINDING_ODD);
                        break;
                    }
                }
                
                switch (geom->getWindingType()) { // a text to be added to the scene.
                case         osgUtil::Tessellator::TESS_WINDING_ODD:
                    str="TESS_WINDING_ODD";
                    break;
                case    osgUtil::Tessellator::TESS_WINDING_NONZERO:
                    str="TESS_WINDING_NONZERO";
                    break;
                case    osgUtil::Tessellator::TESS_WINDING_POSITIVE:
                    str="TESS_WINDING_POSITIVE";
                    break;
                case    osgUtil::Tessellator::TESS_WINDING_NEGATIVE:
                    str="TESS_WINDING_NEGATIVE";
                    break;
                case    osgUtil::Tessellator::TESS_WINDING_ABS_GEQ_TWO:
                    str="TESS_WINDING_ABS_GEQ_TWO";
                    break;
                }
                if (geom->getBoundaryOnly()) str += " Boundary";
                
                geom->retessellatePolygons(*geom);
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
{ // extra event handler traps 'n' key to re-tessellate any tessellated geodes.
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
                        // re-tessellate the scene graph. 
                        // the same contours are re-tessellated using a new method. Old contours 
                        // & tessellation type are held internally in the derived Geode class tessellateDemoGeometry.
                        cxTessellateVisitor tsv;
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
        
        osg::Node *_scene;
        
};


int main( int argc, char **argv )
{
    // use an ArgumentParser object to manage the program arguments.
    osg::ArgumentParser arguments(&argc,argv);

    // construct the viewer.
    osgViewer::Viewer viewer;

    // read the scene from the list of file specified commandline args.
    osg::ref_ptr<osg::Node> loadedModel = osgDB::readNodeFiles(arguments);

    // if no model has been successfully loaded report failure.
    if (!loadedModel) 
    {
        loadedModel=makeTessellateExample();

    } else { // if there is a loaded model:

        // tessellate by searching for geode called tessellate & tessellate it
        setTessellateVisitor tsv;
        loadedModel->accept(tsv);

    }

    // create the hud.
    osg::Group *gload= dynamic_cast<osg::Group *> (loadedModel.get());
    gload->addChild(createHUD());


    osgUtil::Optimizer optimizer;
    optimizer.optimize(loadedModel.get() );

    // set the scene to render
    viewer.setSceneData(loadedModel.get());

    // add event handler for keyboard 'n' to retessellate
    viewer.addEventHandler(new KeyboardEventHandler(loadedModel.get()));

    return viewer.run();
}

