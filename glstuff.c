/* $Id: glstuff.c,v 1.12 2003/03/22 01:46:37 d3august Exp $
*/
/*  xtraceroute - graphically show traceroute information.
 *  Copyright (C) 1996-1998  Björn Augustsson 
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#include <stdlib.h>
#include <gtkgl/gtkglarea.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <math.h>     /* for cos(), sin(), and sqrt() */
#include "xt.h"

extern void sphere(int level);

extern GdkPixbuf *earth_texture;
extern GdkPixbuf *night_texture;
extern GdkPixbuf *created_texture;

//extern struct traceroute_stuff;  // from extprog.c
extern GLfloat zoom;

static GLUquadricObj *lineobj;
static GLUquadricObj *earthobj;
static GLUquadricObj *sitemarker;

static const GLfloat hiaccColor[]  = {0.1, 1.0,  0.1, 1.0}; // Green for good.
static const GLfloat medaccColor[] = {1.0, 1.0,  0.1, 1.0}; // Yellow for OK.
static const GLfloat loaccColor[]  = {1.0, 0.53, 0.0, 1.0}; // Orange for bad
static const GLfloat noaccColor[]  = {1.0, 0.1,  0.1, 1.0}; // Red for no info

static const GLfloat lineColor[]         = {1.0, 1.0, 0.1, 1.0}; // Yellow
static const GLfloat defaultColor[]      = {0.8, 0.8, 0.8, 1.0};

static const GLfloat selectedEmission[]  = {0.6, 0.6, 0.6, 1.0};
static const GLfloat defaultEmission[]   = {0.0, 0.0, 0.0, 1.0};

static const GLfloat lightZeroPosition[] = { 10.0,  4.0, 10.0, 1.0};
static const GLfloat lightZeroColor[]    = {  0.8,  1.0,  0.8, 1.0}; /* green-tinted */
static const GLfloat lightOnePosition[]  = { -1.0, -2.0,  1.0, 0.0};
static const GLfloat lightOneColor[]     = {  0.9,  0.9,  0.9, 1.0}; /* bright white */

static int render_mode = NORMAL_MODE;

/**
 * Set the Level Of Detail on the spheres generated in the future.
 * Only affects the earth-sphere, not the ones that represent
 * sites.
 */

void
set_sphere_lod(const int lod)
{
  user_settings->LOD = lod;
}

/**
 * Set the mode in which the earth is to be rendered. Valid modes are:
 * NORMAL_MODE (default, does nothing special)
 * SELECT_MODE (uses glLoadName so we can do picking.)
 */

void 
set_render_mode(int mode)
{
  render_mode = mode;
}

/**
 * between puts the coordinates for a point between the two
 * specified ones in (rlat, rlon).
 */

static void 
between(const GLdouble lat1, const GLdouble lon1,
	const GLdouble lat2, const GLdouble lon2,
	GLdouble *rlat, GLdouble *rlon)
  { 
   GLdouble x1 = tox(lat1, lon1);
   GLdouble y1 = toy(lat1, lon1);
   GLdouble z1 = toz(lat1, lon1); 
   GLdouble x2 = tox(lat2, lon2);
   GLdouble y2 = toy(lat2, lon2);
   GLdouble z2 = toz(lat2, lon2); 

   GLdouble  x = (x1 + x2) / 2.0;
   GLdouble  y = (y1 + y2) / 2.0;
   GLdouble  z = (z1 + z2) / 2.0;

/*  'Normalize' to the radius of the earth. (Which is always 1, since this
     is the Unit Earth (TM))  */

   GLdouble f = 1.0 / sqrt(x*x + y*y + z*z);
    x *= f;
    y *= f;
    z *= f;
    *rlat = tolat(x,y,z);
    *rlon = tolon(x,y,z);
  }

/**
 * This is only used from makeearth, to plot a line on the globe.
 * There is some difficultish maths in this one.
 *
 * "to" and "from" in the comments in the code refers to the two
 * points given as arguments.
 */

static void
plot_line(const GLfloat fromlat, const GLfloat fromlon, const GLfloat tolat, const GLfloat tolon)
{
  GLfloat vink;
  GLfloat linesweep;
  
  glPushMatrix();
  glMaterialfv(GL_FRONT, GL_DIFFUSE, lineColor);
  
  /*    Angle is arccos(Ax*Bx + Ay*By + Az*Bz)    */
  
  linesweep = acos(tox(tolat, tolon)*tox(fromlat, fromlon)
		   +toy(tolat, tolon)*toy(fromlat, fromlon)
		   +toz(tolat, tolon)*toz(fromlat, fromlon))
    * todeg;
  
  /*   This mess calculates the angle to rotate "to" to a position
       'south' of "from". It's a somewhat adapted version of a formula
       that calculates an angle in a triangle on a sphere given all three
       sides. (Found the original in Beta.)                           */
  
  vink = todeg*acos((sin(tolat*torad)-sin(fromlat*torad)*cos(linesweep*torad))/
		    ( cos(fromlat*torad) * sin(linesweep*torad) )      );
  
  /*   Compensate for the usual arccos problem. (It checks if "to" is
       'to the right' or 'to the left' of "from")                 */
  
  if(fromlon > 0.0)
    {
      if(tolon > fromlon || tolon < fromlon - 180.0)
	vink = -vink;
    }
  else
    {
      if(tolon > fromlon && tolon < fromlon + 180.0)
	vink = -vink;
    }
  
  /*   Rotate "to" to a position 'south' of "from".   */
  
  glRotatef(vink, tox(fromlat, fromlon),
	    toy(fromlat, fromlon),
	    toz(fromlat, fromlon));
  
  /*   ...and rotate them onto the z=0 plane.   */
  
  glRotatef(90.0+fromlon, 0.0, 1.0, 0.0);
  
  /*   Fix incorrect names on the lines, identifying them as the 
       sitemarker we just drew.             */
  
  if(render_mode == SELECT_MODE)
    glLoadName(NOT_SELECTABLE);
  
  partialtorus((GLfloat)fromlat-180.0, (GLfloat)linesweep, zoom);
  glPopMatrix();
}

/**
 * This is only used from makeearth, to plot a satellite above the 
 * globe.
 * (It's just plot_line, for high-latency links)
 *
 * "dest" and "from" in the comments in the code refers to the two
 * points given as arguments.
 */

static void
plot_sat(const GLfloat fromlat, const GLfloat fromlon, 
	 const GLfloat destlat, const GLfloat destlon)
{
  static const GLfloat satPanelColor[]  = { 0.0, 0.0, 0.2, 1.0}; /* Glossy dark-blue */
  static const GLfloat satBodyColor[]   = {1.0, 0.53, 0.0, 1.0}; /* Goldish */
  static const GLfloat mat_specular[]   = { 1.0, 1.0, 1.0, 1.0 };
  static const GLfloat mat_nospecular[] = { 0.0, 0.0, 0.0, 1.0 };

  GLfloat factor;
  GLfloat line_size = (SITE_MARKER_SIZE/2.0)/zoom;

  GLfloat fromx = tox(fromlat, fromlon);
  GLfloat fromy = toy(fromlat, fromlon);
  GLfloat fromz = toz(fromlat, fromlon);
  
  GLfloat destx = tox(destlat, destlon);
  GLfloat desty = toy(destlat, destlon);
  GLfloat destz = toz(destlat, destlon);
  
  GLfloat satx;
  GLfloat saty;
  GLfloat satz;
  GLdouble satlat, satlon;

  between(fromlat, fromlon,
	  destlat, destlon,
	  &satlat, &satlon);

  satx = tox(satlat, satlon);

#ifdef SAT_OVER_EQUATOR
  saty = toy(0.0, satlon);
#else
  saty = toy(satlat, satlon);
#endif

  satz = toz(satlat, satlon);
  
  /* Draw the satellite */
  
  glPushMatrix();
  
  factor = (GEOSYNC_SAT_ALT+PHYSICAL_EARTH_RADIUS) / PHYSICAL_EARTH_RADIUS;
  glTranslated(satx*factor,
	       saty*factor,
	       satz*factor);
  
  glMaterialfv(GL_FRONT, GL_DIFFUSE, satBodyColor);
  glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, 80);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  
  gluSphere(sitemarker,SITE_MARKER_SIZE/zoom,7,7);
  
  glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, satPanelColor);
  glDisable(GL_CULL_FACE);
  glBegin(GL_QUADS);
  {
    const GLfloat panel_length = 15*SITE_MARKER_SIZE/zoom; 
    const GLfloat panel_width  = SITE_MARKER_SIZE/zoom; 
    
    glVertex3f( -panel_width,  -panel_width,  panel_width );
    glVertex3f( -panel_length, -panel_width,  panel_width );
    glVertex3f( -panel_length,  panel_width, -panel_width );
    glVertex3f( -panel_width,   panel_width, -panel_width );
    
    glVertex3f(  panel_length, -panel_width,  panel_width );
    glVertex3f(  panel_width,  -panel_width,  panel_width );
    glVertex3f(  panel_width,   panel_width, -panel_width );
    glVertex3f(  panel_length,  panel_width, -panel_width );
  }
  glEnd(); // GL_QUADS
  glEnable(GL_CULL_FACE);
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_nospecular);
  
  glPopMatrix();
  
  /* Make the line from -> sat */
  
  glMaterialfv(GL_FRONT, GL_DIFFUSE, lineColor);
  glPushMatrix();
  {
    GLfloat diffx, diffy, diffz, difflen;
    
    glTranslatef(fromx, fromy, fromz);
    
    /* rotate around (0,0,1)(satx-fromx, saty-fromy, satz-fromz) */
    /* angle between a and b is acos(a*b / abs(a)*abs(b)) */
    
    diffx   = satx*factor - fromx;
    diffy   = saty*factor - fromy;
    diffz   = satz*factor - fromz;
    difflen = sqrt(diffx*diffx + diffy*diffy + diffz*diffz);
    
    //      printf("x: %2.3f\ty: %2.3f\tz: %2.3f\tlen: %2.3f\n", diffx, diffy, diffz, difflen);      
    
    // FIXME Undoubtedly there's an error here, acos only returns angles 
    //       between 0 and pi, so I better do some thinking.
    glRotatef(todeg*acos(diffz/difflen),
	      -diffy, diffx, 0.0);
    
    gluCylinder(lineobj,
		line_size,
		line_size,
		difflen,
		4,
		10);    // FIXME tweak this value later.
  }
  glPopMatrix();
  
  /* Make the line dest -> sat */
  
  glPushMatrix();
  {
    GLfloat diffx, diffy, diffz, difflen;
    
    glTranslated(destx, desty, destz);
    
    diffx   = satx*factor - destx;
    diffy   = saty*factor - desty;
    diffz   = satz*factor - destz;
    difflen = sqrt(diffx*diffx + diffy*diffy + diffz*diffz);
    
    glRotatef(todeg*acos(diffz/difflen),
	      -diffy, diffx, 0.0);
    
    gluCylinder(lineobj,
		line_size,
		line_size,
		difflen,
		4,
		10);    // FIXME tweak this value later.
  }      
  glPopMatrix();
  
}

/**
 * This is only used from makeearth, to plot a site on the globe.
 */
static void
plot_site(const int site_nr, const GLfloat lat, const GLfloat lon, 
	  const int height, const int selected, const int accuracy)
{
  GLfloat factor;

  glPushMatrix();
  if(selected)
    glMaterialfv(GL_FRONT, GL_EMISSION, selectedEmission);
  switch(accuracy)
    {
    case ACC_RFC_1876:
    case ACC_RFC_1712:
      glMaterialfv(GL_FRONT, GL_DIFFUSE, hiaccColor);
      break;
    case ACC_NDG_HOST:
    case ACC_NDG_NET:
    case ACC_LOCAL_SITE:
    case ACC_LOCAL_USER:
    case ACC_INCLUDED:
      glMaterialfv(GL_FRONT, GL_DIFFUSE, medaccColor);	    
      break;
    case ACC_GEOIP:
    case ACC_SUFFIX:
      glMaterialfv(GL_FRONT, GL_DIFFUSE, loaccColor);
      break;
    case ACC_NONE:
      glMaterialfv(GL_FRONT, GL_DIFFUSE, noaccColor);
      break;
    default:
      printf("Argh! Unknown accuracy!\n");
      break;
    }
  factor = 1.0+(2.0*SITE_MARKER_SIZE*(height))/zoom;
  glTranslated(tox(lat, lon)*factor,
	       toy(lat, lon)*factor,
	       toz(lat, lon)*factor);
  
  if(render_mode == SELECT_MODE)
    glLoadName(site_nr);
  
  gluSphere(sitemarker,SITE_MARKER_SIZE/zoom,5,5);
  
  if (selected)
    glMaterialfv(GL_FRONT, GL_EMISSION, defaultEmission);
  
  glPopMatrix();
}

/**
 * map_texture() : map a texture to earthobj
 */

void 
map_texture(const GdkPixbuf* tex) 
{
  GLenum appropriate_format;

  if(gdk_pixbuf_get_has_alpha(tex))
    appropriate_format = GL_RGBA;
  else
    appropriate_format = GL_RGB;


  gluQuadricTexture(earthobj,GL_TRUE);
  glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
  
  /* I would be clever and use the nice appropriate_format variable
     here, but that doesn't work, because gluBuild2DMipmaps doesn't
     support that. See above for rant on that. */
  gluBuild2DMipmaps(GL_TEXTURE_2D,
		    GL_RGBA,
		    gdk_pixbuf_get_width(tex),
		    gdk_pixbuf_get_height(tex),
		    appropriate_format,
		    GL_UNSIGNED_BYTE,
		    gdk_pixbuf_get_pixels(tex));
}


/*-------------------------------------------------------------------------*/
/* set_transparency()                                                      */
/*-------------------------------------------------------------------------*/
void 
set_transparency(gint blend_factor) 
{
  if (blend_factor) {
    glEnable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    switch (blend_factor)  
      {
      case 1 : glBlendFunc(GL_ONE,GL_SRC_ALPHA); 
	break;
      }
    
  }
  else {
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
  }
}

/*-------------------------------------------------------------------------*/
/* set_zoom() : only called from choose_zoom() ;                           */
/* modify the global variable zoom used as scaling factor for each redraw  */
/* of the world and modifies the point we look at iwhen zooming in/out.    */
/* When zoom in tends to max, the look at point tends to the upper pole.   */
/* When zoom tends to min, the look at point tends to earth center.        */
/*-------------------------------------------------------------------------*/
void set_zoom( gint zoom_type) {
  
  switch (zoom_type) 
    {
    case 0: 
      zoom = zoom + .2; 
      break;   /* zoom in */
    case 1: 
      zoom = zoom - .2; 
      break;   /* zoom out */
    case 2: 
      zoom = zoom * 2; 
      break;    /* zoom in x2 */
    case 3: 
      zoom = zoom / 2; 
      break;    /* zoom out x2*/
    case 4: 
      zoom = EARTH_SIZE ;
      break;  /* zoom default */
    }

  if (zoom > Z_OF_EYE - EARTH_SIZE)
    zoom=Z_OF_EYE - EARTH_SIZE;  /* prevents going inside the object */
  
  if (zoom <= EARTH_SIZE/4)
    zoom = EARTH_SIZE/4;              /* prevents going beyond "infinity" */
  
  //  glMatrixMode(GL_MODELVIEW);
  //  glPopMatrix();
  //  glLoadIdentity();
  /* look to y=0 for zoom min and make y tend to * EARTH_SIZE when zoom max*/
  
  //  gluLookAt( 0.0,                   0.0,                 Z_OF_EYE, 
  //	     0.0,   EARTH_SIZE-(EARTH_SIZE/(zoom*zoom)),   0.0,
  //	     0.0,                   1.0,                   0.0);
  
  //  glPushMatrix(); /* dummy push so we can pop on model recalc */
}


/*-------------------------------------------------------------------------*/
/* set_view_motion() : only called from mouse_motion() ; this is the GL    */
/* implementation (hence appart from main.c) of moving the point we look   */
/* at on an x-y axis.                                                      */
/*-------------------------------------------------------------------------*/
void set_view_motion(gfloat strafex, gfloat strafey) 
{
  glPopMatrix();
  glTranslatef((GLfloat) strafex, (GLfloat) strafey,0);
  glPushMatrix(); /* dummy push so we can pop on model recalc */
}


/**
 * Set up the OpenGL state, init variables etc.
 */

void 
init_gl(GtkWidget *wi, gpointer data)
{
  
  if (!gtk_gl_area_make_current(GTK_GL_AREA(wi)))
    { 
      printf("make_current failed early in init_gl()\n");
    }

  lineobj  = gluNewQuadric();
  earthobj = gluNewQuadric();
  sitemarker = gluNewQuadric();
  gluQuadricDrawStyle(lineobj, GLU_FILL);
//  gluQuadricOrientation(earthobj, GLU_INSIDE); // Wow, that got strange...
  
  
  glEnable(GL_NORMALIZE);  /* Or zoom'll break the lighting */
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_LIGHTING);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();
  gluPerspective( /* field of view in degree */ 40.0,
		  /* aspect ratio            */  1.0,
		  /* Z near                  */  1.0, 
		  /* Z far                   */ 10.0);
  glMatrixMode(GL_MODELVIEW);
  gluLookAt(0.0, 0.0, Z_OF_EYE,    /* eye is normally at (0,0,4) */
	    0.0, 0.0, 0.0,         /* center is at (0,0,0) */
	    0.0, 1.0, 0.0);        /* up is in postive Y direction */
  
  glPushMatrix(); /* dummy push so we can pop on model recalc */
  
  /* Lighting stuff. */
  glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, 1);
  glLightfv(GL_LIGHT0, GL_POSITION, lightZeroPosition);
  glLightfv(GL_LIGHT0, GL_DIFFUSE, lightZeroColor);
  glLightf(GL_LIGHT0, GL_CONSTANT_ATTENUATION, 0.1);
  glLightf(GL_LIGHT0, GL_LINEAR_ATTENUATION, 0.05);
  glLightfv(GL_LIGHT1, GL_POSITION, lightOnePosition);
  glLightfv(GL_LIGHT1, GL_DIFFUSE, lightOneColor);
  glEnable(GL_LIGHT0);
  glEnable(GL_LIGHT1);
  
  glDisable(GL_BLEND);
  glEnable(GL_CULL_FACE);
  
#ifdef FLAT_SHADING
  glShadeModel(GL_FLAT);
#endif
  
  switch(user_settings->lighting_mode)
    {
    case DAY_AND_NIGHT:
      composite_texture(night_texture, earth_texture, &created_texture);
      map_texture(created_texture);
      break;
    case DAY_ONLY:
      map_texture(earth_texture);
      break;
    default:
      printf("Funky lighting mode doing textures: %d\n", 
	     user_settings->lighting_mode);
      exit(EXIT_FAILURE);
    }
  
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
#ifdef UGLY_LINES
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
#else
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
#endif //UGLY_LINES
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

  gtk_gl_area_swapbuffers(GTK_GL_AREA(wi));
}

/**
 * This determines wether to plot a site on top of another or put it 
 * on the ground and connect it and the previous one with a line.
 */

static int far_enough_apart(const site a, const site b)
{
  float londiff = fabs(a.lon - b.lon);
  float latdiff = fabs(a.lat - b.lat);
  
  /* This test should really check true distance, but hey... */
  return (londiff > 3.0 || latdiff > 3.0);
}


/**
 * This one builds the WORLD displaylist. It gets called whenever there
 * is new information to be "plotted". (The function, not the list)
 *
 * This is the kind of code everyone hates. 300 lines, one bazillion
 * levels of indentation, obscure maths, and weird names for stuff.
 *
 * It's certainly scheduled for a rewrite soon, but until then...
 *
 * It's a little better now, as of 0.8.8. Not good, better.
 **/

void makeearth(void)
{
  int i;
  int sites_on_top = 0;  
  int bad_behind   = 0;
  int any_good_sites_plotted = 0;

  // glxmakecurrent isn't allowed in select or feedback modes.
  if (render_mode == NORMAL_MODE)
    {
      if (!gtk_gl_area_make_current(GTK_GL_AREA(glarea)))
	printf("make_current failed early in makeearth()\n");
    }
  glNewList(WORLD, GL_COMPILE);
  glPushMatrix();
  
  glMaterialfv(GL_FRONT, GL_DIFFUSE, defaultColor);
  glEnable(GL_TEXTURE_2D);
  
#if 0
  // This doesn't work. For some reason the texture needs to be upside
  //  down for gluSphere to work. (It generates weird texture
  //  coordinates)
  gluSphere(earthobj, 1.0, sphere_lod, sphere_lod);
#else
  sphere(user_settings->LOD);
#endif //0
  
  glDisable(GL_TEXTURE_2D);
  
  glPopMatrix();
  
  for(i=0 ; i<MAX_SITES && sites[i].draw ; i++) /* Here we go */
    {
      /* If the first sites doesn't reply, stack them 
	 under the first known one */
      if(sites[i].accuracy == ACC_NONE && !any_good_sites_plotted)
	continue;
      
      if(!any_good_sites_plotted) /* "good" means accuracy > ACC_NONE */
	{
	  int ii;
	  for(ii=0 ; ii<i ; ii++)
	    {
	      plot_site(ii, sites[i].lat, 
			sites[i].lon,
			ii,
			sites[ii].selected,
			sites[ii].accuracy);
	    }
	  plot_site(i, sites[i].lat, sites[i].lon, ii, 
		    sites[i].selected, sites[i].accuracy);
	  any_good_sites_plotted = 1;
	  continue;
	}

      /* Normal case begins here */
      if(i > 0 && far_enough_apart(sites[i], sites[i-1]))
	{
	  plot_site(i, sites[i].lat, sites[i].lon, 0, 
		    sites[i].selected, sites[i].accuracy);
	  
	  sites_on_top = 0;
	  
	  /* Make a line to it from the last one. (Or a satellite) 

             The "260" value is because 240.2 milliseconds is the time
             it takes light to travel from the ground to 36000 km (where
             geosync satellites are), and back. Seems reasonable. */

	  if(sites[i].time - sites[i-1].time < 260) 
	    {
	      plot_line(sites[i-1].lat, sites[i-1].lon,
			sites[ i ].lat, sites[ i ].lon);
	    }
	  else
	    {
	      plot_sat(sites[i-1].lat, sites[i-1].lon,
			sites[ i ].lat, sites[ i ].lon);
	    }
	  if(bad_behind) /* Then stack them on the line. */
	    {
	      int ii;
	      GLdouble lat, lon;
	      between(sites[ i ].lat,sites[ i ].lon,
		      sites[i-1].lat,sites[i-1].lon,
		      &lat, &lon);
	      for(ii=0;ii<bad_behind;ii++)
		{
		  plot_site(i-bad_behind+ii, lat, lon, ii+1,
			    sites[i-bad_behind+ii].selected, ACC_NONE);
		}
	      bad_behind = 0;
	    }
	}
      else   /* This site and the one before it is very close */ 
	{
	  if(sites[i].accuracy == ACC_NONE)
	    bad_behind++;
	  else
	    {
	      if(bad_behind)
		{
		  int ii;
		  for(ii=0 ; ii<bad_behind ; ii++)
		    {
		      plot_site(i-bad_behind+ii, 
				sites[i].lat, 
				sites[i].lon, 
				1+sites_on_top++,
				sites[i-bad_behind+ii].selected,
				ACC_NONE);
		    }
		  bad_behind = 0;
		}
	      plot_site(i, sites[i].lat, sites[i].lon, 1+sites_on_top,
			sites[i].selected, sites[i].accuracy);
	      sites_on_top++;
	    }
	}
    }
  
  /* If there are still bad_behind, plot them on   */
  /* top of the last known site.                   */
  
  /* !draw , or MAX_SITES sites. */
  
  if(bad_behind)
    {
      int ii;

      for(ii=0 ; ii<bad_behind ; ii++)
	{
	  plot_site(i-bad_behind+ii,
		    sites[i-1].lat,
		    sites[i-1].lon,
		    1+sites_on_top++, 
		    sites[i-bad_behind+ii].selected,
		    ACC_NONE);
	}
      bad_behind = 0;
    }
  glEndList();

  if(render_mode != SELECT_MODE)
    redraw(GTK_WIDGET(glarea), NULL);
}
