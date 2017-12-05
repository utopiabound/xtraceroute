/* $Id: tweaks.h,v 1.2 2003/02/22 23:17:10 d3august Exp $
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

#ifndef __xt_tweaks_h__
#define __xt_tweaks_h__


/**********************************************
 *         Performance-tweaking options.      *
 **********************************************/

/*  Uncomment the line below to use an alternative way to draw the lines 
    connecting the sites. If clipping planes are very expensive on your
    hardware this might be much faster. Also it's much uglier.
    (Actually I don't use clipping planes any more, so this is pretty moot.
    It gives _some_ preformance, but it's very marginal.)*/

/* #define UGLY_LINES */

/*  Uncomment the line below to disable goraud shading. Flat shading is
    uglier, and goraud shading is very fast on most hardware anyway, but
    this might help for software-only renderers (Mesa etc)  */

/* #define FLAT_SHADING */

/* Another performance tip (not compiletime though) might be to use a smaller
   texture for the earth's surface. ( That is, load it into something like
   "xv", scale it down, and save it as PNG. )
    Also look into the --LOD command line option.*/

/**********************************************
 *              Other options.                *
 **********************************************/

/* Uncomment this one to get more realistic locations for satellites. The kind
   of sattelites used for this kind of thing are AFAIK all geostationary, and
   they are always over the equator. Since I don't _know_ where they are, I 
   make a guess. The default way to draw them is inaccurate in the sense that
   they don't neccesarily end up over the equator, but doing it this way means
   that sometimes the sat-ground line will go thru the earth, which obviously
   isn't correct either. */

/* #define SAT_OVER_EQUATOR */



#endif                  /* __xt_tweaks_h__ */
