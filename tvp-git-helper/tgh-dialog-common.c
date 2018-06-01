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
