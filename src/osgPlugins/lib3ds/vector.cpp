/*
 * The 3D Studio File Format Library
 * Copyright (C) 1996-2001 by J.E. Hoffmann <je-h@gmx.net>
 * All rights reserved.
 *
 * This program is  free  software;  you can redistribute it and/or modify it
 * under the terms of the  GNU Lesser General Public License  as published by 
 * the  Free Software Foundation;  either version 2.1 of the License,  or (at 
 * your option) any later version.
 *
 * This  program  is  distributed in  the  hope that it will  be useful,  but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or  FITNESS FOR A  PARTICULAR PURPOSE.  See the  GNU Lesser General Public  
 * License for more details.
 *
 * You should  have received  a copy of the GNU Lesser General Public License
 * along with  this program;  if not, write to the  Free Software Foundation,
 * Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 * $Id$
 */
#define LIB3DS_EXPORT
#include "vector.h"
#include <math.h>


/*!
 * \defgroup vector Vector Mathematics
 *
 * \author J.E. Hoffmann <je-h@gmx.net>
 */
/*!
 * \typedef Lib3dsVector
 *   \ingroup vector
 */


/*!
 * \ingroup vector
 */
void
lib3ds_vector_zero(Lib3dsVector c)
{
  int i;
  for (i=0; i<3; ++i) {
    c[i]=0.0f;
  }
}


/*!
 * \ingroup vector
 */
void
lib3ds_vector_copy(Lib3dsVector dest, Lib3dsVector src)
{
  int i;
  for (i=0; i<3; ++i) {
    dest[i]=src[i];
  }
}


/*!
 * \ingroup vector
 */
void
lib3ds_vector_neg(Lib3dsVector c)
{
  int i;
  for (i=0; i<3; ++i) {
    c[i]=-c[i];
  }
}


/*!
 * \ingroup vector
 */
void
lib3ds_vector_add(Lib3dsVector c, Lib3dsVector a, Lib3dsVector b)
{
  int i;
  for (i=0; i<3; ++i) {
    c[i]=a[i]+b[i];
  }
}


/*!
 * \ingroup vector
 */
void
lib3ds_vector_sub(Lib3dsVector c, Lib3dsVector a, Lib3dsVector b)
{
  int i;
  for (i=0; i<3; ++i) {
    c[i]=a[i]-b[i];
  }
}


/*!
 * \ingroup vector
 */
void
lib3ds_vector_scalar(Lib3dsVector c, Lib3dsFloat k)
{
  int i;
  for (i=0; i<3; ++i) {
    c[i]*=k;
  }
}


/*!
 * \ingroup vector
 */
void
lib3ds_vector_cross(Lib3dsVector c, Lib3dsVector a, Lib3dsVector b)
{
  c[0]=a[1]*b[2] - a[2]*b[1];
  c[1]=a[2]*b[0] - a[0]*b[2];
  c[2]=a[0]*b[1] - a[1]*b[0];
}


/*!
 * \ingroup vector
 */
Lib3dsFloat
lib3ds_vector_dot(Lib3dsVector a, Lib3dsVector b)
{
  return(a[0]*b[0] + a[1]*b[1] + a[2]*b[2]);
}


/*!
 * \ingroup vector
 */
Lib3dsFloat
lib3ds_vector_squared(Lib3dsVector c)
{
  return(c[0]*c[0] + c[1]*c[1] + c[2]*c[2]);
}


/*!
 * \ingroup vector
 */
Lib3dsFloat
lib3ds_vector_length(Lib3dsVector c)
{
  return((Lib3dsFloat)sqrt(c[0]*c[0] + c[1]*c[1] + c[2]*c[2]));
}


/*!
 * \ingroup vector
 */
void
lib3ds_vector_normalize(Lib3dsVector c)
{
  Lib3dsFloat l,m;

  l=(Lib3dsFloat)sqrt(c[0]*c[0] + c[1]*c[1] + c[2]*c[2]);
  if (fabs(l)<LIB3DS_EPSILON) {
    c[0]=c[1]=c[2]=0.0f;
    if ((c[0]>=c[1]) && (c[0]>=c[2])) {
      c[0]=1.0f;
    }
    else
    if (c[1]>=c[2]) {
      c[1]=1.0f;
    }
    else {
      c[2]=1.0f;
    }
  }
  else {
    m=1.0f/l;
    c[0]*=m;
    c[1]*=m;
    c[2]*=m;
  }
}


/*!
 * \ingroup vector
 */
void
lib3ds_vector_normal(Lib3dsVector n, Lib3dsVector a, Lib3dsVector b, Lib3dsVector c)
{
  Lib3dsVector p,q;

  lib3ds_vector_sub(p,c,b);
  lib3ds_vector_sub(q,a,b);
  lib3ds_vector_cross(n,p,q);
  lib3ds_vector_normalize(n);
}


/*!
 * \ingroup vector
 */
void
lib3ds_vector_transform(Lib3dsVector c, Lib3dsMatrix m, Lib3dsVector a)
{
  c[0]= m[0][0]*a[0] + m[1][0]*a[1] + m[2][0]*a[2] + m[3][0];
  c[1]= m[0][1]*a[0] + m[1][1]*a[1] + m[2][1]*a[2] + m[3][1];
  c[2]= m[0][2]*a[0] + m[1][2]*a[1] + m[2][2]*a[2] + m[3][2];
}


/*!
 * \ingroup vector
 */
void
lib3ds_vector_cubic(Lib3dsVector c, Lib3dsVector a, Lib3dsVector p, Lib3dsVector q,
  Lib3dsVector b, Lib3dsFloat t)
{
  Lib3dsDouble x,y,z,w;   

  x=2*t*t*t - 3*t*t + 1;
  y=-2*t*t*t + 3*t*t;
  z=t*t*t - 2*t*t + t;
  w=t*t*t - t*t;
  c[0]=(Lib3dsFloat)(x*a[0] + y*b[0] + z*p[0] + w*q[0]);
  c[1]=(Lib3dsFloat)(x*a[1] + y*b[1] + z*p[1] + w*q[1]);
  c[2]=(Lib3dsFloat)(x*a[2] + y*b[2] + z*p[2] + w*q[2]);
}


/*!
 * c[i] = min(c[i], a[i]);
 * \ingroup vector
 */
void 
lib3ds_vector_min(Lib3dsVector c, Lib3dsVector a)
{
  int i;
  for (i=0; i<3; ++i) {
    if (a[i]<c[i]) {
      c[i] = a[i];
    }
  }
}


/*!
 * c[i] = max(c[i], a[i]);
 * \ingroup vector
 */
void 
lib3ds_vector_max(Lib3dsVector c, Lib3dsVector a)
{
  int i;
  for (i=0; i<3; ++i) {
    if (a[i]>c[i]) {
      c[i] = a[i];
    }
  }
}


/*!
 * \ingroup vector
 */
void
lib3ds_vector_dump(Lib3dsVector c)
{
  fprintf(stderr, "%f %f %f\n", c[0], c[1], c[2]);
}

