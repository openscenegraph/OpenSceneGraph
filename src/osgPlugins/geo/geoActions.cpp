// GEO format (carbon graphics Inc) loader for the OSG real time scene graph
// www.carbongraphics.com for more information about the Geo animation+ modeller
// 2002
// actions & behaviours for Geo loader in OSG

#include <string>

#include <stdio.h>
#include <math.h>
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
#include <osgDB/Input>
#include <osgDB/Output>

// specific to GEO

#include "geoFormat.h"
#include "geoTypes.h"
#include "geoUnits.h"
#include "osgGeoAnimation.h"
#include "osgGeoStructs.h"
#include "osgGeoNodes.h"
#include "osgGeoAction.h"
#include <osgText/Text> // needed for text nodes
#include <osg/Timer>


void geoArithBehaviour::setType(unsigned int iop) {
    switch (iop) {
    case 1: op=addv; break; /* op=addv; addv is a function so the operation can be accessed without a switch(type)... */
    case 2: op=subv; break;
    case 3: op=mulv; break;
    case 4: op=divv; break;
    case 5: op=equa; break;
    }
}
void geoArithBehaviour::doaction(osg::Node *) { // do math operation
    if (in && out && op) {
        (*out)=op(*in,acon.get());
        //    std::cout << " math sum " << out<< " " << (*out) << " " << in <<" " << (*in) << std::endl;
    }
}
bool geoArithBehaviour::makeBehave(const georecord *grec, geoHeaderGeo *theHeader) {
    bool ok=false;
    const geoField *gfd=grec->getField(GEO_DB_ARITHMETIC_ACTION_INPUT_VAR);
    if (gfd) {
        unsigned fid= gfd->getUInt(); // field identifier
        in=theHeader->getVar(fid); // returns address of input var with fid
        //std::cout<< "Input " << fid << " : " << theHeader->getVarname(fid) ;
        if (in) {
            gfd=grec->getField(GEO_DB_ARITHMETIC_ACTION_OUTPUT_VAR);
            if (gfd) {
                fid= gfd->getUInt(); // field identifier
                out=theHeader->getVar(fid); // returns address of output var with fid
                //std::cout<< " Output " <<  fid << " : " << theHeader->getVarname(fid) << std::endl;
                gfd=grec->getField(GEO_DB_ARITHMETIC_ACTION_OP_TYPE);
                unsigned int iop=gfd?gfd->getUInt():1;
                setType(iop); // default add?
                gfd=grec->getField(GEO_DB_ARITHMETIC_ACTION_OPERAND_VALUE);
                if (gfd) {
                    acon.set(gfd->getFloat()); // field identifier
                    ok=true;
                }
                gfd=grec->getField(GEO_DB_ARITHMETIC_ACTION_OPERAND_VAR);
                if (gfd) {
                    unsigned fid= gfd->getUInt(); // field identifier
                    ok=acon.set(theHeader->getVar(fid));
                }
            }
        }
    }
    return ok;
}

void geoAr3Behaviour::setType(unsigned int iact) {
    switch (iact) {
    case DB_DSK_LINEAR_ACTION: op=linear; break; /* op=addv; */
    case DB_DSK_INVERSE_ACTION: op=lininv; break;
        //    case 3: op=linmod; break;
        //    case 4: op=linsqr; break;
    case DB_DSK_TRUNCATE_ACTION: op=trunc; break;
    case DB_DSK_PERIODIC_ACTION: op=linabs; break;
    case DB_DSK_IF_THEN_ELSE_ACTION: op=ifelse; break;
    }
}
void geoAr3Behaviour::setTrigType(int iop) {
    switch (iop) {
    case 1: op=trigsin; break; /* op=trigonometric, sine; */
    case 2: op=trigcos; break;
    case 3: op=trigtan; break;
    case 4: op=trigasin; break;
    case 5: op=trigacos; break;
    case 6: op=trigatan; break;
    case 7: op=trigatan2; break;
    case 8: op=trighypot; break;
    }
}
void geoAr3Behaviour::setPeriodicType(int iop) {
    switch (iop) {
    case 1: op=period_1; break; /* op=period type 1; */
    case 2: op=period_2; break;
    }
}

void geoAr3Behaviour::doaction(osg::Node *) { // do math operation
    if (in && out && op) {
        double var3=bcon.get();
        *out=op(*in,getconstant(),var3);
        //std::cout << " ar3 sum " << out<< " " << (*out) << " con " << getconstant() <<" b: " << bcon.get() << std::endl;
    }
}
bool geoAr3Behaviour::makeBehave(const georecord *grec, geoHeaderGeo *theHeader) {
    bool ok=false;
    const geoField *gfd=grec->getField(GEO_DB_EQUATION_ACTION_INPUT_VAR);
    const unsigned int act=grec->getType();
    if (gfd) {
        unsigned fid= gfd->getUInt(); // field identifier
        in=theHeader->getVar(fid); // returns address of input var with fid
        if (in) {
            gfd=grec->getField(GEO_DB_EQUATION_ACTION_OUTPUT_VAR);
            if (gfd) {
                fid= gfd->getUInt(); // field identifier
                out=theHeader->getVar(fid); // returns address of output var with fid
                if (act==DB_DSK_TRIG_ACTION) {
                    gfd=grec->getField(GEO_DB_TRIG_ACTION_OP);
                    int iop=gfd?gfd->getInt():1;
                    setTrigType(iop); // one of sin...
                } else if (act==DB_DSK_PERIODIC_ACTION) {
                    gfd=grec->getField(GEO_DB_PERIODIC_ACTION_TYPE);
                    int iop=gfd?gfd->getInt():1; // type 1 or 2 periodic
                    setPeriodicType(iop); // one of period 1 or 2...
                } else if (act==DB_DSK_IF_THEN_ELSE_ACTION) {
                    setType(act); // if..else if (a is in range (-1,1) =b else=c
                } else {
                    setType(act); // default linear, inverse, mod.. a.b+c
                    setConstant(1);
                    ok=true;
                }
                gfd=grec->getField(GEO_DB_EQUATION_ACTION_A_VAL);
                if (gfd) {
                    setConstant(gfd->getFloat()); // field identifier
                    ok=true;
                }
                gfd=grec->getField(GEO_DB_EQUATION_ACTION_A_VAR);
                if (gfd) {
                    unsigned fid= gfd->getUInt(); // field identifier
                    ok=setVariable(theHeader->getVar(fid));
                }
                gfd=grec->getField(GEO_DB_EQUATION_ACTION_C_VAL);
                if (gfd) {
                    bcon.set(gfd->getFloat()); // field identifier
                    ok=true;
                }
                gfd=grec->getField(GEO_DB_EQUATION_ACTION_C_VAR);
                if (gfd) {
                    unsigned fid= gfd->getUInt(); // field identifier
                    ok=bcon.set(theHeader->getVar(fid));
                }
            }
        }
    }
    return ok;
}

void geoCompareBehaviour::setType(unsigned int iop) {
    switch (iop) {
    case 1: oper=LESS;break;
    case 2: oper=LESSOREQ; break;
    case 3: oper=GREATER; break;
    case 4: oper=GREATOREQ; break;
    case 5: oper=EQUALTO; break;
    }
}

void geoCompareBehaviour::doaction(osg::Node *) { // do compare operation
    if (in && out) {
        double var2=varop? *varop : constant;
        switch (oper) {
        case 1: *out = (*in < var2) ? 1.0: -1.0; break;// Less
        case 2: *out = (*in <= var2) ? 1.0: -1.0; break;//=LessOREQ
        case 3: *out = (*in > var2) ? 1.0: -1.0; break; // greater...
        case 4: *out = (*in >= var2) ? 1.0: -1.0; break;
        case 5: *out = (*in == var2) ? 1.0: -1.0; break;
        case UNKNOWN: break;
        }
    }
}
bool geoCompareBehaviour::makeBehave(const georecord *grec, geoHeaderGeo *theHeader) {
    bool ok=false;
    const geoField *gfd=grec->getField(GEO_DB_COMPARE_ACTION_INPUT_VAR);
    if (gfd) {
        unsigned fid= gfd->getUInt(); // field identifier
        in=theHeader->getVar(fid); // returns address of input var with fid
        if (in) {
            gfd=grec->getField(GEO_DB_COMPARE_ACTION_OUTPUT_VAR);
            if (gfd) {
                fid= gfd->getUInt(); // field identifier
                out=theHeader->getVar(fid); // returns address of output var with fid
                gfd=grec->getField(GEO_DB_COMPARE_ACTION_OP_TYPE);
                unsigned int iop=gfd?gfd->getUInt():1;
                setType(iop); // default add?
                gfd=grec->getField(GEO_DB_COMPARE_ACTION_OPERAND_VALUE);
                if (gfd) {
                    constant= gfd->getFloat(); // field identifier
                    ok=true;
                }
                gfd=grec->getField(GEO_DB_COMPARE_ACTION_OPERAND_VAR);
                if (gfd) {
                    unsigned fid= gfd->getUInt(); // field identifier
                    varop=theHeader->getVar(fid);
                    ok=varop != NULL;
                }
            }
        }
    }
    return ok;
}

void geoRangeBehaviour::doaction(osg::Node *) { // do math operation
    if (in && out) {
        float v=*in;
        if (v<inmin) v=inmin;
        if (v>inmax) v=inmax;
        v=(v-inmin)/(inmax-inmin);
        *out = outmin+v*(outmax-outmin);
    }
}
bool geoRangeBehaviour::makeBehave(const georecord *grec, geoHeaderGeo *theHeader) {
    bool ok=false;
    const geoField *gfd=grec->getField(GEO_DB_RANGE_ACTION_INPUT_VAR);
    if (gfd) {
        unsigned fid= gfd->getUInt(); // field identifier
        in=theHeader->getVar(fid); // returns address of input var with fid
        if (in) {
            gfd=grec->getField(GEO_DB_RANGE_ACTION_OUTPUT_VAR);
            if (gfd) {
                fid= gfd->getUInt(); // field identifier
                out=theHeader->getVar(fid); // returns address of output var with fid
                gfd=grec->getField(GEO_DB_RANGE_ACTION_IN_MIN_VAL);
                inmin=gfd?gfd->getFloat():-1.e32;
                gfd=grec->getField(GEO_DB_RANGE_ACTION_IN_MAX_VAL);
                inmax= gfd?gfd->getFloat() : 1.e32; // field identifier
                gfd=grec->getField(GEO_DB_RANGE_ACTION_OUT_MIN_VAL);
                outmin=gfd?gfd->getFloat():-1.e32;
                gfd=grec->getField(GEO_DB_RANGE_ACTION_OUT_MAX_VAL);
                outmax= gfd?gfd->getFloat() : 1.e32; // field identifier
                ok=true;
            }
        }
    }
    return ok;
}

void geoClampBehaviour::doaction(osg::Node *) { // do math operation
    if (in && out) {
        float v=*in;
        if (v<min) v=min;
        if (v>max) v=max;
        *out = v;
    }
}
bool geoClampBehaviour::makeBehave(const georecord *grec, geoHeaderGeo *theHeader) {
    bool ok=false;
    const geoField *gfd=grec->getField(GEO_DB_CLAMP_ACTION_INPUT_VAR);
    if (gfd) {
        unsigned fid= gfd->getUInt(); // field identifier
        in=theHeader->getVar(fid); // returns address of input var with fid
        if (in) {
            gfd=grec->getField(GEO_DB_CLAMP_ACTION_OUTPUT_VAR);
            if (gfd) {
                fid= gfd->getUInt(); // field identifier
                out=theHeader->getVar(fid); // returns address of output var with fid
                gfd=grec->getField(GEO_DB_CLAMP_ACTION_MIN_VAL);
                min=gfd?gfd->getFloat():-1.e32;
                gfd=grec->getField(GEO_DB_CLAMP_ACTION_MAX_VAL);
                max= gfd?gfd->getFloat() : 1.e32; // field identifier
                ok=true;
            }
        }
    }
    return ok;
}

void geoDiscreteBehaviour::doaction(osg::Node *) { // do math operation
    if (in && out) {
        float v=*in;
        *out=rangelist.begin()->getVal();
        for (std::vector<geoRange>::const_iterator itr=rangelist.begin(); 
        itr<rangelist.end(); itr++) {
            if (v>=itr->getMin() && v<=itr->getMax()) *out=itr->getVal();
        }
    }
}
bool geoDiscreteBehaviour::makeBehave(const georecord *grec, geoHeaderGeo *theHeader) {
    bool ok=false;
    const geoField *gfd=grec->getField(GEO_DB_DISCRETE_ACTION_INPUT_VAR);
    if (gfd) {
        unsigned fid= gfd->getUInt(); // field identifier
        in=theHeader->getVar(fid); // returns address of input var with fid
        if (in) {
            gfd=grec->getField(GEO_DB_DISCRETE_ACTION_OUTPUT_VAR);
            if (gfd) {
                fid= gfd->getUInt(); // field identifier
                out=theHeader->getVar(fid); // returns address of output var with fid
                gfd=grec->getField(GEO_DB_DISCRETE_ACTION_NUM_ITEMS);
                unsigned int nr=gfd?gfd->getUInt():1;
                unsigned int i;
                for (i=0; i<nr; i++) {
                    geoRange gr;
                    rangelist.push_back(gr);
                }
                const geoField *gfdmin=grec->getField(GEO_DB_DISCRETE_ACTION_MIN_VALS);
                const geoField *gfdmax=grec->getField(GEO_DB_DISCRETE_ACTION_MAX_VALS);
                const geoField *gfdval=grec->getField(GEO_DB_DISCRETE_ACTION_MAP_VALS);
                if (gfdmin) {
                    float *fmin=gfdmin->getFloatArr();
                    float *fmax=gfdmax->getFloatArr();
                    float *fval=gfdval->getFloatArr();
                    if (fmin && fmax && fval) {
                        for (i=0; i<nr; i++) {
                            rangelist[i].setMin(fmin[i]);
                            rangelist[i].setMax(fmax[i]);
                            rangelist[i].setVal(fval[i]);
                        }
                    }
                }
                ok=true;
            }
        }
    }
    return ok;
}


void geoMoveBehaviour::doaction(osg::Node *node) {
    if (getVar()) {
        MatrixTransform *mtr=dynamic_cast<MatrixTransform *> (node);
        switch (getType()) {
        case DB_DSK_SCALE_ACTION:
            mtr->preMult( osg::Matrix::scale(axis*(getValue())) );
            break;
        case DB_DSK_TRANSLATE_ACTION:
            mtr->preMult( osg::Matrix::translate(axis*(getValue())) );
            break;
        case DB_DSK_ROTATE_ACTION:
            //std::cout << node->getName() << " v: " << getVar() << " rotion " << DEG2RAD(getValue()) << std::endl;
            mtr->preMult( osg::Matrix::translate(-centre)* 
                osg::Matrix::rotate(DEG2RAD(getValue()),axis)* // nov 2003 negative rotation convention
                osg::Matrix::translate(centre));
            break;
        }
    }
}

bool geoMoveBehaviour::makeBehave(const georecord *grec, const geoHeaderGeo *theHeader) {
    bool ok=false;
    const unsigned int act=grec->getType();
    setType(act);
    if (act==DB_DSK_ROTATE_ACTION) {
        const geoField *gfd=grec->getField(GEO_DB_ROTATE_ACTION_INPUT_VAR);
        if (gfd) {
            unsigned fid= gfd->getUInt(); // field identifier
            double *vcon=theHeader->getVar(fid); // returns address of var with fid
            if (vcon) {
                // std::cout<< "rotInput " << fid << " : " << theHeader->getVarname(fid)<< std::endl ;
                setVar(vcon);
                const geoField *gfdir=grec->getField(GEO_DB_ROTATE_ACTION_DIR);
                int flip=gfdir!=NULL; // ?(gfdir->getInt()):false;
//                printf("Flip %d gfdir %x\n",flip, gfdir);
                gfd=grec->getField(GEO_DB_ROTATE_ACTION_VECTOR);
                if (gfd) {
                    float *ax= gfd->getVec3Arr(); // field identifier
                    if (flip) setAxis(-osg::Vec3(ax[0],ax[1],ax[2]));
                    else setAxis(osg::Vec3(ax[0],ax[1],ax[2]));
                }
                gfd=grec->getField(GEO_DB_ROTATE_ACTION_ORIGIN);
                if (gfd) {
                    float *ct= gfd->getVec3Arr(); // field identifier
                    setCentre(osg::Vec3(ct[0],ct[1],ct[2]));
                }
                ok=true;
            }
        }
    } else if (act==DB_DSK_TRANSLATE_ACTION) {            
        const geoField *gfd=grec->getField(GEO_DB_TRANSLATE_ACTION_INPUT_VAR);
        if (gfd) {
            unsigned fid= gfd->getUInt(); // field identifier
            double *vcon=theHeader->getVar(fid); // returns address of var with fid
            if (vcon) {
                setVar(vcon);
                gfd=grec->getField(GEO_DB_TRANSLATE_ACTION_VECTOR);
                if (gfd) {
                    float *ax= gfd->getVec3Arr(); // field identifier
                    setAxis(osg::Vec3(ax[0],ax[1],ax[2]));
                }
                gfd=grec->getField(GEO_DB_TRANSLATE_ACTION_ORIGIN);
                if (gfd) {
                    float *ct= gfd->getVec3Arr(); // field identifier
                    setCentre(osg::Vec3(ct[0],ct[1],ct[2]));
                }
                ok=true;
            }
        }
    } else if (act==DB_DSK_SCALE_ACTION) {     // Nov 2002 not yet implemented in the modeller!       
    }
    return ok;
}
void geoMoveVertexBehaviour::doaction(osg::Matrix *mtr) {
    // update the matrix mtr
    if (getVar()) {
            switch (getType()) {
            case DB_DSK_SCALE_ACTION:
                *mtr = (*mtr)*osg::Matrix::scale(getAxis()*(getValue())) ;
                break;
            case DB_DSK_TRANSLATE_ACTION:
                *mtr = (*mtr)*osg::Matrix::translate(getAxis()*(getValue())) ;
                break;
            case DB_DSK_ROTATE_ACTION:
                //std::cout << dr->getName() << " v: " << getVar() << " rotion " << DEG2RAD(getValue()) << std::endl;
                *mtr = (*mtr)*osg::Matrix::translate(-getCentre())* 
                    osg::Matrix::rotate(DEG2RAD(getValue()),getAxis())* 
                    osg::Matrix::translate(getCentre());
                break;
            }
    }
}

bool geoMoveVertexBehaviour::makeBehave(const georecord *grec, const geoHeaderGeo *theHeader)
{
    const unsigned int act=grec->getType();
    bool ok=false;
    setType(act);
    if (act==DB_DSK_ROTATE_ACTION) {
        const geoField *gfd=grec->getField(GEO_DB_ROTATE_ACTION_INPUT_VAR);
        if (gfd) {
            unsigned fid= gfd->getUInt(); // field identifier
            double *vcon=theHeader->getVar(fid); // returns address of var with fid
            if (vcon) {
                // std::cout<< "rotInput " << fid << " : " << theHeader->getVarname(fid)<< std::endl ;
                setVar(vcon);
                gfd=grec->getField(GEO_DB_ROTATE_ACTION_VECTOR);
                if (gfd) {
                    float *ax= gfd->getVec3Arr(); // field identifier
                    setAxis(osg::Vec3(ax[0],ax[1],ax[2]));
                }
                gfd=grec->getField(GEO_DB_ROTATE_ACTION_ORIGIN);
                if (gfd) {
                    float *ct= gfd->getVec3Arr(); // field identifier
                    setCentre(osg::Vec3(ct[0],ct[1],ct[2]));
                }
                ok=true;
            }
        }
    } else if (act==DB_DSK_TRANSLATE_ACTION) {            
        const geoField *gfd=grec->getField(GEO_DB_TRANSLATE_ACTION_INPUT_VAR);
        if (gfd) {
            unsigned fid= gfd->getUInt(); // field identifier
            double *vcon=theHeader->getVar(fid); // returns address of var with fid
            if (vcon) {
                setVar(vcon);
                gfd=grec->getField(GEO_DB_TRANSLATE_ACTION_VECTOR);
                if (gfd) {
                    float *ax= gfd->getVec3Arr(); // field identifier
                    setAxis(osg::Vec3(ax[0],ax[1],ax[2]));
                }
                gfd=grec->getField(GEO_DB_TRANSLATE_ACTION_ORIGIN);
                if (gfd) {
                    float *ct= gfd->getVec3Arr(); // field identifier
                    setCentre(osg::Vec3(ct[0],ct[1],ct[2]));
                }
                ok=true;
            }
        }
    } else if (act==DB_DSK_SCALE_ACTION) {     // Nov 2002 not yet implemented in the modeller!       
    }
    return ok;
}

bool geoVisibBehaviour::makeBehave(const georecord *grec, const geoHeaderGeo *theHeader) {
    bool ok=false;
    const geoField *gfd= grec->getField(GEO_DB_VISIBILITY_ACTION_INPUT_VAR);
    if (gfd) {
        unsigned fid= gfd->getUInt(); // field identifier
        setVar(theHeader->getVar(fid)); // returns address of input var with fid
        ok=true; // all data supplied
    }
    return ok;
}
void geoVisibBehaviour::doaction(osg::Node *node)
{ // do visibility operation on Node
    if (getVar()) {
        if (getValue() <0.0) {
            node->setNodeMask(0x0); // invisible
        } else {
            node->setNodeMask(0xffffffff); // visible
        }
    }
}

bool geoColourBehaviour::makeBehave(const georecord *grec, const geoHeaderGeo *theHeader) {
    bool ok=false;
    const geoField *gfd= grec->getField(GEO_DB_COLOR_RAMP_ACTION_INPUT_VAR);
    if (gfd) {
        unsigned fid= gfd->getUInt(); // field identifier
        setVar(theHeader->getVar(fid)); // returns address of input var with fid
        gfd=grec->getField(GEO_DB_COLOR_RAMP_ACTION_COLOR_FROM_PALETTE);
        if (gfd) {}
        gfd=grec->getField(GEO_DB_COLOR_RAMP_ACTION_TOP_COLOR_INDEX);
        topcindx=(gfd? gfd->getUInt():4096);
        gfd=grec->getField(GEO_DB_COLOR_RAMP_ACTION_BOTTOM_COLOR_INDEX);
        botcindx=(gfd? gfd->getUInt():0);
        //also available: GEO_DB_COLOR_RAMP_ACTION_MATCH_COLUMNS
        ok=true; // all data supplied
    }
    return ok;
}
void geoColourBehaviour::doaction(osg::Drawable *dr)
{ // do visibility operation on Node
    if (getVar()) {
        double val=getValue();
        unsigned int idx=(unsigned int)val;
        osg::Geometry *gm=dynamic_cast<osg::Geometry *>(dr);
        if (gm) {
            osg::Vec4Array* cla = dynamic_cast<osg::Vec4Array*>(gm->getColorArray());
            if (cla) { // traps a colour behaviour added when using material for colour.
                for (unsigned int i=nstart; i<(nend); i++) {
                    unsigned char col[4];
                    unsigned int idxtop=idx/128;
                    (*colours)[idxtop].get(col); // from the colour palette
                    float frac=(float)(idx-idxtop*128)/128.0f;
                    (*cla)[i].set(col[0]*frac/255.0,col[1]*frac/255.0,col[2]*frac/255.0,1);
                }
            }
        }
    }
}

void geoStrContentBehaviour::doaction(osg::Drawable* /*node*/) 
{ // do new text
#ifdef USETEXT // buggy text feb 2003
    osgText::Text *txt=dynamic_cast<osgText::Text *>(node);
    char content[32];
    switch (vt) {
    case INT:
        sprintf(content, format, (int)getValue());
        break;
    case FLOAT:
        sprintf(content, format, (float)getValue());
        break;
    case DOUBLE:
        sprintf(content, format, getValue());
        break;
    case CHAR:
        sprintf(content, format, (char *)getVar());
        break;
    default:
        sprintf(content, format, (char *)getVar());
    }
    txt->setText(std::string(content));
#endif
}
bool geoStrContentBehaviour::makeBehave(const georecord *grec, const geoHeaderGeo *theHeader) {
    bool ok=false;
    const geoField *gfd=grec->getField(GEO_DB_STRING_CONTENT_ACTION_INPUT_VAR);
    if (gfd) {
        unsigned fid= gfd->getUInt(); // field identifier
        setVar(theHeader->getVar(fid)); // returns address of input var with fid
        if (getVar()) {
            gfd=grec->getField(GEO_DB_STRING_CONTENT_ACTION_FORMAT);
            if (gfd) {
                char *ch=gfd->getChar();
                format=new char[strlen(ch)+1];
                strcpy(format, ch);
                {
                    char *ctmp=format;
                    while (*ctmp) {
                        if (*ctmp=='d') vt=INT;
                        if (*ctmp=='f' && vt!=DOUBLE) vt=FLOAT;
                        if (*ctmp=='l') vt=DOUBLE;
                        ctmp++;
                    }
                }
                gfd=grec->getField(GEO_DB_STRING_CONTENT_ACTION_PADDING_TYPE);
                gfd=grec->getField(GEO_DB_STRING_CONTENT_ACTION_PADDING_TYPE);
                ok=true;
            }
        }
    }
    return ok;
}

void geoBehaviourCB::operator() (osg::Node *node, osg::NodeVisitor* nv)
{ // callback updates the transform, colour, string content...
    MatrixTransform *mtr=dynamic_cast<MatrixTransform *> (node);
    if (mtr) mtr->setMatrix(Matrix::identity()); // all actions are multiplied to this
//        printf("setting matrix %x\n", mtr);
 //   PositionAttitudeTransform *patr=dynamic_cast<PositionAttitudeTransform *> (node);
   // if (patr) patr->setMatrix(Matrix::identity()); // all actions are multiplied to this
    for (std::vector<geoBehaviour *>::const_iterator itr=gblist.begin();
    itr<gblist.end();
    itr++) { // motion behaviour
         (*itr)->doaction(node);
/* === the above is equivalent to my old code with lots of tests in: */
/*      geoArithBehaviour *ab=dynamic_cast<geoArithBehaviour *>(*itr);
        if (ab) ab->doaction(node);
        geoAr3Behaviour *a3=dynamic_cast<geoAr3Behaviour *>(*itr);
        if (a3) a3->doaction(node);
        geoClampBehaviour *cb=dynamic_cast<geoClampBehaviour *>(*itr);
        if (cb) cb->doaction(node);
        geoRangeBehaviour *cr=dynamic_cast<geoRangeBehaviour *>(*itr);
        if (cr) cr->doaction(node);
        geoCompareBehaviour *cmb=dynamic_cast<geoCompareBehaviour *>(*itr);
        if (cmb) cmb->doaction(node);
        geoDiscreteBehaviour *db=dynamic_cast<geoDiscreteBehaviour *>(*itr);
        if (db) db->doaction(node);
        geoMoveBehaviour *mb=dynamic_cast<geoMoveBehaviour *>(*itr);
        if (mb) mb->doaction(node);
        // or visibility..
        geoVisibBehaviour *vb=dynamic_cast<geoVisibBehaviour *>(*itr);
        if (vb) vb->doaction(node);  */
    }
    traverse(node,nv);
}

void geoBehaviourDrawableCB::update(osg::NodeVisitor *,osg::Drawable *dr) {
    Matrix mtr;
    int prevvtr=-1; // previously moved vertex
    Vec3 pos;
    mtr.identity();
    std::vector<geoBehaviour *>::const_iterator itr;
    for (itr=gblist.begin();
         itr<gblist.end();
         itr++)
        { // color or string action behaviour, can also do maths...
        //     (*itr)->doaction(dr);
             Node *nd=NULL;
        geoArithBehaviour *ab=dynamic_cast<geoArithBehaviour *>(*itr);
        if (ab) ab->doaction(nd);
        geoAr3Behaviour *a3=dynamic_cast<geoAr3Behaviour *>(*itr);
        if (a3) a3->doaction(nd);
        geoClampBehaviour *cb=dynamic_cast<geoClampBehaviour *>(*itr);
        if (cb) cb->doaction(nd);
        geoRangeBehaviour *cr=dynamic_cast<geoRangeBehaviour *>(*itr);
        if (cr) cr->doaction(nd);
        geoStrContentBehaviour *sb=dynamic_cast<geoStrContentBehaviour *>(*itr);
        if (sb) sb->doaction(dr);
        // colorbehaviour may be for 1 or all vertices
        geoColourBehaviour *clrb=dynamic_cast<geoColourBehaviour *>(*itr);
        if (clrb) clrb->doaction(dr);
        geoMoveVertexBehaviour *mvvb=dynamic_cast<geoMoveVertexBehaviour *>(*itr);
        if (mvvb && (prevvtr<0 || prevvtr==mvvb->getindex())) {
            mvvb->doaction(&mtr);
            pos=mvvb->getpos();
            prevvtr=mvvb->getindex();
        }
    }
    osg::Geometry *gm=dynamic_cast<osg::Geometry *>(dr);
    if (gm && prevvtr>=0) {
        osg::Vec3Array* vtxa = dynamic_cast<osg::Vec3Array*>(gm->getVertexArray());
        bool newpos=false;
        (*vtxa)[prevvtr]=pos*mtr;
        do { // check for other vertices that may be animated
            newpos=false;
            mtr.identity();
            for (itr=gblist.begin();
            itr<gblist.end();
            itr++) { // color or string action behaviour, can also do maths...
                geoMoveVertexBehaviour *mvvb=dynamic_cast<geoMoveVertexBehaviour *>(*itr);
                if (mvvb) {
                    int vidx=mvvb->getindex();
                    if (mvvb && (prevvtr<vidx || (newpos && prevvtr==vidx))) {
                        mvvb->doaction(&mtr);
                        prevvtr=vidx;
                        pos=mvvb->getpos();
                        newpos=true;
                    }
                }
            } 
            if (newpos) {
                osg::Vec3Array* vtxa = dynamic_cast<osg::Vec3Array*>(gm->getVertexArray());
                (*vtxa)[prevvtr]=pos*mtr;
            }
        } while (newpos);
    }
}
