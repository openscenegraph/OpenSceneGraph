/*
 * ReaderWriterMD2.cpp
 *
 * MD2 Reading code
 *
 * Author(s):  Vladimir Vukicevic <vladimir@pobox.com>
 *
 */

#include <osg/TexEnv>
#include <osg/CullFace>

#include <osg/Geode>
#include <osg/Geometry>
#include <osg/Material>
#include <osg/Image>
#include <osg/Texture2D>

#include <osg/Switch>
#include <osg/Sequence>

#include <osg/Notify>
#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileNameUtils>
#include <osgDB/FileUtils>

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>

#if defined(WIN32) && !defined(__CYGWIN__)
#  include <io.h>
#else
#  include <unistd.h>
#endif

#include <sys/stat.h>

#include <assert.h>

static osg::Node* load_md2 (const char *filename, const osgDB::ReaderWriter::Options* options);

class ReaderWriterMD2 : public osgDB::ReaderWriter
{
public:
    ReaderWriterMD2 ()
    {
        supportsExtension("md2","Quak2 MD format");
    }

    virtual const char* className () const {
        return "Quake MD2 Reader";
    }

    virtual ReadResult readNode (const std::string& filename, const osgDB::ReaderWriter::Options* options) const;
};

REGISTER_OSGPLUGIN(md2, ReaderWriterMD2)

osgDB::ReaderWriter::ReadResult
ReaderWriterMD2::readNode (const std::string& file, const osgDB::ReaderWriter::Options* options) const
{
    std::string ext = osgDB::getLowerCaseFileExtension(file);
    if (!acceptsExtension(ext)) return ReadResult::FILE_NOT_HANDLED;

    std::string fileName = osgDB::findDataFile( file, options );
    if (fileName.empty()) return ReadResult::FILE_NOT_FOUND;

    // code for setting up the database path so that internally referenced file are searched for on relative paths. 
    osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
    local_opt->setDatabasePath(osgDB::getFilePath(fileName));

    return load_md2 (fileName.c_str(), options);
}


/////////////////// MD2 parsing code //////////////////////

typedef struct {
    int magic;
    int version;
    int skinWidth;
    int skinHeight;
    int frameSize;                  // size of each frame in bytes
    int numSkins;
    int numVertices;                // number of vertices in each frame
    int numTexcoords;               // number of texcoords in each frame (usually same as numVertices)
    int numTriangles;               // number of triangles in each frame
    int numGlCommands;
    int numFrames;                  // number of frames
    int offsetSkins;
    int offsetTexCoords;
    int offsetTriangles;
    int offsetFrames;
    int offsetGlCommands;           // num dwords in gl commands list
    int offsetEnd;
} MD2_HEADER;

#define MD2_HEADER_MAGIC 0x32504449

typedef struct {
    unsigned char vertex[3];
    unsigned char lightNormalIndex;
} MD2_VERTEX;

typedef struct {
    float scale[3];
    float translate[3];
    char name[16];
    MD2_VERTEX vertices[1];
} MD2_FRAME;

typedef struct {
    short vertexIndices[3];
    short textureIndices[3];
} MD2_TRIANGLE;

typedef struct {
    float s;
    float t;
    int vertexIndex;
} MD2_GLCOMMANDVERTEX;

typedef struct {
    short s;
    short t;
} MD2_TEXTURECOORDINATE;

typedef struct {
    char name[64];
} MD2_SKIN;

#define NUMVERTEXNORMALS 162
float g_md2VertexNormals[NUMVERTEXNORMALS][3] = {
#include "anorms.h"
};

osg::Vec3Array *g_md2NormalsArray = NULL;

//
// the result will be an osg::Switch
// whose children will be osg::Sequences, one for each animation
// the children of the osg::Sequence will be the osg::Geode that contains the
// osg::Geometry of the relevant frame.  The osg::Geode will have a osg::StateSet
// containing the texture information.
//

// this is also quite non-portable to non-little-endian architectures

static osg::Node*
load_md2 (const char *filename, const osgDB::ReaderWriter::Options* options)
{
    struct stat st;
    void *mapbase;
//    void *p;
    int file_fd;
    osg::Node* result = NULL;

    if (stat (filename, &st) < 0) {
        return NULL;
    }

    file_fd = open (filename, O_RDONLY);
    if (file_fd <= 0) {
        return NULL;
    }

#if 0
    mapbase = mmap (NULL, st.st_size, PROT_READ, MAP_SHARED, file_fd, 0);
    if (mapbase == NULL) {
        close (file_fd);
        return NULL;
    }
#else
    mapbase = malloc (st.st_size);
    if (read(file_fd, mapbase, st.st_size)==0)
    {
        close (file_fd);
        return NULL;
    }
#endif

    if (g_md2NormalsArray == NULL) {
        g_md2NormalsArray = new osg::Vec3Array;
        for (int i = 0; i < NUMVERTEXNORMALS; i++)
            g_md2NormalsArray->push_back (osg::Vec3 (g_md2VertexNormals[i][0], g_md2VertexNormals[i][1], g_md2VertexNormals[i][2]));
    }


    MD2_HEADER *md2_header = (MD2_HEADER *) mapbase;
    if (md2_header->magic != MD2_HEADER_MAGIC || md2_header->version != 8) {
#if 0
        munmap (mapbase);
#else
        free (mapbase);
#endif
        close (file_fd);
        return NULL;
    }

    MD2_SKIN *md2_skins = (MD2_SKIN *) ((unsigned char *) mapbase + md2_header->offsetSkins);
    MD2_TEXTURECOORDINATE *md2_texcoords = (MD2_TEXTURECOORDINATE *) ((unsigned char *) mapbase + md2_header->offsetTexCoords);
    MD2_TRIANGLE *md2_triangles = (MD2_TRIANGLE *) ((unsigned char *) mapbase + md2_header->offsetTriangles);

    osg::Switch *base_switch = new osg::Switch ();
    osg::Sequence *current_sequence = NULL;

    // read in the frame info into a vector
    const char *last_frame_name = NULL;

    osg::Vec3Array *vertexCoords = NULL;
    osg::Vec2Array *texCoords = NULL;
    osg::UIntArray *vertexIndices = NULL;
    osg::UIntArray *texIndices = NULL;
    osg::Vec3Array *normalCoords = NULL;
    osg::UIntArray *normalIndices = NULL;

    // load the texture skins

    // there is code here to support multiple skins, but there's no need for it
    // since we really just want to support one; we have no way of returning more
    // than one to the user in any case.
#if 0
    std::vector<osg::Texture2D*> skin_textures;

    for (int si = 0; si < md2_header->numSkins; si++)
    {
        osg::ref_ptr<osg::Image> img;
        osg::ref_ptr<osg::Texture2D> tex;
        std::string imgname (md2_skins[si].name);

        // first try loading the imgname straight
        img = osgDB::readRefImageFile (imgname, options);
        if (img.valid()) {
            tex = new osg::Texture2D;
            tex->setImage (img);
            skin_textures.push_back (tex);
            continue;
        }

        // we failed, so check if it's a PCX image
        if (imgname.size() > 4 &&
            osgDB::equalCaseInsensitive (imgname.substr (imgname.size() - 3, 3), "pcx"))
        {
            // it's a pcx, so try bmp and tga, since pcx sucks
            std::string basename = imgname.substr (0, imgname.size() - 3);
            img = osgDB::readRefImageFile (basename + "bmp", options);
            if (img.valid()) {
                tex = new osg::Texture2D;
                tex->setImage (img.get());
                skin_textures.push_back (tex);
                continue;
            }

            img = osgDB::readRefImageFile (basename + "tga", options);
            if (img.valid()) {
                tex = new osg::Texture2D;
                tex->setImage (img,get());
                skin_textures.push_back (tex);
                continue;
            }
        }

        // couldn't find the referenced texture skin for this model
        skin_textures.push_back (NULL);
        osg::notify(osg::WARN) << "MD2 Loader: Couldn't load skin " << imgname << " referenced by model " << filename << std::endl;
    }
#else
    // load the single skin
    osg::ref_ptr<osg::Image> skin_image;
    osg::ref_ptr<osg::Texture2D> skin_texture = NULL;

    if (md2_header->numSkins > 0) {
        std::string imgname (md2_skins[0].name);

        do {
            // first try loading the imgname straight
            skin_image = osgDB::readRefImageFile(imgname, options);
            if (skin_image.valid()) break;

            // we failed, so check if it's a PCX image
            if (imgname.size() > 4 &&
                osgDB::equalCaseInsensitive (imgname.substr (imgname.size() - 3, 3), "pcx"))
                {
                // it's a pcx, so try bmp and tga, since pcx sucks
                std::string basename = imgname.substr (0, imgname.size() - 3);
                skin_image = osgDB::readRefImageFile (basename + "bmp", options);
                if (skin_image.valid()) break;

                skin_image = osgDB::readRefImageFile (basename + "tga", options);
                if (skin_image.valid()) break;
            }
        } while (0);

        if (skin_image.valid()) {
            skin_texture = new osg::Texture2D;
            skin_texture->setImage (skin_image.get());
            skin_texture->setFilter (osg::Texture2D::MAG_FILTER, osg::Texture2D::NEAREST);
        } else {
            // couldn't find the referenced texture skin for this model
            osg::notify(osg::WARN) << "MD2 Loader: Couldn't load skin " << imgname << " referenced by model " << filename << std::endl;
        }
    }

#endif

    int sequence_frame = 0;

    for (int curFrame = 0; curFrame < md2_header->numFrames; curFrame++) {
        //        std::cerr << "Num vertices " << md2_header->numVertices << std::endl;

        //long *command = (long *) ((unsigned char *) mapbase + md2_header->offsetGlCommands);

        MD2_FRAME *frame = (MD2_FRAME *) ((unsigned char *) mapbase + md2_header->offsetFrames + (md2_header->frameSize * curFrame));
        MD2_VERTEX *frame_vertices = frame->vertices;


        //        std::cerr << "Reading frame " << curFrame << " (gl offset: " << md2_header->offsetGlCommands << ") name: " << frame->name << std::endl;

        int last_len = last_frame_name ? strcspn (last_frame_name, "0123456789") : 0;
        int cur_len = strcspn (frame->name, "0123456789");

        if (last_len != cur_len || strncmp (last_frame_name, frame->name, last_len) != 0) {
            if (current_sequence) {
                current_sequence->setInterval (osg::Sequence::LOOP, 0, -1);
                base_switch->addChild (current_sequence);
            }

            current_sequence = new osg::Sequence ();
            current_sequence->setMode (osg::Sequence::START);
            current_sequence->setDuration (1.0f, -1);
            sequence_frame = 0;

            current_sequence->setName (std::string (frame->name, cur_len));
        }

        vertexCoords = new osg::Vec3Array;
        normalCoords = new osg::Vec3Array;

        for (int vi = 0; vi < md2_header->numVertices; vi++) {
            vertexCoords->push_back
                (osg::Vec3
                 (frame_vertices[vi].vertex[0] * frame->scale[0] + frame->translate[0],
                  -1 * (frame_vertices[vi].vertex[2] * frame->scale[2] + frame->translate[2]),
                  frame_vertices[vi].vertex[1] * frame->scale[1] + frame->translate[1]));
            osg::Vec3 z = (*g_md2NormalsArray) [frame_vertices[vi].lightNormalIndex];
            normalCoords->push_back (z);
        }

        if (curFrame == 0) {
            vertexIndices = new osg::UIntArray;
            normalIndices = new osg::UIntArray;

            texCoords = new osg::Vec2Array;
            texIndices = new osg::UIntArray;

            for (int vi = 0; vi < md2_header->numTexcoords; vi++) {
                texCoords->push_back
                    (osg::Vec2 ((float) md2_texcoords[vi].s / md2_header->skinWidth,
                                1.0f - (float) md2_texcoords[vi].t / md2_header->skinHeight));
            }

            for (int ti = 0; ti < md2_header->numTriangles; ti++) {
                vertexIndices->push_back (md2_triangles[ti].vertexIndices[0]);
                vertexIndices->push_back (md2_triangles[ti].vertexIndices[1]);
                vertexIndices->push_back (md2_triangles[ti].vertexIndices[2]);

                normalIndices->push_back (md2_triangles[ti].vertexIndices[0]);
                normalIndices->push_back (md2_triangles[ti].vertexIndices[1]);
                normalIndices->push_back (md2_triangles[ti].vertexIndices[2]);

                texIndices->push_back (md2_triangles[ti].textureIndices[0]);
                texIndices->push_back (md2_triangles[ti].textureIndices[1]);
                texIndices->push_back (md2_triangles[ti].textureIndices[2]);
            }
        }

        osg::Geometry *geom = new osg::Geometry;

        geom->setVertexArray (vertexCoords);
        geom->setVertexIndices (vertexIndices);

        geom->setTexCoordArray (0, texCoords);
        geom->setTexCoordIndices (0, texIndices);

        geom->setNormalArray (normalCoords);
        geom->setNormalIndices (normalIndices);
        geom->setNormalBinding (osg::Geometry::BIND_PER_VERTEX);

        geom->addPrimitiveSet (new osg::DrawArrays (osg::PrimitiveSet::TRIANGLES, 0, vertexIndices->size ()));

        osg::Geode *geode = new osg::Geode;
        geode->addDrawable (geom);

        current_sequence->addChild (geode);
        current_sequence->setTime (sequence_frame, 0.2f);
        sequence_frame++;

        last_frame_name = frame->name;
    }

    if (current_sequence) {
        current_sequence->setInterval (osg::Sequence::LOOP, 0, -1);
        base_switch->addChild (current_sequence);
    }

    osg::StateSet *state = new osg::StateSet;

    if (skin_texture != NULL) {
        state->setTextureAttributeAndModes (0, skin_texture.get(), osg::StateAttribute::ON);
    }
    base_switch->setStateSet (state);

    //base_switch->setAllChildrenOff ();

    base_switch->setSingleChildOn(0);

    result = base_switch;
    
#if 0
    munamp (mapbase);
#else
    free (mapbase);
#endif
    close (file_fd);
    return result;
}
