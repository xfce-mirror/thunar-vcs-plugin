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
#include "tgh-file-selection-dialog.h"
#include "tgh-notify-dialog.h"

#include "tgh-add.h"

static gboolean add_spawn (GtkWidget *dialog, gchar **files, GPid *pid)
{
  GError *error = NULL;
  gint fd_out, fd_err;
  GIOChannel *chan_out, *chan_err;
  TghOutputParser *parser;
  gsize length;
  gint i;
  gchar **argv;

  length = 5;
  length += g_strv_length(files);

  argv = g_new(gchar*, length);

  argv[0] = "git";
  argv[1] = "add";
  argv[2] = "-v";
  argv[3] = "--";
  argv[length-1] = NULL;

  i = 4;
  while(*files)
    argv[i++] = *files++;

  if(!g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, pid, NULL, &fd_out, &fd_err, &error))
  {
    g_free (argv);
    return FALSE;
  }
  g_free (argv);

  parser = tgh_error_parser_new(GTK_WIDGET(dialog));

  g_child_watch_add(*pid, (GChildWatchFunc)tgh_child_exit, parser);

  chan_out = g_io_channel_unix_new(fd_out);
  chan_err = g_io_channel_unix_new(fd_err);
  g_io_add_watch(chan_out, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, tgh_notify_parser_new(GTK_WIDGET(dialog)));
  g_io_add_watch(chan_err, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, parser);

  return TRUE;
}

gboolean tgh_add (gchar **files, GPid *pid)
{
  GtkWidget *dialog;

  if (files)
    if (chdir(files[0]))
    {
      gchar *dirname = g_path_get_dirname (files[0]);
      if (chdir(dirname))
      {
        g_free (dirname);
        return FALSE;
      }
      g_free (dirname);
    }

  dialog = tgh_file_selection_dialog_new (_("Add"), NULL, 0, TGH_FILE_SELECTION_FLAG_MODIFIED|TGH_FILE_SELECTION_FLAG_UNTRACKED);
  if (gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_OK)
  {
    gtk_widget_destroy (dialog);
    return FALSE;
  }
  g_strfreev (files);
  files = tgh_file_selection_dialog_get_files (TGH_FILE_SELECTION_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  if (!files)
    return FALSE;

  dialog = tgh_notify_dialog_new (_("Add"), NULL, 0);
  g_signal_connect (dialog, "cancel-clicked", tgh_cancel, NULL);
  tgh_dialog_start (GTK_DIALOG(dialog), TRUE);

  return add_spawn (dialog, files, pid);
}

