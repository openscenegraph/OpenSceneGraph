/*
 * osgDB::wrl - a VRML 1.0 loader for OpenSceneGraph
 * Copyright (C) 2002 Ruben Lopez <ryu@gpul.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __LTSTR_H__
#define __LTSTR_H__

#include <string.h>

/** @class ltstr
 *
 * @brief clase/estructura que implementa un algoritmo de comparacion
 * de dos cadenas para mapas cuyo indice es una cadena.
 */
struct ltstr
{
    bool operator()(const char* s1, const char* s2) const
    {
	return strcmp(s1, s2) < 0;
    }
};

#endif
