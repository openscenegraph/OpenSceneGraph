/*
    Copyright (C) 1996-2008 by Jan Eric Kyprianidis <www.kyprianidis.com>
    All rights reserved.

    This program is free  software: you can redistribute it and/or modify
    it under the terms of the GNU Lesser General Public License as published
    by the Free Software Foundation, either version 2.1 of the License, or
    (at your option) any later version.

    Thisprogram  is  distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU Lesser General Public License for more details.

    You should  have received a copy of the GNU Lesser General Public License
    along with  this program; If not, see <http://www.gnu.org/licenses/>.
*/
#include "lib3ds_impl.h"


/*!
 * Return a new Lib3dsCamera object.
 *
 * Object is initialized with the given name and fov=45.  All other
 * values are 0.
 *
 * \param name Name of this camera.  Must not be NULL.  Must be < 64 characters.
 *
 * \return Lib3dsCamera object or NULL on failure.
 */
Lib3dsCamera*
lib3ds_camera_new(const char *name) {
    Lib3dsCamera *camera;

    assert(name);
    assert(strlen(name) < 64);

    camera = (Lib3dsCamera*)calloc(sizeof(Lib3dsCamera), 1);
    if (!camera) {
        return(0);
    }
    stringcopyfixedsize(camera->name, name);
    camera->fov = 45.0f;
    return(camera);
}


/*!
 * Free a Lib3dsCamera object and all of its resources.
 *
 * \param camera Lib3dsCamera object to be freed.
 */
void
lib3ds_camera_free(Lib3dsCamera *camera) {
    memset(camera, 0, sizeof(Lib3dsCamera));
    free(camera);
}


/*!
 * Read a camera definition from a file.
 *
 * This function is called by lib3ds_file_read(), and you probably
 * don't want to call it directly.
 *
 * \param camera A Lib3dsCamera to be filled in.
 * \param io A Lib3dsIo object previously set up by the caller.
 *
 * \see lib3ds_file_read
 */
void
lib3ds_camera_read(Lib3dsCamera *camera, Lib3dsIo *io) {
    Lib3dsChunk c;
    uint16_t chunk;

    lib3ds_chunk_read_start(&c, CHK_N_CAMERA, io);

    {
        int i;
        for (i = 0; i < 3; ++i) {
            camera->position[i] = lib3ds_io_read_float(io);
        }
        for (i = 0; i < 3; ++i) {
            camera->target[i] = lib3ds_io_read_float(io);
        }
    }
    camera->roll = lib3ds_io_read_float(io);
    {
        float s;
        s = lib3ds_io_read_float(io);
        if (fabs(s) < LIB3DS_EPSILON) {
            camera->fov = 45.0;
        } else {
            camera->fov = 2400.0f / s;
        }
    }
    lib3ds_chunk_read_tell(&c, io);

    while ((chunk = lib3ds_chunk_read_next(&c, io)) != 0) {
        switch (chunk) {
            case CHK_CAM_SEE_CONE: {
                camera->see_cone = TRUE;
            }
            break;

            case CHK_CAM_RANGES: {
                camera->near_range = lib3ds_io_read_float(io);
                camera->far_range = lib3ds_io_read_float(io);
            }
            break;

            default:
                lib3ds_chunk_unknown(chunk, io);
        }
    }

    lib3ds_chunk_read_end(&c, io);
}


/*!
 * Write a camera definition to a file.
 *
 * This function is called by lib3ds_file_write(), and you probably
 * don't want to call it directly.
 *
 * \param camera A Lib3dsCamera to be written.
 * \param io A Lib3dsIo object previously set up by the caller.
 *
 * \see lib3ds_file_write
 */
void
lib3ds_camera_write(Lib3dsCamera *camera, Lib3dsIo *io) {
    Lib3dsChunk c;

    c.chunk = CHK_N_CAMERA;
    lib3ds_chunk_write_start(&c, io);

    lib3ds_io_write_vector(io, camera->position);
    lib3ds_io_write_vector(io, camera->target);
    lib3ds_io_write_float(io, camera->roll);
    if (fabs(camera->fov) < LIB3DS_EPSILON) {
        lib3ds_io_write_float(io, 2400.0f / 45.0f);
    } else {
        lib3ds_io_write_float(io, 2400.0f / camera->fov);
    }

    if (camera->see_cone) {
        Lib3dsChunk c;
        c.chunk = CHK_CAM_SEE_CONE;
        c.size = 6;
        lib3ds_chunk_write(&c, io);
    }
    {
        Lib3dsChunk c;
        c.chunk = CHK_CAM_RANGES;
        c.size = 14;
        lib3ds_chunk_write(&c, io);
        lib3ds_io_write_float(io, camera->near_range);
        lib3ds_io_write_float(io, camera->far_range);
    }

    lib3ds_chunk_write_end(&c, io);
}

