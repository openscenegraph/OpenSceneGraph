// GEO format (carbon graphics Inc) loader for the OSG real time scene graph
// www.carbongraphics.com for more information about the Geo animation+ modeller
// 2002

#include "osg/Image"
#include "osg/Group"
#include "osg/LOD"
#include "osg/Billboard"
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
#include <osg/PositionAttitudeTransform>


#include "osgDB/FileNameUtils"
#include "osgDB/Registry"
#include <osgDB/ReadFile>
#include "osgDB/Input"
#include "osgDB/Output"

#include <stdio.h>

// specific to GEO
#include "geoFormat.h"
#include "geoTypes.h"
#include "geoUnits.h"
#include "osgGeoStructs.h"

using namespace osg;
using namespace osgDB;

class vertexInfo { // holds vertex information for an entire osg::geometry
public:
	vertexInfo(const float *coord_pool, const float *normal_pool) {
		norms=new osg::Vec3Array;
		coords=new osg::Vec3Array;
		txcoords=new osg::Vec2Array;
		coordindices=new osg::IntArray;
		normindices=new osg::IntArray;
		txindices=new osg::IntArray;
		cpool=coord_pool; npool=normal_pool;
	}
	inline osg::Vec3Array *getNorms() const { return norms;}
	inline osg::Vec3Array *getCoords() const { return coords;}
	inline osg::Vec2Array *getTexCoords() const { return txcoords;}
	inline osg::IntArray *getCoordIndices() const { return coordindices;}
	inline osg::IntArray *getNormIndices() const { return normindices;}
	inline osg::IntArray *getTextureIndices() const { return txindices;}
	void addIndices(georecord *gr)
	{ // this must only be called with a vertex georecord.
		if (gr->getType()==DB_DSK_VERTEX) {
			const geoField *gfd=gr->getField(GEO_DB_VRTX_NORMAL);
			int nrmindex=gfd ? gfd->getUInt():0;
			normindices->push_back(nrmindex);
			norms->push_back(osg::Vec3(npool[3*nrmindex],npool[3*nrmindex+1],npool[3*nrmindex+2]));
			gfd=gr->getField(GEO_DB_VRTX_COORD);
			unsigned int idx=gfd ? gfd->getInt():0;
			coords->push_back(osg::Vec3(cpool[3*idx],cpool[3*idx+1],cpool[3*idx+2]));
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
	    return output; 	// to enable cascading, monkey copy from osg\plane or \quat, Ubyte4, vec2,3,4,... 
	}
private:
	const float *cpool; // passed in from the geo file
	const float *npool;
	osg::Vec3Array *norms;
	osg::Vec3Array *coords;
	osg::Vec2Array *txcoords;
	osg::IntArray *coordindices;
	osg::IntArray *normindices;
	osg::IntArray *txindices;
};

class ReaderWriterGEO : public ReaderWriter
{
    public:
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

                // load all nodes in file, placing them in a linear list corresponding to the on disk file.
                while(!fin.eof())
                {
					georecord gr;
					gr.readfile(fin);
				//	osg::notify(osg::WARN) << gr << std::endl;
					recs.push_back(gr); // add to a list of all records
                }
				fin.close();
				// now sort the reocrds so that any record followed by a PUSh has a child set = next record, etc
				std::vector<georecord *> sorted=sort(recs); // tree-list of sorted record pointers
				std::fstream fout("georex.txt", std::ios::out );
				fout << "Debug file " << std::endl;
			//	output(fout,sorted); // print details of the sorted tree, with records indented.
				nodeList=makeosg(sorted, fout); // make a list of osg nodes
				fout.close();
				
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
            else
            {
                return 0L;
            }
        }
		std::vector<georecord *> sort(geoRecordList &recs) { // return a tree-list of sorted record pointers
			// which mirrors the original .geo file (containers hold push/pop blocks).
			std::vector<georecord *> sorted;
			class georecord *curparent=NULL;
			for (geoRecordList::iterator itr=recs.begin();
			itr!=recs.end();
			++itr) {
				// osg::notify(osg::WARN) << *itr << std::endl;
				// now parse for push/pops and add to lists
				if ((*itr).getType()==DB_DSK_COORD_POOL) {
					const geoField *gfd=itr->getField(GEO_DB_COORD_POOL_VALUES);
					coord_pool= (gfd) ? (gfd->getVec3Arr()):NULL;
				} else if ((*itr).getType()==DB_DSK_NORMAL_POOL) {
					const geoField *gfd=itr->getField(GEO_DB_NORMAL_POOL_VALUES);
					normal_pool= (gfd) ? (gfd->getVec3Arr()):NULL;
				}
				if ((*itr).getType()==DB_DSK_PUSH) {
					curparent= itr-1;
				} else if ((*itr).getType()==DB_DSK_POP) {
					if (curparent) curparent=curparent->getparent();
				} else {
					if (curparent) {
						(*itr).setparent(curparent);
						curparent->addchild(itr);
					} else {
						sorted.push_back(itr);
					}
				}
			}
			return sorted;
		}
		int getprim(const std::vector<georecord *> gr,vertexInfo &vinf)
		{ // fills vinf with txcoords = texture coordinates, txindex=txindex etc
			int nv=0;
			if (gr.size()>0) {
				for (std::vector<georecord *>::const_iterator itr=gr.begin();
					itr!=gr.end();
					++itr) {
					vinf.addIndices((*itr));
					nv++;
				}
			}
			return nv;
		}
		std::vector<osg::Geometry *>makeGeometry(const std::vector<georecord *> gr, const unsigned int imat)
		{
			// txcoords returns with a set of vec2 (UV coords for texturing)
			std::vector<osg::Geometry *> geom;
			if (gr.size()>0) {
				std::vector<int> ia; // list of texture indices found i this geode; sort into new 
				vertexInfo vinf(coord_pool,normal_pool); // holds all types of coords, indices etc
				int nstart=0; // start of list
				for (std::vector<georecord *>::const_iterator itr=gr.begin();
				itr!=gr.end();
				++itr) {
					if ((*itr)->getType()==DB_DSK_POLYGON) {
						int txidx=-1;
						const geoField *gfd=(*itr)->getField(GEO_DB_POLY_TEX);
						if (gfd) txidx=gfd->getInt();
						int igidx=0, igeom=-1;
						for (IntArray::const_iterator itrint=ia.begin();
						itrint!=ia.end();
						++itrint) {
							if (txidx==(*itrint)) igeom=igidx;
							igidx++;
						}
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
							if (imat>0 && imat<matlist.size()) dstate->setAttribute(matlist[imat]);
							else dstate->setAttribute(matlist[0]);
							nug->setStateSet( dstate );
							geom.push_back(nug);
							igeom=ia.size();
							ia.push_back(txidx); // look up table for which texture corresponds to which geom
						}
						int nv=getprim((*itr)->getchildren(),vinf);
						geom[igeom]->addPrimitive(new osg::DrawArrays(osg::Primitive::POLYGON,nstart,nv));
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
		osg::Group *makeGroup(const georecord *gr) { // group or Static transform
			osg::Group *gp=NULL;
			const geoField *gfd=gr->getField(GEO_DB_GRP_TRANSFORM);
			if (gfd) {
				osg::MatrixTransform *tr=new osg::MatrixTransform;
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
		osg::Group * makeSwitch(const georecord *gr) {
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
		osg::PositionAttitudeTransform *makeHeader(const georecord *gr) {
			osg::PositionAttitudeTransform *nup=NULL;
			const geoField *gfd=gr->getField(GEO_DB_HDR_UP_AXIS);
			if (gfd) {
				unsigned iup=gfd->getUInt();
				if (iup==GEO_DB_UP_AXIS_X) {
					nup=new PositionAttitudeTransform();
					osg::Quat q;
					q.set(1,1,0,0);
					q/=q.length();
					nup->setAttitude(q);
				} else if (iup==GEO_DB_UP_AXIS_Y) {
					nup=new PositionAttitudeTransform();
					osg::Quat q;
					q.set(1,0,0,1);
					q/=q.length();
					nup->setAttitude(q);
				}
			}
			return nup;
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
		std::vector<Node *> makeosg(const std::vector<georecord *> gr, std::fstream &fout) {
			// recursive traversal of records and extract osg::Nodes equivalent
			Group *geodeholder=NULL;
			std::vector<Node *> nodelist;
			if (gr.size()>0) {
				for (std::vector<georecord *>::const_iterator itr=gr.begin();
				itr!=gr.end();
				++itr) {
					const georecord *gr=*itr;
					if (gr->getType()== DB_DSK_GEODE) { // geodes can require >1 geometry for example if polygons have different texture indices.
						if (!geodeholder) {
							geodeholder=new osg::Group;
						}
						Geode *geode=new Geode;
						const geoField *gfd=gr->getField(GEO_DB_RENDERGROUP_MAT);
						const unsigned int imat=gfd ? gfd->getInt():0;
						std::vector<osg::Geometry *>geom=makeGeometry((*itr)->getchildren(),imat);
						for (std::vector<osg::Geometry *>::iterator itr=geom.begin();
						itr!=geom.end();
						++itr)
						{
							geode->addDrawable((*itr));
						}
						geodeholder->addChild(geode);
					} else {
						Group *holder=NULL;
						const geoField *gfd;
						switch (gr->getType()) {
						case DB_DSK_HEADER:
							holder=makeHeader(gr);

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
							break;
						case DB_DSK_BEHAVIOR:
							holder=new MatrixTransform;
							gfd=gr->getField(GEO_DB_BEHAVIOR_NAME);
							if (gfd) {
								holder->setName(gfd->getChar());
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
							holder=new Group;
							gfd=gr->getField(GEO_DB_PAGE_NAME);
							if (gfd) {
								holder->setName(gfd->getChar());
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
							fout << "==Unhandled option " << gr->getType() << std::endl;
							fout << (*gr) << std::endl;
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
							fout << "==Poorly handled option " << gr->getType() << std::endl;
							fout << (*gr) << std::endl;
							break;
						default: {
							osg::Group *gp=new Group;
							holder=gp;
							}
							break;
						}
						nodelist.push_back(holder);

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
			return nodelist;
		}
		void output(std::fstream &fout,std::vector<georecord *> gr)
		{ // debugging - print the tree of records
			static int depth=0;
			depth++;
			if (gr.size()>0) {
				for (std::vector<georecord *>::iterator itr=gr.begin();
					itr!=gr.end();
					++itr) {
					// osg::notify(osg::WARN)
					for (int i=0; i<depth-1; i++) fout << "    " ;
					fout << "Node type " << (*itr)->getType() << " ";
					fout << (**itr) << std::endl;
					fout << std::endl;
					output(fout,(*itr)->getchildren());
				}
			}
			depth--;
		}
private:
//	std::fstream fout; // debug output
	static float *coord_pool; // current vertex ooords
	static float *normal_pool; // current pool of normal vectors
	std::vector<osg::Texture2D *> txlist; // list of textures for this model
	std::vector<osg::TexEnv *> txenvlist; // list of texture environments for the textures
	std::vector<osg::Material *> matlist; // list of materials for current model
};

// now register with Registry to instantiate the above
// reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterGEO> gReaderWriter_GEO_Proxy;
float *ReaderWriterGEO::coord_pool=NULL; // current vertex ooords
float *ReaderWriterGEO::normal_pool=NULL; // current vertex ooords
