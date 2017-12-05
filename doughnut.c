/* $Id: doughnut.c,v 1.2 2003/02/22 23:17:09 d3august Exp $
*/

/* This version modified by Björn Augustsson. (d3august@dtek.chalmers.se)

   It is basically a slightly mutilated "doughnut" from
   glu_shapes.c from the GLUT distribution. Original license follows: */

/* Copyright (c) Mark J. Kilgard, 1994, 1997. */

/**
(c) Copyright 1993, Silicon Graphics, Inc.

ALL RIGHTS RESERVED

Permission to use, copy, modify, and distribute this software
for any purpose and without fee is hereby granted, provided
that the above copyright notice appear in all copies and that
both the copyright notice and this permission notice appear in
supporting documentation, and that the name of Silicon
Graphics, Inc. not be used in advertising or publicity
pertaining to distribution of the software without specific,
written prior permission.

THE MATERIAL EMBODIED ON THIS SOFTWARE IS PROVIDED TO YOU
"AS-IS" AND WITHOUT WARRANTY OF ANY KIND, EXPRESS, IMPLIED OR
OTHERWISE, INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF
MERCHANTABILITY OR FITNESS FOR A PARTICULAR PURPOSE.  IN NO
EVENT SHALL SILICON GRAPHICS, INC.  BE LIABLE TO YOU OR ANYONE
ELSE FOR ANY DIRECT, SPECIAL, INCIDENTAL, INDIRECT OR
CONSEQUENTIAL DAMAGES OF ANY KIND, OR ANY DAMAGES WHATSOEVER,
INCLUDING WITHOUT LIMITATION, LOSS OF PROFIT, LOSS OF USE,
SAVINGS OR REVENUE, OR THE CLAIMS OF THIRD PARTIES, WHETHER OR
NOT SILICON GRAPHICS, INC.  HAS BEEN ADVISED OF THE POSSIBILITY
OF SUCH LOSS, HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
ARISING OUT OF OR IN CONNECTION WITH THE POSSESSION, USE OR
PERFORMANCE OF THIS SOFTWARE.

US Government Users Restricted Rights

Use, duplication, or disclosure by the Government is subject to
restrictions set forth in FAR 52.227.19(c)(2) or subparagraph
(c)(1)(ii) of the Rights in Technical Data and Computer
Software clause at DFARS 252.227-7013 and/or in similar or
successor clauses in the FAR or the DOD or NASA FAR
Supplement.  Unpublished-- rights reserved under the copyright
laws of the United States.  Contractor/manufacturer is Silicon
Graphics, Inc., 2011 N.  Shoreline Blvd., Mountain View, CA
94039-7311.

OpenGL(TM) is a trademark of Silicon Graphics, Inc.
*/

#include <math.h>
#include <GL/gl.h>
#include "xt.h"

static void 
partialdoughnut(GLfloat r, GLfloat R, GLint nsides,
  GLint rings, GLenum type, GLfloat span, GLfloat offset)
{
  int i, j;
  GLfloat theta, phi, theta1, phi1;
  GLfloat p0[03], p1[3], p2[3], p3[3];
  GLfloat n0[3], n1[3], n2[3], n3[3];

  for (i = 0; i < rings; i++) {
    theta = (GLfloat) offset + i *2.0 * (span/360.0) * M_PI / rings;
    theta1 =(GLfloat) offset + (i + 1) * 2.0 * (span/360.0) * M_PI / rings;
    for (j = 0; j < nsides; j++) {
      phi = (GLfloat) j *2.0 * M_PI / nsides;
      phi1 = (GLfloat) (j + 1) * 2.0 * M_PI / nsides;

      p0[0] = cos(theta) * (R + r * cos(phi));
      p0[1] = -sin(theta) * (R + r * cos(phi));
      p0[2] = r * sin(phi);

      p1[0] = cos(theta1) * (R + r * cos(phi));
      p1[1] = -sin(theta1) * (R + r * cos(phi));
      p1[2] = r * sin(phi);

      p2[0] = cos(theta1) * (R + r * cos(phi1));
      p2[1] = -sin(theta1) * (R + r * cos(phi1));
      p2[2] = r * sin(phi1);

      p3[0] = cos(theta) * (R + r * cos(phi1));
      p3[1] = -sin(theta) * (R + r * cos(phi1));
      p3[2] = r * sin(phi1);

      n0[0] = cos(theta) * (cos(phi));
      n0[1] = -sin(theta) * (cos(phi));
      n0[2] = sin(phi);

      n1[0] = cos(theta1) * (cos(phi));
      n1[1] = -sin(theta1) * (cos(phi));
      n1[2] = sin(phi);

      n2[0] = cos(theta1) * (cos(phi1));
      n2[1] = -sin(theta1) * (cos(phi1));
      n2[2] = sin(phi1);

      n3[0] = cos(theta) * (cos(phi1));
      n3[1] = -sin(theta) * (cos(phi1));
      n3[2] = sin(phi1);

      glBegin(type);
      glNormal3fv(n3);
      glVertex3fv(p3);
      glNormal3fv(n2);
      glVertex3fv(p2);
      glNormal3fv(n1);
      glVertex3fv(p1);
      glNormal3fv(n0);
      glVertex3fv(p0);
      glEnd();
    }
  }
}

void partialtorus(GLfloat offset, GLfloat span, GLfloat zoom)
{
  GLfloat size = (SITE_MARKER_SIZE/2)/zoom;

  partialdoughnut(size, size+1, 4, (int)span/6+2, 
		  GL_QUADS, span, (M_PI/180.0)*offset);
}


