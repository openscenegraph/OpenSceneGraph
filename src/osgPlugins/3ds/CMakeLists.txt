# List of C files to be compiled as C++ (else CMake sets ".c" to be compiled as pure C)
SET(C_FILES
    lib3ds/lib3ds_io.c        # Modified to support OSG endianness
)

SET(TARGET_SRC
    ReaderWriter3DS.cpp
    WriterNodeVisitor.cpp
    WriterCompareTriangle.cpp

    ${C_FILES}
    lib3ds/lib3ds_atmosphere.c
    lib3ds/lib3ds_background.c
    lib3ds/lib3ds_camera.c
    lib3ds/lib3ds_chunk.c
    lib3ds/lib3ds_chunktable.c
    lib3ds/lib3ds_file.c
    lib3ds/lib3ds_light.c
    lib3ds/lib3ds_material.c
    lib3ds/lib3ds_math.c
    lib3ds/lib3ds_matrix.c
    lib3ds/lib3ds_mesh.c
    lib3ds/lib3ds_node.c
    lib3ds/lib3ds_quat.c
    lib3ds/lib3ds_shadow.c
    lib3ds/lib3ds_track.c
    lib3ds/lib3ds_util.c
    lib3ds/lib3ds_vector.c
    lib3ds/lib3ds_viewport.c
)
SET(TARGET_H
    WriterNodeVisitor.h
    WriterCompareTriangle.h
    lib3ds/lib3ds.h
    lib3ds/lib3ds_impl.h
)
#### end var setup  ###
SETUP_PLUGIN(3ds)
ADD_DEFINITIONS( -DLIB3DS_STATIC )        # lib3ds is included, so we need the flag
SET_SOURCE_FILES_PROPERTIES(${C_FILES} PROPERTIES LANGUAGE "CXX")        # Force some files to be compiled as C++
