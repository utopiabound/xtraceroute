/* $Id: astronomy.c,v 1.2 2003/03/19 23:39:21 d3august Exp $ */
/*  xtraceroute - graphically show traceroute information.
 *  Copyright (C) 1996-2003  Björn Augustsson 
 *  Copyright (C) 2002 Hari Nair <hari@alumni.caltech.edu>
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

#include <time.h>
#include <math.h>
#include "xt.h"

/* Most of this code was cheerfully stolen from the excellent xplanet
 * by Hari Nair <hari@alumni.caltech.edu>
 * see http://xplanet.sourceforge.net/ 
 * 
 * Some of this code is (to me anyway) pretty scary. There are _a_lot_ 
 * of magic numbers in here, and I don't understand all of it. So don't 
 * expect me to explain this stuff to you... :)
 *
 * /August.
 */

static double
julian(int year, int month, int day, int hour, int min, int sec)
{
    if(month < 3)
    {
        year -= 1;
        month += 12;
    }

    // assume it's the Gregorian calendar (after 1582 Oct 15)
    {
      int a = year/100;
      int b = 2 - a + a/4;
      int c = (int)(365.25 * year);
      int d = (int)(30.6001 * (month + 1));
      double e = day + ((sec/60. + min) / 60. + hour) / 24.;
      
      double jd = b + c + d + e + 1720994.5;
      
      return(jd);
    }
}

static double
julianCentury(const time_t tv_sec)
{
  double jd = julian(gmtime(&tv_sec)->tm_year + 1900,
		     gmtime(&tv_sec)->tm_mon + 1,
		     gmtime(&tv_sec)->tm_mday,
		     gmtime(&tv_sec)->tm_hour,
		     gmtime(&tv_sec)->tm_min,
		     gmtime(&tv_sec)->tm_sec);
  
  return((jd - 2415020)/36525);
}

/* Really scary function. */
static double
poly(const double a1, const double a2, const double a3, const double a4,
     const double t)
{
    return(a1 + t*(a2 + t*(a3 + t*a4)));
}

/* Solve kepler's equation. */
static double
kepler(const double e, const double M)
{
    double E = M;
    double delta = 1.0;
    
    while (fabs(delta) > 1E-10)
      {
        delta = (M + e * sin(E) - E)/(1.0 - e * cos(E));
        E += delta;
      }
    return(E);
}

static double
gmst(const double T, const time_t tv_sec)
{
    // Sidereal time at Greenwich at 0 UT
    double g = poly(6.6460656, 2400.051262, 0.00002581, 0, T);

    // Now find current sidereal time at Greenwich
    double currgmt = (gmtime(&tv_sec)->tm_hour
                      + gmtime(&tv_sec)->tm_min/60.
                      + gmtime(&tv_sec)->tm_sec/3600.);
    currgmt *= 1.002737908;
    g += currgmt;
    g = fmod(g, 24.0);
    if (g < 0) g += 24;
    return(g);
}


/* Return the current sun position, in eclipitic coodinates. */
/* Based on Chapter 18 of Astronomical Formulae for Calculators by Meeus */
static void
sunpos(const double T, double* sunlat, double* sunlon)
{   
    double L = (poly(2.7969668E2, 3.600076892E4, 3.025E-4, 0, T)
                * torad);
    double M = (poly(3.5847583E2, 3.599904975E4, -1.5E-4, -3.3E-6, T)
                * torad);
    /* Eccentricity of the Earth's orbit. */
    double ecc = poly(1.675104E-2, -4.18E-5, -1.26E-7, 0, T);
    
    /* Eccentric anomaly. */
    double eccanom = kepler(ecc, M);
    double nu = 2.0 * atan(sqrt((1.0+ecc) / (1.0-ecc)) * tan(eccanom / 2.0));
    double theta = L + nu - M;

    *sunlon = theta;
    *sunlat = 0.0;
    //    sundist = 1.0000002 * (1 - ecc * cos(eccanom));
}

/**
 * This converts a position (lat, lon) in the ecliptic plane (the sun's 
 * plane) to a "normal" position, ie the position on earth where the 
 * sun is in zenith.
 *
 * NOTE: All lat, lon values are in radians! 
 *
 * I could optimize this a bit. Since I'm always interested in the suns's
 * position, the ec_lat value is always zero. Doesn't matter though.
 */

static void
eclipticToEquatorial(const double ec_lon, const double ec_lat, 
		     const double eps,
                     double *eq_lon, double *eq_lat)
{
    *eq_lat = asin(sin(eps)*sin(ec_lon)*cos(ec_lat) + sin(ec_lat)*cos(eps));

    *eq_lon = atan2(cos(eps)*sin(ec_lon) - tan(ec_lat)*sin(eps), cos(ec_lon));
}

/* I have no idea what this does... */
/* 23.452294 might be angle (in degrees) between the earths' and the 
   suns' orbital planes. Or something. */
static double
calcObliquity(const double T)
{
  return (poly(23.452294, -1.30125E-2, -1.64E-6, 5.03E-7, T)
	  * torad);
}


/**
 * Figure out where the sun is, from current time 
 *
 * NOTE: The lat, lon values "returned" are in radians!
 */

void 
get_sun_position(double* lat, double* lon)
{
  const time_t t   = time(NULL);
  // gmtime here?
  const double jt  = julianCentury(t);
  const double eps = calcObliquity(jt);
  double ec_lat, ec_lon;
  double mylat, mylon;
  double gst = 2.0*M_PI * gmst(jt, t) / 24;

  sunpos(jt, &ec_lat, &ec_lon);
  /* Ah, but these are eclipitic coodinates! Better convert them. */

  eclipticToEquatorial(ec_lon, ec_lat, eps, &mylon, &mylat);
  
  mylon -= gst;
  /* These (lat, lon) values are in radians. */ 
  
  *lat = mylat;
  *lon = mylon;
}
