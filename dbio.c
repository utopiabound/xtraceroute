/* $Id: dbio.c,v 1.7 2003/03/02 13:41:41 d3august Exp $
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

/*  This file implements support for NDG Software's geoboy cache files.
 * 
 *  (I'm using them too, now.)
 */

#include "xt.h"
#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>

/**
 *  Add an entry to a networks database. 
 */

void
addToNetDB(database *db, const dbentry *entry)
{
  int i;
  
  /* Scan the db to see if this is a change or a new entry */
  
  for(i=0 ; i<db->n_entries ; i++)
    if(!strcasecmp(entry->name, db->entries[i].name))
      {
	db->entries[i].lat = entry->lat;
	db->entries[i].lon = entry->lon;
	strcpy(db->entries[i].info, entry->info);
	
	/* Save the file */
	writeNetDB(db, "user_networks.cache");
	return;
      }
  
  db->entries = (dbentry *)realloc(db->entries, (db->n_entries+1)*sizeof(dbentry));
  db->n_entries++;
  
  db->entries[db->n_entries-1].lat = entry->lat;
  db->entries[db->n_entries-1].lon = entry->lon;
  strcpy(db->entries[db->n_entries-1].info, entry->info);
  memcpy(&db->entries[db->n_entries-1].ip, &entry->ip, sizeof(entry->ip));
  strcpy(db->entries[db->n_entries-1].name, entry->name);
  
  /* save the file */
  writeNetDB(db, "user_networks.cache");
  
  return;	 
}    

/**
 *  Add an entry to a generic database. 
 */

void
addToGenDB(database *db, const dbentry *entry)
{
  int i;
  
  /* Scan the db to see if this is a change or a new entry */
  
  for(i=0 ; i<db->n_entries ; i++)
    if(!strcasecmp(entry->name, db->entries[i].name))
      {
	db->entries[i].lat = entry->lat;
	db->entries[i].lon = entry->lon;
	strcpy(db->entries[i].info, entry->info);
	
	/* Save the file */
	writeGenDB(db, "user_generic.cache");
	return;
      }
  
  db->entries = (dbentry *)realloc(db->entries, (db->n_entries+1)*sizeof(dbentry));
  db->n_entries++;
  
  db->entries[db->n_entries-1].lat = entry->lat;
  db->entries[db->n_entries-1].lon = entry->lon;
  strcpy(db->entries[db->n_entries-1].info, entry->info);
  strcpy(db->entries[db->n_entries-1].name, entry->name);
  
  /* save the file */
  writeGenDB(db, "user_generic.cache");
  
  return;	 
}    

/**
 *  Add an entry to a hosts database. 
 */

void
addToHostDB(database *db, const dbentry *entry)
{
  int i;
  
  /* Scan the db to see if this is a change or a new entry */
  
  for(i=0 ; i<db->n_entries ; i++)
    if(!strcasecmp(entry->name, db->entries[i].name))
      {
	db->entries[i].lat = entry->lat;
	db->entries[i].lon = entry->lon;
	strcpy(db->entries[i].info, entry->info);
	
	/* Save the file */
	writeHostDB(db, "user_hosts.cache");
	return;
      }
  
  db->entries = (dbentry *)realloc(db->entries, ((db->n_entries)+1)*sizeof(dbentry));
  db->n_entries++;
  
  db->entries[db->n_entries-1].lat = entry->lat;
  db->entries[db->n_entries-1].lon = entry->lon;
  strcpy(db->entries[db->n_entries-1].info, entry->info);
  memcpy(&db->entries[db->n_entries-1].ip, &entry->ip, sizeof(entry->ip));
  strcpy(db->entries[db->n_entries-1].name, entry->name);
  
  /* save the file */
  writeHostDB(db, "user_hosts.cache");
  
  return;	 
}    


/**
 *  dbopen() works like fopen() but it'll fix the paths for
 *  the files:
 *
 *  If the name begins with a slash it will be interpreted
 *    as an absolute path.
 *  If the name begins with "user", "$HOME/.xt/" will be
 *    prepended to the filename.
 *  Otherwise the file will be searched for in the 
 *    DATADIR directory.
 */

static FILE *
dbopen(const char *dbname, const char *mode)
{
    FILE *fp;
    char fn[200];

    if(dbname[0] == 'u' &&
       dbname[1] == 's' &&
       dbname[2] == 'e' &&
       dbname[3] == 'r')
    {
	strcpy(fn, getenv("HOME"));
	strcat(fn, "/.xt/");

	/* Check that the directory $HOME/.xt exists and is a directory. */
	if(!strcmp(mode, "w")) {
	    struct stat sb;
	    if(stat(fn, &sb))
		mkdir(fn, 0700);    // Too paranoid?
	    else if(!(sb.st_mode&S_IFDIR)) {
		printf("Ouch! $HOME/.xt (\"%s\") exists but is not a directory!\n", fn);
		printf("      Can't open \"%s\".\n", dbname);
		return NULL;
            }
        }

	strcat(fn, dbname);
    }
    else if(dbname[0] == '/')
	strcpy(fn, dbname);
    else {
	strcpy(fn, DATADIR);
	strcat(fn, "/");
	strcat(fn, dbname);
    }
    fp = fopen(fn, mode);
    if(!fp) {
	DPRINTF("Can't open the database file %s!(In mode %s)\n", fn, mode);
	return NULL;
    }
    return fp;
}

/**
 * Writes a host database to disk.
 */

int
writeHostDB(database *db, const char *filename)
{
    FILE *fh;
    int i;
    char ip[70];

    fh = dbopen(filename, "w");
    if(!fh)
	return(FALSE);
  
    for(i=0 ; i<db->n_entries ; i++) {
	dbentry *entry = &(db->entries[i]);
	char *lat = locNumToStr(entry->lat, LATITUDE);
	char *lon = locNumToStr(entry->lon, LONGITUDE);
	addr2str(&entry->ip, ip, sizeof(ip));

	fprintf(fh,"%s %s %s %s\n", ip, entry->name, lat, lon);
	free(lat); /* Bigtime memleak fixed... */
	free(lon);
	//      if(i != db->n_entries-1) /* Last one */
	//	fprintf(fh,"\n");
    }

    fclose(fh);
    return TRUE;
}

/**
 * Writes a network database to disk.
 */

int
writeNetDB(database *db, const char *filename)
{
    FILE *fh;
    int i;
    char ip[70];

    fh = dbopen(filename, "w");
    if(!fh)
	return(FALSE);
  
    for(i=0 ; i<db->n_entries ; i++)
    {
	dbentry *entry = &(db->entries[i]);
	char *lat = locNumToStr(entry->lat, LATITUDE);
	char *lon = locNumToStr(entry->lon, LONGITUDE);

	addr2str(&entry->ip, ip, sizeof(ip));
	int tabs = (15-strlen(ip))/8;
	int loclen;

	fprintf(fh, "%s ", ip);
	while(tabs-- >= 0)
	    putc('\t', fh);

	loclen = fprintf(fh, "%s %s", lat, lon);

	tabs = (31-loclen)/8;
	while(tabs-- >= 0)
	    putc('\t', fh);

	fprintf(fh, "#%s\n", entry->info);

	free(lat);
	free(lon);

      //      if(i != db->n_entries-1) /* Last one */
      //	fprintf(fh,"\n");
    }

  fclose(fh);
  return TRUE;
}

/**
 * Writes a generic database to disk.
 */
int
writeGenDB(database *db, const char *filename)
{
  FILE *fh;
  int i;

  fh = dbopen(filename, "w");
  if(!fh)
    return(FALSE);
  
  for(i=0 ; i<db->n_entries ; i++)
    {
      dbentry *entry = &(db->entries[i]);
      char *lat = locNumToStr(entry->lat, LATITUDE);
      char *lon = locNumToStr(entry->lon, LONGITUDE);
      /*      int tabs = (15-strlen(entry->name))/8; */
      int loclen;
      
      loclen = fprintf(fh, "%s %s", lat, lon);
      
      /*      tabs = (31-loclen)/8;
      while(tabs-- >= 0)
      */
      putc('\t', fh);

      fprintf(fh, "%s", entry->name);
      /* while(tabs-- >= 0)
       */
      putc('\t', fh);
      
	
      fprintf(fh, "#%s\n", entry->info);

      free(lat);
      free(lon);

      //      if(i != db->n_entries-1) /* Last one */
      //	fprintf(fh,"\n");
    }

  fclose(fh);
  return TRUE;
}


/**
 *  Reads a location (lat or lon depending on the type
 *  argument from a FILE and returns the value.
 */

static double
readLocation(FILE *fp, int type)
{
  char location[15];
  int j = 1;
  int a,A,b,B;
  int tmp = ' ';

  a = A = b = B = 0;
  switch(type)
    {
    case LATITUDE:
      a = 'n';
      b = 's';
      A = 'N';
      B = 'S';
      break;
    case LONGITUDE:
      a = 'w';
      b = 'e';
      A = 'W';
      B = 'E';
      break;
    default:
      printf("ERROR: readLocation: arg 2 not LATITUDE or LONGITUDE\n");
      break;
    }
  
  /* Skip all whitespace */

  while(is_whitespace(tmp))
    tmp = fgetc(fp);
  
  location[0] = tmp;
  
  while((tmp = fgetc(fp)) != EOF && j < sizeof(location)) {
      location[j++] = tmp;
      if(location[j-1] == a ||
	 location[j-1] == b ||
	 location[j-1] == A ||
	 location[j-1] == B)
	break;
  }
  
  location[j] = '\0';  
  return locStrToNum(location, type);
}


/**
 * reads a host database file, returns a pointer to the 
 * database struct.
 */

database *
readHostDB(const char *dbfile)
{
    int i;
    FILE *fp;
    int a;
    int n_entries = 0;
    dbentry *entry;
    database *hostdb;
    char ip[IP_STR_LEN];

    hostdb = (database *)malloc(sizeof(database));
    if(!hostdb) {
	perror("Couldn't malloc memory for hosts cache!\n");
	return NULL;
    }

    if(!(fp = dbopen(dbfile, "r"))) {
	hostdb->n_entries = 0;
	hostdb->entries = NULL;
	DPRINTF("readHostDB(%s) for %d entries\n", dbfile, hostdb->n_entries);
	return hostdb;
    }
    DPRINTF("Found a %s with ", dbfile);
    while((a = fgetc(fp)) != EOF)
	if(a == '\n')
	    n_entries++;
    DPRINTF("%d entries.\n", n_entries);

    if(n_entries > 0) {
	hostdb->entries = (dbentry *)malloc(n_entries*sizeof(dbentry));
	if(!hostdb->entries) {
	    perror("Couldn't malloc memory for hosts cache.\n");
	    return NULL;
	}
    }
    rewind(fp);

    for(i=0; i<n_entries; i++) {
	entry = &(hostdb->entries[i]);
 
	/* Get the IP-address and name */

	fscanf(fp, "%s %s", ip, entry->name);
	str2addr(&entry->ip, ip);

	/* Get the location. */
	entry->lat = readLocation(fp, LATITUDE);
	entry->lon = readLocation(fp, LONGITUDE);
    }
  
    hostdb->n_entries = n_entries;
    fclose(fp);

    DPRINTF("readHostDB(%s) for %d entries\n", dbfile, hostdb->n_entries);
    return hostdb;
}

/*****
 *
 * Reads a user generic database
 *  Format :
 *  LAT LONG name   # Comment
 */

database *
readGenDB(const char *dbfile)
{
    int i, j;
    FILE *fp;
    int a, tmp;
    int n_entries = 0;
    dbentry *entry;
    database *gendb;

    gendb = (database *)malloc(sizeof(database));
    if(!gendb) {
	perror("Couldn't malloc memory for generic cache!\n");
	return NULL;
    }
  
    if(!(fp = dbopen(dbfile, "r"))) {
	gendb->n_entries = 0;
	gendb->entries = NULL;
	DPRINTF("readGenDB(%s) for %d entries\n", dbfile, gendb->n_entries);
	return gendb;
    }
    DPRINTF("Found a %s with ", dbfile);
    while((a = fgetc(fp)) != EOF)
	if(a == '\n')
	    n_entries++;
    DPRINTF("%d entries.\n", n_entries);
  
    if(n_entries > 0) {
	gendb->entries = (dbentry *)malloc(n_entries*sizeof(dbentry));
	if(!gendb->entries) {
	    perror("Couldn't malloc memory for generic cache.\n");
	    return NULL;
	}
    }
    rewind(fp);
  
    for(i=0;i<n_entries;i++) {
	entry = &(gendb->entries[i]);
      
	/* Get the location. */
      
	entry->lat = readLocation(fp, LATITUDE);
	entry->lon = readLocation(fp, LONGITUDE);

	/* Get the name */
	fscanf(fp, "%s", entry->name);

	/* Strip all whitespace */
	tmp = ' ';
	while(is_whitespace(tmp))
	    tmp = fgetc(fp);
      
	/* Get the info. */
	j = 0;
	tmp = fgetc(fp);
	while(tmp != '\n' && tmp != EOF && j < 50) {
	    entry->info[j++] = tmp;
	    tmp = fgetc(fp);
	}
	entry->info[j] = '\0';
    }

    gendb->n_entries = n_entries;
    fclose(fp);

    DPRINTF("readGenDB(%s) for %d entries\n", dbfile, gendb->n_entries);
    return gendb;
}

/**
 * reads a networks database file, returns a pointer 
 * to the database struct.
 */

database *
readNetDB(const char *dbfile)
{
    int i,j;
    FILE *fp;
    int a, tmp;
    int n_entries = 0;
    dbentry *entry;
    database *netdb;
    char ip[IP_STR_LEN];

    netdb = (database *)malloc(sizeof(database));
    if(!netdb) {
	perror("Couldn't malloc memory for net cache!\n");
	return NULL;
    }

    if(!(fp = dbopen(dbfile, "r"))) {
	netdb->n_entries = 0;
	netdb->entries = NULL;
	DPRINTF("readNetDB(%s) for %d entries\n", dbfile, netdb->n_entries);
	return netdb;
    }
    DPRINTF("Found a %s with ", dbfile);
    while((a = fgetc(fp)) != EOF)
	if(a == '\n')
	    n_entries++;
    DPRINTF("%d entries.\n", n_entries);

    netdb->entries = (dbentry *)malloc(n_entries*sizeof(dbentry));
    if(!netdb->entries) {
	perror("Couldn't malloc memory for net cache.\n");
	return NULL;
    }
    rewind(fp);

    for(i=0; i < n_entries; i++) {
	entry = &(netdb->entries[i]);
      
	/* Get the IP-number.*/
	fscanf(fp, "%s", ip);

	str2addr(&entry->ip, ip);

	/* Get the location. */
	entry->lat = readLocation(fp, LATITUDE);
	entry->lon = readLocation(fp, LONGITUDE);

	/* Strip all whitespace */
	tmp = ' ';
	while(is_whitespace(tmp) && !feof(fp))
	    tmp = fgetc(fp);
	/* ungetc(tmp, fp); // Don't need to ungetc; it's the hash sign. */

	/* Get the info. */
	if (feof(fp))
	    break;

	j = 0;
	tmp = fgetc(fp);
	while(tmp != '\n' && tmp != EOF && j < 50) {
	    entry->info[j++] = tmp;
	    tmp = fgetc(fp);
	}
	entry->info[j] = '\0';
    }

    netdb->n_entries = n_entries;

    fclose(fp);

    DPRINTF("readNetDB(%s) for %d entries\n", dbfile, netdb->n_entries);
    return netdb;
}
