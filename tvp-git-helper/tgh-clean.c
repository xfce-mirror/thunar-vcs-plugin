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

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_WAIT_H
#include <sys/wait.h>
#endif

#include <glib.h>
#include <gtk/gtk.h>

#include <libxfce4util/libxfce4util.h>

#include "tgh-common.h"
#include "tgh-dialog-common.h"
#include "tgh-clean-dialog.h"
#include "tgh-notify-dialog.h"

#include "tgh-clean.h"

static gboolean clean_spawn (GtkWidget *dialog, gchar **files, gboolean direcotries, TghCleanIgnore ignore, gboolean force, GPid *pid)
{
  GError *error = NULL;
  gint fd_out, fd_err;
  GIOChannel *chan_out, *chan_err;
  TghOutputParser *parser;
  gsize length;
  gint i;
  gchar **argv;

  length = 5;
  if (direcotries)
    length++;
  if (ignore != TGH_CLEAN_IGNORE_EXCLUDE)
    length++;
  if (force)
    length++;
  length += g_strv_length (files);

  argv = g_new (gchar*, length);

  argv[0] = "git";
  argv[1] = "--no-pager";
  argv[2] = "clean";
  argv[length-1] = NULL;

  i = 3;
  if (direcotries)
    argv[i++] = "-d";
  switch (ignore)
  {
    case TGH_CLEAN_IGNORE_EXCLUDE:
      break;
    case TGH_CLEAN_IGNORE_INCLUDE:
      argv[i++] = "-x";
      break;
    case TGH_CLEAN_IGNORE_ONLY:
      argv[i++] = "-X";
      break;
  }
  if (force)
    argv[i++] = "-f";
  argv[i++] = "--";

  while (*files)
    argv[i++] = *files++;

  if (!g_spawn_async_with_pipes (NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, pid, NULL, &fd_out, &fd_err, &error))
  {
    g_free (argv);
    return FALSE;
  }
  g_free (argv);

  parser = tgh_error_parser_new (GTK_WIDGET (dialog));

  g_child_watch_add (*pid, (GChildWatchFunc)tgh_child_exit, parser);

  chan_out = g_io_channel_unix_new (fd_out);
  chan_err = g_io_channel_unix_new (fd_err);
  g_io_add_watch (chan_out, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, tgh_clean_parser_new (dialog));
  g_io_add_watch (chan_err, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, parser);

  return TRUE;
}

gboolean tgh_clean (gchar **files, GPid *pid)
{
  GtkWidget *dialog;
  gboolean direcotries, force;
  TghCleanIgnore ignore;
  gchar *prefix;

  prefix = tgh_common_prefix (files);

  if (prefix)
  {
    if (chdir(prefix))
    {
      gchar *dirname = g_path_get_dirname (prefix);
      if (chdir(dirname))
      {
        g_free (dirname);
        return FALSE;
      }
      g_free (prefix);
      prefix = dirname;
    }
    files = tgh_strip_prefix (files, prefix);
    g_free (prefix);
  }

  dialog = tgh_clean_dialog_new (NULL, NULL, 0);
  if (gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_OK)
  {
    gtk_widget_destroy (dialog);
    return FALSE;
  }

  direcotries = tgh_clean_dialog_get_diretories (TGH_CLEAN_DIALOG (dialog));
  ignore = tgh_clean_dialog_get_ignore (TGH_CLEAN_DIALOG (dialog));
  force = tgh_clean_dialog_get_force (TGH_CLEAN_DIALOG (dialog));

  gtk_widget_destroy (dialog);

  dialog = tgh_notify_dialog_new (_("Clean"), NULL, 0);
  g_signal_connect (dialog, "cancel-clicked", tgh_cancel, NULL);
  tgh_dialog_start (GTK_DIALOG(dialog), TRUE);

  return clean_spawn (dialog, files, direcotries, ignore, force, pid);
}

