// GEO format (carbon graphics Inc) loader for the OSG real time scene graph
// www.carbongraphics.com for more information about the Geo animation+ modeller
// 2002

#include <osg/Image>
#include <osg/Group>
#include <osg/LOD>
#include <osg/Billboard>
#include <osg/Sequence>
#include <osg/Switch>
#include <osg/Geode>
#include <osg/Geometry>
#include <osg/MatrixTransform>
#include <osg/Material>
#include <osg/Notify>
#include <osg/Texture2D>
#include <osg/TexEnv>
#include <osg/StateSet>


#include <osgDB/FileNameUtils>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/Input>
#include <osgDB/Output>

#include <stdio.h>

// specific to GEO

#include "geoFormat.h"
#include "geoTypes.h"
#include "geoUnits.h"
#include "osgGeoAnimation.h"
#include "osgGeoStructs.h"

using namespace osg;
using namespace osgDB;


class geoHeaderCB: public osg::NodeCallback {
public:
    geoHeaderCB() {}
    ~geoHeaderCB() {}
    virtual void operator()(osg::Node* node, osg::NodeVisitor* nv)
    { // update action vars
        geoHeader *gh=(geoHeader *)node;
        gh->update();
        traverse(node,nv);
        //    std::cout<<"app callback - post traverse"<< (float)_frameStamp.getReferenceTime() <<std::endl;
    }
private:
};

class geoArithBehaviour {
public:
    geoArithBehaviour() { constant=0; oper=UNKNOWN; in=out=NULL; varop=NULL;}
    ~geoArithBehaviour() { }
    enum optype{UNKNOWN, ADD, SUBTR, MULT, DIVIDE};
    void setConstant(const float v) { constant=v;}
    void setInVar(const double *indvar) {in=indvar;}
    void setOutVar(double *outdvar) {out=outdvar;}
    void setType(uint iop)
        {
        switch (iop) {
            case 1: oper=ADD; break;
            case 2: oper=SUBTR; break;
            case 3: oper=MULT; break;
            case 4: oper=DIVIDE; break;
        }
    }
    void setVariable(const double *varvar) { varop=varvar;}
    void doaction(osg::Node*) const
        { // do math operation
        if (in && out) {
            double mult=varop? *varop : constant;
            switch (oper) {
            case ADD: *out = *in+mult;break;
            case SUBTR: *out = *in-mult;break;
            case MULT: *out = *in*mult;break;
            case DIVIDE: *out = *in/mult;break;
                        case UNKNOWN: break; // do nothing.
            }
        }
    }
private:
    float constant;
    optype oper;
    const double *in;
    double *out;
    const double *varop; // if null use constant value in maths; else 
};

class geoMatBehaviour {
public:
    geoMatBehaviour() { var=NULL; type=0;axis.set(0,0,1); centre.set(0,0,0);}
    ~geoMatBehaviour() { 
        var=NULL;}
    void setType(const unsigned int t) { type=t; }
    void setVar(const double *v) { var=v;}
    void setCentre(const Vec3 v) { centre=v;}
    void setAxis(const Vec3 v) { axis=v;}
    void doaction(osg::Node *node) const {
        if (var) {
            MatrixTransform *mtr=dynamic_cast<MatrixTransform *> (node);
            switch (type) {
            case DB_DSK_TRANSLATE_ACTION:
                mtr->preMult( osg::Matrix::translate(axis*(*var)) );
                break;
            case DB_DSK_ROTATE_ACTION:
                mtr->preMult( osg::Matrix::translate(-centre)* 
                    osg::Matrix::rotate((*var),axis)* 
                    osg::Matrix::translate(centre));
                break;
            }
        }
    }
private:
    // for fast transform behaviours
    unsigned int type; // eg GEO_DB_ROTATE_ACTION_INPUT_VAR, translate etc
    const double *var; // variable controls this behaviour
    osg::Vec3 axis; // axis of rotation
    osg::Vec3 centre; // centre of rotation
};
class geoBehaviourCB: public osg::NodeCallback {
public:
    geoBehaviourCB() { }
    ~geoBehaviourCB() { }
    void addBehaviour(geoMatBehaviour *gb) {gblist.push_back(*gb);}
    void addBehaviour(geoArithBehaviour *gb) {galist.push_back(*gb);}
    virtual void operator() (osg::Node *node, osg::NodeVisitor* nv)
    { // update the transform
    //    std::cout<<"geoBehaviourCB callback - "<< var << " " << (*var) <<std::endl;
        MatrixTransform *mtr=dynamic_cast<MatrixTransform *> (node);
        mtr->setMatrix(Matrix::identity());
        {
            for (std::vector<geoArithBehaviour>::const_iterator itr=galist.begin();
            itr<galist.end();
            itr++) {
                itr->doaction(node);
            }
        }
        {
            for (std::vector<geoMatBehaviour>::const_iterator itr=gblist.begin();
            itr<gblist.end();
            itr++) {
                itr->doaction(node);
            }
        }
        traverse(node,nv);
    }
private:
    std::vector<geoMatBehaviour> gblist;
    std::vector<geoArithBehaviour> galist;
};

class pack_colour {
public:
    pack_colour() { cr=cg=cb=0; ca=1;}
    ~pack_colour() {}
    pack_colour(const unsigned char col[4]) { cr= col[0]; cg= col[1];cb= col[2];; ca= col[2];}
    void get(unsigned char col[4]) const { col[0]=cr; col[1]=cg; col[2]=cb; col[3]=ca; }
    friend inline std::ostream& operator << (std::ostream& output, const pack_colour& pc)
    {
        output << " cpalette: " <<(int)pc.cr << " " <<(int)pc.cg << " " <<(int)pc.cb << " " <<(int)pc.ca;
        return output;     // to enable cascading.. 
    }
private:
    unsigned char cr, cg, cb, ca;
};

class vertexInfo { // holds vertex information for an entire osg::geometry
public:
    vertexInfo(const std::vector<osg::Vec3> *coord_pool, const std::vector<osg::Vec3> *normal_pool) {
        norms=new osg::Vec3Array;
        coords=new osg::Vec3Array;
        txcoords=new osg::Vec2Array;
        coordindices=new osg::IntArray;
        normindices=new osg::IntArray;
        txindices=new osg::IntArray;
        colors=new osg::Vec4Array;
        cpool=coord_pool; npool=normal_pool;
    }
    inline osg::Vec4Array *getColors() const { return colors;}
    inline osg::Vec3Array *getNorms() const { return norms;}
    inline osg::Vec3Array *getCoords() const { return coords;}
    inline osg::Vec2Array *getTexCoords() const { return txcoords;}
    inline osg::IntArray *getCoordIndices() const { return coordindices;}
    inline osg::IntArray *getNormIndices() const { return normindices;}
    inline osg::IntArray *getTextureIndices() const { return txindices;}
    void addIndices(georecord *gr,    const     std::vector<pack_colour> color_palette, const unsigned char cdef[4])
    { // this must only be called with a vertex georecord.
        if (gr->getType()==DB_DSK_VERTEX) {
            const geoField *gfd=gr->getField(GEO_DB_VRTX_NORMAL);
            int nrmindex=gfd ? gfd->getUInt():0;
            normindices->push_back(nrmindex);
            norms->push_back((*npool)[nrmindex]);
            gfd=gr->getField(GEO_DB_VRTX_COORD);
            unsigned int idx=gfd ? gfd->getInt():0;
            coords->push_back((*cpool)[idx]); //osg::Vec3(cpool[3*idx],cpool[3*idx+1],cpool[3*idx+2]));
            coordindices->push_back(txcoords->size());
            txindices->push_back(txcoords->size());
            float *uvc=NULL;
            gfd=gr->getField(GEO_DB_VRTX_UV_SET_1);
            if (gfd) {
                uvc=(float *)gfd->getstore(0);
            }

            if (uvc) { // then there are tx coords
                osg::Vec2 uv(uvc[0], uvc[1]);
                txcoords->push_back(uv);
            } else {
                txcoords->push_back(osg::Vec2(0,0));
            }
            gfd=gr->getField(GEO_DB_VRTX_PACKED_COLOR);
            if (gfd) {
                unsigned char *cp=gfd->getUCh4Arr();
                float red=cp[0];
                float green=cp[1];
                float blue=cp[2];
                colors->push_back(Vec4(red,green,blue,1.0));
            } else { // lok for a colour index (exclusive!)
                gfd=gr->getField(GEO_DB_VRTX_COLOR_INDEX);
                if (gfd) {
                    int icp=gfd->getInt();
                    float red=1.0f; // convert to range {0-1}
                    float green=1.0f;
                    float blue=1.0f;
                    float alpha=1.0f;
                    uint maxcol=(icp)/128; // the maximum intensity, 0-127 in bank 0, 128-255 =b2
                    float frac = (float)(icp-maxcol*128)/128.0f;
                    unsigned char col[4];
                    if (maxcol < color_palette.size()) {
                        color_palette[maxcol].get(col);
                        red=col[0]*frac/255.0f; // convert to range {0-1}
                        green=col[1]*frac/255.0f;
                        blue=col[2]*frac/255.0f;
                        alpha=1.0; // col[3]*frac/255.0f;
                    } else {
                //        unsigned char *cp=gfd->getUCh4Arr();
                //        frac = (float)(cp[3])/255.0f;
                //        maxcol=cp[2];
                //        color_palette[maxcol].get(col);
                        red=cdef[0]/255.0f; // convert to range {0-1}
                        green=cdef[1]/255.0f;
                        blue=cdef[2]/255.0f;
                        alpha=1.0; // col[3]*frac/255.0f;
                    }
                    colors->push_back(Vec4(red,green,blue,alpha));
                }
            }
            
            
        }
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
    osg::IntArray *coordindices;
    osg::IntArray *normindices;
    osg::IntArray *txindices;
};

class ReaderWriterGEO : public ReaderWriter
{
    public:
        ReaderWriterGEO() {     }
        ~ReaderWriterGEO() {    
        }
        virtual const char* className() { return "GEO Reader/Writer"; }

        virtual bool acceptsExtension(const std::string& extension)
        {
            return equalCaseInsensitive(extension,"gem") || equalCaseInsensitive(extension,"geo");
        }

        virtual ReadResult readObject(const std::string& fileName, const Options* opt) { return readNode(fileName,opt); }

        virtual ReadResult readNode(const std::string& fileName, const Options*)
        {
            std::string ext = getFileExtension(fileName);
            if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

            std::ifstream fin(fileName.c_str(), std::ios::binary | std::ios::in );
            if (fin.is_open() )
            { // read the input file.
                typedef std::vector<osg::Node*> NodeList;
                NodeList nodeList;
                geoRecordList recs;
                osg::Material *mt=new osg::Material;
                matlist.push_back(mt);
                theHeader=NULL;

                // load all nodes in file, placing them in a linear list corresponding to the on disk file.
                while(!fin.eof())
                {
                    georecord gr;
                    gr.readfile(fin);
                //    osg::notify(osg::WARN) << gr << std::endl;
                    recs.push_back(gr); // add to a list of all records
                }
                fin.close();
                // now sort the reocrds so that any record followed by a PUSh has a child set = next record, etc
                std::vector<georecord *> sorted=sort(recs); // tree-list of sorted record pointers
                osgDB::Output fout("georex.txt"); //, std::ios_base::out );
                fout << "Debug file " << std::endl;
            //    output(fout,sorted); // print details of the sorted tree, with records indented.
                nodeList=makeosg(sorted, fout); // make a list of osg nodes
                fout.close();
                
                recs.erase(recs.begin(),recs.end());
                color_palette.erase(color_palette.begin(),color_palette.end());
                geotxlist.erase(geotxlist.begin(),geotxlist.end());
                geomatlist.erase(geomatlist.begin(),geomatlist.end()); 
                txlist.erase(txlist.begin(),txlist.end());
                txenvlist.erase(txenvlist.begin(),txenvlist.end());
                matlist.erase(matlist.begin(),matlist.end());/* */
                coord_pool.erase(coord_pool.begin(),coord_pool.end());
                normal_pool.erase(normal_pool.begin(),normal_pool.end());
                if  (nodeList.empty())
                {
                    return ReadResult("No data loaded from "+fileName);
                }
                else if (nodeList.size()==1)
                {
                    return nodeList.front();
                }
                else
                {
                    Group* group = osgNew Group;
                    group->setName("import group");
                    for(NodeList::iterator itr=nodeList.begin();
                        itr!=nodeList.end();
                        ++itr)
                    {
                        group->addChild(*itr);
                    }
                    return group;
                }
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
                    curparent= itr;
                    sorted.push_back(itr);
                    break;
                case DB_DSK_INTERNAL_VARS: // attach to parent
                case DB_DSK_LOCAL_VARS:
                case DB_DSK_EXTERNAL_VARS:
                    (curparent)->addBehaviour((itr));
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
                    (curparent)->addBehaviour((itr));
                        break;
                case DB_DSK_TEXTURE: // attach to parent
                    geotxlist.push_back(itr);
                    break;
                case DB_DSK_MATERIAL: // attach to parent
                    geomatlist.push_back(itr);
                    break;
                case DB_DSK_VIEW: // not needed for Real Time
                    break;
                case DB_DSK_COORD_POOL: // global - attach to readerwriterGEO class for whole model
                    gfd=itr->getField(GEO_DB_COORD_POOL_VALUES);
                    {
                        float *crds= (gfd) ? (gfd->getVec3Arr()):NULL;
                        int nm=gfd->getNum();
                        for (int i=0; i<nm; i++) {
                            coord_pool.push_back(Vec3(crds[i*3],crds[i*3+1],crds[i*3+2]));
                        }
                    }
                    break;
                case DB_DSK_NORMAL_POOL: // global - attach to readerwriterGEO
                    gfd=itr->getField(GEO_DB_NORMAL_POOL_VALUES);
                    {
                        float *nrms= (gfd) ? (gfd->getVec3Arr()):NULL;
                        int nm=gfd->getNum();
                        for (int i=0; i<nm; i++) {
                            normal_pool.push_back(Vec3(nrms[i*3],nrms[i*3+1],nrms[i*3+2]));
                        }
                    }
                    break;
                case DB_DSK_COLOR_PALETTE: // global - attach to readerwriterGEO
                    gfd=itr->getField(GEO_DB_COLOR_PALETTE_HIGHEST_INTENSITIES);
                    if (gfd) {
                        unsigned char *cpal=gfd->getstore(0);
                        for (int i=1; i<gfd->getNum(); i++) {
                            color_palette.push_back(cpal);
                            cpal+=4;
                        }
                    //color_palette= (gfd) ? (gfd->getUCh4Arr()):NULL;
                    }
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
                case DB_DSK_PERIODIC2_ACTION:
                case DB_DSK_TRIG_ACTION:
                case DB_DSK_INVERSE_ACTION:
                case DB_DSK_TRUNCATE_ACTION:
                case DB_DSK_ABS_ACTION:
                case DB_DSK_IF_THEN_ELSE_ACTION:
                case DB_DSK_DCS_ACTION:
                    (curparent->getLastChild())->addBehaviour((itr));
                    break;
                default:
                    if (curparent) {
                        (*itr).setparent(curparent);
                        curparent->addchild(itr);
                    } 
                    break;
                }
            }
            return sorted;
        }
        void outputPrim(const georecord *grec, osgDB::Output &fout) {
            const std::vector<georecord *> gr=grec->getchildren();
            if (gr.size()>0) {
                for (std::vector<georecord *>::const_iterator itr=gr.begin();
                    itr!=gr.end();
                    ++itr) {
                    fout << *(*itr) << std::endl;
                }
            }
        }
        int getprim(const georecord *grec,vertexInfo &vinf)
        { // fills vinf with txcoords = texture coordinates, txindex=txindex etc
            int nv=0;
            const std::vector<georecord *> gr=grec->getchildren();
            const geoField *gfd=grec->getField(GEO_DB_POLY_PACKED_COLOR); // the colour
            unsigned char defcol[4]; // a default colour for vertices
            defcol[0]=defcol[1]=defcol[2]=defcol[3]=255;
            if (gfd) {
                unsigned char *cls=gfd->getUCh4Arr();
                defcol[0]=cls[0];
                defcol[1]=cls[1];
                defcol[2]=cls[2];
                defcol[3]=255;
            } else {
                gfd=grec->getField(GEO_DB_POLY_COLOR_INDEX); // the colour
                if (gfd) {
                    int icp= gfd ? gfd->getInt() : 0;
                    int maxcol=icp/128; // the maximum intensity index
                    color_palette[maxcol].get(defcol);
                } else {
                    defcol[0]=defcol[1]=defcol[2]=defcol[3]=255;
                }
            }

            if (gr.size()>0) {
                for (std::vector<georecord *>::const_iterator itr=gr.begin();
                    itr!=gr.end();
                    ++itr) {
                    vinf.addIndices((*itr), color_palette, defcol);
                    nv++;
                }
            }
            return nv;
        }
        void outputGeode(georecord grec, osgDB::Output &fout) {
            const std::vector<georecord *> gr=grec.getchildren();
            if (gr.size()>0) {
                for (std::vector<georecord *>::const_iterator itr=gr.begin();
                itr!=gr.end();
                ++itr) {
                    fout << *(*itr) << std::endl;
                    if ((*itr)->getType()==DB_DSK_POLYGON) {
                        outputPrim((*itr), fout);
                    }
                }
            }
        }
        std::vector<osg::Geometry *>makeGeometry(georecord grec, const unsigned int imat)
        {
            // makegeometry returns a set of Geometrys to attach to current parent (Geode)
            const std::vector<georecord *> gr=grec.getchildren();
            std::vector<osg::Geometry *> geom;
            if (gr.size()>0) {
                Vec4Array *polycols= new osg::Vec4Array; // polygon colours
                std::vector<int> ia; // list of texture indices found i this geode; sort into new 
                vertexInfo vinf(&coord_pool, &normal_pool); // holds all types of coords, indices etc
                int nstart=0; // start of list
                for (std::vector<georecord *>::const_iterator itr=gr.begin();
                itr!=gr.end();
                ++itr) {
                    if ((*itr)->getType()==DB_DSK_POLYGON) {
                        const geoField *gfd=(*itr)->getField(GEO_DB_POLY_TEX);
                        int txidx= gfd ? gfd->getInt() : -1;
                        int igidx=0, igeom=-1;

                        gfd=(*itr)->getField(GEO_DB_POLY_DSTYLE); // solid, wire...
                        int dstyle= gfd ? gfd->getInt() : (int)GEO_DB_SOLID;
                        gfd=(*itr)->getField(GEO_DB_POLY_SHADEMODEL); // shaded gouraud, flat...
                        int shademodel=gfd ? (uint)gfd->getInt() : GEO_POLY_SHADEMODEL_LIT_GOURAUD;
    //shade models GEO_POLY_SHADEMODEL_FLAT GEO_POLY_SHADEMODEL_GOURAUD
    //    GEO_POLY_SHADEMODEL_LIT GEO_POLY_SHADEMODEL_LIT_GOURAUD
                        gfd=(*itr)->getField(GEO_DB_POLY_USE_MATERIAL_DIFFUSE); // true: use material...
                        bool usemat= gfd ? gfd->getBool() : false;
                        if (!usemat && 
                            (shademodel== GEO_POLY_SHADEMODEL_LIT ||shademodel== GEO_POLY_SHADEMODEL_LIT_GOURAUD) ) { // get the per vertex colours OR per face colours.
                            gfd=(*itr)->getField(GEO_DB_POLY_PACKED_COLOR); // the colour
                            if (gfd) {
                                unsigned char *cls=gfd->getUCh4Arr();
                                //polycols->push_back(osg::Vec4((float)cls[0],(float)cls[2],(float)cls[3],(float)cls[4]));
                                float red=cls[0]/255.0f;
                                float green=cls[1]/255.0f;
                                float blue=cls[2]/255.0f;
                                float alpha=1.0f; // cls[3]*frac/255.0f;
                                polycols->push_back(osg::Vec4(red,green,blue,alpha));
                            } else { // get colour from palette
                                gfd=(*itr)->getField(GEO_DB_POLY_COLOR_INDEX); // use color pool...
                                int icp= gfd ? gfd->getInt() : 0;
                                int maxcol=icp/128; // the maximum intensity index
                                unsigned char col[4];
                                color_palette[maxcol].get(col);
                                float frac = (float)(icp-maxcol*128)/128.0f; // how high up the intensity we are
                                float red=col[0]*frac/255.0f;
                                float green=col[1]*frac/255.0f;
                                float blue=col[2]*frac/255.0f;
                                float alpha=col[3]*frac/255.0f;
                                polycols->push_back(osg::Vec4(red,green,blue,alpha));
                            }
                        }
                        gfd=(*itr)->getField(GEO_DB_POLY_NORMAL); // polygon normal for whole poly
                        for (IntArray::const_iterator itrint=ia.begin();
                        itrint!=ia.end();
                        ++itrint) { // find a geometry that shares this texture.
                            if (txidx==(*itrint)) igeom=igidx;
                            igidx++;
                        }
                        int nv=getprim((*itr),vinf);
                        if (igeom<0) {
                            osg::Geometry *nug;
                            nug=new osg::Geometry;
                            nug->setNormalBinding(osg::Geometry::BIND_PER_VERTEX);
                            nug->setVertexArray(vinf.getCoords());
                            nug->setNormalArray(vinf.getNorms());
                            StateSet *dstate=new StateSet;
                            if (txidx>=0 && (unsigned int)txidx<txlist.size()) {
                                dstate->setTextureAttribute(0, txenvlist[txidx] );
                                dstate->setTextureAttributeAndModes(0,txlist[txidx],osg::StateAttribute::ON);
                            }
                            if (imat>0 && imat<matlist.size()) {
                                matlist[imat]->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);
                                dstate->setAttribute(matlist[imat]);
                            } else {
                                matlist[0]->setColorMode(osg::Material::AMBIENT_AND_DIFFUSE);
                                dstate->setAttribute(matlist[0]);
                            }
                            dstate->setMode(GL_COLOR_MATERIAL, osg::StateAttribute::ON);
                            if (shademodel==GEO_POLY_SHADEMODEL_LIT ||
                                shademodel==GEO_POLY_SHADEMODEL_LIT_GOURAUD) dstate->setMode( GL_LIGHTING, osg::StateAttribute::ON );
                            else 
                                dstate->setMode( GL_LIGHTING, osg::StateAttribute::OFF );
                            { // reclaim the colours
                                gfd=(*itr)->getField(GEO_DB_POLY_USE_MATERIAL_DIFFUSE); // true: use material...
                                bool usemat= gfd ? gfd->getBool() : false;
                                if (!usemat) { // get the per vertex colours OR per face colours.
                                    gfd=(*itr)->getField(GEO_DB_POLY_USE_VERTEX_COLORS); // true: use material...
                                    bool usevert=gfd ? gfd->getBool() : true;
                                    if (usevert) {
                                        Vec4Array *cls=vinf.getColors();
                                        if (cls) {
                                            nug->setColorArray(cls);
                                            nug->setColorBinding(osg::Geometry::BIND_PER_VERTEX);
                                        }
                                    } else {
                                        if (polycols->size() > 0) {
                                            nug->setColorArray(polycols);
                                            nug->setColorBinding(osg::Geometry::BIND_PER_VERTEX); //PRIMITIVE);
                                        }
                                    }
                                }
                            }
                            nug->setStateSet( dstate );
                            geom.push_back(nug);
                            igeom=ia.size();
                            ia.push_back(txidx); // look up table for which texture corresponds to which geom
                        }
                        
                        if (dstyle==0x00000004 || dstyle & GEO_DB_SOLID || dstyle & GEO_DB_OUTLINED) geom[igeom]->addPrimitive(new osg::DrawArrays(osg::Primitive::POLYGON,nstart,nv));
                        if (dstyle & GEO_DB_WIRE || dstyle & GEO_DB_OUTLINED) geom[igeom]->addPrimitive(new osg::DrawArrays(osg::Primitive::LINE_STRIP,nstart,nv));
                        nstart+=nv;
                    }
                }
                osg::Vec2Array *txa=vinf.getTexCoords();
                if (txa->size() > 0 ) {
                    for (std::vector<osg::Geometry *>::iterator itr=geom.begin();
                        itr!=geom.end();
                        ++itr) {
                        (*itr)->setTexCoordArray(0, txa);
                    }
                }
                // osg::notify(osg::WARN) << vinf;
            }
            return geom;
        }
        Geode *makeGeode(const georecord *gr, osgDB::Output &fout)
        {
            Geode *nug=new Geode;
            const geoField *gfd=gr->getField(GEO_DB_RENDERGROUP_MAT);
            const unsigned int imat=gfd ? gfd->getInt():0;
            gfd=gr->getField(GEO_DB_RENDERGROUP_NAME);
            if (gfd) {
                nug->setName(gfd->getChar());
            }
            std::vector<osg::Geometry *>geom=makeGeometry((*gr),imat);
            outputGeode((*gr), fout);
            for (std::vector<osg::Geometry *>::iterator itg=geom.begin();
            itg!=geom.end();
            ++itg)
            {
                nug->addDrawable((*itg));
            }
            return nug;
        }
        osg::Group *makePage(const georecord *gr)
        {
            osg::Group *gp=new Group;
            const geoField *gfd=gr->getField(GEO_DB_PAGE_NAME);
            if (gfd) {
                gp->setName(gfd->getChar());
            }
            return gp;
        }
        osg::Group *makeGroup(const georecord *gr) { // group or Static transform
            osg::Group *gp=NULL;
            const geoField *gfd=gr->getField(GEO_DB_GRP_TRANSFORM);
            if (gfd) {
                MatrixTransform *tr=new MatrixTransform;
                osg::Matrix mx;
                float * m44=gfd->getMat44Arr();
                mx.set(m44); // hope uses same convention as OSG else will need to use set(m44[0],m44[1]...)
                tr->setMatrix(mx);
                gp=tr;
            } else {
                gp=new osg::Group;
            }
            gfd=gr->getField(GEO_DB_GRP_NAME);
            if (gfd) {
                gp->setName(gfd->getChar());
            }
            return gp;
        }
        osg::Group *makeSwitch(const georecord *gr) {
            osg::Switch *sw=new Switch;
            const geoField *gfd=gr->getField(GEO_DB_SWITCH_CURRENT_MASK);
            sw->setValue(osg::Switch::ALL_CHILDREN_OFF);
            if (gfd) {
                int imask;
                imask=gfd->getInt();
                sw->setValue(imask);
                osg::notify(osg::WARN) << gr << " imask " << imask << std::endl;
            } else {
                sw->setValue(0);
                osg::notify(osg::WARN) << gr << " No mask " << std::endl;
            }
            gfd=gr->getField(GEO_DB_SWITCH_NAME);
            if (gfd) {
                sw->setName(gfd->getChar());
            }
            return sw;
        }
        osg::Sequence *makeSequence(const georecord *gr)
        {
            Sequence *sq=new Sequence;
            const geoField *gfd=gr->getField(GEO_DB_SEQUENCE_NAME);
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
            gfd=gr->getField(GEO_DB_LOD_NAME);
            if (gfd) {
                gp->setName(gfd->getChar());
            }
            return gp;
        }
        geoHeader *makeHeader(const georecord *gr) {
            // the header contains variables as well as a transform for the XYZup cases
            theHeader=new geoHeader();
            const geoField *gfd=gr->getField(GEO_DB_HDR_UP_AXIS);
            osg::Quat q;
            unsigned iup=gfd ? gfd->getUInt() : GEO_DB_UP_AXIS_Y;

            switch (iup) {
            case GEO_DB_UP_AXIS_X:
                    q.set(1,0,1,0);
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
                    q.set(1,0,0,0);
                    q/=q.length();
                    theHeader->setAttitude(q);
                break;
            }
            for (std::vector<georecord *>::const_iterator itr=geotxlist.begin(); itr<geotxlist.end(); itr++) {
                makeTexture(*itr);
            }
            std::vector< georecord *>bhv=gr->getBehaviour();
            if (!bhv.empty()) { // then add internal, user, extern variables
                for (std::vector< georecord *>::const_iterator rcitr=bhv.begin();
                rcitr!=bhv.end();
                ++rcitr)
                {
                    if ((*rcitr)->getType()==DB_DSK_INTERNAL_VARS) {
                        theHeader->addInternalVars(**rcitr);
                //        theHeader->setAppCallback(theHeader->getInternalVars());                        
                    }
                    if ((*rcitr)->getType()==DB_DSK_FLOAT_VAR) {
                        if (theHeader) theHeader->addUserVar((**rcitr));
                    }
                }
                theHeader->setAppCallback(new geoHeaderCB);
            }
            return theHeader;
        }
        void makeTexture(const georecord *gr) {
            // scans the fields of this record and puts a new texture & environment into 'pool' stor
            const geoField *gfd=gr->getField(GEO_DB_TEX_FILE_NAME);
            const char *name = gfd->getChar();
            if (name) {
                Texture2D *tx=new Texture2D;
                Image *ctx=osgDB::readImageFile(name);
                if (ctx) {
                    ctx->setFileName(name);
                    tx->setImage(ctx);
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
                txlist.push_back(tx);
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
            std::vector< georecord *>bhv=gr->getBehaviour();
            if (!bhv.empty()) { // then add a DCS/matrix_transform
                mtr=new MatrixTransform;
                geoBehaviourCB *gcb=new geoBehaviourCB;
                mtr->setAppCallback(gcb);

                for (std::vector< georecord *>::const_iterator rcitr=bhv.begin();
                rcitr!=bhv.end();
                ++rcitr)
                {
                    if ((*rcitr)->getType()==DB_DSK_BEHAVIOR) {
                        const geoField *gfd=(*rcitr)->getField(GEO_DB_BEHAVIOR_NAME);
                        if (gfd) {
                            mtr->setName(gfd->getChar());
                        }
                    }
                    if ((*rcitr)->getType()==DB_DSK_ROTATE_ACTION) {
                        const geoField *gfd=(*rcitr)->getField(GEO_DB_ROTATE_ACTION_INPUT_VAR);
                        if (gfd) {
                            unsigned fid= gfd->getUInt(); // field identifier
                            const double *dvar=theHeader->getVar(fid); // returns address of var with fid
                            if (dvar) {
                                geoMatBehaviour *cb=new geoMatBehaviour;
                                cb->setVar(dvar);
                                cb->setType(DB_DSK_ROTATE_ACTION);
                                gfd=(*rcitr)->getField(GEO_DB_ROTATE_ACTION_VECTOR);
                                if (gfd) {
                                    float *ax= gfd->getVec3Arr(); // field identifier
                                    cb->setAxis(osg::Vec3(ax[0],ax[1],ax[2]));
                                }
                                gfd=(*rcitr)->getField(GEO_DB_ROTATE_ACTION_ORIGIN);
                                if (gfd) {
                                    float *ct= gfd->getVec3Arr(); // field identifier
                                    cb->setCentre(osg::Vec3(ct[0],ct[1],ct[2]));
                                }
                                gcb->addBehaviour(cb);
                            }
                        }
                    }
                    if ((*rcitr)->getType()==DB_DSK_TRANSLATE_ACTION) {
                        const geoField *gfd=(*rcitr)->getField(GEO_DB_TRANSLATE_ACTION_INPUT_VAR);
                        if (gfd) {
                            unsigned fid= gfd->getUInt(); // field identifier
                            const double *dvar=theHeader->getVar(fid); // returns address of var with fid
                            if (dvar) {
                                geoMatBehaviour *cb=new geoMatBehaviour;
                                cb->setVar(dvar);
                                cb->setType(DB_DSK_TRANSLATE_ACTION);
                                gfd=(*rcitr)->getField(GEO_DB_TRANSLATE_ACTION_VECTOR);
                                if (gfd) {
                                    float *ax= gfd->getVec3Arr(); // field identifier
                                    cb->setAxis(osg::Vec3(ax[0],ax[1],ax[2]));
                                }
                                gfd=(*rcitr)->getField(GEO_DB_TRANSLATE_ACTION_ORIGIN);
                                if (gfd) {
                                    float *ct= gfd->getVec3Arr(); // field identifier
                                    cb->setCentre(osg::Vec3(ct[0],ct[1],ct[2]));
                                }
                                gcb->addBehaviour(cb);
                            }
                        }
                    }
                    if ((*rcitr)->getType()==DB_DSK_ARITHMETIC_ACTION) {
                        const geoField *gfd=(*rcitr)->getField(GEO_DB_ARITHMETIC_ACTION_INPUT_VAR);
                        if (gfd) {
                            unsigned fid= gfd->getUInt(); // field identifier
                            const double *indvar=theHeader->getVar(fid); // returns address of input var with fid
                            if (indvar) {
                                gfd=(*rcitr)->getField(GEO_DB_ARITHMETIC_ACTION_OUTPUT_VAR);
                                if (gfd) {
                                    fid= gfd->getUInt(); // field identifier
                                    double *outdvar=theHeader->getVar(fid); // returns address of output var with fid
                                    geoArithBehaviour *cb=new geoArithBehaviour;
                                    cb->setInVar(indvar);
                                    cb->setOutVar(outdvar);
                                    gfd=(*rcitr)->getField(GEO_DB_ARITHMETIC_ACTION_OP_TYPE);
                                    uint iop=gfd?gfd->getUInt():1;
                                    cb->setType(iop); // default add?
                                    gfd=(*rcitr)->getField(GEO_DB_ARITHMETIC_ACTION_OPERAND_VALUE);
                                    if (gfd) {
                                        float ax= gfd->getFloat(); // field identifier
                                        cb->setConstant(ax);
                                    }
                                    gfd=(*rcitr)->getField(GEO_DB_ARITHMETIC_ACTION_OPERAND_VAR);
                                    if (gfd) {
                                        unsigned fid= gfd->getUInt(); // field identifier
                                        const double *varvar=theHeader->getVar(fid);
                                        cb->setVariable(varvar);
                                    }
                                    gcb->addBehaviour(cb);
                                }
                            }
                        }
                    }
                }
            }
            return mtr;
        }
        std::vector<Node *> makeosg(const std::vector<georecord *> gr, osgDB::Output &fout) {
            // recursive traversal of records and extract osg::Nodes equivalent
            Group *geodeholder=NULL;
            std::vector<Node *> nodelist;
            fout.moveIn(); // increase indent
            if (gr.size()>0) {
                for (std::vector<georecord *>::const_iterator itr=gr.begin();
                itr!=gr.end();
                ++itr) {
                    const georecord *gr=*itr;
                    MatrixTransform *mtr=makeBehave(gr);
                    fout.indent() << (*gr) << std::endl;
                    if (gr->getType()== DB_DSK_GEODE) { // geodes can require >1 geometry for example if polygons have different texture indices.
                        Geode *geode=makeGeode(gr, fout);
                        if (mtr) {
                            mtr->addChild(geode);
                            nodelist.push_back(mtr);
                            mtr=NULL;
                        } else {
                            if (!geodeholder) {
                                geodeholder=new osg::Group;
                            }
                            geodeholder->addChild(geode);
                        }
                    } else {
                        Group *holder=NULL;
                        const geoField *gfd;
                        switch (gr->getType()) {
                        case DB_DSK_HEADER:
                            holder=makeHeader(gr);
                            {
                                for (std::vector<pack_colour>::const_iterator itr=color_palette.begin();
                                itr!=color_palette.end();
                                ++itr) {
                                    fout << (*itr) << std::endl;
                                }
                            }
                            
                            break;
                        case DB_DSK_MATERIAL: {
                            osg::Material *mt=new osg::Material;
                            gr->setMaterial(mt);
                            matlist.push_back(mt);
                                              }
                            break;
                        case DB_DSK_TEXTURE:
                            makeTexture(gr);
                            break;
                        case DB_DSK_GROUP:
                            holder=makeGroup(gr);
                            if (mtr) {
                                mtr->addChild(holder);
                                holder=mtr;
                            }
                            break;
                        case DB_DSK_LOD: 
                            holder=makeLOD(gr);
                            break;
                        case DB_DSK_SEQUENCE:
                            holder=makeSequence(gr);
                            break;
                        case DB_DSK_SWITCH:
                            holder=makeSwitch(gr);
                            break;
                        case DB_DSK_CUBE:
                            holder=new Group;
                            gfd=gr->getField(GEO_DB_GRP_NAME);
                            if (gfd) {
                                holder->setName(gfd->getChar());
                            }
                            break;
                        case DB_DSK_SPHERE:
                            holder=new Group;
                            gfd=gr->getField(GEO_DB_GRP_NAME);
                            if (gfd) {
                                holder->setName(gfd->getChar());
                            }
                            break;
                        case DB_DSK_CONE:
                            holder=new Group;
                            gfd=gr->getField(GEO_DB_GRP_NAME);
                            if (gfd) {
                                holder->setName(gfd->getChar());
                            }
                            break;
                        case DB_DSK_CYLINDER:
                            holder=new Group;
                            gfd=gr->getField(GEO_DB_GRP_NAME);
                            if (gfd) {
                                holder->setName(gfd->getChar());
                            }
                            break;
                        case DB_DSK_INSTANCE:
                            holder=new Group;
                            /*gfd=gr->getField(GEO_DB_GRP_NAME);
                            if (gfd) {
                                holder->setName(gfd->getChar());
                            } */
                            break;
                        case DB_DSK_PAGE:
                            holder=makePage(gr);
                            break;
                        case DB_DSK_FLOAT_VAR:
                        case DB_DSK_INT_VAR:
                        case DB_DSK_LONG_VAR:
                        case DB_DSK_DOUBLE_VAR:
                        case DB_DSK_BOOL_VAR:
                        case DB_DSK_FLOAT2_VAR:
                        case DB_DSK_FLOAT3_VAR:
                        case DB_DSK_FLOAT4_VAR:
                            fout.indent() << "AVars " << gr->getType() << std::endl;
                            fout.indent() << (*gr) << std::endl;
                            break;
                        case DB_DSK_INTERNAL_VARS:                   
                        case DB_DSK_LOCAL_VARS:                   
                        case DB_DSK_EXTERNAL_VARS:
                            fout.indent() << "==Vars " << gr->getType() << std::endl;
                            fout.indent() << (*gr) << std::endl;
                            break;
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
                            holder=new Group;
                            fout.indent() << "==Poorly handled option " << gr->getType() << std::endl;
                            fout.indent() << (*gr) << std::endl;
                            break;
                        default: {
                            osg::Group *gp=new Group;
                            holder=gp;
                            }
                            break;
                        }
                        // fout.indent() << (*gr) << std::endl;
                        if (holder) nodelist.push_back(holder);

                        std::vector<Node *> child=makeosg((*itr)->getchildren(),fout);
                        for (std::vector<Node *>::iterator itr=child.begin();
                            itr!=child.end();
                            ++itr) {
                                holder->addChild(*itr);
                        }
                    }
                }
            }
            if (geodeholder) nodelist.push_back(geodeholder);
            fout.moveOut(); // decrease indent
            return nodelist;
        }
        void output(osgDB::Output &fout,std::vector<georecord *> gr)
        { // debugging - print the tree of records
        //    static int depth=0;
        //    depth++;
            fout.moveIn();
            if (gr.size()>0) {
                for (std::vector<georecord *>::iterator itr=gr.begin();
                    itr!=gr.end();
                    ++itr) {
                    // osg::notify(osg::WARN)
                    fout.indent() << "Node type " << (*itr)->getType() << " ";
                    fout.indent() << (**itr) << std::endl;
                    fout.indent() << std::endl;
                    output(fout,(*itr)->getchildren());
                }
            }
            fout.moveOut();
        //    depth--;
        }
private:
//    std::fstream fout; // debug output
    std::vector<osg::Vec3> coord_pool; // current vertex ooords
    std::vector<osg::Vec3> normal_pool; // current pool of normal vectors
    //static unsigned char *
    std::vector<pack_colour> color_palette;
    geoHeader *theHeader; // has animation vars etc
    std::vector<georecord *> geotxlist; // list of geo::textures for this model
    std::vector<georecord *> geomatlist; // list of geo::materials for this model
    std::vector<osg::Texture2D *> txlist; // list of osg::textures for this model
    std::vector<osg::TexEnv *> txenvlist; // list of texture environments for the textures
    std::vector<osg::Material *> matlist; // list of materials for current model
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
        //float fval=gfd ? gfd->getFloat():0.0f;
        gfd= gr.getField(GEO_DB_FLOAT_VAR_DEFAULT);
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
        //float fstp=gfd ? gfd->getFloat():0.0f;
        vars.push_back(*nm);
    }
}
void internalVars::update(osg::Timer  &_timer,osg::FrameStamp &_frameStamp) {
    double time=_frameStamp.getReferenceTime();
    osg::Timer_t _frameTick = _timer.tick();;
    int iord=0;
    for (std::vector<geoValue>::const_iterator itr=vars.begin(); //gfl.begin();
    itr!=vars.end(); // gfl.end();
    ++itr, iord++)
    {// for each field
        unsigned int typ=itr->getToken();
        switch (typ) {
        case GEO_DB_INTERNAL_VAR_FRAMECOUNT:
            vars[iord].setVal(_frameTick);
            break;
        case GEO_DB_INTERNAL_VAR_CURRENT_TIME:
            vars[iord].setVal(_timer.delta_s(0,_frameTick));
            break;
        case GEO_DB_INTERNAL_VAR_ELAPSED_TIME:
            vars[iord].setVal(_frameStamp.getReferenceTime());
            break;
        case GEO_DB_INTERNAL_VAR_SINE:
            vars[iord].setVal(sin(time));
            break;
        case GEO_DB_INTERNAL_VAR_COSINE:
            vars[iord].setVal(cos(time));
            break;
        case GEO_DB_INTERNAL_VAR_TANGENT:
            vars[iord].setVal(tan(time));
            break;
        case GEO_DB_INTERNAL_VAR_MOUSE_X: // this is all windowing system dependent
            //    vars[iord]=_frameStamp.getReferenceTime();
            break;
        case GEO_DB_INTERNAL_VAR_MOUSE_Y:
            //    vars[iord]=_frameStamp.getReferenceTime();
            break;
        case GEO_DB_INTERNAL_VAR_LEFT_MOUSE:
            //    vars[iord]=_frameStamp.getReferenceTime();
            break;
        case GEO_DB_INTERNAL_VAR_MIDDLE_MOUSE:
            //    vars[iord]=_frameStamp.getReferenceTime();
            break;
        case GEO_DB_INTERNAL_VAR_RIGHT_MOUSE:
            //    vars[iord]=_frameStamp.getReferenceTime();
            break;
        case GEO_DB_INTERNAL_VAR_TEMP_FLOAT:
            //    vars[iord]=_frameStamp.getReferenceTime();
            break;
        case GEO_DB_INTERNAL_VAR_TEMP_INT:
            //    vars[iord]=_frameStamp.getReferenceTime();
            break;
        case GEO_DB_INTERNAL_VAR_TEMP_BOOL:
            //    vars[iord]=_frameStamp.getReferenceTime();
            break;
        case GEO_DB_INTERNAL_VAR_TEMP_STRING:
            //    vars[iord]=_frameStamp.getReferenceTime();
            break;
        }
    }
    //    std::cout<<"app callback - post traverse"<< (float)_frameStamp.getReferenceTime() <<std::endl;
}

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterGEO> gReaderWriter_GEO_Proxy;

