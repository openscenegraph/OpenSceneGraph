#include <osg/ShapeDrawable>
#include <osg/GL>

using namespace osg;

///////////////////////////////////////////////////////////////////////////////
//
// draw shape
//

class DrawShapeVisitor : public ConstShapeVisitor
{
    public:
    
    	DrawShapeVisitor(State& state,const TessellationHints* hints):
	    _state(state),
	    _hints(hints) {}
    
    	virtual void apply(const Sphere&);
    	virtual void apply(const Box&);
    	virtual void apply(const Cone&);
    	virtual void apply(const Cylinder&);
    	virtual void apply(const InfinitePlane&);

    	virtual void apply(const TriangleMesh&);
    	virtual void apply(const ConvexHull&);
    	virtual void apply(const HeightField&);

    	virtual void apply(const CompositeShape&);
	
	State&	    	    _state;
	const TessellationHints*  _hints;
};


void DrawShapeVisitor::apply(const Sphere& sphere)
{
    glPushMatrix();

	glTranslatef(sphere.getCenter().x(),sphere.getCenter().y(),sphere.getCenter().z());
	
	
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
    	float nzBase=-1.0f;
    	float nRatioBase=0.0f;
	
    	for(unsigned int rowi=0;
	    rowi<numRows;
	    ++rowi)
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

    		for(unsigned int topi=0;
		    topi<numSegments;
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
	
	

    glPopMatrix();        
}

void DrawShapeVisitor::apply(const Box& box)
{

    float dx = box.getHalfLengths().x();
    float dy = box.getHalfLengths().y();
    float dz = box.getHalfLengths().z();

    glPushMatrix();

	glTranslatef(box.getCenter().x(),box.getCenter().y(),box.getCenter().z());

	if (!box.zeroRotation())
	{
    	    Matrix rotation(box.getRotationMatrix());
    	    glMultMatrixf(rotation.ptr());
	}

	glBegin(GL_QUADS);

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

	glEnd();

    glPopMatrix();

}

void DrawShapeVisitor::apply(const Cone& cone)
{
    glPushMatrix();

	glTranslatef(cone.getCenter().x(),cone.getCenter().y(),cone.getCenter().z());

	if (!cone.zeroRotation())
	{
    	    Matrix rotation(cone.getRotationMatrix());
    	    glMultMatrixf(rotation.ptr());
	}


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
    	float hDelta = 	cone.getHeight()/(float)numRows;
    	float rDelta = 	cone.getRadius()/(float)numRows;

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
	    glBegin(GL_QUAD_STRIP);

    		angle = 0.0f;
    		texCoord = 0.0f;
    		for(unsigned int topi=0;
		    topi<numSegments;
		    ++topi,angle+=angleDelta,texCoord+=texCoordHorzDelta)
    		{

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

    	// we can't use a fan for the cone top
	// since we need different normals at the top
	// for each face..
	glBegin(GL_TRIANGLE_FAN);
	
    	    angle = osg::PI*2.0f;
    	    texCoord = 1.0f;
    	    basez = cone.getBaseOffset();

	    glNormal3f(0.0f,0.0f,-1.0f);
	    glTexCoord2f(0.5f,0.5f);
	    glVertex3f(0.0f,0.0f,basez);

    	    for(unsigned int bottomi=0;
		bottomi<numSegments;
		++bottomi,angle-=angleDelta,texCoord-=texCoordHorzDelta)
    	    {

    		float c = cosf(angle);
    		float s = sinf(angle);

		glTexCoord2f(c*0.5f+0.5f,s*0.5f+0.5f);
		glVertex3f(c*r,s*r,basez);

    	    }

	    glTexCoord2f(1.0f,0.0f);
	    glVertex3f(r,0.0f,basez);
	
	glEnd();

    glPopMatrix();
}

void DrawShapeVisitor::apply(const Cylinder& cylinder)
{
    glPushMatrix();

	glTranslatef(cylinder.getCenter().x(),cylinder.getCenter().y(),cylinder.getCenter().z());

	if (!cylinder.zeroRotation())
	{
    	    Matrix rotation(cylinder.getRotationMatrix());
    	    glMultMatrixf(rotation.ptr());
	}


    	unsigned int numSegments = 40;
	
	float angleDelta = 2.0f*osg::PI/(float)numSegments;
	
	float texCoordDelta = 1.0/(float)numSegments;
	
	float r = cylinder.getRadius();
	float h = cylinder.getHeight();
	
	float basez = -h*0.5f;
	float topz = h*0.5f;
	
    	// cylinder body
	glBegin(GL_QUAD_STRIP);
	
    	    float angle = 0.0f;
    	    float texCoord = 0.0f;
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
	
	glEnd();



    	// cylinder top
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

    	// cylinder bottom
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

    glPopMatrix();
}

void DrawShapeVisitor::apply(const InfinitePlane& plane)
{
    std::cout << "draw a Plane ("<<plane<<") "<<std::endl;
}

void DrawShapeVisitor::apply(const TriangleMesh& mesh)
{
    const Vec3Array* vertices = mesh.getVertices();
    const IndexArray* indices = mesh.getIndices();
    
     if (vertices && indices)
     {
	glBegin(GL_TRIANGLES);

	for(unsigned int i=0;i<indices->getNumElements();i+=3)
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
    	    Matrix rotation(field.getRotationMatrix());
    	    glMultMatrixf(rotation.ptr());
	}
	
	float dx = field.getXInterval();
	float dy = field.getYInterval();

	float du = 1.0f/((float)field.getNumColumns()-1.0f);
	float dv = 1.0f/((float)field.getNumRows()-1.0f);

    	float vBase = 0.0f;
	for(unsigned int row=0;row<field.getNumRows()-1;++row,vBase+=dv)
	{
	
	    float vTop = vBase+dv;
	    float u = 0.0f;
	
    	    glBegin(GL_QUAD_STRIP);

	    for(unsigned int col=0;col<field.getNumColumns();++col,u+=du)
	    {
    	    	Vec3 vertTop(dx*(float)col,dy*(float)row+dy,field.getHeight(col,row+1));
    	    	Vec3 normTop(field.getNormal(col,row+1));

    	    	Vec3 vertBase(dx*(float)col,dy*(float)row,field.getHeight(col,row));
    	    	Vec3 normBase(field.getNormal(col,row));
	
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
    	virtual void apply(const InfinitePlane&);

    	virtual void apply(const TriangleMesh&);
    	virtual void apply(const ConvexHull&);
    	virtual void apply(const HeightField&);

    	virtual void apply(const CompositeShape&);
	
	BoundingBox&	_bb;
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

    	Vec3 base_1(box.getCenter()+Vec3(-x,-y,-z));
    	Vec3 base_2(box.getCenter()+Vec3(x,-y,-z));
    	Vec3 base_3(box.getCenter()+Vec3(x,y,-z));
    	Vec3 base_4(box.getCenter()+Vec3(-x,y,-z));
    
    	Vec3 top_1(box.getCenter()+Vec3(-x,-y,z));
    	Vec3 top_2(box.getCenter()+Vec3(x,-y,z));
    	Vec3 top_3(box.getCenter()+Vec3(x,y,z));
    	Vec3 top_4(box.getCenter()+Vec3(-x,y,z));

        Matrix matrix = box.getRotationMatrix();
    	_bb.expandBy(base_1*matrix);
    	_bb.expandBy(base_2*matrix);
    	_bb.expandBy(base_3*matrix);
    	_bb.expandBy(base_4*matrix);

    	_bb.expandBy(top_1*matrix);
    	_bb.expandBy(top_2*matrix);
    	_bb.expandBy(top_3*matrix);
    	_bb.expandBy(top_4*matrix);
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
    	Vec3 top(cone.getCenter()+Vec3(cone.getRadius(),cone.getRadius(),cone.getHeight()+cone.getBaseOffset()));
    	Vec3 base_1(cone.getCenter()+Vec3(-cone.getRadius(),-cone.getRadius(),cone.getBaseOffset()));
    	Vec3 base_2(cone.getCenter()+Vec3(cone.getRadius(),-cone.getRadius(),cone.getBaseOffset()));
    	Vec3 base_3(cone.getCenter()+Vec3(cone.getRadius(),cone.getRadius(),cone.getBaseOffset()));
    	Vec3 base_4(cone.getCenter()+Vec3(-cone.getRadius(),cone.getRadius(),cone.getBaseOffset()));
    
        Matrix matrix = cone.getRotationMatrix();
    	_bb.expandBy(base_1*matrix);
    	_bb.expandBy(base_2*matrix);
    	_bb.expandBy(base_3*matrix);
    	_bb.expandBy(base_4*matrix);
	_bb.expandBy(top*matrix);
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

    	Vec3 base_1(cylinder.getCenter()+Vec3(-r,-r,-z));
    	Vec3 base_2(cylinder.getCenter()+Vec3(r,-r,-z));
    	Vec3 base_3(cylinder.getCenter()+Vec3(r,r,-z));
    	Vec3 base_4(cylinder.getCenter()+Vec3(-r,r,-z));
    
    	Vec3 top_1(cylinder.getCenter()+Vec3(-r,-r,z));
    	Vec3 top_2(cylinder.getCenter()+Vec3(r,-r,z));
    	Vec3 top_3(cylinder.getCenter()+Vec3(r,r,z));
    	Vec3 top_4(cylinder.getCenter()+Vec3(-r,r,z));

        Matrix matrix = cylinder.getRotationMatrix();
    	_bb.expandBy(base_1*matrix);
    	_bb.expandBy(base_2*matrix);
    	_bb.expandBy(base_3*matrix);
    	_bb.expandBy(base_4*matrix);

    	_bb.expandBy(top_1*matrix);
    	_bb.expandBy(top_2*matrix);
    	_bb.expandBy(top_3*matrix);
    	_bb.expandBy(top_4*matrix);
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

	    
    if (field.zeroRotation())
    {
	_bb.expandBy(field.getOrigin()+osg::Vec3(0.0f,0.0f,zMin));
	_bb.expandBy(field.getOrigin()+osg::Vec3(field.getXInterval()*field.getNumColumns(),field.getYInterval()*field.getNumRows(),zMax));
    }
    else
    {
    	float x = field.getXInterval()*field.getNumColumns();
    	float y = field.getYInterval()*field.getNumRows();

    	Vec3 base_1(field.getOrigin()+Vec3(0,0,zMin));
    	Vec3 base_2(field.getOrigin()+Vec3(x,0,zMin));
    	Vec3 base_3(field.getOrigin()+Vec3(x,y,zMin));
    	Vec3 base_4(field.getOrigin()+Vec3(0,y,zMin));

    	Vec3 top_1(field.getOrigin()+Vec3(0,0,zMax));
    	Vec3 top_2(field.getOrigin()+Vec3(x,0,zMax));
    	Vec3 top_3(field.getOrigin()+Vec3(x,y,zMax));
    	Vec3 top_4(field.getOrigin()+Vec3(0,y,zMax));

        Matrix matrix = field.getRotationMatrix();
    	_bb.expandBy(base_1*matrix);
    	_bb.expandBy(base_2*matrix);
    	_bb.expandBy(base_3*matrix);
    	_bb.expandBy(base_4*matrix);

    	_bb.expandBy(top_1*matrix);
    	_bb.expandBy(top_2*matrix);
    	_bb.expandBy(top_3*matrix);
    	_bb.expandBy(top_4*matrix);
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
    
    	PrimitiveShapeVisitor(Drawable::PrimitiveFunctor& functor,const TessellationHints* hints):
            _functor(functor),
	    _hints(hints) {}
    
    	virtual void apply(const Sphere&);
    	virtual void apply(const Box&);
    	virtual void apply(const Cone&);
    	virtual void apply(const Cylinder&);
    	virtual void apply(const InfinitePlane&);

    	virtual void apply(const TriangleMesh&);
    	virtual void apply(const ConvexHull&);
    	virtual void apply(const HeightField&);

    	virtual void apply(const CompositeShape&);
	
        Drawable::PrimitiveFunctor& _functor;
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
	    _functor.vertex(ty+rBase,ty,tz+zBase);

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
        Matrix matrix = box.getRotationMatrix();
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

    _functor.vertex(base_2);
    _functor.vertex(base_3);
    _functor.vertex(base_4);
    _functor.vertex(base_1);

    _functor.end();

}

void PrimitiveShapeVisitor::apply(const Cone& cone)
{
    Matrix matrix = cone.getRotationMatrix();
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
    float hDelta = 	cone.getHeight()/(float)numRows;
    float rDelta = 	cone.getRadius()/(float)numRows;

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
    Matrix matrix = cylinder.getRotationMatrix();
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

void PrimitiveShapeVisitor::apply(const InfinitePlane& plane)
{
    std::cout << "Primitive a Plane ("<<plane<<") "<<std::endl;
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
    
    Matrix matrix = field.getRotationMatrix();
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


ShapeDrawable::ShapeDrawable()
{
}

ShapeDrawable::ShapeDrawable(Shape* shape)
{
    setShape(shape);
}

ShapeDrawable::ShapeDrawable(const ShapeDrawable& pg,const CopyOp& copyop):
    Drawable(pg,copyop)
{
}

ShapeDrawable::~ShapeDrawable()
{
}

void ShapeDrawable::drawImplementation(State& state) const
{
    if (_shape.valid())
    {
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


bool ShapeDrawable::computeBound() const
{

    if (_shape.valid())
    {
        ComputeBoundShapeVisitor cbsv(_bbox);
    	_shape->accept(cbsv);
        _bbox_computed = true;

	return true;
    }

    return false;
}

