#include <osg/GL>
#include <osg/Notify>
#include <osgUtil/Tesselator>

#include <GL/glu.h>


using namespace osg;
using namespace osgUtil;

/* Win32 calling conventions. (or a least thats what the GLUT example tess.c uses.)*/
#ifndef CALLBACK
#define CALLBACK
#endif

static Tesselator* s_tesselator = NULL;
    
void CALLBACK beginCallback(GLenum which)
{
    s_tesselator->beginPrimitive(which);
}

void CALLBACK errorCallback(GLenum errorCode)
{
    s_tesselator->_errorCode = errorCode;
}

void CALLBACK endCallback()
{
    s_tesselator->endPrimitive();
}

void CALLBACK vertexCallback(GLvoid *data)
{
    Tesselator::VertexIndexSet* vip = (Tesselator::VertexIndexSet*)data;
    vip->accumulate();
}


Tesselator::Tesselator()
{
}

Tesselator::~Tesselator()
{
}

void Tesselator::tesselate(osg::Vec3* coords,int numIndices, int* indices,InputBoundaryDirection ibd)
{
    init();
    _coordVec.reserve(numIndices);
	if (ibd==COUNTER_CLOCK_WISE)
	{
        for(int i=0;i<numIndices;++i)
        {
            _coordVec.push_back(VertexIndexSet(this,coords[indices[i]],indices[i]));
        }
    }
	else
    {
        for(int i=numIndices-1;i>=0;--i)
        {
            _coordVec.push_back(VertexIndexSet(this,coords[indices[i]],indices[i]));
        }
	}
    do_it();
}

void Tesselator::tesselate(osg::Vec3* coords,int numIndices, osg::ushort* indices,InputBoundaryDirection ibd)
{
    init();
    _coordVec.reserve(numIndices);
	if (ibd==COUNTER_CLOCK_WISE)
	{
        for(int i=0;i<numIndices;++i)
        {
            _coordVec.push_back(VertexIndexSet(this,coords[indices[i]],indices[i]));
        }
    }
	else
    {
        for(int i=numIndices-1;i>=0;--i)
        {
            _coordVec.push_back(VertexIndexSet(this,coords[indices[i]],indices[i]));
        }
	}
    do_it();
}

void Tesselator::tesselate(osg::Vec3* coords,int numIndices, osg::uint* indices,InputBoundaryDirection ibd)
{
    init();
    _coordVec.reserve(numIndices);
	if (ibd==COUNTER_CLOCK_WISE)
	{
        for(int i=0;i<numIndices;++i)
        {
            _coordVec.push_back(VertexIndexSet(this,coords[indices[i]],indices[i]));
        }
    }
	else
    {
        for(int i=numIndices-1;i>=0;--i)
        {
            _coordVec.push_back(VertexIndexSet(this,coords[indices[i]],indices[i]));
        }
	}
    do_it();
}

void Tesselator::init()
{
    _errorCode = 0;
    _coordVec.clear();
    _acummulated_indices.clear();
    _tesselated_indices.clear();
    _currentPrimtiveType=0;
}

#ifdef GLU_VERSION_1_2
void Tesselator::do_it()
{
    GLUtesselator *tobj = gluNewTess();
    
    gluTessCallback(tobj, (GLenum)GLU_TESS_VERTEX, 
                   (GLvoid (CALLBACK*) ()) (&vertexCallback));
    gluTessCallback(tobj, (GLenum)GLU_TESS_BEGIN, 
                   (GLvoid (CALLBACK*) ()) (&beginCallback));
    gluTessCallback(tobj, (GLenum)GLU_TESS_END, 
                   (GLvoid (CALLBACK*) ()) (&endCallback));
    gluTessCallback(tobj, (GLenum)GLU_TESS_ERROR, 
                   (GLvoid (CALLBACK*) ()) (&errorCallback));

    s_tesselator = this;

    gluTessBeginPolygon(tobj,NULL);
    gluTessBeginContour(tobj);
      
    for(CoordVec::iterator itr=_coordVec.begin();
      itr!=_coordVec.end();
      ++itr)
    {
        gluTessVertex(tobj,itr->_vertex,itr->_vertex);
    }
        
    gluTessEndContour(tobj);
    gluTessEndPolygon(tobj);

    gluDeleteTess(tobj);

    if (_errorCode!=0)
    {
       const GLubyte *estring = gluErrorString((GLenum)_errorCode);
       osg::notify(osg::WARN)<<"Tessellation Error: "<<estring<<endl;
       osg::notify(osg::WARN)<<"  Num indices created = "<<_tesselated_indices.size()<<endl;
    }
}

#else

// old style glu tesseleation.
void Tesselator::do_it()
{
    GLUtriangulatorObj *tobj = gluNewTess();
    
    gluTessCallback(tobj, (GLenum)GLU_VERTEX, 
                   (GLvoid (CALLBACK*) ()) (&vertexCallback));
    gluTessCallback(tobj, (GLenum)GLU_BEGIN, 
                   (GLvoid (CALLBACK*) ()) (&beginCallback));
    gluTessCallback(tobj, (GLenum)GLU_END, 
                   (GLvoid (CALLBACK*) ()) (&endCallback));
    gluTessCallback(tobj, (GLenum)GLU_ERROR, 
                   (GLvoid (CALLBACK*) ()) (&errorCallback));

    s_tesselator = this;

    gluBeginPolygon(tobj);
      
    for(CoordVec::iterator itr=_coordVec.begin();
      itr!=_coordVec.end();
      ++itr)
    {
        gluTessVertex(tobj,itr->_vertex,itr->_vertex);
    }
        
    gluEndPolygon(tobj);

    gluDeleteTess(tobj);

    if (_errorCode!=0)
    {
       const GLubyte *estring = gluErrorString((GLenum)_errorCode);
       osg::notify(osg::WARN)<<"Tessellation Error: "<<estring<<endl;
       osg::notify(osg::WARN)<<"  Num indices created = "<<_tesselated_indices.size()<<endl;
    }
}

#endif

void Tesselator::beginPrimitive(int primitiveType)
{
    _currentPrimtiveType = primitiveType;
}

void Tesselator::endPrimitive()
{
    if (_acummulated_indices.size()>=3)
    {
    
        switch(_currentPrimtiveType)
        {
        case(GL_TRIANGLE_FAN):
            {
                osg::uint first = _acummulated_indices[0];
                for(unsigned int i=2;i<_acummulated_indices.size();++i)
                {
                    _tesselated_indices.push_back(first);
                    _tesselated_indices.push_back(_acummulated_indices[i-1]);
                    _tesselated_indices.push_back(_acummulated_indices[i]);
                }
            }
            break;
        case(GL_TRIANGLE_STRIP):
            {
                for(unsigned int i=2;i<_acummulated_indices.size();++i)
                {
                    if (i%2)
                    {
                        // i == 3,5,7 etc
                        // add in order.
                        _tesselated_indices.push_back(_acummulated_indices[i-2]);
                        _tesselated_indices.push_back(_acummulated_indices[i-1]);
                        _tesselated_indices.push_back(_acummulated_indices[i]);
                    }
                    else
                    {
                        // i == 2,4,6 etc
                        // add in flipping orde to preserve anticlockwise direction.
                        _tesselated_indices.push_back(_acummulated_indices[i-1]);
                        _tesselated_indices.push_back(_acummulated_indices[i-2]);
                        _tesselated_indices.push_back(_acummulated_indices[i]);
                    }
                }
            }
            break;
        case(GL_TRIANGLES):
            {
                
                for(unsigned int i=2;i<_acummulated_indices.size();i+=3)
                {
                    _tesselated_indices.push_back(_acummulated_indices[i-2]);
                    _tesselated_indices.push_back(_acummulated_indices[i-1]);
                    _tesselated_indices.push_back(_acummulated_indices[i]);
                }
            }
            break;
        }
    }
    
    _acummulated_indices.clear();
}

