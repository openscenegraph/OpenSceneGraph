// GEO format (carbon graphics Inc) loader for the OSG real time scene graph
// www.carbongraphics.com for more information about the Geo animation+ modeller
// supports geometry and group & face level animations.
// Vertex level animation partly supported - defines movement (translate, rotate, colour)!
// Loader has been divided into two parts 
// 1- general geometry (here) & 
// 2- animation (see geoActions.cpp).
// ver 1.2 GWM Nov 2003

#include <string>

#include <osg/Image>
#include <osg/Group>
#include <osg/LOD>
#include <osg/Billboard>
#include <osg/Sequence>
#include <osg/Switch>
#include <osg/Geode>
#include <osg/Depth>
#include <osg/LineWidth>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/Material>
#include <osg/Notify>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/StateSet>
#include <osg/CullFace>
#include <osg/Point>
#include "ClipRegion.h"

#include <osgSim/LightPointNode>

#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>
#include <osgDB/fstream>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/WriteFile>
#include <osgDB/Input>
#include <osgDB/Output>
#include <osgUtil/Tessellator>

#include <stdio.h>

// specific to GEO

#include "geoFormat.h"
#include "geoTypes.h"
#include "geoUnits.h"
#include "osgGeoAnimation.h"
#include "osgGeoStructs.h"
#include "osgGeoNodes.h"
#include "osgGeoAction.h"
#include <osgText/Text> // needed for text nodes


//
geoHeaderGeo::geoHeaderGeo()
{ // animations for the header - actually updates all control variables
    intVars=new internalVars; useVars=new userVars;
    extVars=new userVars;
    _initialTick = _timer.tick();
    color_palette=new colourPalette;
}
const geoValue *geoHeaderGeo::getGeoVar(const unsigned fid) const {
    const geoValue *st=intVars->getGeoVar(fid);
    if (!st) {
        st=useVars->getGeoVar(fid);
        if (!st) {
            st=extVars->getGeoVar(fid);
        }
    }
    return st;
}
double *geoHeaderGeo::getVar(const unsigned fid) const { 
    double *dv=NULL;
    dv=intVars->getVar(fid);
    if (!dv) {
        dv=useVars->getVar(fid);
        if (!dv) {
            dv=extVars->getVar(fid);
        }
    }
    return dv;
}
void geoHeaderGeo::addUserVar(const georecord &gr)
{ // this georecord defines a single variable of type<>
    useVars->addUserVar(gr);
}
//== handler for updating internal variables
void geoHeaderGeo::update(const osg::FrameStamp *_frameStamp)
{ // update the scene
    osg::Timer_t _frameTick = _timer.tick();
    _lastFrameTick=_frameTick;
    
    double time = _frameStamp->getSimulationTime();
    intVars->update( _frameStamp);
    moveit(time);
}
void geoHeaderGeo::moveit(const double t)
{ // all the local and external variables declared in the geo modeller are here available.
    if (uvarupdate) {
        std::vector<geoValue> *lvals=useVars->getvars();
        for (std::vector<geoValue>::iterator itr=lvals->begin();
        itr!=lvals->end();
        ++itr) {// for each user var
            double vv=uvarupdate(t, itr->getVal(), itr->getName());
            //        std::cout << " updatee " << itr->getName() << " " << vv << " " << itr->getVar() << std::endl;
            itr->setVal(vv);
            //        vv=itr->getVal(); std::cout << " result " << itr->getName() << " " << vv << std::endl;
        }
    }
    if (extvarupdate) {
        std::vector<geoValue> *lvals=extVars->getvars();
        for (std::vector<geoValue>::iterator itr=lvals->begin();
        itr!=lvals->end();
        ++itr) {// for each user var
            itr->setVal(extvarupdate(t, itr->getVal(), itr->getName()));
        }
    }
}

class geoHeaderCB: public osg::NodeCallback {
public:
    geoHeaderCB() {}
    ~geoHeaderCB() {}
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    { // update action vars
        geoHeaderGeo *gh=(geoHeaderGeo *)node;
        gh->update(nv->getFrameStamp());
        nv->setNodeMaskOverride(0xffffffff); // need to make the visitor override the nodemask
            //    so that it visits 'invisible' nodes to update visibility. Or could use
            // a visitor with setTraversalMode(TraversalMode==TRAVERSE_ALL_CHILDREN)?
        traverse(node,nv);
        //    std::cout<<"update callback - post traverse"<< (float)_frameStamp->getSimulationTime() <<std::endl;
    }
private:
};


//=============

class vertexInfo { // holds vertex information for an entire osg::geometry
public:
    vertexInfo() {
        norms=new osg::Vec3Array;
        coords=new osg::Vec3Array;
        txcoords=new osg::Vec2Array;
        colorindices=new osg::IntArray;
        coordindices=new osg::IntArray;
        normindices=new osg::IntArray;
        txindices=new osg::IntArray;
        colors=new osg::Vec4Array;
        cpool=NULL; npool=NULL;
        polycols= new osg::Vec4Array; // polygon colours
    }
    typedef std::vector<geoActionBehaviour *> drBehList;
    void setPools(const std::vector<osg::Vec3> *coord_pool, const std::vector<osg::Vec3> *normal_pool) {
        cpool=coord_pool; npool=normal_pool;
    }
    inline bool hasVertexActions(void) const { return !(BehList.empty()); }
    inline osg::Vec4Array *getColors() const { return colors;}
    inline osg::Vec3Array *getNorms() const { return norms;}
    inline osg::Vec3Array *getCoords() const { return coords;}
    inline osg::Vec2Array *getTexCoords() const { return txcoords;}
    inline osg::IntArray *getColorIndices() const { return colorindices;}
    inline osg::IntArray *getCoordIndices() const { return coordindices;}
    inline osg::IntArray *getNormIndices() const { return normindices;}
    inline osg::IntArray *getTextureIndices() const { return txindices;}
    void addPolcolour( osg::Vec4 cl) { polycols->push_back(cl);}
    osg::Vec4Array *getPolcolours() const { return polycols;}
    void addVertexActions(geoBehaviourDrawableCB *gcb) const { // add the actions to callback
        if ( !(BehList.empty()) ) {
            for (drBehList::const_iterator rcitr=BehList.begin();
            rcitr!=BehList.end();
            ++rcitr)
            {
                gcb->addBehaviour(*rcitr);
            }
        }
    }
    bool addFlat( const georecord *gface)
    { // this must only be called with a vertex georecord.
        bool isflat=false;
        const geoField *gfshade=gface->getField(GEO_DB_POLY_SHADEMODEL); // shaded gouraud, flat...
        int shademodel=gfshade ? gfshade->getInt() : -1;
        if (shademodel==GEO_POLY_SHADEMODEL_LIT) { // flat shaded - need the index
            const geoField *gfd=gface->getField(GEO_DB_POLY_NORMAL);
            if (gfd) {
                float *normal= (gfd) ? (gfd->getVec3Arr()):NULL;
                osg::Vec3 nrm(normal[0], normal[1], normal[2]);
                norms->push_back(nrm);
                isflat=true;
            }
        }
        return isflat;
    }
    bool addIndices(georecord *gr,    const geoHeaderGeo *ghdr, const float cdef[4], const georecord *gface)
    { // this must only be called with a vertex georecord.
        // gr is tha vertex; gface is the face containing the vertex
        bool hbeh=false; // true if this vertex has a behaviour
        if (gr->getType()==DB_DSK_VERTEX || 
            gr->getType()==DB_DSK_FAT_VERTEX || 
            gr->getType()==DB_DSK_SLIM_VERTEX) {
            const geoField *gfshade=gface->getField(GEO_DB_POLY_SHADEMODEL); // shaded gouraud, flat...
            int shademodel=gfshade ? gfshade->getInt() : -1;
            if (shademodel!=GEO_POLY_SHADEMODEL_LIT && shademodel!=GEO_POLY_SHADEMODEL_FLAT) {
                const geoField *gfd=gr->getField(GEO_DB_VRTX_NORMAL);
                if (gfd->getType()==DB_UINT) {
                    if (gfd) {
                        unsigned int idx=gfd->getUInt();
                        normindices->push_back(idx);
                        norms->push_back((*npool)[idx]);
                    } else {
                        osg::notify(osg::WARN) << "No valid vertex index" << std::endl;
                    }
                } else if (gfd->getType()==DB_VEC3F) {
                    float *p=gfd->getVec3Arr();
                    osg::Vec3 nrm;
                    nrm.set(p[0],p[1],p[2]);
                    norms->push_back(nrm); 
                }
            }
            const geoField *gfd=gr->getField(GEO_DB_VRTX_COORD);
            osg::Vec3 pos;
            if (gfd->getType()==DB_INT) {
                if (gfd) {
                                        int idx=gfd->getInt();
                    pos=(*cpool)[idx];
                    coords->push_back((*cpool)[idx]); //osg::Vec3(cpool[3*idx],cpool[3*idx+1],cpool[3*idx+2]));
                    coordindices->push_back(coords->size());
                } else {
                    osg::notify(osg::WARN) << "No valid vertex index" << std::endl;
                }
            } else if (gfd->getType()==DB_VEC3F) {
                float *p=gfd->getVec3Arr();
                pos.set(p[0],p[1],p[2]);
                coords->push_back(pos); //osg::Vec3(cpool[3*idx],cpool[3*idx+1],cpool[3*idx+2]));
            }
            std::vector< georecord *>bhv=gr->getBehaviour(); // behaviours for vertices, eg tranlate, colour!
            if (!bhv.empty()) {
                int ncoord=coords->size();
                for (std::vector< georecord *>::const_iterator rcitr=bhv.begin();
                rcitr!=bhv.end();
                ++rcitr)
                {
                    if ((*rcitr)->getType()==DB_DSK_TRANSLATE_ACTION) {
                        geoMoveVertexBehaviour *mb=new geoMoveVertexBehaviour;
                        mb->makeBehave((*rcitr),ghdr);
                        mb->setpos(pos);
                        mb->setindx(ncoord-1);
                        BehList.push_back(mb);
                    }
                    if ((*rcitr)->getType()==DB_DSK_ROTATE_ACTION) {
                        geoMoveVertexBehaviour *mb=new geoMoveVertexBehaviour;
                        mb->makeBehave((*rcitr),ghdr);
                        mb->setpos(pos);
                        mb->setindx(ncoord-1);
                        BehList.push_back(mb);
                    }
                    if ((*rcitr)->getType()==DB_DSK_COLOR_RAMP_ACTION) {
                        const geoField *gfd=gface->getField(GEO_DB_POLY_USE_MATERIAL_DIFFUSE); // true: use material...
                        bool usemat= gfd ? gfd->getBool() : false;
                        if (!usemat) { // modify the per vertex colours
                            gfd=gface->getField(GEO_DB_POLY_SHADEMODEL); // shaded gouraud, flat...
                            int shademodel=gfd ? gfd->getInt() : GEO_POLY_SHADEMODEL_LIT_GOURAUD;
                            gfd=gface->getField(GEO_DB_POLY_USE_VERTEX_COLORS); // true: use material...
                            bool usevert=gfd ? gfd->getBool() : false;
                            if (usevert || shademodel==GEO_POLY_SHADEMODEL_GOURAUD) { // then the vertex colours are used
                                geoColourBehaviour *cb=new geoColourBehaviour;
                                cb->setColorPalette(ghdr->getColorPalette());
                                cb->setVertIndices(ncoord-1,1); // part of colours array to be modified
                                bool ok=cb->makeBehave((*rcitr), ghdr);
                                if (ok) BehList.push_back(cb);
                            } // if the model does not use vertex colours... there can be no colour animation at vertex level
                        }
                    }
                }
                hbeh=true;
            }
            txindices->push_back(txcoords->size());
            float *uvc=NULL;
            gfd=gr->getField(GEO_DB_VRTX_UV_SET_0);
            if (gfd) {
                uvc=(float *)gfd->getstore(0);
                
                if (uvc) { // then there are tx coords
                    osg::Vec2 uv(uvc[0], uvc[1]);
                    txcoords->push_back(uv);
                } else {
                    txcoords->push_back(osg::Vec2(0,0));
                }
            } else {
                    txcoords->push_back(osg::Vec2(0,0));
            }
            gfd=gr->getField(GEO_DB_VRTX_PACKED_COLOR);
            if (gfd) {
                unsigned char *cp=gfd->getUCh4Arr();
                float red=cp[0]/255.0f;
                float green=cp[1]/255.0f;
                float blue=cp[2]/255.0f;
                // may need alpha in future:: float alpha=cp[3]/255.0f;
                colors->push_back(Vec4(red,green,blue,1.0));
            } else { // look for a colour index (exclusive!)
                gfd=gr->getField(GEO_DB_VRTX_COLOR_INDEX);
                if (gfd) {
                    uint icp=gfd->getInt();
                    if (icp<128*(ghdr->getColorPalette())->size()) {
                        float col[4];
                        ghdr->getPalette(icp,col);
                        colors->push_back(Vec4(col[0],col[1],col[2],1.0));
                    } else {
                        colors->push_back(Vec4(cdef[0],cdef[1],cdef[2],cdef[3]));
                    }
                } else {
                    colors->push_back(Vec4(cdef[0],cdef[1],cdef[2],cdef[3]));
                }
                int idx=colors->size()-1;
                colorindices->push_back(idx);
            }
        }
        return hbeh;
    }
    friend inline std::ostream& operator << (std::ostream& output, const vertexInfo& vf)
    {
        const osg::Vec2Array *txa=vf.getTexCoords();
        osg::IntArray *normindices=vf.getNormIndices();
        osg::IntArray *txind = vf.getTextureIndices();
        output << " vertexinfo " << txa->size() << " nrm: " << normindices->size()<<
            " txinds " << txind->size()<<std::endl;
        uint i;
        for (i=0; i< txa->size(); i++) {
            const osg::Vec2 uvt=(*txa)[i];
            output << " U " << uvt.x() << " v " <<  uvt.y() << std::endl;
        }
        for (i=0; i<normindices->size(); i++) {
            output << "Nind " << i << " = " <<  (*normindices)[i] << std::endl;
        }
        return output;     // to enable cascading, monkey copy from osg\plane or \quat, Ubyte4, vec2,3,4,... 
    }
private:
    const std::vector<osg::Vec3> *cpool; // passed in from the geo file
    const std::vector<osg::Vec3> *npool;
    osg::Vec3Array *norms;
    osg::Vec3Array *coords;
    osg::Vec2Array *txcoords;
    osg::Vec4Array *colors;
    osg::IntArray *colorindices;
    osg::IntArray *coordindices;
    osg::IntArray *normindices;
    osg::IntArray *txindices;
    drBehList BehList;
    Vec4Array *polycols;
};

class geoInfo { // identifies properties required to make a new Geometry, and holds collection of vertices, indices, etc
public:
    geoInfo(const int txidx=-2, const int sm=1, const int bs=1) { texture=txidx; // will be -1 or 0-number of textures
        geom=NULL; nstart=0; linewidth=1;
        bothsides=bs; shademodel=sm;
    }
    virtual ~geoInfo() { };
    inline int getShademodel(void) const { return shademodel;}
    inline int getBothsides(void) const { return bothsides;}
    inline int getTexture(void) const { return texture;}
    inline vertexInfo *getVinf(void) { return &vinf;}
    void setPools(const std::vector<osg::Vec3> *coord_pool, const std::vector<osg::Vec3> *normal_pool) {
        vinf.setPools(coord_pool,normal_pool);
    }
    float getlinewidth(void) const { return linewidth;}
    void setlineWidth(const int w) { linewidth=w;}
    void setGeom(osg::Geometry *nugeom) { geom=nugeom;}
    osg::Geometry *getGeom() { return geom.get();}
    uint getStart(uint nv) { uint ns=nstart; nstart+=nv; return ns; }
    bool operator == (const geoInfo *gt) { // compare two geoInfos for same type of geometry
        if (gt->texture!=texture) return false;
        if (gt->bothsides == !bothsides) return false;
        if (gt->shademodel!=shademodel) return false;
        // other tests if failed return false
        return true;
    }
private:
    int texture; // texture index
    int bothsides; // none, back,front
    int shademodel;
    int linewidth;
    vertexInfo vinf;
    uint nstart; // start vertex for a primitive
    osg::ref_ptr<osg::Geometry> geom; // the geometry created for this vinf and texture
};


class ReaderGEO
{
    public:

        osgDB::ReaderWriter::ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options)
        {

            osgDB::ifstream fin(fileName.c_str(), std::ios::binary | std::ios::in );
            if (fin.is_open() )
            { // read the input file.
                // code for setting up the database path so that internally referenced file are searched for on relative paths. 
                osg::ref_ptr<osgDB::ReaderWriter::Options> local_opt = options ? 
                    static_cast<osgDB::ReaderWriter::Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : 
                    new osgDB::ReaderWriter::Options;
                local_opt->setDatabasePath(osgDB::getFilePath(fileName));

                typedef std::vector<osg::Node*> NodeList;
                NodeList nodeList;
                osg::Material *mt=new osg::Material;
                matlist.push_back(mt);
                theHeader=NULL;

                // load all nodes in file, placing them in a linear list corresponding to the on disk file.
                while(!fin.eof())
                {
                    georecord gr;
                    gr.readfile(fin);
//            osg::notify(osg::WARN) << "end of record " << (int)gr.getType() << std::endl;
                    if (gr.getType() == DB_DSK_NORMAL_POOL) {
                        geoField *gfff=gr.getModField(GEO_DB_NORMAL_POOL_VALUES);
                        gfff->uncompress();// uncompress the normals
                    }
                    recs.push_back(gr); // add to a list of all records
                }
                fin.close();
                // now sort the records so that any record followed by a PUSh has a child set = next record, etc
                std::vector<georecord *> sorted=sort(recs); // tree-list of sorted record pointers
#ifdef _DEBUG
                osgDB::Output fout("georex.txt"); //, std::ios_base::out );
              //  fout << "Debug raw file " << fileName << std::endl;
              //  output(fout,recs);
                fout << "Debug Sorted file " << fileName << std::endl;
                output(fout,sorted);
                fout.close();
#endif /**/
                makeHeader(*(sorted.begin()), local_opt.get());

                nodeList=makeosg(sorted, local_opt.get()); // make a list of osg nodes
                geotxlist.clear();
                geomatlist.clear(); 
                txlist.clear();
                txenvlist.clear();
                matlist.clear();/* */
                coord_pool.clear();
                normal_pool.clear();
                osg::Node * groupnode = NULL;
                if  (nodeList.empty())
                {
                    return osgDB::ReaderWriter::ReadResult("No data loaded from "+fileName);
                }
                else if (nodeList.size()==1)
                {
                    groupnode = nodeList.front();
                }
                else
                {
                    osg::Group *group = new Group;
                    group->setName("import group");
                    for(NodeList::iterator itr=nodeList.begin();
                        itr!=nodeList.end();
                        ++itr)
                    {
                        group->addChild(*itr);
                    }
                    groupnode=group;
                }
                (theHeader.get())->addChild(groupnode);
                groupnode=theHeader.get();
#ifdef _DEBUG // output a .osg version
                osgDB::writeNodeFile(*groupnode,"geoosg.osg");
#endif /**/
                recs.clear();
                return groupnode;
            }
            return 0L;
        }
        std::vector<georecord *> sort(geoRecordList &recs) { // return a tree-list of sorted record pointers
            // which mirrors the original .geo file (containers hold push/pop blocks).
            std::vector<georecord *> sorted;
            class georecord *curparent=NULL;
            for (geoRecordList::iterator itr=recs.begin();
            itr!=recs.end();
            ++itr) {
                const geoField *gfd;
                // osg::notify(osg::WARN) << *itr << std::endl;
                // now parse for push/pops and add to lists
                
                switch ((*itr).getType()) {
                case 101: // old header - not appropriate!
                    curparent= &(*itr);
                    sorted.push_back(&(*itr));
                    osg::notify(osg::WARN) << "Old version 2 header block found - possible error!" << std::endl;
                    break;
                case DB_DSK_PUSH:
                    if (!(curparent->getchildren().empty())) {
                        curparent= curparent->getLastChild(); // itr-1;
                    } else {
                        //curparent=itr-1;
                    }
                    break;
                case DB_DSK_POP:
                    if (curparent) curparent=curparent->getparent();
                    break;
                case DB_DSK_HEADER: // attach to previous
                    curparent= &(*itr);
                    sorted.push_back(&(*itr));
                    cpalrec=NULL; 
                    break;
                case DB_DSK_INTERNAL_VARS: // attach to parent
                case DB_DSK_LOCAL_VARS:
                case DB_DSK_EXTERNAL_VARS:
                    (curparent)->addBehaviourRecord(&(*itr));
                    break;
                case DB_DSK_FLOAT_VAR: // attach to parent
                case DB_DSK_INT_VAR:
                case DB_DSK_LONG_VAR:
                case DB_DSK_DOUBLE_VAR:
                case DB_DSK_BOOL_VAR:
                case DB_DSK_FLOAT2_VAR:
                case DB_DSK_FLOAT3_VAR:
                case DB_DSK_FLOAT4_VAR:
                    // else if ((*itr).isVar():
                    (curparent)->addBehaviourRecord(&(*itr));
                        break;
                case DB_DSK_TEXTURE: // attach to parent
                    geotxlist.push_back(&(*itr));
                    break;
                case DB_DSK_MATERIAL: // attach to parent
                    geomatlist.push_back(&(*itr));
                    break;
                case DB_DSK_VIEW: // not needed for Real Time
                    break;
                case DB_DSK_COORD_POOL: // global - attach to readerwriterGEO class for whole model
                    gfd=itr->getField(GEO_DB_COORD_POOL_VALUES);
                    {
                        float *crds= (gfd) ? (gfd->getVec3Arr()):NULL;
                        uint nm=gfd->getNum();
                        for (uint i=0; i<nm; i++) {
                            coord_pool.push_back(Vec3(crds[i*3],crds[i*3+1],crds[i*3+2]));
                        }
                    }
                    break;
                case DB_DSK_NORMAL_POOL: // global - attach to readerwriterGEO
                    gfd=itr->getField(GEO_DB_NORMAL_POOL_VALUES);
                    {
                        float *nrms= (gfd) ? (gfd->getVec3Arr()):NULL;
                        uint nm=gfd->getNum();
                        for (uint i=0; i<nm; i++) {
                            normal_pool.push_back(Vec3(nrms[i*3],nrms[i*3+1],nrms[i*3+2]));
                        }
                    }
                    break;
                case DB_DSK_COLOR_PALETTE: // global - attach to readerwriterGEO
                    cpalrec=&(*itr);
                    break;
                case DB_DSK_BEHAVIOR: // || (*itr).isAction() // attach to previous
                case DB_DSK_CLAMP_ACTION:
                case DB_DSK_RANGE_ACTION:
                case DB_DSK_ROTATE_ACTION:
                case DB_DSK_TRANSLATE_ACTION:
                case DB_DSK_SCALE_ACTION:
                case DB_DSK_ARITHMETIC_ACTION:
                case DB_DSK_LOGIC_ACTION:
                case DB_DSK_CONDITIONAL_ACTION:
                case DB_DSK_LOOPING_ACTION:
                case DB_DSK_COMPARE_ACTION:
                case DB_DSK_VISIBILITY_ACTION:
                case DB_DSK_STRING_CONTENT_ACTION:
                case DB_DSK_COLOR_RAMP_ACTION:
                case DB_DSK_LINEAR_ACTION:
                case DB_DSK_TASK_ACTION:
                case DB_DSK_PERIODIC_ACTION:
#ifdef DB_DSK_PERIODIC2_ACTION
                case DB_DSK_PERIODIC2_ACTION:
#endif
                case DB_DSK_TRIG_ACTION:
                case DB_DSK_DISCRETE_ACTION:
                case DB_DSK_INVERSE_ACTION:
                case DB_DSK_TRUNCATE_ACTION:
                case DB_DSK_ABS_ACTION:
                case DB_DSK_IF_THEN_ELSE_ACTION:
                case DB_DSK_DCS_ACTION:
                case DB_DSK_SQRT_ACTION:    // an action
                    if (curparent->getType()==DB_DSK_HEADER)
                        curparent->addBehaviourRecord(&(*itr));
                    else {
                        class georecord *cp=curparent->getLastChild();
                        if (cp) cp->addBehaviourRecord(&(*itr));
                    }
                    break;
                case DB_DSK_PERSPECTIVE_GRID_INFO: // Feb 2003 not sure what this is yet!
                    (curparent)->addchild(&(*itr));
                    break;
                case DB_DSK_PLANE_TEXTURE_MAPPING_INFO: // not needed for real time
                case DB_DSK_CYLINDER_TEXTURE_MAPPING_INFO:    // not implemented in 1.0
                case DB_DSK_SPHERE_TEXTURE_MAPPING_INFO:    // not implemented in 1.0
                case DB_DSK_GRID_TEXTURE_MAPPING_INFO:    // not implemented in 1.0
                    (curparent->getLastChild())->addMappingRecord(&(*itr));
                    break;
                default:
                    if (curparent) {
                        (*itr).setparent(curparent);
                        curparent->addchild(&(*itr));
                    } 
                    break;
                }
            }
            return sorted;
        }
        void outputPrim(const georecord *grec, osgDB::Output &fout) { // output to file for debug
            const std::vector<georecord *> gr=grec->getchildren();
            if (gr.size()>0) {
                for (std::vector<georecord *>::const_iterator itr=gr.begin();
                    itr!=gr.end();
                    ++itr) {
                    fout << *(*itr) << std::endl;
                }
            }
        }
 /*       bool allOneSided(const georecord *grec)    {
            bool one=false;
            const std::vector<georecord *> gr=grec->getchildren();
            if (gr.size()>0) {
                for (std::vector<georecord *>::const_iterator itr=gr.begin();
                itr!=gr.end() && !one;
                ++itr) {
                    if ((*itr)->getType()==DB_DSK_POLYGON) {
                        const geoField *gfd=(*itr)->getField(GEO_DB_POLY_DSTYLE);
                        if (gfd) {
                            int dstyle=gfd->getInt();
                            one=(dstyle==GEO_POLY_DSTYLE_SOLID_BOTH_SIDES);
                        }
                    }
                }
            }
            return one;
        }*/
        osg::Geometry *makeNewGeometry(const georecord *grec, geoInfo &ginf, int imat) {
            const int shademodel=ginf.getShademodel();
            const int bothsides=ginf.getBothsides();
            osg::Geometry *nug;
            int txidx=ginf.getTexture();
            nug=new osg::Geometry;
            const vertexInfo *vinf=ginf.getVinf();
            nug->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
            nug->setVertexArray(vinf->getCoords());
            StateSet *dstate=new StateSet;
            if (bothsides==0) {
                osg::CullFace *cf = new osg::CullFace; // to define non-default culling
                cf->setMode(osg::CullFace::BACK);
                dstate->setAttributeAndModes(cf,osg::StateAttribute::ON);
            }
            else if (bothsides==1) {
                osg::CullFace *cf = new osg::CullFace; // to define non-default culling
                cf->setMode(osg::CullFace::FRONT);
                dstate->setAttributeAndModes(cf,osg::StateAttribute::ON);
            }
            else if (bothsides==2) {
                osg::CullFace *cf = new osg::CullFace; // to define non-default culling
                dstate->setAttributeAndModes(cf,osg::StateAttribute::OFF);
            }
            Point *pt=new Point;
            pt->setSize(4);
            dstate->setAttribute(pt);
            if (txidx>=0 && (unsigned int)txidx<txlist.size()) {
                dstate->setTextureAttribute(0, txenvlist[txidx].get() );
                dstate->setTextureAttributeAndModes(0,txlist[txidx].get(),osg::StateAttribute::ON);
                const Image *txim=txlist[txidx]->getImage();
                if (txim) {
                    GLint icm=txim->computeNumComponents(txim->getPixelFormat());
                    if (icm ==2 || icm==4) { // an alpha texture
                        dstate->setMode(GL_BLEND,StateAttribute::ON);
                        dstate->setRenderingHint(StateSet::TRANSPARENT_BIN);
                    }
                }
            }
            if (imat<0 || imat>=(int)matlist.size()) imat=0;
            const geoField *gfd=grec->getField(GEO_DB_POLY_USE_MATERIAL_DIFFUSE); // true: use material...
            bool usemat= gfd ? gfd->getBool() : false;
            if (!usemat) {
                matlist[imat]->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);
                dstate->setMode(GL_COLOR_MATERIAL, osg::StateAttribute::ON);
            }
            dstate->setAttribute(matlist[imat].get());
            Vec4 col=matlist[imat]->getAmbient(Material::FRONT);
            if (col[3]<0.99) {
                dstate->setMode(GL_BLEND,StateAttribute::ON);
                dstate->setRenderingHint(StateSet::TRANSPARENT_BIN);
            }

            if (shademodel==GEO_POLY_SHADEMODEL_LIT ||
                shademodel==GEO_POLY_SHADEMODEL_LIT_GOURAUD) dstate->setMode( GL_LIGHTING, osg::StateAttribute::ON );
            else 
                dstate->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
            { // reclaim the colours
                gfd=grec->getField(GEO_DB_POLY_USE_MATERIAL_DIFFUSE); // true: use material...
                bool usemat= gfd ? gfd->getBool() : false;
                if (!usemat) { // get the per vertex colours OR per face colours.
                    gfd=grec->getField(GEO_DB_POLY_USE_VERTEX_COLORS); // true: use material...
                    bool usevert=gfd ? gfd->getBool() : false;
                    if (usevert || shademodel==GEO_POLY_SHADEMODEL_GOURAUD) {
                        Vec4Array *cls=vinf->getColors();
                        if (cls) {
                            nug->setColorArray(cls);
                            nug->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
                        }
                    } else {
                        if (shademodel==GEO_POLY_SHADEMODEL_LIT_GOURAUD) {
                            nug->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
                        } else if (shademodel==GEO_POLY_SHADEMODEL_LIT) {
                            nug->setNormalBinding(osg::Geometry::BIND_PER_PRIMITIVE);
                        }
                        osg::Vec4Array *polycols=vinf->getPolcolours();
                        nug->setColorArray(polycols);
                        nug->setColorBinding(osg::Geometry::BIND_PER_PRIMITIVE);
                    }
                }
            }
            osg::LineWidth *lw=new osg::LineWidth;
            lw->setWidth(ginf.getlinewidth());
            dstate->setAttributeAndModes(lw,osg::StateAttribute::ON);
            nug->setStateSet( dstate );
            ginf.setGeom(nug);
            return nug;
        }
        int getprim(const georecord *grec, geoInfo &gi)
        { // fills vinf with txcoords = texture coordinates, txindex=txindex etc
            // for one primitive (one tri, quad, pol, tristrip....
            vertexInfo *vinf=gi.getVinf();
            int nv=0;
            const std::vector<georecord *> gr=grec->getchildren();
            const geoField *gfd=grec->getField(GEO_DB_POLY_PACKED_COLOR); // the colour
            float defcol[4]; // a default colour for vertices
            defcol[0]=defcol[1]=defcol[2]=defcol[3]=1.0f;
            if (gfd) {
                unsigned char *cls=gfd->getUCh4Arr();
                defcol[0]=cls[0]/255.0f;
                defcol[1]=cls[1]/255.0f;
                defcol[2]=cls[2]/255.0f;
                defcol[3]=1.0f;
            } else {
                gfd=grec->getField(GEO_DB_POLY_COLOR_INDEX); // the colour
                if (gfd) {
                    int icp= gfd ? gfd->getInt() : 0;
                    theHeader->getPalette(icp,defcol);
                } else {
                    defcol[0]=defcol[1]=defcol[2]=defcol[3]=1.0f;
                }
            }

            if (gr.size()>0) {
                vinf->addFlat(grec); // for flat normal shading
                for (std::vector<georecord *>::const_iterator itr=gr.begin();
                    itr!=gr.end();
                    ++itr) {
                    vinf->addIndices((*itr), theHeader.get(), defcol, grec);
                    nv++;
                }
            }
            return nv;
        }
        void outputGeode(georecord grec, osgDB::Output &fout) { // 
            const std::vector<georecord *> gr=grec.getchildren();
            if (gr.size()>0) {
                fout.moveIn();
                for (std::vector<georecord *>::const_iterator itr=gr.begin();
                itr!=gr.end();
                ++itr) {
                    fout.indent() << *(*itr) << std::endl;
                    if ((*itr)->getType()==DB_DSK_POLYGON) {
                        outputPrim((*itr),fout);
                    }
                }
                 fout.moveOut();
            }
        }
        osg::MatrixTransform *makeText(georecord *gr) { // make transform, geode & text
            osg::MatrixTransform *numt=NULL;
            std::string    ttfPath("fonts/times.ttf");
            // unused
            //int    gFontSize1=2;
            osgText::Text *text= new  osgText::Text;
            text->setFont(ttfPath);
            const geoField *gfd=gr->getField(GEO_DB_NODE_NAME);
            const char *name=gfd ? gfd->getChar() : "a text";
            gfd=gr->getField(GEO_DB_TEXT_STRING);
            const char *content=gfd ? gfd->getChar() : " ";
            text->setText(std::string(content));
            gfd=gr->getField(GEO_DB_TEXT_SCALE_X);
            //const float scx=gfd ? gfd->getFloat() : 1.0f;
            gfd=gr->getField(GEO_DB_TEXT_SCALE_Y);
            //const float scy=gfd ? gfd->getFloat() : 1.0f;
            gfd=gr->getField(GEO_DB_TEXT_JUSTIFICATION); // GEO_DB_TEXT_DIRECTION);
            int tjus=gfd? gfd->getInt() : GEO_TEXT_LEFT_JUSTIFY;
            switch(tjus) {
                case GEO_TEXT_LEFT_JUSTIFY:  text->setAlignment(osgText::Text::LEFT_BOTTOM); break;
                case GEO_TEXT_CENTER_JUSTIFY:  text->setAlignment(osgText::Text::CENTER_BOTTOM); break;
                case GEO_TEXT_RIGHT_JUSTIFY:  text->setAlignment(osgText::Text::RIGHT_BOTTOM); break;
            }
            gfd=gr->getField(GEO_DB_TEXT_PACKED_COLOR);
            if (gfd) {
                unsigned char *cp=gfd->getUCh4Arr();
                float red=(float)cp[0]/255.0f;
                float green=(float)cp[1]/255.0f;
                float blue=(float)cp[2]/255.0f;
                text->setColor(osg::Vec4(red,green,blue,1.0f));
            } else { // lok for a colour index (exclusive!)
                gfd=gr->getField(GEO_DB_TEXT_COLOR_INDEX);
                if (gfd) {
                    int icp=gfd->getInt();
                    float col[4];
                    theHeader->getPalette(icp,col);
                    text->setColor(osg::Vec4(col[0],col[1],col[2],1.0));
                }
            }
            osg::Geode *geod=new osg::Geode;
            osg::StateSet *textState = new osg::StateSet();
            textState->setMode(GL_LIGHTING, osg::StateAttribute::OFF);
            geod->setStateSet( textState );
            numt=new osg::MatrixTransform;
            numt->setName(name);
            gfd=gr->getField(GEO_DB_TEXT_MATRIX);
            if (gfd) {
                float *fmat=gfd->getMat44Arr();            
                // text->setPosition(osg::Vec3(fmat[12],fmat[13],fmat[14]));
                numt->setMatrix(Matrix(fmat));
            }
            numt->addChild(geod);
            geod->addDrawable(text);
            {
                std::vector< georecord *>bhv=gr->getBehaviour();
                if (!bhv.empty()) { // then check for a string content/colour.. action
                    bool ok=false;
                    geoBehaviourDrawableCB *gcb=new geoBehaviourDrawableCB;
                    text->setUpdateCallback(gcb);
                    for (std::vector< georecord *>::const_iterator rcitr=bhv.begin();
                    rcitr!=bhv.end();
                    ++rcitr)
                    {
                        if ((*rcitr)->getType()==DB_DSK_STRING_CONTENT_ACTION) {
                            geoStrContentBehaviour *cb=new geoStrContentBehaviour;
                            gfd=(*rcitr)->getField(GEO_DB_STRING_CONTENT_ACTION_INPUT_VAR);
                            if (gfd) {
                                ok=cb->makeBehave((*rcitr), theHeader.get());
                                if (ok) gcb->addBehaviour(cb);
                                else delete cb;
                                // ok=false;
                            }
                        }
                    }
                }
            }
            return numt;
        }
        void addPolyActions(std::vector< georecord *>bhv, geoInfo &gi , const uint nv) {
            const vertexInfo *vinf=gi.getVinf();
            const uint nstart=gi.getStart(nv);
            if (hasColorAction(bhv) || vinf->hasVertexActions()) {
                osg::Geometry *nugeom=gi.getGeom();
                geoBehaviourDrawableCB *gcb=new geoBehaviourDrawableCB;
                nugeom->setUpdateCallback(gcb);
                nugeom->setUseDisplayList(false); // as we are updating arrays, cannot change colours
                for (std::vector< georecord *>::const_iterator rcitr=bhv.begin();
                rcitr!=bhv.end();
                ++rcitr)
                {
                    if ((*rcitr)->getType()==DB_DSK_COLOR_RAMP_ACTION) {
                        geoColourBehaviour *cb=new geoColourBehaviour;
                        cb->setColorPalette(theHeader->getColorPalette());
                        if (nugeom->getColorBinding()==osg::Geometry::BIND_PER_VERTEX) {
                            cb->setVertIndices(nstart,nv); // part of colours array to be modified
                        } else if (nugeom->getColorBinding()==osg::Geometry::BIND_PER_PRIMITIVE) { // per primitive
                            const uint nst=nugeom->getNumPrimitiveSets();
                            cb->setVertIndices(nst,1); // part of colours array to be modified
                        } else { // overall
                            cb->setVertIndices(0,1); // part of colours array to be modified
                        }
                        bool ok=cb->makeBehave((*rcitr), theHeader.get());
                        if (ok) gcb->addBehaviour(cb);
                        else delete cb;
                    }
                }
                vinf->addVertexActions(gcb);
            }
        }
        void makeLightPointNode(const georecord *grec, osgSim::LightPointNode *lpn) {
          // light points.. require OSG professional license
        // OR LGPL software.
            const std::vector<georecord *> gr=grec->getchildren();
            for (std::vector<georecord *>::const_iterator itr=gr.begin();
                itr!=gr.end();
                ++itr)
            {
                if ((*itr)->getType()==DB_DSK_VERTEX || 
                    (*itr)->getType()==DB_DSK_FAT_VERTEX || 
                    (*itr)->getType()==DB_DSK_SLIM_VERTEX)
                { // light point vertices
                    const geoField *gfd=(*itr)->getField(GEO_DB_VRTX_COORD);
                    osg::Vec3 pos;
                    if (gfd->getType()==DB_INT) {
                        if (gfd) {
                                                        int idx=gfd->getInt();
                            pos=coord_pool[idx];
                        } else {
                            osg::notify(osg::WARN) << "No valid vertex index" << std::endl;
                        }
                    } else if (gfd->getType()==DB_VEC3F) {
                        float *p=gfd->getVec3Arr();
                        pos.set(p[0],p[1],p[2]);
                    }
                    gfd=(*itr)->getField(GEO_DB_VRTX_PACKED_COLOR);
                    if (gfd) {
                        unsigned char *cls=gfd->getUCh4Arr();
                        float red=cls[0]/255.0f;
                        float green=cls[1]/255.0f;
                        float blue=cls[2]/255.0f;
                        //float alpha=1.0f; // cls[3]*frac/255.0f;
                        osg::Vec4 colour(red,green,blue,1.0f);
                        lpn->addLightPoint(osgSim::LightPoint(true,pos,colour,1.0f,1.0f,0,0,osgSim::LightPoint::BLENDED));
                    } else { // get colour from palette
                        gfd=(*itr)->getField(GEO_DB_VRTX_COLOR_INDEX); // use color pool...
                        int icp= gfd ? gfd->getInt() : 0;
                        float col[4];
                        theHeader->getPalette(icp, col);
                        lpn->addLightPoint(osgSim::LightPoint(pos, osg::Vec4(col[0],col[1],col[2],1.0f)));
                    }
                }
            }
        }
        void makeLightPointGeometry(const georecord *grec, Group *nug) {
            const std::vector<georecord *> gr=grec->getchildren();
            for (std::vector<georecord *>::const_iterator itr=gr.begin();
                itr!=gr.end();
                ++itr)
            {
                if ((*itr)->getType()==DB_DSK_LIGHTPT) { // light points ONLY
                    geoInfo ginf(0,0, 1);
                    ginf.setPools(&coord_pool, &normal_pool); // holds all types of coords, indices etc
                    osgSim::LightPointNode *gd=new osgSim::LightPointNode;
                 // to be implemented   const geoField *gfd=(*itr)->getField(GEO_DB_LIGHTPT_TYPE); // omni, uni, bi
                    makeLightPointNode((*itr),gd); // add vertex positions to light point set
                    nug->addChild(gd);
                }
            }
        }
        int  makeAnimatedGeometry(const georecord grec, const int imat,Group *nug) {
            // animated polygons - create a matrix & geode & poly & add to group nug
            const std::vector<georecord *> gr=grec.getchildren();
            int nanimations=0;
            const geoField *gfd=grec.getField(GEO_DB_RENDERGROUP_CULLING); // back, front, none
            unsigned int bothsides=gfd ? gfd->getUInt() : 0;
//            int bothsides =allOneSided(&grec);
            for (std::vector<georecord *>::const_iterator itr=gr.begin();
            itr!=gr.end();
            ++itr) {
                std::vector< georecord *>bhv=(*itr)->getBehaviour(); // behaviours attached to facets, eg colour!
                if ((*itr)->getType()==DB_DSK_POLYGON && !bhv.empty()) { // animated facets go here
                    nanimations++;
                    if (hasMotionAction(bhv)) { // make matrix if motion needed.
                        const geoField *gfd=(*itr)->getField(GEO_DB_POLY_TEX0);
                        int txidx= gfd ? gfd->getInt() : -1;
                        gfd=(*itr)->getField(GEO_DB_POLY_SHADEMODEL); // shaded gouraud, flat...
                        int shademodel=gfd ? gfd->getInt() : GEO_POLY_SHADEMODEL_LIT_GOURAUD;
                        gfd=(*itr)->getField(GEO_DB_POLY_USE_MATERIAL_DIFFUSE); // true: use material...
                        bool usemat= gfd ? gfd->getBool() : false;
                        geoInfo ginf(txidx,shademodel, bothsides);
                        ginf.setPools(&coord_pool, &normal_pool); // holds all types of coords, indices etc
                        MatrixTransform *mtr=makeBehave(*itr);
                        Geode *gd=new Geode;
                        gfd=(*itr)->getField(GEO_DB_POLY_DSTYLE); // solid, wire...
                        int dstyle= gfd ? gfd->getInt() : GEO_POLY_DSTYLE_SOLID;
                        if (!usemat && 
                            (shademodel== GEO_POLY_SHADEMODEL_LIT ||shademodel== GEO_POLY_SHADEMODEL_LIT_GOURAUD) ) { // get the per vertex colours OR per face colours.
                            gfd=(*itr)->getField(GEO_DB_POLY_PACKED_COLOR); // the colour
                            if (gfd) {
                                unsigned char *cls=gfd->getUCh4Arr();
                                float red=cls[0]/255.0f;
                                float green=cls[1]/255.0f;
                                float blue=cls[2]/255.0f;
                                float alpha=1.0f; // cls[3]*frac/255.0f;
                                ginf.getVinf()->addPolcolour(osg::Vec4(red,green,blue,alpha));
                            } else { // get colour from palette
                                gfd=(*itr)->getField(GEO_DB_POLY_COLOR_INDEX); // use color pool...
                                int icp= gfd ? gfd->getInt() : 0;
                                float col[4];
                                theHeader->getPalette(icp, col);
                                ginf.getVinf()->addPolcolour(osg::Vec4(col[0],col[1],col[2],1.0));
                            }
                        }
                        nug->addChild(mtr);
                        mtr->addChild(gd);
                        osg::Geometry *nugeom=makeNewGeometry((*itr), ginf, imat);
                        int nv=getprim((*itr),ginf);
                        gd->addDrawable(nugeom); // now add the polygon
                        if (dstyle==GEO_POLY_DSTYLE_SOLID_BOTH_SIDES || dstyle == GEO_POLY_DSTYLE_SOLID) nugeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POLYGON,0,nv));
                        if (dstyle==GEO_POLY_DSTYLE_OPEN_WIRE) nugeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,0,nv));
                        if (dstyle==GEO_POLY_DSTYLE_CLOSED_WIRE) nugeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP,0,nv));
                        if (dstyle==GEO_POLY_DSTYLE_POINTS) nugeom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::POINTS,0,nv));
                        addPolyActions(bhv, ginf ,nv);
                    }
                }

            }
            return nanimations;
        }
        bool hasColorAction(std::vector< georecord *>bhv) { // true if one of the actions changes colour
            bool ok=false;
        //    if (bhv) {
                for (std::vector< georecord *>::const_iterator rcitr=bhv.begin();
                rcitr!=bhv.end() && !ok;
                ++rcitr)
                {
                    switch ((*rcitr)->getType()) {
                    case DB_DSK_COLOR_RAMP_ACTION:
                        ok=true;
                        break;
                    default:
                        break;
                    }
                }
        //    }
            return ok;
        }
        bool hasMotionAction(std::vector< georecord *>bhv) { // true if one of the actions is a motion
            bool ok=false;
        //    if (bhv) {
                for (std::vector< georecord *>::const_iterator rcitr=bhv.begin();
                rcitr!=bhv.end() && !ok;
                ++rcitr)
                {
                    switch ((*rcitr)->getType()) {
                    case DB_DSK_ROTATE_ACTION:
                    case DB_DSK_SCALE_ACTION:
                    case DB_DSK_TRANSLATE_ACTION:
                        ok=true;
                        break;
                    default:
                        break;
                    }
                }
                //    }
                return ok;
        }
        geoInfo *getGeometry(const georecord *grec,Geode *nug, std::vector<class geoInfo> *ia,
            const unsigned int imat, const int shademodel, const int bothsides) {
            int igidx=0, igeom=-1;
            const geoField *gfd=grec->getField(GEO_DB_POLY_TEX0);
            int txidx= gfd ? gfd->getInt() : -1;
            for (std::vector<class geoInfo>::iterator itrint=ia->begin();
            itrint!=ia->end() && igeom<0;
            ++itrint) { // find a geometry that shares this texture.
                // also test for other properties of a unique material:
                // - use material/vertex colours;
                geoInfo gu(txidx,shademodel, bothsides);
                if (gu==&(*itrint) && !(*itrint).getGeom()->getUpdateCallback()) igeom=igidx;
                igidx++;
            }
            std::vector< georecord *>bhv=grec->getBehaviour(); // behaviours attached to facets, eg colour!
            if (igeom<0 || hasColorAction(bhv)) { // we need a new geometry for this due to new texture/material combo or an action
                gfd=grec->getField(GEO_DB_POLY_SHADEMODEL); // shaded gouraud, flat...
                int shademodel=gfd ? gfd->getInt() : GEO_POLY_SHADEMODEL_LIT_GOURAUD;
                geoInfo gi(txidx,shademodel, bothsides);
                gi.setPools(&coord_pool, &normal_pool);
                gfd=grec->getField(GEO_DB_POLY_LINE_WIDTH); // integer line width...
                if (gfd) {
                    int w=gfd->getInt();
                    gi.setlineWidth(w);
                }
                osg::Geometry *nugeom=makeNewGeometry(grec, gi, imat);
                nug->addDrawable(nugeom);
                igeom=ia->size();
                ia->push_back(gi); // look up table for which texture corresponds to which geom
            }
            return (&((*ia)[igeom]));
        }
        int makeGeometry(const georecord &grec, const unsigned int imat,Geode *nug)
        {    // makegeometry makes a set of Geometrys attached to current parent (Geode nug)
            const std::vector<georecord *> gr=grec.getchildren();
            // std::vector<osg::Geometry *> geom;
            if (gr.size()>0) {
                std::vector<class geoInfo> ia; // list of texture indices & vinfo found in this geode; sort into new 
                const geoField *gfd=grec.getField(GEO_DB_RENDERGROUP_CULLING); // back, front, none
                unsigned int bothsides=gfd ? gfd->getUInt() : 0;
                //  vertexInfo vinf(&coord_pool, &normal_pool); // holds all types of coords, indices etc
//                bool bothsides=allOneSided(&grec);
                for (std::vector<georecord *>::const_iterator itr=gr.begin();
                itr!=gr.end();
                ++itr) {
                    std::vector< georecord *>bhv=(*itr)->getBehaviour(); // behaviours attached to facets, eg colour!
                    if ( !hasMotionAction(bhv)) { // animated facets go elsewhere
                        if ((*itr)->getType()==DB_DSK_POLYGON) { // a normal facet
                            const geoField *gfd=(*itr)->getField(GEO_DB_POLY_DSTYLE); // solid, wire...
                            int dstyle= gfd ? gfd->getInt() : GEO_POLY_DSTYLE_SOLID;
                            gfd=(*itr)->getField(GEO_DB_POLY_SHADEMODEL); // shaded gouraud, flat...
                            int shademodel=gfd ? gfd->getInt() : GEO_POLY_SHADEMODEL_LIT_GOURAUD;
                            geoInfo *gi=getGeometry((*itr), nug, &ia, imat,shademodel, bothsides);
                            
                            //shade models GEO_POLY_SHADEMODEL_FLAT GEO_POLY_SHADEMODEL_GOURAUD
                            //    GEO_POLY_SHADEMODEL_LIT GEO_POLY_SHADEMODEL_LIT_GOURAUD
                            gfd=(*itr)->getField(GEO_DB_POLY_USE_MATERIAL_DIFFUSE); // true: use material...
                            bool usemat= gfd ? gfd->getBool() : false;
                            if (!usemat || 
                                shademodel== GEO_POLY_SHADEMODEL_LIT /*||shademodel== GEO_POLY_SHADEMODEL_LIT_GOURAUD) */ ) { // get the per vertex colours OR per face colours.
                                gfd=(*itr)->getField(GEO_DB_POLY_PACKED_COLOR); // the colour
                                if (gfd) {
                                    unsigned char *cls=gfd->getUCh4Arr();
                                    float red=cls[0]/255.0f;
                                    float green=cls[1]/255.0f;
                                    float blue=cls[2]/255.0f;
                                    float alpha=1.0f; // cls[3]*frac/255.0f;
                                    gi->getVinf()->addPolcolour(osg::Vec4(red,green,blue,alpha));
                                } else { // get colour from palette
                                    gfd=(*itr)->getField(GEO_DB_POLY_COLOR_INDEX); // use color pool...
                                    int icp= gfd ? gfd->getInt() : 0;
                                    float col[4];
                                    theHeader->getPalette(icp, col);
                                    gi->getVinf()->addPolcolour(osg::Vec4(col[0],col[1],col[2],1.0));
                                }
                            }
                            int nv=getprim((*itr), *gi);
                            {
                                const vertexInfo *vinf=gi->getVinf();
                                if (vinf->getNorms() && vinf->getNorms()->size()>0) {
                                    gi->getGeom()->setNormalArray(vinf->getNorms());
                                    gi->getGeom()->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
                                } else {
                                    gi->getGeom()->setNormalBinding(osg::Geometry::BIND_OFF);
                                }
                            }
                            if (hasColorAction(bhv)) addPolyActions(bhv, *gi, nv);
                            
                            if (dstyle==GEO_POLY_DSTYLE_SOLID_BOTH_SIDES || dstyle == GEO_POLY_DSTYLE_SOLID) {
                                osg::DrawArrays *drw=new osg::DrawArrays(osg::PrimitiveSet::POLYGON,gi->getStart(nv),nv);                                
                                gi->getGeom()->addPrimitiveSet(drw);
                            }
                            if (dstyle == GEO_POLY_DSTYLE_OPEN_WIRE) {
                                osg::DrawArrays *drw=new osg::DrawArrays(osg::PrimitiveSet::LINE_STRIP,gi->getStart(nv),nv);
                                gi->getGeom()->addPrimitiveSet(drw);
                            }
                            if (dstyle == GEO_POLY_DSTYLE_CLOSED_WIRE) {
                                osg::DrawArrays *drw=new osg::DrawArrays(osg::PrimitiveSet::LINE_LOOP,gi->getStart(nv),nv);
                                gi->getGeom()->addPrimitiveSet(drw);
                            }
                            if (dstyle==GEO_POLY_DSTYLE_POINTS) {
                                osg::DrawArrays *drw=new osg::DrawArrays(osg::PrimitiveSet::POINTS,gi->getStart(nv),nv);
                                gi->getGeom()->addPrimitiveSet(drw);
                            }
                        }
                    }
                }
                {
                    int igeom=0;
                    for (std::vector<geoInfo>::iterator itr=ia.begin();
                    itr!=ia.end();
                    ++itr) {
                        if ((*itr).getTexture() >=0) {
                            osg::Vec2Array *txa=ia[igeom].getVinf()->getTexCoords();
                            if (txa->size() > 0 ) {
                                ((*itr).getGeom())->setTexCoordArray(0, txa);
                            }
                        }
                        igeom++;
                    }
                }
            }
            return gr.size();
        }
        void makeTexts(georecord grec, Group *nug)
        {    // makeTexts adds a set of text+transform Geometrys attached to current parent (Group nug)
            const std::vector<georecord *> gr=grec.getchildren();
            std::vector<osg::Geometry *> geom;
            if (gr.size()>0) {
                std::vector<int> ia; // list of texture indices found in this geode; sort into new 
                for (std::vector<georecord *>::const_iterator itr=gr.begin();
                itr!=gr.end();
                ++itr) {
                    if ((*itr)->getType()==DB_DSK_TEXT) {
                        osg::MatrixTransform *text=makeText((*itr));
                        if (text) nug->addChild(text);
                    }
                }
                // osg::notify(osg::WARN) << vinf;
            }
            return;
        }
        Group *makeTextGeode(const georecord *gr)
        {
             // in geo text is defined with a matrix included in the geo.geode (gr is this geo.geode)
            // - we need to create this tree to render text
#if 1            
            return NULL; // temporary disable april 2003
#else
            Group *nug=new Group;
            const geoField *gfd=gr->getField(GEO_DB_RENDERGROUP_MAT);
            // may be used in future const unsigned int imat=gfd ? gfd->getInt():0;
            gfd=gr->getField(GEO_DB_NODE_NAME);
            if (gfd) {
                nug->setName(gfd->getChar());
            }
            makeTexts((*gr),nug);
            if (nug->getNumChildren() <=0) {
                nug=NULL;
            }
            return nug;
#endif
        }
        
        Group *makeLightPointGeodes(const georecord *gr) {
            const geoField *gfd=gr->getField(GEO_DB_RENDERGROUP_MAT);
            Group *nug=new Group;
            gfd=gr->getField(GEO_DB_NODE_NAME);
            if (gfd) {
                char *name = gfd->getChar();
                nug->setName(name);
            }
            makeLightPointGeometry(gr,nug);
            if (nug->getNumChildren() <=0) {
                nug=NULL;
            }
            return nug;
        }
        Group *makeAnimatedGeodes(const georecord *gr)
        { // create a group full of animated geodes.  Used for any animations applied to facets!
            // movement actions require a transform node to be inserted, and this cannot be 
            // derived from Geode.  So create a group, add matrix transform(s) for each animated polygon
            const geoField *gfd=gr->getField(GEO_DB_RENDERGROUP_MAT);
            const int imat=gfd ? gfd->getInt():0;
       //     gfd=gr->getField(GEO_DB_RENDERGROUP_IS_BILLBOARD);
         //   bool isbillb = gfd ? gfd->getBool() : false;
            Group *nug=new Group;
       /*     if (isbillb) {
                Billboard *bilb= new Billboard ;
                bilb->setAxis(Vec3(0,0,1));
                bilb->setNormal(Vec3(0,-1,0));
                nug=bilb;
            } else {
                nug=new Geode;
            } */
            gfd=gr->getField(GEO_DB_NODE_NAME);
            if (gfd) {
                char *name = gfd->getChar();
                nug->setName(name);
            }
            int nans=makeAnimatedGeometry((*gr),imat,nug);
            if (nans <=0) {
                nug=NULL;
            }
            return nug;
        }
        Geode *makeGeode(const georecord &gr)
        {
            const geoField *gfd=gr.getField(GEO_DB_RENDERGROUP_MAT);
            const unsigned int imat=gfd ? gfd->getInt():0;
            gfd=gr.getField(GEO_DB_RENDERGROUP_BILLBOARD);
            bool isbillb = gfd ? gfd->getBool() : false;
            osg::Geode *nug;
            if (isbillb) {
                Billboard *bilb= new Billboard ;
                bilb->setAxis(Vec3(0,0,1));
                bilb->setNormal(Vec3(0,-1,0));
                nug=bilb;
            } else {
                nug=new Geode;
            }
            int nchild=makeGeometry(gr,imat,nug);
            if (nchild>0) { // complete the geode
                gfd=gr.getField(GEO_DB_NODE_NAME);
                if (gfd) {
                    nug->setName(gfd->getChar());
                }
                return nug;
            } else {
                return NULL;
            }
        }
        osg::Group *makePage(const georecord *gr)
        {
            osg::Group *gp=new Group;
            const geoField *gfd=gr->getField(GEO_DB_NODE_NAME);
            if (gfd) {
                gp->setName(gfd->getChar());
            }
            return gp;
        }
        osg::Group *setmatrix(const georecord *gr) { // find one of the types of matrix supported
            const geoField *gfd=gr->getField(GEO_DB_GRP_MATRIX_TRANSFORM);
            if (!gfd) gfd=gr->getField(GEO_DB_GRP_TRANSLATE_TRANSFORM);
            if (!gfd) gfd=gr->getField(GEO_DB_GRP_ROTATE_TRANSFORM);
            if (!gfd) gfd=gr->getField(GEO_DB_GRP_SCALE_TRANSFORM);
            if (gfd) {
                MatrixTransform *tr=new MatrixTransform;
                osg::Matrix mx;
                float * m44=gfd->getMat44Arr();
                mx.set(m44); // hope uses same convention as OSG else will need to use set(m44[0],m44[1]...)
                tr->setMatrix(mx);
                return tr;
            } else {
                return NULL;
            }
        }
        osg::Group *makeGroup(const georecord *gr) { // group or Static transform
            osg::Group *gp=setmatrix(gr);
            if (!gp) {
                gp=new osg::Group;
            }
            const geoField *gfd=gr->getField(GEO_DB_NODE_NAME);
            if (gfd) {
                gp->setName(gfd->getChar());
            }
            return gp;
        }
        osg::Group *makeSwitch(const georecord *gr)
        {
            osg::Switch *sw=new Switch;
            const geoField *gfd=gr->getField(GEO_DB_SWITCH_CURRENT_MASK);
            sw->setAllChildrenOff();
            if (gfd) {
                int imask;
                
                imask=gfd->getInt();
                
                // set the bits in the osg::Switch.
                int selector_mask = 0x1;
                for(int pos=0;pos<32;++pos)
                {
                    sw->setValue(pos,((imask&selector_mask)!=0)); 
                    selector_mask <<= 1;
                }
                osg::notify(osg::WARN) << gr << " imask " << imask << std::endl;
            } else {
                sw->setSingleChildOn(0);
                osg::notify(osg::WARN) << gr << " Switch has No mask- only 1 child " << std::endl;
            }
            gfd=gr->getField(GEO_DB_NODE_NAME);
            if (gfd) {
                sw->setName(gfd->getChar());
            }
            return sw;
        }
        
        osg::Sequence *makeSequence(const georecord *gr)
        {
            Sequence *sq=new Sequence;
            const geoField *gfd=gr->getField(GEO_DB_NODE_NAME);
            if (gfd) {
                sq->setName(gfd->getChar());
            }
            return sq;
        }
        osg::LOD *makeLOD(const georecord *gr)
        {
            osg::LOD *gp=new LOD;
            const geoField *gfd=gr->getField(GEO_DB_LOD_IN);
            float in  = gfd ? gfd->getFloat() : 100.0;
            gfd=gr->getField(GEO_DB_LOD_OUT);
            float out = gfd ? gfd->getFloat() : 0.0;
            gp->setRange(0,out,in);
            gfd=gr->getField(GEO_DB_NODE_NAME);
            if (gfd) {
                gp->setName(gfd->getChar());
            }
            return gp;
        }
        osg::Drawable* createClipSurface(float xMin,float xMax,float yMin,float yMax,float z)
        {            // set up the Geometry that defines the clipped region.
            osg::Geometry* geom = new osg::Geometry;
            
            osg::Vec3Array* coords = new osg::Vec3Array(4);
            (*coords)[0].set(xMin,yMax,z);
            (*coords)[1].set(xMin,yMin,z);
            (*coords)[2].set(xMax,yMin,z);
            (*coords)[3].set(xMax,yMax,z);
            geom->setVertexArray(coords);
            
            geom->addPrimitiveSet(new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4));
            
            return geom;
        }
        Group *makeClipRegion(const georecord *gr) {
            GeoClipRegion *clp=new GeoClipRegion;
            const geoField *gfd=gr->getField(GEO_DB_NODE_NAME);
            if (gfd) {
                clp->setName(gfd->getChar());
            }
            gfd=gr->getField(140);
            float *lleft = (gfd) ? (gfd->getVec3Arr()):NULL;
            gfd=gr->getField(141);
            float *uright= (gfd) ? (gfd->getVec3Arr()):NULL;
            if (uright && lleft) {
                Geode *geod=new Geode;
                Drawable *drw=createClipSurface(lleft[0],uright[0],lleft[1],uright[1],lleft[2]);
                geod->addDrawable(drw);
                clp->addClipNode(geod);
            }
            return clp;
        }

        geoHeader *makeHeader(const georecord *gr, const osgDB::ReaderWriter::Options* options) {
            if (!theHeader.valid()) theHeader=new geoHeaderGeo();
            // the header contains variables as well as a transform for the XYZup cases
            const geoField *gfd;
            if (cpalrec) { // global - attach to geoheader
                gfd=cpalrec->getField(GEO_DB_COLOR_PALETTE_HIGHEST_INTENSITIES);
                if (gfd) {
                    unsigned char *cpal=gfd->getstore(0);
                    for (uint i=1; i<gfd->getNum(); i++) {
                        theHeader->addColour(cpal);
                        cpal+=4;
                    }
                }
            }
            gfd=gr->getField(GEO_DB_HDR_UP_AXIS);
            osg::Quat q;
            int iup=gfd ? gfd->getInt() : GEO_DB_UP_AXIS_Y;

            switch (iup) {
            case GEO_DB_UP_AXIS_X:
                    q.set(0,1,0,1);
                    q/=q.length();
                    theHeader->setAttitude(q);
                break;
            case GEO_DB_UP_AXIS_Y:
                    q.set(1,0,0,1);
                    q/=q.length();
//                    theHeader->setMatrix(Matrix::rotate(pi2,  osg::Vec3(1,0,0 )));//setAttitude(q);
                    theHeader->setAttitude(q);
                break;
            case GEO_DB_UP_AXIS_Z: // no change
                    q.set(0,0,0,1);
                    q/=q.length();
                    theHeader->setAttitude(q); // set(q);
                break;
            }
            std::vector<georecord *>::const_iterator itr;
            for (itr=geotxlist.begin(); itr<geotxlist.end(); itr++) {
                makeTexture(*itr, options);
            }
            std::vector< georecord *>bhv=gr->getBehaviour();
            if (!bhv.empty()) { // then add internal, user, extern variables
                for (std::vector< georecord *>::const_iterator rcitr=bhv.begin();
                rcitr!=bhv.end();
                ++rcitr)
                {
                    if ((*rcitr)->getType()==DB_DSK_INTERNAL_VARS) {
                        theHeader->addInternalVars(**rcitr);
                //        theHeader->setUpdateCallback(theHeader->getInternalVars());                        
                    }
                    if ((*rcitr)->getType()==DB_DSK_FLOAT_VAR) {
                        if (theHeader.valid()) theHeader->addUserVar((**rcitr));
                    }
                }
                theHeader->setUpdateCallback(new geoHeaderCB);
            }
            for (itr=geomatlist.begin(); itr< geomatlist.end(); itr++) {
                 osg::Material *mt=new osg::Material;
                 (*itr)->setMaterial(mt);
                 matlist.push_back(mt);
            }
            return theHeader.get();
        }
        void makeTexture(const georecord *gr, const osgDB::ReaderWriter::Options* options) {
            // scans the fields of this record and puts a new texture & environment into 'pool' stor
            const geoField *gfd=gr->getField(GEO_DB_TEX_FILE_NAME);
            const char *name = gfd->getChar();
            if (name) {
                osg::ref_ptr<osg::Texture2D> tx = new Texture2D;
                osg::ref_ptr<osg::Image> ctx = osgDB::readImageFile(name,options);
                if (ctx.valid()) {
                    ctx->setFileName(name);
                    tx->setImage(ctx.get());
                }
                gfd=gr->getField(GEO_DB_TEX_WRAPS);
                osg::Texture2D::WrapMode wm=Texture2D::REPEAT;
                if (gfd) {
                    unsigned iwrap= gfd->getUInt();
                    wm = (iwrap==GEO_DB_TEX_CLAMP) ? Texture2D::CLAMP : Texture2D::REPEAT;
                }
                tx->setWrap(Texture2D::WRAP_S, wm);
                gfd=gr->getField(GEO_DB_TEX_WRAPT);
                wm=Texture2D::REPEAT;
                if (gfd) {
                    unsigned iwrap= gfd->getUInt();
                    wm = (iwrap==GEO_DB_TEX_CLAMP) ? Texture2D::CLAMP : Texture2D::REPEAT;
                }
                tx->setWrap(Texture2D::WRAP_T, wm);
                txlist.push_back(tx.get());
                osg::TexEnv* texenv = new osg::TexEnv;
                osg::TexEnv::Mode md=osg::TexEnv::MODULATE;
                gfd=gr->getField(GEO_DB_TEX_ENV);
                texenv->setMode(md);
                if (gfd) {
                    unsigned imod=gfd->getUInt();
                    switch (imod) {
                    case GEO_DB_TEX_MODULATE:
                        md=osg::TexEnv::MODULATE;
                        break;
                    case GEO_DB_TEX_DECAL:
                        md=osg::TexEnv::DECAL;
                        break;
                    case GEO_DB_TEX_BLEND:
                        md=osg::TexEnv::BLEND;
                        break;
                    }
                }
                gfd=gr->getField(GEO_DB_TEX_MINFILTER);
                osg::Texture::FilterMode filt=osg::Texture::NEAREST_MIPMAP_NEAREST;
                if (gfd) {
                    unsigned imod=gfd->getUInt();
                    switch (imod) {
                    case GEO_DB_TEX_NEAREST_MIPMAP_NEAREST:
                        filt=osg::Texture::LINEAR_MIPMAP_LINEAR;
                        break;
                    case GEO_DB_TEX_LINEAR_MIPMAP_NEAREST:
                        filt=osg::Texture::LINEAR_MIPMAP_NEAREST;
                        break;
                    case GEO_DB_TEX_NEAREST_MIPMAP_LINEAR:
                        filt=osg::Texture::NEAREST_MIPMAP_LINEAR;
                        break;
                    case GEO_DB_TEX_LINEAR_MIPMAP_LINEAR:
                        filt=osg::Texture::NEAREST_MIPMAP_NEAREST;
                        break;
                    }
                }
                tx->setFilter(osg::Texture::MIN_FILTER, filt);
                gfd=gr->getField(GEO_DB_TEX_MAGFILTER);
                if (gfd) { 
                    unsigned imod=gfd->getUInt();
                    switch (imod) {
                    case GEO_DB_TEX_NEAREST:
                        filt=osg::Texture::LINEAR;
                        break;
                    case GEO_DB_TEX_LINEAR:
                        filt=osg::Texture::NEAREST;
                        break;
                    }
                }
                txenvlist.push_back(texenv);
            }
        }
        MatrixTransform *makeBehave(const georecord *gr)
        {
            MatrixTransform *mtr=NULL;
            bool ok=false; // true if the matrix transform is required
            std::vector< georecord *>bhv=gr->getBehaviour();
            if (!bhv.empty()) { // then add a DCS/matrix_transform
                mtr=new MatrixTransform;
                geoBehaviourCB *gcb=new geoBehaviourCB;
                mtr->setUpdateCallback(gcb);

                for (std::vector< georecord *>::const_iterator rcitr=bhv.begin();
                rcitr!=bhv.end();
                ++rcitr)
                {
                    switch ((*rcitr)->getType()) {
                    case DB_DSK_BEHAVIOR: {
                        const geoField *gfd=(*rcitr)->getField(GEO_DB_BEHAVIOR_NAME);
                        if (gfd) {
                            mtr->setName(gfd->getChar());
                        }
                                          }
                        break;
                    case DB_DSK_ROTATE_ACTION: {
                        geoMoveBehaviour *cb= new geoMoveBehaviour;
                        ok=cb->makeBehave((*rcitr), theHeader.get());
                        if (ok) gcb->addBehaviour(cb);
                        else delete cb;
                                               }
                        break;
                        
                    case DB_DSK_SCALE_ACTION: {
                        geoMoveBehaviour *sb=new geoMoveBehaviour;
                        ok=sb->makeBehave((*rcitr), theHeader.get());
                        if (ok) gcb->addBehaviour(sb);
                        else delete sb;
                                              }
                        break;
                    case DB_DSK_TRANSLATE_ACTION: {
                        geoMoveBehaviour *cb= new geoMoveBehaviour;
                        ok=cb->makeBehave((*rcitr), theHeader.get());
                        if (ok) gcb->addBehaviour(cb);
                        else delete cb;
                                                  }
                        break;
                        
                    case DB_DSK_COMPARE_ACTION: {
                        geoCompareBehaviour *cb=new geoCompareBehaviour;
                        ok=cb->makeBehave((*rcitr), theHeader.get());
                        if (ok) gcb->addBehaviour(cb);
                        else delete cb;
                                                }
                        break;
                    case DB_DSK_ARITHMETIC_ACTION: {
                        geoArithBehaviour *cb=new geoArithBehaviour;
                        ok=cb->makeBehave((*rcitr), theHeader.get());
                        if (ok) gcb->addBehaviour(cb);
                        else delete cb;
                                                   }
                        break;
                    case DB_DSK_CLAMP_ACTION: {
                        geoClampBehaviour *cb=new geoClampBehaviour;
                        ok=cb->makeBehave((*rcitr), theHeader.get());
                        if (ok) gcb->addBehaviour(cb);
                        else delete cb;
                                              }
                        break;
                    case DB_DSK_RANGE_ACTION: {
                        geoRangeBehaviour *cb=new geoRangeBehaviour;
                        ok=cb->makeBehave((*rcitr), theHeader.get());
                        if (ok) gcb->addBehaviour(cb);
                        else delete cb;
                                              }
                        break;
                    case DB_DSK_VISIBILITY_ACTION: {
                        geoVisibBehaviour *vb = new geoVisibBehaviour;
                        ok=vb->makeBehave((*rcitr), theHeader.get());
                        if (ok) gcb->addBehaviour(vb);
                        else delete vb;
                                                   }
                        break;
                        // ar3 types
                    case DB_DSK_TRIG_ACTION: {
                        geoAr3Behaviour *vb = new geoAr3Behaviour;
                        ok=vb->makeBehave((*rcitr), theHeader.get());
                        if (ok) gcb->addBehaviour(vb);
                        else delete vb;
                                             }
                        break;
                    case DB_DSK_INVERSE_ACTION: {
                        geoAr3Behaviour *vb = new geoAr3Behaviour;
                        ok=vb->makeBehave((*rcitr), theHeader.get());
                        if (ok) gcb->addBehaviour(vb);
                        else delete vb;
                                                }
                        break;
                    case DB_DSK_LINEAR_ACTION: {
                        geoAr3Behaviour *vb = new geoAr3Behaviour;
                        ok=vb->makeBehave((*rcitr), theHeader.get());
                        if (ok) gcb->addBehaviour(vb);
                        else delete vb;
                                               }
                        break;
                    case DB_DSK_PERIODIC_ACTION: {
                        geoAr3Behaviour *vb = new geoAr3Behaviour;
                        ok=vb->makeBehave((*rcitr), theHeader.get());
                        if (ok) gcb->addBehaviour(vb);
                        else delete vb;
                                                 }
                        break;
#ifdef DB_DSK_PERIODIC2_ACTION
                    case DB_DSK_PERIODIC2_ACTION: {
                        geoAr3Behaviour *vb = new geoAr3Behaviour;
                        ok=vb->makeBehave((*rcitr), theHeader.get());
                        if (ok) gcb->addBehaviour(vb);
                        else delete vb;
                                                  }
                        break;
#endif
                    case DB_DSK_TRUNCATE_ACTION: {
                        geoAr3Behaviour *vb = new geoAr3Behaviour;
                        ok=vb->makeBehave((*rcitr), theHeader.get());
                        if (ok) gcb->addBehaviour(vb);
                        else delete vb;
                                                 }
                        break;
                    case DB_DSK_ABS_ACTION: {
                        geoAr3Behaviour *vb = new geoAr3Behaviour;
                        ok=vb->makeBehave((*rcitr), theHeader.get());
                        if (ok) gcb->addBehaviour(vb);
                        else delete vb;
                                            }
                        break;
                        /*
                    case DB_DSK_DCS_ACTION: */
                    case DB_DSK_DISCRETE_ACTION: {
                        geoDiscreteBehaviour *db = new geoDiscreteBehaviour;
                        ok=db->makeBehave((*rcitr), theHeader.get());
                        if (ok) gcb->addBehaviour(db);
                        else delete db;
                                                 }
                        break;
                    case DB_DSK_STRING_CONTENT_ACTION: {// cant be shared with this
                        // ok=false; can be a mixed action, rotate & string content
                                                       }
                        break;
                    case DB_DSK_IF_THEN_ELSE_ACTION:
                        {
                            geoAr3Behaviour *vb = new geoAr3Behaviour;
                            ok=vb->makeBehave((*rcitr), theHeader.get());
                            if (ok) gcb->addBehaviour(vb);
                            else delete vb;
                        }
                        break;
                    }
                }
            }
            if (!ok) {
                mtr=NULL;
            }
            return mtr;
        }
        std::vector<Node *> makeosg(const std::vector<georecord *> gr, const osgDB::ReaderWriter::Options* options) {
            // recursive traversal of records and extract osg::Nodes equivalent
            Group *geodeholder=NULL;
            std::vector<Node *> nodelist;
            if (gr.size()>0) {
                for (std::vector<georecord *>::const_iterator itr=gr.begin();
                itr!=gr.end();
                ++itr) {
                    const georecord *gr=*itr;
                    Group *mtr=makeBehave(gr);
                    if (gr->getType()== DB_DSK_RENDERGROUP) { // geodes can require >1 geometry for example if polygons have different texture indices.
                        // and for example if the node has a colour or other fine behaviour
                        Geode *geode=makeGeode(*gr); // geode of geometrys
                        Group *animatedGeodes= makeAnimatedGeodes(gr);
                        Group *lightptGeodes= makeLightPointGeodes(gr);
                        Group *textgeode=makeTextGeode(gr); // group of matrices & texts
                        const geoField *gfd=gr->getField(GEO_DB_GRP_ZBUFFER);
                        if (gfd) {
                            bool onoff=gfd->getBool();
                            if (!onoff) { // no z buffer - force to use unsorted renderBin
                                StateSet *dstate=new StateSet;
                                osg::Depth* depth = new osg::Depth;
                                depth->setFunction(osg::Depth::ALWAYS);
                                dstate->setAttribute(depth);
                                dstate->setMode(GL_DEPTH_TEST,osg::StateAttribute::OFF);
                                dstate->setRenderBinDetails(osg::StateSet::TRANSPARENT_BIN + 1,"RenderBin");
                        //        dstate->setRenderBinDetails(osg::StateSet::TRANSPARENT_BIN + 12,  "UnSortedBin");
                                if (geode) geode->setStateSet( dstate );
                                if (animatedGeodes) animatedGeodes->setStateSet( dstate );
                                if (lightptGeodes) lightptGeodes->setStateSet( dstate );
                                if (textgeode) textgeode->setStateSet( dstate );
                            }
                        }

                        if (mtr) {
                            if (geode) mtr->addChild(geode);
                            if (animatedGeodes) mtr->addChild(animatedGeodes);
                            if (lightptGeodes) mtr->addChild(lightptGeodes);
                            if (textgeode) mtr->addChild(textgeode);
                            nodelist.push_back(mtr);
                            mtr=NULL;
                        } else {
                            if (!geodeholder && (geode || textgeode)) {
                                geodeholder=new osg::Group;
                                geodeholder->setName("geodeHolder");
                            }
                            if (geode) geodeholder->addChild(geode);
                            if (animatedGeodes) geodeholder->addChild(animatedGeodes);
                            if (lightptGeodes) geodeholder->addChild(lightptGeodes);
                            if (textgeode) geodeholder->addChild(textgeode);
                        }
                    } else {
                        Group *holder=NULL;
                        const geoField *gfd;
                        switch (gr->getType()) {
                        case 101:
                        case DB_DSK_HEADER:
                            holder=new osg::Group; // makeGroup(gr);
                            if (mtr) {
                                mtr->addChild(holder);
                                holder=mtr;
                            }
                            (*itr)->setNode(holder);
                            break;
/*                            holder= theHeader.get(); // makeHeader(gr);// 
                            (*itr)->setNode(holder);
                            if (mtr) {
                                holder->addChild(mtr);
                                osg::Group *grp=makeGroup(gr);
                                mtr->addChild(grp);
                                holder=mtr;
                            }
                            break; */
                        case DB_DSK_TEXTURE:
                            makeTexture(gr, options);
                            break;
                        case DB_DSK_BASE_GROUP: // start of a group plus extra features
                            holder=makeClipRegion(gr);
                            if (mtr) {
                                mtr->addChild(holder);
                                holder=mtr;
                            }
                            (*itr)->setNode(holder);
                            break;
                        case DB_DSK_GROUP:
                            holder=makeGroup(gr);
                            if (mtr) {
                                mtr->addChild(holder);
                                holder=mtr;
                            }
                            (*itr)->setNode(holder);
                            break;
                        case DB_DSK_LOD: 
                            holder=makeLOD(gr);
                            (*itr)->setNode(holder);
                            break;
                        case DB_DSK_SEQUENCE:
                            holder=makeSequence(gr);
                            (*itr)->setNode(holder);
                            break;
                        case DB_DSK_SWITCH:
                            holder=makeSwitch(gr);
                            (*itr)->setNode(holder);
                            break;
                        case DB_DSK_CUBE:
                            holder=new Group;
                            (*itr)->setNode(holder);
                            gfd=gr->getField(GEO_DB_NODE_NAME);
                            if (gfd) {
                                holder->setName(gfd->getChar());
                            }
                            break;
                        case DB_DSK_SPHERE:
                            holder=new Group;
                            (*itr)->setNode(holder);
                            gfd=gr->getField(GEO_DB_NODE_NAME);
                            if (gfd) {
                                holder->setName(gfd->getChar());
                            }
                            break;
                        case DB_DSK_CONE:
                            holder=new Group;
                            (*itr)->setNode(holder);
                            gfd=gr->getField(GEO_DB_NODE_NAME);
                            if (gfd) {
                                holder->setName(gfd->getChar());
                            }
                            break;
                        case DB_DSK_CYLINDER:
                            holder=new Group;
                            (*itr)->setNode(holder);
                            gfd=gr->getField(GEO_DB_NODE_NAME);
                            if (gfd) {
                                holder->setName(gfd->getChar());
                            }
                            break;
                        case DB_DSK_INSTANCE:
                            {
                                MatrixTransform *mtr=new MatrixTransform;
                                gfd=gr->getField(GEO_DB_NODE_NAME);
                                if (gfd) {
                                    mtr->setName(gfd->getChar());
                                }
                                gfd=gr->getField(GEO_DB_GRP_MATRIX_TRANSFORM); // was: GEO_DB_INSTANCE_TRANSFORM);
                                if (gfd) {
                                    float *fmat=gfd->getMat44Arr();
                                    mtr->setMatrix(Matrix(fmat));
                                }
                                gfd=gr->getField(GEO_DB_INSTANCE_DEF);
                                if (gfd) { // get the fID of a node
                                    uint fid=gfd->getUInt();
                                    georecord *grec=getInstance(fid);
                                    if (grec) {
                                        osg::Node *nd=grec->getNode();
                                        if (nd) { // node already loaded, so instance
                                            mtr->addChild(nd);
                                            holder=mtr;
                                        } else { // store unsatisfied instance matrix in georecord...
                                            grec->addInstance(mtr);
                                        }
                                    }
                                }
                            }
                            break;
                        case DB_DSK_PAGE:
                            holder=makePage(gr);
                            (*itr)->setNode(holder);
                            break;
                        case DB_DSK_PERSPECTIVE_GRID_INFO: 
                            { // relates to how model is viewed in Geo modeller
                                osg::Group *gp=new Group;
                                holder=gp;
                            }
                            break;
                        case DB_DSK_FLOAT_VAR:
                        case DB_DSK_INT_VAR:
                        case DB_DSK_LONG_VAR:
                        case DB_DSK_DOUBLE_VAR:
                        case DB_DSK_BOOL_VAR:
                        case DB_DSK_FLOAT2_VAR:
                        case DB_DSK_FLOAT3_VAR:
                        case DB_DSK_FLOAT4_VAR:
                        case DB_DSK_INTERNAL_VARS:                   
                        case DB_DSK_LOCAL_VARS:                   
                        case DB_DSK_EXTERNAL_VARS:
                        case DB_DSK_CLAMP_ACTION:
                        case DB_DSK_RANGE_ACTION:    
                        case DB_DSK_ROTATE_ACTION:        
                        case DB_DSK_TRANSLATE_ACTION:    
                        case DB_DSK_SCALE_ACTION:                    
                        case DB_DSK_ARITHMETIC_ACTION:            
                        case DB_DSK_LOGIC_ACTION:                
                        case DB_DSK_CONDITIONAL_ACTION:        
                        case DB_DSK_LOOPING_ACTION:            
                        case DB_DSK_COMPARE_ACTION:           
                        case DB_DSK_VISIBILITY_ACTION:         
                        case DB_DSK_STRING_CONTENT_ACTION:
                        default: {
                            osg::Group *gp=new Group;
                            osg::notify(osg::WARN) << "Unhandled item " << gr->getType() <<
                               "address " << (*itr) << std::endl;
                            holder=gp;
                            }
                            break;
                        }
                        if (holder) nodelist.push_back(holder);

                        std::vector<Node *> child=makeosg((*itr)->getchildren(), options);
                        GeoClipRegion *clip=dynamic_cast<GeoClipRegion *>(holder);
                        for (std::vector<Node *>::iterator itr=child.begin();
                            itr!=child.end();
                            ++itr) {
                                if (clip) clip->addClippedChild(*itr);
                                else holder->addChild(*itr);
                        }
                    }
                }
            }
            if (geodeholder) {
                osgUtil::Tessellator tessellator;
                for(unsigned int ige=0;ige<geodeholder->getNumChildren();++ige) {
                    osg::Geode *geode=dynamic_cast<osg::Geode*>(geodeholder->getChild(ige));
                    if (geode) {
                        for(unsigned int i=0;i<geode->getNumDrawables();++i)
                        {
                            osg::Geometry* geom = dynamic_cast<osg::Geometry*>(geode->getDrawable(i));
                            if (geom) tessellator.retessellatePolygons(*geom);
                        }
                    }
                }
                nodelist.push_back(geodeholder);
            }
            return nodelist;
        }
        void output(osgDB::Output &fout,std::vector<georecord> gr)
        { // debugging - print the tree of records
            if (gr.size()>0) {
                for (std::vector<georecord>::iterator itr=gr.begin();
                    itr!=gr.end();
                    ++itr) {
                    fout.indent() << "Node type " << (*itr).getType() << " ";
                    fout.indent() << (*itr) << std::endl;
                }
            }
        }
        void output(osgDB::Output &fout,std::vector<georecord *> gr)
        { // debugging - print the tree of records
            fout.moveIn();
            if (gr.size()>0) {
                for (std::vector<georecord *>::iterator itr=gr.begin();
                    itr!=gr.end();
                    ++itr) {
                    fout.indent() << "Node type " << (*itr)->getType() << " ";
                    fout.indent() << (**itr) << std::endl;
                    fout.indent() << std::endl;
                    output(fout,(*itr)->getchildren());
                }
            }
            fout.moveOut();
        }
        georecord *getInstance(uint fid) { // find record with instance fid
            for (geoRecordList::iterator itr=recs.begin();
                itr!=recs.end();
                ++itr) {
                    const geoField *gfd;
                switch ((*itr).getType()) {
                case DB_DSK_GROUP:
                    gfd=(*itr).getField(GEO_DB_GRP_INSTANCE_DEF);
                    if (gfd) {
                        uint fidnod=gfd->getUInt();
                        if (fidnod==fid) return &(*itr);
                    }
                    break;
                case DB_DSK_LOD:
                    gfd=(*itr).getField(GEO_DB_INSTANCE_DEF);
                    if (gfd) {
                        uint fidnod=gfd->getUInt();
                        if (fidnod==fid) return &(*itr);
                    }
                    break;
                case DB_DSK_SEQUENCE:
                    gfd=(*itr).getField(GEO_DB_INSTANCE_DEF);
                    if (gfd) {
                        uint fidnod=gfd->getUInt();
                        if (fidnod==fid) return &(*itr);
                    }
                    break;
                case DB_DSK_SWITCH:
                    gfd=(*itr).getField(GEO_DB_INSTANCE_DEF);
                    if (gfd) {
                        uint fidnod=gfd->getUInt();
                        if (fidnod==fid) return &(*itr);
                    }
                    break;
                case DB_DSK_RENDERGROUP:
                    gfd=(*itr).getField(GEO_DB_INSTANCE_DEF);
                    if (gfd) {
                        uint fidnod=gfd->getUInt();
                        if (fidnod==fid) return &(*itr);
                    }
                    break;
                }
            }
            return NULL;
        }
                        
private:
    geoRecordList recs; // the records read from file
    std::vector<osg::Vec3> coord_pool; // current vertex ooords
    std::vector<osg::Vec3> normal_pool; // current pool of normal vectors
    osg::ref_ptr<geoHeaderGeo> theHeader; // an OSG class - has animation vars etc
    std::vector<georecord *> geotxlist; // list of geo::textures for this model
    std::vector<georecord *> geomatlist; // list of geo::materials for this model
    std::vector< osg::ref_ptr<osg::Texture2D> > txlist; // list of osg::textures for this model
    std::vector< osg::ref_ptr<osg::TexEnv> > txenvlist; // list of texture environments for the textures
    std::vector< osg::ref_ptr<osg::Material> > matlist; // list of materials for current model
    georecord *cpalrec; // colour palette record
};

//=======
void internalVars::addInternalVars(const georecord &gr){
    const georecord::geoFieldList gfl=gr.getFields();
    for (georecord::geoFieldList::const_iterator itr=gfl.begin();
    itr!=gfl.end();
    ++itr)
    {// for each variable
        if ((*itr).getToken() >0) {
            geoValue *nm=new geoValue((*itr).getToken(),(*itr).getUInt());
            vars.push_back(*nm);
        }
    }
}

void userVars::addUserVar(const georecord &gr) {
    const georecord::geoFieldList gfl=gr.getFields();
    if (gr.getType() == DB_DSK_FLOAT_VAR) {
        unsigned int tok=0; // ? what for?
        const geoField *gfd= gr.getField(GEO_DB_FLOAT_VAR_FID);
        unsigned int fid=gfd ? gfd->getUInt():0;
        geoValue *nm=new geoValue(tok,fid);
        
        gfd= gr.getField(GEO_DB_FLOAT_VAR_NAME);
        const char *name=gfd->getChar();
        nm->setName(name);
        
        gfd= gr.getField(GEO_DB_FLOAT_VAR_VALUE);
        nm->setVal(gfd ? gfd->getFloat():0.0f);

        gfd= gr.getField(GEO_DB_FLOAT_VAR_DEFAULT);
        //nm->setdefault(gfd ? gfd->getFloat():0.0f);
        //float fdef=gfd ? gfd->getFloat():0.0f;
        
        gfd= gr.getField(GEO_DB_FLOAT_VAR_CONSTRAINED);
        if (gfd) {
            nm->setConstrained();
            gfd= gr.getField(GEO_DB_FLOAT_VAR_MIN);
            if (gfd) {
                nm->setMinRange(gfd->getFloat());
            }
            gfd= gr.getField(GEO_DB_FLOAT_VAR_MAX);
            if (gfd) {
                nm->setMaxRange(gfd->getFloat());
            }
        }
        gfd= gr.getField(GEO_DB_FLOAT_VAR_STEP);
        //float fstp=gfd ? gfd->getFloat():0;
        vars.push_back(*nm);
    }
}

void internalVars::update(const osg::FrameStamp *_frameStamp) {
    double stmptime=_frameStamp->getSimulationTime();
    int iord=0;
    for (std::vector<geoValue>::const_iterator itr=vars.begin(); //gfl.begin();
    itr!=vars.end(); // gfl.end();
    ++itr, iord++)
    {// for each field
        unsigned int typ=itr->getToken();
        switch (typ) {
        case GEO_DB_INTERNAL_VAR_FRAMECOUNT:
            vars[iord].setVal((float)_frameStamp->getFrameNumber());
            break;
        case GEO_DB_INTERNAL_VAR_CURRENT_TIME:
            {
                static double timestart=-1;
                if (timestart<0) {
                    time_t long_time;
                    struct tm *newtime;
                    
                    long_time=time( NULL );                // * Get time as long integer.
                    newtime = localtime( &long_time ); // * Convert to local time.
                    timestart=newtime->tm_hour*3600 +newtime->tm_min*60+ newtime->tm_sec;
                }
                double timeofday=timestart+_frameStamp->getSimulationTime();
                vars[iord].setVal(timeofday);
            }
            break;
        case GEO_DB_INTERNAL_VAR_ELAPSED_TIME:
            vars[iord].setVal(_frameStamp->getSimulationTime());
            break;
        case GEO_DB_INTERNAL_VAR_SINE:
            vars[iord].setVal(sin(stmptime));
            break;
        case GEO_DB_INTERNAL_VAR_COSINE:
            vars[iord].setVal(cos(stmptime));
            break;
        case GEO_DB_INTERNAL_VAR_TANGENT:
            vars[iord].setVal(tan(stmptime));
            break;
        case GEO_DB_INTERNAL_VAR_MOUSE_X: // this is all windowing system dependent
            //    vars[iord]=_frameStamp->getSimulationTime();
            break;
        case GEO_DB_INTERNAL_VAR_MOUSE_Y:
            //    vars[iord]=_frameStamp->getSimulationTime();
            break;
        case GEO_DB_INTERNAL_VAR_LEFT_MOUSE:
            //    vars[iord]=_frameStamp->getSimulationTime();
            break;
        case GEO_DB_INTERNAL_VAR_MIDDLE_MOUSE:
            //    vars[iord]=_frameStamp->getSimulationTime();
            break;
        case GEO_DB_INTERNAL_VAR_RIGHT_MOUSE:
            //    vars[iord]=_frameStamp->getSimulationTime();
            break;
        case GEO_DB_INTERNAL_VAR_TEMP_FLOAT:
            //    vars[iord]=_frameStamp->getSimulationTime();
            break;
        case GEO_DB_INTERNAL_VAR_TEMP_INT:
            //    vars[iord]=_frameStamp->getSimulationTime();
            break;
        case GEO_DB_INTERNAL_VAR_TEMP_BOOL:
            //    vars[iord]=_frameStamp->getSimulationTime();
            break;
        case GEO_DB_INTERNAL_VAR_TEMP_STRING:
            //    vars[iord]=_frameStamp->getSimulationTime();
            break;
        }
    }
    //    std::cout<<"update callback - post traverse"<< (float)_frameStamp->getSimulationTime() <<std::endl;
}

void geoField::parseExt(std::ifstream &fin) const { // Feb 2003 parse onme extension fields
    static int nread=0; // debug only
    // unused
    //geoExtensionDefRec *geoExt=(geoExtensionDefRec *)storage;
    for (uint i=0; i<numItems; i++) {
        geoExtensionDefRec rec;
            fin.read((char *)&rec,sizeof(rec));
            geoField ginner; // inside reading
            ginner.readfile(fin,0);
    }
    nread++;
}
void geoField::readfile(std::ifstream &fin, const uint id) { // is part of a record id 
    unsigned char tokid, type;
    unsigned short nits;
    if (!fin.eof()) {
        fin.read((char *)&tokid,1);fin.read((char *)&type,1);
        fin.read((char *)&nits,sizeof(unsigned short));
    //    osg::notify(osg::WARN) << "geoField " << (int)tokid << " type " << (int)type << " nit " << (int)nits << std::endl;
        if (type == DB_EXTENDED_FIELD_STRUCT) { // change for true extended type
            fin.read((char *)&tokenId,sizeof(tokenId));fin.read((char *)&TypeId,sizeof(TypeId));
            fin.read((char *)&numItems,sizeof(unsigned int));
        } else {
            tokenId=tokid; TypeId=type;
            numItems=nits;
        }
        if (id== 0 && tokenId == GEO_DB_NODE_EXTENDED && numItems==1) { // Feb 2003 parse extension template records
            if (TypeId == DB_SHORT ||
                TypeId == DB_USHORT) {
                short upad;
                fin.read((char *)&upad,SIZEOF_SHORT); // skip the padding on extension template
                upad=1;
            } else if (TypeId == DB_CHAR ||
                TypeId == DB_UCHAR) {
                char cpad[4];
                fin.read(cpad,SIZEOF_CHAR); // skip the padding
            } else {
            }
        }
        if (id== DB_DSK_HEADER && tokenId == GEO_DB_HDR_EXT_TEMPLATE) { // Feb 2003 parse extension records
        //    osg::notify(osg::WARN) << "header extension template " << (int)getType() << std::endl;
            parseExt(fin); // multiple structs occur here
        } else {
            if (numItems>0) {
                storageRead(fin); // allocate & fill the storage
                if (tokenId == GEO_DB_NODE_EXT) { // added Nov 2003 to parse extension nodes
                    if (TypeId == DB_SHORT ||TypeId == DB_USHORT) fin.ignore(2); // skip padding
                //    if (TypeId == DB_CHAR ||TypeId == DB_UCHAR) fin.ignore(3); // skip padding
                }
                if (tokenId == GEO_DB_NODE_EXTENDED) {
                    if (id==DB_DSK_POLYGON || id==DB_DSK_RENDERGROUP || id==DB_DSK_GROUP
                         || id==DB_DSK_LOD || id==DB_DSK_MESH || id==DB_DSK_CUBE
                         || id==DB_DSK_SPHERE || id==DB_DSK_CONE || id==DB_DSK_CYLINDER
                         || id==DB_DSK_TEXTURE || id==DB_DSK_MATERIAL || id==DB_DSK_VIEW) {
                        if (TypeId == DB_SHORT ||TypeId == DB_USHORT) fin.ignore(2); // skip padding
                    }
                }
            }
        }
    }
}


class ReaderWriterGEO : public osgDB::ReaderWriter
{
    public:
    
        ReaderWriterGEO()
        {
            supportsExtension("gem","CarbonGraphics Geo model format");
            supportsExtension("geo","CarbonGraphics Geo model format");
        }
    
        virtual const char* className() const { return "GEO Reader/Writer"; }

        virtual bool acceptsExtension(const std::string& extension) const
        {
            return osgDB::equalCaseInsensitive(extension,"gem") || osgDB::equalCaseInsensitive(extension,"geo");
        }

        virtual ReadResult readObject(const std::string& fileName, const Options* opt) const { return readNode(fileName,opt); }

        virtual ReadResult readNode(const std::string& file, const Options* options) const
        {
            std::string ext = osgDB::getLowerCaseFileExtension(file);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::string fileName = osgDB::findDataFile( file, options );
            if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

            ReaderGEO reader;
            return reader.readNode(fileName,options);
        }

};


// now register with Registry to instantiate the above
// reader/writer.
REGISTER_OSGPLUGIN(geo, ReaderWriterGEO)
