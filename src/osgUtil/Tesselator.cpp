#include <osg/GL>
#include <osg/GLU>

#include <osg/Notify>
#include <osgUtil/Tesselator>

using namespace osg;
using namespace osgUtil;

static Tesselator* s_currentTesselator=0;


Tesselator::Tesselator()
{
    _tobj = 0;
    _errorCode = 0;
}

Tesselator::~Tesselator()
{
    if (_tobj) gluDeleteTess(_tobj);
}

void Tesselator::beginTesselation()
{
    reset();

    if (!_tobj) _tobj = gluNewTess();
    
    gluTessCallback(_tobj, GLU_TESS_VERTEX_DATA, (GLvoid (CALLBACK*)()) vertexCallback);
    gluTessCallback(_tobj, GLU_TESS_BEGIN_DATA,  (GLvoid (CALLBACK*)()) beginCallback);
    gluTessCallback(_tobj, GLU_TESS_END_DATA,    (GLvoid (CALLBACK*)()) endCallback);
    gluTessCallback(_tobj, GLU_TESS_COMBINE,     (GLvoid (CALLBACK*)()) combineCallback);
    gluTessCallback(_tobj, GLU_TESS_ERROR_DATA,  (GLvoid (CALLBACK*)()) errorCallback);

    gluTessBeginPolygon(_tobj,this);
    
    s_currentTesselator = this;
    
}    
    
void Tesselator::beginContour()
{
    if (_tobj)
    {
        gluTessBeginContour(_tobj);
    }
}
      
void Tesselator::addVertex(osg::Vec3* vertex)
{
    if (_tobj)
    {
        Vec3d* data = new Vec3d;
        _coordData.push_back(data);
        (*data)._v[0]=(*vertex)[0];
        (*data)._v[1]=(*vertex)[1];
        (*data)._v[2]=(*vertex)[2];
        gluTessVertex(_tobj,data->_v,vertex);
    }
}

void Tesselator::endContour()
{
    if (_tobj)
    {
        gluTessEndContour(_tobj);
    }
}

void Tesselator::endTesselation()
{
    if (_tobj)
    {
        gluTessEndPolygon(_tobj);
        gluDeleteTess(_tobj);
        _tobj = 0;
        
        if (_errorCode!=0)
        {
           const GLubyte *estring = gluErrorString((GLenum)_errorCode);
           osg::notify(osg::WARN)<<"Tessellation Error: "<<estring<< std::endl;
        }
    }
}

void Tesselator::reset()
{
    if (_tobj)
    {
        gluDeleteTess(_tobj);
        _tobj = 0;
    }
    _primList.clear();
    _coordData.clear();
    _errorCode = 0;
}

void Tesselator::retesselatePolygons(osg::Geometry& geom)
{
    Vec3Array* vertices = geom.getVertexArray();
    if (!vertices || vertices->empty() || geom.getPrimitiveList().empty()) return;

    int noPrimitiveAtStart = geom.getPrimitiveList().size();
    for(int primNo=0;primNo<noPrimitiveAtStart;++primNo)
    {
        osg::Primitive* primitive = geom.getPrimitiveList()[primNo].get();
        if (primitive->getMode()==osg::Primitive::POLYGON)
        {
            beginTesselation();
            beginContour();

            switch(primitive->getType())
            {
                case(osg::Primitive::DrawArraysPrimitiveType):
                {
                    osg::DrawArrays* drawArray = static_cast<osg::DrawArrays*>(primitive);
                    unsigned int first = drawArray->getFirst(); 
                    unsigned int last = first+drawArray->getCount();
                    for(unsigned int i=first;i<last;++i)
                    {
                        addVertex(&((*vertices)[i]));
                    }
                    break;
                }
                case(osg::Primitive::DrawElementsUBytePrimitiveType):
                {
                    osg::DrawElementsUByte* drawElements = static_cast<osg::DrawElementsUByte*>(primitive);
                    for(osg::DrawElementsUByte::iterator indexItr=drawElements->begin();
                        indexItr!=drawElements->end();
                        ++indexItr)
                    {
                        addVertex(&((*vertices)[*indexItr]));
                    }
                    break;
                }
                case(osg::Primitive::DrawElementsUShortPrimitiveType):
                {
                    osg::DrawElementsUShort* drawElements = static_cast<osg::DrawElementsUShort*>(primitive);
                    for(osg::DrawElementsUShort::iterator indexItr=drawElements->begin();
                        indexItr!=drawElements->end();
                        ++indexItr)
                    {
                        addVertex(&((*vertices)[*indexItr]));
                    }
                    break;
                }
                case(osg::Primitive::DrawElementsUIntPrimitiveType):
                {
                    osg::DrawElementsUInt* drawElements = static_cast<osg::DrawElementsUInt*>(primitive);
                    for(osg::DrawElementsUInt::iterator indexItr=drawElements->begin();
                        indexItr!=drawElements->end();
                        ++indexItr)
                    {
                        addVertex(&((*vertices)[*indexItr]));
                    }
                    break;
                }
                default:
                    break;
            }
            
            endContour();
            endTesselation();
            
            Vec3* vbase = &(vertices->front());
            Vec3* vtop = &(vertices->back());
            
            for(PrimList::iterator primItr=_primList.begin();
                primItr!=_primList.end();
                ++primItr)
            {
                Prim* prim = primItr->get();

                osg::DrawElementsUShort* elements = new osg::DrawElementsUShort(prim->_mode);
                for(Prim::VecList::iterator vitr=prim->_vertices.begin();
                    vitr!=prim->_vertices.end();
                    ++vitr)
                {
                    if (*vitr<vbase || *vitr>vtop)
                    {
                        // new vertex.
                        std::cout<<"Ooohhh we're getting funky, extra vertices need to be inserted"<<std::endl;
                    }
                    else
                    {
                        // old vertex.
                        unsigned int i=*vitr-vbase;
                        elements->push_back(i);
                    }
                }

                if (primItr==_primList.begin()) 
                {
                    // first new primitive so overwrite the previous polygon.
                    geom.getPrimitiveList()[primNo] = elements;                    
                }
                else
                {
                    // subsequence primtives add to the back of the primitive list.
                    geom.addPrimitive(elements);
                }
            }
            
        }
        
    }
}

void Tesselator::begin(GLenum mode)
{
    _primList.push_back(new Prim(mode));
}

void Tesselator::vertex(osg::Vec3* vertex)
{
    if (!_primList.empty())
    {
        Prim* prim = _primList.back().get();
        prim->_vertices.push_back(vertex);
    }
}

void Tesselator::combine(osg::Vec3* vertex)
{
    if (!_primList.empty())
    {
        Prim* prim = _primList.back().get();
        prim->_vertices.push_back(vertex);
    }
}

void Tesselator::end()
{
    // no need to do anything right now...
}

void Tesselator::error(GLenum errorCode)
{
    _errorCode = errorCode;
}

void CALLBACK Tesselator::beginCallback(GLenum which, void* userData)
{
    ((Tesselator*)userData)->begin(which);
}

void CALLBACK Tesselator::endCallback(void* userData)
{
    ((Tesselator*)userData)->end();
}

void CALLBACK Tesselator::vertexCallback(GLvoid *data, void* userData)
{
    ((Tesselator*)userData)->vertex((Vec3*)data);
}

void CALLBACK Tesselator::combineCallback(GLdouble coords[3], void* /*vertex_data*/[4],
                              GLfloat /*weight*/[4], void** outData,
                              void* userData)
{
    Vec3* newData = new osg::Vec3(coords[0],coords[2],coords[3]);
    *outData = newData;
    //((Tesselator*)userData)->combine(newData);
    s_currentTesselator->combine(newData);
}

void CALLBACK Tesselator::errorCallback(GLenum errorCode, void* userData)
{
    ((Tesselator*)userData)->error(errorCode);
}
