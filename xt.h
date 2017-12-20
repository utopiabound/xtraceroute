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

#ifndef __xt_h__
#define __xt_h__

#include "config.h"

#include <stdio.h>

#ifdef ENABLE_NLS
# include <locale.h>
#    include <libintl.h>
#    define _(String) gettext (String)
#    ifdef gettext_noop
#        define N_(String) gettext_noop (String)
#    else
#        define N_(String) (String)
#    endif
#else
/* Stubs that do something close enough.  */
#    define textdomain(String) (String)
#    define gettext(String) (String)
#    define dgettext(Domain,Message) (Message)
#    define dcgettext(Domain,Message,Type) (Message)
#    define bindtextdomain(Domain,Directory) (Domain)
#    define _(String) (String)
#    define N_(String) (String)
#endif

#include <sys/types.h>
#include <inttypes.h>
#include <gtk/gtk.h>
#include <gdk-pixbuf/gdk-pixbuf.h>
#include <netinet/in.h>

#include "tweaks.h"

#ifndef M_PI
#define M_PI	3.141592654
#endif

#ifndef M_PI_2
#define M_PI_2	(3.141592654/2.0)
#endif

/* radian <-> degree conversion factors. */

#define torad (M_PI/180.0f)
#define todeg (180.0f/M_PI)

#define MAX_SITES  40
#define Z_OF_EYE 4              /* s.g. distance between eye and object on z axis */
#define SITE_MARKER_SIZE  0.03
#define EARTH_SIZE 1.0
#define PHYSICAL_EARTH_RADIUS 6378137  /* Earth's radius, in meters. (From RFC1876) */
#define SPEED_OF_LIGHT 299792458       /* The big C, in meters/second. */
#define MAXSELECT 100
#define PHYSICAL_EARTH_CIRC (2.0 * M_PI * PHYSICAL_EARTH_RADIUS)
#define GEOSYNC_SAT_ALT 36000000 /* Approx. altitude of geosynchronous */
                                 /* satellites, in meters. */
#define NOT_SELECTABLE ~0U       /* For picking reasons, see which_site() */

/**
 *  This is a replacement for isspace(), since it doesn't work
 *  as expected in some locales.
 */

#define is_whitespace(c) (c == '\t' || c == ' ' || c == '\n')

/* Displaylist */

#define WORLD 1

/* For makeearth() */

#define NORMAL_MODE 0
#define SELECT_MODE 1

#define DEFAULT_WINDOW_WIDTH  450   /* Default window size */
#define DEFAULT_WINDOW_HEIGHT 640   /* Ought to be enough for everyone. */

struct address {
    int type; /**< AF_INET or AF_INET6 */
    int prefix; /**< IP/PREFIX */
    union {
	struct in6_addr in6;
	struct in_addr in;
	char buf[0]; /* for being fed into inet_ntop */
    } addr;
};

#define NET_PREFIX(_a) ((_a).type == AF_INET) ?32 :((_a).type == AF_INET6) ?128 :-1

#define IP_STR_LEN 70

typedef struct
{
    double lat;
    double lon;
    struct address ip;
    char name[70];
    char info[70];
} dbentry;

typedef struct
{
    double lat;
    double lon;
    char suffix[4];
    char name[70];
} countrycode;

typedef struct
{
    dbentry *entries;
    int n_entries;
} database;

typedef enum
{
  ACC_NONE, ACC_SUFFIX, ACC_INCLUDED, ACC_NDG_NET, ACC_NDG_HOST,
  ACC_RFC_1712, ACC_RFC_1876, ACC_LOCAL_SITE, ACC_LOCAL_USER, ACC_GEOIP
} accuracy_t;

typedef struct
{
    short draw;           /** Plot this site if draw. */
    double lat;           /** Latitude */
    double lon;           /** Longitude */
    accuracy_t accuracy;  /** How certain is the position? */
    char name[70];        /** Hostname */
    char info[70];
    struct address ip;
    int time;             /** In milliseconds. */
    int selected;         /** flag */
    int extpipe[2];       /** Pipe to an external RFC1876 checking program here */
    int extpipe_tag;      /** For keeping tabs on the pipe */
    int extpipe_active;   /** If this is zero, the extprog is done and the data parsed. */
    pid_t extpipe_pid;    /** pid of the child */
    int extpipe_data_counter;
    char extpipe_data[200];    /** Data from the pipe */
} site;

site sites[MAX_SITES];

typedef enum 
  {
    DAY_ONLY,
    DAY_AND_NIGHT
  } lighting_t;

typedef struct
{
  lighting_t  lighting_mode;
  char   day_texname[400];
  char night_texname[400];
  int LOD;      // level-of-detail of the sphere.
  char current_target[100];
} user_settings_t;

user_settings_t* user_settings;

typedef struct
{
  int tag;               /** Also used from dialogs.c */
  int scanning;          /** Are we waiting for input from traceroute? */  
  int fd[2];             /** The pipe to traceroute. */
  char row_so_far[300];
  int buffer_counter;
  pid_t pid;             /** PID of the child */
} traceroute_state_t;

enum
{
  LATITUDE, LONGITUDE
};

/* Choosen to be orthogonal to the GDK_INPUT_{READ,WRITE,EXCEPTION} #defines
   from gtk. */ /* Yes, this is a hack. */
 
#define SYNCH_RESOLV 0x4000

database *ndg_hosts;
database *ndg_nets;
database *local_site_hosts;
database *local_site_nets;
database *local_user_hosts;
database *local_user_nets;
database *local_user_generic;
database *internal;


/*   Eduoard
enum
{
  USER, SITE, GLOBAL
};

enum
{
  HOSTS, NETS, GENERIC
};

database* DBs[3][3];
*/

GtkWidget *glarea;

GdkPixbuf *readTexture(const char *);
void composite_texture(GdkPixbuf*, const GdkPixbuf*, GdkPixbuf**);
database *readNetDB(const char *);
database *readHostDB(const char *);
database *readGenDB(const char *);
database *init_internal_db(void);
int writeHostDB(database *, const char *);
int writeNetDB(database *, const char *);
int writeGenDB(database *, const char *);
void addToHostDB(database *, const dbentry *);
void addToNetDB(database *, const dbentry *);
void addToGenDB(database *, const dbentry *);
void calltrace(void);
void set_sphere_lod(int);
void set_render_mode(int);
void makeearth(void);
gint new_trace(GtkWidget *, gpointer *);
void tell_user(const char *);
GtkWidget* spinner_new(void);
void spinner_ref(const char *);
void spinner_unref(const char *);
//void spinner_reset(void);
unsigned long distance(int);
gint info_window(gint);
void infowin_change_site(gint);
void partialtorus(const float, const float, const float);
void init_gl(GtkWidget *, gpointer);
char*  locNumToStr(double, int);
double locStrToNum(const char *, int);
double tox(const double, const double);
double toy(const double, const double);
double toz(const double, const double);
double tolat(const double, const double, const double);
double tolon(const double, const double, const double);
void redraw(GtkWidget *, GdkEvent *);
void resolve_by_id(int, char*);
void resolve(site *);
int isin(const char[],const char[]);
void getsuff(const char[], char[]);
void addHost(GtkWidget *);
void addNet(GtkWidget *);
void addGen(GtkWidget *);
void map_texture(const GdkPixbuf *);
void set_transparency(gint);               /* s.g. */
void set_zoom(gint);                       /* s.g. */
void set_view_motion(gfloat,gfloat);       /* s.g. */
gint get_from_extDNS(site*, int, GdkInputCondition);
void callExternalDNS(site*);
void arrangeUnknownSites(void);

int addrInAddr(struct address *net, struct address *ip);
int addr2str(struct address *addr, char *buf, int size);
int str2addr(struct address *addr, const char *buf);

#ifdef XT_DEBUG
#       define DPRINTF printf
#else
        /* Hopefully this gets optimized away, but probably not */
        extern int dontprintf(const char *format, ...);
//#       define DPRINTF printf
#       define DPRINTF dontprintf
#endif

// #define MAX(a, b)  (((a) > (b)) ? (a) : (b)) 
#endif                  /* __xt_h__ */
