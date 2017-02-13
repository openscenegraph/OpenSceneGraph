

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/Vec3>
#include <osg/MatrixTransform>
#include <osg/Texture2D>
#include <osg/PolygonStipple>
#include <osg/TriangleFunctor>
#include <osg/io_utils>

#include <osgDB/ReadFile>
#include <osgDB/WriteFile>

#include <osgGA/TrackballManipulator>

#include <osgViewer/Viewer>
#include <osgViewer/ViewerEventHandlers>

#include <osg/Math>

#include <iostream>

#include "SharedVAOGeometry"

///1)transform Geometries into SharedVAOGeometries
///2)transform primset to their basevertex equivalents
///3)maximize BOset sharing among SharedVAOGeometries :1VAS/BOset (responsible of its create is so called master geometry )
///if attribute are per vertex nothing can go wrong, all arrays will share basevertex in their bo
///4)compute primsets basevertices according bo structure
class  MakeSharedBufferObjectsVisitor : public osg::NodeVisitor
{
public:

    MakeSharedBufferObjectsVisitor():
        osg::NodeVisitor(osg::NodeVisitor::TRAVERSE_ALL_CHILDREN)
    {
        _hardMaxbuffsize=1000000000;///min of all glbuffermaxsize
        _softMaxbuffsize=900000000;///ofline: keep all boset during traversal
        _numVAOsInUsed=0;
    }
    virtual void apply(osg::Geode& transform);
    // virtual void apply(osg::Geometry& transform);
    void setHardBufferSize(unsigned int i)    {        _hardMaxbuffsize=i;    }
    unsigned int getHardBufferSize()const    {        return _hardMaxbuffsize;    }
    void setSoftBufferSize(unsigned int i)    {        _softMaxbuffsize=i;    }
    unsigned int getSoftBufferSize()const    {        return _softMaxbuffsize;    }
    unsigned int getNumBufferSetGenerated()const    {        return _numVAOsInUsed;    }

protected:

    void treatBufferObjects(SharedVAOGeometry* g);
    typedef std::vector< osg::BufferObject* > BuffSet;
    typedef std::pair< std::vector<BuffSet>,SharedVAOGeometry *> BuffSetAndMaster;
    std::map<unsigned int,BuffSetAndMaster > _store;

    unsigned int _hardMaxbuffsize;///prohibit bufferdata concatenation in bufferobject
    unsigned int _softMaxbuffsize;///hint a bufferobject is full (and increment lastempty)
    unsigned int _numVAOsInUsed;///for stats
};



void SharedVAOGeometry::setMaster(SharedVAOGeometry*m)
{
    //if(m!=this)
    master=m;

    if(!m)return;
    GLint basevertex=0;
    if(getVertexArray()){
for(int i=0;i<getVertexArray()->getBufferIndex();i++){

basevertex+=getVertexArray()->getBufferObject()->getBufferData(i)->getTotalDataSize();
    }
basevertex/=getVertexArray()->getElementSize();
///  setbasevertex without the pointer hack in drawElementsbasevertex
///
osg::Geometry::PrimitiveSetList& drawelmts=this->getPrimitiveSetList();

for(osg::Geometry::PrimitiveSetList::iterator prit=drawelmts.begin(); prit!=drawelmts.end(); prit++)
{
if(dynamic_cast<osg::DrawElementsUByte*>(prit->get()))
    dynamic_cast<DrawElementsBaseVertexUBYTE*>(prit->get())->_basevertex=basevertex;
else if(dynamic_cast<osg::DrawElementsUInt*>(prit->get()))
dynamic_cast<DrawElementsBaseVertexUINT*>(prit->get())->_basevertex=basevertex;
else if(dynamic_cast<osg::DrawElementsUShort*>(prit->get()))
dynamic_cast<DrawElementsBaseVertexUSHORT*>(prit->get())->_basevertex=basevertex;
else    if(dynamic_cast<osg::DrawArrays*>(prit->get()))
            dynamic_cast<DrawArraysBaseVertex*>(prit->get())->_basevertex=basevertex;

// g->removePrimitiveSet(g->getPrimitiveSetIndex(*prit));

}


}
}
/*
void MakeSharedBufferObjectsVisitor::apply(osg::Geometry&g)
{
    treatBufferObjects(&g);
}*/
void MakeSharedBufferObjectsVisitor::apply(osg::Geode&g)
{
    osg::ref_ptr<osg::Geode> gr=new osg::Geode;
    osg::Geometry * geom;
    for(int i=0; i<g.getNumDrawables(); i++)
    {
        geom=g.getChild(i)->asGeometry();
        if(geom &&geom->getVertexArray())
            gr->addDrawable(new SharedVAOGeometry(*(osg::Geometry*)g.getDrawable(i),osg::CopyOp(osg::CopyOp::DEEP_COPY_ALL)));
    }
    g.removeDrawables(0,g.getNumDrawables());

    for(int i=0; i<gr->getNumDrawables(); i++)
    {
        geom=gr->getChild(i)->asGeometry();
        ///force data variance to static
        geom->setDataVariance(osg::Object::STATIC);
        geom->setUseVertexBufferObjects(true);
        geom->setUseDisplayList(false);
        geom->setUseVertexArrayObject(true);


        treatBufferObjects( (SharedVAOGeometry*) geom);
        g.addDrawable((SharedVAOGeometry    *)geom);
    }
}
///return Hash without lastbit (index array bit)
#define MAX_TEX_COORD 8
#define MAX_VERTEX_ATTRIB 16
unsigned int getArrayList(osg::Geometry*g,osg::Geometry::ArrayList &arrayList)
{
    unsigned int hash=0;

    if (g->getVertexArray())
    {
        hash++;
        arrayList.push_back(g->getVertexArray());
    }
    hash<<=1;
    if (g->getNormalArray())
    {
        hash++;
        arrayList.push_back(g->getNormalArray());
    }
    hash<<=1;
    if (g->getColorArray())
    {
        hash++;
        arrayList.push_back(g->getColorArray());
    }
    hash<<=1;
    if (g->getSecondaryColorArray())
    {
        hash++;
        arrayList.push_back(g->getSecondaryColorArray());
    }
    hash<<=1;
    if (g->getFogCoordArray())
    {
        hash++;
        arrayList.push_back(g->getFogCoordArray());
    }
    hash<<=1;
    for(unsigned int unit=0; unit<g->getNumTexCoordArrays(); ++unit)
    {
        osg::Array* array = g->getTexCoordArray(unit);
        if (array)
        {
            hash++;
            arrayList.push_back(array);
        }
        hash<<=1;
    }
    hash<<=MAX_TEX_COORD-g->getNumTexCoordArrays();
    for(unsigned int  index = 0; index <g->getNumVertexAttribArrays(); ++index )
    {
        osg::Array* array =g->getVertexAttribArray(index);
        if (array)
        {
            hash++;
            arrayList.push_back(array);
        }
        hash<<=1;
    }
    hash<<=MAX_VERTEX_ATTRIB-g->getNumVertexAttribArrays();
    return hash;
}


///check bufferobject already guesting the bd
bool isGuesting(const osg::BufferObject&bo,const osg::BufferData*bd)
{
    for(unsigned int i=0; i<bo.getNumBufferData(); i++)
        if(bo.getBufferData(i)==bd)return true;
    return false;
}


void  MakeSharedBufferObjectsVisitor::treatBufferObjects(SharedVAOGeometry* g)
{

    osg::Geometry::PrimitiveSetList newdrawelmts;

    ///convert primset to BaseVertex
#if 1
    osg::Geometry::PrimitiveSetList& prlist=g->getPrimitiveSetList();;
    for(osg::Geometry::PrimitiveSetList::iterator prit=prlist.begin(); prit!=prlist.end(); prit++)
    {
        if(dynamic_cast<osg::DrawElementsUByte*>(prit->get()))
            newdrawelmts.push_back( new DrawElementsBaseVertexUBYTE(*dynamic_cast<osg::DrawElementsUByte*>(prit->get()),osg::CopyOp(osg::CopyOp::DEEP_COPY_ALL)));
        else if(dynamic_cast<osg::DrawElementsUInt*>(prit->get()))
            newdrawelmts.push_back( new DrawElementsBaseVertexUINT(*dynamic_cast<osg::DrawElementsUInt*>(prit->get()),osg::CopyOp(osg::CopyOp::DEEP_COPY_ALL)));
        else if(dynamic_cast<osg::DrawElementsUShort*>(prit->get()))
            newdrawelmts.push_back( new DrawElementsBaseVertexUSHORT(*dynamic_cast<osg::DrawElementsUShort*>(prit->get()),osg::CopyOp(osg::CopyOp::DEEP_COPY_ALL)));
        else if(dynamic_cast<osg::DrawArrays*>(prit->get()))
                newdrawelmts.push_back( new DrawArraysBaseVertex(*dynamic_cast<osg::DrawArrays*>(prit->get()),g,osg::CopyOp(osg::CopyOp::DEEP_COPY_ALL)));
            else
        {
            std::cout<<"Not taken into account"<<std::endl;
        }
       // g->removePrimitiveSet(g->getPrimitiveSetIndex(*prit));

    }
    ///basevertices will be setted later
    g->removePrimitiveSet(0,g->getNumPrimitiveSets());
    for(osg::Geometry::PrimitiveSetList::iterator prit=newdrawelmts.begin(); prit!=newdrawelmts.end(); prit++)
        g->addPrimitiveSet(*prit);
#endif
    osg::Geometry::ArrayList  bdlist;
osg::Geometry::DrawElementsList drawelmts;
    unsigned int hash= getArrayList(g,bdlist),hasht;
    //std::cout<<bdlist.size()<<" "<<hash<<std::endl;
    drawelmts.clear();
    g->getDrawElementsList(drawelmts);
    hasht=hash;
    unsigned int nbborequired=bdlist.size(),nbdrawelmt=0;


    for(unsigned index=0; index< drawelmts.size(); index++)
    {
        if(nbborequired==bdlist.size())
        {
            nbborequired=bdlist.size()+1;
            hash=hasht+1;
        }
        nbdrawelmt++;
    }

    std::pair< std::vector<BuffSet>,SharedVAOGeometry *> & vecBuffSetANDmaster=_store[hash];
//std::vector<BuffSet>  vecBuffSet=vecBuffSetANDmaster.first;

    std::vector<BuffSet>::iterator itbuffset=  vecBuffSetANDmaster.first.begin();//+vecBuffSet.lastempty;

    unsigned int * buffSize=new unsigned int [nbborequired+nbdrawelmt],*ptrbuffsize=buffSize;

    //while !itbuffset.canGuest(bdlist)
    bool canGuest=false;
    bool alreadyGuesting=true;
    osg::Geometry::ArrayList::iterator arit;
    while(!canGuest && itbuffset!=vecBuffSetANDmaster.first.end())
    {
        canGuest=true;
        alreadyGuesting=true;

        BuffSet::iterator itbo=(*itbuffset).begin();
        for(arit=bdlist.begin(); arit!=bdlist.end(); itbo++,arit++,ptrbuffsize++)
        {
            *ptrbuffsize=(*itbo)->computeRequiredBufferSize()+(*arit)->getTotalDataSize();
            if(*ptrbuffsize>_hardMaxbuffsize)canGuest=false;

            ///check bufferobject already guesting the bd
            if(!isGuesting(**itbo,(*arit)))alreadyGuesting=false;

        }
        for(unsigned index=0; index< drawelmts.size(); index++)
        {

            *ptrbuffsize=(*itbo)->computeRequiredBufferSize()+drawelmts[index]->getTotalDataSize();
            if(*ptrbuffsize++>_hardMaxbuffsize)                    canGuest=false;
            ///check bufferobject already guesting the bd
            if(!isGuesting(**itbo,drawelmts[index]))alreadyGuesting=false;

        }

        ptrbuffsize=buffSize;
        if(alreadyGuesting)
        {
            delete [] buffSize;

            if(!vecBuffSetANDmaster.second)
                std::cout<<"l258 reusing vao of "<<vecBuffSetANDmaster.second<<std::endl;

            g->setMaster(vecBuffSetANDmaster.second);
            return;
        }
        if(!canGuest)itbuffset++;

    }

    if(itbuffset!=vecBuffSetANDmaster.first.end())
    {
        unsigned buffSetMaxSize=0;
        BuffSet::iterator itbo= itbuffset->begin();
        ptrbuffsize=buffSize;
        for( arit=bdlist.begin(); arit!=bdlist.end(); itbo++,arit++,ptrbuffsize++)
        {
            (*arit)->setBufferObject(*itbo);
            buffSetMaxSize=buffSetMaxSize<*ptrbuffsize?*ptrbuffsize:buffSetMaxSize;
        }
        for(unsigned index=0; index< drawelmts.size(); index++)
        {
            drawelmts[index]->setBufferObject(*itbo);
            buffSetMaxSize=buffSetMaxSize<*ptrbuffsize?*ptrbuffsize:buffSetMaxSize;
            ptrbuffsize++;
        }
        ///check if bufferobject is full against soft limit
        if( buffSetMaxSize>_softMaxbuffsize)
            vecBuffSetANDmaster.first.erase(itbuffset);

        delete [] buffSize;
        if(!vecBuffSetANDmaster.second)
            std::cout<<"l288 reusing vao of "<<vecBuffSetANDmaster.second<<std::endl;

        g->setMaster(vecBuffSetANDmaster.second);
        return;
    }


    _numVAOsInUsed++;
    vecBuffSetANDmaster.second=g;

    OSG_WARN<<"create new vao buffset, num vao currently in used:"<<_numVAOsInUsed<<" geom:"<<g<<std::endl;
    ///new BuffSetis required
    BuffSet newBuffSet;
    /// Check if buffer object set offsets configuration is compatible with base vertex draw
    int commonoffset=0;
    bool ready2go=true;
    bool canbereused=true;
    arit=bdlist.begin();
    if(!bdlist.empty())
    {
        for(int i=0; i<(*arit)->getBufferIndex(); i++)commonoffset+=(*arit)->getBufferObject()->getBufferData(i)->getTotalDataSize();
        newBuffSet.push_back((*arit)->getBufferObject());
        for( arit++; arit!=bdlist.end()&&ready2go; arit++)
        {
            int offset=0;
            for(int i=0; i<(*arit)->getBufferIndex(); i++)offset+=(*arit)->getBufferObject()->getBufferData(i)->getTotalDataSize();
            if(offset!=commonoffset)ready2go=false;
            if((*arit)->getBufferObject()->computeRequiredBufferSize()>_softMaxbuffsize)canbereused=false;
            newBuffSet.push_back((*arit)->getBufferObject());
        }
    }
    else
        for(unsigned index=0; index< drawelmts.size()&&ready2go; index++)
        {
            if(drawelmts[0]->getBufferObject()!=drawelmts[index]->getBufferObject())ready2go=false;
            if(drawelmts[index]->getBufferObject()->computeRequiredBufferSize()>_softMaxbuffsize)canbereused=false;
            newBuffSet.push_back(drawelmts[index]->getBufferObject());
            break;
        }

    ///if configuration is good assume bo set is ready to be used as it is
    if(ready2go)
    {
        if(canbereused) vecBuffSetANDmaster.first.push_back(newBuffSet);
        delete [] buffSize;
        if(!vecBuffSetANDmaster.second)
            std::cout<<"l327 reusing vao of "<<vecBuffSetANDmaster.second<<std::endl;

        g->setMaster(vecBuffSetANDmaster.second);
        return;
    }

    ///current configuration doesn't fit so create a new buffset
    newBuffSet.clear();
    std::cout<<newBuffSet.size()<<" "<<hash<<std::endl;

    osg::ElementBufferObject *ebo=0;
    for( arit=bdlist.begin(); arit!=bdlist.end(); arit++)
    {
        newBuffSet.push_back(new osg::VertexBufferObject());
        (*arit)->setBufferObject(newBuffSet.back());
    }
   if(!drawelmts.empty()) ebo=new osg::ElementBufferObject();
    for(unsigned index=0; index<drawelmts.size(); index++)
    {
        //if(!ebo)
        {

            newBuffSet.push_back(ebo);
        }
        drawelmts[index]->setBufferObject(ebo);
    }

    vecBuffSetANDmaster.first.push_back(newBuffSet);
    delete [] buffSize;
     g->setMaster(vecBuffSetANDmaster.second);
}

/// This demo illustrates how VAO Sharing and basevertex draw reduce CPU draw overhead
/// when mesh number get bigger
///(for massive mesh number Indirect draw is better)
int main(int argc, char **argv)
{

    osg::ArgumentParser args(&argc,argv);

    osg::Node * loaded=osgDB::readNodeFiles(args);

    args.getApplicationUsage()->setApplicationName(args.getApplicationName());
    args.getApplicationUsage()->setDescription(args.getApplicationName()+" is an example on how to use  bufferobject factorization+basevertex drawing in order to minimize state changes.");
    args.getApplicationUsage()->setCommandLineUsage(args.getApplicationName()+" [options] filename ...");
    args.getApplicationUsage()->addCommandLineOption("--Hmaxsize <factor>","max bufferobject size allowed (hard limit)");
    args.getApplicationUsage()->addCommandLineOption("--Smaxsize <factor>","max bufferobject size allowed (soft limit)");
       MakeSharedBufferObjectsVisitor vaovis;
       GLuint maxsize;
       while(args.read("--Hmaxsize",maxsize) ) {
           vaovis.setHardBufferSize(maxsize);
       }
       while(args.read("--Smaxsize",maxsize) ) {
           vaovis.setSoftBufferSize(maxsize);
       }
    // create the model
    if(loaded)
    {

        loaded->accept(vaovis);

        osgViewer::Viewer viewer;

        viewer.addEventHandler(new osgViewer::StatsHandler);
        viewer.addEventHandler(new osgViewer::WindowSizeHandler);
        // add model to viewer.
        viewer.setSceneData( loaded );

        return viewer.run();
    }
 args.getApplicationUsage()->write(std::cout,osg::ApplicationUsage::COMMAND_LINE_OPTION);

}
