/* $Id: util.c,v 1.4 2003/03/22 14:39:04 d3august Exp $
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

#include <ctype.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "xt.h"

/**
 *  This one converts a string with a latitude or 
 *  longitude in "XXX YY ZZc" format, where
 *  XXX is the degree, YY is the minute, zz is the 
 *  second and c is s, n, e or w for direction into
 *  a double.
 *
 * That double is returned.
 */

double
locStrToNum(const char *str, int type)
{
  double factor = 1.0;
  double loc = 0;
  float sub;
  int i = 0;
  
  while(1)
    {
      int retval = sscanf(&str[i], "%f", &sub);
      if(retval == 0 || retval == EOF)
	{
	  /* Trouble! */
	  printf("Xtraceroute: locStrToNum[1]: got bad LOC data!\n");
	  printf("\tData: \"%s\"\n", str);
	  return(-1000);
	}
      loc += sub*factor;

      // Skip all spaces and tabs
      while(is_whitespace(str[i]))
	  i++;

      // Skip all numbers (including decimal point)
      while(isdigit((int)str[i]) || str[i] == '.')
	  i++;
      
      switch(type)
        {
	case LATITUDE:
	  if(toupper(str[i]) == 'N' || 
	     (str[i] == ' ' && toupper(str[i+1]) == 'N') )
	    {
	      return loc;
	    }
	  else if(toupper(str[i]) == 'S' || 
		  (str[i] == ' ' && toupper(str[i+1]) == 'S'))
	    {
	      return -loc;
	    }
	  break;

	case LONGITUDE:
	  if(toupper(str[i]) == 'E' || 
	     (str[i] == ' ' && toupper(str[i+1]) == 'E'))
	    {
	      return loc;
	    }
	  else if(toupper(str[i]) == 'W' || 
		  (str[i] == ' ' && toupper(str[i+1]) == 'W'))
	    {
	      return -loc;
	    }
	  break;
	}

      factor /= 60.0;

      if (factor <= 1.0 / (60.0 * 60.0 * 60.0))
	{
	  /* We've gotten more than 3 number fields! */
	  /* Malformed data, this is not a proper lat/lon string! */
	  printf("Xtraceroute: locStrToNum[2]: got bad LOC data!\n");
	  printf("\tData: \"%s\"\n", str);
	  return(-2000);	  
	}
    }
  /* Not reached */
}

/**
 *  This one converts a double with a latitude or 
 *  longitude into a "XXX YY ZZc" format, where
 *  XXX is the degree, YY is the minute, zz is the 
 *  second and c is s, n, e or w for direction. 
 *
 * That string is returned.
 */

char *
locNumToStr(double loc, int type)
{
  char *str;
  int deg, min, sec;
  char direction;
  
  str = (char *)malloc(12);

  switch(type)
    {
    case LATITUDE:
      if(loc < 0)
	{
	  direction = 's';
	  loc = -loc;
	}
      else
	direction = 'n';
      break;
    default:  // LONGITUDE
      if(loc < 0)
	{
	  direction = 'w';
	  loc = -loc;
	}
      else
	direction = 'e';
      break;
    }

  deg = (int)floor(loc);  

  loc -= floor(loc);  
  loc *= 60.0;
  
  min = (int)floor(loc);
  
  loc -= floor(loc);
  loc *= 60.0;
  
  sec = (int)floor(loc);
  
  sprintf(str,"%d %d %d%c", deg, min, sec, direction);
  return str;
}

/**
 * The following three functions returns the x-, y-, and z-coordinate
 * respectively for the specified point on the surface.
 * (These are often used together: maybe make a combined function?)
 */

double 
tox(double lat, double lon)
  {  return(sin(lon*torad)*cos(lat*torad)); }

double 
toy(double lat, double lon)
  {  return(sin(lat*torad));  }

double 
toz(double lat, double lon)
  {  return(cos(lon*torad)*cos(lat*torad)); }

/**
 * tolon() returns the longitude of a point on the unit sphere.
 * (Longitudes are the "x" axis on a mercator projection map.)
 */

double 
tolon(double x, double y, double z)
{
  double s;
  if(y == 1.0 || y == -1.0)
    return(5000.0);    /* Problem! The texture will warp! Actually these */
  s = sqrt(1 - y*y);   /* points (the poles) are on ALL the longitudes! */
  return((atan2(x/s,z/s) * todeg));
}

/**
 * tolat() returns the latitude of a point on the unit sphere.
 * (Latitudes are the "y" axis on a mercator projection map.)
 */

double 
tolat(double x, double y, double z)
{
  return(atan2(y, sqrt(1 - y*y)) * todeg);
}

/**
 * distance returns the actual theoretical minimum 
 * distance (in meters) the signal has to travel. 
 */

unsigned long 
distance(int site)
{
  int i;
  unsigned long distance_covered = 0;
  
  for(i=1 ; i<=site ; i++)
    {
      /* This is not really the way to do this. This assumes that
	 earth is round (it isn't!). I'm sure there's a good way to
	 compute the actual distance between two locations on earth. */
      
      /*    Angle is arccos(Ax*Bx + Ay*By + Az*Bz)    */
      
      float angle = acos((float)
			 (tox(sites[ i ].lat,sites[ i ].lon)
			  *tox(sites[i-1].lat,sites[i-1].lon)
			  +toy(sites[ i ].lat,sites[ i ].lon)
			  *toy(sites[i-1].lat,sites[i-1].lon)
			  +toz(sites[ i ].lat,sites[ i ].lon)
			  *toz(sites[i-1].lat,sites[i-1].lon)))
	* todeg;
      
      distance_covered += (int)rint((angle/360.0) * PHYSICAL_EARTH_CIRC);
    }
  return distance_covered;
}

/**
 * isin() returns 1 if a is in b  (sort of like grep)    
 */

int 
isin(const char a[],const char b[])
{
 int i;
 int a_len    = strlen(a);
 int len_diff = strlen(b) - a_len;

/*   'grep' backwards, since most DB entries are endings, like 
     "chalmers.se". Performance opt! Yeah!  */

 for(i=len_diff;i>=0;i--)
   if(!strncasecmp(a,&b[i],a_len))
     return(1);
 return(0);
}

/**
 * getsuff puts the suffix (whatever is afer the last '.' in the string)
 * of site a in b. (Or "None" if there's no '.') 
 */

void 
getsuff(const char a[], char b[])
{
 int i;
 for(i=strlen(a);i>0;i--)
   if(a[i]=='.')
    {
     strcpy(b, &a[i+1]);
     return;
    }
 strcpy(b, "None");
 return;
}

/**
 * This is a debug stub. See xt.h.
 */

#ifndef XT_DEBUG
int dontprintf(const char *format, ...)
{
	return (0);
}
#endif /* !XT_DEBUG */

