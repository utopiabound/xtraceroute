/* $Id: spinner.c,v 1.4 2003/03/02 13:41:42 d3august Exp $
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

static gint tag = 0;
static gint spinner_state;
static GtkWidget *label;
static gint refcount = 0;

gint spinner_spin(gpointer data)
{
  switch(spinner_state)
    {
    case 0:
      gtk_label_set(GTK_LABEL(label)," O ");
      spinner_state++;
      break;
    case 1:
      gtk_label_set(GTK_LABEL(label),"O  ");
      spinner_state++;
      break;
    case 2:
      gtk_label_set(GTK_LABEL(label)," O ");
      spinner_state++;
      break;
    case 3:
      gtk_label_set(GTK_LABEL(label),"  O");
      spinner_state = 0;
      break;
    }
    return TRUE;
}

/**
 * References the spinner.
 * The reason is only used for debug purposes.
 */

void spinner_ref(const char* reason)
{

  DPRINTF("DEBUG: spinner_ref:\t\"%s\"\tcount: %d -> %d\n",
          reason, refcount, refcount+1);

  refcount++;

  if(tag == 0)
    tag = gtk_timeout_add (100, spinner_spin, NULL);
}

/**
 * Unreferences the spinner.
 * The reason is only used for debug purposes.
 */

void spinner_unref(const char* reason)
{
  
  DPRINTF("DEBUG: spinner_unref:\t\"%s\"\tcount: %d -> %d\n",
          reason, refcount, refcount-1);
  
  refcount--;
  
  if(refcount > 0)
    return;

  if(refcount < 0)
    printf("Aigh! spinner refcount negative! (%d)\n", refcount);

  if(tag != 0)
    {
      gtk_timeout_remove(tag);
    }
  tag = 0;
  spinner_state = 0;
  if(label)
    gtk_label_set(GTK_LABEL(label)," ");
}


GtkWidget* spinner_new(void)
{
  GtkWidget *spin;

  spin  = gtk_button_new();
  label = gtk_label_new(" ");
  gtk_container_add(GTK_CONTAINER(spin), label);
  /*  
  gtk_signal_connect(GTK_OBJECT(spin), "clicked",
		     (GtkSignalFunc)new_trace,
		     (gpointer)NULL);
  */
  spinner_state = 0;
  gtk_widget_show(label);

  return spin;
}

/*
  void spinner_reset(void)
  {
  refcount = 0;
  
  if(tag != 0)
  gtk_timeout_remove (tag);
  }
*/
