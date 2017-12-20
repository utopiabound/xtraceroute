/* $Id: resolve.c,v 1.4 2003/02/22 23:17:09 d3august Exp $
 */
/*  xtraceroute - graphically show traceroute information.
 *  Copyright (C) 1996-1998  Bj�rn Augustsson 
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

#include "xt.h"
#include <ctype.h>
#include <string.h>
#ifdef HAVE_GEOIP
#include <GeoIPCity.h>
#endif

extern const countrycode countries[];
extern const int n_countries;

static int
scanHostDB(database *db, site* key)
{
    int i;
  
    if(!db || db->n_entries == 0) /* No database */
	return FALSE;

    /* Future: Hash here. Or tree, or whatever.
       Make it faster anyway. */
  
    for(i=0 ; i<db->n_entries ; i++)
	if(!strcasecmp(key->name, db->entries[i].name) ||
	   addrInAddr(&db->entries[i].ip, &key->ip))
	{
	    key->lat      = db->entries[i].lat;
	    key->lon      = db->entries[i].lon;
	    key->draw     = 1;
	    return TRUE;
	}
    return FALSE;
}

static int
scanNetDB(database *db, site* key)
{
    int i;
  
    if(!db || db->n_entries == 0) /* No database */
	return FALSE;

    for(i=0 ; i < db->n_entries ; i++) {
	if (addrInAddr(&db->entries[i].ip, &key->ip)) {
	    key->lat      = db->entries[i].lat;
	    key->lon      = db->entries[i].lon;
	    strcpy(key->info, db->entries[i].info);
	    key->draw     = 1;
	    return TRUE;
	}
    }
    return FALSE;
}


static int
scanGenericDB(database *db, site* key)
{
    int i;

    if(!db || db->n_entries == 0) /* No database */
	return FALSE;

    for(i=0 ; i<db->n_entries ; i++)
	if(isin(db->entries[i].name,  key->name))
	{
	    key->lat      = db->entries[i].lat;
	    key->lon      = db->entries[i].lon;
	    key->draw     = 1;
	    strcpy(key->info, db->entries[i].info);
	    return TRUE;
	}
    return FALSE;
}

#ifdef HAVE_GEOIP
static int
scanGeoIPDB(site* key)
{
    int i;
    GeoIP *gi;
    GeoIPRecord *gir = NULL;
    int rc = FALSE;

    int search4[] = { GEOIP_CITY_EDITION_REV0,
		      GEOIP_CITY_EDITION_REV1,
		      -1 };
    int search6[] = { GEOIP_CITY_EDITION_REV0_V6,
		      GEOIP_CITY_EDITION_REV1_V6,
		      -1 };
    int *search = NULL;

    /* IPv4 check */
    if (key->ip.type == AF_INET)
	search = search4;
    else if (key->ip.type == AF_INET6)
	search = search6;
    else /* bad ip.type */
	return FALSE;

    for (i = 0; search[i] >= 0; ++i) {
	if (!GeoIP_db_avail(search[i]))
	    continue;
	gi = GeoIP_open_type(search[i], GEOIP_STANDARD | GEOIP_SILENCE);
	if (gi != NULL)
	    break;
    }
    if (gi == NULL)
	return FALSE;

    if (key->ip.type == AF_INET)
	gir = GeoIP_record_by_ipnum(gi, ntohl(key->ip.addr.in.s_addr));
    else // if (key->ip.type == AF_INET6)
	gir = GeoIP_record_by_ipnum_v6(gi, key->ip.addr.in6);

    if (gir != NULL) {
	rc = TRUE;
	key->lat = gir->latitude;
	key->lon = gir->longitude;
	key->draw = 1;
	snprintf(key->info, sizeof(key->info), "%s, %s, %s", gir->city, gir->region, gir->country_code);
    }
    GeoIP_delete(gi);
    return rc;

}
#else
static int
scanGeoIPDB(site* key)
{
    return FALSE;
}
#endif


/**
 * resolve() tries to associate a site with a position.
 */
void
resolve(site* site)
{
    int i;
    char suffix[6];

    if(!isdigit((int)site->name[0])) {
	/* Call external program to look up RFC1876 data if it isn't
	   an IP number, but a proper DNS name.  Names can't begin
	   with a digit. */
      
	callExternalDNS(site);
    }

    /*  Check for the name/IP in the user files  */

    if(scanHostDB(local_user_hosts, site))
    {
	site->accuracy = ACC_LOCAL_USER;
	return;
    }

    if(scanNetDB(local_user_nets, site))
    {
	site->accuracy = ACC_LOCAL_USER;
	return;
    }
    /// GENERIC

    /*  Check for the name/IP in the site-wide files  */
  
    if(scanHostDB(local_site_hosts, site))
    {
	site->accuracy = ACC_LOCAL_SITE;
	return;
    }
    if(scanNetDB(local_site_nets, site))
    {
	site->accuracy = ACC_LOCAL_SITE;
	return;
    }
    // GENERIC


    /*  Check for the name/IP in the NDG host cache.  */
  
    if(scanHostDB(ndg_hosts, site))
    {
	site->accuracy = ACC_NDG_HOST;
	return;
    }
    /* Check for the IP in the NDG net cache.  */
  
    if(scanNetDB(ndg_nets, site))
    {
	site->accuracy = ACC_NDG_NET;
	return;
    }
  
    /*  Is this the "home" site?  */
    /*
      if(site_nr == 0)
      tell_user(_("You should enter better\ndata for your site!"));
    */
  
    /*  OK, let's see if we can find a name we know in the name of the
	site. (Search the internal database.) */
  
    /* First search the user generic database */
  
    if(scanGenericDB(local_user_generic, site))
    {
	site->accuracy = ACC_LOCAL_USER;
	return;
    }

    /* Then search the internal database */

    if(scanGenericDB(internal, site))
    {
	site->accuracy = ACC_INCLUDED;
	return;
    }

    if(scanGeoIPDB(site))
    {
	site->accuracy = ACC_GEOIP;
	return;
    }

    /*  As a last resort, give it a location based on the suffix.  */

    getsuff(site->name, suffix);
    for(i=0;i<n_countries;i++)
	if(!strcasecmp(suffix, countries[i].suffix))
	{
	    site->lat      = countries[i].lat;
	    site->lon      = countries[i].lon;
	    site->draw     = 1;
	    site->accuracy = ACC_SUFFIX;
	    strcpy(site->info, _(countries[i].name));
	    return;
	}
  
    /*  OK, admit failure then...  */

    site->lat      = 0.0;
    site->lon      = 0.0;
    site->draw     = 1;
    site->accuracy = ACC_NONE;
    strcpy(site->info,_("I don't know where this is!"));
    return;
}


void
resolve_by_id(int site_nr, char *buf)
{
    site* site;
    static int user_nagged = FALSE;

    site = &(sites[site_nr]);

    if (buf != NULL)
	str2addr(&site->ip, buf);

    resolve(site);
  
    if(site_nr == 0 && site->accuracy < ACC_INCLUDED &&
       user_nagged == FALSE)
    {
	//tell_user(_("You should enter better\ndata for your site!"));
	user_nagged = TRUE;
    }
    if(site->accuracy == ACC_NONE) /* If it didn't find anything. */
    {
	/* Set the location to the same as the previous site. */

	site->lat      = sites[site_nr-1].lat;
	site->lon      = sites[site_nr-1].lon;
    }
    return;
}

/**
 * Goes over the sites and (re)arranges the locations of unknown sites
 * (Useful if the prevoius site was relocated for some reason.)
 */

void 
arrangeUnknownSites(void)
{
    int i;

    for(i=0 ; i<MAX_SITES ; i++)
    {
	if(sites[i].accuracy == ACC_NONE) {
	    /* Set the location to the same as the previous site. */
	  
	    sites[i].lat = sites[i-1].lat;
	    sites[i].lon = sites[i-1].lon;
	}
    }
}
