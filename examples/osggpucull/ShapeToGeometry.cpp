/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2014 Robert Osfield
 *  Copyright (C) 2014 Pawel Ksiezopolski
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
 *
*/
#include "ShapeToGeometry.h"
#include <osg/Matrix>


FakeGLBeginEndAdapter::FakeGLBeginEndAdapter()
    : osg::GLBeginEndAdapter(NULL)
{
    geometry = new osg::Geometry;
}

void FakeGLBeginEndAdapter::PushMatrix()
{
    if (_matrixStack.empty())
        _matrixStack.push_back(osg::Matrixd());
    else
        _matrixStack.push_back(_matrixStack.back());
}
void FakeGLBeginEndAdapter::MultMatrixd(const GLdouble* m)
{
    if (_matrixStack.empty())
        _matrixStack.push_back(osg::Matrixd());
    _matrixStack.back().preMult(osg::Matrixd(m));
}
void FakeGLBeginEndAdapter::Translated(GLdouble x, GLdouble y, GLdouble z)
{
    if (_matrixStack.empty())
        _matrixStack.push_back(osg::Matrixd());
    _matrixStack.back().preMultTranslate(osg::Vec3d(x,y,z));
}

void FakeGLBeginEndAdapter::Scaled(GLdouble x, GLdouble y, GLdouble z)
{
    if (_matrixStack.empty())
    {
        _matrixStack.push_back(osg::Matrixd());
    }
    _matrixStack.back().preMultScale(osg::Vec3d(x,y,z));
}

void FakeGLBeginEndAdapter::Rotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z)
{
    if (_matrixStack.empty())
        _matrixStack.push_back(osg::Matrixd());
    _matrixStack.back().preMultRotate(osg::Quat(osg::DegreesToRadians(angle), osg::Vec3d(x,y,z)));
}

void FakeGLBeginEndAdapter::End()
{
    if (!_vertices || _vertices->empty()) return;

    if (!_matrixStack.empty())
    {
        const osg::Matrixd& matrix = _matrixStack.back();
        osg::Matrixd inverse;
        inverse.invert(matrix);

        for(osg::Vec3Array::iterator itr = _vertices->begin();
            itr != _vertices->end();
            ++itr)
        {
            *itr = *itr * matrix;
        }

        if (_normalAssigned && _normals.valid())
        {
            for(osg::Vec3Array::iterator itr = _normals->begin();
                itr != _normals->end();
                ++itr)
            {
                *itr = osg::Matrixd::transform3x3(inverse, *itr);
                (*itr).normalize();
            }
        }
        else
        {
            _overallNormal = osg::Matrixd::transform3x3(inverse, _overallNormal);
            _overallNormal.normalize();
        }
    }


    if (_colorAssigned)
    {
        if(geometry->getColorArray() == NULL )
            geometry->setColorArray( new osg::Vec4Array, osg::Array::BIND_PER_VERTEX );
        osg::Vec4Array* gColors = dynamic_cast<osg::Vec4Array*>(geometry->getColorArray());
        gColors->insert( gColors->end(), _colors->begin(), _colors->end() );
    }
    else if (_overallColorAssigned)
    {
        if(geometry->getColorArray() == NULL )
            geometry->setColorArray( new osg::Vec4Array, osg::Array::BIND_PER_VERTEX );
        osg::Vec4Array* gColors=dynamic_cast<osg::Vec4Array*>(geometry->getColorArray());
        gColors->insert( gColors->end(), _vertices->size(), _overallColor );
    }

    if (_normalAssigned)
    {
        if(geometry->getNormalArray() == NULL )
            geometry->setNormalArray( new osg::Vec3Array, osg::Array::BIND_PER_VERTEX );
        osg::Vec3Array* gNormals = dynamic_cast<osg::Vec3Array*>(geometry->getNormalArray());
        gNormals->insert( gNormals->end(), _normals->begin(), _normals->end() );
    }
    else if (_overallNormalAssigned)
    {
        if(geometry->getNormalArray() == NULL )
            geometry->setNormalArray( new osg::Vec3Array, osg::Array::BIND_PER_VERTEX );
        osg::Vec3Array* gNormals = dynamic_cast<osg::Vec3Array*>(geometry->getNormalArray());
        gNormals->insert( gNormals->end(), _vertices->size(), _overallNormal );
    }

    for(unsigned int unit=0; unit<_texCoordAssignedList.size(); ++unit)
    {
        if (_texCoordAssignedList[unit] && _texCoordsList[unit].valid())
        {
            if(geometry->getTexCoordArray(unit) == NULL )
                geometry->setTexCoordArray( unit, new osg::Vec4Array, osg::Array::BIND_PER_VERTEX );
            osg::Vec4Array* gTexCoords = dynamic_cast<osg::Vec4Array*>(geometry->getTexCoordArray(unit));
            gTexCoords->insert( gTexCoords->end(), _texCoordsList[unit]->begin(), _texCoordsList[unit]->end() );
        }
    }

    for(unsigned int unit=0; unit<_vertexAttribAssignedList.size(); ++unit)
    {
        if (_vertexAttribAssignedList[unit] && _vertexAttribsList[unit].valid())
        {
            if(geometry->getVertexAttribArray(unit) == NULL )
                geometry->setVertexAttribArray( unit, new osg::Vec4Array, osg::Array::BIND_PER_VERTEX );
            osg::Vec4Array* gVertexAttribs = dynamic_cast<osg::Vec4Array*>(geometry->getVertexAttribArray(unit));
            gVertexAttribs->insert( gVertexAttribs->end(), _vertexAttribsList[unit]->begin(), _vertexAttribsList[unit]->end() );
        }
    }

    if(geometry->getVertexArray() == NULL )
        geometry->setVertexArray( new osg::Vec3Array );
    osg::Vec3Array* gVertices = dynamic_cast<osg::Vec3Array*>(geometry->getVertexArray());
    unsigned int vOffset = gVertices->size();
    unsigned int vSize = _vertices->size();
    gVertices->insert( gVertices->end(), _vertices->begin(), _vertices->end() );

    if (_primitiveMode==GL_QUAD_STRIP) // will the winding be wrong? Do we need to swap it?
        geometry->addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLE_STRIP, vOffset, vSize ) );
    else if (_primitiveMode==GL_POLYGON)
        geometry->addPrimitiveSet( new osg::DrawArrays( GL_TRIANGLE_FAN, vOffset, vSize ) );
    else
        geometry->addPrimitiveSet( new osg::DrawArrays( _primitiveMode, vOffset, vSize ) );
}

void ShapeToGeometryVisitor::drawCylinderBody(unsigned int numSegments, float radius, float height)
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

    gl.Begin(GL_QUAD_STRIP);

    if (drawFrontFace) {

      for(unsigned int bodyi=0;
          bodyi<numSegments;
          ++bodyi,angle+=angleDelta,texCoord+=texCoordDelta)
      {
          float c = cosf(angle);
          float s = sinf(angle);

          gl.Normal3f(c,s,0.0f);

          gl.TexCoord2f(texCoord,1.0f);
          gl.Vertex3f(c*r,s*r,topz);

          gl.TexCoord2f(texCoord,0.0f);
          gl.Vertex3f(c*r,s*r,basez);
      }

      // do last point by hand to ensure no round off errors.
      gl.Normal3f(1.0f,0.0f,0.0f);

      gl.TexCoord2f(1.0f,1.0f);
      gl.Vertex3f(r,0.0f,topz);

      gl.TexCoord2f(1.0f,0.0f);
      gl.Vertex3f(r,0.0f,basez);
    }

    if (drawBackFace) {
      for(unsigned int bodyi=0;
          bodyi<numSegments;
          ++bodyi,angle+=angleDelta,texCoord+=texCoordDelta)
      {
          float c = cosf(angle);
          float s = sinf(angle);

          gl.Normal3f(-c,-s,0.0f);

          gl.TexCoord2f(texCoord,0.0f);
          gl.Vertex3f(c*r,s*r,basez);

          gl.TexCoord2f(texCoord,1.0f);
          gl.Vertex3f(c*r,s*r,topz);
      }

      // do last point by hand to ensure no round off errors.
      gl.Normal3f(-1.0f,0.0f,0.0f);

      gl.TexCoord2f(1.0f,0.0f);
      gl.Vertex3f(r,0.0f,basez);

      gl.TexCoord2f(1.0f,1.0f);
      gl.Vertex3f(r,0.0f,topz);
    }

    gl.End();
}


void ShapeToGeometryVisitor::drawHalfSphere(unsigned int numSegments, unsigned int numRows, float radius, SphereHalf which, float zOffset)
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

        gl.Begin(GL_QUAD_STRIP);

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

                  gl.Normal3f(c*nRatioTop,s*nRatioTop,nzTop);

                  gl.TexCoord2f(texCoord,vTop);
                  gl.Vertex3f(c*rTop,s*rTop,zTop+zOffset);

                  gl.Normal3f(c*nRatioBase,s*nRatioBase,nzBase);

                  gl.TexCoord2f(texCoord,vBase);
                  gl.Vertex3f(c*rBase,s*rBase,zBase+zOffset);

              }

              // do last point by hand to ensure no round off errors.
              gl.Normal3f(nRatioTop,0.0f,nzTop);

              gl.TexCoord2f(1.0f,vTop);
              gl.Vertex3f(rTop,0.0f,zTop+zOffset);

              gl.Normal3f(nRatioBase,0.0f,nzBase);

              gl.TexCoord2f(1.0f,vBase);
              gl.Vertex3f(rBase,0.0f,zBase+zOffset);
            }

          if (drawBackFace) {
              for(unsigned int topi=0; topi<numSegments;
                  ++topi,angle+=angleDelta,texCoord+=texCoordHorzDelta)
              {

                  float c = cosf(angle);
                  float s = sinf(angle);

                  gl.Normal3f(-c*nRatioBase,-s*nRatioBase,-nzBase);

                  gl.TexCoord2f(texCoord,vBase);
                  gl.Vertex3f(c*rBase,s*rBase,zBase+zOffset);

                  gl.Normal3f(-c*nRatioTop,-s*nRatioTop,-nzTop);

                  gl.TexCoord2f(texCoord,vTop);
                  gl.Vertex3f(c*rTop,s*rTop,zTop+zOffset);
              }

              // do last point by hand to ensure no round off errors.
              gl.Normal3f(-nRatioBase,0.0f,-nzBase);

              gl.TexCoord2f(1.0f,vBase);
              gl.Vertex3f(rBase,0.0f,zBase+zOffset);

              gl.Normal3f(-nRatioTop,0.0f,-nzTop);

              gl.TexCoord2f(1.0f,vTop);
              gl.Vertex3f(rTop,0.0f,zTop+zOffset);

          }

        gl.End();

        lBase=lTop;
        rBase=rTop;
        zBase=zTop;
        vBase=vTop;
        nzBase=nzTop;
        nRatioBase=nRatioTop;

    }
}


void ShapeToGeometryVisitor::apply(const osg::Sphere& sphere)
{
    gl.PushMatrix();

    gl.Translated(sphere.getCenter().x(),sphere.getCenter().y(),sphere.getCenter().z());

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

            gl.Begin(GL_QUAD_STRIP);

                float angle = 0.0f;
                float texCoord = 0.0f;

                for(unsigned int topi=0; topi<numSegments;
                    ++topi,angle+=angleDelta,texCoord+=texCoordHorzDelta)
                {

                    float c = cosf(angle);
                    float s = sinf(angle);

                    gl.Normal3f(-c*nRatioBase,-s*nRatioBase,-nzBase);

                    gl.TexCoord2f(texCoord,vBase);
                    gl.Vertex3f(c*rBase,s*rBase,zBase);

                    gl.Normal3f(-c*nRatioTop,-s*nRatioTop,-nzTop);

                    gl.TexCoord2f(texCoord,vTop);
                    gl.Vertex3f(c*rTop,s*rTop,zTop);


                }


                // do last point by hand to ensure no round off errors.
                gl.Normal3f(-nRatioBase,0.0f,-nzBase);

                gl.TexCoord2f(1.0f,vBase);
                gl.Vertex3f(rBase,0.0f,zBase);

                gl.Normal3f(-nRatioTop,0.0f,-nzTop);

                gl.TexCoord2f(1.0f,vTop);
                gl.Vertex3f(rTop,0.0f,zTop);

            gl.End();


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

            gl.Begin(GL_QUAD_STRIP);

                float angle = 0.0f;
                float texCoord = 0.0f;

                for(unsigned int topi=0; topi<numSegments;
                    ++topi,angle+=angleDelta,texCoord+=texCoordHorzDelta)
                {

                    float c = cosf(angle);
                    float s = sinf(angle);

                    gl.Normal3f(c*nRatioTop,s*nRatioTop,nzTop);

                    gl.TexCoord2f(texCoord,vTop);
                    gl.Vertex3f(c*rTop,s*rTop,zTop);

                    gl.Normal3f(c*nRatioBase,s*nRatioBase,nzBase);

                    gl.TexCoord2f(texCoord,vBase);
                    gl.Vertex3f(c*rBase,s*rBase,zBase);

                }

                // do last point by hand to ensure no round off errors.
                gl.Normal3f(nRatioTop,0.0f,nzTop);

                gl.TexCoord2f(1.0f,vTop);
                gl.Vertex3f(rTop,0.0f,zTop);

                gl.Normal3f(nRatioBase,0.0f,nzBase);

                gl.TexCoord2f(1.0f,vBase);
                gl.Vertex3f(rBase,0.0f,zBase);

            gl.End();


            lBase=lTop;
            rBase=rTop;
            zBase=zTop;
            vBase=vTop;
            nzBase=nzTop;
            nRatioBase=nRatioTop;

        }
    }

    gl.PopMatrix();
}

void ShapeToGeometryVisitor::apply(const osg::Box& box)
{
    // evaluate hints
    bool createBody = (_hints ? _hints->getCreateBody() : true);
    bool createTop = (_hints ? _hints->getCreateTop() : true);
    bool createBottom = (_hints ? _hints->getCreateBottom() : true);

    float dx = box.getHalfLengths().x();
    float dy = box.getHalfLengths().y();
    float dz = box.getHalfLengths().z();

    gl.PushMatrix();

    gl.Translated(box.getCenter().x(),box.getCenter().y(),box.getCenter().z());

    if (!box.zeroRotation())
    {
        osg::Matrixd rotation(box.computeRotationMatrix());
        gl.MultMatrixd(rotation.ptr());
    }

    gl.Begin(GL_QUADS);

    if (createBody) {
        // -ve y plane
        gl.Normal3f(0.0f,-1.0f,0.0f);

        gl.TexCoord2f(0.0f,1.0f);
        gl.Vertex3f(-dx,-dy,dz);

        gl.TexCoord2f(0.0f,0.0f);
        gl.Vertex3f(-dx,-dy,-dz);

        gl.TexCoord2f(1.0f,0.0f);
        gl.Vertex3f(dx,-dy,-dz);

        gl.TexCoord2f(1.0f,1.0f);
        gl.Vertex3f(dx,-dy,dz);

        // +ve y plane
        gl.Normal3f(0.0f,1.0f,0.0f);

        gl.TexCoord2f(0.0f,1.0f);
        gl.Vertex3f(dx,dy,dz);

        gl.TexCoord2f(0.0f,0.0f);
        gl.Vertex3f(dx,dy,-dz);

        gl.TexCoord2f(1.0f,0.0f);
        gl.Vertex3f(-dx,dy,-dz);

        gl.TexCoord2f(1.0f,1.0f);
        gl.Vertex3f(-dx,dy,dz);

        // +ve x plane
        gl.Normal3f(1.0f,0.0f,0.0f);

        gl.TexCoord2f(0.0f,1.0f);
        gl.Vertex3f(dx,-dy,dz);

        gl.TexCoord2f(0.0f,0.0f);
        gl.Vertex3f(dx,-dy,-dz);

        gl.TexCoord2f(1.0f,0.0f);
        gl.Vertex3f(dx,dy,-dz);

        gl.TexCoord2f(1.0f,1.0f);
        gl.Vertex3f(dx,dy,dz);

        // -ve x plane
        gl.Normal3f(-1.0f,0.0f,0.0f);

        gl.TexCoord2f(0.0f,1.0f);
        gl.Vertex3f(-dx,dy,dz);

        gl.TexCoord2f(0.0f,0.0f);
        gl.Vertex3f(-dx,dy,-dz);

        gl.TexCoord2f(1.0f,0.0f);
        gl.Vertex3f(-dx,-dy,-dz);

        gl.TexCoord2f(1.0f,1.0f);
        gl.Vertex3f(-dx,-dy,dz);
    }

    if (createTop) {
        // +ve z plane
        gl.Normal3f(0.0f,0.0f,1.0f);

        gl.TexCoord2f(0.0f,1.0f);
        gl.Vertex3f(-dx,dy,dz);

        gl.TexCoord2f(0.0f,0.0f);
        gl.Vertex3f(-dx,-dy,dz);

        gl.TexCoord2f(1.0f,0.0f);
        gl.Vertex3f(dx,-dy,dz);

        gl.TexCoord2f(1.0f,1.0f);
        gl.Vertex3f(dx,dy,dz);
    }

    if (createBottom) {
        // -ve z plane
        gl.Normal3f(0.0f,0.0f,-1.0f);

        gl.TexCoord2f(0.0f,1.0f);
        gl.Vertex3f(dx,dy,-dz);

        gl.TexCoord2f(0.0f,0.0f);
        gl.Vertex3f(dx,-dy,-dz);

        gl.TexCoord2f(1.0f,0.0f);
        gl.Vertex3f(-dx,-dy,-dz);

        gl.TexCoord2f(1.0f,1.0f);
        gl.Vertex3f(-dx,dy,-dz);
    }

    gl.End();

    gl.PopMatrix();

}

void ShapeToGeometryVisitor::apply(const osg::Cone& cone)
{
    gl.PushMatrix();

    gl.Translated(cone.getCenter().x(),cone.getCenter().y(),cone.getCenter().z());

    if (!cone.zeroRotation())
    {
        osg::Matrixd rotation(cone.computeRotationMatrix());
        gl.MultMatrixd(rotation.ptr());
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
                gl.Begin(GL_QUAD_STRIP);

                angle = 0.0f;
                texCoord = 0.0f;
                for(unsigned int topi=0; topi<numSegments;
                    ++topi,angle+=angleDelta,texCoord+=texCoordHorzDelta) {

                    float c = cosf(angle);
                    float s = sinf(angle);

                    gl.Normal3f(c*normalRatio,s*normalRatio,normalz);

                    gl.TexCoord2f(texCoord,topv);
                    gl.Vertex3f(c*topr,s*topr,topz);

                    gl.TexCoord2f(texCoord,basev);
                    gl.Vertex3f(c*baser,s*baser,basez);
                }

                // do last point by hand to ensure no round off errors.
                gl.Normal3f(normalRatio,0.0f,normalz);

                gl.TexCoord2f(1.0f,topv);
                gl.Vertex3f(topr,0.0f,topz);

                gl.TexCoord2f(1.0f,basev);
                gl.Vertex3f(baser,0.0f,basez);

                gl.End();
        }
    }

    if (createBottom) {
        gl.Begin(GL_TRIANGLE_FAN);

        angle = osg::PI*2.0f;
        texCoord = 1.0f;
        basez = cone.getBaseOffset();

        gl.Normal3f(0.0f,0.0f,-1.0f);
        gl.TexCoord2f(0.5f,0.5f);
        gl.Vertex3f(0.0f,0.0f,basez);

        for(unsigned int bottomi=0;bottomi<numSegments;
            ++bottomi,angle-=angleDelta,texCoord-=texCoordHorzDelta) {

            float c = cosf(angle);
            float s = sinf(angle);

            gl.TexCoord2f(c*0.5f+0.5f,s*0.5f+0.5f);
            gl.Vertex3f(c*r,s*r,basez);
        }

        gl.TexCoord2f(1.0f,0.0f);
        gl.Vertex3f(r,0.0f,basez);

        gl.End();
    }

    gl.PopMatrix();
}

void ShapeToGeometryVisitor::apply(const osg::Cylinder& cylinder)
{
    gl.PushMatrix();

    gl.Translated(cylinder.getCenter().x(),cylinder.getCenter().y(),cylinder.getCenter().z());

    if (!cylinder.zeroRotation())
    {
        osg::Matrixd rotation(cylinder.computeRotationMatrix());
        gl.MultMatrixd(rotation.ptr());
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
        gl.Begin(GL_TRIANGLE_FAN);

        gl.Normal3f(0.0f,0.0f,1.0f);
        gl.TexCoord2f(0.5f,0.5f);
        gl.Vertex3f(0.0f,0.0f,topz);

        angle = 0.0f;
        texCoord = 0.0f;
        for(unsigned int topi=0;
            topi<numSegments;
            ++topi,angle+=angleDelta,texCoord+=texCoordDelta)
        {
            float c = cosf(angle);
            float s = sinf(angle);

            gl.TexCoord2f(c*0.5f+0.5f,s*0.5f+0.5f);
            gl.Vertex3f(c*r,s*r,topz);
        }

        gl.TexCoord2f(1.0f,0.5f);
        gl.Vertex3f(r,0.0f,topz);

        gl.End();
    }

    // cylinder bottom
    if (createBottom)
    {
        gl.Begin(GL_TRIANGLE_FAN);

        gl.Normal3f(0.0f,0.0f,-1.0f);
        gl.TexCoord2f(0.5f,0.5f);
        gl.Vertex3f(0.0f,0.0f,basez);

        angle = osg::PI*2.0f;
        texCoord = 1.0f;
        for(unsigned int bottomi=0;
            bottomi<numSegments;
            ++bottomi,angle-=angleDelta,texCoord-=texCoordDelta)
        {
            float c = cosf(angle);
            float s = sinf(angle);

            gl.TexCoord2f(c*0.5f+0.5f,s*0.5f+0.5f);
            gl.Vertex3f(c*r,s*r,basez);
        }

        gl.TexCoord2f(1.0f,0.5f);
        gl.Vertex3f(r,0.0f,basez);

        gl.End();
    }

    gl.PopMatrix();
}

void ShapeToGeometryVisitor::apply(const osg::Capsule& capsule)
{
    gl.PushMatrix();

    gl.Translated(capsule.getCenter().x(),capsule.getCenter().y(),capsule.getCenter().z());

    if (!capsule.zeroRotation())
    {
        osg::Matrixd rotation(capsule.computeRotationMatrix());
        gl.MultMatrixd(rotation.ptr());
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

    // if numRows is odd the top and bottom halves of sphere won't match, so bump up to the next event numRows
    if ((numRows%2)!=0) ++numRows;


    // capsule cylindrical body
    if (createBody)
        drawCylinderBody(numSegments, capsule.getRadius(), capsule.getHeight());

    // capsule top cap
    if (createTop)
        drawHalfSphere(numSegments, numRows, capsule.getRadius(), SphereTopHalf, capsule.getHeight()/2.0f);

    // capsule bottom cap
    if (createBottom)
        drawHalfSphere(numSegments, numRows, capsule.getRadius(), SphereBottomHalf, -capsule.getHeight()/2.0f);

    gl.PopMatrix();
}

void ShapeToGeometryVisitor::apply(const osg::InfinitePlane&)
{
    OSG_NOTICE<<"Warning: ShapeToGeometryVisitor::apply(const InfinitePlane& plane) not yet implemented. "<<std::endl;
}

void ShapeToGeometryVisitor::apply(const osg::TriangleMesh& mesh)
{
    const osg::Vec3Array* vertices = mesh.getVertices();
    const osg::IndexArray* indices = mesh.getIndices();

     if (vertices && indices)
     {
        gl.Begin(GL_TRIANGLES);

        for(unsigned int i=0;i+2<indices->getNumElements();i+=3)
        {
            const osg::Vec3& v1=(*vertices)[indices->index(i)];
            const osg::Vec3& v2=(*vertices)[indices->index(i+1)];
            const osg::Vec3& v3=(*vertices)[indices->index(i+2)];
            osg::Vec3 normal = (v2-v1)^(v3-v2);
            normal.normalize();

            gl.Normal3fv(normal.ptr());
            gl.Vertex3fv(v1.ptr());
            gl.Vertex3fv(v2.ptr());
            gl.Vertex3fv(v3.ptr());

        }

        gl.End();
    }
}

void ShapeToGeometryVisitor::apply(const osg::ConvexHull& hull)
{
    apply((const osg::TriangleMesh&)hull);
}

void ShapeToGeometryVisitor::apply(const osg::HeightField& field)
{
    if (field.getNumColumns()==0 || field.getNumRows()==0) return;

    gl.PushMatrix();

    gl.Translated(field.getOrigin().x(),field.getOrigin().y(),field.getOrigin().z());


    if (!field.zeroRotation())
    {
        osg::Matrixd rotation(field.computeRotationMatrix());
        gl.MultMatrixd(rotation.ptr());
    }

    float dx = field.getXInterval();
    float dy = field.getYInterval();

    float du = 1.0f/((float)field.getNumColumns()-1.0f);
    float dv = 1.0f/((float)field.getNumRows()-1.0f);

    float vBase = 0.0f;

    osg::Vec3 vertTop;
    osg::Vec3 normTop;

    osg::Vec3 vertBase;
    osg::Vec3 normBase;

    if (field.getSkirtHeight()!=0.0f)
    {
        gl.Begin(GL_QUAD_STRIP);

        float u = 0.0f;

        // draw bottom skirt
        unsigned int col;
        vertTop.y() = 0.0f;
        for(col=0;col<field.getNumColumns();++col,u+=du)
        {
            vertTop.x() = dx*(float)col;
            vertTop.z() = field.getHeight(col,0);
            normTop.set(field.getNormal(col,0));

            gl.TexCoord2f(u,0.0f);
            gl.Normal3fv(normTop.ptr());

            gl.Vertex3fv(vertTop.ptr());

            vertTop.z()-=field.getSkirtHeight();

            gl.Vertex3fv(vertTop.ptr());
        }

        gl.End();

        // draw top skirt
        gl.Begin(GL_QUAD_STRIP);

        unsigned int row = field.getNumRows()-1;

        u = 0.0f;
        vertTop.y() = dy*(float)(row);
        for(col=0;col<field.getNumColumns();++col,u+=du)
        {
            vertTop.x() = dx*(float)col;
            vertTop.z() = field.getHeight(col,row);
            normTop.set(field.getNormal(col,row));

            gl.TexCoord2f(u,1.0f);
            gl.Normal3fv(normTop.ptr());

            gl.Vertex3f(vertTop.x(),vertTop.y(),vertTop.z()-field.getSkirtHeight());

            //vertTop.z()-=field.getSkirtHeight();

            gl.Vertex3fv(vertTop.ptr());
        }

        gl.End();
    }



    // draw each row of HeightField
    for(unsigned int row=0;row<field.getNumRows()-1;++row,vBase+=dv)
    {

        float vTop = vBase+dv;
        float u = 0.0f;


        gl.Begin(GL_QUAD_STRIP);

        // draw skirt at beginning of this row if required.
        if (field.getSkirtHeight()!=0.0f)
        {
            vertTop.set(0.0f,dy*(float)(row+1),field.getHeight(0,row+1)-field.getSkirtHeight());
            normTop.set(field.getNormal(0,row+1));

            vertBase.set(0.0f,dy*(float)row,field.getHeight(0,row)-field.getSkirtHeight());
            normBase.set(field.getNormal(0,row));

            gl.TexCoord2f(u,vTop);
            gl.Normal3fv(normTop.ptr());
            gl.Vertex3fv(vertTop.ptr());

            gl.TexCoord2f(u,vBase);
            gl.Normal3fv(normBase.ptr());
            gl.Vertex3fv(vertBase.ptr());
        }

        // draw the actual row
        for(unsigned int col=0;col<field.getNumColumns();++col,u+=du)
        {
            vertTop.set(dx*(float)col,dy*(float)(row+1),field.getHeight(col,row+1));
            normTop.set(field.getNormal(col,row+1));

            vertBase.set(dx*(float)col,dy*(float)row,field.getHeight(col,row));
            normBase.set(field.getNormal(col,row));

            gl.TexCoord2f(u,vTop);
            gl.Normal3fv(normTop.ptr());
            gl.Vertex3fv(vertTop.ptr());

            gl.TexCoord2f(u,vBase);
            gl.Normal3fv(normBase.ptr());
            gl.Vertex3fv(vertBase.ptr());

        }

        // draw skirt at end of this row if required.
        if (field.getSkirtHeight()!=0.0f)
        {

            vertBase.z()-=field.getSkirtHeight();
            vertTop.z()-=field.getSkirtHeight();

            gl.TexCoord2f(u,vTop);
            gl.Normal3fv(normTop.ptr());
            gl.Vertex3fv(vertTop.ptr());

            gl.TexCoord2f(u,vBase);
            gl.Normal3fv(normBase.ptr());
            gl.Vertex3fv(vertBase.ptr());
        }

        gl.End();
    }


    gl.PopMatrix();

}

void ShapeToGeometryVisitor::apply(const osg::CompositeShape& group)
{
    for(unsigned int i=0;i<group.getNumChildren();++i)
    {
        group.getChild(i)->accept(*this);
    }
}

osg::Geometry* convertShapeToGeometry(const osg::Shape& shape, const osg::TessellationHints* hints)
{
    osg::ref_ptr<osg::Geometry> geometry;
    {
        ShapeToGeometryVisitor gfsVisitor(hints);
        shape.accept( gfsVisitor );
        geometry = gfsVisitor.getGeometry();
    }
    return geometry.release();
}

osg::Geometry* convertShapeToGeometry(const osg::Shape& shape, const osg::TessellationHints* hints, const osg::Vec4& color)
{
    osg::ref_ptr<osg::Geometry> geometry = convertShapeToGeometry(shape,hints);
    osg::Vec4Array* colorArray = new osg::Vec4Array;
    colorArray->insert( colorArray->end(), geometry->getVertexArray()->getNumElements(), color );
    geometry->setColorArray( colorArray, osg::Array::BIND_PER_VERTEX );
    return geometry.release();
}


osg::Geode* convertShapeToGeode(const osg::Shape& shape, const osg::TessellationHints* hints)
{
    osg::Geode *geode = new osg::Geode;
    geode->addDrawable( convertShapeToGeometry(shape,hints) );
    return geode;
}

osg::Geode* convertShapeToGeode(const osg::Shape& shape, const osg::TessellationHints* hints, const osg::Vec4& color)
{
    osg::Geode *geode = new osg::Geode;
    geode->addDrawable( convertShapeToGeometry(shape,hints,color) );
    return geode;
}
