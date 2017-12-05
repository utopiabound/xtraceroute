/* $Id: dialogs.c,v 1.3 2003/02/22 23:17:09 d3august Exp $
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

#include <unistd.h>
#include <string.h>
#include "xt.h"

static gint destroy_widget_callback(GtkWidget *wi, gpointer *target)
{
  gtk_widget_destroy(GTK_WIDGET(target));

  return TRUE;
}

/** 
 * Open a small window and display the string mess. The window has 
 * an "OK" button that closes the window.
 */
void tell_user(const char *mess)
{
    GtkWidget *dialog;
    GtkWidget *button; 
    GtkWidget *label;
    
    dialog = gtk_dialog_new();
    gtk_container_border_width(GTK_CONTAINER(dialog), 10);
    
    label  = gtk_label_new(mess);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->vbox), label, TRUE,
	    TRUE, 0);
    gtk_widget_show(label);
    
    
    button = gtk_button_new_with_label(_("OK"));
    gtk_signal_connect(GTK_OBJECT (button), "clicked",
	    GTK_SIGNAL_FUNC (destroy_widget_callback), dialog);
    gtk_box_pack_start(GTK_BOX(GTK_DIALOG(dialog)->action_area), button,
	    TRUE, TRUE, 0);
    GTK_WIDGET_SET_FLAGS(button, GTK_CAN_DEFAULT);
    gtk_widget_grab_default(button);
    gtk_widget_show(button);
    
    gtk_widget_show(dialog);    
}

