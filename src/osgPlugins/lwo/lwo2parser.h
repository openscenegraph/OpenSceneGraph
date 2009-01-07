/*******************************************************
      Lightwave Object Loader for OSG

  Copyright (C) 2004 Marco Jez <marco.jez@poste.it>
  OpenSceneGraph is (C) 2004 Robert Osfield
********************************************************/

#ifndef LWO2PARSER_
#define LWO2PARSER_

#include "iffparser.h"
#include "lwo2chunks.h"
#include "lwo2read.h"

#include <stdexcept>
#include <vector>

namespace lwo2
{

	class parser_error: public std::runtime_error {
	public:
		parser_error(const std::string &message): std::runtime_error("[LWO2 parser error] " + message) {}
	};

	template<typename Iter>
	class Parser: public iff::GenericParser<Iter> {
	public:
		Parser();
		Parser(std::ostream &os);

		virtual ~Parser();

	protected:
		virtual iff::Chunk *parse_chunk_data(const std::string &tag, const std::string &context, Iter it, Iter end);
		iff::Chunk *parse_subchunk(Iter &it, const std::string &context);
                
                Parser& operator = (const Parser&) { return *this; }
	};


	/////////////////////////////////////////////////////////////////////////
	// IMPLEMENTATION OF TEMPLATE FUNCTIONS

#	define LP_TMP	template<class Iter>

	LP_TMP Parser<Iter>::Parser()
		: iff::GenericParser<Iter>()
	{
	}

	LP_TMP Parser<Iter>::Parser(std::ostream &os)
		: iff::GenericParser<Iter>(os)
	{
	}

	LP_TMP Parser<Iter>::~Parser()
	{
	}

	LP_TMP iff::Chunk *Parser<Iter>::parse_chunk_data(const std::string &tag, const std::string &context, Iter it, Iter end)
	{
		// GLOBAL CONTEXT
		if (context.empty()) {
			if (tag == "FORM") {
				FORM *chk = new FORM;
				chk->type = read_ID4(it);
				if (std::string(chk->type.id, 4) != "LWO2") {
					throw parser_error("invalid file format");
				}
				while (it < end)
					chk->data.push_back(parse_chunk(it, "FORM"));
				return chk;
			}
		}

		// FORM CONTEXT
		if (context == "FORM") {

			if (tag == "LAYR") {
				FORM::LAYR *chk = new FORM::LAYR;
				chk->number = read_U2(it);
				chk->flags = read_U2(it);
				chk->pivot = read_VEC12(it);
				chk->name = read_S0(it);
				if (it < end) {
					chk->parent = read_I2(it);
				} else {
					chk->parent = -1;
				}
				return chk;
			}

			if (tag == "PNTS") {
				FORM::PNTS *chk = new FORM::PNTS;
				while (it < end) {
					chk->point_location.push_back(read_VEC12(it));
				}
				return chk;
			}

			if (tag == "VMAP") {
				FORM::VMAP *chk = new FORM::VMAP;
				chk->type = read_ID4(it);
				chk->dimension = read_U2(it);
				chk->name = read_S0(it);
				while (it < end) {
					FORM::VMAP::mapping_type mp;
					mp.vert = read_VX(it);
					for (int i=0; i<chk->dimension; ++i) {
						mp.value.push_back(read_F4(it));
					}
					chk->mapping.push_back(mp);
				}
				return chk;
			}

			if (tag == "POLS") {
				FORM::POLS *chk = new FORM::POLS;
				chk->type = read_ID4(it);
				while (it < end) {
					FORM::POLS::polygon_type pl;
					U2 nvf = read_U2(it);
					pl.flags = nvf >> 10;
					pl.numvert = nvf & 0x03FF;
					for (int i=0; i<pl.numvert; ++i)
						pl.vert.push_back(read_VX(it));
					chk->polygons.push_back(pl);					
				}
				return chk;
			}

			if (tag == "TAGS") {
				FORM::TAGS *chk = new FORM::TAGS;
				while (it < end) {
					std::string tags = read_S0(it);
					chk->tag_string.push_back(tags);
				}
				return chk;
			}

			if (tag == "PTAG") {
				FORM::PTAG *chk = new FORM::PTAG;
				chk->type = read_ID4(it);
				while (it < end) {
					FORM::PTAG::mapping_type mp;
					mp.poly = read_VX(it);
					mp.tag = read_U2(it);
					chk->mapping.push_back(mp);
				}
				return chk;
			}

			if (tag == "VMAD") {
				FORM::VMAD *chk = new FORM::VMAD;
				chk->type = read_ID4(it);
				chk->dimension = read_U2(it);
				chk->name = read_S0(it);
				while (it < end) {
					FORM::VMAD::mapping_type mp;
					mp.vert = read_VX(it);
					mp.poly = read_VX(it);
					for (int i=0; i<chk->dimension; ++i)
						mp.value.push_back(read_F4(it));
					chk->mapping.push_back(mp);
				}
				return chk;
			}

			if (tag == "ENVL") {
				FORM::ENVL *chk = new FORM::ENVL;
				chk->index = read_VX(it);
				while (it < end) {
					chk->attributes.push_back(parse_subchunk(it, "FORM::ENVL"));
				}
				return chk;
			}

			if (tag == "CLIP") {
				FORM::CLIP *chk = new FORM::CLIP;
				chk->index = read_U4(it);
				while (it < end) {
					chk->attributes.push_back(parse_subchunk(it, "FORM::CLIP"));
				}
				return chk;
			}

			if (tag == "SURF") {
				FORM::SURF *chk = new FORM::SURF;
				chk->name = read_S0(it);
				chk->source = read_S0(it);
				while (it < end) {
					chk->attributes.push_back(parse_subchunk(it, "FORM::SURF"));
				}
				return chk;
			}

			if (tag == "BBOX") {
				FORM::BBOX *chk = new FORM::BBOX;
				chk->min = read_VEC12(it);
				chk->max = read_VEC12(it);
				return chk;
			}

			if (tag == "DESC") {
				FORM::DESC *chk = new FORM::DESC;
				chk->description_line = read_S0(it);
				return chk;
			}

			if (tag == "TEXT") {
				FORM::TEXT *chk = new FORM::TEXT;
				chk->comment = read_S0(it);
				return chk;
			}

			if (tag == "ICON") {
				FORM::ICON *chk = new FORM::ICON;
				chk->encoding = read_U2(it);
				chk->width = read_U2(it);
				while (it < end) chk->data.push_back(read_U1(it));
				return chk;
			}

		}

		// ENVELOPE CONTEXT
		if (context == "FORM::ENVL") {

			if (tag == "TYPE") {
				FORM::ENVL::TYPE *chk = new FORM::ENVL::TYPE;
				chk->user_format = read_U1(it);
				chk->type = read_U1(it);
				return chk;
			}

			if (tag == "PRE ") {
				FORM::ENVL::PRE *chk = new FORM::ENVL::PRE;
				chk->type = read_U2(it);
				return chk;
			}

			if (tag == "POST") {
				FORM::ENVL::POST *chk = new FORM::ENVL::POST;
				chk->type = read_U2(it);
				return chk;
			}

			if (tag == "KEY ") {
				FORM::ENVL::KEY *chk = new FORM::ENVL::KEY;
				chk->time = read_F4(it);
				chk->value = read_F4(it);
				return chk;
			}

			if (tag == "SPAN") {
				FORM::ENVL::SPAN *chk = new FORM::ENVL::SPAN;
				chk->type = read_ID4(it);
				while (it < end) chk->parameters.push_back(read_F4(it));
				return chk;
			}

			if (tag == "CHAN") {
				FORM::ENVL::CHAN *chk = new FORM::ENVL::CHAN;
				chk->server_name = read_S0(it);
				chk->flags = read_U2(it);
				while (it < end) chk->data.push_back(read_U1(it));
				return chk;
			}

			if (tag == "NAME") {
				FORM::ENVL::NAME *chk = new FORM::ENVL::NAME;
				chk->channel_name = read_S0(it);
				return chk;
			}

		}

		// CLIP CONTEXT
		if (context == "FORM::CLIP") {

			if (tag == "STIL") {
				FORM::CLIP::STIL *chk = new FORM::CLIP::STIL;
				chk->name = read_FNAM0(it);
				return chk;
			}

			if (tag == "ISEQ") {
				FORM::CLIP::ISEQ *chk = new FORM::CLIP::ISEQ;
				chk->num_digits = read_U1(it);
				chk->flags = read_U1(it);
				chk->offset = read_I2(it);
				chk->reserved = read_U2(it);
				chk->start = read_I2(it);
				chk->end = read_I2(it);
				chk->prefix = read_FNAM0(it);
				chk->suffix = read_S0(it);
				return chk;
			}

			if (tag == "ANIM") {
				FORM::CLIP::ANIM *chk = new FORM::CLIP::ANIM;
				chk->filename = read_FNAM0(it);
				chk->server_name = read_S0(it);
				chk->flags = read_U2(it);
				while (it < end) chk->data.push_back(read_U1(it));
				return chk;
			}

			if (tag == "XREF") {
				FORM::CLIP::XREF *chk = new FORM::CLIP::XREF;
				chk->index = read_U4(it);
				chk->string = read_S0(it);
				return chk;
			}

			if (tag == "STCC") {
				FORM::CLIP::STCC *chk = new FORM::CLIP::STCC;
				chk->lo = read_I2(it);
				chk->hi = read_I2(it);
				chk->name = read_FNAM0(it);
			}

			if (tag == "TIME") {
				FORM::CLIP::TIME *chk = new FORM::CLIP::TIME;
				chk->start_time = read_FP4(it);
				chk->duration = read_FP4(it);
				chk->frame_rate = read_FP4(it);
				return chk;
			}

			if (tag == "CONT") {
				FORM::CLIP::CONT *chk = new FORM::CLIP::CONT;
				chk->contrast_delta = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "BRIT") {
				FORM::CLIP::BRIT *chk = new FORM::CLIP::BRIT;
				chk->brightness_delta = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "SATR") {
				FORM::CLIP::SATR *chk = new FORM::CLIP::SATR;
				chk->saturation_delta = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "HUE ") {
				FORM::CLIP::HUE *chk = new FORM::CLIP::HUE;
				chk->hue_rotation = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "GAMM") {
				FORM::CLIP::GAMM *chk = new FORM::CLIP::GAMM;
				chk->gamma = read_F4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "NEGA") {
				FORM::CLIP::NEGA *chk = new FORM::CLIP::NEGA;
				chk->enable = read_U2(it);
				return chk;
			}

			if (tag == "IFLT") {
				FORM::CLIP::IFLT *chk = new FORM::CLIP::IFLT;
				chk->server_name = read_S0(it);
				chk->flags = read_U2(it);
				while (it < end) chk->data.push_back(read_U1(it));
				return chk;
			}

			if (tag == "PFLT") {
				FORM::CLIP::PFLT *chk = new FORM::CLIP::PFLT;
				chk->server_name = read_S0(it);
				chk->flags = read_U2(it);
				while (it < end) chk->data.push_back(read_U1(it));
				return chk;
			}

		}

		// SURFACE CONTEXT
		if (context == "FORM::SURF") {

			if (tag == "COLR") {
				FORM::SURF::COLR *chk = new FORM::SURF::COLR;
				chk->base_color = read_COL12(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "DIFF") {
				FORM::SURF::DIFF *chk = new FORM::SURF::DIFF;
				chk->intensity = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "LUMI") {
				FORM::SURF::LUMI *chk = new FORM::SURF::LUMI;
				chk->intensity = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "SPEC") {
				FORM::SURF::SPEC *chk = new FORM::SURF::SPEC;
				chk->intensity = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "REFL") {
				FORM::SURF::REFL *chk = new FORM::SURF::REFL;
				chk->intensity = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "TRAN") {
				FORM::SURF::TRAN *chk = new FORM::SURF::TRAN;
				chk->intensity = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "TRNL") {
				FORM::SURF::TRNL *chk = new FORM::SURF::TRNL;
				chk->intensity = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "GLOS") {
				FORM::SURF::GLOS *chk = new FORM::SURF::GLOS;
				chk->glossiness = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "SHRP") {
				FORM::SURF::SHRP *chk = new FORM::SURF::SHRP;
				chk->sharpness = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "BUMP") {
				FORM::SURF::BUMP *chk = new FORM::SURF::BUMP;
				chk->strength = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "SIDE") {
				FORM::SURF::SIDE *chk = new FORM::SURF::SIDE;
				chk->sidedness = read_U2(it);
				return chk;
			}

			if (tag == "SMAN") {
				FORM::SURF::SMAN *chk = new FORM::SURF::SMAN;
				chk->max_smoothing_angle = read_ANG4(it);
				return chk;
			}

			if (tag == "RFOP") {
				FORM::SURF::RFOP *chk = new FORM::SURF::RFOP;
				chk->reflection_options = read_U2(it);
				return chk;
			}

			if (tag == "RIMG") {
				FORM::SURF::RIMG *chk = new FORM::SURF::RIMG;
				chk->image = read_VX(it);
				return chk;
			}

			if (tag == "RSAN") {
				FORM::SURF::RSAN *chk = new FORM::SURF::RSAN;
				chk->seam_angle = read_ANG4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "RBLR") {
				FORM::SURF::RBLR *chk = new FORM::SURF::RBLR;
				chk->blur_percentage = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "RIND") {
				FORM::SURF::RIND *chk = new FORM::SURF::RIND;
				chk->refractive_index = read_F4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "TROP") {
				FORM::SURF::TROP *chk = new FORM::SURF::TROP;
				chk->transparency_options = read_U2(it);
				return chk;
			}

			if (tag == "TIMG") {
				FORM::SURF::TIMG *chk = new FORM::SURF::TIMG;
				chk->image = read_VX(it);
				return chk;
			}

			if (tag == "TBLR") {
				FORM::SURF::TBLR *chk = new FORM::SURF::TBLR;
				chk->blur_percentage = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "CLRH") {
				FORM::SURF::CLRH *chk = new FORM::SURF::CLRH;
				chk->color_highlights = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "CLRF") {
				FORM::SURF::CLRF *chk = new FORM::SURF::CLRF;
				chk->color_filter = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "ADTR") {
				FORM::SURF::ADTR *chk = new FORM::SURF::ADTR;
				chk->additive = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "GLOW") {
				FORM::SURF::GLOW *chk = new FORM::SURF::GLOW;
				chk->type = read_U2(it);
				chk->intensity = read_F4(it);
				chk->intensity_envelope = read_VX(it);
				chk->size = read_F4(it);
				chk->size_envelope = read_VX(it);
				return chk;
			}

			if (tag == "LINE") {
				FORM::SURF::LINE *chk = new FORM::SURF::LINE;
				chk->flags = read_U2(it);
				if (it < end) {
					chk->size = read_F4(it);
					chk->size_envelope = read_VX(it);
					if (it < end) {
						chk->color = read_COL12(it);
						chk->color_envelope = read_VX(it);
					}
				}
				return chk;
			}

			if (tag == "ALPH") {
				FORM::SURF::ALPH *chk = new FORM::SURF::ALPH;
				chk->mode = read_U2(it);
				chk->value = read_FP4(it);
				return chk;
			}

			if (tag == "VCOL") {
				FORM::SURF::VCOL *chk = new FORM::SURF::VCOL;
				chk->intensity = read_FP4(it);
				chk->envelope = read_VX(it);
				chk->vmap_type = read_ID4(it);
				chk->name = read_S0(it);
				return chk;
			}

			// surface blocks

			if (tag == "BLOK") {
				FORM::SURF::BLOK *chk = new FORM::SURF::BLOK;
				std::string hid;
				for (Iter tempit=it; tempit<(it+4); ++tempit) hid += *tempit;
				chk->header = parse_subchunk(it, "FORM::SURF::BLOK");
				while (it < end) {
					chk->attributes.push_back(parse_subchunk(it, "FORM::SURF::BLOK::" + hid));
				}
				return chk;
			}

		}

		if (context == "FORM::SURF::BLOK") {	// block headers

			if (tag == "IMAP") {
				FORM::SURF::BLOK::IMAP *chk = new FORM::SURF::BLOK::IMAP;
				chk->ordinal = read_S0(it);
				while (it < end) {
					chk->block_attributes.push_back(parse_subchunk(it, "FORM::SURF::BLOK"));
				}
				return chk;
			}

			if (tag == "PROC") {
				FORM::SURF::BLOK::PROC *chk = new FORM::SURF::BLOK::PROC;
				chk->ordinal = read_S0(it);
				while (it < end) {
					chk->block_attributes.push_back(parse_subchunk(it, "FORM::SURF::BLOK"));
				}
				return chk;
			}

			if (tag == "GRAD") {
				FORM::SURF::BLOK::GRAD *chk = new FORM::SURF::BLOK::GRAD;
				chk->ordinal = read_S0(it);
				while (it < end) {
					chk->block_attributes.push_back(parse_subchunk(it, "FORM::SURF::BLOK"));
				}
				return chk;
			}

			if (tag == "SHDR") {
				FORM::SURF::BLOK::SHDR *chk = new FORM::SURF::BLOK::SHDR;
				chk->ordinal = read_S0(it);
				while (it < end) {
					chk->block_attributes.push_back(parse_subchunk(it, "FORM::SURF::BLOK"));
				}
				return chk;
			}

		}

		if (context == "FORM::SURF::BLOK") {	// attributes common to all block headers

			if (tag == "CHAN") {
				FORM::SURF::BLOK::CHAN *chk = new FORM::SURF::BLOK::CHAN;
				chk->texture_channel = read_ID4(it);
				return chk;
			}

			if (tag == "ENAB") {
				FORM::SURF::BLOK::ENAB *chk = new FORM::SURF::BLOK::ENAB;
				chk->enable = read_U2(it);
				return chk;
			}

			if (tag == "OPAC") {
				FORM::SURF::BLOK::OPAC *chk = new FORM::SURF::BLOK::OPAC;
				chk->type = read_U2(it);
				chk->opacity = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "AXIS") {
				FORM::SURF::BLOK::AXIS *chk = new FORM::SURF::BLOK::AXIS;
				chk->displacement_axis = read_U2(it);
				return chk;
			}

		}

		if (context == "FORM::SURF::BLOK::IMAP" || context == "FORM::SURF::BLOK::PROC") {

			if (tag == "TMAP") {
				FORM::SURF::BLOK::IMAP::TMAP *chk = new FORM::SURF::BLOK::IMAP::TMAP;
				while (it < end) chk->attributes.push_back(parse_subchunk(it, "FORM::SURF::BLOK::IMAP/PROC::TMAP"));
				return chk;
			}

		}

		if (context == "FORM::SURF::BLOK::IMAP/PROC::TMAP") {

			if (tag == "CNTR") {
				FORM::SURF::BLOK::IMAP::TMAP::CNTR *chk = new FORM::SURF::BLOK::IMAP::TMAP::CNTR;
				chk->vector = read_VEC12(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "SIZE") {
				FORM::SURF::BLOK::IMAP::TMAP::SIZE *chk = new FORM::SURF::BLOK::IMAP::TMAP::SIZE;
				chk->vector = read_VEC12(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "ROTA") {
				FORM::SURF::BLOK::IMAP::TMAP::ROTA *chk = new FORM::SURF::BLOK::IMAP::TMAP::ROTA;
				chk->vector = read_VEC12(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "OREF") {
				FORM::SURF::BLOK::IMAP::TMAP::OREF *chk = new FORM::SURF::BLOK::IMAP::TMAP::OREF;
				chk->object_name = read_S0(it);
				return chk;
			}

			if (tag == "FALL") {
				FORM::SURF::BLOK::IMAP::TMAP::FALL *chk = new FORM::SURF::BLOK::IMAP::TMAP::FALL;
				chk->type = read_U2(it);
				chk->vector = read_VEC12(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "CSYS") {
				FORM::SURF::BLOK::IMAP::TMAP::CSYS *chk = new FORM::SURF::BLOK::IMAP::TMAP::CSYS;
				chk->type = read_U2(it);
				return chk;
			}

		}

		if (context == "FORM::SURF::BLOK::IMAP") {

			if (tag == "PROJ") {
				FORM::SURF::BLOK::IMAP::PROJ *chk = new FORM::SURF::BLOK::IMAP::PROJ;
				chk->projection_mode = read_U2(it);
				return chk;
			}

			if (tag == "AXIS") {
				FORM::SURF::BLOK::IMAP::AXIS *chk = new FORM::SURF::BLOK::IMAP::AXIS;
				chk->texture_axis = read_U2(it);
				return chk;
			}

			if (tag == "IMAG") {
				FORM::SURF::BLOK::IMAP::IMAG *chk = new FORM::SURF::BLOK::IMAP::IMAG;
				chk->texture_image = read_VX(it);
				return chk;
			}

			if (tag == "WRAP") {
				FORM::SURF::BLOK::IMAP::WRAP *chk = new FORM::SURF::BLOK::IMAP::WRAP;
				chk->width_wrap = read_U2(it);
				chk->height_wrap = read_U2(it);
				return chk;
			}

			if (tag == "WRPW") {
				FORM::SURF::BLOK::IMAP::WRPW *chk = new FORM::SURF::BLOK::IMAP::WRPW;
				chk->cycles = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "WRPH") {
				FORM::SURF::BLOK::IMAP::WRPH *chk = new FORM::SURF::BLOK::IMAP::WRPH;
				chk->cycles = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

			if (tag == "VMAP") {
				FORM::SURF::BLOK::IMAP::VMAP *chk = new FORM::SURF::BLOK::IMAP::VMAP;
				chk->txuv_map_name = read_S0(it);
				return chk;
			}

			if (tag == "AAST") {
				FORM::SURF::BLOK::IMAP::AAST *chk = new FORM::SURF::BLOK::IMAP::AAST;
				chk->flags = read_U2(it);
				chk->antialiasing_strength = read_FP4(it);
				return chk;
			}

			if (tag == "PIXB") {
				FORM::SURF::BLOK::IMAP::PIXB *chk = new FORM::SURF::BLOK::IMAP::PIXB;
				chk->flags = read_U2(it);
				return chk;
			}

			if (tag == "STCK") {
				FORM::SURF::BLOK::IMAP::STCK *chk = new FORM::SURF::BLOK::IMAP::STCK;
				chk->on_off = read_U2(it);
				chk->time = read_FP4(it);
				return chk;
			}

			if (tag == "TAMP") {
				FORM::SURF::BLOK::IMAP::TAMP *chk = new FORM::SURF::BLOK::IMAP::TAMP;
				chk->amplitude = read_FP4(it);
				chk->envelope = read_VX(it);
				return chk;
			}

		}

		if (context == "FORM::SURF::BLOK::PROC") {

			if (tag == "AXIS") {
				FORM::SURF::BLOK::PROC::AXIS *chk = new FORM::SURF::BLOK::PROC::AXIS;
				chk->axis = read_U2(it);
				return chk;
			}

			if (tag == "VALU") {
				FORM::SURF::BLOK::PROC::VALU *chk = new FORM::SURF::BLOK::PROC::VALU;
				while (it < end) chk->value.push_back(read_FP4(it));
				return chk;
			}

			if (tag == "FUNC") {
				FORM::SURF::BLOK::PROC::FUNC *chk = new FORM::SURF::BLOK::PROC::FUNC;
				chk->algorithm_name = read_S0(it);
				while (it < end) chk->data.push_back(read_U1(it));
				return chk;
			}

		}

		if (context == "FORM::SURF::BLOK::GRAD") {

			if (tag == "PNAM") {
				FORM::SURF::BLOK::GRAD::PNAM *chk = new FORM::SURF::BLOK::GRAD::PNAM;
				chk->parameter = read_S0(it);
				return chk;
			}

			if (tag == "INAM") {
				FORM::SURF::BLOK::GRAD::INAM *chk = new FORM::SURF::BLOK::GRAD::INAM;
				chk->item_name = read_S0(it);
				return chk;
			}

			if (tag == "GRST") {
				FORM::SURF::BLOK::GRAD::GRST *chk = new FORM::SURF::BLOK::GRAD::GRST;
				chk->input_range = read_FP4(it);
				return chk;
			}

			if (tag == "GREN") {
				FORM::SURF::BLOK::GRAD::GREN *chk = new FORM::SURF::BLOK::GRAD::GREN;
				chk->input_range = read_FP4(it);
				return chk;
			}

			if (tag == "GRPT") {
				FORM::SURF::BLOK::GRAD::GRPT *chk = new FORM::SURF::BLOK::GRAD::GRPT;
				chk->repeat_mode = read_U2(it);
				return chk;
			}

			if (tag == "FKEY") {
				FORM::SURF::BLOK::GRAD::FKEY *chk = new FORM::SURF::BLOK::GRAD::FKEY;
				while (it < end) {
					FORM::SURF::BLOK::GRAD::FKEY::value_type val;
					val.input = read_FP4(it);
					for (int i=0; i<4; ++i) val.output[i] = read_FP4(it);
					chk->values.push_back(val);
				}
				return chk;
			}

			if (tag == "IKEY") {
				FORM::SURF::BLOK::GRAD::IKEY *chk = new FORM::SURF::BLOK::GRAD::IKEY;
				while (it < end) chk->interpolation.push_back(read_U2(it));
				return chk;
			}

		}

		if (context == "FORM::SURF::BLOK::SHDR") {

			if (tag == "FUNC") {
				FORM::SURF::BLOK::SHDR::FUNC *chk = new FORM::SURF::BLOK::SHDR::FUNC;
				chk->algorithm_name = read_S0(it);
				while (it < end) chk->data.push_back(read_U1(it));
				return chk;
			}

		}

		return 0;
	}

	LP_TMP iff::Chunk *Parser<Iter>::parse_subchunk(Iter &it, const std::string &context)
	{
		std::string tag;
		for (int i=0; i<4; ++i) tag += *(it++);
		unsigned int len = ((static_cast<unsigned int>(*(it++)) & 0xFF) << 8) |
			(static_cast<unsigned int>(*(it++)) & 0xFF);
		this->os() << "DEBUG INFO: lwo2parser: reading subchunk " << tag << ", length = " << len << ", context = " << context << "\n";
		iff::Chunk *chk = parse_chunk_data(tag, context, it, it+len);
		if (!chk) this->os() << "DEBUG INFO: lwo2parser: \tprevious subchunk not handled\n";
		it += len;
		if ((len % 2) != 0) ++it;
		return chk;
	}


}

#endif
