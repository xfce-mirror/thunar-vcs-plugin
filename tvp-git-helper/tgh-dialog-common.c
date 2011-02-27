/*-
 * Copyright (C) 2007-2011  Peter de Ridder <peter@xfce.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <gtk/gtk.h>

#include "tgh-dialog-common.h"

static void quit_response (GtkDialog*, gint, gpointer);
static void close_response (GtkDialog*, gint, gpointer);

void
tgh_dialog_start (GtkDialog *dialog, gboolean quit_on_exit)
{
  g_signal_connect (G_OBJECT (dialog), "response", G_CALLBACK (quit_on_exit?quit_response:close_response), NULL);

  gtk_widget_show (GTK_WIDGET (dialog));
}

static void
quit_response (GtkDialog *dialog, gint response, gpointer user_data)
{
  gtk_widget_destroy (GTK_WIDGET (dialog));

  gtk_main_quit();
}

static void
close_response (GtkDialog *dialog, gint response, gpointer user_data)
{
  gtk_widget_destroy (GTK_WIDGET (dialog));
}

void
tgh_dialog_replace_action_area (GtkDialog *dialog)
{
  GtkWidget *box;

  gtk_container_remove (GTK_CONTAINER (GTK_DIALOG (dialog)->vbox), GTK_DIALOG (dialog)->action_area);

  GTK_DIALOG (dialog)->action_area = box = gtk_hbox_new (FALSE, 0);

  gtk_box_pack_end (GTK_BOX (GTK_DIALOG (dialog)->vbox), box,
      FALSE, TRUE, 0);
  gtk_widget_show (box);

  gtk_box_reorder_child (GTK_BOX (GTK_DIALOG (dialog)->vbox), box, 0);
}

void
tgh_make_homogeneous (GtkWidget *first, ...)
{
  GtkWidget *iter;
  GtkRequisition request;
  gint max_width = 0;
  gint max_height = 0;
  va_list ap;

  va_start (ap, first);
  iter = first;
  while (iter)
  {
    gtk_widget_size_request(iter, &request);
    if (request.width > max_width)
      max_width = request.width;
    if (request.height > max_height)
      max_height = request.height;
    iter = va_arg (ap, GtkWidget *);
  }
  va_end (ap);

  va_start (ap, first);
  iter = first;
  while (iter)
  {
    gtk_widget_set_size_request (iter, max_width, max_height);
    iter = va_arg (ap, GtkWidget *);
  }
  va_end (ap);
}

