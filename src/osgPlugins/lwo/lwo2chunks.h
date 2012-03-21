
/****************************************************************************

	Chunk definitions for the LWO2 file format

	Copyright (C) 2002 by Marco Jez

****************************************************************************/

#ifndef LWO2CHUNKS_
#define LWO2CHUNKS_

#ifdef _MSC_VER
#	pragma warning ( disable : 4786 )
#endif

#include <vector>

#include "iffparser.h"
#include "lwo2types.h"

#define CHUNK(name)				struct name: public iff::Chunk
#define SUBCHUNK				CHUNK
#define FREE_ON_DESTROY(list)	iff::Chunk_list *free_on_destroy() { return &list; }

namespace lwo2
{

	typedef iff::Chunk SubChunk;

	CHUNK (FORM) {

		ID4 type;
		iff::Chunk_list data;

		FREE_ON_DESTROY(data);

		CHUNK (LAYR) {
			U2 number;
			U2 flags;
			VEC12 pivot;
			S0 name;
			I2 parent;
		};

		CHUNK (PNTS) {
			typedef std::vector<VEC12> Point_list;
			Point_list point_location;
		};

		CHUNK (VMAP) {
			ID4 type;
			U2 dimension;
			S0 name;

			struct mapping_type {
				VX vert;
				std::vector<F4> value;
			};

			typedef std::vector<mapping_type> Mapping_list;
			Mapping_list mapping;
		};

		CHUNK (POLS) {
			ID4 type;

			struct polygon_type {
				U2 numvert;
				U2 flags;
				std::vector<VX> vert;
			};

			typedef std::vector<polygon_type> Polygon_list;
			Polygon_list polygons;
		};

		CHUNK (TAGS) {
			typedef std::vector<S0> String_list;
			String_list tag_string;
		};

		CHUNK (PTAG) {
			ID4 type;

			struct mapping_type {
				VX poly;
				U2 tag;
			};

			typedef std::vector<mapping_type> Mapping_list;
			Mapping_list mapping;
		};

		CHUNK (VMAD) {
			ID4 type;
			U2 dimension;
			S0 name;

			struct mapping_type {
				VX vert;
				VX poly;
				std::vector<F4> value;
			};

			typedef std::vector<mapping_type> Mapping_list;
			Mapping_list mapping;
		};

		CHUNK (ENVL) {
			VX index;
			iff::Chunk_list attributes;

			FREE_ON_DESTROY(attributes);

			SUBCHUNK (TYPE) {
				U1 user_format;
				U1 type;
			};

			SUBCHUNK (PRE) {
				U2 type;
			};

			SUBCHUNK (POST) {
				U2 type;
			};

			SUBCHUNK (KEY) {
				F4 time;
				F4 value;
			};

			SUBCHUNK (SPAN) {
				ID4 type;
				std::vector<F4> parameters;
			};

			SUBCHUNK (CHAN) {
				S0 server_name;
				U2 flags;
				std::vector<U1> data;
			};

			SUBCHUNK (NAME) {
				S0 channel_name;
			};

		};

		CHUNK (CLIP) {
			U4 index;
			iff::Chunk_list attributes;

			FREE_ON_DESTROY(attributes);

			SUBCHUNK (STIL) {
				FNAM0 name;
			};

			SUBCHUNK (ISEQ) {
				U1 num_digits;
				U1 flags;
				I2 offset;
				U2 reserved;
				I2 start;
				I2 end;
				FNAM0 prefix;
				S0 suffix;
			};

			SUBCHUNK (ANIM) {
				FNAM0 filename;
				S0 server_name;
				U2 flags;
				std::vector<U1> data;
			};

			SUBCHUNK (XREF) {
				U4 index;
				S0 string;
			};

			SUBCHUNK (STCC) {
				I2 lo;
				I2 hi;
				FNAM0 name;
			};

			SUBCHUNK (TIME) {
				FP4 start_time;
				FP4 duration;
				FP4 frame_rate;
			};

			SUBCHUNK (CONT) {
				FP4 contrast_delta;
				VX envelope;
			};

			SUBCHUNK (BRIT) {
				FP4 brightness_delta;
				VX envelope;
			};

			SUBCHUNK (SATR) {
				FP4 saturation_delta;
				VX envelope;
			};

			SUBCHUNK (HUE) {
				FP4 hue_rotation;
				VX envelope;
			};

			SUBCHUNK (GAMM) {
				F4 gamma;
				VX envelope;
			};

			SUBCHUNK (NEGA) {
				U2 enable;
			};

			SUBCHUNK (IFLT) {
				S0 server_name;
				U2 flags;
				std::vector<U1> data;
			};

			SUBCHUNK (PFLT) {
				S0 server_name;
				U2 flags;
				std::vector<U1> data;
			};

		};

		CHUNK (SURF) {
			S0 name;
			S0 source;
			iff::Chunk_list attributes;

			FREE_ON_DESTROY(attributes);

			SUBCHUNK (COLR) {
				COL12 base_color;
				VX envelope;
			};

			SUBCHUNK (DIFF) {
				FP4 intensity;
				VX envelope;
			};

			SUBCHUNK (LUMI) {
				FP4 intensity;
				VX envelope;
			};

			SUBCHUNK (SPEC) {
				FP4 intensity;
				VX envelope;
			};

			SUBCHUNK (REFL) {
				FP4 intensity;
				VX envelope;
			};

			SUBCHUNK (TRAN) {
				FP4 intensity;
				VX envelope;
			};

			SUBCHUNK (TRNL) {
				FP4 intensity;
				VX envelope;
			};

			SUBCHUNK (GLOS) {
				FP4 glossiness;
				VX envelope;
			};

			SUBCHUNK (SHRP) {
				FP4 sharpness;
				VX envelope;
			};

			SUBCHUNK (BUMP) {
				FP4 strength;
				VX envelope;
			};

			SUBCHUNK (SIDE) {
				U2 sidedness;
			};

			SUBCHUNK (SMAN) {
				ANG4 max_smoothing_angle;
			};

			SUBCHUNK (RFOP) {
				U2 reflection_options;
			};

			SUBCHUNK (RIMG) {
				VX image;
			};

			SUBCHUNK (RSAN) {
				ANG4 seam_angle;
				VX envelope;
			};

			SUBCHUNK (RBLR) {
				FP4 blur_percentage;
				VX envelope;
			};

			SUBCHUNK (RIND) {
				F4 refractive_index;
				VX envelope;
			};

			SUBCHUNK (TROP) {
				U2 transparency_options;
			};

			SUBCHUNK (TIMG) {
				VX image;
			};

			SUBCHUNK (TBLR) {
				FP4 blur_percentage;
				VX envelope;
			};

			SUBCHUNK (CLRH) {
				FP4 color_highlights;
				VX envelope;
			};

			SUBCHUNK (CLRF) {
				FP4 color_filter;
				VX envelope;
			};

			SUBCHUNK (ADTR) {
				FP4 additive;
				VX envelope;
			};

			SUBCHUNK (GLOW) {
				U2 type;
				F4 intensity;
				VX intensity_envelope;
				F4 size;
				VX size_envelope;
			};

			SUBCHUNK (LINE) {
				U2 flags;
				F4 size;
				VX size_envelope;
				COL12 color;
				VX color_envelope;
			};

			SUBCHUNK (ALPH) {
				U2 mode;
				FP4 value;
			};

			SUBCHUNK (VCOL) {
				FP4 intensity;
				VX envelope;
				ID4 vmap_type;
				S0 name;
			};

			// surface blocks

			SUBCHUNK (BLOK) {
				SubChunk *header;
				iff::Chunk_list attributes;

				FREE_ON_DESTROY(attributes);

				// block headers

				SUBCHUNK (IMAP) {
					S0 ordinal;
					iff::Chunk_list block_attributes;	// common attributes (see above)

					FREE_ON_DESTROY(block_attributes);

					// attributes specific to IMAP and PROC

					SUBCHUNK (TMAP) {
						iff::Chunk_list attributes;

						FREE_ON_DESTROY(attributes);

						SUBCHUNK (CNTR) {
							VEC12 vector;
							VX envelope;
						};

						SUBCHUNK (SIZE) {
							VEC12 vector;
							VX envelope;
						};

						SUBCHUNK (ROTA) {
							VEC12 vector;
							VX envelope;
						};

						SUBCHUNK (OREF) {
							S0 object_name;
						};

						SUBCHUNK (FALL) {
							U2 type;
							VEC12 vector;
							VX envelope;
						};

						SUBCHUNK (CSYS) {
							U2 type;
						};

					};

					// attributes specific to image maps

					SUBCHUNK (PROJ) {
						U2 projection_mode;
					};

					SUBCHUNK (AXIS) {
						U2 texture_axis;
					};

					SUBCHUNK (IMAG) {
						VX texture_image;
					};

					SUBCHUNK (WRAP) {
						U2 width_wrap;
						U2 height_wrap;
					};

					SUBCHUNK (WRPW) {
						FP4 cycles;
						VX envelope;
					};

					SUBCHUNK (WRPH) {
						FP4 cycles;
						VX envelope;
					};

					SUBCHUNK (VMAP) {
						S0 txuv_map_name;
					};

					SUBCHUNK (AAST) {
						U2 flags;
						FP4 antialiasing_strength;
					};

					SUBCHUNK (PIXB) {
						U2 flags;
					};

					SUBCHUNK (STCK) {
						U2 on_off;
						FP4 time;
					};

					SUBCHUNK (TAMP) {
						FP4 amplitude;
						VX envelope;
					};

				};

				SUBCHUNK (PROC) {
					S0 ordinal;
					iff::Chunk_list block_attributes;

					FREE_ON_DESTROY(block_attributes);

					typedef IMAP::TMAP	TMAP;

					// attributes specific to procedural textures

					SUBCHUNK (AXIS) {
						U2 axis;
					};

					SUBCHUNK (VALU) {
						std::vector<FP4> value;
					};

					SUBCHUNK (FUNC) {
						S0 algorithm_name;
						std::vector<U1> data;
					};

				};

				SUBCHUNK (GRAD) {
					S0 ordinal;
					iff::Chunk_list block_attributes;

					FREE_ON_DESTROY(block_attributes);

					// attributes specific to gradient textures

					SUBCHUNK (PNAM) {
						S0 parameter;
					};

					SUBCHUNK (INAM) {
						S0 item_name;
					};

					SUBCHUNK (GRST) {
						FP4 input_range;
					};

					SUBCHUNK (GREN) {
						FP4 input_range;
					};

					SUBCHUNK (GRPT) {
						U2 repeat_mode;
					};

					SUBCHUNK (FKEY) {
						struct value_type {
							FP4 input;
							FP4 output[4];
						};

						typedef std::vector<value_type> Value_list;
						Value_list values;
					};

					SUBCHUNK (IKEY) {
						std::vector<U2> interpolation;
					};

				};

				SUBCHUNK (SHDR) {
					S0 ordinal;
					iff::Chunk_list block_attributes;

					FREE_ON_DESTROY(block_attributes);

					// attributes specific to shaders

					SUBCHUNK (FUNC) {
						S0 algorithm_name;
						std::vector<U1> data;
					};

				};

				// attributes common to all header types

				SUBCHUNK (CHAN) {
					ID4 texture_channel;
				};

				SUBCHUNK (ENAB) {
					U2 enable;
				};

				SUBCHUNK (OPAC) {
					U2 type;
					FP4 opacity;
					VX envelope;
				};

				SUBCHUNK (AXIS) {
					U2 displacement_axis;
				};

			};

		};

		CHUNK (BBOX) {
			VEC12 min;
			VEC12 max;
		};

		CHUNK (DESC) {
			S0 description_line;
		};

		CHUNK (TEXT) {
			S0 comment;
		};

		CHUNK (ICON) {
			U2 encoding;
			U2 width;
			std::vector<U1> data;
		};

	};


}

#endif
