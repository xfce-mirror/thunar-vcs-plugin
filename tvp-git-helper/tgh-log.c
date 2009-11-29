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

#include <libxfce4util/libxfce4util.h>

#include "tgh-common.h"
#include "tgh-dialog-common.h"
#include "tgh-log-dialog.h"

#include "tgh-log.h"

static gboolean log_spawn (TghLogDialog *dialog, gchar **files, GPid *pid)
{
  GError *error = NULL;
  gint fd_out, fd_err;
  GIOChannel *chan_out, *chan_err;
  TghOutputParser *parser;
  gsize length;
  gint i;
  gchar **argv;

  length = 7;
  if(files)
    length += g_strv_length(files);

  argv = g_new(gchar*, length);

  argv[0] = "git";
  argv[1] = "log";
  argv[2] = "--numstat";
  argv[3] = "--pretty=fuller";
  argv[4] = "--boundary";
  argv[5] = "--";
  argv[length-1] = NULL;

  i = 6;
  if(files)
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
  g_io_add_watch(chan_out, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, tgh_log_parser_new(GTK_WIDGET(dialog)));
  g_io_add_watch(chan_err, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, parser);

  return TRUE;
}

static void create_log_child(TghLogDialog *dialog, gpointer user_data)
{
  GPid pid;
  if (log_spawn(dialog, user_data, &pid))
    tgh_replace_child(TRUE, pid);
  else
    tgh_log_dialog_done(dialog);
}

gboolean tgh_log (gchar **files, GPid *pid)
{
  GtkWidget *dialog;

  dialog = tgh_log_dialog_new (NULL, NULL, 0);
  g_signal_connect(dialog, "cancel-clicked", tgh_cancel, NULL);
  tgh_dialog_start (GTK_DIALOG (dialog), TRUE);

  g_signal_connect(dialog, "refresh-clicked", G_CALLBACK(create_log_child), files);

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

  return log_spawn(TGH_LOG_DIALOG(dialog), files, pid);
}

