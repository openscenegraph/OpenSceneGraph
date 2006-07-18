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
#include <osg/ShapeDrawable>
#include <osg/GL>
#include <osg/Notify>

using namespace osg;

// arbitrary minima for rows & segments
const unsigned int MIN_NUM_ROWS = 3;
const unsigned int MIN_NUM_SEGMENTS = 5;


///////////////////////////////////////////////////////////////////////////////
//
// draw shape
//

class DrawShapeVisitor : public ConstShapeVisitor
{
    public:
    
        DrawShapeVisitor(State& state,const TessellationHints* hints):
            _state(state),
            _hints(hints)
        {
#if 0
            if (hints)
            {
                notify(NOTICE)<<"Warning: TessellationHints ignored in present osg::ShapeDrawable implementation."<<std::endl;
            }
#endif   
        }
    
        virtual void apply(const Sphere&);
        virtual void apply(const Box&);
        virtual void apply(const Cone&);
        virtual void apply(const Cylinder&);
        virtual void apply(const Capsule&);
        virtual void apply(const InfinitePlane&);

        virtual void apply(const TriangleMesh&);
        virtual void apply(const ConvexHull&);
        virtual void apply(const HeightField&);

        virtual void apply(const CompositeShape&);

        State&                      _state;
        const TessellationHints*    _hints;
        
    protected:
        enum SphereHalf { SphereTopHalf, SphereBottomHalf };
    
        // helpers for apply( Cylinder | Sphere | Capsule )
        void drawCylinderBody(unsigned int numSegments, float radius, float height);
        void drawHalfSphere(unsigned int numSegments, unsigned int numRows, float radius, SphereHalf which, float zOffset = 0.0f);
};


void DrawShapeVisitor::drawCylinderBody(unsigned int numSegments, float radius, float height)
{
    const float angleDelta = 2.0f*osg::PI/(float)numSegments;
    const float texCoordDelta = 1.0f/(float)numSegments;
    
    const float r = radius;
    const float h = height;
    
    float basez = -h*0.5f;
    float topz = h*0.5f;
    
    float angle = 0.0f;
    float texCoord = 0.0f;
    
    bool drawFrontFace = _hints ? _hints->getCreateFrontFace() : true;
    bool drawBackFace = _hints ? _hints->getCreateBackFace() : false;
    
    // The only difference between the font & back face loops is that the
    //  normals are inverted and the order of the vertex pairs is reversed.
    //  The code is mostly duplicated in order to hoist the back/front face 
    //  test out of the loop for efficiency

    glBegin(GL_QUAD_STRIP);

    if (drawFrontFace) {

      for(unsigned int bodyi=0;
          bodyi<numSegments;
          ++bodyi,angle+=angleDelta,texCoord+=texCoordDelta)
      {
          float c = cosf(angle);
          float s = sinf(angle);
  
          glNormal3f(c,s,0.0f);
  
          glTexCoord2f(texCoord,1.0f);
          glVertex3f(c*r,s*r,topz);
  
          glTexCoord2f(texCoord,0.0f);
          glVertex3f(c*r,s*r,basez);
      }
  
      // do last point by hand to ensure no round off errors.
      glNormal3f(1.0f,0.0f,0.0f);
  
      glTexCoord2f(1.0f,1.0f);
      glVertex3f(r,0.0f,topz);
  
      glTexCoord2f(1.0f,0.0f);
      glVertex3f(r,0.0f,basez);
    }
    
    if (drawBackFace) {
      for(unsigned int bodyi=0;
          bodyi<numSegments;
          ++bodyi,angle+=angleDelta,texCoord+=texCoordDelta)
      {
          float c = cosf(angle);
          float s = sinf(angle);
  
          glNormal3f(-c,-s,0.0f);
  
          glTexCoord2f(texCoord,0.0f);
          glVertex3f(c*r,s*r,basez);

          glTexCoord2f(texCoord,1.0f);
          glVertex3f(c*r,s*r,topz);
      }
  
      // do last point by hand to ensure no round off errors.
      glNormal3f(-1.0f,0.0f,0.0f);
  
      glTexCoord2f(1.0f,0.0f);
      glVertex3f(r,0.0f,basez);
  
      glTexCoord2f(1.0f,1.0f);
      glVertex3f(r,0.0f,topz);
    }

    glEnd();
}


void DrawShapeVisitor::drawHalfSphere(unsigned int numSegments, unsigned int numRows, float radius, SphereHalf which, float zOffset)
{
    float lDelta = osg::PI/(float)numRows;
    float vDelta = 1.0f/(float)numRows;

    bool top = (which==SphereTopHalf);

    bool drawFrontFace = _hints ? _hints->getCreateFrontFace() : true;
    bool drawBackFace = _hints ? _hints->getCreateBackFace() : false;

    float angleDelta = osg::PI*2.0f/(float)numSegments;
    float texCoordHorzDelta = 1.0f/(float)numSegments;

    float lBase=-osg::PI*0.5f + (top?(lDelta*(numRows/2)):0.0f);
    float rBase=(top?(cosf(lBase)*radius):0.0f);
    float zBase=(top?(sinf(lBase)*radius):-radius);
    float vBase=(top?(vDelta*(numRows/2)):0.0f);
    float nzBase=(top?(sinf(lBase)):-1.0f);
    float nRatioBase=(top?(cosf(lBase)):0.0f);

    unsigned int rowbegin = top?numRows/2:0;
    unsigned int rowend   = top?numRows:numRows/2;

    for(unsigned int rowi=rowbegin; rowi<rowend; ++rowi)
    {

        float lTop = lBase+lDelta;
        float rTop = cosf(lTop)*radius;
        float zTop = sinf(lTop)*radius;
        float vTop = vBase+vDelta;
        float nzTop= sinf(lTop);
        float nRatioTop= cosf(lTop);

        glBegin(GL_QUAD_STRIP);

            float angle = 0.0f;
            float texCoord = 0.0f;

            // The only difference between the font & back face loops is that the
            //  normals are inverted and the order of the vertex pairs is reversed.
            //  The code is mostly duplicated in order to hoist the back/front face 
            //  test out of the loop for efficiency
            
            if (drawFrontFace) {
              for(unsigned int topi=0; topi<numSegments;
                  ++topi,angle+=angleDelta,texCoord+=texCoordHorzDelta)
              {
  
                  float c = cosf(angle);
                  float s = sinf(angle);
  
                  glNormal3f(c*nRatioTop,s*nRatioTop,nzTop);
  
                  glTexCoord2f(texCoord,vTop);
                  glVertex3f(c*rTop,s*rTop,zTop+zOffset);
  
                  glNormal3f(c*nRatioBase,s*nRatioBase,nzBase);
  
                  glTexCoord2f(texCoord,vBase);
                  glVertex3f(c*rBase,s*rBase,zBase+zOffset);
  
              }
  
              // do last point by hand to ensure no round off errors.
              glNormal3f(nRatioTop,0.0f,nzTop);
  
              glTexCoord2f(1.0f,vTop);
              glVertex3f(rTop,0.0f,zTop+zOffset);
  
              glNormal3f(nRatioBase,0.0f,nzBase);
  
              glTexCoord2f(1.0f,vBase);
              glVertex3f(rBase,0.0f,zBase+zOffset);
            }
            
          if (drawBackFace) {
              for(unsigned int topi=0; topi<numSegments;
                  ++topi,angle+=angleDelta,texCoord+=texCoordHorzDelta)
              {
  
                  float c = cosf(angle);
                  float s = sinf(angle);
  
                  glNormal3f(-c*nRatioBase,-s*nRatioBase,-nzBase);
  
                  glTexCoord2f(texCoord,vBase);
                  glVertex3f(c*rBase,s*rBase,zBase+zOffset);
  
                  glNormal3f(-c*nRatioTop,-s*nRatioTop,-nzTop);
  
                  glTexCoord2f(texCoord,vTop);
                  glVertex3f(c*rTop,s*rTop,zTop+zOffset);
              }
  
              // do last point by hand to ensure no round off errors.
              glNormal3f(-nRatioBase,0.0f,-nzBase);
  
              glTexCoord2f(1.0f,vBase);
              glVertex3f(rBase,0.0f,zBase+zOffset);

              glNormal3f(-nRatioTop,0.0f,-nzTop);
  
              glTexCoord2f(1.0f,vTop);
              glVertex3f(rTop,0.0f,zTop+zOffset);
  
          }

        glEnd();
        
        
        lBase=lTop;
        rBase=rTop;
        zBase=zTop;
        vBase=vTop;
        nzBase=nzTop;
        nRatioBase=nRatioTop;

    }
        
}


void DrawShapeVisitor::apply(const Sphere& sphere)
{
    glPushMatrix();

    glTranslatef(sphere.getCenter().x(),sphere.getCenter().y(),sphere.getCenter().z());

    bool drawFrontFace = _hints ? _hints->getCreateFrontFace() : true;
    bool drawBackFace = _hints ? _hints->getCreateBackFace() : false;

    unsigned int numSegments = 40;
    unsigned int numRows = 20;
    float ratio = (_hints ? _hints->getDetailRatio() : 1.0f);
    if (ratio > 0.0f && ratio != 1.0f) {
        numRows = (unsigned int) (numRows * ratio);
        if (numRows < MIN_NUM_ROWS)
            numRows = MIN_NUM_ROWS;
        numSegments = (unsigned int) (numSegments * ratio);
        if (numSegments < MIN_NUM_SEGMENTS)
            numSegments = MIN_NUM_SEGMENTS;
    }

    float lDelta = osg::PI/(float)numRows;
    float vDelta = 1.0f/(float)numRows;

    float angleDelta = osg::PI*2.0f/(float)numSegments;
    float texCoordHorzDelta = 1.0f/(float)numSegments;

    if (drawBackFace)
    {
        float lBase=-osg::PI*0.5f;
        float rBase=0.0f;
        float zBase=-sphere.getRadius();
        float vBase=0.0f;
        float nzBase=-1.0f;
        float nRatioBase=0.0f;

        for(unsigned int rowi=0; rowi<numRows; ++rowi)
        {

            float lTop = lBase+lDelta;
            float rTop = cosf(lTop)*sphere.getRadius();
            float zTop = sinf(lTop)*sphere.getRadius();
            float vTop = vBase+vDelta;
            float nzTop= sinf(lTop);
            float nRatioTop= cosf(lTop);

            glBegin(GL_QUAD_STRIP);

                float angle = 0.0f;
                float texCoord = 0.0f;

                for(unsigned int topi=0; topi<numSegments;
                    ++topi,angle+=angleDelta,texCoord+=texCoordHorzDelta)
                {

                    float c = cosf(angle);
                    float s = sinf(angle);

                    glNormal3f(-c*nRatioBase,-s*nRatioBase,-nzBase);

                    glTexCoord2f(texCoord,vBase);
                    glVertex3f(c*rBase,s*rBase,zBase);

                    glNormal3f(-c*nRatioTop,-s*nRatioTop,-nzTop);

                    glTexCoord2f(texCoord,vTop);
                    glVertex3f(c*rTop,s*rTop,zTop);


                }


                // do last point by hand to ensure no round off errors.
                glNormal3f(-nRatioBase,0.0f,-nzBase);

                glTexCoord2f(1.0f,vBase);
                glVertex3f(rBase,0.0f,zBase);

                glNormal3f(-nRatioTop,0.0f,-nzTop);

                glTexCoord2f(1.0f,vTop);
                glVertex3f(rTop,0.0f,zTop);

            glEnd();


            lBase=lTop;
            rBase=rTop;
            zBase=zTop;
            vBase=vTop;
            nzBase=nzTop;
            nRatioBase=nRatioTop;

        }
    }


    if (drawFrontFace)
    {
        float lBase=-osg::PI*0.5f;
        float rBase=0.0f;
        float zBase=-sphere.getRadius();
        float vBase=0.0f;
        float nzBase=-1.0f;
        float nRatioBase=0.0f;

        for(unsigned int rowi=0; rowi<numRows; ++rowi)
        {

            float lTop = lBase+lDelta;
            float rTop = cosf(lTop)*sphere.getRadius();
            float zTop = sinf(lTop)*sphere.getRadius();
            float vTop = vBase+vDelta;
            float nzTop= sinf(lTop);
            float nRatioTop= cosf(lTop);

            glBegin(GL_QUAD_STRIP);

                float angle = 0.0f;
                float texCoord = 0.0f;

                for(unsigned int topi=0; topi<numSegments;
                    ++topi,angle+=angleDelta,texCoord+=texCoordHorzDelta)
                {

                    float c = cosf(angle);
                    float s = sinf(angle);

                    glNormal3f(c*nRatioTop,s*nRatioTop,nzTop);

                    glTexCoord2f(texCoord,vTop);
                    glVertex3f(c*rTop,s*rTop,zTop);

                    glNormal3f(c*nRatioBase,s*nRatioBase,nzBase);

                    glTexCoord2f(texCoord,vBase);
                    glVertex3f(c*rBase,s*rBase,zBase);

                }

                // do last point by hand to ensure no round off errors.
                glNormal3f(nRatioTop,0.0f,nzTop);

                glTexCoord2f(1.0f,vTop);
                glVertex3f(rTop,0.0f,zTop);

                glNormal3f(nRatioBase,0.0f,nzBase);

                glTexCoord2f(1.0f,vBase);
                glVertex3f(rBase,0.0f,zBase);

            glEnd();


            lBase=lTop;
            rBase=rTop;
            zBase=zTop;
            vBase=vTop;
            nzBase=nzTop;
            nRatioBase=nRatioTop;

        }
    }


    glPopMatrix();        
}

void DrawShapeVisitor::apply(const Box& box)
{
    // evaluate hints
    bool createBody = (_hints ? _hints->getCreateBody() : true);
    bool createTop = (_hints ? _hints->getCreateTop() : true);
    bool createBottom = (_hints ? _hints->getCreateBottom() : true);

    float dx = box.getHalfLengths().x();
    float dy = box.getHalfLengths().y();
    float dz = box.getHalfLengths().z();

    glPushMatrix();

    glTranslatef(box.getCenter().x(),box.getCenter().y(),box.getCenter().z());

    if (!box.zeroRotation())
    {
        Matrix rotation(box.computeRotationMatrix());
        glMultMatrix(rotation.ptr());
    }

    glBegin(GL_QUADS);

    if (createBody) {
        // -ve y plane
        glNormal3f(0.0f,-1.0f,0.0f);

        glTexCoord2f(0.0f,1.0f);
        glVertex3f(-dx,-dy,dz);

        glTexCoord2f(0.0f,0.0f);
        glVertex3f(-dx,-dy,-dz);

        glTexCoord2f(1.0f,0.0f);
        glVertex3f(dx,-dy,-dz);

        glTexCoord2f(1.0f,1.0f);
        glVertex3f(dx,-dy,dz);

        // +ve y plane
        glNormal3f(0.0f,1.0f,0.0f);

        glTexCoord2f(0.0f,1.0f);
        glVertex3f(dx,dy,dz);

        glTexCoord2f(0.0f,0.0f);
        glVertex3f(dx,dy,-dz);

        glTexCoord2f(1.0f,0.0f);
        glVertex3f(-dx,dy,-dz);

        glTexCoord2f(1.0f,1.0f);
        glVertex3f(-dx,dy,dz);

        // +ve x plane
        glNormal3f(1.0f,0.0f,0.0f);

        glTexCoord2f(0.0f,1.0f);
        glVertex3f(dx,-dy,dz);

        glTexCoord2f(0.0f,0.0f);
        glVertex3f(dx,-dy,-dz);

        glTexCoord2f(1.0f,0.0f);
        glVertex3f(dx,dy,-dz);

        glTexCoord2f(1.0f,1.0f);
        glVertex3f(dx,dy,dz);

        // -ve x plane
        glNormal3f(-1.0f,0.0f,0.0f);

        glTexCoord2f(0.0f,1.0f);
        glVertex3f(-dx,dy,dz);

        glTexCoord2f(0.0f,0.0f);
        glVertex3f(-dx,dy,-dz);

        glTexCoord2f(1.0f,0.0f);
        glVertex3f(-dx,-dy,-dz);

        glTexCoord2f(1.0f,1.0f);
        glVertex3f(-dx,-dy,dz);
    }

    if (createTop) {
        // +ve z plane
        glNormal3f(0.0f,0.0f,1.0f);

        glTexCoord2f(0.0f,1.0f);
        glVertex3f(-dx,dy,dz);

        glTexCoord2f(0.0f,0.0f);
        glVertex3f(-dx,-dy,dz);

        glTexCoord2f(1.0f,0.0f);
        glVertex3f(dx,-dy,dz);

        glTexCoord2f(1.0f,1.0f);
        glVertex3f(dx,dy,dz);
    }

    if (createBottom) {
        // -ve z plane
        glNormal3f(0.0f,0.0f,-1.0f);

        glTexCoord2f(0.0f,1.0f);
        glVertex3f(dx,dy,-dz);

        glTexCoord2f(0.0f,0.0f);
        glVertex3f(dx,-dy,-dz);

        glTexCoord2f(1.0f,0.0f);
        glVertex3f(-dx,-dy,-dz);

        glTexCoord2f(1.0f,1.0f);
        glVertex3f(-dx,dy,-dz);
    }

    glEnd();

    glPopMatrix();

}

void DrawShapeVisitor::apply(const Cone& cone)
{
    glPushMatrix();

    glTranslatef(cone.getCenter().x(),cone.getCenter().y(),cone.getCenter().z());

    if (!cone.zeroRotation())
    {
        Matrix rotation(cone.computeRotationMatrix());
        glMultMatrix(rotation.ptr());
    }

    // evaluate hints
    bool createBody = (_hints ? _hints->getCreateBody() : true);
    bool createBottom = (_hints ? _hints->getCreateBottom() : true);

    unsigned int numSegments = 40;
    unsigned int numRows = 10;
    float ratio = (_hints ? _hints->getDetailRatio() : 1.0f);
    if (ratio > 0.0f && ratio != 1.0f) {
        numRows = (unsigned int) (numRows * ratio);
        if (numRows < MIN_NUM_ROWS)
            numRows = MIN_NUM_ROWS;
        numSegments = (unsigned int) (numSegments * ratio);
        if (numSegments < MIN_NUM_SEGMENTS)
            numSegments = MIN_NUM_SEGMENTS;
    }

    float r = cone.getRadius();
    float h = cone.getHeight();

    float normalz = r/(sqrtf(r*r+h*h));
    float normalRatio = 1.0f/(sqrtf(1.0f+normalz*normalz));
    normalz *= normalRatio;

    float angleDelta = 2.0f*osg::PI/(float)numSegments;
    float texCoordHorzDelta = 1.0/(float)numSegments;
    float texCoordRowDelta = 1.0/(float)numRows;
    float hDelta = cone.getHeight()/(float)numRows;
    float rDelta = cone.getRadius()/(float)numRows;

    float topz=cone.getHeight()+cone.getBaseOffset();
    float topr=0.0f;
    float topv=1.0f;
    float basez=topz-hDelta;
    float baser=rDelta;
    float basev=topv-texCoordRowDelta;
    float angle;
    float texCoord;

    if (createBody) {
        for(unsigned int rowi=0; rowi<numRows;
            ++rowi,topz=basez, basez-=hDelta, topr=baser, baser+=rDelta, topv=basev, basev-=texCoordRowDelta) {
                // we can't use a fan for the cone top
                // since we need different normals at the top
                // for each face..
                glBegin(GL_QUAD_STRIP);

                angle = 0.0f;
                texCoord = 0.0f;
                for(unsigned int topi=0; topi<numSegments;
                    ++topi,angle+=angleDelta,texCoord+=texCoordHorzDelta) {

                    float c = cosf(angle);
                    float s = sinf(angle);

                    glNormal3f(c*normalRatio,s*normalRatio,normalz);

                    glTexCoord2f(texCoord,topv);
                    glVertex3f(c*topr,s*topr,topz);

                    glTexCoord2f(texCoord,basev);
                    glVertex3f(c*baser,s*baser,basez);
                }

                // do last point by hand to ensure no round off errors.
                glNormal3f(normalRatio,0.0f,normalz);

                glTexCoord2f(1.0f,topv);
                glVertex3f(topr,0.0f,topz);

                glTexCoord2f(1.0f,basev);
                glVertex3f(baser,0.0f,basez);

                glEnd();
        }
    }

    if (createBottom) {
        glBegin(GL_TRIANGLE_FAN);

        angle = osg::PI*2.0f;
        texCoord = 1.0f;
        basez = cone.getBaseOffset();

        glNormal3f(0.0f,0.0f,-1.0f);
        glTexCoord2f(0.5f,0.5f);
        glVertex3f(0.0f,0.0f,basez);

        for(unsigned int bottomi=0;bottomi<numSegments;
            ++bottomi,angle-=angleDelta,texCoord-=texCoordHorzDelta) {

            float c = cosf(angle);
            float s = sinf(angle);

            glTexCoord2f(c*0.5f+0.5f,s*0.5f+0.5f);
            glVertex3f(c*r,s*r,basez);
        }

        glTexCoord2f(1.0f,0.0f);
        glVertex3f(r,0.0f,basez);

        glEnd();
    }

    glPopMatrix();
}

void DrawShapeVisitor::apply(const Cylinder& cylinder)
{
    glPushMatrix();

    glTranslatef(cylinder.getCenter().x(),cylinder.getCenter().y(),cylinder.getCenter().z());

    if (!cylinder.zeroRotation())
    {
        Matrix rotation(cylinder.computeRotationMatrix());
        glMultMatrix(rotation.ptr());
    }

    // evaluate hints
    bool createBody = (_hints ? _hints->getCreateBody() : true);
    bool createTop = (_hints ? _hints->getCreateTop() : true);
    bool createBottom = (_hints ? _hints->getCreateBottom() : true);

    unsigned int numSegments = 40;
    float ratio = (_hints ? _hints->getDetailRatio() : 1.0f);
    if (ratio > 0.0f && ratio != 1.0f) {
        numSegments = (unsigned int) (numSegments * ratio);
        if (numSegments < MIN_NUM_SEGMENTS)
            numSegments = MIN_NUM_SEGMENTS;
    }


    // cylinder body
    if (createBody) 
        drawCylinderBody(numSegments, cylinder.getRadius(), cylinder.getHeight());

    float angleDelta = 2.0f*osg::PI/(float)numSegments;
    float texCoordDelta = 1.0f/(float)numSegments;

    float r = cylinder.getRadius();
    float h = cylinder.getHeight();

    float basez = -h*0.5f;
    float topz = h*0.5f;

    float angle = 0.0f;
    float texCoord = 0.0f;

    // cylinder top
    if (createTop) {
        glBegin(GL_TRIANGLE_FAN);

        glNormal3f(0.0f,0.0f,1.0f);
        glTexCoord2f(0.5f,0.5f);
        glVertex3f(0.0f,0.0f,topz);

        angle = 0.0f;
        texCoord = 0.0f;
        for(unsigned int topi=0;
            topi<numSegments;
            ++topi,angle+=angleDelta,texCoord+=texCoordDelta)
        {
            float c = cosf(angle);
            float s = sinf(angle);

            glTexCoord2f(c*0.5f+0.5f,s*0.5f+0.5f);
            glVertex3f(c*r,s*r,topz);
        }

        glTexCoord2f(1.0f,0.0f);
        glVertex3f(r,0.0f,topz);

        glEnd();
    }

    // cylinder bottom
    if (createBottom)
    {
        glBegin(GL_TRIANGLE_FAN);

        glNormal3f(0.0f,0.0f,-1.0f);
        glTexCoord2f(0.5f,0.5f);
        glVertex3f(0.0f,0.0f,basez);

        angle = osg::PI*2.0f;
        texCoord = 1.0f;
        for(unsigned int bottomi=0;
            bottomi<numSegments;
            ++bottomi,angle-=angleDelta,texCoord-=texCoordDelta)
        {
            float c = cosf(angle);
            float s = sinf(angle);

            glTexCoord2f(c*0.5f+0.5f,s*0.5f+0.5f);
            glVertex3f(c*r,s*r,basez);
        }

        glTexCoord2f(0.0f,0.0f);
        glVertex3f(r,0.0f,basez);

        glEnd();
    }

    glPopMatrix();
}

void DrawShapeVisitor::apply(const Capsule& capsule)
{
    glPushMatrix();

    glTranslatef(capsule.getCenter().x(),capsule.getCenter().y(),capsule.getCenter().z());

    if (!capsule.zeroRotation())
    {
        Matrix rotation(capsule.computeRotationMatrix());
        glMultMatrix(rotation.ptr());
    }

    // evaluate hints
    bool createBody = (_hints ? _hints->getCreateBody() : true);
    bool createTop = (_hints ? _hints->getCreateTop() : true);
    bool createBottom = (_hints ? _hints->getCreateBottom() : true);

    unsigned int numSegments = 40;
    unsigned int numRows = 20;
    float ratio = (_hints ? _hints->getDetailRatio() : 1.0f);
    if (ratio > 0.0f && ratio != 1.0f) {
        numSegments = (unsigned int) (numSegments * ratio);
        if (numSegments < MIN_NUM_SEGMENTS)
            numSegments = MIN_NUM_SEGMENTS;
        numRows = (unsigned int) (numRows * ratio);
        if (numRows < MIN_NUM_ROWS)
            numRows = MIN_NUM_ROWS;
    }


    // capsule cylindrical body
    if (createBody) 
        drawCylinderBody(numSegments, capsule.getRadius(), capsule.getHeight());

    // capsule top cap
    if (createTop) 
        drawHalfSphere(numSegments, numRows, capsule.getRadius(), SphereTopHalf, capsule.getHeight()/2.0f);

    // capsule bottom cap
    if (createBottom) 
        drawHalfSphere(numSegments, numRows, capsule.getRadius(), SphereBottomHalf, -capsule.getHeight()/2.0f);

    glPopMatrix();
}

void DrawShapeVisitor::apply(const InfinitePlane&)
{
    notify(NOTICE)<<"Warning: DrawShapeVisitor::apply(const InfinitePlane& plane) not yet implementated. "<<std::endl;
}

void DrawShapeVisitor::apply(const TriangleMesh& mesh)
{
    const Vec3Array* vertices = mesh.getVertices();
    const IndexArray* indices = mesh.getIndices();
    
     if (vertices && indices)
     {
        glBegin(GL_TRIANGLES);

        for(unsigned int i=0;i+2<indices->getNumElements();i+=3)
        {
            const osg::Vec3& v1=(*vertices)[indices->index(i)];
            const osg::Vec3& v2=(*vertices)[indices->index(i+1)];
            const osg::Vec3& v3=(*vertices)[indices->index(i+2)];
            Vec3 normal = (v2-v1)^(v3-v2);
            normal.normalize();

            glNormal3fv(normal.ptr());
            glVertex3fv(v1.ptr());
            glVertex3fv(v2.ptr());
            glVertex3fv(v3.ptr());

        }

        glEnd();
        }
}

void DrawShapeVisitor::apply(const ConvexHull& hull)
{
    apply((const TriangleMesh&)hull);
}

void DrawShapeVisitor::apply(const HeightField& field)
{
    if (field.getNumColumns()==0 || field.getNumRows()==0) return;
    
    glPushMatrix();

    glTranslatef(field.getOrigin().x(),field.getOrigin().y(),field.getOrigin().z());


    if (!field.zeroRotation())
    {
        Matrix rotation(field.computeRotationMatrix());
        glMultMatrix(rotation.ptr());
    }

    float dx = field.getXInterval();
    float dy = field.getYInterval();

    float du = 1.0f/((float)field.getNumColumns()-1.0f);
    float dv = 1.0f/((float)field.getNumRows()-1.0f);

    float vBase = 0.0f;

    Vec3 vertTop;
    Vec3 normTop;

    Vec3 vertBase;
    Vec3 normBase;

    if (field.getSkirtHeight()!=0.0f)
    {
        glBegin(GL_QUAD_STRIP);

        float u = 0.0f;

        // draw bottom skirt
        unsigned int col;
        vertTop.y() = 0.0f;
        for(col=0;col<field.getNumColumns();++col,u+=du)
        {
            vertTop.x() = dx*(float)col;
            vertTop.z() = field.getHeight(col,0);
            normTop.set(field.getNormal(col,0));

            glTexCoord2f(u,0.0f);
            glNormal3fv(normTop.ptr());

            glVertex3fv(vertTop.ptr());

            vertTop.z()-=field.getSkirtHeight();

            glVertex3fv(vertTop.ptr());
        }

        glEnd();

        // draw top skirt
        glBegin(GL_QUAD_STRIP);

        unsigned int row = field.getNumRows()-1;

        u = 0.0f;
        vertTop.y() = dy*(float)(row);
        for(col=0;col<field.getNumColumns();++col,u+=du)
        {
            vertTop.x() = dx*(float)col;
            vertTop.z() = field.getHeight(col,row);
            normTop.set(field.getNormal(col,row));

            glTexCoord2f(u,1.0f);
            glNormal3fv(normTop.ptr());

            glVertex3f(vertTop.x(),vertTop.y(),vertTop.z()-field.getSkirtHeight());

            //vertTop.z()-=field.getSkirtHeight();

            glVertex3fv(vertTop.ptr());
        }

        glEnd();
    }



    for(unsigned int row=0;row<field.getNumRows()-1;++row,vBase+=dv)
    {

        float vTop = vBase+dv;
        float u = 0.0f;


        glBegin(GL_QUAD_STRIP);

        // draw skirt at begining if required.
        if (field.getSkirtHeight()!=0.0f)
        {
            vertTop.set(0.0f,dy*(float)row+dy,field.getHeight(0,row+1)-field.getSkirtHeight());
            normTop.set(field.getNormal(0,row+1));

            vertBase.set(0.0f,dy*(float)row,field.getHeight(0,row)-field.getSkirtHeight());
            normBase.set(field.getNormal(0,row));

            glTexCoord2f(u,vTop);
            glNormal3fv(normTop.ptr());
            glVertex3fv(vertTop.ptr());

            glTexCoord2f(u,vBase);
            glNormal3fv(normBase.ptr());
            glVertex3fv(vertBase.ptr());
        }

        for(unsigned int col=0;col<field.getNumColumns();++col,u+=du)
        {
            vertTop.set(dx*(float)col,dy*(float)row+dy,field.getHeight(col,row+1));
            normTop.set(field.getNormal(col,row+1));

            vertBase.set(dx*(float)col,dy*(float)row,field.getHeight(col,row));
            normBase.set(field.getNormal(col,row));

            glTexCoord2f(u,vTop);
            glNormal3fv(normTop.ptr());
            glVertex3fv(vertTop.ptr());

            glTexCoord2f(u,vBase);
            glNormal3fv(normBase.ptr());
            glVertex3fv(vertBase.ptr());

        }

        // draw skirt at end if required.
        if (field.getSkirtHeight()!=0.0f)
        {

            vertBase.z()-=field.getSkirtHeight();
            vertTop.z()-=field.getSkirtHeight();

            glTexCoord2f(u,vTop);
            glNormal3fv(normTop.ptr());
            glVertex3fv(vertTop.ptr());

            glTexCoord2f(u,vBase);
            glNormal3fv(normBase.ptr());
            glVertex3fv(vertBase.ptr());
        }

        glEnd();
    }


    glPopMatrix();

}

void DrawShapeVisitor::apply(const CompositeShape& group)
{
    for(unsigned int i=0;i<group.getNumChildren();++i)
    {
        group.getChild(i)->accept(*this);
    }
}




///////////////////////////////////////////////////////////////////////////////
//
// Compute bounding of shape
//

class ComputeBoundShapeVisitor : public ConstShapeVisitor
{
    public:
    
        ComputeBoundShapeVisitor(BoundingBox& bb):_bb(bb) {}

        virtual void apply(const Sphere&);
        virtual void apply(const Box&);
        virtual void apply(const Cone&);
        virtual void apply(const Cylinder&);
        virtual void apply(const Capsule&);
        virtual void apply(const InfinitePlane&);

        virtual void apply(const TriangleMesh&);
        virtual void apply(const ConvexHull&);
        virtual void apply(const HeightField&);

        virtual void apply(const CompositeShape&);

        BoundingBox&    _bb;
};


void ComputeBoundShapeVisitor::apply(const Sphere& sphere)
{
    Vec3 halfLengths(sphere.getRadius(),sphere.getRadius(),sphere.getRadius());
    _bb.expandBy(sphere.getCenter()-halfLengths);
    _bb.expandBy(sphere.getCenter()+halfLengths);
}

void ComputeBoundShapeVisitor::apply(const Box& box)
{
    if (box.zeroRotation())
    {
        _bb.expandBy(box.getCenter()-box.getHalfLengths());
        _bb.expandBy(box.getCenter()+box.getHalfLengths());
    }
    else
    {
        float x = box.getHalfLengths().x();
        float y = box.getHalfLengths().y();
        float z = box.getHalfLengths().z();

        Vec3 base_1(Vec3(-x,-y,-z));
        Vec3 base_2(Vec3(x,-y,-z));
        Vec3 base_3(Vec3(x,y,-z));
        Vec3 base_4(Vec3(-x,y,-z));

        Vec3 top_1(Vec3(-x,-y,z));
        Vec3 top_2(Vec3(x,-y,z));
        Vec3 top_3(Vec3(x,y,z));
        Vec3 top_4(Vec3(-x,y,z));

        Matrix matrix = box.computeRotationMatrix();
        _bb.expandBy(box.getCenter()+base_1*matrix);
        _bb.expandBy(box.getCenter()+base_2*matrix);
        _bb.expandBy(box.getCenter()+base_3*matrix);
        _bb.expandBy(box.getCenter()+base_4*matrix);

        _bb.expandBy(box.getCenter()+top_1*matrix);
        _bb.expandBy(box.getCenter()+top_2*matrix);
        _bb.expandBy(box.getCenter()+top_3*matrix);
        _bb.expandBy(box.getCenter()+top_4*matrix);
    }
}

void ComputeBoundShapeVisitor::apply(const Cone& cone)
{
    if (cone.zeroRotation())
    {
        _bb.expandBy(cone.getCenter()+Vec3(-cone.getRadius(),-cone.getRadius(),cone.getBaseOffset()));
        _bb.expandBy(cone.getCenter()+Vec3(cone.getRadius(),cone.getRadius(),cone.getHeight()+cone.getBaseOffset()));

    }
    else
    {
        Vec3 top(Vec3(cone.getRadius(),cone.getRadius(),cone.getHeight()+cone.getBaseOffset()));
        Vec3 base_1(Vec3(-cone.getRadius(),-cone.getRadius(),cone.getBaseOffset()));
        Vec3 base_2(Vec3(cone.getRadius(),-cone.getRadius(),cone.getBaseOffset()));
        Vec3 base_3(Vec3(cone.getRadius(),cone.getRadius(),cone.getBaseOffset()));
        Vec3 base_4(Vec3(-cone.getRadius(),cone.getRadius(),cone.getBaseOffset()));

        Matrix matrix = cone.computeRotationMatrix();
        _bb.expandBy(cone.getCenter()+base_1*matrix);
        _bb.expandBy(cone.getCenter()+base_2*matrix);
        _bb.expandBy(cone.getCenter()+base_3*matrix);
        _bb.expandBy(cone.getCenter()+base_4*matrix);
        _bb.expandBy(cone.getCenter()+top*matrix);
    }
}

void ComputeBoundShapeVisitor::apply(const Cylinder& cylinder)
{
    if (cylinder.zeroRotation())
    {
        Vec3 halfLengths(cylinder.getRadius(),cylinder.getRadius(),cylinder.getHeight()*0.5f);
        _bb.expandBy(cylinder.getCenter()-halfLengths);
        _bb.expandBy(cylinder.getCenter()+halfLengths);

    }
    else
    {
        float r = cylinder.getRadius();
        float z = cylinder.getHeight()*0.5f;

        Vec3 base_1(Vec3(-r,-r,-z));
        Vec3 base_2(Vec3(r,-r,-z));
        Vec3 base_3(Vec3(r,r,-z));
        Vec3 base_4(Vec3(-r,r,-z));

        Vec3 top_1(Vec3(-r,-r,z));
        Vec3 top_2(Vec3(r,-r,z));
        Vec3 top_3(Vec3(r,r,z));
        Vec3 top_4(Vec3(-r,r,z));

        Matrix matrix = cylinder.computeRotationMatrix();
        _bb.expandBy(cylinder.getCenter()+base_1*matrix);
        _bb.expandBy(cylinder.getCenter()+base_2*matrix);
        _bb.expandBy(cylinder.getCenter()+base_3*matrix);
        _bb.expandBy(cylinder.getCenter()+base_4*matrix);

        _bb.expandBy(cylinder.getCenter()+top_1*matrix);
        _bb.expandBy(cylinder.getCenter()+top_2*matrix);
        _bb.expandBy(cylinder.getCenter()+top_3*matrix);
        _bb.expandBy(cylinder.getCenter()+top_4*matrix);
    }
}

void ComputeBoundShapeVisitor::apply(const Capsule& capsule)
{
    if (capsule.zeroRotation())
    {
        Vec3 halfLengths(capsule.getRadius(),capsule.getRadius(),capsule.getHeight()*0.5f + capsule.getRadius());
        _bb.expandBy(capsule.getCenter()-halfLengths);
        _bb.expandBy(capsule.getCenter()+halfLengths);

    }
    else
    {
        float r = capsule.getRadius();
        float z = capsule.getHeight()*0.5f + capsule.getRadius();

        Vec3 base_1(Vec3(-r,-r,-z));
        Vec3 base_2(Vec3(r,-r,-z));
        Vec3 base_3(Vec3(r,r,-z));
        Vec3 base_4(Vec3(-r,r,-z));

        Vec3 top_1(Vec3(-r,-r,z));
        Vec3 top_2(Vec3(r,-r,z));
        Vec3 top_3(Vec3(r,r,z));
        Vec3 top_4(Vec3(-r,r,z));

        Matrix matrix = capsule.computeRotationMatrix();
        _bb.expandBy(capsule.getCenter()+base_1*matrix);
        _bb.expandBy(capsule.getCenter()+base_2*matrix);
        _bb.expandBy(capsule.getCenter()+base_3*matrix);
        _bb.expandBy(capsule.getCenter()+base_4*matrix);

        _bb.expandBy(capsule.getCenter()+top_1*matrix);
        _bb.expandBy(capsule.getCenter()+top_2*matrix);
        _bb.expandBy(capsule.getCenter()+top_3*matrix);
        _bb.expandBy(capsule.getCenter()+top_4*matrix);
    }
}

void ComputeBoundShapeVisitor::apply(const InfinitePlane&)
{
    // can't compute the bounding box of an infinite plane!!! :-)
}

void ComputeBoundShapeVisitor::apply(const TriangleMesh& mesh)
{
    const Vec3Array* vertices = mesh.getVertices();
    const IndexArray* indices = mesh.getIndices();
    
    if (vertices && indices)
    {
        for(unsigned int i=0;i<indices->getNumElements();++i)
        {
            const osg::Vec3& v=(*vertices)[indices->index(i)];
            _bb.expandBy(v);
        }
    }
}

void ComputeBoundShapeVisitor::apply(const ConvexHull& hull)
{
    apply((const TriangleMesh&)hull);
}

void ComputeBoundShapeVisitor::apply(const HeightField& field)
{
    float zMin=FLT_MAX;
    float zMax=-FLT_MAX;

    for(unsigned int row=0;row<field.getNumRows();++row)
    {
        for(unsigned int col=0;col<field.getNumColumns();++col)
        {
            float z = field.getHeight(col,row);
            if (z<zMin) zMin = z;
            if (z>zMax) zMax = z;
        }
    }

    if (zMin>zMax)
    {
        // no valid entries so don't reset the bounding box
        return;
    }


    if (field.zeroRotation())
    {
        _bb.expandBy(field.getOrigin()+osg::Vec3(0.0f,0.0f,zMin));
        _bb.expandBy(field.getOrigin()+osg::Vec3(field.getXInterval()*(field.getNumColumns()-1),field.getYInterval()*(field.getNumRows()-1),zMax));
    }
    else
    {
        float x = field.getXInterval()*(field.getNumColumns()-1);
        float y = field.getYInterval()*(field.getNumRows()-1);

        Vec3 base_1(Vec3(0,0,zMin));
        Vec3 base_2(Vec3(x,0,zMin));
        Vec3 base_3(Vec3(x,y,zMin));
        Vec3 base_4(Vec3(0,y,zMin));

        Vec3 top_1(Vec3(0,0,zMax));
        Vec3 top_2(Vec3(x,0,zMax));
        Vec3 top_3(Vec3(x,y,zMax));
        Vec3 top_4(Vec3(0,y,zMax));

        Matrix matrix = field.computeRotationMatrix();
        _bb.expandBy(field.getOrigin()+base_1*matrix);
        _bb.expandBy(field.getOrigin()+base_2*matrix);
        _bb.expandBy(field.getOrigin()+base_3*matrix);
        _bb.expandBy(field.getOrigin()+base_4*matrix);

        _bb.expandBy(field.getOrigin()+top_1*matrix);
        _bb.expandBy(field.getOrigin()+top_2*matrix);
        _bb.expandBy(field.getOrigin()+top_3*matrix);
        _bb.expandBy(field.getOrigin()+top_4*matrix);
    }

}

void ComputeBoundShapeVisitor::apply(const CompositeShape& group)
{
    for(unsigned int i=0;i<group.getNumChildren();++i)
    {
        group.getChild(i)->accept(*this);
    }
}




///////////////////////////////////////////////////////////////////////////////
//
// Accept a primtive functor for each of the shapes.
//

class PrimitiveShapeVisitor : public ConstShapeVisitor
{
    public:
    
        PrimitiveShapeVisitor(PrimitiveFunctor& functor,const TessellationHints* hints):
            _functor(functor),
            _hints(hints) {}

        virtual void apply(const Sphere&);
        virtual void apply(const Box&);
        virtual void apply(const Cone&);
        virtual void apply(const Cylinder&);
        virtual void apply(const Capsule&);
        virtual void apply(const InfinitePlane&);

        virtual void apply(const TriangleMesh&);
        virtual void apply(const ConvexHull&);
        virtual void apply(const HeightField&);

        virtual void apply(const CompositeShape&);

        PrimitiveFunctor& _functor;
        const TessellationHints*  _hints;
};




void PrimitiveShapeVisitor::apply(const Sphere& sphere)
{

    float tx = sphere.getCenter().x();
    float ty = sphere.getCenter().y();
    float tz = sphere.getCenter().z();

    unsigned int numSegments = 40;
    unsigned int numRows = 20;

    float lDelta = osg::PI/(float)numRows;
    float vDelta = 1.0f/(float)numRows;

    float angleDelta = osg::PI*2.0f/(float)numSegments;
    float texCoordHorzDelta = 1.0f/(float)numSegments;

    float lBase=-osg::PI*0.5f;
    float rBase=0.0f;
    float zBase=-sphere.getRadius();
    float vBase=0.0f;

    for(unsigned int rowi=0;
        rowi<numRows;
        ++rowi)
    {

        float lTop = lBase+lDelta;
        float rTop = cosf(lTop)*sphere.getRadius();
        float zTop = sinf(lTop)*sphere.getRadius();
        float vTop = vBase+vDelta;
        //float nzTop= sinf(lTop);
        //float nRatioTop= cosf(lTop);

        _functor.begin(GL_QUAD_STRIP);

            float angle = 0.0f;
            float texCoord = 0.0f;

            for(unsigned int topi=0;
                topi<numSegments;
                ++topi,angle+=angleDelta,texCoord+=texCoordHorzDelta)
            {

                float c = cosf(angle);
                float s = sinf(angle);

                _functor.vertex(tx+c*rTop,ty+s*rTop,tz+zTop);
                _functor.vertex(tx+c*rBase,ty+s*rBase,tz+zBase);

            }

            // do last point by hand to ensure no round off errors.
            _functor.vertex(tx+rTop,ty,tz+zTop);
            _functor.vertex(tx+rBase,ty,tz+zBase);

        _functor.end();

        lBase=lTop;
        rBase=rTop;
        zBase=zTop;
        vBase=vTop;

    }
}

void PrimitiveShapeVisitor::apply(const Box& box)
{


    float x = box.getHalfLengths().x();
    float y = box.getHalfLengths().y();
    float z = box.getHalfLengths().z();

    Vec3 base_1(-x,-y,-z);
    Vec3 base_2(x,-y,-z);
    Vec3 base_3(x,y,-z);
    Vec3 base_4(-x,y,-z);

    Vec3 top_1(-x,-y,z);
    Vec3 top_2(x,-y,z);
    Vec3 top_3(x,y,z);
    Vec3 top_4(-x,y,z);

    if (box.zeroRotation())
    {
        base_1 += box.getCenter();
        base_2 += box.getCenter();
        base_3 += box.getCenter();
        base_4 += box.getCenter();

        top_1 += box.getCenter();
        top_2 += box.getCenter();
        top_3 += box.getCenter();
        top_4 += box.getCenter();
    }
    else
    {
        Matrix matrix = box.computeRotationMatrix();
        matrix.setTrans(box.getCenter());
        
        base_1 = base_1*matrix;
        base_2 = base_2*matrix;
        base_3 = base_3*matrix;
        base_4 = base_4*matrix;

        top_1 = top_1*matrix;
        top_2 = top_2*matrix;
        top_3 = top_3*matrix;
        top_4 = top_4*matrix;
    }

    _functor.begin(GL_QUADS);
    
    _functor.vertex(top_1);
    _functor.vertex(base_1);
    _functor.vertex(base_2);
    _functor.vertex(top_2);

    _functor.vertex(top_2);
    _functor.vertex(base_2);
    _functor.vertex(base_3);
    _functor.vertex(top_3);

    _functor.vertex(top_3);
    _functor.vertex(base_3);
    _functor.vertex(base_4);
    _functor.vertex(top_4);

    _functor.vertex(top_4);
    _functor.vertex(base_4);
    _functor.vertex(base_1);
    _functor.vertex(top_1);

    _functor.vertex(top_4);
    _functor.vertex(top_1);
    _functor.vertex(top_2);
    _functor.vertex(top_3);

    _functor.vertex(base_4);
    _functor.vertex(base_3);
    _functor.vertex(base_2);
    _functor.vertex(base_1);

    _functor.end();

}

void PrimitiveShapeVisitor::apply(const Cone& cone)
{
    Matrix matrix = cone.computeRotationMatrix();
    matrix.setTrans(cone.getCenter());


    unsigned int numSegments = 40;
    unsigned int numRows = 10;

    float r = cone.getRadius();
    float h = cone.getHeight();

    float normalz = r/(sqrtf(r*r+h*h));
    float normalRatio = 1.0f/(sqrtf(1.0f+normalz*normalz));
    normalz *= normalRatio;

    float angleDelta = 2.0f*osg::PI/(float)numSegments;
    float texCoordHorzDelta = 1.0/(float)numSegments;
    float texCoordRowDelta = 1.0/(float)numRows;
    float hDelta = cone.getHeight()/(float)numRows;
    float rDelta = cone.getRadius()/(float)numRows;

    float topz=cone.getHeight()+cone.getBaseOffset();
    float topr=0.0f;
    float topv=1.0f;
    float basez=topz-hDelta;
    float baser=rDelta;
    float basev=topv-texCoordRowDelta;
    float angle;
    float texCoord;

    for(unsigned int rowi=0;
        rowi<numRows;
        ++rowi,topz=basez, basez-=hDelta, topr=baser, baser+=rDelta, topv=basev, basev-=texCoordRowDelta)
    {
        // we can't use a fan for the cone top
        // since we need different normals at the top
        // for each face..
        _functor.begin(GL_QUAD_STRIP);

            angle = 0.0f;
            texCoord = 0.0f;
            for(unsigned int topi=0;
                topi<numSegments;
                ++topi,angle+=angleDelta,texCoord+=texCoordHorzDelta)
            {

                float c = cosf(angle);
                float s = sinf(angle);

                _functor.vertex(Vec3(c*topr,s*topr,topz)*matrix);
                _functor.vertex(Vec3(c*baser,s*baser,basez)*matrix);

            }

            // do last point by hand to ensure no round off errors.
            _functor.vertex(Vec3(topr,0.0f,topz)*matrix);
            _functor.vertex(Vec3(baser,0.0f,basez)*matrix);

        _functor.end();

    }

    // we can't use a fan for the cone top
    // since we need different normals at the top
    // for each face..
    _functor.begin(GL_TRIANGLE_FAN);

    angle = osg::PI*2.0f;
    texCoord = 1.0f;
    basez = cone.getBaseOffset();

    _functor.vertex(Vec3(0.0f,0.0f,basez)*matrix);

    for(unsigned int bottomi=0;
        bottomi<numSegments;
        ++bottomi,angle-=angleDelta,texCoord-=texCoordHorzDelta)
    {

        float c = cosf(angle);
        float s = sinf(angle);

        _functor.vertex(Vec3(c*r,s*r,basez)*matrix);

    }

    _functor.vertex(Vec3(r,0.0f,basez)*matrix);

    _functor.end();
}

void PrimitiveShapeVisitor::apply(const Cylinder& cylinder)
{
    Matrix matrix = cylinder.computeRotationMatrix();
    matrix.setTrans(cylinder.getCenter());

    unsigned int numSegments = 40;

    float angleDelta = 2.0f*osg::PI/(float)numSegments;

    float texCoordDelta = 1.0/(float)numSegments;

    float r = cylinder.getRadius();
    float h = cylinder.getHeight();

    float basez = -h*0.5f;
    float topz = h*0.5f;

    // cylinder body
    _functor.begin(GL_QUAD_STRIP);

        float angle = 0.0f;
        float texCoord = 0.0f;
        for(unsigned int bodyi=0;
            bodyi<numSegments;
            ++bodyi,angle+=angleDelta,texCoord+=texCoordDelta)
        {

            float c = cosf(angle);
            float s = sinf(angle);

            _functor.vertex(Vec3(c*r,s*r,topz)*matrix);
            _functor.vertex(Vec3(c*r,s*r,basez)*matrix);

        }

        // do last point by hand to ensure no round off errors.
        _functor.vertex(Vec3(r,0.0f,topz)*matrix);
        _functor.vertex(Vec3(r,0.0f,basez)*matrix);

    _functor.end();


    // cylinder top
    _functor.begin(GL_TRIANGLE_FAN);

        _functor.vertex(Vec3(0.0f,0.0f,topz)*matrix);

        angle = 0.0f;
        texCoord = 0.0f;
        for(unsigned int topi=0;
            topi<numSegments;
            ++topi,angle+=angleDelta,texCoord+=texCoordDelta)
        {

            float c = cosf(angle);
            float s = sinf(angle);

            _functor.vertex(Vec3(c*r,s*r,topz)*matrix);

        }

        _functor.vertex(Vec3(r,0.0f,topz)*matrix);

    _functor.end();

    // cylinder bottom
    _functor.begin(GL_TRIANGLE_FAN);

        _functor.vertex(Vec3(0.0f,0.0f,basez)*matrix);

        angle = osg::PI*2.0f;
        texCoord = 1.0f;
        for(unsigned int bottomi=0;
            bottomi<numSegments;
            ++bottomi,angle-=angleDelta,texCoord-=texCoordDelta)
        {

            float c = cosf(angle);
            float s = sinf(angle);

            _functor.vertex(Vec3(c*r,s*r,basez)*matrix);

        }

        _functor.vertex(Vec3(r,0.0f,basez)*matrix);

    _functor.end();
}

void PrimitiveShapeVisitor::apply(const Capsule& /*capsule*/)
{
#if 0
     notify(NOTICE)<<"Warning: PrimitiveShapeVisitor doesn't implement apply(Capsule&) (yet)."<<std::endl;
#endif   
}

void PrimitiveShapeVisitor::apply(const InfinitePlane&)
{
    notify(NOTICE)<<"Warning: PrimitiveShapeVisitor::apply(const InfinitePlane& plane) not yet implementated. "<<std::endl;
}

void PrimitiveShapeVisitor::apply(const TriangleMesh& mesh)
{
    const Vec3Array* vertices = mesh.getVertices();
    const IndexArray* indices = mesh.getIndices();
    
     if (vertices && indices)
     {
        _functor.begin(GL_TRIANGLES);

        for(unsigned int i=0;i<indices->getNumElements();i+=3)
        {
            _functor.vertex((*vertices)[indices->index(i)]);
            _functor.vertex((*vertices)[indices->index(i+1)]);
            _functor.vertex((*vertices)[indices->index(i+2)]);
        }

        _functor.end();
     }


}

void PrimitiveShapeVisitor::apply(const ConvexHull& hull)
{
    apply((const TriangleMesh&)hull);
}

void PrimitiveShapeVisitor::apply(const HeightField& field)
{
    if (field.getNumColumns()==0 || field.getNumRows()==0) return;
    
    Matrix matrix = field.computeRotationMatrix();
    matrix.setTrans(field.getOrigin());

    float dx = field.getXInterval();
    float dy = field.getYInterval();

    for(unsigned int row=0;row<field.getNumRows()-1;++row)
    {

        _functor.begin(GL_QUAD_STRIP);

        for(unsigned int col=0;col<field.getNumColumns();++col)
        {
            Vec3 vertTop(dx*(float)col,dy*(float)row+dy,field.getHeight(col,row+1));
            Vec3 vertBase(dx*(float)col,dy*(float)row,field.getHeight(col,row));

            _functor.vertex(vertTop*matrix);
            _functor.vertex(vertBase*matrix);

        }

        _functor.end();
    }

}

void PrimitiveShapeVisitor::apply(const CompositeShape& group)
{
    for(unsigned int i=0;i<group.getNumChildren();++i)
    {
        group.getChild(i)->accept(*this);
    }
}


///////////////////////////////////////////////////////////////////////////////
//
// ShapeDrawable itself..
//


ShapeDrawable::ShapeDrawable():
    _color(1.0f,1.0f,1.0f,1.0f)
{
}

ShapeDrawable::ShapeDrawable(Shape* shape,TessellationHints* hints):
  _color(1.0f,1.0f,1.0f,1.0f),
  _tessellationHints(hints)
{
    setShape(shape);
}

ShapeDrawable::ShapeDrawable(const ShapeDrawable& pg,const CopyOp& copyop):
    Drawable(pg,copyop),
    _color(pg._color),
    _tessellationHints(pg._tessellationHints)
{
}

ShapeDrawable::~ShapeDrawable()
{
}

void ShapeDrawable::setColor(const Vec4& color)
{
    if (_color!=color)
    {
        _color = color; dirtyDisplayList();
    }
}

void ShapeDrawable::setTessellationHints(TessellationHints* hints)
{
    if (_tessellationHints!=hints) 
    {
        _tessellationHints = hints; 
        dirtyDisplayList();
    }
}

void ShapeDrawable::drawImplementation(State& state) const
{
    if (_shape.valid())
    {
        glColor4fv(_color.ptr());
    
        DrawShapeVisitor dsv(state,_tessellationHints.get());
        
        _shape->accept(dsv);
    }
}

void ShapeDrawable::accept(ConstAttributeFunctor&) const
{
}

void ShapeDrawable::accept(PrimitiveFunctor& pf) const
{
    if (_shape.valid())
    {
        PrimitiveShapeVisitor psv(pf,_tessellationHints.get());
        _shape->accept(psv);
    }
}


BoundingBox ShapeDrawable::computeBound() const
{
    BoundingBox bbox;
    if (_shape.valid())
    {
        ComputeBoundShapeVisitor cbsv(bbox);
        _shape->accept(cbsv);
    }
    return bbox;
}

