/* $Id: db_host_gui.c,v 1.2 2003/02/22 23:17:09 d3august Exp $
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
#include <stdlib.h>
#include <string.h>

/* Allow updating the database. */
/* get the text uncentered. */


static gint text_has_changed = FALSE;
static gint guard = FALSE;


static GtkWidget *host_entry;
static GtkWidget *ip_entry;
static GtkWidget *lat_entry;
static GtkWidget *lon_entry;
static GtkWidget *info_entry;



static gint destroy_widget_callback(GtkWidget *wi, gpointer *target)
{
  gtk_widget_destroy(GTK_WIDGET(target));
  guard = FALSE;
  return TRUE;
}

static gint yesbutton_callback(GtkWidget *wi, gpointer *data)
{
  dbentry *ent;

  if(text_has_changed)
    {
      //  tell_user(_("Info has changed!\n Updating the database is not yet supported!"));
      text_has_changed = FALSE;
      
      
      ent = (dbentry *)malloc(sizeof(dbentry));

      /* This is a good place to do some sanity/bounds checking! */
      
      ent->lat = atof(gtk_entry_get_text(GTK_ENTRY(lat_entry)));
      ent->lon = atof(gtk_entry_get_text(GTK_ENTRY(lon_entry)));
      strcpy(ent->ip,   gtk_entry_get_text(GTK_ENTRY(ip_entry)));
      strcpy(ent->name, gtk_entry_get_text(GTK_ENTRY(host_entry)));
      strcpy(ent->info, gtk_entry_get_text(GTK_ENTRY(info_entry)));
      
      /* Always add new entries to the private database. */
      addToHostDB(local_user_hosts, ent);
      
      free(ent);
    }
  return TRUE;
}

static gint helpbutton_callback(GtkWidget *wi, gpointer *data)
{
  tell_user(_("When entering lat/lon information, give them as a signed "
	    "floating point number.\n(For example 23.456 or -123.45)\n\n"
	    "Negative numbers for longitude means they're on the western " 
	    "hemisphere.\n"
	    "Negative numbers for latitude means they're on the southern "
	    "hemisphere."));

  return TRUE;
}


static gint text_change_callback(GtkWidget *wi, gpointer *data)
{
    text_has_changed = TRUE;
    return TRUE;
}


void 
add_widget_to_table(GtkWidget* wi, GtkTable* table, gint row, gint column)
{
  if(GTK_IS_MISC(wi))
    gtk_misc_set_alignment(GTK_MISC(wi), column ? 0.0 : 1.0, 0.5);
  gtk_table_attach_defaults(table, GTK_WIDGET(wi), column, column+1, row, row+1);
  gtk_widget_show(GTK_WIDGET(wi));
}

void 
add_string_to_table(const char* string, GtkTable* table, gint row, gint column)
{
  GtkWidget *wi;

  wi = gtk_label_new(string);
  add_widget_to_table(GTK_WIDGET(wi), table, row, column);
}

void addHost(GtkWidget* wi)
{
  GtkWidget *dialog;
  GtkWidget *table;
  GtkWidget *yesbutton;
  GtkWidget *nobutton;
  GtkWidget *helpbutton;
  GtkWidget *title;
  GtkWidget *frame;
  char string[100];
  
  if(guard == TRUE)
    {
      tell_user(_("You can only have one\ndatabase window at at time!"));
      return;
    }
  guard = TRUE;
  
  dialog = gtk_dialog_new();
  
  sprintf(string, _("New host record"));
  title  = gtk_label_new(string);
  gtk_widget_show(GTK_WIDGET(title));
  
  gtk_window_set_title (GTK_WINDOW(dialog),_("Record info"));
  gtk_container_border_width(GTK_CONTAINER(dialog), 10);
  
  table  = gtk_table_new(5,2,FALSE);
  gtk_container_border_width(GTK_CONTAINER(table), 10);

  add_string_to_table(_("Hostname:"),   GTK_TABLE(table), 0, 0);
  add_string_to_table(_("IP number:"),  GTK_TABLE(table), 1, 0);
  add_string_to_table(_("Latitude:"),   GTK_TABLE(table), 2, 0);
  add_string_to_table(_("Longitude:"),  GTK_TABLE(table), 3, 0);
  add_string_to_table(_("Info:"),       GTK_TABLE(table), 4, 0);

  host_entry = gtk_entry_new();
  ip_entry   = gtk_entry_new();
  lat_entry  = gtk_entry_new();
  lon_entry  = gtk_entry_new();
  info_entry = gtk_entry_new();

  gtk_signal_connect(GTK_OBJECT(host_entry),"changed",
		     GTK_SIGNAL_FUNC(text_change_callback), NULL);
  gtk_signal_connect(GTK_OBJECT(ip_entry),  "changed",
		     GTK_SIGNAL_FUNC(text_change_callback), NULL);
  gtk_signal_connect(GTK_OBJECT(lat_entry), "changed",
		     GTK_SIGNAL_FUNC(text_change_callback), NULL);
  gtk_signal_connect(GTK_OBJECT(lon_entry), "changed",
		     GTK_SIGNAL_FUNC(text_change_callback), NULL);
  gtk_signal_connect(GTK_OBJECT(info_entry),"changed",
		     GTK_SIGNAL_FUNC(text_change_callback), NULL);

  add_widget_to_table(GTK_WIDGET(host_entry), GTK_TABLE(table), 0, 1);
  add_widget_to_table(GTK_WIDGET(ip_entry),   GTK_TABLE(table), 1, 1);
  add_widget_to_table(GTK_WIDGET(lat_entry),  GTK_TABLE(table), 2, 1);
  add_widget_to_table(GTK_WIDGET(lon_entry),  GTK_TABLE(table), 3, 1);
  add_widget_to_table(GTK_WIDGET(info_entry), GTK_TABLE(table), 4, 1);

  gtk_widget_show(GTK_WIDGET(table));

  /* Pack everything up */

  frame = gtk_frame_new (_("Host info"));
  gtk_container_border_width (GTK_CONTAINER (frame), 10);
  gtk_container_add(GTK_CONTAINER(frame), table);
  gtk_widget_show(GTK_WIDGET(frame));

  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), title, TRUE,
		     TRUE, 0);
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), frame, TRUE,
		     TRUE, 0);
  
  yesbutton = gtk_button_new_with_label(_("OK"));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), yesbutton,
		     TRUE, TRUE, 0);
  gtk_signal_connect(GTK_OBJECT (yesbutton), "clicked",
		     GTK_SIGNAL_FUNC (yesbutton_callback), dialog);
  gtk_signal_connect(GTK_OBJECT (yesbutton), "clicked",
		     GTK_SIGNAL_FUNC (destroy_widget_callback), dialog);
  GTK_WIDGET_SET_FLAGS(yesbutton, GTK_CAN_DEFAULT);
  gtk_widget_grab_default(yesbutton);
  gtk_widget_show(yesbutton);

  nobutton = gtk_button_new_with_label(_("Cancel"));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), nobutton,
		     TRUE, TRUE, 0);
  gtk_signal_connect(GTK_OBJECT (nobutton), "clicked",
		      GTK_SIGNAL_FUNC(destroy_widget_callback), dialog);
  gtk_widget_show(nobutton);

  helpbutton = gtk_button_new_with_label(_("Help"));
  gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), helpbutton,
		     TRUE, TRUE, 0);
  gtk_signal_connect(GTK_OBJECT (helpbutton), "clicked",
		      GTK_SIGNAL_FUNC(helpbutton_callback), dialog);
  gtk_widget_show(helpbutton);

  gtk_widget_show (dialog);
}
