/* $Id: main.c,v 1.22 2003/03/25 00:30:47 d3august Exp $
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
#include <GL/glx.h>
#include <GL/glu.h>
#include <gtkgl/gtkglarea.h>
#include <sys/stat.h>
#include <string.h>
#include <netdb.h>
#include <sys/utsname.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>    // for sigaction
#include <sys/wait.h>  // for waitpid
#include <unistd.h>    // for close
#include <stdlib.h>    // for atoi
#include "trackball.h"

GtkWidget *clist;               /* The list of sites below the globe */

GdkPixbuf *earth_texture;
GdkPixbuf *night_texture;
GdkPixbuf *created_texture;

int xbegin, ybegin, zbegin;
int xbeginstrafe, ybeginstrafe;

GLfloat zoom = EARTH_SIZE;

float curquat[4];
GLint vp[4];                      /*  viewport  */

int newModel = 1;  /* Do we need to recalc the modelview matrix? */

site local;  /* For resolving where localhost is (to rotate earth right) */

static const char* versionstring = N_("Xtraceroute version ");

/**
 * Called to reset the sites.
 */
static void 
clear_sites(void)
{
  int i;
  
  for(i=0;i<MAX_SITES;i++)
    {
      sites[i].draw = 0;
      if(sites[i].extpipe_active == TRUE)   /* Kill any lookups in progress */
	{
	  if(sites[i].extpipe_pid != 0)
	    {
	      kill(sites[i].extpipe_pid, SIGTERM);
	      //printf("Killed %ld\n", sites[i].extpipe_pid);
	      
	    }
	}
    }
  //  spinner_reset();
}

/** 
 * Recalculates the modelview matrix.
 * Call this when a user has rotated the globe with the mouse or similar.
 * If the world needs changing, call makeearth instead. 
 * (Zooming does this, the lines are constant size independent of zoom.)
 */

static void 
recalcModelView(void)
{
  GLfloat m[4][4];
 
  glPopMatrix();
  glPushMatrix();

  build_rotmatrix(m, curquat);
  glMultMatrixf(&m[0][0]);

  glScalef(zoom,zoom,zoom);
}


/** 
 * Used as a callback for expose events.
 */

void 
redraw(GtkWidget *wi, GdkEvent *gdk_event)
{
  if (!gtk_gl_area_make_current(GTK_GL_AREA(wi)))
    printf("make_current failed in redraw()\n");
  
  if (newModel)
    {
      recalcModelView();
      newModel = 0;
    }
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glCallList(WORLD);
  
  gtk_gl_area_swapbuffers(GTK_GL_AREA(wi));
}

/** 
 * Called when someone clicks on the globe. If he clicked on a site, it 
 * returns that site's number. Otherwise it returns -1. 
 */

static int 
which_site(GLint x, GLint y)
{
  typedef struct {
    GLuint num_names; 
    GLuint min_dist;
    GLuint max_dist;
    GLuint name;
  } hit_record;
  hit_record selectBuf[MAXSELECT/4];
  GLint hits = 0;
  GLdouble aspect;

  if (!gtk_gl_area_make_current(GTK_GL_AREA(glarea)))
      printf("make_current failed early in which_site()\n");

  //printf("which_site?\n");
  glSelectBuffer(MAXSELECT, (GLuint *) &selectBuf);
  glRenderMode(GL_SELECT);
  glInitNames();
  glPushName(NOT_SELECTABLE);
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  gluPickMatrix(x, glarea->allocation.height - y, 1, 1, vp);

  // Note: This code is common to some in reshape. Maybe look into that.

  aspect = (float)glarea->allocation.width / (float)glarea->allocation.height;
  if(glarea->allocation.width > glarea->allocation.height)
    {
      /* It's wide. */
      gluPerspective( 40.0,    /* Field of view, in "y" direction. */
                      aspect,  /* Aspect ratio */
                      1.0,     /* Z near       */
                      10.0);   /* Z far        */
    }
  else
    {
      /* It's high. */
      gluPerspective( 40.0/aspect,  /* Field of view, in "y" direction. */
                      aspect,       /* Aspect ratio */
                      1.0,          /* Z near       */
                      10.0);        /* Z far        */
    } 
  
  glMatrixMode(GL_MODELVIEW);
    
  set_render_mode(SELECT_MODE);
  makeearth();
  set_render_mode(NORMAL_MODE);
  
  glCallList(WORLD);
  hits = glRenderMode(GL_RENDER);
  
  /* Change back to projection here to pop my old matrix.
     Otherwise we'll look at the world through the pickmatrix
     in the future.            */
      
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);

  /* Process the hits, find the closest one */

  {
    GLuint closest_distance = ~0U;  // A really big number
    int retval = -1;
    int i;
    
    for(i=0 ; i<hits ; i++)
      {
  
        /* The hit records are potentially variable length, but I only
         * use one name per object. */
  
        if(selectBuf[i].num_names != 1)
          {
            printf("Ouch! More than one name in hit record!\n");
            return -1;
          }

        if(selectBuf[i].name != NOT_SELECTABLE)
          {
            if(selectBuf[i].min_dist < closest_distance)
              {
                 closest_distance = selectBuf[i].min_dist;
                 retval = selectBuf[i].name;
              }
          }
      }
    return retval;
  }
}

/**
 * reshape() : called whenever the window size changes.
 */

static void 
reshape(GtkWidget *wi, gpointer data)
{
  gint w, h;
  GLdouble aspect;

  if(!GTK_WIDGET_REALIZED (wi))
    {
      gtk_widget_queue_resize(wi);
      return;
    }
  if (!gtk_gl_area_make_current(GTK_GL_AREA(wi)))
    {
      printf("make_current failed in reshape()\n");
      return;
    }

  w      = wi->allocation.width;
  h      = wi->allocation.height;
  aspect = (float)w / (float)h;

  glViewport(0, 0, w, h);

  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glLoadIdentity();

  if(w > h)
    {
      /* It's wide. */
      gluPerspective( 40.0,    /* Field of view, in "y" direction. */
                      aspect,  /* Aspect ratio */
                      1.0,                /* Z near       */
                      10.0);              /* Z far        */
    }
  else
    {
      /* It's high. */
      gluPerspective( 40.0/aspect,  /* Field of view, in "y" direction. */
                      aspect,       /* Aspect ratio */
                      1.0,          /* Z near       */
                      10.0);        /* Z far        */
    } 
  glPushMatrix();
  glMatrixMode(GL_MODELVIEW);
  glGetIntegerv(GL_VIEWPORT, vp);
  gtk_gl_area_swapbuffers(GTK_GL_AREA(wi));
}


// have these un-globalized or put in a separate file later
gint rotation_track_tag    = 0;
gint translation_track_tag = 0;
gint zoom_track_tag = 0;


/**
 * mouse_motion(): Called when the mouse moves. (Callback.)
 */

static gint 
mouse_motion(GtkWidget *wi, GdkEventMotion *ev)
{
  gboolean any_change = FALSE;
  gboolean zoom_change = FALSE;
  float tempquat[4];
  gint x, y, w, h;
  gfloat strafex,strafey;
  GdkModifierType mods;
  gdk_window_get_pointer (wi->window, &x, &y, &mods);

  w = glarea->allocation.width;
  h = glarea->allocation.height;

  // Begin zoom stuff, sort this out later.
  //  if (zoom_track_tag && (zbegin > -1) )
  if (zoom_track_tag && (y != zbegin) )
    {
      double fakt;
      fakt = (1.0 - (2*(y-zbegin) / (double)h));
      //      printf("fact: %f, zoom: %f\n", fakt, zoom);
      zoom*=fakt;

      if (zoom > Z_OF_EYE - EARTH_SIZE)
	zoom=Z_OF_EYE - EARTH_SIZE;  /* prevents going inside the object */
      
      if (zoom <= EARTH_SIZE/4)
	zoom = EARTH_SIZE/4;         /* prevents going beyond "infinity" */
      
      zbegin=y;

      any_change = TRUE;
      zoom_change = TRUE;
    }

  /* Rotation: */

  /* This stuff doesn't really work well unless the 
     canvas is quadratic. */
  if(rotation_track_tag && (x != xbegin || y != ybegin) )
    {
      trackball(tempquat,
		(2.0*xbegin - w) / w,
		(h - 2.0*ybegin) / h,
		(2.0*x - w) / w,
		(h - 2.0*y) / h);
      
      add_quats(tempquat, curquat, curquat);

      xbegin = x;
      ybegin = y;
      
      any_change = TRUE;
    }

  /* Translation: */

  if(translation_track_tag && (x != xbeginstrafe || y != ybeginstrafe) )
    {
      /* Modify factor 4 to get a lower/higher sensitivity. */
      strafex = (gfloat) -4*(xbeginstrafe-x)/w;
      strafey = (gfloat)  4*(ybeginstrafe-y)/h;
      
      set_view_motion(strafex,strafey);
      xbeginstrafe = x;
      ybeginstrafe = y;

      any_change = TRUE;
    }


  if(zoom_change == TRUE)
    {
      /* We need to rebuild the earth in this case or the sites 
	 and lines would not be in proportion to the earth any more. */
      
      newModel = 1;
      makeearth();
    }  
  else if(any_change == TRUE)
    {
      newModel = 1;
      redraw(glarea, NULL); 
    }

  return TRUE;
}

/**
 * Called whenever any mouse button is pressed on the glarea.
 */

static gint 
mouse_button_down(GtkWidget *wi, GdkEventButton *ev)
{
  int site;
  int x = (int)ev->x;
  int y = (int)ev->y;
  
  switch(ev->button)
    {
    case 1:  /* Left button, rotation or selection */
      //      printf("Mouse 1 down!\n");
      
      /* In case the user is trying to select a site */
      site = which_site(x, y);
      if(site != -1)
	{
	  gtk_clist_select_row(GTK_CLIST(clist), site, 1);

	  /* Scroll the list here to make sure the listitem
	     is visible in the window. */	  
	  gtk_clist_moveto(GTK_CLIST(clist),site, 1, 0.5, 0.5);
	  break;
	}
      /* He wasn't, he's trying to rotate the earth. */

//printf("no site!\n");
      
      xbegin = x;
      ybegin = y;
      /*vafan ? FIXME*/          zbegin = -1;
      if(rotation_track_tag == 0)  /* To prevent duplicate callbacks. */
	rotation_track_tag = gtk_idle_add((GtkFunction)mouse_motion, wi);
      
      break;
    case 2: /* Middle button, translation */
      //      printf("Mouse 2 down!\n");

      xbeginstrafe = x;
      ybeginstrafe = y;
      if(translation_track_tag == 0)  /* To prevent duplicate callbacks. */
	translation_track_tag = gtk_idle_add((GtkFunction)mouse_motion, wi);
      break; 

     case 3: /* Right button, zoom */
       //       printf("Mouse 3 down!\n");
       
       zbegin = y;
       if(zoom_track_tag == 0)  /* To prevent duplicate callbacks. */
	 zoom_track_tag = gtk_idle_add((GtkFunction)mouse_motion, wi);
       break;
    }
  return TRUE;
}


/*-------------------------------------------------------------------------*/
/* mouse_button_up(): Called when any mouse button gets released.          */
/*-------------------------------------------------------------------------*/

static gint 
mouse_button_up(GtkWidget *wi, GdkEventButton *ev)
{
  if(ev->button == 1 && rotation_track_tag != 0) 
    { 
      gtk_idle_remove(rotation_track_tag);
      rotation_track_tag = 0;
      //      printf("Mouse 1 up!\n");
    }

 if(ev->button == 2 && translation_track_tag != 0) 
    { 
      gtk_idle_remove(translation_track_tag);
      translation_track_tag = 0;
      //      printf("Mouse 2 up!\n");
    }

 if(ev->button == 3 && zoom_track_tag != 0) 
    { 
      gtk_idle_remove(zoom_track_tag);
      zoom_track_tag = 0;
      //      printf("Mouse 3 up!\n");
    }

  return TRUE;
}


/**
 * Makes sure the binary we're going to use is really there.
 */

static void 
lookfor(const char* const program)
{
  struct stat statbuf;
  
  if(stat(program, &statbuf) < 0)
    {
      char tmp[501];
      g_snprintf(tmp,500,_("Can't find a binary I need!\n"
			   "I'm looking for \"%s\"."),program);
      perror(tmp);
      exit(EXIT_FAILURE);
    }

  if(! (statbuf.st_mode & S_IFREG) )
    {
      printf(_("Problem looking for \"%s\"! It isn't a regular file!\n")
	     ,program);

      exit(EXIT_FAILURE);
    }

  if(! (statbuf.st_mode & S_IXUSR) )
    {
      printf(_("%s isn't executable!\n"),program);

      exit(EXIT_FAILURE);
    }

  DPRINTF("%s\t is there and looks OK.\n", program);
}

/**
 * exit_program(): Guess?
 */

static void 
exit_program(GtkWidget *wi, gpointer *data)
{
  extern traceroute_state_t traceroute_state; // From extprog.c
  
  if(traceroute_state.fd[0] != -1)
    {
      close(traceroute_state.fd[0]);
    }
  gtk_main_quit();
}

/**
 * about_program()
 */

void 
about_program(GtkWidget *wi, gpointer *data)
{
  char mess[501];
  g_snprintf(mess,500,_("%s%s\nBy Björn Augustsson (d3august@dtek.chalmers.se)\n"
	  "Homepage: http://www.dtek.chalmers.se/~d3august/xt\n")
	  ,_(versionstring),VERSION);
  tell_user(mess);
}

/*-------------------------------------------------------------------------*/
/* clist_item_selected()                                                   */
/*-------------------------------------------------------------------------*/

static gint 
clist_item_selected(GtkCList *parentlist, gint row, gint column,
		    GdkEventButton *event)
{
  if (event && event->type == GDK_2BUTTON_PRESS)
    {
      info_window(row); /* Double click detected */
    }
  else
    {
      int i;

      if(sites[row].selected == 0)
        {
          /* Clear all other sites (so two sites can't be selected) */
          for(i=0;i<MAX_SITES;i++)
	    sites[i].selected = 0;
	  
          sites[row].selected = 1;
	  
          infowin_change_site(row);
	  
          makeearth();
        }
    }
  return TRUE;
}

/**
 * info_button_callback()
 */

static gint 
info_button_callback(GtkWidget *wi, gpointer *data)
{
  int i;
  
  for(i=0;i<MAX_SITES;i++)
    if(sites[i].selected)
      {
	info_window(i);
	return TRUE;
      }
  printf("Huh, no site selected!?\n");

  /* Get into info-about-next-item-mode */

  return TRUE;
}
#if 0
/*-------------------------------------------------------------------------*/
/* set_texture_name() : used to select a different map if a Map menu was   */
/* enabled.                                                                */
/*-------------------------------------------------------------------------*/
static void 
set_texture_name(gint number) 
{  
  char texname[200];

   strcpy(texname, DATADIR);
   strcat(texname,"/");
   strcat(texname, texture_name[number]);
   g_print("Trying to load map [%s] ... ",texname);
   earth_texture = readTexture(texname);
   g_print("done.\n");
}
#endif //0
/*-------------------------------------------------------------------------*/
/* choose_transparency() : wrapper for set_transparency                    */
/*-------------------------------------------------------------------------*/
void choose_transparency (gpointer data, guint action, GtkWidget *w) {
   set_transparency(action);
   redraw(GTK_WIDGET(glarea), NULL);
}
#if 0
/*-------------------------------------------------------------------------*/
/* choose_map()                                                            */
/*-------------------------------------------------------------------------*/
void choose_map (GtkWidget *w, gpointer data ) {
   set_texture_name((gint) data);
   /* re-afficher la terre avec texture */
   map_texture();
   redraw(GTK_WIDGET(glarea), NULL);
}
#endif //0
/*-------------------------------------------------------------------------*/
/* choose_zoom()                                                           */
/*-------------------------------------------------------------------------*/
void choose_zoom (gpointer data, guint action, GtkWidget *w) {

       set_zoom(action);  /* the type of zoom is passed to the gl impl. */
       newModel = 1;
       recalcModelView();
       makeearth();  /* makeearth calls redraw() */
}


/* Sets the lighting mode in the user settings.
 * Also updates the texture if neccessary.
 *
 * This is used as a callback.
 */

void set_lighting_mode(gpointer data, guint action, GtkWidget *w)
{

  // This is neccessary because the callback gets called when I 
  // set the defaults.
  
  if(! GTK_WIDGET_REALIZED(w))
    {
      return;
    }
  
  if(user_settings->lighting_mode == (lighting_t)action)
    {
      // It wasn't a change.
      return;
    }
     
  user_settings->lighting_mode = (lighting_t)action;
  
  if (user_settings->lighting_mode == DAY_AND_NIGHT)
    {
      composite_texture(night_texture, earth_texture, &created_texture);
      map_texture(created_texture);
    }
  else
    map_texture(earth_texture);
}

/*-------------------------------------------------------------------------*/
/* menu_items[] : The menu struct                                          */
/*-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*/
/* build_menu()                                                            */
/*-------------------------------------------------------------------------*/
static void 
build_menu( GtkWidget *window, GtkWidget **menubar )
{
  /* strings are marked with a preceding N_() for translation purpose
     see gettext()
  */
static GtkItemFactoryEntry menu_items[] = {
  { N_("/_File"),                    NULL,           NULL,                0, "<Branch>" },
  //  { N_("/File/_New..."),             "<control>O",   (GtkItemFactoryCallback)new_trace,   0, NULL },
  { "/File/sep1",                    NULL,           NULL,                0, "<Separator>" },
  { N_("/File/_Quit"),               "<control>Q",   exit_program,        0, NULL },

  { N_("/_Preferences"),             NULL,           NULL,               0, "<Branch>" },
  { N_("/Preferences/_Lighting"),    NULL,           NULL,               0, "<Branch>" },
  { N_("/Preferences/Lighting/Day and _night"),NULL, set_lighting_mode,  DAY_AND_NIGHT, "<RadioItem>" },
  { N_("/Preferences/Lighting/_Day only"), NULL,     set_lighting_mode,  DAY_ONLY,      "/Preferences/Lighting/Day and night" },
  
  { N_("/_Database"),                NULL,           NULL,                0, "<Branch>" },
  { N_("/Database/Add host..."),     NULL,           addHost,             0, NULL },
  { N_("/Database/Add net..."),      NULL,           addNet,              0, NULL },
  { N_("/Database/Add keyword..."),  NULL,           addGen,              0, NULL },

  /*        
  { N_("/_Map"),                     NULL,           NULL,                0, "<Branch>" },
  { N_("/Map/Map one"),              NULL,           choose_map,          0, NULL },
  { N_("/Map/Map two"),              NULL,           choose_map,          1, NULL },
  { N_("/Map/Map three"),            NULL,           choose_map,          2, NULL },
  */
  { N_("/_View"),                    NULL,           NULL,                0, "<Branch>" },
  { N_("/View/Zoom _In"),            "<control>i",   choose_zoom,         0, NULL },
  { N_("/View/Zoom _Out"),           "<control>o",   choose_zoom,         1, NULL },
  { N_("/View/Zoom In x2"),          NULL,           choose_zoom,         2, NULL },
  { N_("/View/Zoom Out x2"),         NULL,           choose_zoom,         3, NULL },
  { N_("/View/Restore default"),     NULL,           choose_zoom,         4, NULL },
  
  { N_("/_Transparency"),            NULL,           NULL,                0, "<Branch>" },
  { N_("/Transparency/Off"),         NULL,           choose_transparency, 0, NULL },
  { N_("/Transparency/On"),          NULL,           choose_transparency, 1, NULL },

  { N_("/_Help"),                    NULL,           NULL,                0, "<LastBranch>" },
  { N_("/_Help/About"),              NULL,           about_program,       0, NULL },
};

  gint nmenu_items = sizeof (menu_items) / sizeof (menu_items[0]);
  
  GtkAccelGroup *accel_group = gtk_accel_group_new();
  GtkItemFactory *item_factory = gtk_item_factory_new(GTK_TYPE_MENU_BAR, 
						       "<main>", accel_group);
  gtk_item_factory_create_items(item_factory, nmenu_items, menu_items, NULL);

  /* Set the preferences */
 
  switch(user_settings->lighting_mode)
    {
    case DAY_AND_NIGHT:
      gtk_check_menu_item_set_active(
          GTK_CHECK_MENU_ITEM (
              gtk_item_factory_get_item (item_factory,
					 "/Preferences/Lighting/Day and night")),
	  TRUE);
      break;
    case DAY_ONLY:
      gtk_check_menu_item_set_active(
	  GTK_CHECK_MENU_ITEM(
	      gtk_item_factory_get_item (item_factory,
					 "/Preferences/Lighting/Day only")),
	  TRUE);
      break;
    default:
      printf("Funky lighting mode making menu: %d\n", 
	user_settings->lighting_mode);
      exit(EXIT_FAILURE);
    }

  /* Attach the new accelerator group to the window. */
  gtk_window_add_accel_group (GTK_WINDOW (window), accel_group);
  
  if (menubar)
    *menubar = gtk_item_factory_get_widget (item_factory, "<main>");
}

/**
 * Usage...
 */

static void 
usage(void)
{
  g_print(_("%s%s\n\n"
	    "Usage: xtraceroute [--texture  <(day) texture file>]\n"
            "                   [--ntexture <night texture file>]\n"
            "                   [--night | --nonight]\n"
	    "                   [--lod <level-of-detail>]\n"
	    "                   [--stdin | - | <site> ]\n"
	    "                   [--version]\n"
	    "                   [--help]\n")
	  ,_(versionstring), VERSION);
}

/**
 * Handler for SIGCHLD. We get these when the extprogs die.
 */

static void 
childhandler(int sig, siginfo_t* sip, void* uap)
{  
  pid_t pid = sip->si_pid;
  extern traceroute_state_t traceroute_state;

  //  DPRINTF("pid: %ld died\n", (long)pid);
  
  if(traceroute_state.pid == pid)
    {
      traceroute_state.pid = 0;
      spinner_unref("traceroute");
    }
  else  // It was a host process that died.
    {
      int i;
      for(i=0 ; i<MAX_SITES ; i++)
	{
	  if(sites[i].extpipe_pid == pid)
	    {
	      spinner_unref("host");
	      sites[i].extpipe_active = FALSE;
	      sites[i].extpipe_pid    = 0;
              goto out;
	    }
	}
      /* Maybe it was the initial one for localhost. */
      if(local.extpipe_pid == pid)
	{
	  spinner_unref("localhost");
	  goto out;
	}
      
      DPRINTF("Huh? Who was pid %d?\n", (int)pid);
      spinner_unref("Unknown!");
    }
 out:
  waitpid(pid, NULL, 0);
}


/**
 * Adds a hostname to the lookup history in the combo widget.
 *
 * Maintains a 10-entry history.
 */

static void
combo_add_to_history(const char* name, GtkCombo* combo)
{
  static GList *items = NULL;
  char *newloc;

  if(!(newloc = (char*)malloc(100)))
    {
      printf("unable to allocate memory for newloc in history!\n");
      exit_program(NULL, NULL);
    }

  strncpy(newloc, name, 100);
  
  items = g_list_prepend(items, newloc);

  if(g_list_length(items) > 10)
    {
      char* item_to_remove = g_list_nth_data(items, 10);
      
      items = g_list_remove(items, item_to_remove);
      free(item_to_remove);
    }
  
  gtk_combo_set_popdown_strings (GTK_COMBO(combo), items);
}

#if 0
/**
 * Callback function for the combo widget in the top of the screen.
 * When you click on an entry in the history there, this gets called.
 * It starts a new trace and adds the name to the history (again).
 *
 * The "list" argument is the gtk_list part of the combo widget that
 * caused the signal, and "member" is the entry in the list.
 *
 * The "combo" argument is the combo widget itself.
 *
 * See also combo_entry_callback().
 */

static gint
combo_list_callback(GtkWidget *list, 
		    GtkWidget *member, 
		    gpointer  *combo)
{
  char* data;

  gtk_label_get(GTK_LABEL(GTK_BIN(member)->child), &data);

  DPRINTF("combo_list_callback called. member: %s\n", data);
  
  if(!strcmp(user_settings->current_target, data))
    {
      //     combo_add_to_history(data, GTK_COMBO(combo));
    }

  return(TRUE);
}

static gint
change_callback(GtkWidget *list, 
		    GtkWidget *member, 
		    gpointer  *combo)
{ printf("change\n"); return(TRUE);}
static gint
combochange_callback(GtkWidget *list, 
		    GtkWidget *member, 
		    gpointer  *combo)
{ printf("combo change\n"); return(TRUE);}


static gint
hide_callback(GtkWidget *list, 
		    GtkWidget *member, 
		    gpointer  *combo)
{ printf("hide\n"); return(TRUE);}

#endif //0

/**
 * Callback function for the combo widget in the top of the screen.
 * When you press enter there, this gets called. It starts a new 
 * trace and adds the name to the history.
 *
 * The "entry" argument is the gtk_entry part of the combo widget that
 * caused the signal.
 *
 * The "combo" argument is the combo widget itself.
 *
 * See also combo_list_callback().
 */

static gint
combo_entry_callback(GtkWidget *entry, gpointer *combo)
{
    int i;
    
    extern traceroute_state_t traceroute_state;  // From extprog.c
    
    /* Might make sense to have a better test for hostname
       validity here. Nasty things might happen if there's
       an ampersand character in the string for example, since
       it gets passed to the shell via popen().

       God I'm happy this won't run SUID root... :)     */

    /* Maybe tell_user here or something? */
    /*
    if(strlen(user_settings->current_target) < 2)
      {
	new_trace(NULL, NULL);
	return TRUE;
      }
    */

    combo_add_to_history(gtk_entry_get_text(GTK_ENTRY(entry)), 
			 GTK_COMBO(combo));
    strcpy(user_settings->current_target, gtk_entry_get_text(GTK_ENTRY(entry)));
    

    if(traceroute_state.fd[0] != -1)
      {
	gdk_input_remove(traceroute_state.tag); // Should this be here? Think so.
	close(traceroute_state.fd[0]);
	traceroute_state.fd[0] = -1;
      }
    traceroute_state.buffer_counter = 0;

    for(i=0;i<MAX_SITES;i++)
      {
	sites[i].draw = 0;
	sites[i].selected = 0;
	/* Kill all possible extprograms still running. */
	if(sites[i].extpipe_tag != 0)
	  {
	    gdk_input_remove(sites[i].extpipe_tag);
	    sites[i].extpipe_tag = 0;
	    if(sites[i].extpipe[0] != -1)
	      {
		close(sites[i].extpipe[0]);
		sites[i].extpipe[0] = -1;
	      }
	  }
	sites[i].extpipe_data_counter = 0;
      }
    /* Clear the whole list. */
    gtk_clist_clear(GTK_CLIST(clist));
    makeearth();
    calltrace();
    return TRUE;
}


/**
 * parse commandline arguments, and set relevant fields in the
 * user_settings struct.
 *
 * Might make sense to use getopt for this.
 */

static void 
parse_commandline(int argc, char *argv[])
{
  int has_target        = FALSE;
  int has_texture       = FALSE;
  int has_night_texture = FALSE;
  int has_night         = FALSE;
  int has_nonight       = FALSE;
  
  unsigned int i;
  for(i=1 ; i<argc ; i++)
    { 
      if(!strcmp(argv[i],"--version"))
	{
	  g_print("%s%s\n", _(versionstring), VERSION);
	  exit(EXIT_SUCCESS);
	}
      else if(!strcmp(argv[i],"--help")
	      || !strcmp(argv[i],"-h"))
	{
	  usage();
	  exit(EXIT_SUCCESS);
	}
      else if(!strcmp(argv[i],"--texture") 
	      || !strcmp(argv[i],"-T"))
	{
	  if(has_texture == TRUE)
	    g_print(_("Two (day) textures specified! Using second one.\n"));
	  
	  earth_texture = readTexture(argv[++i]);
	  has_texture   = TRUE;
	}
      else if(!strcmp(argv[i],"--ntexture") 
	      || !strcmp(argv[i],"-N"))
	{
	  if(has_night_texture == TRUE)
	    g_print(_("Two night textures specified! Using second one.\n"));
	  
	  night_texture     = readTexture(argv[++i]);
	  has_night_texture = TRUE;
	}
      else if(!strcmp(argv[i],"--night"))
	{
	  if(has_nonight == TRUE)
	    {
	      g_print("Both --night and --no-night specified!\n");
	      usage();
	      exit(EXIT_FAILURE);
	    }
	  user_settings->lighting_mode = DAY_AND_NIGHT;
	  has_night = TRUE;
	}
      else if(!strcmp(argv[i],"--no-night"))
	{
	  if(has_night == TRUE)
	    {
	      g_print("Both --night and --no-night specified!\n");
	      usage();
	      exit(EXIT_FAILURE);
	    }
	  user_settings->lighting_mode = DAY_ONLY;
	  has_nonight = TRUE;
	}
      else if(!strcasecmp(argv[i], "--stdin")
	      || !strcmp(argv[i], "-"))
	{
	  has_target = TRUE;
	  strcpy(user_settings->current_target, "-");
	}
      else if(!strcasecmp(argv[i],"--LOD"))
	{
	  set_sphere_lod(atoi(argv[++i]));
	}
      else // Has to be a site to traceroute or a bad option.
	{
	  if(argv[i][0] == '-')
	    {
	      printf("Unknown option \"%s\"\n", argv[i]);
	      usage();
	      exit(EXIT_FAILURE);
	    }
	  else if(has_target == TRUE)
	    {
	      printf("More than one site argument detected!\n");
	      usage();
	      exit(EXIT_FAILURE);
	    }
	  strcpy(user_settings->current_target, argv[i]);
	  has_target = TRUE;
	}
    }
}


/**
 * main()
 */

int 
main(int argc, char **argv)
{
  GtkWidget *window;
  GtkWidget *vbox;
  GtkWidget *hbox;
  GtkWidget *vbox2;
  GtkWidget *pane;
  GtkWidget *spinner;
  GtkWidget *info_button;
  GtkWidget *menubar;
  GtkWidget *notebook;
  GtkWidget *label;
  GtkWidget *combo_hbox;
  GtkWidget *combo_label;
  GtkWidget *combo;
  GtkWidget *dummyscrwin;
  
  gchar *titles[] =
  {
    N_("Nr"),
    N_("Hostname"),
    N_("IP number")
  };
  static int translated;

#ifdef ENABLE_NLS
  setlocale (LC_ALL, "");
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
#endif

  gtk_init(&argc, &argv);

  if (gdk_gl_query() == FALSE) 
    {
      g_print(_("OpenGL not supported\n"));
      exit(EXIT_FAILURE);
    }

  if (!(user_settings = (user_settings_t *)malloc(sizeof (user_settings_t))))
    {
      perror("Alloc space for settings");
      exit(EXIT_FAILURE);
    }

  /* These are the default settings. */

  user_settings->lighting_mode = DAY_ONLY;

  strcpy(user_settings->day_texname,   DATADIR "/earth.png");
  strcpy(user_settings->night_texname, DATADIR "/night.png");
  
  user_settings->LOD = 3;
  
  strcpy(user_settings->current_target,"");
  
  /* Handle loading rc file or similar in the future here. */
  
  parse_commandline(argc, argv);
  
  if(strlen(user_settings->current_target) > 0)
    {
      calltrace();
    }
  
  lookfor(TRACEPGM);   // The "traceroute" binary.
  lookfor(HOSTPGM);    // The "host" binary.
  lookfor(DATADIR "/xtraceroute-resolve-location.sh"); // the DNS helper

  earth_texture = readTexture(user_settings->day_texname);
  night_texture = readTexture(user_settings->night_texname);
  
  /*   Edouard
  DBs[USER][ HOSTS ] = readHostDB("user_hosts.cache");
  DBs[USER][ NETS  ] = readNetDB ("user_networks.cache");
  DBs[USER][GENERIC] = readGenDB ("user_generic.cache");

  DBs[SITE][ HOSTS ] = readHostDB("site_hosts.cache");
  DBs[SITE][ NETS  ] = readNetDB ("site_networks.cache");
  DBs[SITE][GENERIC] = readGenDB ("site_generic.cache");

  DBs[GLOBAL][ HOSTS ] = readHostDB("hosts.cache");
  DBs[GLOBAL][ NETS  ] = readNetDB ("networks.cache");
  DBs[GLOBAL][GENERIC] = readGenDB ("generic.cache");
  */

  ndg_hosts          = readHostDB("hosts.cache");
  local_site_hosts   = readHostDB("site_hosts.cache");
  local_user_hosts   = readHostDB("user_hosts.cache");
  
  //  writeHostDB(ndg_hosts, "/usr/scratch/host_apa");
  
  ndg_nets           = readNetDB("networks.cache");
  local_site_nets    = readNetDB("site_networks.cache");
  local_user_nets    = readNetDB("user_networks.cache");

  //  local_site_generic = readGenDB("generic.cache");
  //  local_site_generic = readGenDB("site_generic.cache");
  local_user_generic = readGenDB("user_generic.cache");

  /* Set up a signal handler for reaping dead children. */
  {
    struct sigaction sig;
    sig.sa_sigaction = childhandler;
    sig.sa_flags = SA_SIGINFO;
    sigaction(SIGCHLD, &sig, NULL);
  }
  
  internal = init_internal_db();
  clear_sites();
  
#ifdef XT_DEBUG
  {
    extern const int n_countries;
    
    DPRINTF(_("Known countries: %d\n"
	     "Built-in database: %d\n"), n_countries, internal->n_entries); 
  }
#endif /* XT_DEBUG */

  //  writeNetDB(ndg_nets, "/usr/scratch/net_apa");

  trackball(curquat, 0.0, 0.0, 0.0, 0.0);

  //    Set initial orientation of the globe.

  {
    float tmpquat[4];
    float vect[3] = {0.0, 1.0, 0.0};
    struct utsname  un;
    struct hostent* he;
    struct in_addr  in;

    memset(&local, 0, sizeof(site));

    uname(&un);
    strcpy(local.name, un.nodename);
    he = gethostbyname(un.nodename);
    if(!he)
      { 
        printf("Error gethostbynaming local hostname");
	printf("(Not connected to network?)\n");
      }
    else
      {
	memcpy(&in.s_addr, *(he->h_addr_list), sizeof(in.s_addr));
	sprintf(local.ip, "%s", inet_ntoa(in));
	
	resolve(&local);
	
	/* resolve starts a subprocess to look at DNS-LOC. Must block here 
	   until it completes. */
	
	while (local.extpipe_active != FALSE)
	  {
	   // 	printf(".");
	    get_from_extDNS(&local, local.extpipe[0], 
			    GDK_INPUT_READ|SYNCH_RESOLV);
	  }
	DPRINTF("resolved localhost.\n");
	
	if(local.accuracy == ACC_NONE)
	  {
	    printf("TIP:\tTo get xtraceroute to show your location centered on the globe\n"
		   "\twhen it starts up, add information about this host,\n"
		   "\t(%s) or your whole net.\n", local.name);
	    printf("\n\tOR, even better, make your sysadmin add a LOC record to the DNS.\n"
		   "\tThat way it will work for everyone else as well. Plus he gets to do\n" 
		   "\tthe work instead of you! See the README file for more info on this.\n");
	  }
	else
	  {
	    /* Rotate the globe to show the users' home. */
	    
	    axis_to_quat(vect, torad*local.lon, tmpquat);
	    add_quats(tmpquat, curquat, curquat);
	    
	    /* This is to do the "Y-axis", or latitude as well. */

	    vect[0] = 1.0;
	    vect[1] = 0.0;
	    vect[2] = 0.0;
	    axis_to_quat(vect, torad*-local.lat, tmpquat);
	    add_quats(tmpquat, curquat, curquat); 
	  }
      }
  }

  window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
  
  {
    char tmp[501];
    g_snprintf(tmp,500,"%s%s",_(versionstring),VERSION);
    gtk_window_set_title (GTK_WINDOW (window), tmp);
  }
  
  gtk_window_set_default_size ((GtkWindow *)window, 
			       DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT);
  
  gtk_window_set_policy ((GtkWindow *)window,
			 FALSE,  // allow shrink
			 TRUE,   // allow_grow
			 FALSE   // auto_shrink
			 );
  
  combo_hbox  = gtk_hbox_new(FALSE, 0);
  combo_label = gtk_label_new (_("Target: "));
  combo       = gtk_combo_new ();
  gtk_combo_disable_activate (GTK_COMBO(combo));
  
  gtk_signal_connect(GTK_OBJECT (GTK_COMBO(combo)->entry), "activate",
		     GTK_SIGNAL_FUNC (combo_entry_callback), GTK_COMBO(combo));
  
  /* OK, this _really_ sucks. What I want from my combo widget:
     
     A) When you type something into it and press enter, I get to know 
        what you wrote.
     B) When you select something from the dropdown, I get to know which
        one it was.

	This one gets called for each keypress or any other activity in 
        the entry part, which means it gets called if you move among the
        entries in the list w/ arrow keys.

   gtk_signal_connect(GTK_OBJECT (GTK_COMBO(combo)->entry), "changed",
		      GTK_SIGNAL_FUNC (change_callback), GTK_COMBO(combo));
		      
	This is a trick to get to know when the dropdown list closes. 
        Unfortunatly I can't tell if you chose an item, or just "closed 
        the list". Useless.

   gtk_signal_connect(GTK_OBJECT (GTK_COMBO(combo)->popwin), "hide",
                      GTK_SIGNAL_FUNC (hide_callback), GTK_COMBO(combo));

        This gets called when a list entry gets selected. (clicked) Or 
        when you move around in the list with the arrow keys. Sigh.   
   gtk_signal_connect(GTK_OBJECT (GTK_COMBO(combo)->list), "select-child",
		      GTK_SIGNAL_FUNC(combo_list_callback), GTK_COMBO(combo));
  */

  if(strlen(user_settings->current_target) > 2)
    {
      /* If we got a site to trace from the command line, add it to 
	 the history. */
      combo_add_to_history(user_settings->current_target, GTK_COMBO(combo));
    }

  gtk_box_pack_start(GTK_BOX(combo_hbox), combo_label, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(combo_hbox), combo, TRUE, TRUE, 0);

  /* FIXME  Fix this to work with pseudocolor visuals. */

  {
    int attribs[] = {GDK_GL_RGBA,
		     GDK_GL_RED_SIZE,1,
		     GDK_GL_GREEN_SIZE,1,
		     GDK_GL_BLUE_SIZE,1,
		     GDK_GL_DEPTH_SIZE,1,
		     GDK_GL_DOUBLEBUFFER,
		     GDK_GL_NONE};
    glarea = gtk_gl_area_new(attribs);
  }
  vbox    = gtk_vbox_new(FALSE,0);
  pane    = gtk_vpaned_new();

  // FIXME where the hell did this weird code come from?
  if (!translated){
        int i;
        for (i = 0; i < 3; i++)
            titles [i] = _(titles [i]);
        translated = 1;
  }

  /* This causes a warning, because GTK is broken. */
  clist = gtk_clist_new_with_titles(3, titles);

  gtk_clist_set_selection_mode((GtkCList *)clist, GTK_SELECTION_BROWSE);
  
  /* FIXME: These should probably be font-dependent. */
  
  gtk_clist_set_column_width (GTK_CLIST(clist), 0, 20);
  gtk_clist_set_column_width (GTK_CLIST(clist), 1, 230);
  gtk_clist_set_column_width (GTK_CLIST(clist), 2, 100);
  
  gtk_signal_connect(GTK_OBJECT(clist), "select_row",
		     GTK_SIGNAL_FUNC(clist_item_selected), NULL);
 
  gtk_signal_connect (GTK_OBJECT (window), "destroy",
		      GTK_SIGNAL_FUNC(exit_program), NULL);
  
  gtk_widget_set_usize(GTK_WIDGET(glarea), 
		       DEFAULT_WINDOW_WIDTH/4, 
		       DEFAULT_WINDOW_WIDTH/4);
  
  gtk_container_add(GTK_CONTAINER(window), vbox);
  
  gtk_widget_set_events(glarea, GDK_EXPOSURE_MASK
			| GDK_BUTTON_PRESS_MASK
			| GDK_BUTTON_RELEASE_MASK);
  
  gtk_signal_connect(GTK_OBJECT(glarea), "realize",
		     (GtkSignalFunc)init_gl,
		     (gpointer)NULL);
  gtk_signal_connect(GTK_OBJECT(glarea), "expose_event",
		     (GtkSignalFunc)redraw,
		     (gpointer)NULL);
  gtk_signal_connect(GTK_OBJECT(glarea), "button_press_event",
		     (GtkSignalFunc)mouse_button_down,
		     (gpointer)NULL);
  gtk_signal_connect(GTK_OBJECT(glarea), "button_release_event",
		     (GtkSignalFunc)mouse_button_up,
		     (gpointer)NULL);
  gtk_signal_connect(GTK_OBJECT(glarea), "motion_notify_event",
		     (GtkSignalFunc)mouse_motion,
		     (gpointer)NULL);
  gtk_signal_connect(GTK_OBJECT(glarea), "configure_event",
		     (GtkSignalFunc)reshape,
		     (gpointer)NULL);

  build_menu(window,&menubar);

  gtk_box_pack_start(GTK_BOX(vbox), menubar, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), combo_hbox, FALSE, FALSE, 0);
  gtk_box_pack_start(GTK_BOX(vbox), pane, TRUE, TRUE, 0);
  
  spinner = spinner_new();
  info_button = gtk_button_new_with_label(_("Info"));
  gtk_signal_connect(GTK_OBJECT(info_button), "clicked",
		     (GtkSignalFunc)info_button_callback,
		     (gpointer)NULL);
  
  hbox  = gtk_hbox_new(FALSE, 0);
  vbox2 = gtk_vbox_new(FALSE, 5);

  gtk_container_border_width(GTK_CONTAINER(vbox2), 5);

  dummyscrwin = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(dummyscrwin),
				 GTK_POLICY_AUTOMATIC,
				 GTK_POLICY_AUTOMATIC);
  gtk_widget_show(dummyscrwin);
  gtk_container_add(GTK_CONTAINER(dummyscrwin), clist);
  gtk_box_pack_start(GTK_BOX(hbox), dummyscrwin, TRUE, TRUE, 0);
  
  gtk_box_pack_start(GTK_BOX(hbox), vbox2,   FALSE, TRUE, 0);
  
  gtk_box_pack_start(GTK_BOX(vbox2), spinner, TRUE, TRUE, 0);
  gtk_box_pack_start(GTK_BOX(vbox2), info_button, TRUE, TRUE, 0);
  
  notebook = gtk_notebook_new ();
  
  gtk_notebook_set_tab_pos (GTK_NOTEBOOK (notebook), GTK_POS_TOP);
  gtk_widget_show(notebook);
  
  /*  FIXME! */
  /* Now let's add all the pages. All of these are in separate files,
     and have local data. The traceroute page must be moved out of here. */
  
  /* Traceroute part: */
  label = gtk_label_new (_("Traceroute"));
  gtk_notebook_append_page (GTK_NOTEBOOK (notebook), hbox, label);
  gtk_widget_set_usize(GTK_WIDGET(notebook), -1, DEFAULT_WINDOW_HEIGHT/4);
  
  // for all other pages: 
  //
  // widg = create_page(...)
  // label = gtk_label_new ("page name");  
  // gtk_notebook_append_page (GTK_NOTEBOOK (notebook), widg, label);
  
  gtk_paned_add1(GTK_PANED(pane), glarea);
  gtk_paned_add2(GTK_PANED(pane), notebook);

  /* This makes the glarea quadratic in the beginning. */
  gtk_paned_set_position(GTK_PANED(pane), DEFAULT_WINDOW_WIDTH);

  gtk_widget_show(clist);
  gtk_widget_show(glarea);
  gtk_widget_show(pane);
  gtk_widget_show(info_button);
  gtk_widget_show(spinner);
  gtk_widget_show(vbox2);
  gtk_widget_show(vbox);
  gtk_widget_show(hbox);
  gtk_widget_show(menubar);
  gtk_widget_show(combo_label);
  gtk_widget_show(combo);
  gtk_widget_show(combo_hbox);
  gtk_widget_show(window);
  
  makeearth();
  
  gtk_main();
  
  return 0;             /* ANSI C requires main to return int. */
}
