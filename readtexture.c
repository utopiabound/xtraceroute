/* $Id: readtexture.c,v 1.12 2003/03/21 01:10:49 d3august Exp $
*/
/*  xtraceroute - graphically show traceroute information.
 *  Copyright (C) 1996-2003  Björn Augustsson 
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 *  02111-1307, USA.
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "xt.h"

/**
 * Readtexture does just what the name suggests.
 */

GdkPixbuf *
readTexture(const char *imgname)
{
  GdkPixbuf *img;
  GError *error = NULL;

  DPRINTF("Loading texture from file \"%s\"... ", imgname);

  img = gdk_pixbuf_new_from_file(imgname, &error);
  if (img == NULL) 
    {
      fprintf(stderr, "Couldn't load texture file %s\n", imgname);
      exit(EXIT_FAILURE);
    }

  DPRINTF("Done!\n\tTexture size is %dx%d\n",
	  gdk_pixbuf_get_width(img), gdk_pixbuf_get_height(img));

  return img;
}


extern void 
get_sun_position(double* , double*);


#define DAY 0
#define NIGHT 0xFF
#define SET_ALPHA(x, y, val) *(pt+4*(width*y+x)+3) = val
#define SET_WHOLE_ROWS(from, to, val) \
 {                                    \
   int y;                             \
   for(y=from ; y<to ; y++)           \
     {                                \
       int x;                         \
       for(x=0 ; x<width ; x++)       \
          SET_ALPHA(x, y, val);       \
     }                                \
  }


/**
 * A high alpha value in the night texture means it's dark
 * Most of this code taken from xplanet. See astronomy.c.
 */
static void
update_alpha_in_texture(GdkPixbuf* tex)
{
  int y;
  int width  = gdk_pixbuf_get_width(tex);
  int height = gdk_pixbuf_get_height(tex);
  guchar* pt = gdk_pixbuf_get_pixels(tex);
  int polar_rows, inoon;
  
  float lat_per_row = 180.0/(float)height;
  double sunlat, sunlon;
  
  get_sun_position(&sunlat, &sunlon);

  DPRINTF("SUN:   lat: %2.2f     lon: %2.2f\n", sunlat, sunlon);

  /* number of rows at top and bottom that are in polar day/night */
  polar_rows = abs((int) (sunlat/lat_per_row));
  
  if (sunlat < 0) // North pole is dark
    {
      SET_WHOLE_ROWS(0,                 polar_rows, NIGHT);
      SET_WHOLE_ROWS(height-polar_rows, height,     DAY);
      }
  else            // South pole is dark
    {
      SET_WHOLE_ROWS(0,                 polar_rows,   DAY);
      SET_WHOLE_ROWS(height-polar_rows, height,       NIGHT);
    }
  
  // subsolar longitude pixel column - this is where it's noon
  inoon = (int) (width/2 * (sunlon / M_PI - 1));
  while (inoon < 0) inoon += width;
  while (inoon >= width) inoon -= width;
  
  for (y=polar_rows ; y<height-polar_rows ; y++)
    {
      double length_of_day, H0;
      int ilight, dark_pixels;
      
      /* compute length of daylight as a fraction of the day at 
       * the current latitude.  Based on Chapter 42 of Astronomical 
       * Formulae for Calculators by Meeus. */
      
      H0 = tan(M_PI_2 - M_PI*(float)y/(float)height) * tan(sunlat*torad);
      if (H0 > 1) 
	length_of_day = 1;
      else if (H0 < -1) 
	length_of_day = 0;
      else 
	length_of_day = 1.0 - (acos(H0) / M_PI);
      
      /* ilight = number of pixels from noon to the terminator. */
      ilight = (int) (width/2 * length_of_day + 0.5);
      
      /* dark_pixels = number of pixels that are in darkness at the current 
	 latitude */
      dark_pixels = width - 2 * ilight;
      
      // start at the evening terminator
      {
	int pos = inoon + ilight;
	int j;
	
	for (j=0 ; j<dark_pixels ; j++)
	  {
	    if (pos >= width) 
	      pos -= width;
	    SET_ALPHA(pos, y, NIGHT);
	    pos++;
	  }
	for (j=dark_pixels ; j<width ; j++)
	  {
	    if (pos >= width) 
	      pos -= width;
	    SET_ALPHA(pos, y, DAY);
	    pos++;
	  }
      }
    }
}


/**
 * Makes a composite texture from the day and night images, according to 
 * what time it is.
 *
 * dest needs to be at least as big as the biggest one of the night and
 * day textures.
 *
 * Note: Allocates memory. (For the dest argument.)
 */

void
composite_texture(GdkPixbuf* night, const GdkPixbuf* day, GdkPixbuf** dest)
{
  int dest_w = MAX(gdk_pixbuf_get_width(night),  gdk_pixbuf_get_width(day)); 
  int dest_h = MAX(gdk_pixbuf_get_height(night), gdk_pixbuf_get_height(day)); 

  
  if(!gdk_pixbuf_get_has_alpha(night))
    night = gdk_pixbuf_add_alpha(night, FALSE, 0,0,0);
  
  update_alpha_in_texture(night);

  
  {

#if 0
  // Testpattern.
    pt += 3;

    for(i=0; i < height ; i++)
      {
	for(j=0; j < width ; j++)
	  {
	    *(pt) = j%255;
	    
	    pt += 4;
	  }
      }
#endif //0 
  }

  /* Copy day texture to dest */
  
  *dest = gdk_pixbuf_scale_simple(day, dest_w, dest_h, GDK_INTERP_HYPER);
  
  /* Composite night onto dest */
  {
    double scale_x = (double)gdk_pixbuf_get_width(*dest)  / (double)gdk_pixbuf_get_width(night);
    double scale_y = (double)gdk_pixbuf_get_height(*dest) / (double)gdk_pixbuf_get_height(night);

    gdk_pixbuf_composite(night, *dest,
			 0,0,            // dest_x, dest_y,
			 dest_w, dest_h,
			 0.0, 0.0,       // offset_x, offset_y,
			 scale_x, scale_y, 
			 GDK_INTERP_HYPER,
			 255);    
  }

  return;
}
