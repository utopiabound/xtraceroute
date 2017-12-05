/* $Id: sphere.c,v 1.2 2003/02/22 23:17:09 d3august Exp $
*/

/* This version of sphere.c (with texture coodinate generation) by
   Björn Augustsson (d3august@dtek.chalmers.se) 1998. 
   (I also removed "sphere_tris()" from this file) */
  
/* sphere.c - by David Blythe, SGI */

/* Instead of tessellating a sphere by lines of longitude and latitude
   (a technique that over tessellates the poles and under tessellates
   the equator of the sphere), tesselate based on regular solids for a
   more uniform tesselation.

   Based on code originally written by Jon Leech 
   (leech@cs.unc.edu) 3/24/89

   This approach is arguably better than the gluSphere routine's
   approach using slices and stacks (latitude and longitude). -mjk
*/

/* Here is a real problem with this sphere:
   
   This is a texture, with a "map" over where the triangles "map":
   __________________
   |x/\xx/\xx/\xx/\x|
   |/__\/__\/__\/__\|
   |\  /\  /\  /\  /|
   |_\/__\/__\/__\/_|
   | /\  /\  /\  /\ |
   |/__\/__\/__\/__\|
   |\  /\  /\  /\  /|
   |x\/xx\/xx\/xx\/x|
   ^^^^^^^^^^^^^^^^^^
   
   There is no way to get actual, correct texturing on a sphere made up
   completely from triangles. The stuff where the "x"es are in the
   picture above won't get drawn.
   
   What you can do is to use quads for the top and bottom layers.
   (Like in gluSphere) But quads are Bad. I might do this one day
   though.
   
   On the other hand, at levels 2 and above (which you'd probably 
   typically use, or you'd get a really weird "sphere") it's not very
   noticeable.

   --August,  */

#include <stdlib.h>
#include <math.h>
#include <GL/gl.h>
#include "xt.h"

/* If you want to use this function in another program, cut&paste the
   tolat() and tolon() functions from util.c in the xtraceroute source,
   and #include <GL/gl.h> here instead of "xt.h".      --August.     */

typedef struct {
    float x, y, z;
} point;

typedef struct {
    point pt[3];
} triangle;

/* for icosahedron */
#define CZ (0.866025403)        /* cos(30) */
#define SZ (0.5)                /* sin(30) */
#define C1 (0.951056516)        /* cos(18) */
#define S1 (0.309016994)        /* sin(18) */
#define C2 (0.587785252)        /* cos(54) */
#define S2 (0.809016994)        /* sin(54) */
#define X1 (C1*CZ)
#define Y1 (S1*CZ)
#define X2 (C2*CZ)
#define Y2 (S2*CZ)

#define Ip0     {0.,    1.,     0.}
#define Ip1     {-X2,   SZ,    -Y2}
#define Ip2     {X2,    SZ,    -Y2}
#define Ip3     {X1,    SZ,     Y1}
#define Ip4     {0,     SZ,     CZ}
#define Ip5     {-X1,   SZ,     Y1}

#define Im0     {-X1,   -SZ,    -Y1}
#define Im1     {0,     -SZ,    -CZ}
#define Im2     {X1,    -SZ,    -Y1}
#define Im3     {X2,    -SZ,     Y2}
#define Im4     {-X2,   -SZ,     Y2}
#define Im5     {0.,    -1.,     0.}

/* vertices of a unit icosahedron */
static triangle icosahedron[20]= {
        /* "north" pole */

        { {Ip1, Ip0, Ip2}, },
        { {Ip5, Ip0, Ip1}, },
        { {Ip4, Ip0, Ip5}, },
        { {Ip3, Ip0, Ip4}, },
        { {Ip2, Ip0, Ip3}, },

        /* mid */
        { {Ip1, Im1, Im0}, },
        { {Im0, Ip5, Ip1}, },
        { {Ip5, Im0, Im4}, },
        { {Im4, Ip4, Ip5}, },
        { {Ip4, Im4, Im3}, },
        { {Im3, Ip3, Ip4}, },
        { {Ip3, Im3, Im2}, },
        { {Im2, Ip2, Ip3}, },
        { {Ip2, Im2, Im1}, },
        { {Im1, Ip1, Ip2}, },

        /* "south" pole */
        { {Im3, Im5, Im2}, },
        { {Im4, Im5, Im3}, },
        { {Im0, Im5, Im4}, },
        { {Im1, Im5, Im0}, },
        { {Im2, Im5, Im1}, },
};

/* normalize point r */
static void
normalize(point *r) {
    float mag;

    mag = r->x * r->x + r->y * r->y + r->z * r->z;
    if (mag != 0.0f) {
        mag = 1.0f / sqrt(mag);
        r->x *= mag;
        r->y *= mag;
        r->z *= mag;
    }
}

/* linearly interpolate between a & b, by fraction f */
static void
lerp(point *a, point *b, float f, point *r) {
    r->x = a->x + f*(b->x-a->x);
    r->y = a->y + f*(b->y-a->y);
    r->z = a->z + f*(b->z-a->z);
}

/**
 * Convenience function used only in sphere().
 * Adds a vertex to a tstrip.
 */

static void 
addVertex(point v, GLdouble *lastlon)
{
  GLdouble a, b;
  point x;
  x = v;
  normalize(&x);
  glNormal3fv(&x.x);
  a = tolon(x.x, x.y, x.z);
  b = tolat(x.x, x.y, x.z);

  if(a == 5000.0)   /* 5000 is the slightly unintuitive value tolon() */
    {               /* returns when it finds a value on a pole. */
      if(b > 0)                        /* If it's the north pole */
	a = *lastlon - 36.0;
      else                             /* South pole. */
	a = *lastlon + 36.0;
    }
  else if(*lastlon != 4000.0)   /* Not the first vertex in a tstrip.*/
    {
      /* This stuff is to get the wraparoud at the backside of the 
	 sphere to work. (Or you'd get a weird zigzag line down the 
	 back of the sphere.) */

      GLdouble dist = a - *lastlon;
      if(dist > 100.0)
	a -= 360.0;
      else if(dist < -100.0)
	a += 360.0;
    }
  *lastlon = a;
  glTexCoord2f(a/360.0+0.5, -(b/180.0+0.5));
  glVertex3fv(&x.x);
}

void sphere(int maxlevel) {
    int nrows = 1 << maxlevel;
    int s;
    GLdouble last_lon;
    /* iterate over the 20 sides of the icosahedron */
    for(s = 0; s < 20; s++) {
        int i;
        triangle *t = &icosahedron[s];
        for(i = 0; i < nrows; i++) {
            /* create a tstrip for each row */
            /* number of triangles in this row is number in previous +2 */
            /* strip the ith trapezoid block */
            point v0, v1, v2, v3, va, vb;
            int j;
            lerp(&t->pt[1], &t->pt[0], (float)(i+1)/nrows, &v0);
            lerp(&t->pt[1], &t->pt[0], (float)i/nrows, &v1);
            lerp(&t->pt[1], &t->pt[2], (float)(i+1)/nrows, &v2);
            lerp(&t->pt[1], &t->pt[2], (float)i/nrows, &v3);

	    /* Hack to get the texturing right. */
	    last_lon = 4000.0;

            glBegin(GL_TRIANGLE_STRIP);
            addVertex(v0, &last_lon);
            addVertex(v1, &last_lon);
            for(j = 0; j < i; j++) {
	        /* calculate 2 more vertices at a time */
	        lerp(&v0, &v2, (float)(j+1)/(i+1), &va);
                lerp(&v1, &v3, (float)(j+1)/i, &vb);
                addVertex(va, &last_lon);
                addVertex(vb, &last_lon);
            }
            addVertex(v2, &last_lon);
            glEnd(); /* TRIANGLE_STRIP */
        }
    }
}
