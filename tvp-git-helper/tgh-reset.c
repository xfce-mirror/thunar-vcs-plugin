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
#include "tgh-reset-dialog.h"

#include "tgh-reset.h"

typedef struct
{
  TghOutputParser parent;
  GtkWidget *dialog;
} ResetParser;

static void reset_parser_func(ResetParser *parser, gchar *line)
{
  TghResetDialog *dialog = TGH_RESET_DIALOG(parser->dialog);
  if(line)
  {
    gchar *file = line;
    gchar *state = strchr(line, ':');
    if(state)
    {
      *state++ = '\0';
      state = g_strstrip(state);
    }
    else
    {
      state = "";
      file = g_strstrip(file);
    }

    tgh_reset_dialog_add(dialog, file, state);
  }
  else
  {
    tgh_reset_dialog_done(dialog);
    g_free(parser);
  }
}

static TghOutputParser* reset_parser_new(GtkWidget *dialog)
{
  ResetParser *parser = g_new(ResetParser, 1);

  TGH_OUTPUT_PARSER(parser)->parse = TGH_OUTPUT_PARSER_FUNC(reset_parser_func);

  parser->dialog = dialog;

  return TGH_OUTPUT_PARSER(parser);
}

static gboolean reset_spawn (GtkWidget *dialog, gchar **files, GPid *pid)
{
  GError *error = NULL;
  gint fd_out, fd_err;
  GIOChannel *chan_out, *chan_err;
  TghOutputParser *parser;
  gsize length;
  gint i;
  gchar **argv;

  length = 3;
  length += g_strv_length(files);

  argv = g_new(gchar*, length);

  argv[0] = "git";
  argv[1] = "reset";
  argv[length-1] = NULL;

  i = 2;
  while(*files)
    argv[i++] = *files++;

  if(!g_spawn_async_with_pipes(NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, pid, NULL, &fd_out, &fd_err, &error))
  {
    return FALSE;
  }

  parser = tgh_error_parser_new(GTK_WIDGET(dialog));

  g_child_watch_add(*pid, (GChildWatchFunc)tgh_child_exit, parser);

  chan_out = g_io_channel_unix_new(fd_out);
  chan_err = g_io_channel_unix_new(fd_err);
  g_io_add_watch(chan_out, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, reset_parser_new(dialog));
  g_io_add_watch(chan_err, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, parser);

  return TRUE;
}

gboolean tgh_reset (gchar **files, GPid *pid)
{
  GtkWidget *dialog;

  dialog = tgh_file_selection_dialog_new (_("Reset"), NULL, 0, files, TGH_FILE_SELECTION_FLAG_ADDED);
  if(gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_OK)
  {
    gtk_widget_destroy (dialog);
    return FALSE;
  }
  g_strfreev (files);
  files = tgh_file_selection_dialog_get_files (TGH_FILE_SELECTION_DIALOG (dialog));
  gtk_widget_destroy (dialog);

  if(!files)
    return FALSE;

  dialog = tgh_reset_dialog_new(NULL, NULL, 0);
  g_signal_connect(dialog, "cancel-clicked", tgh_cancel, NULL);
	tgh_dialog_start (GTK_DIALOG (dialog), TRUE);

  return reset_spawn(dialog, files, pid);
}

