/*-
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include <thunar-vfs/thunar-vfs.h>

#include "tgh-common.h"
#include "tgh-dialog-common.h"
#include "tgh-status-dialog.h"

#include "tgh-status.h"

static gchar *argv[] = {"git", "status", NULL};

struct proc_args
{
    GtkWidget *dialog;
    gchar *error;
    gchar **files;
};

static gboolean status_spawn (TghStatusDialog *dialog, GPid *pid)
{
  GError *error = NULL;
  gint fd_out;
  gint fd_err;
  GIOChannel *chan_out;
  GIOChannel *chan_err;
  TghOutputParser *parser;

  if(!g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, pid, NULL, &fd_out, &fd_err, &error))
  {
    return FALSE;
  }

  parser = tgh_error_parser_new(GTK_WIDGET(dialog));

  g_child_watch_add(*pid, (GChildWatchFunc)tgh_child_exit, parser);

  chan_out = g_io_channel_unix_new(fd_out);
  chan_err = g_io_channel_unix_new(fd_err);
  g_io_add_watch(chan_out, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, tgh_status_parser_new(GTK_WIDGET(dialog)));
  g_io_add_watch(chan_err, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, parser);

  return TRUE;
}

static void create_status_child(TghStatusDialog *dialog, gpointer user_data)
{
  GPid pid;
  if (status_spawn(dialog, &pid))
    tgh_replace_child(TRUE, pid);
  else
    tgh_status_dialog_done(dialog);
}

gboolean tgh_status (gchar **files, GPid *pid)
{
  GtkWidget *dialog;

  dialog = tgh_status_dialog_new (NULL, NULL, 0);
  g_signal_connect(dialog, "cancel-clicked", tgh_cancel, NULL);
  tgh_dialog_start (GTK_DIALOG (dialog), TRUE);

  g_signal_connect(dialog, "refresh-clicked", G_CALLBACK(create_status_child), NULL);

  if (files)
      chdir(files[0]);

  return status_spawn(TGH_STATUS_DIALOG(dialog), pid);
}

