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
#include "tgh-notify-dialog.h"

#include "tgh-move.h"

struct exit_args
{
  TghOutputParser *parser;
  GtkWidget *dialog;
};

static void child_exit (GPid pid, gint status, gpointer user_data)
{
  struct exit_args *args = user_data;

  gtk_widget_destroy (args->dialog);

  if (WEXITSTATUS (status) <= 1)
  {
    GtkWidget *dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_OTHER, GTK_BUTTONS_CLOSE, _("Move finished"));
    tgh_dialog_start (GTK_DIALOG (dialog), TRUE);
  }

  tgh_child_exit (pid, status, args->parser);

  g_free (args);
}

static gboolean move_spawn (GtkWidget *dialog, gchar **files, gchar *dest, GPid *pid)
{
  GError *error = NULL;
  gint fd_err;
  GIOChannel *chan_err;
  TghOutputParser *parser;
  gsize length;
  gint i;
  gchar **argv;
  struct exit_args *args;

  length = 5;
  length += g_strv_length (files);

  argv = g_new (gchar*, length);

  argv[0] = "git";
  argv[1] = "--no-pager";
  argv[2] = "mv";
  argv[length-1] = NULL;

  i = 3;
  while (*files)
    argv[i++] = *files++;

  argv[i] = dest;

  if (!g_spawn_async_with_pipes (NULL, argv, NULL, G_SPAWN_DO_NOT_REAP_CHILD | G_SPAWN_SEARCH_PATH, NULL, NULL, pid, NULL, NULL, &fd_err, &error))
  {
    g_free (argv);
    return FALSE;
  }
  g_free (argv);

  parser = tgh_error_parser_new (NULL);

  args = g_new (struct exit_args, 1);
  args->parser = parser;
  args->dialog = dialog;

  g_child_watch_add (*pid, (GChildWatchFunc)child_exit, args);

  chan_err = g_io_channel_unix_new (fd_err);
  g_io_add_watch (chan_err, G_IO_IN|G_IO_HUP, (GIOFunc)tgh_parse_output_func, parser);

  return TRUE;
}

gboolean tgh_move (gchar **files, GPid *pid)
{
  GtkWidget *dialog;
  gchar *to;
  gboolean multiple = FALSE;
  gchar *prefix;

  if (files && files[0] && files[1])
    multiple = TRUE;

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

  dialog = gtk_file_chooser_dialog_new (_("Move To"), NULL,
      multiple?GTK_FILE_CHOOSER_ACTION_SELECT_FOLDER:GTK_FILE_CHOOSER_ACTION_SAVE,
      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
      GTK_STOCK_OK, GTK_RESPONSE_OK,
      NULL);

  if (gtk_dialog_run (GTK_DIALOG (dialog)) != GTK_RESPONSE_OK)
  {
    gtk_widget_destroy (dialog);
    return FALSE;
  }

  to = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

  gtk_widget_destroy (dialog);

  dialog = gtk_message_dialog_new (NULL, 0, GTK_MESSAGE_OTHER, GTK_BUTTONS_CANCEL, _("Move ..."));
  g_signal_connect (G_OBJECT (dialog), "response", G_CALLBACK (tgh_cancel), NULL);
  tgh_dialog_start (GTK_DIALOG (dialog), TRUE);

  return move_spawn (dialog, files, to, pid);
}

