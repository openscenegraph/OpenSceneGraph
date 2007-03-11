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
#include "tcb.h"
#include "readwrite.h"
#include <math.h>


/*!
 * \defgroup tcb Tension/Continuity/Bias Splines
 *
 * \author J.E. Hoffmann <je-h@gmx.net>
 */


/*!
 * \ingroup tcb 
 */
void
lib3ds_tcb(Lib3dsTcb *p, Lib3dsTcb *pc, Lib3dsTcb *c, Lib3dsTcb *nc, Lib3dsTcb *n,
  Lib3dsFloat *ksm, Lib3dsFloat *ksp, Lib3dsFloat *kdm, Lib3dsFloat *kdp)
{
  Lib3dsFloat tm,cm,cp,bm,bp,tmcm,tmcp,cc;
  Lib3dsFloat dt,fp,fn;

  if (!pc) {
    pc=c;
  }
  if (!nc) {
    nc=c;
  }
  
  fp=fn=1.0f;
  if (p&&n) {
    dt=0.5f*(Lib3dsFloat)(pc->frame-p->frame+n->frame-nc->frame);
    fp=((Lib3dsFloat)(pc->frame-p->frame))/dt;
    fn=((Lib3dsFloat)(n->frame-nc->frame))/dt;
    cc=(Lib3dsFloat)fabs(c->cont);
    fp=fp+cc-cc*fp;
    fn=fn+cc-cc*fn;
  }

  cm=1.0f-c->cont;
  tm=0.5f*(1.0f-c->tens);
  cp=2.0f-cm;
  bm=1.0f-c->bias;
  bp=2.0f-bm;      
  tmcm=tm*cm;
  tmcp=tm*cp;
  *ksm=tmcm*bp*fp;
  *ksp=tmcp*bm*fp;
  *kdm=tmcp*bp*fn;
  *kdp=tmcm*bm*fn;
}


/*!
 * \ingroup tcb 
 */
Lib3dsBool
lib3ds_tcb_read(Lib3dsTcb *tcb, FILE *f)
{
  Lib3dsWord flags;
  
  tcb->frame=lib3ds_intd_read(f);
  tcb->flags=flags=lib3ds_word_read(f);
  if (flags&LIB3DS_USE_TENSION) {
    tcb->tens=lib3ds_float_read(f);
  }
  if (flags&LIB3DS_USE_CONTINUITY) {
    tcb->cont=lib3ds_float_read(f);
  }
  if (flags&LIB3DS_USE_BIAS) {
    tcb->bias=lib3ds_float_read(f);
  }
  if (flags&LIB3DS_USE_EASE_TO) {
    tcb->ease_to=lib3ds_float_read(f);
  }
  if (flags&LIB3DS_USE_EASE_FROM) {
    tcb->ease_from=lib3ds_float_read(f);
  }
  if (ferror(f)) {
    return(LIB3DS_FALSE);
  }
  return(LIB3DS_TRUE);
}


/*!
 * \ingroup tcb 
 */
Lib3dsBool
lib3ds_tcb_write(Lib3dsTcb *tcb, FILE *f)
{
  lib3ds_intd_write(tcb->frame,f);
  lib3ds_word_write(tcb->flags,f);
  if (tcb->flags&LIB3DS_USE_TENSION) {
    lib3ds_float_write(tcb->tens,f);
  }
  if (tcb->flags&LIB3DS_USE_CONTINUITY) {
    lib3ds_float_write(tcb->cont,f);
  }
  if (tcb->flags&LIB3DS_USE_BIAS) {
    lib3ds_float_write(tcb->bias,f);
  }
  if (tcb->flags&LIB3DS_USE_EASE_TO) {
    lib3ds_float_write(tcb->ease_to,f);
  }
  if (tcb->flags&LIB3DS_USE_EASE_FROM) {
    lib3ds_float_write(tcb->ease_from,f);
  }
  if (ferror(f)) {
    return(LIB3DS_FALSE);
  }
  return(LIB3DS_TRUE);
}




