/* $Id: extprog.c,v 1.11 2003/03/25 00:21:46 d3august Exp $
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


#include "xt.h"
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>

traceroute_state_t traceroute_state;

extern GtkWidget *clist;    // from main.c

/**
 * get_from_extDNS collects data from the external program into a per-site
 * buffer. When the pipe runs out of data, it closes it.
 *
 * It is used as a callback.
 *
 * The fd argument is the same thing as site->extpipe[0].
 */

gint 
get_from_extDNS(site* site, int fd, GdkInputCondition cond)
{
  int count;
  char c;

  if(!(cond & GDK_INPUT_READ))
    {
      DPRINTF("Huh? Not read?\n");
      return FALSE;
    }
  
  count = read(fd, &c, 1);   //FIXME future performance opt, read more than a byte at a time.
  
  /* This can happen, it's a nonblocking pipe */
  if (count == -1)
    { 
      if(errno != EAGAIN)
	perror("read in get_from_extdns"); 

      if(site->extpipe_data[strlen(site->extpipe_data)-1] == '\n' )
	{
          /* Trigger the test to abort if we're resolving localhost. 
             Add that to here? */
          site->extpipe_active = FALSE;
	  return FALSE;
	}
      else
	return TRUE; 
    }
 
  if(count == 0)
    {
      DPRINTF("wow");
      return TRUE;
    }
 
  if(c == 0)
    {
      DPRINTF("wow2");
      return TRUE;
     }
 
  site->extpipe_data[site->extpipe_data_counter++] = c;
  
  if(c == '\n')
    {
      int i = 0;
      site->extpipe_data[site->extpipe_data_counter] = '\0';
      
      /* Parse the data. If the first word is "Nothing", abort now. */

      if(!strncmp(site->extpipe_data, "Nothing", strlen("Nothing")))
	goto escape;
      else if(!strncmp(site->extpipe_data, "Strange host version", 
		       strlen("Strange host version")))
        {
	  printf("Strange version of the \"host\" program found!\n"
		 "I can't use this, see the man page for details.\n");
	  exit(EXIT_FAILURE);
        }

      /*  Otherwise skip it. */
      while(!is_whitespace(site->extpipe_data[i]))
	i++;
      /* Now skip all whitespace after that. */
      while(is_whitespace(site->extpipe_data[i]))
	i++;
      
      if(!strncmp(&site->extpipe_data[i], "LOC", 3))
	{
	  float tmplat, tmplon;
	  i += 3;
	  
	  /* Actually do something with the data. */
	  
	  tmplat = locStrToNum(&site->extpipe_data[i], LATITUDE);
	  if(tmplat < -900)
	    {
	      /* It was an illegal LOC string! */
	      goto escape;
	    }
	  site->lat = tmplat;

	  while(toupper(site->extpipe_data[i]) != 'N' && 
		toupper(site->extpipe_data[i]) != 'S')
	    {
	      i++;
	    }
	  i++;
	  
	  tmplon = locStrToNum(&site->extpipe_data[i], LONGITUDE);
	  if(tmplon < -900)
	    {
	      /* It was an illegal LOC string! */
	      goto escape;
	    }
	  site->lon = tmplon;

	  site->accuracy = ACC_RFC_1876;
	  DPRINTF("Whee! LOC data for: %s\n", site->name);
	  strcpy(site->info, "No further info.");
	  
escape:
	  gdk_input_remove(site->extpipe_tag); 
	  close(fd);
	  site->extpipe[0]           = -1;
	  site->extpipe_tag          = 0;
	  site->extpipe_data_counter = 0;
	  site->extpipe_active       = FALSE;
	  
	  if( !(cond & SYNCH_RESOLV) )
	    {
	      arrangeUnknownSites();
	      makeearth();
	    }
	  return FALSE;
	}
      else if(i==1)
	{
	  /* Not supposed to happen. It's probably a bug if it does. */
	  DPRINTF("Hmmm. An empty line...\n");
          DPRINTF("\tData:\"%s\"\n", site->extpipe_data);
	}
      else   /* Strange data */
	{
	  printf("Didn't get expected in the string from host helper!\n");
	  printf("This is what I got: \"%s\"\n", site->extpipe_data);
	}
      site->extpipe_data_counter = 0;
    }
  return TRUE;
} 


/**
 * callExternalDNS() calls an external program (at this point "host") and
 * sets up a per-site pipe to its' output.
 */

void callExternalDNS(site* site)
{
  if(pipe(site->extpipe) == -1)
    {
      perror("pipe"); 
      exit(EXIT_FAILURE);
    }

  if(fcntl(site->extpipe[0], F_SETFL, O_NONBLOCK) != 0)
    {
      perror("O_NONBLOCKifying pipe");
      exit(EXIT_FAILURE);
    }

  /* Make a distinction here host/localhost here? */

  spinner_ref("host");

  site->extpipe_pid = fork();

  switch (site->extpipe_pid)
    {
    case -1:
      /* Fork failed! */
      perror("fork");
      exit(EXIT_FAILURE);
      break;
    case 0:      
      /* We're going to be the exernal program! */
      
      close(site->extpipe[0]);    /* close read end of pipe               */
      //      close(STDERR_FILENO);       /* close stderr. I'm not interested.    */
      dup2(site->extpipe[1], STDOUT_FILENO); /* make 1 same as write-to end of pipe  */
      dup2(site->extpipe[1], STDERR_FILENO); /* make 2 same as write-to end of pipe  */
      close(site->extpipe[1]);    /* close excess fildes                  */

      execlp(DATADIR "/xtraceroute-resolve-location.sh", 
             "xtraceroute-resolve-location.sh", site->name, NULL); 
      perror("exec");       /* still around?  exec failed           */
      _exit(EXIT_FAILURE);  /* no flush                             */ 
      break;
    default:
      /* We're the parent */
      site->extpipe_active = TRUE;
      site->extpipe_tag = gdk_input_add(site->extpipe[0], GDK_INPUT_READ,
					(GdkInputFunction)get_from_extDNS, site);
    }
}


/*********************************************************************/


/**
 * Only called from get_from_traceroute() when it gets a whole line.
 */

static void 
parse_row_from_traceroute(char *input)
{
  int i;
  int no;
  int last;
  char *clistrow[3];

  //  DPRINTF("O");

  /* This is a SGI compatibility thing; On SGI "traceroute" prints
     the first line "traceroute to test.com (0.0.0.0), 40 hops max, 
     40 byte packets" to stdout instad of stderr (which most other
     machines does). FWIW, I actually think SGI's behavior is saner 
     (stderr is for errors!), but... */
  /* On the other hand, SGI-traceroute prints errors as well to 
     stdout... Oh well... */
  
  if(!strncasecmp("traceroute",input, 10))
    {
      DPRINTF("Got traceroute header!\n");

      if(isin("unknown host", input))
	{
	  char tmp[200];
	  DPRINTF("unknown host!\n");
	  gdk_input_remove(traceroute_state.tag);
	  close(traceroute_state.fd[0]);
	  traceroute_state.fd[0] = -1;
	  strcpy(tmp, _("xtraceroute: unknown host "));
	  strcat(tmp, user_settings->current_target);
	  tell_user(tmp);
	  traceroute_state.scanning = 0;
	}
      return;
    }
  else if(strlen(input)< 10)
    {
      /* All we got was a couple of spaces, from a "XXX * * *"-row, that's
	 had the stars stripped off */

      sscanf(input,"%d",&no);
      no--;
      strcpy(sites[no].name, "No response");
      strcpy(sites[no].ip  , "No response");
      strcpy(sites[no].info, "There were no response from this machine.");
      sites[no].lat = sites[no-1].lat; // FIXME:
      sites[no].lon = sites[no-1].lon; //potential trouble here if no == 0.
      sites[no].time = 0;
      sites[no].accuracy = ACC_NONE;
      sites[no].draw = 1;
    }
  else /* All is well, do the normal stuff. */
    {
      i = sscanf(input,"%d",&no);
      
      /* Skip past the first number. */
      while(!is_whitespace(input[i]))
	i++;
      no--;
      sscanf(input+i,"%s %*c%s %d",sites[no].name,sites[no].ip,&sites[no].time);
      
      /* Get rid of the trailing ')' from the IP... */
      sites[no].ip[ strlen(sites[no].ip) - 1 ] = '\0';

      resolve_by_id(no);
      makeearth();       /* FIXME maybe this should'nt be here. */
    }

  clistrow[0] = (char *)malloc(10);
  clistrow[1] = (char *)malloc(sizeof(sites[no].name));
  clistrow[2] = (char *)malloc(sizeof(sites[no].ip));
  
  sprintf(clistrow[0],"%d",no);
  sprintf(clistrow[1],"%s",sites[no].name);
  sprintf(clistrow[2],"%s",sites[no].ip);
  
  /* Add an entry to the list */
  last = gtk_clist_append(GTK_CLIST(clist), clistrow);
  
  /* Free the clistrow here */
  free(clistrow[0]);
  free(clistrow[1]);
  free(clistrow[2]);
  /* The above SHOULD be done! But there's an error here somewhere.
     Doing that causes malloc in glibc-linux to dump sometimes. 
     Investigating... This just has to be a glibc bug... Efence finds
     nothing... (This comment kept in case it shows up again.) */
  
  /* Scroll the clist so the new entry is visible */
  gtk_clist_moveto(GTK_CLIST(clist), last, 1, 1.0, 0.5);
  
  if(no >= MAX_SITES-1)
    {
      /*
	printf("Too many hops! Change the MAX_SITES value in xt.h,\n");
	printf("recompile, and mail August and tell him about it!\n");
      */
      close(traceroute_state.fd[0]);
      traceroute_state.fd[0]    = -1;
      traceroute_state.scanning = 0;
      gdk_input_remove(traceroute_state.tag);
      spinner_unref("traceroute");
      makeearth();
    }
  return;
}

/**
 * get_from_traceroute collects data from the external traceroute 
 * program into a buffer. When it has a whole line, it calls
 * parse_row_from_traceroute().
 *
 * When the pipe runs out of data, it gets closed.
 *
 * It is used as a callback.
 */

gint 
get_from_traceroute(char* nope, int fd, GdkInputCondition cond)
{
  //    DPRINTF(".");

  if(traceroute_state.scanning == 0)
    {
      /* FIX for API brokenness. This callback should unregister if it 
	 returns false, as all other callbacks (in gtk at least) do. */
      gdk_input_remove(traceroute_state.tag);      

      return FALSE;
    }
  if(cond & GDK_INPUT_READ)
    {
      int count;
      char c;

      count = read(fd, &c, 1);   //FIXME future performance opt, read more than a byte at a time.
      
      if(c == -1)  printf("Hmmm.. -1 from read.\n");

      if(c == 0)  printf("Hmmm.. This shouldn't happen.\n");

      if(count == 0)          /* Other end of pipe closed. */
	{
	  close(traceroute_state.fd[0]); // Hmmm... what if this is stdin?
	  traceroute_state.fd[0] = -1;
	  traceroute_state.scanning = 0;
	  makeearth();
	  spinner_unref("traceroute");
	  gdk_input_remove(traceroute_state.tag); /* Fix for bad API. See above. */
	  return FALSE;
	}

      if(c == '*')
	return TRUE;
      
      traceroute_state.row_so_far[traceroute_state.buffer_counter++] = c;
      
      if(c == '\n')
	{
	  traceroute_state.row_so_far[traceroute_state.buffer_counter] = '\0';
	  parse_row_from_traceroute(traceroute_state.row_so_far);
	  traceroute_state.buffer_counter = 0;
	}
    }
  return TRUE;
} 


/** 
 * Calls traceroute if the argument is 0, else reads debug input from stdin. 
 */

void 
calltrace(void)
{
  /*   I don't know if there are several versions of traceroute out there,
       but hopefully they can be configured to deliver what I want with this.
       Otherwise, this function must be hacked some more.  */
  
  char tracepgm[]   = TRACEPGM;
  
  DPRINTF("calltrace\n");

  if(!strncmp(user_settings->current_target, "-", strlen("-")))
    {
      traceroute_state.fd[0] = STDIN_FILENO;
      traceroute_state.scanning = 1;
      spinner_ref("traceroute");
      traceroute_state.tag = gdk_input_add(traceroute_state.fd[0], GDK_INPUT_READ,
					   (GdkInputFunction)get_from_traceroute, NULL);
      return;
    }
  
  if (pipe(traceroute_state.fd) == -1)
    {
      perror("traceroute pipe"); 
      exit(EXIT_FAILURE);
    }
  
  traceroute_state.pid = fork();
  
  switch (traceroute_state.pid)
    {
    case -1:
      /* Fork failed! */
      perror("fork");
      exit(EXIT_FAILURE);
      break;
    case 0:      
      /* We're going to be the exernal program! */
      
      close(traceroute_state.fd[0]);    /* close read end of pipe               */
      dup2(traceroute_state.fd[1], STDOUT_FILENO);   /* make 1 same as write-to end of pipe  */
      dup2(traceroute_state.fd[1], STDERR_FILENO);   /* make 2 same as write-to end of pipe  */
      close(traceroute_state.fd[1]);    /* close excess fildes                  */

      /* FIXME I'd _like_ to use the MAX_SITES define instead of "40"
	 here, but that doesn't work since it's in a string. 
	 Suggestions? */ 
      execlp(tracepgm, "traceroute", 
	     "-q", "2",  /* Two queries (default is 3.) */
	     "-w", "20", /* Time before timing out (default is 5) */ 
	     "-m", "40", /* Max hops, default is 30, we need more. */
	     user_settings->current_target, NULL);
      perror("exec");       /* still around?  exec failed           */
      _exit(EXIT_FAILURE);  /* no flush                             */ 
      break;
    default:
      /* We're the parent */
      traceroute_state.scanning = 1;
      spinner_ref("traceroute");
      traceroute_state.tag = gdk_input_add(traceroute_state.fd[0], GDK_INPUT_READ,
					   (GdkInputFunction)get_from_traceroute, NULL);
    }
}
