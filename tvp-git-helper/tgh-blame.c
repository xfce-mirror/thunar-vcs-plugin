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
#include "tgh-blame-dialog.h"

#include "tgh-blame.h"

static gboolean blame_spawn (GtkWidget *dialog, gchar *file, GPid *pid)
{
  GError *error = NULL;
  gint fd_out, fd_err;
  GIOChannel *chan_out, *chan_err;
  TghOutputParser *parser;
  gchar **argv;

  argv = g_new (gchar*, 6);

  argv[0] = "git";
  argv[1] = "--no-pager";
  argv[2] = "blame";
  argv[3] = "--";
  argv[4] = file;
  argv[5] = NULL;

  if (!g_spawn_async_with_pipes (NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, pid, NULL, &fd_out, &fd_err, &error))
  {
    g_free (argv);
    return FALSE;
  }
  g_free (argv);

  parser = tgh_error_parser_new (dialog);

  g_child_watch_add (*pid, (GChildWatchFunc)tgh_child_exit, parser);

  chan_out = g_io_channel_unix_new (fd_out);
  chan_err = g_io_channel_unix_new (fd_err);
  g_io_add_watch (chan_out, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, tgh_blame_parser_new (dialog));
  g_io_add_watch (chan_err, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, parser);

  return TRUE;
}

gboolean tgh_blame (gchar **files, GPid *pid)
{
  GtkWidget *dialog;

  if (!files)
    return FALSE;

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

  dialog = tgh_blame_dialog_new (NULL, NULL, 0);
  g_signal_connect (dialog, "cancel-clicked", tgh_cancel, NULL);
  tgh_dialog_start (GTK_DIALOG(dialog), TRUE);

  return blame_spawn (dialog, files[0], pid);
}

